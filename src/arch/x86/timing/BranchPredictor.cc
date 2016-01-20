/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <lib/cpp/Misc.h>

#include "BranchPredictor.h"
#include "Uop.h"

namespace x86
{

BranchPredictor::Kind BranchPredictor::kind;
int BranchPredictor::btb_num_sets;
int BranchPredictor::btb_num_ways;
int BranchPredictor::ras_size;
int BranchPredictor::bimod_size;
int BranchPredictor::choice_size;
int BranchPredictor::two_level_l1_size;
int BranchPredictor::two_level_l2_size;
int BranchPredictor::two_level_history_size;
int BranchPredictor::two_level_l2_height;

misc::StringMap BranchPredictor::KindMap =
{
	{ "Perfect", KindPerfect},
	{ "Taken", KindTaken },
	{"NotTaken", KindNottaken},
	{"Bimodal", KindBimod},
	{"TwoLevel", KindTwoLevel},
	{"Combined", KindCombined}
};

BranchPredictor::BranchPredictor(const std::string &name)
	:
	name(name)
{
	// Initialize
	ras = misc::new_unique_array<int>(ras_size);
	
	// Bimodal predictor
	if (kind == KindBimod || kind == KindCombined)
	{
		bimod = misc::new_unique_array<char>(bimod_size);
		for (int i = 0; i < bimod_size; i++)
			bimod[i] = (char)2;
	}

	// Two-level adaptive branch predictor
	if (kind == KindTwoLevel || kind == KindCombined)
	{
		two_level_bht = misc::new_unique_array<unsigned int>(two_level_l1_size);
		two_level_pht = misc::new_unique_array<char>(two_level_l2_size * two_level_l2_height);
		for (int i = 0; i < two_level_l2_size * two_level_l2_height; i++)
			two_level_pht[i] = (char)2;
	}

	// Choice predictor
	if (kind == KindCombined)
	{
		choice = misc::new_unique_array<char>(choice_size);
		for (int i = 0; i < choice_size; i++)
			choice[i] = 2;
	}

	// Allocate BTB and assign LRU counters
	btb = misc::new_unique_array<BtbEntry>(btb_num_sets * btb_num_ways);
	for (int i = 0; i < btb_num_sets; i++)
		for (int j = 0; j < btb_num_ways; j++)
			btb[i * btb_num_ways + j].counter = j;
}


void BranchPredictor::ParseConfiguration(misc::IniFile *ini_file)
{
	// section
	std::string section = "BranchPredictor";

	// Load branch predictor type
	kind = (Kind) ini_file->ReadEnum(section, "Kind",
			KindMap, KindTwoLevel);

	// Load branch predictor parameter
	btb_num_sets = ini_file->ReadInt(section, "BTB.Sets", 256);
	btb_num_ways = ini_file->ReadInt(section, "BTB.Assoc", 4);
	bimod_size = ini_file->ReadInt(section, "Bimod.Size", 1024);
	choice_size = ini_file->ReadInt(section, "Choice.Size", 1024);
	ras_size = ini_file->ReadInt(section, "RAS.Size", 32);
	two_level_l1_size = ini_file->ReadInt(section, "TwoLevel.L1Size", 1);
	two_level_l2_size = ini_file->ReadInt(section, "TwoLevel.L2Size", 1024);
	two_level_history_size = ini_file->ReadInt(section, "TwoLevel.HistorySize", 8);

	// Two-level branch predictor parameter
	two_level_l2_height = 1 << two_level_history_size;

	// Integrity
	if (bimod_size & (bimod_size - 1))
		throw Error("number of entries in bimodal precitor must be a power of 2");
	if (choice_size & (choice_size - 1))
		throw Error("number of entries in choice predictor must be power of 2");
	if (btb_num_sets & (btb_num_sets - 1))
		throw Error("number of BTB sets must be a power of 2");
	if (btb_num_ways & (btb_num_ways - 1))
		throw Error("BTB associativity must be a power of 2");
	if (two_level_history_size < 1 || two_level_history_size > 30)
		throw Error("predictor history size must be >=1 and <=30");
	if (two_level_l1_size & (two_level_l1_size - 1))
		throw Error("two-level predictor sizes must be power of 2");
	if (two_level_l2_size & (two_level_l2_size - 1))
		throw Error("two-level predictor sizes must be power of 2");
}


void BranchPredictor::DumpConfiguration(std::ostream &os)
{
	// Dump branch predictor information
	os << misc::fmt("\n***** BranchPredictor *****\n");
	os << misc::fmt("\tBTB.Sets: %d\n", btb_num_sets);
	os << misc::fmt("\tBTB.Assoc: %d\n", btb_num_ways);
	os << misc::fmt("\tBimod.Size: %d\n", bimod_size);
	os << misc::fmt("\tChoice.Size: %d\n", choice_size);
	os << misc::fmt("\tRAS.Size: %d\n", ras_size);
	os << misc::fmt("\tTwoLevel.L1Size: %d\n", two_level_l1_size);
	os << misc::fmt("\tTwoLevel.L2Size: %d\n", two_level_l2_size);
	os << misc::fmt("\tTwoLevel.HistorySize: %d\n", two_level_history_size);
}


BranchPredictor::Prediction BranchPredictor::Lookup(Uop *uop)
{
	// Local variable
	Prediction prediction;

	// If branch predictor is accessed, a BTB hit must have occurred before, which
	// provides information about the branch, i.e., target address and whether it
	// is a call, ret, jump, or conditional branch. Thus, branches other than
	// conditional ones are always predicted taken.
	assert(uop->getFlags() & Uinst::FlagCtrl);
	if (uop->getFlags() & Uinst::FlagUncond)
	{
		uop->prediction = PredictionTaken;
		return PredictionTaken;
	}

	// An internal branch (string operations) is always predicted taken
	if (uop->getUinst()->getOpcode() == Uinst::OpcodeIbranch)
	{
		uop->prediction = PredictionTaken;
		return PredictionTaken;
	}

	// Perfect predictor
	if (kind == KindPerfect)
	{
		if (uop->neip != uop->eip + uop->mop_size)
			prediction = PredictionTaken;
		else
			prediction = PredictionNotTaken;
		uop->prediction = prediction;
	}

	// Taken predictor
	if (kind == KindTaken)
		uop->prediction = PredictionTaken;

	// Not-taken predictor
	if (kind == KindNottaken)
		uop->prediction = PredictionNotTaken;

	// Bimodal predictor
	if (kind == KindBimod || kind == KindCombined)
	{
		int bimod_index = uop->eip & (bimod_size - 1);
		Prediction bimod_prediction = bimod[bimod_index] > 1 ?
				PredictionTaken :
				PredictionNotTaken;
		uop->bimod_index = bimod_index;
		uop->bimod_prediction = bimod_prediction;
		uop->prediction = bimod_prediction;
	}

	// Two-level adaptive
	if (kind == KindTwoLevel || kind == KindCombined)
	{
		int two_level_bht_index = uop->eip & (two_level_l1_size - 1);
		int two_level_pht_row = two_level_bht[two_level_bht_index];
		assert(two_level_pht_row < two_level_l2_height);
		int two_level_pht_col = uop->eip & (two_level_l2_size - 1);
		Prediction two_level_prediction = two_level_pht[two_level_pht_row * two_level_l2_size + two_level_pht_col] > 1 ?
				PredictionTaken : PredictionNotTaken;
		uop->two_level_bht_index = two_level_bht_index;
		uop->two_level_pht_row = two_level_pht_row;
		uop->two_level_pht_col = two_level_pht_col;
		uop->two_level_prediction = two_level_prediction;
		uop->prediction = two_level_prediction;
	}

	// Combined
	if (kind == KindCombined)
	{
		int choice_index = uop->eip & (choice_size - 1);
		Prediction choice_prediction = choice[choice_index] > 1 ?
				uop->two_level_prediction :
				uop->bimod_prediction;
		uop->choice_index = choice_index;
		uop->choice_prediction = choice_prediction;
		uop->prediction = choice_prediction;
	}

	// Return prediction
	assert(uop->prediction == PredictionTaken || uop->prediction == PredictionNotTaken);
	return uop->prediction;
}


int BranchPredictor::LookupMultiple(unsigned int eip, int count)
{

	// First make a regular prediction. This updates the necessary fields in
	// the uop for a later call to UpdateBranchPredictor(), and makes the
	// first prediction considering known characteristics of the primary
	// branch.
	assert(kind == KindTwoLevel);
	int bht_index = eip & (two_level_l1_size - 1);
	int pht_row = two_level_bht[bht_index];
	assert(pht_row < two_level_l2_height);
	int pht_col = eip & (two_level_l2_size - 1);
	int prediction = two_level_pht[pht_row * two_level_l2_size + pht_col] > 1 ?
			PredictionTaken : PredictionNotTaken;

	// Make the rest of predictions
	int temp_prediction = prediction;
	for (int i = 1; i < count; i++)
	{
		pht_row = ((pht_row << 1) | temp_prediction) & (two_level_l2_height - 1);
		temp_prediction = two_level_pht[pht_row * two_level_l2_size + pht_col] > 1;
		assert(!temp_prediction || temp_prediction == 1);
		prediction |= temp_prediction << i;
	}

	// Return
	return prediction;
}


void BranchPredictor::Update(Uop *uop)
{
	// Taken/NotTaken flag
	bool taken;

	// pointer to 2-bit counter
	char *bimod_ptr;

	// pointer to two-level branch history table
	unsigned int *bht_ptr;

	// pointer to two-level pattern history table
	char *pht_ptr;

	// pointer to combined branch prediction table
	char *choice_ptr;

	assert(!uop->speculative_mode);
	assert(uop->getFlags() & Uinst::FlagCtrl);
	taken = uop->neip != uop->eip + uop->mop_size;

	// Stats
	accesses++;
	if (uop->neip == uop->predicted_neip)
		hits++;

	// Update predictors. This is only done for conditional branches. Thus,
	// exit now if instruction is a call, ret, or jmp.
	// No update is performed in a perfect branch predictor either.
	if (kind == KindPerfect)
		return;
	if (uop->getFlags() & Uinst::FlagUncond)
		return;

	// Bimodal predictor was used
	if (kind == KindBimod ||
			(kind == KindCombined && uop->choice_prediction == PredictionNotTaken))
	{
		bimod_ptr = &bimod[uop->bimod_index];
		if (taken)
			*bimod_ptr = *bimod_ptr + 1 > 3 ? 3 : *bimod_ptr + 1;
		else
			*bimod_ptr = *bimod_ptr - 1 < 0 ? 0 : *bimod_ptr - 1;
	}

	// Two-level adaptive predictor was used
	if (kind == KindTwoLevel || (kind == KindCombined &&
			uop->choice_prediction == PredictionTaken))
	{
		// Shift entry in BHT (level 1), and append direction
		bht_ptr = &two_level_bht[uop->two_level_bht_index];
		*bht_ptr = ((*bht_ptr << 1) | taken) & (two_level_l2_height - 1);

		// Update counter in PHT (level 2) as per direction
		pht_ptr = &two_level_pht[uop->two_level_pht_row *
		                            two_level_l2_size + uop->two_level_pht_col];
		if (taken)
			*pht_ptr = *pht_ptr + 1 > 3 ? 3 : *pht_ptr + 1;
		else
			*pht_ptr = *pht_ptr - 1 < 0 ? 0 : *pht_ptr - 1;
	}

	// Choice predictor - update only if bimodal and two-level
	// predictions differ.
	if (kind == KindCombined && uop->bimod_prediction != uop->two_level_prediction)
	{
		choice_ptr = &choice[uop->choice_index];
		if (uop->bimod_prediction == PredictionTaken)
			*choice_ptr = *choice_ptr - 1 < 0 ? 0 : *choice_ptr - 1;
		else
			*choice_ptr = *choice_ptr + 1 > 3 ? 3 : *choice_ptr + 1;
	}
}


unsigned int BranchPredictor::LookupBtb(Uop *uop)
{
	// Local variable
	BtbEntry *entry;
	unsigned int target = 0;
	bool hit = false;

	// Assertion
	assert(uop->getFlags() & Uinst::FlagCtrl);

	// Perfect branch predictor
	if (kind == KindPerfect)
		return uop->neip;

	// Internal branch (string operations) always predicted to jump to itself
	if (uop->getUinst()->getOpcode() == Uinst::OpcodeIbranch)
		return uop->eip;

	// Search address in BTB
	int set = uop->eip & (btb_num_sets - 1);
	for (int way = 0; way < btb_num_ways; way++)
	{
		entry = &btb[set * btb_num_ways + way];
		if (entry->source != uop->eip)
			continue;
		target = entry->target;
		hit = true;
		break;
	}

	// If there was a hit, we know whether branch is a call.
	// In this case, push return address into RAS. To avoid
	// updates at recovery, do it only for non-spec instructions.
	if (hit && uop->getUinst()->getOpcode() == Uinst::OpcodeCall
			&& !uop->speculative_mode)
	{
		ras[ras_index] = uop->eip + uop->mop_size;
		ras_index = (ras_index + 1) % ras_size;
	}

	// If there was a hit, we know whether branch is a ret. In this case,
	// pop target from the RAS, and ignore target obtained from BTB.
	if (hit && uop->getUinst()->getOpcode() == Uinst::OpcodeRet
			&& !uop->speculative_mode)
	{
		ras_index = (ras_index + ras_size - 1) % ras_size;
		target = ras[ras_index];
	}

	// Return
	return target;
}


void BranchPredictor::UpdateBtb(Uop *uop)
{
	// Local variable
	BtbEntry *entry;
	BtbEntry *found_entry;
	bool found = false;

	// No update for perfect branch predictor
	if (kind == KindPerfect)
		return;

	// Search address in BTB
	int set = uop->eip & (btb_num_sets - 1);
	for (int way = 0; way < btb_num_ways; way++)
	{
		entry = &btb[set * btb_num_ways + way];
		if (entry->source == uop->eip)
		{
			found = true;
			found_entry = entry;
			break;
		}
	}

	// If address was not found, evict LRU entry
	if (!found)
	{
		for (int way = 0; way < btb_num_ways; way++)
		{
			entry = &btb[set * btb_num_ways + way];
			entry->counter--;
			if (entry->counter < 0) {
				entry->counter = btb_num_ways - 1;
				entry->source = uop->eip;
				entry->target = uop->neip;
			}
		}
	}

	// If address was found, update LRU counters and target
	if (found)
	{
		for (int way = 0; way < btb_num_ways; way++)
		{
			entry = &btb[set * btb_num_ways + way];
			if (entry->counter > found_entry->counter)
				entry->counter--;
		}
		found_entry->counter = btb_num_ways - 1;
		found_entry->target = uop->neip;
	}
}


unsigned int BranchPredictor::getNextBranch(unsigned int eip,
		unsigned int block_size)
{
	// Sanity check
	assert(!(block_size & (block_size - 1)));

	// Trying to find the next branch address within one block size
	unsigned int ret_addr = 0;
	unsigned int limit = (eip + block_size) & ~(block_size - 1);
	while (eip < limit)
	{
		int set = eip & (btb_num_sets - 1);
		for (int way = 0; way < btb_num_ways; way++)
		{
			BtbEntry *entry = &btb[set * btb_num_ways + way];
			if (entry->source == eip)
			{
				ret_addr = eip;
				break;
			}
		}
		eip++;
	}

	// Return
	return ret_addr;
}

}
