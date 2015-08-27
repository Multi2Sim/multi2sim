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

#include "ObjectPool.h"

namespace x86
{


//
// canRename() tests
//


// Tests that canRename() returns false when there are no more integer
// registers available in a private register
TEST(TestRegisterFile, can_rename_private_register_0)
{
	// Config file
	// Sets number of physical integer registers to 26
	std::string config =
			"[ Queues ]\n"
			"RfIntSize = 26";

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	RegisterFile::ParseConfiguration(&ini_file);

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
	// Config file
	// Sets number of physical integer registers to 26
	std::string config =
			"[ Queues ]\n"
			"RfFpSize = 15";


	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	RegisterFile::ParseConfiguration(&ini_file);

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

	// Occupy integer registers
	register_file->Rename(uop_0.get());
	register_file->Rename(uop_1.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_0.get()));
}

// Tests that canRename() returns false when there are no more XMM registers
// available in a private register
TEST(TestRegisterFile, can_rename_private_register_2)
{
	// Config file
	// Sets number of physical integer registers to 26
	std::string config =
			"[ Queues ]\n"
			"RfXmmSize = 15";

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	RegisterFile::ParseConfiguration(&ini_file);

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

	// Occupy integer registers
	register_file->Rename(uop_0.get());
	register_file->Rename(uop_1.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_0.get()));
}

// Tests that canRename() returns false when there are no more integer
// registers available in a shared register
TEST(TestRegisterFile, can_rename_shared_register_0)
{
	// Config file
	// Sets number of physical integer registers to 26
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared"
			"RfIntSize = 26";

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	RegisterFile::ParseConfiguration(&ini_file);

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
	// Config file
	// Sets number of physical integer registers to 26
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared"
			"RfFpSize = 15";

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	RegisterFile::ParseConfiguration(&ini_file);

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

	// Occupy integer registers
	register_file->Rename(uop_0.get());
	register_file->Rename(uop_1.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_0.get()));
}

// Tests that canRename() returns false when there are no more XMM registers
// available in a shared register
TEST(TestRegisterFile, can_rename_shared_register_2)
{
	// Config file
	// Sets number of physical integer registers to 26
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared"
			"RfXmmSize = 15";

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	RegisterFile::ParseConfiguration(&ini_file);

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

	// Occupy integer registers
	register_file->Rename(uop_0.get());
	register_file->Rename(uop_1.get());

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop_0.get()));
}




//
// Rename() tests
//


// Checks correct logical-to-physical register renaming in a uop with 1 integer
// input and 1 integer output.
TEST(TestRegisterFile, rename_0)
{
	// Config file
	// Sets number of physical integer registers to 26
	std::string config =
			"[ Queues ]\n"
			"RfKind = Private"
			"RfIntSize = 26";


	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	RegisterFile::ParseConfiguration(&ini_file);

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

	register_file->PrintNumIntegerRegisters();

	register_file->PrintNumIntegerRegisters();

	// Rename uop
	register_file->Rename(uop_0.get());

	// Check values
	EXPECT_EQ(uop_0->getInput(0), 25);
	EXPECT_EQ(uop_0->getOutput(0), 24);

}


// Check correct logical-to-physical register renaming in a uop with with 1
// floating point input + 1 floating point output.  Verifies that the proper
// floating point stack register is selected based on the current
// floating-point stack top.
TEST(TestRegisterFile, rename_1)
{
	/*
	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Set uinst dependencies
	uinst_1->setIDep(0, 23);
	uinst_1->setODep(0, 24);

	// Rename uop
	register_file->Rename(uop.get());

	// Check values
	EXPECT_EQ(uop->getInput(0), 39);
	EXPECT_EQ(uop->getOutput(0), 28);
	*/
}

// Check correct logical-to-physical register renaming in a uop
// with 1 XMM input + 1 XMM output.
TEST(TestRegisterFile, rename_2)
{
	/*
	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Set uinst dependencies
	uinst_1->setIDep(0, 34);
	uinst_1->setODep(0, 35);

	// Rename uop
	register_file->Rename(uop.get());

	// Check values
	EXPECT_EQ(uop->getInput(0), 39);
	EXPECT_EQ(uop->getOutput(0), 30);
	*/
}

// Test renaming uop with flag dependencies, requiring additional
// integer registers.
TEST(TestRegisterFile, rename_3)
{
	/*
	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Set uinst dependencies
	uinst_1->setIDep(0, 1);
	uinst_1->setODep(0, 2);

	// Set uinst flag dependencies
	uinst_1->setIDep(1, 15);
	uinst_1->setIDep(2, 16);

	// Rename uop
	register_file->Rename(uop.get());

	// Check values
	EXPECT_EQ(uop->getInput(0), 79);
	EXPECT_EQ(uop->getOutput(0), 61);
	EXPECT_EQ(uop->getInput(1), 66);
	EXPECT_EQ(uop->getInput(2), 66);
	*/
}




//
// isUopReady() tests
//


// Tests isUopReady() uop with 1 integer, 1 FP, and 1 XMM input.
// Test function returning false due to missing integer input,
// missing FP input, and missing XMM input.
TEST(TestRegisterFile, is_uop_ready_0)
{

}

// Test function returning true with all of them ready.
TEST(TestRegisterFile, is_uop_ready_1)
{

}

// Test function returning true in the fast path (first "if")
// after it returned true before and recorded the ready
// state in field "->ready".
TEST(TestRegisterFile, is_uop_ready_2)
{

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

}




//
// UndoUop() Tests
//


// Test UndoUop(): sets an initial state of the register file,
// creates uop with 1 integer + 1 FP + 1 XMM output, invokes RanameUop()
// for the uop, checks valid new state, then UndoUop(), then checks
// that the state is back to the original. Verifies state of physical
// registers, number of free registers, and free lists.
TEST(TestRegisterFile, undo_uop_0)
{

}

// Special case for FpPop
TEST(TestRegisterFile, undo_uop_1)
{

}

// Special case for FpPush
TEST(TestRegisterFile, undo_uop_2)
{

}




//
// CommitUop() Tests
//


// Test CommitUop(): same structure as the UndoUop() test. Same uop,
// checks for original state, invokes RenameUop(), checks new state,
// invokes WriteUop(), checks state, invokes CommitUop().
TEST(TestRegisterFile, commit_uop_0)
{

}


} // Namespace x86







































































