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
#include <arch/hsa/asm/BrigOperandEntry.h>
#include <arch/hsa/asm/BrigImmed.h>

#include "Function.h"
#include "Emu.h"
#include "WorkGroup.h"
#include "StackFrame.h"
#include "ProgramLoader.h"


namespace HSA
{

class Emu;
class WorkGroup;
class ProgramLoader;

/// HSA work item
class WorkItem
{
 	// Emulator that is belongs to 
 	Emu *emu;

 	// Loader object
 	ProgramLoader *loader;

 	// Binary file
 	BrigFile *binary;

 	// Work group that current work item belongs to
 	WorkGroup *work_group;

 	// work item absolute ids, equivalent to global_id in OpenCL
 	unsigned int abs_id_x;
 	unsigned int abs_id_y;
 	unsigned int abs_id_z;




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

 	// Execute inst auxiliary functions
 	template<typename T> void Inst_ABS_Aux();
 	template<typename T> void Inst_ADD_Aux();
 	template<typename T> void Inst_BORROW_Aux();
 	template<typename T> void Inst_CARRY_Aux();
 	template<typename T> void Inst_DIV_Aux();
 	template<typename T> void Inst_MAD_Aux();
 	template<typename T> void Inst_MAX_Aux();
 	template<typename T> void Inst_MIN_Aux();
 	template<typename T> void Inst_MUL_Aux();
 	template<typename T> void Inst_MULHI_Aux(int half_width, T masks);
 	template<typename T> void Inst_NEG_Aux();
 	template<typename T> void Inst_SUB_Aux();
 	template<typename T> void Inst_REM_Aux();
 	template<typename T> void Inst_SHL_Aux();
 	template<typename T> void Inst_SHR_Aux();
 	template<typename T> void Inst_AND_Aux();
 	template<typename T> void Inst_NOT_Aux();
 	template<typename T> void Inst_OR_Aux();
 	template<typename T> void Inst_POPCOUNT_Aux();
 	template<typename T> void Inst_XOR_Aux();
 	template<typename T> void Inst_BITEXTRACT_Aux();
	template<typename T> void Inst_LDA_Aux();
	template<typename T> void Inst_LDC_Aux();
 	template<typename T> void Inst_MOV_Aux();
 	template<typename SrcType, typename DstType> void Inst_CMP_Aux();
 	template<typename T> void Inst_LD_Aux();
 	template<typename T> void Inst_ST_Aux();

 	// Get the value of the index-th operand
 	template <typename Type>
 	Type getOperandValue(unsigned int index)
 	{
 		// Get the operand entry
 		StackFrame *stack_top = stack.back().get();
 		BrigInstEntry inst(stack_top->getPc(), loader->getBinary());
 		BrigOperandEntry operand(inst.getOperand(index), inst.getFile(),
 				&inst, index);

 		// Do coresponding action according to the type of operand
 		switch (operand.getKind())
 		{
 		case BRIG_OPERAND_IMMED:

 		{
 			BrigImmed immed(operand.getImmedBytes(),
 					operand.getOperandType());
 			Type value = immed.getImmedValue<Type>();
 			return value;
 		}

 		case BRIG_OPERAND_WAVESIZE:

 			return 1;

 		case BRIG_OPERAND_REG:

 		{
 			std::string register_name = operand.getRegisterName();
 			return stack_top->getRegisterValue<Type>(register_name);
 		}

 		default:

 			throw misc::Panic("Unsupported operand type "
 					"for getOperandValue");
 			break;
 		}
 		return 0;
 	}

 	// Store the value into registers marked by the operand
 	template <typename Type>
 	void storeOperandValue(unsigned int index, Type value)
 	{
 		// Get the operand entry
 		StackFrame *stack_top = stack.back().get();
 		BrigInstEntry inst(stack_top->getPc(), loader->getBinary());
 		BrigOperandEntry operand(inst.getOperand(index), inst.getFile(),
 				&inst, index);

 		// Do corresponding action according to the type of operand
 		// I do not think there should be other type except reg
 		switch (operand.getKind())
 		{
 		case BRIG_OPERAND_REG:

 		{
 			std::string register_name = operand.getRegisterName();
 			stack_top->setRegisterValue<Type>(register_name, value);
 			break;
 		}

 		default:

 			throw misc::Panic("Unsupported operand type "
 					"for storeOperandValue");
 		}
 	}

 	// Table of functions that implement instructions
 	static ExecuteInstFn execute_inst_fn[InstOpcodeCount + 1];




 	//
 	// Memory related fields and function
 	//
 	// Return buffer in host memory
 	char *getVariableBuffer(unsigned char segment,
 			const std::string &name);

 	// Allocate memory for variable
 	void DeclearVariable();

 	// Process directives between last PC and current PC
 	void ProcessRelatedDirectives();




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
 			Function *root_function);

 	/// Destructor
 	~WorkItem();

 	/// Run one instruction for the workitem at the position pointed 
 	bool Execute();

 	/// Move the program counter by one. Return false if current PC is
 	/// at the end of the function
 	bool MovePcForwardByOne();

 	/// Dump backtrace information
 	void Backtrace(std::ostream &os) const;

	/// Return the stack top stack frame
	StackFrame* getStackTop()
	{
		StackFrame *stack_top = stack.back().get();
		stack_top->Dump(std::cout);
		return stack.back().get();
	}

 	/// Push a stack frame into the stack
 	void PushStackFrame(StackFrame *stack_frame)
 	{
 		this->stack.push_back(std::unique_ptr<StackFrame>(stack_frame));
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

};

}  // namespace HSA

 #endif

