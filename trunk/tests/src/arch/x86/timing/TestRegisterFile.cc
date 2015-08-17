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
	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Set output dependency to eax
	uop->setOutput(0, 1);

	// Occupy all integer registers
	for (int i = 0; i < 63; i++)
		register_file->incNumOccupiedIntegerRegisters();

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop.get()));
}

// Tests that canRename() returns false when there are no more floating-point
// registers available in a private register
TEST(TestRegisterFile, can_rename_private_register_1)
{
	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Set output dependency to St0
	uop->setOutput(0, 23);

	// Occupy all integer registers
	for (int i = 0; i < 63; i++)
		register_file->incNumOccupiedFloatingPointRegisters();

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop.get()));
}

// Tests that canRename() returns false when there are no more XMM registers
// available in a private register
TEST(TestRegisterFile, can_rename_private_register_2)
{
	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Set output dependency to Xmm0
	uop->setOutput(0, 34);

	// Occupy all integer registers
	for (int i = 0; i < 63; i++)
		register_file->incNumOccupiedXmmRegisters();

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop.get()));
}

// Tests that canRename() returns false when there are no more integer
// registers available in a shared register
TEST(TestRegisterFile, can_rename_shared_register_0)
{
	// Config file
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared\n";

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
					object_pool->getContext(),
					uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	register_file->ParseConfiguration(&ini_file);

	// Set output dependency to eax
	uop->setOutput(0, 1);

	// Occupy all integer registers
	for (int i = 0; i < 63; i++)
		register_file->incNumOccupiedIntegerRegisters();

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop.get()));
}

// Tests that canRename() returns false when there are no more floating-point
// registers available in a shared register
TEST(TestRegisterFile, can_rename_shared_register_1)
{
	// Config file
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared\n";

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeFpAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	register_file->ParseConfiguration(&ini_file);

	// Set output dependency to St0
	uop->setOutput(0, 23);

	// Occupy all integer registers
	for (int i = 0; i < 63; i++)
		register_file->incNumOccupiedFloatingPointRegisters();

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop.get()));
}

// Tests that canRename() returns false when there are no more XMM registers
// available in a public register
TEST(TestRegisterFile, can_rename_shared_register_2)
{
	// Config file
	std::string config =
			"[ Queues ]\n"
			"RfKind = Shared\n";

	// Get object pool instance
	ObjectPool *object_pool = ObjectPool::getInstance();

	// Create uop with integer register dependency
	auto uinst_1 = misc::new_shared<Uinst>(Uinst::OpcodeXmmAdd);
	auto uop = misc::new_unique<Uop>(object_pool->getThread(),
			object_pool->getContext(),
			uinst_1);

	// Set up register file
	auto register_file = misc::new_unique<RegisterFile>(object_pool->getThread());

	// Parse register config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);
	register_file->ParseConfiguration(&ini_file);

	// Set output dependency to Xmm0
	uop->setOutput(0, 34);

	// Occupy all integer registers
	for (int i = 0; i < 63; i++)
		register_file->incNumOccupiedXmmRegisters();

	// Check that canRename() returns false
	EXPECT_FALSE(register_file->canRename(uop.get()));
}




//
// Rename() tests
//


// Check correct logical-to-physical register renaming in a uop with 1 integer
// input + 1 integer output.
TEST(TestRegisterFile, rename_0)
{

}

// Check correct logical-to-physical register renaming in a uop with with 1
// floating point input + 1 floating point output.  Verifies that the proper
// FP stack register is selected based on the current floating-point stack top.
TEST(TestRegisterFile, rename_1)
{

}

// Check correct logical-to-physical register renaming in a uop
// with 1 XMM input + 1 XMM output.
TEST(TestRegisterFile, rename_2)
{

}

// Test renaming uop with flag dependencies, requiring additional
// integer registers.
TEST(TestRegisterFile, rename_3)
{

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







































































