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

#include <string>

#include <lib/cpp/IniFile.h>
#include <lib/cpp/Misc.h>

#include "BranchPredictor.h"
#include "Thread.h"
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
int BranchPredictor::twolevel_hist_size;
int BranchPredictor::twolevel_l2height;


void BranchPredictor::Create(std::string &branch_predictor_name)
{
	// Initialize
	name = branch_predictor_name;
	ras = std::unique_ptr<int[]> (new int[ras_size]);

	/* Bimodal predictor */
	if (kind == KindBimod || kind == KindCombined)
	{
		bimod = std::unique_ptr<char[]> (new char[bimod_size]);
		for (int i = 0; i < bimod_size; i++)
			bimod[i] = (char)2;
	}

	// Two-level adaptive branch predictor
	if (kind == KindTwolevel || kind == KindCombined)
	{
		twolevel_bht = std::unique_ptr<unsigned int[]> (new unsigned int[twolevel_l1size]);
		twolevel_pht = std::unique_ptr<char[]> (new char[twolevel_l2size * twolevel_l2height]);
		for (int i = 0; i < twolevel_l2size * twolevel_l2height; i++)
			twolevel_pht[i] = (char)2;
	}

	// Choice predictor
	if (kind == KindCombined)
	{
		choice = std::unique_ptr<char[]> (new char[choice_size]);
		for (int i = 0; i < choice_size; i++)
			choice[i] = 2;
	}

	// Allocate BTB and assign LRU counters
	btb = std::unique_ptr<BTBEntry[]>(new BTBEntry[btb_sets * btb_assoc]);
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
	twolevel_hist_size = config.ReadInt(section, "TwoLevel.HistorySize", 8);

	// Two-level branch predictor parameter
	twolevel_l2height = 1 << twolevel_hist_size;

	// Integrity
	if (bimod_size & (bimod_size - 1))
		misc::fatal("number of entries in bimodal precitor must be a power of 2");
	if (choice_size & (choice_size - 1))
		misc::fatal("number of entries in choice predictor must be power of 2");
	if (btb_sets & (btb_sets - 1))
		misc::fatal("number of BTB sets must be a power of 2");
	if (btb_assoc & (btb_assoc - 1))
		misc::fatal("BTB associativity must be a power of 2");
	if (twolevel_hist_size < 1 || twolevel_hist_size > 30)
		misc::fatal("predictor history size must be >=1 and <=30");
	if (twolevel_l1size & (twolevel_l1size - 1))
		misc::fatal("two-level predictor sizes must be power of 2");
	if (twolevel_l2size & (twolevel_l2size - 1))
		misc::fatal("two-level predictor sizes must be power of 2");
}


void BranchPredictor::DumpConfig(std::ostream &os)
{
	// Dump network information
	os << misc::fmt("\n***** BranchPredictor *****\n");
	os << misc::fmt("\tBTB.Sets: %d\n", btb_sets);
	os << misc::fmt("\tBTB.Assoc: %d\n", btb_assoc);
	os << misc::fmt("\tBimod.Size: %d\n", bimod_size);
	os << misc::fmt("\tChoice.Size: %d\n", choice_size);
	os << misc::fmt("\tRAS.Size: %d\n", ras_size);
	os << misc::fmt("\tTwoLevel.L1Size: %d\n", twolevel_l1size);
	os << misc::fmt("\tTwoLevel.L2Size: %d\n", twolevel_l2size);
	os << misc::fmt("\tTwoLevel.HistorySize: %d\n", twolevel_hist_size);
}

}
