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
	BranchPredictor::ParseConfiguration("BranchPredictor", ini_file);

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
	const int num_branch_uop = 3;
	Uop *uop;
	BranchPredictor::Prediction pred;
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
	mock_uop_list.emplace_back(misc::new_unique<Uop>());
	uop = mock_uop_list.back().get();
	UInst uinst_1(UInstBranch);
	uop->setUInst(&uinst_1);
	uop->setFlags(UInstFlagCtrl | UInstFlagCond);
	uop->setEip(branch_addr);
	uop->setNeip(branch_addr + branch_target_distance);
	uop->setMopSize(branch_inst_size);

	// Second micro-operation (Not Taken)
	mock_uop_list.emplace_back(misc::new_unique<Uop>());
	uop = mock_uop_list.back().get();
	UInst uinst_2(UInstBranch);
	uop->setUInst(&uinst_2);
	uop->setFlags(UInstFlagCtrl | UInstFlagCond);
	uop->setEip(branch_addr);
	uop->setNeip(branch_addr + branch_inst_size);
	uop->setMopSize(branch_inst_size);

	// Third micro-operation (Not Taken)
	mock_uop_list.emplace_back(misc::new_unique<Uop>());
	uop = mock_uop_list.back().get();
	UInst uinst_3(UInstBranch);
	uop->setUInst(&uinst_3);
	uop->setFlags(UInstFlagCtrl | UInstFlagCond);
	uop->setEip(branch_addr);
	uop->setNeip(branch_addr + branch_inst_size);
	uop->setMopSize(branch_inst_size);

	// Parse configuration
	BranchPredictor::ParseConfiguration("BranchPredictor", ini_file);

	// Create a branch predictor instance
	BranchPredictor branch_predictor;

	// Test branch predictor
	// Weakly Taken -> Strongly Taken -> Weakly Taken -> Weakly Not Taken
	// 2 -> 3 -> 2 -> 1
	// Prediction: Taken -> Taken -> Taken
	BranchPredictor::Prediction pred_result[num_branch_uop] =
	{
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken
	};
	int bimod_status_trace[num_branch_uop] =
	{
		3,
		2,
		1
	};
	for (unsigned int i = 0; i < mock_uop_list.size(); i++)
	{
		// Look up predictor and verify prediction
		branch_predictor.LookupBranchPrediction(*(mock_uop_list[i]));
		pred = mock_uop_list[i]->getPrediction();
		EXPECT_EQ(pred_result[i], pred);

		// Update predictor and verify the bimodal status
		branch_predictor.UpdateBranchPredictor(*(mock_uop_list[i]));
		bimod_status = branch_predictor.getBimodStatus(mock_uop_list[i]->getBimodIndex());
		EXPECT_EQ(bimod_status_trace[i], (int)bimod_status);
	}
}


TEST(TestBranchPredictor, test_twolevel_branch_predictor_1)
{
	// Local variable declaration
	const int num_branch_uop = 3;
	const int N = 3;
	Uop *uop;
	BranchPredictor::Prediction pred;
	int bht_status;
	char pht_status;
	unsigned int branch_addr;
	unsigned int branch_inst_size = 4;
	unsigned int branch_target_distance = 8;

	// Setup configuration file for branch preditor
	std::string config =
			"[ BranchPredictor ]\n"
			"Kind = TwoLevel\n"
			"TwoLevel.L1Size = 1\n"
			"TwoLevel.L2Size = 512\n"
			"TwoLevel.HistorySize = 6";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Parse configuration
	BranchPredictor::ParseConfiguration("BranchPredictor", ini_file);

	// Create a branch predictor instance
	BranchPredictor branch_predictor;

	// Setup mock micro-op list
	std::vector<std::unique_ptr<Uop>> mock_uop_list;

	//Shared by different Uop, but this is incorrect in real scenario
	UInst uinst(UInstBranch);

	// Create N pattern
	for (int i = 0; i < N; i++)
	{
		// First micro-operation (Taken)
		branch_addr = 0;
		mock_uop_list.emplace_back(misc::new_unique<Uop>());
		uop = mock_uop_list.back().get();
		uop->setUInst(&uinst);
		uop->setFlags(UInstFlagCtrl | UInstFlagCond);
		uop->setEip(branch_addr);
		uop->setNeip(branch_addr + branch_target_distance);
		uop->setMopSize(branch_inst_size);

		// Second micro-operation (Not Taken)
		branch_addr = 16;
		mock_uop_list.emplace_back(misc::new_unique<Uop>());
		uop = mock_uop_list.back().get();
		uop->setUInst(&uinst);
		uop->setFlags(UInstFlagCtrl | UInstFlagCond);
		uop->setEip(branch_addr);
		uop->setNeip(branch_addr + branch_inst_size);
		uop->setMopSize(branch_inst_size);

		// Third micro-operation (Not Taken)
		branch_addr = 32;
		mock_uop_list.emplace_back(misc::new_unique<Uop>());
		uop = mock_uop_list.back().get();
		uop->setUInst(&uinst);
		uop->setFlags(UInstFlagCtrl | UInstFlagCond);
		uop->setEip(branch_addr);
		uop->setNeip(branch_addr + branch_inst_size);
		uop->setMopSize(branch_inst_size);
	}


	// Test branch predictor N = 3
	// History based on each branch instruction
	//
	// Since L1Size = 1, there is only one entry that contains the history
	// 000000->000001->000010->000100->001001->010010->100100->
	// 001001->010010->100100
	//
	// The prediction for each branch instruction should be
	// Taken; Taken; Taken; Taken; Taken; Taken; Taken; NotTaken; NotTaken
	//
	// For each PHT entry, the status is different
	// [000000]: Strongly Taken; [000001]: Weakly NotTaken; [000010]: Weakly NotTaken
	// [000100]: Strongly Taken; [001001]: Weakly NotTaken; [010010]: Weakly NotTaken
	// [100100]: Strongly Taken; [001001]: NotTaken; [010010]: NotTaken
	BranchPredictor::Prediction pred_result[num_branch_uop * N] =
	{
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionTaken,
		BranchPredictor::PredictionNotTaken,
		BranchPredictor::PredictionNotTaken
	};
	int bht_status_trace[num_branch_uop * N] =
	{
		0b000001,
		0b000010,
		0b000100,
		0b001001,
		0b010010,
		0b100100,
		0b001001,
		0b010010,
		0b100100
	};
	int pht_status_trace[num_branch_uop * N] =
	{
		3,
		1,
		1,
		3,
		1,
		1,
		3,
		0,
		0
	};
	for (unsigned int i = 0; i < mock_uop_list.size(); i++)
	{
		// Look up predictor and verify prediction
		branch_predictor.LookupBranchPrediction(*(mock_uop_list[i]));
		pred = mock_uop_list[i]->getPrediction();
		EXPECT_EQ(pred_result[i], pred);

		// Update predictor and verify the two-level branch predictor status
		branch_predictor.UpdateBranchPredictor(*(mock_uop_list[i]));
		bht_status = branch_predictor.getTwolevelBHTStatus(mock_uop_list[i]->getTwolevelBHTIndex());
		pht_status = branch_predictor.getTwolevelPHTStatus(mock_uop_list[i]->getTwolevelPHTRow(),
				mock_uop_list[i]->getTwolevelPHTCol());
		EXPECT_EQ(bht_status_trace[i], bht_status);
		EXPECT_EQ(pht_status_trace[i], (int)pht_status);
	}
}


TEST(TestBranchPredictor, test_combined_branch_predictor_1)
{
	// Local variable declaration
	const int num_branch_uop = 3;
	const int N = 3;
	Uop *uop;
	BranchPredictor::Prediction bimodal_pred;
	BranchPredictor::Prediction twolevel_pred;
	BranchPredictor::Prediction pred;
	int bht_status;
	char pht_status;
	int bimodal_status;
	int choice_status;
	unsigned int branch_addr;
	unsigned int branch_inst_size = 4;
	unsigned int branch_target_distance = 8;

	// Setup configuration file for branch preditor
	std::string config =
			"[ BranchPredictor ]\n"
			"Kind = Combined\n"
			"Bimod.Size = 512\n"
			"Choice.Size = 2048\n"
			"TwoLevel.L1Size = 1\n"
			"TwoLevel.L2Size = 512\n"
			"TwoLevel.HistorySize = 6";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Parse configuration
	BranchPredictor::ParseConfiguration("BranchPredictor", ini_file);

	// Create a branch predictor instance
	BranchPredictor branch_predictor;

	// Setup mock micro-op list
	std::vector<std::unique_ptr<Uop>> mock_uop_list;

	//Shared by different Uop, but this is incorrect in real scenario
	UInst uinst(UInstBranch);

	// Create N pattern
	for (int i = 0; i < N; i++)
	{
		// First micro-operation (Taken)
		branch_addr = 0;
		mock_uop_list.emplace_back(misc::new_unique<Uop>());
		uop = mock_uop_list.back().get();
		uop->setUInst(&uinst);
		uop->setFlags(UInstFlagCtrl | UInstFlagCond);
		uop->setEip(branch_addr);
		uop->setNeip(branch_addr + branch_target_distance);
		uop->setMopSize(branch_inst_size);

		// Second micro-operation (Not Taken)
		branch_addr = 16;
		mock_uop_list.emplace_back(misc::new_unique<Uop>());
		uop = mock_uop_list.back().get();
		uop->setUInst(&uinst);
		uop->setFlags(UInstFlagCtrl | UInstFlagCond);
		uop->setEip(branch_addr);
		uop->setNeip(branch_addr + branch_inst_size);
		uop->setMopSize(branch_inst_size);

		// Third micro-operation (Not Taken)
		branch_addr = 32;
		mock_uop_list.emplace_back(misc::new_unique<Uop>());
		uop = mock_uop_list.back().get();
		uop->setUInst(&uinst);
		uop->setFlags(UInstFlagCtrl | UInstFlagCond);
		uop->setEip(branch_addr);
		uop->setNeip(branch_addr + branch_inst_size);
		uop->setMopSize(branch_inst_size);
	}


	///////////////////////////////
	// Test branch predictor N = 3
	///////////////////////////////

	// Below is the intermediate and final results of two level branch predictor
	// History based on each branch instruction
	//
	// Since L1Size = 1, there is only one entry that contains the history
	// 000000->000001->000010->000100->001001->010010->100100->
	// 001001->001001->010010
	//
	// The prediction for each branch instruction should be
	// Taken; Taken; Taken; Taken; Taken; Taken; Taken; NotTaken; NotTaken
	//
	// For each PHT entry, the status is different
	// [000000]: Strongly Taken; [000001]: Weakly NotTaken; [000010]: Weakly NotTaken
	// [000100]: Strongly Taken; [001001]: Weakly NotTaken; [010010]: Weakly NotTaken
	// [100100]: Strongly Taken; [001001]: Weakly NotTaken; [001001]: Weakly NotTaken
	BranchPredictor::Prediction twolevel_pred_result[num_branch_uop * N] =
	{
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionNotTaken,
			BranchPredictor::PredictionTaken
	};
	int bht_status_trace[num_branch_uop * N] =
	{
			0b000001,
			0b000010,
			0b000100,
			0b001001,
			0b010010,
			0b100100,
			0b001001,
			0b001001,
			0b010010
	};
	int pht_status_trace[num_branch_uop * N] =
	{
			3,
			1,
			1,
			3,
			1,
			1,
			3,
			1,
			1
	};

	// Below is the intermediate and final results of two level branch predictor
	// For branch instruction with address 0:
	// Weakly Taken(2) -> Weakly Taken(2) -> Weakly Taken(2) -> Weakly Taken(2)
	// Prediction: Taken -> Taken -> Taken
	//
	// For branch instruction with address 16:
	// Weakly Taken(2) -> Weakly Taken(2) -> Weakly Taken(2) -> Weakly Not Taken(1)
	// Prediction: Taken -> Taken -> Taken
	//
	// For branch instruction with address 32:
	// Weakly Taken(2) -> Weakly Taken(2) -> Weakly Taken(2) -> Weakly Taken(2)
	// Prediction: Taken -> Taken -> Taken
	BranchPredictor::Prediction bimodal_pred_result[num_branch_uop * N] =
	{
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken
	};
	int bimodal_status_trace[num_branch_uop * N] =
	{
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			1,
			2
	};

	// Below is the intermediate and final results of combined branch predictor
	// For branch instruction with address 0:
	// Choice status: 2 -> 2 -> 2 -> 2
	// Prediction: Taken -> Taken -> Taken
	//
	// For branch instruction with address 16:
	// Choice status: 2 -> 2 -> 2 -> 1
	// Prediction: Taken -> Taken -> Not Taken
	//
	// For branch instruction with address 16:
	// Choice status: 2 -> 2 -> 2 -> 2
	// Prediction: Taken -> Taken -> Taken
	BranchPredictor::Prediction choice_pred_result[num_branch_uop * N] =
	{
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionTaken,
			BranchPredictor::PredictionNotTaken,
			BranchPredictor::PredictionTaken
	};
	int choice_status_trace[num_branch_uop * N] =
	{
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			1,
			2
	};

	// Start the test and verifiction
	for (unsigned int i = 0; i < mock_uop_list.size(); i++)
	{
		// Look up predictor and verify prediction
		branch_predictor.LookupBranchPrediction(*(mock_uop_list[i]));
		twolevel_pred = mock_uop_list[i]->getTwolevelPrediction();
		bimodal_pred = mock_uop_list[i]->getBimodPrediction();
		pred = mock_uop_list[i]->getPrediction();
		EXPECT_EQ(twolevel_pred_result[i], twolevel_pred);
		EXPECT_EQ(bimodal_pred_result[i], bimodal_pred);
		EXPECT_EQ(choice_pred_result[i], pred);

		// Update predictor and verify the two-level branch predictor status
		branch_predictor.UpdateBranchPredictor(*(mock_uop_list[i]));
		bht_status = branch_predictor.getTwolevelBHTStatus(mock_uop_list[i]->getTwolevelBHTIndex());
		pht_status = branch_predictor.getTwolevelPHTStatus(mock_uop_list[i]->getTwolevelPHTRow(),
				mock_uop_list[i]->getTwolevelPHTCol());
		bimodal_status = branch_predictor.getBimodStatus(mock_uop_list[i]->getBimodIndex());
		choice_status = branch_predictor.getChoiceStatus(mock_uop_list[i]->getChoiceIndex());
		EXPECT_EQ(bht_status_trace[i], bht_status);
		EXPECT_EQ(pht_status_trace[i], (int)pht_status);
		EXPECT_EQ(bimodal_status_trace[i], bimodal_status);
		EXPECT_EQ(choice_status_trace[i], choice_status);
	}
}

}




