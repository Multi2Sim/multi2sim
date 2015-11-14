/*
 *  Multi2Sim
 *  Copyright (C) 2015  Spencer Hance (hance.s@husky.neu.edu)
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

#include <lib/cpp/IniFile.h>
#include <arch/x86/emulator/Emulator.h>
#include <arch/x86/emulator/Uinst.h>
#include <arch/x86/timing/TraceCache.h>
#include <arch/x86/timing/Core.h>
#include <arch/x86/timing/Cpu.h>
#include <arch/x86/timing/Thread.h>
#include <arch/x86/timing/Timing.h>
#include <arch/x86/timing/Uop.h>
#include <arch/common/Arch.h>

#include "ObjectPool.h"

namespace x86
{

// Default ini file to reset all relevant values to their defaults
const std::string default_config =
		"[ Queues ]\n"
		"RfKind = Private\n"
		"RfIntSize = 80\n"
		"RfFpSize = 40\n"
		"RfXmmSize = 40";



//
// canRename() tests
//


// Tests that canRename() returns false when there are no more integer
// registers available in a private register
TEST(TestRegisterFile, can_rename_private_register_0)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical integer registers
	std::string config =
			"[ Queues ]\n"
			"RfIntSize = 26";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 1);
	uinst_0->setIDep(1, 2);
	uinst_0->setIDep(2, 3);
	uinst_0->setODep(0, 4);
	uinst_0->setODep(1, 5);
	uinst_0->setODep(2, 6);
	uinst_0->setODep(3, 7);
	uinst_1->setIDep(0, 1);
	uinst_1->setIDep(1, 2);
	uinst_1->setIDep(2, 3);
	uinst_1->setODep(0, 4);
	uinst_1->setODep(1, 5);
	uinst_1->setODep(2, 6);
	uinst_1->setODep(3, 7);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
					 object_pool->getContext(),
					 uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
					object_pool->getContext(),
					uinst_1);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Check that canRename() returns true
	EXPECT_TRUE(register_file->canRename(uop_0.get()));

	// Occupy integer registers
	register_file->Rename(uop_0.get());
	register_file->Rename(uop_1.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_0.get()));
}

// Tests that canRename() returns false when there are no more floating-point
// registers available in a private register
TEST(TestRegisterFile, can_rename_private_register_1)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical floating point registers
	std::string config =
			"[ Queues ]\n"
			"RfFpSize = 15";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 23);
	uinst_0->setIDep(1, 24);
	uinst_0->setIDep(2, 25);
	uinst_0->setODep(0, 26);
	uinst_0->setODep(1, 27);
	uinst_0->setODep(2, 28);
	uinst_0->setODep(3, 29);
	uinst_1->setIDep(0, 23);
	uinst_1->setIDep(1, 24);
	uinst_1->setIDep(2, 25);
	uinst_1->setODep(0, 26);
	uinst_1->setODep(1, 27);
	uinst_1->setODep(2, 28);
	uinst_1->setODep(3, 29);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
					 object_pool->getContext(),
					 uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
					object_pool->getContext(),
					uinst_1);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Check that canRename() returns true
	EXPECT_TRUE(register_file->canRename(uop_1.get()));

	// Occupy floating point registers
	register_file->Rename(uop_0.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_1.get()));
}

// Tests that canRename() returns false when there are no more XMM registers
// available in a private register
TEST(TestRegisterFile, can_rename_private_register_2)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical Xmm registers
	std::string config =
			"[ Queues ]\n"
			"RfXmmSize = 15";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 34);
	uinst_0->setIDep(1, 35);
	uinst_0->setIDep(2, 36);
	uinst_0->setODep(0, 37);
	uinst_0->setODep(1, 38);
	uinst_0->setODep(2, 39);
	uinst_0->setODep(3, 40);
	uinst_1->setIDep(0, 41);
	uinst_1->setIDep(1, 34);
	uinst_1->setIDep(2, 35);
	uinst_1->setODep(0, 36);
	uinst_1->setODep(1, 37);
	uinst_1->setODep(2, 38);
	uinst_1->setODep(3, 39);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
					 object_pool->getContext(),
					 uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
					object_pool->getContext(),
					uinst_1);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Check that canRename() returns true
	EXPECT_TRUE(register_file->canRename(uop_1.get()));

	// Occupy Xmm registers
	register_file->Rename(uop_0.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_1.get()));
}

// Tests that canRename() returns false when there are no more integer
// registers available in a shared register
TEST(TestRegisterFile, can_rename_shared_register_0)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical integer registers
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared\n"
			"RfIntSize = 26";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 1);
	uinst_0->setIDep(1, 2);
	uinst_0->setIDep(2, 3);
	uinst_0->setODep(0, 4);
	uinst_0->setODep(1, 5);
	uinst_0->setODep(2, 6);
	uinst_0->setODep(3, 7);
	uinst_1->setIDep(0, 1);
	uinst_1->setIDep(1, 2);
	uinst_1->setIDep(2, 3);
	uinst_1->setODep(0, 4);
	uinst_1->setODep(1, 5);
	uinst_1->setODep(2, 6);
	uinst_1->setODep(3, 7);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
					 object_pool->getContext(),
					 uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
					object_pool->getContext(),
					uinst_1);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Check that canRename() returns true
	EXPECT_TRUE(register_file->canRename(uop_0.get()));

	// Occupy integer registers
	register_file->Rename(uop_0.get());
	register_file->Rename(uop_1.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_0.get()));
}

// Tests that canRename() returns false when there are no more floating-point
// registers available in a shared register
TEST(TestRegisterFile, can_rename_shared_register_1)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical floating point registers
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared\n"
			"RfFpSize = 15";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 23);
	uinst_0->setIDep(1, 24);
	uinst_0->setIDep(2, 25);
	uinst_0->setODep(0, 26);
	uinst_0->setODep(1, 27);
	uinst_0->setODep(2, 28);
	uinst_0->setODep(3, 29);
	uinst_1->setIDep(0, 23);
	uinst_1->setIDep(1, 24);
	uinst_1->setIDep(2, 25);
	uinst_1->setODep(0, 26);
	uinst_1->setODep(1, 27);
	uinst_1->setODep(2, 28);
	uinst_1->setODep(3, 29);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
					 object_pool->getContext(),
					 uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
					object_pool->getContext(),
					uinst_1);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Check that canRename() returns true
	EXPECT_TRUE(register_file->canRename(uop_1.get()));

	// Occupy floating point registers
	register_file->Rename(uop_0.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_1.get()));
}

// Tests that canRename() returns false when there are no more XMM registers
// available in a private register
TEST(TestRegisterFile, can_rename_shared_register_2)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical Xmm registers
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared\n"
			"RfXmmSize = 15";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 34);
	uinst_0->setIDep(1, 35);
	uinst_0->setIDep(2, 36);
	uinst_0->setODep(0, 37);
	uinst_0->setODep(1, 38);
	uinst_0->setODep(2, 39);
	uinst_0->setODep(3, 40);
	uinst_1->setIDep(0, 41);
	uinst_1->setIDep(1, 34);
	uinst_1->setIDep(2, 35);
	uinst_1->setODep(0, 36);
	uinst_1->setODep(1, 37);
	uinst_1->setODep(2, 38);
	uinst_1->setODep(3, 39);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
					 object_pool->getContext(),
					 uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
					object_pool->getContext(),
					uinst_1);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Check that canRename() returns true
	EXPECT_TRUE(register_file->canRename(uop_1.get()));

	// Occupy Xmm registers
	register_file->Rename(uop_0.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_1.get()));
}




//
// Rename() tests
//


// Checks correct logical-to-physical register renaming in a uop with 1 integer
// input and 1 integer output.
TEST(TestRegisterFile, rename_0)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical integer registers
	std::string config =
			"[ Queues ]\n"
			"RfIntSize = 26";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst with integer register dependency
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Manually set uinst dependencies
	uinst_0->setIDep(0, 1);
	uinst_0->setODep(0, 2);

	// Create uop with one uinst
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Check values
	EXPECT_EQ(uop_0->getInput(0), 25);
	EXPECT_EQ(uop_0->getOutput(0), 7);
}


// Check correct logical-to-physical register renaming in a uop with with 1
// floating point input + 1 floating point output.  Verifies that the proper
// floating point stack register is selected based on the current
// floating-point stack top.
TEST(TestRegisterFile, rename_1)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical integer registers
	std::string config =
			"[ Queues ]\n"
			"RfFpSize = 15";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst with floating point register dependency
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);

	// Manually set uinst dependencies
	uinst_0->setIDep(0, 23);
	uinst_0->setODep(0, 24);

	// Create uop with one uinst
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Check values
	EXPECT_EQ(uop_0->getInput(0), 14);
	EXPECT_EQ(uop_0->getOutput(0), 3);
}

// Check correct logical-to-physical register renaming in a uop
// with 1 XMM input + 1 XMM output.
TEST(TestRegisterFile, rename_2)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical integer registers
	std::string config =
			"[ Queues ]\n"
			"RfXmmSize = 15";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst with Xmm sregister dependency
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);

	// Manually set uinst dependencies
	uinst_0->setIDep(0, 34);
	uinst_0->setODep(0, 35);

	// Create uop with one uinst
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Check values
	EXPECT_EQ(uop_0->getInput(0), 14);
	EXPECT_EQ(uop_0->getOutput(0), 5);
}

// Test renaming uop with flag dependencies, requiring additional
// integer registers.
TEST(TestRegisterFile, rename_3)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Config file
	// Sets number of physical integer registers
	std::string config =
			"[ Queues ]\n"
			"RfIntSize = 26";

	// Create ini file with config
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set ini file in Timing
	Timing::ParseConfiguration(&ini_file);

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst with integer register dependency
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Manually set uinst dependencies
	uinst_0->setIDep(0, 1);
	uinst_0->setODep(0, 2);

	// Set flag dependency "Zps"
	uinst_0->setODep(1, 15);

	// Create uop with one uinst
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	// Set up register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Check values
	EXPECT_EQ(uop_0->getInput(0), 25);
	EXPECT_EQ(uop_0->getOutput(1), 7);
	EXPECT_EQ(uop_0->getOutput(0), 7);
}




//
// isUopReady() tests
//


// Tests isUopReady() uop with 1 integer, 1 FP, and 1 XMM input.
// Test function returning false due to missing integer input,
// missing FP input, and missing XMM input.
TEST(TestRegisterFile, is_uop_ready_0)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 1);
	uinst_0->setIDep(1, 23);
	uinst_0->setIDep(2, 34);
	uinst_1->setODep(0, 1);
	uinst_1->setODep(1, 23);
	uinst_1->setODep(2, 34);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop_1 to change physical register values to pending
	register_file->Rename(uop_1.get());

	// Rename uop_0 to map logical registers
	register_file->Rename(uop_0.get());

	// Check that the input deps for uop_0 are pending
	EXPECT_FALSE(register_file->isUopReady(uop_0.get()));
}

// Test isUopReady() returning true with all of them ready.
TEST(TestRegisterFile, is_uop_ready_1)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	uinst_0->setIDep(0, 1);
	uinst_0->setIDep(1, 23);
	uinst_0->setIDep(2, 34);

	// Create uop
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Map logical registers for uop_0
	register_file->Rename(uop_0.get());

	// This should return true since the deps are not pending
	EXPECT_TRUE(register_file->isUopReady(uop_0.get()));
}

// Test function returning true in the fast path (first "if")
// after it returned true before and recorded the ready
// state in field "->ready".
TEST(TestRegisterFile, is_uop_ready_2)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 1);
	uinst_0->setIDep(1, 23);
	uinst_0->setIDep(2, 34);

	// Create uop
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	// This should be false by default.  We just created the uop.
	EXPECT_FALSE(uop_0->ready);

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Map the logical register values
	register_file->Rename(uop_0.get());

	// This should return true since the deps are not pending.  This will
	// also set the 'ready' value to true
	EXPECT_TRUE(register_file->isUopReady(uop_0.get()));

	// The ready value should have been set by isUopReady() so that it
	// does not need to be evaluated later
	EXPECT_TRUE(uop_0->ready);
}




//
// WriteUop() tests
//


// Tests WriteUop() with two uops that depend on each other.
// The isUopReady() of the second should return false first.
// After the first one invoked WriteUop(), isUopReady() for the
// second should return true.
TEST(TestRegisterFile, write_uop_0)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinsts
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Set uinst dependencies
	uinst_0->setIDep(0, 1);
	uinst_0->setIDep(1, 23);
	uinst_0->setIDep(2, 34);
	uinst_1->setODep(0, 1);
	uinst_1->setODep(1, 23);
	uinst_1->setODep(2, 34);

	// Create uops
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);
	auto uop_1 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop_1 to change physical register values to pending
	register_file->Rename(uop_1.get());

	// Rename uop_0 to map logical registers
	register_file->Rename(uop_0.get());

	// Check that the input deps for uop_0 are pending
	EXPECT_FALSE(register_file->isUopReady(uop_0.get()));

	// Update state of register file and resolve deps
	register_file->WriteUop(uop_1.get());

	// This should now return true since the deps are no longer pending
	EXPECT_TRUE(register_file->isUopReady(uop_0.get()));
}




//
// UndoUop() Tests
//


// Test UndoUop(): sets an initial state of the register file,
// creates uop with 1 integer + 1 FP + 1 XMM output, invokes RenameUop()
// for the uop, checks valid new state, then UndoUop(), then checks
// that the state is back to the original. Verifies state of physical
// registers
TEST(TestRegisterFile, undo_uop_0)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Set dependencies
	uinst_0->setODep(0, 1);
	uinst_0->setODep(1, 23);
	uinst_0->setODep(2, 34);

	// Create uop
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	uop_0->speculative_mode = true;

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Store physical registers in temporary variables
	int new_physical_register_0 = uop_0->getOutput(0);
	int new_physical_register_1 = uop_0->getOutput(1);
	int new_physical_register_2 = uop_0->getOutput(2);

	// Check that the physical registers have been allocated
	EXPECT_FALSE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_FALSE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_FALSE(register_file->isXmmRegisterFree(new_physical_register_2));

	// Write uop to remove pending value from registers
	register_file->WriteUop(uop_0.get());

	// Undo uop
	register_file->UndoUop(uop_0.get());

	// Check that the physical registers have been deallocated
	EXPECT_TRUE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_TRUE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_TRUE(register_file->isXmmRegisterFree(new_physical_register_2));
}

// Special case for FpPop
TEST(TestRegisterFile, undo_uop_1)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeFpPop);

	// Set dependencies
	uinst_0->setODep(0, 1);
	uinst_0->setODep(1, 23);
	uinst_0->setODep(2, 34);

	// Create uop
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	uop_0->speculative_mode = true;

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Store physical registers in temporary variables
	int new_physical_register_0 = uop_0->getOutput(0);
	int new_physical_register_1 = uop_0->getOutput(1);
	int new_physical_register_2 = uop_0->getOutput(2);

	// Check that the physical registers have been allocated
	EXPECT_FALSE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_FALSE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_FALSE(register_file->isXmmRegisterFree(new_physical_register_2));

	// Write uop to remove pending value from registers
	register_file->WriteUop(uop_0.get());

	// Undo uop
	register_file->UndoUop(uop_0.get());

	// Check that the physical registers have been deallocated
	EXPECT_TRUE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_TRUE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_TRUE(register_file->isXmmRegisterFree(new_physical_register_2));
}

// Special case for FpPush
TEST(TestRegisterFile, undo_uop_2)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeFpPush);

	// Set dependencies
	uinst_0->setODep(0, 1);
	uinst_0->setODep(1, 23);
	uinst_0->setODep(2, 34);

	// Create uop
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	uop_0->speculative_mode = true;

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Store physical registers in temporary variables
	int new_physical_register_0 = uop_0->getOutput(0);
	int new_physical_register_1 = uop_0->getOutput(1);
	int new_physical_register_2 = uop_0->getOutput(2);

	// Check that the physical registers have been allocated
	EXPECT_FALSE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_FALSE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_FALSE(register_file->isXmmRegisterFree(new_physical_register_2));

	// Write uop to remove pending value from registers
	register_file->WriteUop(uop_0.get());

	// Undo uop
	register_file->UndoUop(uop_0.get());

	// Check that the physical registers have been deallocated
	EXPECT_TRUE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_TRUE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_TRUE(register_file->isXmmRegisterFree(new_physical_register_2));
}




//
// CommitUop() Tests
//


// Test CommitUop(): same structure as the UndoUop() test. Same uop,
// checks for original state, invokes RenameUop(), checks new state,
// invokes WriteUop(), checks state, invokes CommitUop().
TEST(TestRegisterFile, commit_uop_0)
{
	// Cleanup singleton instances
	ObjectPool::Destroy();

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uinst
	auto uinst_0 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);

	// Set dependencies
	uinst_0->setODep(0, 1);
	uinst_0->setODep(1, 23);
	uinst_0->setODep(2, 34);

	// Create uop
	auto uop_0 = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_0);

	// Get register file
	auto register_file = object_pool->getThread()->getRegisterFile();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Store physical registers in temporary variables
	int new_physical_register_0 = uop_0->getOutput(0);
	int new_physical_register_1 = uop_0->getOutput(1);
	int new_physical_register_2 = uop_0->getOutput(2);

	// Check that the physical registers have been allocated
	EXPECT_FALSE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_FALSE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_FALSE(register_file->isXmmRegisterFree(new_physical_register_2));

	// Write uop to remove pending value from registers
	register_file->WriteUop(uop_0.get());

	// Undo uop
	register_file->CommitUop(uop_0.get());

	// Check that the physical registers are still allocated
	EXPECT_FALSE(register_file->isIntegerRegisterFree(new_physical_register_0));
	EXPECT_FALSE(register_file->isFloatingPointRegisterFree(new_physical_register_1));
	EXPECT_FALSE(register_file->isXmmRegisterFree(new_physical_register_2));
}


} // Namespace x86







































































