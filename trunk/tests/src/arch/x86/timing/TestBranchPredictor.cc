/*
 *  Multi2Sim
 *  Copyright (C) 2014  Shi Dong (dong.sh@husky.neu.edu)
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

#include "gtest/gtest.h"

#include <vector>

#include <lib/cpp/IniFile.h>
#include <arch/x86/emu/UInst.h>
#include <arch/x86/timing/BranchPredictor.h>
#include <arch/x86/timing/Uop.h>

namespace x86
{

TEST(TestBranchPredictor, read_ini_configuration_file)
{
	// Setup configuration file
	std::string config =
		"[ BranchPredictor ]\n"
		"Kind = Perfect\n"
		"BTB.Sets = 128\n"
		"BTB.Assoc = 16\n"
		"Bimod.Size = 512\n"
		"Choice.Size = 2048\n"
		"RAS.Size = 64\n"
		"TwoLevel.L1Size = 2\n"
		"TwoLevel.L2Size = 2048\n"
		"TwoLevel.HistorySize = 16";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Find target section
	for (int i = 0; i < ini_file.getNumSections(); i++)
	{
		std::string section_name = ini_file.getSection(i);
		if (misc::StringPrefix(section_name, "BranchPredictor"))
		{
			BranchPredictor::ParseConfiguration(section_name, ini_file);
		}
	}

	// Assertions
	EXPECT_EQ(BranchPredictor::KindPerfect, BranchPredictor::getKind());
	EXPECT_EQ(128, BranchPredictor::getBTBSets());
	EXPECT_EQ(16, BranchPredictor::getBTBAssociativity());
	EXPECT_EQ(512, BranchPredictor::getBimodSize());
	EXPECT_EQ(2048, BranchPredictor::getChoiceSize());
	EXPECT_EQ(64, BranchPredictor::getRasSize());
	EXPECT_EQ(2, BranchPredictor::getTwolevelL1size());
	EXPECT_EQ(2048, BranchPredictor::getTwolevelL2size());
	EXPECT_EQ(16, BranchPredictor::getTwolevelHistorySize());
}


TEST(TestBranchPredictor, test_bimodal_branch_predictor_1)
{
	// Local variable declaration
	Uop *uop;
	BranchPredictorPred pred;
	char bimod_status;
	unsigned int branch_addr = 0;
	unsigned int branch_inst_size = 4;
	unsigned int branch_target_distance = 8;

	// Setup configuration file for branch preditor
	std::string config =
			"[ BranchPredictor ]\n"
			"Kind = Bimodal\n"
			"Bimod.Size = 512";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Setup mock micro-op list
	std::vector<std::unique_ptr<Uop>> mock_uop_list;

	// First micro-operation (Taken)
	uop = new Uop;
	UInst uinst_1(UInstBranch);
	uop->setUInst(&uinst_1);
	uop->setFlags(UInstFlagCtrl | UInstFlagCond);
	uop->setEip(branch_addr);
	uop->setNeip(branch_addr + branch_target_distance);
	uop->setMopSize(branch_inst_size);
	mock_uop_list.emplace_back(uop);

	// Second micro-operation (Not Taken)
	uop = new Uop;
	UInst uinst_2(UInstBranch);
	uop->setUInst(&uinst_2);
	uop->setFlags(UInstFlagCtrl | UInstFlagCond);
	uop->setEip(branch_addr);
	uop->setNeip(branch_addr + branch_inst_size);
	uop->setMopSize(branch_inst_size);
	mock_uop_list.emplace_back(uop);

	// Third micro-operation (Not Taken)
	uop = new Uop;
	UInst uinst_3(UInstBranch);
	uop->setUInst(&uinst_3);
	uop->setFlags(UInstFlagCtrl | UInstFlagCond);
	uop->setEip(branch_addr);
	uop->setNeip(branch_addr + branch_inst_size);
	uop->setMopSize(branch_inst_size);
	mock_uop_list.emplace_back(uop);

	// Parse configuration
	BranchPredictor::ParseConfiguration("BranchPredictor", ini_file);

	// Create a branch predictor instance
	BranchPredictor branch_predictor;

	// Test branch predictor
	// Weakly Taken -> Strongly Taken -> Weakly Taken -> Weakly Not Taken
	// 2 -> 3 -> 2 -> 1
	// Prediction: Taken -> Taken -> Taken
	for (unsigned int i = 0; i < mock_uop_list.size(); i++)
	{
		branch_predictor.LookupBranchPrediction(*(mock_uop_list[i]));
		branch_predictor.UpdateBranchPredictor(*(mock_uop_list[i]));
		pred = mock_uop_list[i]->getPrediction();
		bimod_status = branch_predictor.getBimodStatus(mock_uop_list[i]->getBimodIndex());
	}

	// Assertions
	EXPECT_EQ(BranchPredictorPredTaken, pred);
	EXPECT_EQ(1, (int)bimod_status);
}

}




