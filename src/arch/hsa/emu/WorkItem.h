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


namespace HSA
{

class Emu;
class WorkGroup;

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
 	//

 	// The program counter, pointing to the inst in .code section
 	char* pc;

 	// Prototype of member function of class WorkItem devoted to the 
 	// execution of HSA virtual ISA instructions.
 	typedef void (WorkItem::*ExecuteInstFn)();

 	// Instruction emulation functions. Each entry of of Inst.def will be 
 	// expanded into a function prototype.
#define DEFINST(name, opstr) \
 		void ExecuteInst_##name();
#include <arch/hsa/asm/Inst.def>
#undef DEFINST

 	// Unsupported opcode
 	void ExecuteInst_unsupported();

 	// Get the value of the index-th operand
 	template <typename Type>
 	Type getOperandValue(unsigned int index);

 	// Store the value into registers marked by the operand
 	template <typename Type>
 	void storeOperandValue(unsigned int index, Type value);

 	// Table of functions that implement instructions
 	static ExecuteInstFn execute_inst_fn[InstOpcodeCount + 1];

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

