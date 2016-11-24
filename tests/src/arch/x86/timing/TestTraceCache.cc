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

#include <string>

#include <lib/cpp/IniFile.h>
#include <arch/x86/emulator/Emulator.h>
#include <arch/x86/emulator/Uinst.h>
#include <arch/x86/timing/TraceCache.h>
#include <arch/x86/timing/Core.h>
#include <arch/x86/timing/Cpu.h>
#include <arch/x86/timing/Thread.h>
#include <arch/x86/timing/Timing.h>
#include <arch/x86/timing/Uop.h>

#include "ObjectPool.h"


namespace x86
{


enum UopInstanceType
{
	UopInstanceTypeBranch = 0,
	UopInstanceTypeOther
};

// Set up a static branch instruction
static auto uinst_branch = misc::new_shared<Uinst>(Uinst::OpcodeBranch);

// Set up a static move instruction
static auto uinst_move = misc::new_shared<Uinst>(Uinst::OpcodeMove);

static std::vector<std::unique_ptr<Uop>> uop_list;

void ParseUopInstance(UopInstanceType instance_type, int address,
		int instruction_size, int target_distance, bool taken)
{
	// Local variable
	Uop *uop;
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Generate uop and insert it into Uop list
	switch (instance_type)
	{
	case UopInstanceTypeBranch:

		uop_list.push_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst_branch));
		uop = uop_list.back().get();
		uop->eip = address;
		if (taken)
			uop->neip = address + target_distance;
		else
			uop->neip = address + instruction_size;
		uop->mop_size = instruction_size;
		break;

	case UopInstanceTypeOther:

		uop_list.push_back(misc::new_unique<Uop>(
				object_pool->getThread(),
				object_pool->getContext(),
				uinst_move));
		uop = uop_list.back().get();
		uop->eip = address;
		uop->mop_size = instruction_size;
		break;

	default:

		break;
	}
}


TEST(TestTraceCache, read_ini_configuration_file)
{
	// Setup configuration file
	std::string config =
			"[ TraceCache ]\n"
			"Present = True\n"
			"Sets = 128\n"
			"Assoc = 8\n"
			"TraceSize = 32\n"
			"BranchMax = 5\n"
			"QueueSize = 64";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Find target section
	TraceCache::ParseConfiguration(&ini_file);

	// Assertions
	EXPECT_EQ(true, TraceCache::isPresent());
	EXPECT_EQ(128, TraceCache::getNumSets());
	EXPECT_EQ(8, TraceCache::getNumWays());
	EXPECT_EQ(32, TraceCache::getTraceSize());
	EXPECT_EQ(5, TraceCache::getMaxBranches());
	EXPECT_EQ(64, TraceCache::getQueueSize());
}

TEST(TestTraceCache, test_multiple_branch_predictor)
{

}


}
