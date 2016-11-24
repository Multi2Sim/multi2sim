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

#ifndef ARCH_HSA_EMULATOR_WORKITEM_H
#define ARCH_HSA_EMULATOR_WORKITEM_H

#include <memory>

#include <arch/hsa/disassembler/BrigCodeEntry.h>
#include <arch/hsa/disassembler/BrigDataEntry.h>
#include <arch/hsa/disassembler/BrigOperandEntry.h>
#include <arch/hsa/disassembler/BrigImmed.h>

#include "Emulator.h"
#include "Function.h"
#include "StackFrame.h"
#include "WorkGroup.h"
#include "HsaInstructionWorker.h"

namespace HSA
{

class Emulator;
class ProgramLoader;
class HsaExecutable;

/// List of HSA opcode
/*
enum InstOpcode
{
#define DEFINST(name, opcode, opstr) \
	INST_##name,
#include <arch/hsa/disassembler/Instruction.def>
#undef DEFINST
	// Max
	InstOpcodeCount
};
*/

/// HSA work item
class WorkItem
{
public: 
	
	/// Status of the work item
	enum WorkItemStatus
	{
		WorkItemStatusActive = 0,
		WorkItemStatusSuspend
	};

private:

 	// Emulator that is belongs to 
 	Emulator *emulator = nullptr;

 	// Executable
 	HsaExecutable *executable = nullptr;

 	// Work group that current work item belongs to
 	WorkGroup *work_group = nullptr;

 	// The private segment memory manager
 	std::unique_ptr<SegmentManager> private_segment;

	// The status of the work item;
	WorkItemStatus status = WorkItemStatusActive;

 	// work item absolute ids, equivalent to global_id in OpenCL
 	unsigned int abs_id_x;
 	unsigned int abs_id_y;
 	unsigned int abs_id_z;




 	//
 	// Functions related with the insts of HSA assembly, implemented in
 	// WorkItemIsa.cc
	//

 	// Stack of current work item.
 	std::vector<std::unique_ptr<StackFrame>> stack;

 	// Process directives befor an instruction
 	void ExecuteDirective();

	// Get HSA instruction worker according to the instruction
	std::unique_ptr<HsaInstructionWorker> getInstructionWorker(
			BrigCodeEntry *instruction);




 	//
 	// Memory related fields and function
 	//

	// Declare variable in global segment
	void DeclareVariableGlobal(const std::string &name, BrigType type, 
			unsigned long long dim);

	// Declare variable in group segment
	void DeclareVariableGroup(const std::string &name, BrigType type,
			unsigned long long dim);

	// Declare variable in private segment
	void DeclareVariablePrivate(const std::string &name, BrigType type,
			unsigned long long dim);

	// Declare variable in argument segment
	void DeclareVariableArgument(const std::string &name, BrigType type,
			unsigned long long dim);

 	// Allocate memory for variable
 	void DeclareVariable();

public:

 	/// Create a work item. HSA should let grid object to create work item
 	WorkItem();

 	/// Initialize the work item
 	void Initialize(WorkGroup *work_group,
 			unsigned private_segment_size,
 			unsigned int abs_id_x,
 			unsigned int abs_id_y,
 			unsigned int abs_id_z,
 			Function *root_function);

 	/// Destructor
 	virtual ~WorkItem();

 	/// Run one instruction for the workitem at the position pointed 
 	bool Execute();

 	/// Move the program counter by one. Return false if current PC is
 	/// at the end of the function
 	virtual bool MovePcForwardByOne();

 	/// Dump backtrace information
 	void Backtrace(std::ostream &os) const;

	/// Return the stack top stack frame
	StackFrame* getStackTop() const
	{
		//StackFrame *stack_top = stack.back().get();
		return stack.back().get();
	}

 	/// Push a stack frame into the stack
 	void PushStackFrame(std::unique_ptr<StackFrame> stack_frame)
 	{
 		this->stack.push_back(std::move(stack_frame));
 	}

	/// Pop the stackframe at the stack top
	void PopStack()
	{
		stack.pop_back();
	}

 	/// Finish the execution of a function by poping a frame from the stack
 	///
 	/// \return
 	///	Return false if the stack is empty after poping
 	bool ReturnFunction();




 	//
 	// Work item id read only getters
 	//

 	/// Return local ids
 	unsigned int getLocalIdX() const;
 	unsigned int getLocalIdY() const;
 	unsigned int getLocalIdZ() const;

 	/// Return flattened id
 	unsigned int getFlattenedId() const;

 	/// Return absolute flattened id
 	unsigned int getAbsoluteFlattenedId() const;

	/// Return abs id
	virtual unsigned int getAbsoluteIdX() const { return abs_id_x; }
	virtual unsigned int getAbsoluteIdY() const { return abs_id_y; }
	virtual unsigned int getAbsoluteIdZ() const { return abs_id_z; }

	/// Return the status of the work item
	WorkItemStatus getStatue() const { return status; }

	/// Set the status of the work item
	void setStatus(WorkItemStatus status) 
	{
		this->status = status;
	}

	/// Return the work group that this workitem is in
	WorkGroup *getWorkGroup() const
	{
		return work_group;
	}

	/// Return the grid that this work item belongs to
	Grid *getGrid() const;

	/// Translate inner address to flat address
	unsigned getFlatAddress(BrigSegment segment, unsigned address);

};

}  // namespace HSA

 #endif

