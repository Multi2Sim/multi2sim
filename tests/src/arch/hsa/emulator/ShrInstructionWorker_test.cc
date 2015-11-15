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

#include <arch/hsa/emulator/ShrInstructionWorker.h>
#include <arch/hsa/emulator/WorkItem.h>

namespace HSA
{

namespace shr
{

class MockupOperandValueRetriever : public OperandValueRetriever
{
public:
	long long return_value[2] = {0};
	unsigned int size[2] = {0};
	unsigned int count = 0;

	MockupOperandValueRetriever() :
		OperandValueRetriever(nullptr, nullptr)
	{
	}

	void Retrieve(BrigCodeEntry *instruction, unsigned int index,
			void *buffer) override
	{
		memcpy(buffer, return_value + count, size[count]);
		count++;
	}
};

class MockupOperandValueWriter : public OperandValueWriter
{
public:
	long long expect_value = 0;
	unsigned int size = 0;

	MockupOperandValueWriter() :
		OperandValueWriter(nullptr, nullptr)
	{
	}

	void Write(BrigCodeEntry *instruction, unsigned int index,
			void *buffer) override
	{
		long long actual_value = 0;
		memcpy(&actual_value, buffer, size);
		EXPECT_EQ(expect_value, actual_value);
	}
};

class MockupWorkItem : public WorkItem
{
public:
	MockupWorkItem()
	{
	};

	unsigned int pc = 0;
	bool MovePcForwardByOne() override
	{
		pc++;
		return true;
	}
};

class MockupInstruction : public BrigCodeEntry
{
public:
	MockupInstruction() :
		BrigCodeEntry(nullptr)
	{
	}

	BrigType type = BRIG_TYPE_NONE;
	BrigType getType() const override { return type; }
};


TEST(ShrInstructionWorker, singed_result)
{
	// Setup environment
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupOperandValueWriter *writer =
			new MockupOperandValueWriter();
	MockupWorkItem *work_item = new MockupWorkItem();
	ShrInstructionWorker worker(work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);
	MockupInstruction *instruction = new MockupInstruction();

	// Execute
	instruction->type = BRIG_TYPE_S32;
	retriever->return_value[0] = 0xF000F00F;
	retriever->size[0] = 4;
	retriever->return_value[1] = 12;
	retriever->size[1] = 4;
	writer->expect_value = 0xFFFF000F;
	writer->size = 4;
	worker.Execute(instruction);

	// Assertion
	EXPECT_EQ(1, work_item->pc);
}


TEST(ShrInstructionWorker, unsinged_result)
{
	// Setup environment
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupOperandValueWriter *writer =
			new MockupOperandValueWriter();
	MockupWorkItem *work_item = new MockupWorkItem();
	ShrInstructionWorker worker(work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);
	MockupInstruction *instruction = new MockupInstruction();

	// Execute
	instruction->type = BRIG_TYPE_U32;
	retriever->return_value[0] = 0xF000F00F;
	retriever->size[0] = 4;
	retriever->return_value[1] = 12;
	retriever->size[1] = 4;
	writer->expect_value = 0x000F000F;
	writer->size = 4;
	worker.Execute(instruction);

	// Assertion
	EXPECT_EQ(1, work_item->pc);
}

}

}  // namespace HSA
