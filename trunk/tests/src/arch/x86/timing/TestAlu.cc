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
#include <arch/x86/timing/Alu.h>
#include <arch/x86/timing/Core.h>
#include <arch/x86/timing/Cpu.h>
#include <arch/x86/timing/Thread.h>
#include <arch/x86/timing/Timing.h>
#include <arch/x86/timing/Uop.h>

#include "ObjectPool.h"

namespace x86
{

TEST(TestAlu, read_ini_configuration_file_int)
{
	// Setup configuration file
	std::string config =
			"[ FunctionalUnits ]\n"
			"IntAdd.Count = 5\n"
			"IntAdd.OpLat = 18\n"
			"IntAdd.IssueLat = 16\n"
			"IntMult.Count = 6\n"
			"IntMult.OpLat = 19\n"
			"IntMult.IssueLat = 17\n"
			"IntDiv.Count = 7\n"
			"IntDiv.OpLat = 20\n"
			"IntDiv.IssueLat = 18\n"
			"EffAddr.Count = 8\n"
			"EffAddr.OpLat = 21\n"
			"EffAddr.IssueLat = 19\n"
			"Logic.Count = 9\n"
			"Logic.OpLat = 22\n"
			"Logic.IssueLat = 20";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Find target section
	Alu::ParseConfiguration(&ini_file);

	// Test coverage
	int alu_type_start = 1;
	int alu_type_end = 5;

	// Set up reference results
	int ref_configuration[5][3] =
	{
			{ 5, 18, 16 },  // IntAdd
			{ 6, 19, 17 },  // IntMult
			{ 7, 20, 18 },  // IntDiv
			{ 8, 21, 19 },  // Effaddr
			{ 9, 22, 20 }  // Logic
	};

	// Test starts
	for (int i = 0; i < alu_type_end - alu_type_start + 1; i++)
	{
		EXPECT_EQ(ref_configuration[i][0], Alu::getAluCount(i + alu_type_start));
		EXPECT_EQ(ref_configuration[i][1], Alu::getAluOperationLatency(i + alu_type_start));
		EXPECT_EQ(ref_configuration[i][2], Alu::getAluIssueLatency(i + alu_type_start));
	}
}


TEST(TestAlu, read_ini_configuration_file_fp)
{
	// Setup configuration file
	std::string config =
			"[ FunctionalUnits ]\n"
			"FloatSimple.Count = 3\n"
			"FloatSimple.OpLat = 10\n"
			"FloatSimple.IssueLat = 13\n"
			"FloatAdd.Count = 4\n"
			"FloatAdd.OpLat = 11\n"
			"FloatAdd.IssueLat = 14\n"
			"FloatCompare.Count = 5\n"
			"FloatCompare.OpLat = 12\n"
			"FloatCompare.IssueLat = 15\n"
			"FloatMult.Count = 6\n"
			"FloatMult.OpLat = 13\n"
			"FloatMult.IssueLat = 16\n"
			"FloatDiv.Count = 7\n"
			"FloatDiv.OpLat = 14\n"
			"FloatDiv.IssueLat = 17\n"
			"FloatComplex.Count = 8\n"
			"FloatComplex.OpLat = 15\n"
			"FloatComplex.IssueLat = 18";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Find target section
	Alu::ParseConfiguration(&ini_file);

	// Test coverage
	int alu_type_start = 6;
	int alu_type_end = 11;

	// Set up reference results
	int ref_configuration[6][3] =
	{
			{ 3, 10, 13 },  // FloatSimple
			{ 4, 11, 14 },  // FloatAdd
			{ 5, 12, 15 },  // FloatCompare
			{ 6, 13, 16 },  // FloatMult
			{ 7, 14, 17 },  // FloatDiv
			{ 8, 15, 18 },  // FloatComplex
	};

	// Test starts
	for (int i = 0; i < alu_type_end - alu_type_start + 1; i++)
	{
		EXPECT_EQ(ref_configuration[i][0], Alu::getAluCount(i + alu_type_start));
		EXPECT_EQ(ref_configuration[i][1], Alu::getAluOperationLatency(i + alu_type_start));
		EXPECT_EQ(ref_configuration[i][2], Alu::getAluIssueLatency(i + alu_type_start));
	}
}


TEST(TestAlu, read_ini_configuration_file_xmm)
{
	// Setup configuration file
	std::string config =
			"[ FunctionalUnits ]\n"
			"XmmIntAdd.Count = 7\n"
			"XmmIntAdd.OpLat = 5\n"
			"XmmIntAdd.IssueLat = 12\n"
			"XmmIntMult.Count = 8\n"
			"XmmIntMult.OpLat = 6\n"
			"XmmIntMult.IssueLat = 13\n"
			"XmmIntDiv.Count = 9\n"
			"XmmIntDiv.OpLat = 7\n"
			"XmmIntDiv.IssueLat = 14\n"
			"XmmLogic.Count = 10\n"
			"XmmLogic.OpLat = 8\n"
			"XmmLogic.IssueLat = 15\n"
			"XmmFloatAdd.Count = 11\n"
			"XmmFloatAdd.OpLat = 9\n"
			"XmmFloatAdd.IssueLat = 16\n"
			"XmmFloatCompare.Count = 12\n"
			"XmmFloatCompare.OpLat = 10\n"
			"XmmFloatCompare.IssueLat = 17\n"
			"XmmFloatMult.Count = 13\n"
			"XmmFloatMult.OpLat = 11\n"
			"XmmFloatMult.IssueLat = 18\n"
			"XmmFloatDiv.Count = 14\n"
			"XmmFloatDiv.OpLat = 12\n"
			"XmmFloatDiv.IssueLat = 19\n"
			"XmmFloatConv.Count = 15\n"
			"XmmFloatConv.OpLat = 13\n"
			"XmmFloatConv.IssueLat = 20\n"
			"XmmFloatComplex.Count = 16\n"
			"XmmFloatComplex.OpLat = 14\n"
			"XmmFloatComplex.IssueLat = 21";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Find target section
	Alu::ParseConfiguration(&ini_file);

	// Test coverage
	int alu_type_start = 12;
	int alu_type_end = 21;

	// Set up reference results
	int ref_configuration[10][3] =
	{
			{ 7, 5, 12 },  // XmmIntAdd
			{ 8, 6, 13 },  // XmmIntMult
			{ 9, 7, 14 },  // XmmIntDiv
			{ 10, 8, 15 },  // XmmLogic
			{ 11, 9, 16 },  // XmmFloatAdd
			{ 12, 10, 17 },  // XmmFloatCompare
			{ 13, 11, 18 },  // XmmFloatMult
			{ 14, 12, 19 },  // XmmFloatDiv
			{ 15, 13, 20 },  // XmmFloatConv
			{ 16, 14, 21 }  // XmmFloatComplex
	};

	// Test starts
	for (int i = 0; i < alu_type_end - alu_type_start + 1; i++)
	{
		EXPECT_EQ(ref_configuration[i][0], Alu::getAluCount(i + alu_type_start));
		EXPECT_EQ(ref_configuration[i][1], Alu::getAluOperationLatency(i + alu_type_start));
		EXPECT_EQ(ref_configuration[i][2], Alu::getAluIssueLatency(i + alu_type_start));
	}
}

TEST(TestAlu, reserve_functional_unit_int_add)
{
	// Setup the simulation engine
	esim::Engine *esim = esim::Engine::getInstance();

	// Setup the configuration
	std::string config =
			"[ FunctionalUnits ]\n"
			"IntAdd.Count = 2\n"
			"IntAdd.OpLat = 2\n"
			"IntAdd.IssueLat = 3";

	// Parse default configuration
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	Alu::ParseConfiguration(&ini_file);

	// Setup the timing simulator related object pool
	ObjectPool::Destroy();
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Setup the simulated running cycle
	const int running_cycle = 6;

	// Setup mock micro-op list
	std::vector<std::unique_ptr<Uop>> uops;

	// Create running_cycle number of Uop with type intAdd
	for (int i = 0; i < running_cycle; i++)
	{
		auto uinst = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
		uops.emplace_back(misc::new_unique<Uop>(
			object_pool->getThread(),
			object_pool->getContext(),
			uinst));
	}

	// Start simulation----->timing by cycle
	// Uop ID:    |0   |1   |2   |3   |4   |5   |
	// ---------------------------------------------------------------------
	// F.U. status
	// instance 0 |null|Uop0|Uop0|null|Uop3|Uop3|
	// instance 1 |null|null|Uop1|Uop1|null|Uop4|
	// reserve status: S/Successfull F/Fail
	//            |S   |S   |F   |S   |S   |F   |
	int ref_value[running_cycle] = {2,
			2,
			0,
			2,
			2,
			0};
	for (int i = 0; i < running_cycle; i++)
	{
		// Reserve
		Alu *alu = object_pool->getCore()->getAlu();
		int ret_value = alu->Reserve(uops[i].get());

		std::cout << ref_value[i] << ' ' << ret_value << '\n';

		// Check the value
		EXPECT_EQ(ref_value[i], ret_value);

		// Advance one cycle of simulated system time
		esim->ProcessEvents();
	}

}

}
