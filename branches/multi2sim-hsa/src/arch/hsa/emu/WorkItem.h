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

#include <arch/hsa/asm/BrigCodeEntry.h>
#include <arch/hsa/asm/BrigDataEntry.h>
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

/// List of HSA opcode
enum InstOpcode
{
#define DEFINST(name, opcode, opstr) \
	INST_##name,
#include <arch/hsa/asm/Inst.def>
#undef DEFINST
	// Max
	InstOpcodeCount
};

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

 	// The private segment memory manager
 	std::unique_ptr<SegmentManager> private_segment;

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
#define DEFINST(name, opcode, opstr) \
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
 	template<typename T> void Inst_MOV_Aux();
 	template<typename SrcType, typename DstType> void Inst_CMP_Aux();
 	template<typename SrcType, typename DstType> void Inst_CVT_chop_Aux();
 	template<typename SrcType, typename DstType> void Inst_CVT_zext_Aux();
 	template<typename T> void Inst_LD_Aux();
 	template<typename T> void Inst_ST_Aux();
 	template<typename T> void Inst_LDI_Aux();

 	// Get the value of the index-th operand
 	template <typename Type>
 	Type getOperandValue(unsigned int index)
 	{
 		// Get the operand entry
 		StackFrame *stack_top = stack.back().get();
 		BrigCodeEntry *inst = stack_top->getPc();
 		auto operand = inst->getOperand(index);

 		// Do coresponding action according to the type of operand
 		switch (operand->getKind())
 		{
 		case BRIG_KIND_OPERAND_DATA:
 		{
 			BrigImmed immed(operand->getData(),
 					inst->getOperandType(index));
 			Type value = immed.getImmedValue<Type>();
 			return value;
 		}

 		case BRIG_KIND_OPERAND_WAVESIZE:

 			return 1;

 		case BRIG_KIND_OPERAND_REG:

 		{
 			std::string register_name = operand->getRegisterName();
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
 		BrigCodeEntry *inst = stack_top->getPc();
 		auto operand = inst->getOperand(index);

 		// Do corresponding action according to the type of operand
 		// I do not think there should be other type except reg
 		switch (operand->getKind())
 		{
 		case BRIG_KIND_OPERAND_REG:

 		{
 			std::string register_name = operand->getRegisterName();
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

 	// Process directives befor an instruction
 	void ExecuteDirective();




 	//
 	// Memory related fields and function
 	//
 	// Return buffer in host memory
 	char *getVariableBuffer(unsigned char segment,
 			const std::string &name);

 	// Allocate memory for variable
 	void DeclearVariable();

 public:

 	/// Create a work item. HSA should let grid object to create work item
 	WorkItem(WorkGroup *work_group,
 			unsigned private_segment_size,
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
	void PopStack(){
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

};

}  // namespace HSA

 #endif

