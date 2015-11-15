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


#ifndef ARCH_HSA_EMULATOR_HSAINSTRUCTIONWORKER_H
#define ARCH_HSA_EMULATOR_HSAINSTRUCTIONWORKER_H

#include <memory>

#include "OperandValueRetriever.h"
#include "OperandValueWriter.h"

namespace HSA
{
class WorkItem;
class StackFrame;
class BrigCodeEntry;
class OperandValueRetriever;
class OperandValueWriter;

/// An HsaInstructionWorker is a unit that emulates an instruction
class HsaInstructionWorker
{
protected:
	// The work item that this instruction worker is working on
	WorkItem *work_item;

	// The stack fram that this instruction worker is working on
	StackFrame *stack_frame;

	// Operand value retriever
	std::unique_ptr<OperandValueRetriever> operand_value_retriever;

	// Operand value writer
	std::unique_ptr<OperandValueWriter> operand_value_writer;

public:
	/// Constructor
	HsaInstructionWorker(WorkItem *work_item, StackFrame *stack_frame);

	/// Destructor
	virtual ~HsaInstructionWorker() {};

	/// Execute the instruction
	virtual void Execute(BrigCodeEntry *instruction) = 0;

	/// Set the operand value retriever
	void setOperandValueRetriever(OperandValueRetriever *retriever)
	{
		operand_value_retriever.reset(retriever);
	}

	/// Set the operand value writer
	void setOperandValueWriter(OperandValueWriter *writer)
	{
		operand_value_writer.reset(writer);
	}
};

}

#endif  // ARCH_HSA_EMULATOR_HSAINSTRUCTIONWORKER_H
