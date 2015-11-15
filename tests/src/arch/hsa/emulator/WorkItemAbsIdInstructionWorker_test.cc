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

#include <arch/hsa/emulator/WorkItemAbsIdInstructionWorker.h>
#include <arch/hsa/emulator/WorkItem.h>

namespace HSA
{
namespace workitemabsid
{

class MockupOperandValueRetriever : public OperandValueRetriever
{
public:
	unsigned int return_value = 0;

	MockupOperandValueRetriever() :
		OperandValueRetriever(nullptr, nullptr)
	{
	}

	void Retrieve(BrigCodeEntry *instruction, unsigned int index,
			void *buffer) override
	{
		*(unsigned int *)buffer = return_value;
	}
};

class MockupOperandValueWriter : public OperandValueWriter
{
public:
	unsigned int expect_value = 100;

	MockupOperandValueWriter() :
		OperandValueWriter(nullptr, nullptr)
	{
	}

	void Write(BrigCodeEntry *instruction, unsigned int index,
			void *buffer) override
	{
		EXPECT_EQ(expect_value, *(unsigned int *)buffer);
	}
};

class MockupWorkItem : public WorkItem
{
public:
	MockupWorkItem()
	{
	};

	unsigned int pc = 0;
	unsigned int getAbsoluteIdX() const override { return 100; }
	unsigned int getAbsoluteIdY() const override { return 200; }
	unsigned int getAbsoluteIdZ() const override { return 300; }
	bool MovePcForwardByOne() override
	{
		pc++;
		return true;
	}
};


TEST(WorkItemAbsIdInstructionWorker, should_get_correct_result_x)
{
	// Setup
	MockupWorkItem work_item;
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupOperandValueWriter *writer =
			new MockupOperandValueWriter();
	WorkItemAbsIdInstructionWorker worker(&work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);
	retriever->return_value = 0;
	writer->expect_value = 100;

	// Execute
	worker.Execute(nullptr);

	// Assert
	EXPECT_EQ(1, work_item.pc);
}

TEST(WorkItemAbsIdInstructionWorker, should_get_correct_result_y)
{
	// Setup
	MockupWorkItem work_item;
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupOperandValueWriter *writer =
			new MockupOperandValueWriter();
	WorkItemAbsIdInstructionWorker worker(&work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);
	retriever->return_value = 1;
	writer->expect_value = 200;

	// Execute
	worker.Execute(nullptr);

	// Assert
	EXPECT_EQ(1, work_item.pc);
}

TEST(WorkItemAbsIdInstructionWorker, should_get_correct_result_z)
{
	// Setup
	MockupWorkItem work_item;
	MockupOperandValueRetriever *retriever =
			new MockupOperandValueRetriever();
	MockupOperandValueWriter *writer =
			new MockupOperandValueWriter();
	WorkItemAbsIdInstructionWorker worker(&work_item, nullptr);
	worker.setOperandValueRetriever(retriever);
	worker.setOperandValueWriter(writer);
	retriever->return_value = 2;
	writer->expect_value = 300;

	// Execute
	worker.Execute(nullptr);

	// Assert
	EXPECT_EQ(1, work_item.pc);
}
}
}






