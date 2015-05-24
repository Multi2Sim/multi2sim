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
int BranchPredictor::btb_sets;
int BranchPredictor::btb_assoc;
int BranchPredictor::ras_size;
int BranchPredictor::bimod_size;
int BranchPredictor::choice_size;
int BranchPredictor::twolevel_l1size;
int BranchPredictor::twolevel_l2size;
int BranchPredictor::twolevel_history_size;
int BranchPredictor::twolevel_l2height;

misc::StringMap BranchPredictor::KindMap =
{
	{ "Perfect", KindPerfect},
	{ "Taken", KindTaken },
	{"NotTaken", KindNottaken},
	{"Bimodal", KindBimod},
	{"TwoLevel", KindTwolevel},
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
	if (kind == KindTwolevel || kind == KindCombined)
	{
		twolevel_bht = misc::new_unique_array<unsigned int>(twolevel_l1size);
		twolevel_pht = misc::new_unique_array<char>(twolevel_l2size * twolevel_l2height);
		for (int i = 0; i < twolevel_l2size * twolevel_l2height; i++)
			twolevel_pht[i] = (char)2;
	}

	// Choice predictor
	if (kind == KindCombined)
	{
		choice = misc::new_unique_array<char>(choice_size);
		for (int i = 0; i < choice_size; i++)
			choice[i] = 2;
	}

	// Allocate BTB and assign LRU counters
	btb = misc::new_unique_array<BTBEntry>(btb_sets * btb_assoc);
	for (int i = 0; i < btb_sets; i++)
		for (int j = 0; j < btb_assoc; j++)
			btb[i * btb_assoc + j].counter = j;
}


void BranchPredictor::ParseConfiguration(const std::string &section,
		misc::IniFile &config)
{
	// Load branch predictor type
	kind = (Kind)config.ReadEnum(section, "Kind",
			KindMap, KindTwolevel);

	// Load branch predictor parameter
	btb_sets = config.ReadInt(section, "BTB.Sets", 256);
	btb_assoc = config.ReadInt(section, "BTB.Assoc", 4);
	bimod_size = config.ReadInt(section, "Bimod.Size", 1024);
	choice_size = config.ReadInt(section, "Choice.Size", 1024);
	ras_size = config.ReadInt(section, "RAS.Size", 32);
	twolevel_l1size = config.ReadInt(section, "TwoLevel.L1Size", 1);
	twolevel_l2size = config.ReadInt(section, "TwoLevel.L2Size", 1024);
	twolevel_history_size = config.ReadInt(section, "TwoLevel.HistorySize", 8);

	// Two-level branch predictor parameter
	twolevel_l2height = 1 << twolevel_history_size;

	// Integrity
	if (bimod_size & (bimod_size - 1))
		misc::fatal("number of entries in bimodal precitor must be a power of 2");
	if (choice_size & (choice_size - 1))
		misc::fatal("number of entries in choice predictor must be power of 2");
	if (btb_sets & (btb_sets - 1))
		misc::fatal("number of BTB sets must be a power of 2");
	if (btb_assoc & (btb_assoc - 1))
		misc::fatal("BTB associativity must be a power of 2");
	if (twolevel_history_size < 1 || twolevel_history_size > 30)
		misc::fatal("predictor history size must be >=1 and <=30");
	if (twolevel_l1size & (twolevel_l1size - 1))
		misc::fatal("two-level predictor sizes must be power of 2");
	if (twolevel_l2size & (twolevel_l2size - 1))
		misc::fatal("two-level predictor sizes must be power of 2");
}


void BranchPredictor::DumpConfiguration(std::ostream &os)
{
	// Dump branch predictor information
	os << misc::fmt("\n***** BranchPredictor *****\n");
	os << misc::fmt("\tBTB.Sets: %d\n", btb_sets);
	os << misc::fmt("\tBTB.Assoc: %d\n", btb_assoc);
	os << misc::fmt("\tBimod.Size: %d\n", bimod_size);
	os << misc::fmt("\tChoice.Size: %d\n", choice_size);
	os << misc::fmt("\tRAS.Size: %d\n", ras_size);
	os << misc::fmt("\tTwoLevel.L1Size: %d\n", twolevel_l1size);
	os << misc::fmt("\tTwoLevel.L2Size: %d\n", twolevel_l2size);
	os << misc::fmt("\tTwoLevel.HistorySize: %d\n", twolevel_history_size);
}


BranchPredictorPred BranchPredictor::LookupBranchPrediction(Uop &uop)
{
	// Local variable
	BranchPredictorPred pred;

	// If branch predictor is accessed, a BTB hit must have occurred before, which
	// provides information about the branch, i.e., target address and whether it
	// is a call, ret, jump, or conditional branch. Thus, branches other than
	// conditional ones are always predicted taken.
	assert(uop.getFlags() & UInstFlagCtrl);
	if (uop.getFlags() & UInstFlagUncond)
	{
		uop.setPrediction(BranchPredictorPredTaken);
		return BranchPredictorPredTaken;
	}

	// An internal branch (string operations) is always predicted taken
	if (uop.getUinst()->getOpcode() == UInstIbranch)
	{
		uop.setPrediction(BranchPredictorPredTaken);
		return BranchPredictorPredTaken;
	}

	// Perfect predictor
	if (kind == KindPerfect)
	{
		if (uop.getNeip() != uop.getEip() + uop.getMopSize())
			pred = BranchPredictorPredTaken;
		else
			pred = BranchPredictorPredNotTaken;
		uop.setPrediction(pred);
	}

	// Taken predictor
	if (kind == KindTaken)
	{
		uop.setPrediction(BranchPredictorPredTaken);
	}

	// Not-taken predictor
	if (kind == KindNottaken)
	{
		uop.setPrediction(BranchPredictorPredNotTaken);
	}

	// Bimodal predictor
	if (kind == KindBimod || kind == KindCombined)
	{
		int bimod_index = uop.getEip() & (bimod_size - 1);
		BranchPredictorPred bimod_pred = bimod[bimod_index] > 1 ?
				BranchPredictorPredTaken : BranchPredictorPredNotTaken;
		uop.setBimodIndex(bimod_index);
		uop.setBimodPrediction(bimod_pred);
		uop.setPrediction(bimod_pred);
	}

	// Two-level adaptive
	if (kind == KindTwolevel || kind == KindCombined)
	{
		int twolevel_bht_index = uop.getEip() & (twolevel_l1size - 1);
		int twolevel_pht_row = twolevel_bht[twolevel_bht_index];
		assert(twolevel_pht_row < twolevel_l2height);
		int twolevel_pht_col = uop.getEip() & (twolevel_l2size - 1);
		BranchPredictorPred twolevel_pred = twolevel_pht[twolevel_pht_row * twolevel_l2size + twolevel_pht_col] > 1 ?
				BranchPredictorPredTaken : BranchPredictorPredNotTaken;
		uop.setTwolevelBHTIndex(twolevel_bht_index);
		uop.setTwolevelPHTRow(twolevel_pht_row);
		uop.setTwolevelPHTCol(twolevel_pht_col);
		uop.setTwolevelPrediction(twolevel_pred);
		uop.setPrediction(twolevel_pred);
	}

	// Combined
	if (kind == KindCombined)
	{
		int choice_index = uop.getEip() & (choice_size - 1);
		BranchPredictorPred choice_pred = choice[choice_index] > 1 ?
				uop.getTwolevelPrediction() : uop.getBimodPrediction();
		uop.setChoiceIndex(choice_index);
		uop.setPrediction(choice_pred);
	}

	// Return prediction
	assert(uop.getPrediction() == BranchPredictorPredTaken ||
			uop.getPrediction() == BranchPredictorPredNotTaken);
	return uop.getPrediction();
}


int BranchPredictor::LookupBranchPredictionMultiple(unsigned int eip, int count)
{
	// Local variable declaration
	int pred, temp_pred;

	// First make a regular prediction. This updates the necessary fields in the
	// uop for a later call to UpdateBranchPredictor(), and makes the first prediction
	// considering known characteristics of the primary branch.
	assert(kind == KindTwolevel);
	int bht_index = eip & (twolevel_l1size - 1);
	int pht_row = twolevel_bht[bht_index];
	assert(pht_row < twolevel_l2height);
	int pht_col = eip & (twolevel_l2size - 1);
	pred = temp_pred = twolevel_pht[pht_row * twolevel_l2size + pht_col] > 1 ?
			BranchPredictorPredTaken : BranchPredictorPredNotTaken;

	// Make the rest of predictions
	for (int i = 1; i < count; i++)
	{
		pht_row = ((pht_row << 1) | temp_pred) & (twolevel_l2height - 1);
		temp_pred = twolevel_pht[pht_row * twolevel_l2size + pht_col] > 1;
		assert(!temp_pred || temp_pred == 1);
		pred |= temp_pred << i;
	}

	// Return
	return pred;
}


void BranchPredictor::UpdateBranchPredictor(Uop &uop)
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

	assert(!uop.getSpeculativeMode());
	assert(uop.getFlags() & UInstFlagCtrl);
	taken = uop.getNeip() != uop.getEip() + uop.getMopSize();

	// Stats
	accesses++;
	if (uop.getNeip() == uop.getPredictedNeip())
		hits++;

	// Update predictors. This is only done for conditional branches. Thus,
	// exit now if instruction is a call, ret, or jmp.
	// No update is performed in a perfect branch predictor either.
	if (kind == KindPerfect)
		return;
	if (uop.getFlags() & UInstFlagUncond)
		return;

	// Bimodal predictor was used
	if (kind == KindBimod ||
			(kind == KindCombined && uop.getChoicePrediction() == BranchPredictorPredNotTaken))
	{
		bimod_ptr = &bimod[uop.getBimodIndex()];
		if (taken)
			*bimod_ptr = *bimod_ptr + 1 > 3 ? 3 : *bimod_ptr + 1;
		else
			*bimod_ptr = *bimod_ptr - 1 < 0 ? 0 : *bimod_ptr - 1;
	}

	// Two-level adaptive predictor was used
	if (kind == KindTwolevel ||
			(kind == KindCombined && uop.getChoicePrediction() == BranchPredictorPredTaken))
	{
		// Shift entry in BHT (level 1), and append direction
		bht_ptr = &twolevel_bht[uop.getTwolevelBHTIndex()];
		*bht_ptr = ((*bht_ptr << 1) | taken) & (twolevel_l2height - 1);

		// Update counter in PHT (level 2) as per direction
		pht_ptr = &twolevel_pht[uop.getTwolevelPHTRow() *
		                            twolevel_l2size + uop.getTwolevelPHTCol()];
		if (taken)
			*pht_ptr = *pht_ptr + 1 > 3 ? 3 : *pht_ptr + 1;
		else
			*pht_ptr = *pht_ptr - 1 < 0 ? 0 : *pht_ptr - 1;
	}

	// Choice predictor - update only if bimodal and two-level
	// predictions differ.
	if (kind == KindCombined && uop.getBimodPrediction() != uop.getTwolevelPrediction())
	{
		choice_ptr = &choice[uop.getChoiceIndex()];
		if (uop.getBimodPrediction() == BranchPredictorPredTaken)
			*choice_ptr = *choice_ptr - 1 < 0 ? 0 : *choice_ptr - 1;
		else
			*choice_ptr = *choice_ptr + 1 > 3 ? 3 : *choice_ptr + 1;
	}
}


unsigned int BranchPredictor::LookupBTB(Uop &uop)
{
	// Local variable
	BTBEntry *entry;
	unsigned int target = 0;
	bool hit = false;

	// Assertion
	assert(uop.getFlags() & UInstFlagCtrl);

	// Perfect branch predictor
	if (kind == KindPerfect)
		return uop.getNeip();

	// Internal branch (string operations) always predicted to jump to itself
	if (uop.getUinst()->getOpcode() == UInstIbranch)
		return uop.getEip();

	// Search address in BTB
	int set = uop.getEip() & (btb_sets - 1);
	for (int way = 0; way < btb_assoc; way++)
	{
		entry = &btb[set * btb_assoc + way];
		if (entry->source != uop.getEip())
			continue;
		target = entry->target;
		hit = true;
		break;
	}

	// If there was a hit, we know whether branch is a call.
	// In this case, push return address into RAS. To avoid
	// updates at recovery, do it only for non-spec instructions.
	if (hit && uop.getUinst()->getOpcode() == UInstCall && !uop.getSpeculativeMode())
	{
		ras[ras_index] = uop.getEip() + uop.getMopSize();
		ras_index = (ras_index + 1) % ras_size;
	}

	// If there was a hit, we know whether branch is a ret. In this case,
	// pop target from the RAS, and ignore target obtained from BTB.
	if (hit && uop.getUinst()->getOpcode() == UInstRet && !uop.getSpeculativeMode())
	{
		ras_index = (ras_index + ras_size - 1) % ras_size;
		target = ras[ras_index];
	}

	// Return
	return target;
}


void BranchPredictor::UpdateBTB(Uop &uop)
{
	// Local variable
	BTBEntry *entry;
	BTBEntry *found_entry;
	bool found = false;

	// No update for perfect branch predictor
	if (kind == KindPerfect)
		return;

	// Search address in BTB
	int set = uop.getEip() & (btb_sets - 1);
	for (int way = 0; way < btb_assoc; way++)
	{
		entry = &btb[set * btb_assoc + way];
		if (entry->source == uop.getEip())
		{
			found = true;
			found_entry = entry;
			break;
		}
	}

	// If address was not found, evict LRU entry
	if (!found)
	{
		for (int way = 0; way < btb_assoc; way++)
		{
			entry = &btb[set * btb_assoc + way];
			entry->counter--;
			if (entry->counter < 0) {
				entry->counter = btb_assoc - 1;
				entry->source = uop.getEip();
				entry->target = uop.getNeip();
			}
		}
	}

	// If address was found, update LRU counters and target
	if (found)
	{
		for (int way = 0; way < btb_assoc; way++)
		{
			entry = &btb[set * btb_assoc + way];
			if (entry->counter > found_entry->counter)
				entry->counter--;
		}
		found_entry->counter = btb_assoc - 1;
		found_entry->target = uop.getNeip();
	}
}


unsigned int BranchPredictor::GetNextBranch(unsigned int eip, unsigned int block_size)
{
	// Local variable
	BTBEntry *entry;
	int set;
	unsigned int ret_addr = 0;

	// Argument sanity check
	assert(!(block_size & (block_size - 1)));

	// Trying to find the next branch address within one block size
	unsigned int limit = (eip + block_size) & ~(block_size - 1);
	while (eip < limit)
	{
		set = eip & (btb_sets - 1);
		for (int way = 0; way < btb_assoc; way++)
		{
			entry = &btb[set * btb_assoc + way];
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
