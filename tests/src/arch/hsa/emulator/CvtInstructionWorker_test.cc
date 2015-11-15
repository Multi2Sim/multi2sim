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

#include <cstring>
#include <arch/hsa/emulator/CvtInstructionWorker.h>
#include <arch/hsa/emulator/WorkItem.h>

namespace HSA
{
namespace cvt
{

class MockupOperandValueRetriever : public OperandValueRetriever
{
public:
	long long return_value = 0;
	unsigned int size = 0;

	MockupOperandValueRetriever() :
		OperandValueRetriever(nullptr, nullptr)
	{
	}

	void Retrieve(BrigCodeEntry *instruction, unsigned int index,
			void *buffer) override
	{
		memcpy(buffer, &return_value, size);
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
	BrigType source_type = BRIG_TYPE_NONE;
	BrigType getType() const override { return type; }
	BrigType getSourceType() const override { return source_type; }
};


TEST(CvtInstructionWorker, should_convert_u16_to_u32)
{
	MockupOperandValueWriter *writer = new MockupOperandValueWriter();
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupWorkItem work_item;
	CvtInstructionWorker worker(&work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);

	// Mockup instruction
	MockupInstruction instruction;
	instruction.type = BRIG_TYPE_U32;
	writer->size = 4;
	instruction.source_type = BRIG_TYPE_U16;
	retriever->size = 2;

	// Execute
	retriever->return_value = 16;
	writer->expect_value = 16;
	worker.Execute(&instruction);

	// Assertion
	EXPECT_EQ(1, work_item.pc);
}

TEST(CvtInstructionWorker, should_convert_u32_to_u64)
{
	MockupOperandValueWriter *writer = new MockupOperandValueWriter();
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupWorkItem work_item;
	CvtInstructionWorker worker(&work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);

	// Mockup instruction
	MockupInstruction instruction;
	instruction.type = BRIG_TYPE_U64;
	writer->size = 8;
	instruction.source_type = BRIG_TYPE_U32;
	retriever->size = 4;

	// Execute
	retriever->return_value = 16;
	writer->expect_value = 16;
	worker.Execute(&instruction);

	// Assertion
	EXPECT_EQ(1, work_item.pc);
}

TEST(CvtInstructionWorker, should_convert_u64_to_u32)
{
	MockupOperandValueWriter *writer = new MockupOperandValueWriter();
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupWorkItem work_item;
	CvtInstructionWorker worker(&work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);

	// Mockup instruction
	MockupInstruction instruction;
	instruction.type = BRIG_TYPE_U32;
	writer->size = 4;
	instruction.source_type = BRIG_TYPE_U64;
	retriever->size = 8;

	// Execute
	retriever->return_value = 0x5555555555555555;
	writer->expect_value = 0x55555555;
	worker.Execute(&instruction);

	// Assertion
	EXPECT_EQ(1, work_item.pc);
}

TEST(CvtInstructionWorker, should_convert_u64_to_f32)
{
	MockupOperandValueWriter *writer = new MockupOperandValueWriter();
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupWorkItem work_item;
	CvtInstructionWorker worker(&work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);

	// Mockup instruction
	MockupInstruction instruction;
	instruction.type = BRIG_TYPE_F32;
	writer->size = 4;
	instruction.source_type = BRIG_TYPE_U64;
	retriever->size = 8;

	// Execute
	retriever->return_value = 1024;
	float expect_value = 1024.0f;
	memcpy(&writer->expect_value, &expect_value, 4);
	worker.Execute(&instruction);

	// Assertion
	EXPECT_EQ(1, work_item.pc);
}

}  // namespace cvt

}



