/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>

#include "../BrigFile.h"
#include "../BrigDataEntry.h"
#include "../BrigCodeEntry.h"
#include "../BrigOperandEntry.h"

namespace HSA
{

TEST(TestBrigCodeEntry, should_dump_module_directive_correctly)
{
	// Mock up an BrigFile Class
	class MockupBrigFile : public BrigFile
	{
	public: 
		const std::string getStringByOffset(unsigned int offset) const
			override
		{
			return "&module";
		}
	};
	MockupBrigFile *file = new MockupBrigFile();

	// Setup directive buffer
	auto module = misc::new_unique<BrigDirectiveModule>();
	module->base.byteCount = 20;
	module->base.kind = BRIG_KIND_DIRECTIVE_MODULE;
	module->name = 0;
	module->hsailMajor = 12;
	module->hsailMinor = 34;
	module->profile = BRIG_PROFILE_BASE;
	module->machineModel = BRIG_MACHINE_SMALL;
	module->defaultFloatRound = BRIG_ROUND_FLOAT_DEFAULT;

	// Create a string stream
	std::ostringstream os;

	// Init object
	BrigCodeEntry dir((char *)module.get());
	dir.setBinary(file);

	// Dump
	dir.Dump(os);	

	// Result
	std::string output = os.str();
	EXPECT_STREQ("module &module:12:34:$base:$small:$default;\n",
			output.c_str());
}


TEST(TestBrigCodeEntry, should_dump_directive_variable_right)
{
	// Mockup an BrigFile class
	class MockupBrigFile : public BrigFile
	{
	public:
		const std::string getStringByOffset(unsigned int offset) const
			override
		{
			return "&variable";
		}
	};
	MockupBrigFile *file = new MockupBrigFile();

	// Set directive buffer
	auto dir = misc::new_unique<BrigDirectiveVariable>();
	dir->base.byteCount = 100;
	dir->base.kind = BRIG_KIND_DIRECTIVE_VARIABLE;
	dir->name = 0;
	dir->init = 0;
	dir->type = BRIG_TYPE_U32;
	dir->segment = BRIG_SEGMENT_GLOBAL;
	dir->align = BRIG_ALIGNMENT_NONE;
	dir->dim.lo = 0;
	dir->dim.hi = 0;
	dir->modifier.allBits = 3;
	dir->linkage = BRIG_LINKAGE_NONE;
	dir->allocation = BRIG_ALLOCATION_AUTOMATIC;

	// Init object
	BrigCodeEntry entry((char *)dir.get());
	entry.setBinary(file);
	std::ostringstream os;

	// Dump
	entry.Dump(os);

	// Result
	std::string output = os.str();
	EXPECT_STREQ("const global_u32 &variable;\n", output.c_str());
}


TEST(TestBrigCodeEntry, should_dump_variable_array_right)
{
	// Mockup an BrigFile class
	class MockupBrigFile : public BrigFile
	{
	public:
		const std::string getStringByOffset(unsigned int offset) const
			override
		{
			return "&variable";
		}
	};
	MockupBrigFile *file = new MockupBrigFile();

	// Set directive buffer
	auto dir = misc::new_unique<BrigDirectiveVariable>();
	dir->base.byteCount = 100;
	dir->base.kind = BRIG_KIND_DIRECTIVE_VARIABLE;
	dir->name = 0;
	dir->init = 0;
	dir->type = BRIG_TYPE_S32_ARRAY;
	dir->segment = BRIG_SEGMENT_PRIVATE;
	dir->align = BRIG_ALIGNMENT_NONE;
	dir->dim.lo = 10;
	dir->dim.hi = 0;
	dir->modifier.allBits = 1;
	dir->linkage = BRIG_LINKAGE_NONE;
	dir->allocation = BRIG_ALLOCATION_AUTOMATIC;

	// Init object
	BrigCodeEntry entry((char *)dir.get());
	entry.setBinary(file);
	std::ostringstream os;

	// Dump
	entry.Dump(os);

	// Result
	std::string output = os.str();
	EXPECT_STREQ("private_s32 &variable[10];\n", output.c_str());
}


TEST(TestBrigCodeEntry, should_dump_float_point_operation_right)
{
	// Mockup an BrigFile class
	class MockupBrigFile : public BrigFile
	{
	public:
		std::unique_ptr<BrigOperandEntry> getOperandByOffset(
				unsigned int offset) const 
		{
			printf("Get operand by offset %d\n", offset);
			auto operand = misc::new_unique<BrigOperandConstantBytes>();
			operand->base.byteCount = 0;
			operand->base.kind = BRIG_KIND_OPERAND_CONSTANT_BYTES;
			operand->type = BRIG_TYPE_F32;
			operand->bytes = 200;
			return misc::new_unique<BrigOperandEntry>(
					(char *)operand.get());
		}

		std::unique_ptr<BrigDataEntry> getDataEntryByOffset(
				unsigned offset) const
		{
			// Set operand list to have 0 operand
			auto data = misc::new_unique<BrigData>();
			data->byteCount = 0;
			return misc::new_unique<BrigDataEntry>(
					(char *)data.get());
		}
	};
	MockupBrigFile *file = new MockupBrigFile();


	// Set directive buffer
	auto inst = misc::new_unique<BrigInstMod>();
	inst->base.base.byteCount = 100;
	inst->base.base.kind = BRIG_KIND_INST_MOD;
	inst->base.opcode = 2;
	inst->base.type = BRIG_TYPE_F32;
	inst->base.operands = 100;
	inst->modifier.allBits = 0;
	inst->pack = BRIG_PACK_NONE;
	inst->round = BRIG_ROUND_FLOAT_ZERO;

	// Init object
	BrigCodeEntry entry((char *)inst.get());
	entry.setBinary(file);
	std::ostringstream os;

	// Dump
	entry.Dump(os);

	// Result
	std::string output = os.str();
	EXPECT_STREQ("add_zero_f32;\n", output.c_str());
}


TEST(TestBrigCodeEntry, should_dump_memfence_right)
{
	// Set buffer
	auto inst = misc::new_unique<BrigInstMemFence>();
	inst->base.base.byteCount = 100;
	inst->base.base.kind = BRIG_KIND_INST_MEM_FENCE;
	inst->base.opcode = BRIG_OPCODE_MEMFENCE;
	inst->base.type = BRIG_TYPE_NONE;
	inst->memoryOrder = BRIG_MEMORY_ORDER_SC_ACQUIRE_RELEASE;
	inst->globalSegmentMemoryScope = BRIG_MEMORY_SCOPE_AGENT;
	inst->groupSegmentMemoryScope = BRIG_MEMORY_SCOPE_AGENT;
	inst->imageSegmentMemoryScope = BRIG_MEMORY_SCOPE_NONE;

	// Init object
	BrigCodeEntry entry((char *)inst.get());

	// Dump
	std::ostringstream os;
	entry.Dump(os);

	// Result
	EXPECT_STREQ("memfence_scar_agent;\n", os.str().c_str());
}



}


