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

namespace HSA
{

class Emu;

/// HSA WorkItem
class WorkItem{

 	// Emulator that is belongs to 
 	Emu *emu;

 	// Process ID
 	int pid;





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
 	// expanded into a funtion prototype.
#define DEFINST(name, opstr) \
 		void ExecuteInst_##name();
#include <arch/hsa/asm/Inst.def>
#undef DEFINST

 	// unsupported inst opcode
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

 	/// Create a work item from a command line. To safely create a  
 	/// function Emu::NewContext should be used instead. After the creation 
 	/// of a work item, its basic data structures are initialized with 
 	/// Load(), Clone(), or Fork()
 	WorkItem();

 	/// Destructor
 	~WorkItem();

 	/// Returns the pid of this work item
 	int getPid() const{return pid;}

 	/// Run one instruction for the workitem at the position pointed 
 	void Execute();

};

}  // namespace HSA

 #endif

