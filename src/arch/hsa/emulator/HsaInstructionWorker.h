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

namespace HSA
{
class WorkItem;
class StackFrame;
class BrigCodeEntry;

/// An HsaInstructionWorker is a unit that emulates an instruction
class HsaInstructionWorker
{
protected:
	// The work item that this instruction worker is working on
	WorkItem *work_item;

	// The stack fram that this instruction worker is working on
	StackFrame *stack_frame;

	// Get the value of the index-th operand, stores the result in
	// the \a buffer
	virtual void getOperandValue(unsigned int index, void *buffer);

	// Store the value into registers marked by the operand, from the
	// value pointer
	virtual void setOperandValue(unsigned int index, void *value);

public:
	/// Constructor
	HsaInstructionWorker(WorkItem *work_item, StackFrame *stack_frame);

	/// Destructor
	virtual ~HsaInstructionWorker() {};

	/// Execute the instruction
	virtual void Execute(BrigCodeEntry *instruction);

	// Set the stack frame for this instruction worker to work on
	virtual void setStackFrame(StackFrame *stack_frame)
	{
		this->stack_frame = stack_frame;
	}




	///
	/// Functions to execute each instruction
	///
	virtual void ExecuteWORKITEMABSID();
};

}

#endif  // ARCH_HSA_EMULATOR_HSAINSTRUCTIONWORKER_H
