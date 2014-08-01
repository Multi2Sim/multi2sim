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

#ifndef ARCH_HSA_EMU_WORKITEM_H
#define ARCH_HSA_EMU_WORKITEM_H

#include <memory>

#include <arch/hsa/asm/BrigInstEntry.h>
#include <arch/hsa/asm/BrigDirEntry.h>
#include <arch/hsa/asm/BrigStrEntry.h>

#include "Function.h"
#include "Emu.h"
#include "WorkGroup.h"
#include "StackFrame.h"


namespace HSA
{

class Emu;
class WorkGroup;
class StackFrame;

/// HSA work item
class WorkItem
{
 	// Emulator that is belongs to 
 	Emu *emu;

 	// Work group that current work item belongs to
 	WorkGroup *work_group;

 	// work item absolute ids, equivalent to global_id in OpenCL
 	unsigned int abs_id_x;
 	unsigned int abs_id_y;
 	unsigned int abs_id_z;

 	// kernel arguments storage
 	unsigned long long kernel_args;





 	//
 	// Functions related with the insts of HSA assembly, implemented in
 	// WorkItemIsa.cc

 	// Stack of current work item.
 	std::vector<std::unique_ptr<StackFrame>> stack;

 	// Prototype of member function of class WorkItem devoted to the 
 	// execution of HSA virtual ISA instructions.
 	typedef void (WorkItem::*ExecuteInstFn)();

 	// Instruction emulation functions. Each entry of Inst.def will be
 	// expanded into a function prototype.
#define DEFINST(name, opstr) \
 		void ExecuteInst_##name();
#include <arch/hsa/asm/Inst.def>
#undef DEFINST

 	// Unsupported opcode
 	void ExecuteInst_unsupported();

 	// Move the program counter by one. Return false if current PC is
 	// at the end of the function
 	bool MovePcForwardByOne();

 	// Get the value of the index-th operand
 	template <typename Type>
 	Type getOperandValue(unsigned int index);

 	// Store the value into registers marked by the operand
 	template <typename Type>
 	void storeOperandValue(unsigned int index, Type value);

 	// Table of functions that implement instructions
 	static ExecuteInstFn execute_inst_fn[InstOpcodeCount + 1];






 	//
 	// Functions and fields related to system call services
 	//

 	// Prototype of member function of class WorkItem devoted to the
	// execution of the Syscall services
	typedef void (WorkItem::*ExecuteSyscallFn)();

	// Enumeration with all syscall codes
	enum
	{
		SyscallCode_nop = 0,
#define DEFSYSCALL(name, code) SyscallCode_##name = code,
#include "Syscall.def"
#undef DEFSYSCALL
		SyscallCodeCount
	};

	// Syscall follows the SPIM simulator style. Syscall emulation
	// functions. Each entry of Syscall.def will be expanded into a
	// function prototype.
#define DEFSYSCALL(name, code) \
		void ExecuteSyscall_##name();
#include "Syscall.def"
#undef DEFSYSCALL

	// Table of functions that implement instructions
	static ExecuteSyscallFn execute_syscall_fn[SyscallCodeCount + 1];

 public:

 	/// Create a work item. HSA should let grid object to create work item
 	WorkItem(WorkGroup *work_group,
 			unsigned int abs_id_x,
 			unsigned int abs_id_y,
 			unsigned int abs_id_z,
 			Function *root_function,
 			unsigned long long kernel_args);

 	/// Destructor
 	~WorkItem();

 	/// Run one instruction for the workitem at the position pointed 
 	bool Execute();

 	/// Dump backtrace information
 	void Backtrace(std::ostream &os) const;

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

};

}  // namespace HSA

 #endif

