/*
 *  Multi2Sim
 *  Copyright (C) 2015  Shi Dong (dong.sh@husky.neu.edu)
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
#include <arch/x86/emulator/Emulator.h>
#include <arch/x86/emulator/Uinst.h>
#include <arch/x86/timing/BranchPredictor.h>
#include <arch/x86/timing/Core.h>
#include <arch/x86/timing/Cpu.h>
#include <arch/x86/timing/Thread.h>
#include <arch/x86/timing/Timing.h>
#include <arch/x86/timing/Uop.h>

#include "ObjectPool.h"

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
	BranchPredictor::ParseConfiguration(&ini_file);

	// Assertions
	EXPECT_EQ(BranchPredictor::KindPerfect, BranchPredictor::getKind());
	EXPECT_EQ(128, BranchPredictor::getBtbNumSets());
	EXPECT_EQ(16, BranchPredictor::getBtbNumWays());
	EXPECT_EQ(512, BranchPredictor::getBimodSize());
	EXPECT_EQ(2048, BranchPredictor::getChoiceSize());
	EXPECT_EQ(64, BranchPredictor::getRasSize());
	EXPECT_EQ(2, BranchPredictor::getTwoLevelL1Size());
	EXPECT_EQ(2048, BranchPredictor::getTwoLevelL2Size());
	EXPECT_EQ(16, BranchPredictor::getTwoLevelHistorySize());
}


TEST(TestBranchPredictor, test_bimodal_branch_predictor_1)
{
	// Setup configuration file for branch predictor
	std::string config =
			"[ BranchPredictor ]\n"
			"Kind = Bimodal\n"
			"Bimod.Size = 512";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Local variable declaration
	const int num_branch_uop = 3;
	BranchPredictor::Prediction pred;
	char bimod_status;
	unsigned int branch_addr = 0;
	unsigned int branch_inst_size = 4;
	unsigned int branch_target_distance = 8;
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Setup mock micro-op list
	std::vector<std::unique_ptr<Uop>> uops;

	// First micro-operation (Taken)
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeBranch);
	uops.emplace_back(misc::new_unique<Uop>(
			object_pool->getThread(),
			object_pool->getContext(),
			uinst_1));
	Uop *uop = uops.back().get();
	uop->eip = branch_addr;
	uop->neip = branch_addr + branch_target_distance;
	uop->mop_size = branch_inst_size;

	// Second micro-operation (Not Taken)
	auto uinst_2 = misc::new_shared<Uinst>(Uinst::OpcodeBranch);
	uops.emplace_back(misc::new_unique<Uop>(
			object_pool->getThread(),
			object_pool->getContext(),
			uinst_2));
	uop = uops.back().get();
	uop->eip = branch_addr;
	uop->neip = branch_addr + branch_inst_size;
	uop->mop_size = branch_inst_size;

	// Third micro-operation (Not Taken)
	auto uinst_3 = misc::new_shared<Uinst>(Uinst::OpcodeBranch);
	uops.emplace_back(misc::new_unique<Uop>(
			object_pool->getThread(),
			object_pool->getContext(),
			uinst_2));
	uop = uops.back().get();
	uop->eip = branch_addr;
	uop->neip = branch_addr + branch_inst_size;
	uop->mop_size = branch_inst_size;

	// Parse configuration
	BranchPredictor::ParseConfiguration(&ini_file);

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
	int bimod_index[num_branch_uop] =
	{
		0,
		0,
		0
	};
	for (unsigned int i = 0; i < uops.size(); i++)
	{
		// Look up predictor and verify prediction
		branch_predictor.Lookup(uops[i].get());
		pred = uops[i]->prediction;
		EXPECT_EQ(pred_result[i], pred);

		// Verify the bimodal index
		EXPECT_EQ(bimod_index[i], uops[i]->bimod_index);

		// Update predictor and verify the bimodal status
		branch_predictor.Update(uops[i].get());
		bimod_status = branch_predictor.getBimodStatus(uops[i]->bimod_index);
		EXPECT_EQ(bimod_status_trace[i], (int) bimod_status);
	}
}


TEST(TestBranchPredictor, test_two_level_branch_predictor_1)
{
	// Local variable declaration
	const int num_branch_uop = 3;
	const int N = 3;
	BranchPredictor::Prediction pred;
	int bht_status;
	char pht_status;
	unsigned int branch_addr;
	unsigned int branch_inst_size = 4;
	unsigned int branch_target_distance = 8;
	ObjectPool *object_pool = ObjectPool::getInstance();;

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
	BranchPredictor::ParseConfiguration(&ini_file);

	// Create a branch predictor instance
	BranchPredictor branch_predictor;

	// Setup mock micro-op list
	std::vector<std::unique_ptr<Uop>> uops;

	//Shared by different Uop, but this is incorrect in real scenario
	auto uinst = misc::new_shared<Uinst>(Uinst::OpcodeBranch);
	Uop *uop;

	// Create N pattern
	for (int i = 0; i < N; i++)
	{
		// First micro-operation (Taken)
		branch_addr = 0;
		uops.emplace_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst));
		uop = uops.back().get();
		uop->eip = branch_addr;
		uop->neip = branch_addr + branch_target_distance;
		uop->mop_size = branch_inst_size;

		// Second micro-operation (Not Taken)
		branch_addr = 16;
		uops.emplace_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst));
		uop = uops.back().get();
		uop->eip = branch_addr;
		uop->neip = branch_addr + branch_inst_size;
		uop->mop_size = branch_inst_size;

		// Third micro-operation (Not Taken)
		branch_addr = 32;
		uops.emplace_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst));
		uop = uops.back().get();
		uop->eip = branch_addr;
		uop->neip = branch_addr + branch_inst_size;
		uop->mop_size = branch_inst_size;
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
	int bht_index = 0;
	int pht_row[num_branch_uop * N] =
	{
		0b000000,
		0b000001,
		0b000010,
		0b000100,
		0b001001,
		0b010010,
		0b100100,
		0b001001,
		0b010010
	};
	int pht_col[num_branch_uop * N] =
	{
		0,
		16,
		32,
		0,
		16,
		32,
		0,
		16,
		32
	};
	for (unsigned int i = 0; i < uops.size(); i++)
	{
		// Look up predictor and verify prediction
		branch_predictor.Lookup(uops[i].get());
		pred = uops[i]->prediction;
		EXPECT_EQ(pred_result[i], pred);

		// Verify the BHT index and PHT row colomn
		EXPECT_EQ(bht_index, uops[i]->two_level_bht_index);
		EXPECT_EQ(pht_row[i], uops[i]->two_level_pht_row);
		EXPECT_EQ(pht_col[i], uops[i]->two_level_pht_col);

		// Update predictor and verify the two-level branch predictor status
		branch_predictor.Update(uops[i].get());
		bht_status = branch_predictor.getTwoLevelBhtStatus(uops[i]->two_level_bht_index);
		pht_status = branch_predictor.getTwoLevelPhtStatus(uops[i]->two_level_pht_row,
				uops[i]->two_level_pht_col);
		EXPECT_EQ(bht_status_trace[i], bht_status);
		EXPECT_EQ(pht_status_trace[i], (int)pht_status);
	}
}


TEST(TestBranchPredictor, test_combined_branch_predictor_1)
{
	// Local variable declaration
	const int num_branch_uop = 3;
	const int N = 3;
	BranchPredictor::Prediction bimodal_pred;
	BranchPredictor::Prediction two_level_pred;
	BranchPredictor::Prediction pred;
	int bht_status;
	char pht_status;
	int bimodal_status;
	int choice_status;
	unsigned int branch_addr;
	unsigned int branch_inst_size = 4;
	unsigned int branch_target_distance = 8;
	ObjectPool *object_pool = ObjectPool::getInstance();;

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
	BranchPredictor::ParseConfiguration(&ini_file);

	// Create a branch predictor instance
	BranchPredictor branch_predictor;

	// Setup mock micro-op list
	std::vector<std::unique_ptr<Uop>> uops;

	//Shared by different Uop, but this is incorrect in real scenario
	auto uinst = misc::new_shared<Uinst>(Uinst::OpcodeBranch);
	Uop *uop;

	// Create N pattern
	for (int i = 0; i < N; i++)
	{
		// First micro-operation (Taken)
		branch_addr = 0;
		uops.emplace_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst));
		uop = uops.back().get();
		uop->eip = branch_addr;
		uop->neip = branch_addr + branch_target_distance;
		uop->mop_size = branch_inst_size;

		// Second micro-operation (Not Taken)
		branch_addr = 16;
		uops.emplace_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst));
		uop = uops.back().get();


		uop->eip = branch_addr;
		uop->neip = branch_addr + branch_inst_size;
		uop->mop_size = branch_inst_size;

		// Third micro-operation (Not Taken)
		branch_addr = 32;
		uops.emplace_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst));
		uop = uops.back().get();


		uop->eip = branch_addr;
		uop->neip = branch_addr + branch_inst_size;
		uop->mop_size = branch_inst_size;
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
	BranchPredictor::Prediction two_level_pred_result[num_branch_uop * N] =
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
	for (unsigned int i = 0; i < uops.size(); i++)
	{
		// Look up predictor and verify prediction
		branch_predictor.Lookup(uops[i].get());
		two_level_pred = uops[i]->two_level_prediction;
		bimodal_pred = uops[i]->bimod_prediction;
		pred = uops[i]->prediction;
		EXPECT_EQ(two_level_pred_result[i], two_level_pred);
		EXPECT_EQ(bimodal_pred_result[i], bimodal_pred);
		EXPECT_EQ(choice_pred_result[i], pred);

		// Update predictor and verify the two-level branch predictor status
		branch_predictor.Update(uops[i].get());
		bht_status = branch_predictor.getTwoLevelBhtStatus(uops[i]->two_level_bht_index);
		pht_status = branch_predictor.getTwoLevelPhtStatus(uops[i]->two_level_pht_row,
				uops[i]->two_level_pht_col);
		bimodal_status = branch_predictor.getBimodStatus(uops[i]->bimod_index);
		choice_status = branch_predictor.getChoiceStatus(uops[i]->choice_index);
		EXPECT_EQ(bht_status_trace[i], bht_status);
		EXPECT_EQ(pht_status_trace[i], (int)pht_status);
		EXPECT_EQ(bimodal_status_trace[i], bimodal_status);
		EXPECT_EQ(choice_status_trace[i], choice_status);
	}
}


}




