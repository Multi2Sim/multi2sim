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

#include "WorkItem.h"

namespace HSA
{

WorkItem::ExecuteInstFn WorkItem::execute_inst_fn[InstOpcodeCount + 1] = 
{
#define DEFINST(name, opstr) \
		&WorkItem::ExecuteInst_##name ,
#include <arch/hsa/asm/Inst.def>
#undef DEFINST
	&WorkItem::ExecuteInst_unsupported
};


void WorkItem::ExecuteInst_NOP()
{
	throw misc::Panic("Instruction not implemented");
}


template<typename T>
void WorkItem::Inst_ABS_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T des = abs(src0);
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_ABS()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_ABS_Aux<int>();
			break;
		case BRIG_TYPE_S64:
			Inst_ABS_Aux<long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_ADD_Aux()
{
	// Retrieve action
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	//Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 + src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_ADD()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_ADD_Aux<int>();
			break;
		case BRIG_TYPE_S64:
			Inst_ADD_Aux<long long>();
			break;
		case BRIG_TYPE_U32:
			Inst_ADD_Aux<unsigned int>();
			break;
		case BRIG_TYPE_U64:
			Inst_ADD_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_BORROW_Aux()
{
	// Retrieve instruction
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);

	// If overflows, set des to 1;
	T des = 0;
	if (src0 < src1)
		des = 1;

	// Store result
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_BORROW()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
		case BRIG_TYPE_U32:
			// Regard 32-bit value always as unsinged.
			Inst_BORROW_Aux<unsigned int>();
			break;
		case BRIG_TYPE_S64:
		case BRIG_TYPE_U64:
			Inst_BORROW_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_CARRY_Aux()
{
	// Retrieve instruction
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);

	// If overflows, set des to 1;
	T des = 0;
	T sum = src0 + src1;
	if (sum < src0 || sum < src1)
		des = 1;

	// Store result
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_CARRY()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
		case BRIG_TYPE_U32:
			// Regard 32-bit value always as unsinged.
			Inst_CARRY_Aux<unsigned int>();
			break;
		case BRIG_TYPE_S64:
		case BRIG_TYPE_U64:
			Inst_CARRY_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


void WorkItem::ExecuteInst_CEIL()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_COPYSIGN()
{
	throw misc::Panic("Instruction not implemented");
}


template<typename T>
void WorkItem::Inst_DIV_Aux()
{
	// Retrieve the insts
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 / src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_DIV()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_DIV_Aux<int>();
		break;
		case BRIG_TYPE_S64:
			Inst_DIV_Aux<long long>();
			break;
		case BRIG_TYPE_U32:
			Inst_DIV_Aux<unsigned int>();
			break;
		case BRIG_TYPE_U64:
			Inst_DIV_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


void WorkItem::ExecuteInst_FLOOR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_FMA()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_FRACT()
{
	throw misc::Panic("Instruction not implemented");
}


template<typename T>
void WorkItem::Inst_MAD_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T src2 = getOperandValue<T>(3);
	T des = (src0 * src1) + src2;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MAD()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform different action according to the type
	switch (inst.getType())
	{
	case BRIG_TYPE_S32:
		Inst_MAD_Aux<int>();
	break;
	case BRIG_TYPE_S64:
		Inst_MAD_Aux<long long>();
		break;
	case BRIG_TYPE_U32:
		Inst_MAD_Aux<unsigned int>();
		break;
	case BRIG_TYPE_U64:
		Inst_MAD_Aux<unsigned long long>();
		break;
	}


	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_MAX_Aux()
{
	// Retrieve insts
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = (src0 > src1) ? src0 : src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MAX()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_MAX_Aux<int>();
		break;
		case BRIG_TYPE_S64:
			Inst_MAX_Aux<long long>();
			break;
		case BRIG_TYPE_U32:
			Inst_MAX_Aux<unsigned int>();
			break;
		case BRIG_TYPE_U64:
			Inst_MAX_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_MIN_Aux()
{
	// Retrieve insts
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = (src0 < src1) ? src0 : src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MIN()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_MIN_Aux<int>();
			break;
		case BRIG_TYPE_S64:
			Inst_MIN_Aux<long long>();
			break;
		case BRIG_TYPE_U32:
			Inst_MIN_Aux<unsigned int>();
			break;
		case BRIG_TYPE_U64:
			Inst_MIN_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}

template<typename T> void WorkItem::Inst_MUL_Aux()
{
	// Retrieve instruction
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 * src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MUL()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_MUL_Aux<int>();
			break;
		case BRIG_TYPE_S64:
			Inst_MUL_Aux<long long>();
			break;
		case BRIG_TYPE_U32:
			Inst_MUL_Aux<unsigned int>();
		break;
		case BRIG_TYPE_U64:
			Inst_MUL_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_MULHI_Aux(int half_width, T lo_mask)
{
	// Retrive the inst to execute
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Get source value
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);

	// Split source value to upper and higher bits
	T src0_lo = src0 | lo_mask;
	T src0_hi = src0 >> half_width;
	T src1_lo = src1 | lo_mask;
	T src1_hi = src1 >> half_width;

	// Multiply parts to get result
	// int part1 = src0_lo * src1_lo;
	T part2 = src0_hi * src1_lo;
	T part3 = src0_lo * src1_hi;
	T part4 = src0_hi * src1_hi;

	T des = part4;

	// Detemine if part2 + part3 contribute to part 4
	if (part2 != (part2 + part3) - part3)
		des += 1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MULHI()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_MULHI_Aux<int>(16, 0xffff);
			break;
		case BRIG_TYPE_S64:
			Inst_MULHI_Aux<long long>(32, 0xffffffff);
			break;
		case BRIG_TYPE_U32:
			Inst_MULHI_Aux<unsigned int>(16, 0xffff);
			break;
		case BRIG_TYPE_U64:
			Inst_MULHI_Aux<unsigned long long>(32, 0xffffffff);
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T> void WorkItem::Inst_NEG_Aux()
{
	// Retrieve instruction
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T des = -src0;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_NEG()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_NEG_Aux<int>();
			break;
		case BRIG_TYPE_S64:
			Inst_NEG_Aux<long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_REM_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 % src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_REM()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_REM_Aux<int>();
			break;
		case BRIG_TYPE_S64:
			Inst_REM_Aux<long long>();
			break;
		case BRIG_TYPE_U32:
			Inst_REM_Aux<unsigned int>();
			break;
		case BRIG_TYPE_U64:
			Inst_REM_Aux<unsigned long long>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


void WorkItem::ExecuteInst_RINT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SQRT()
{
	throw misc::Panic("Instruction not implemented");
}


template<typename T> void WorkItem::Inst_SUB_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 - src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_SUB()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
			Inst_SUB_Aux<int>();
			break;
		case BRIG_TYPE_S64:
			Inst_SUB_Aux<long long>();
			break;
		case BRIG_TYPE_U32:
			Inst_SUB_Aux<unsigned int>();
			break;
		case BRIG_TYPE_U64:
			Inst_SUB_Aux<unsigned int>();
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


void WorkItem::ExecuteInst_TRUNC()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAD24()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAD24HI()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MUL24()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MUL24HI()
{
	throw misc::Panic("Instruction not implemented");
}


template<typename T>
void WorkItem::Inst_SHL_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	unsigned int src1 = getOperandValue<unsigned int>(2);
	T des = src0 << src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_SHL()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getType())
	{
	case BRIG_TYPE_S32:
		Inst_SHL_Aux<int>();
		break;
	case BRIG_TYPE_S64:
		Inst_SHL_Aux<long long>();
		break;
	case BRIG_TYPE_U32:
		Inst_SHL_Aux<unsigned int>();
		break;
	case BRIG_TYPE_U64:
		Inst_SHL_Aux<unsigned int>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_SHR_Aux()
{
	// FIXME:  Logic right shift
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	unsigned int src1 = getOperandValue<unsigned int>(2);
	T des = src0 >> src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_SHR()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getType())
	{
	case BRIG_TYPE_S32:
		Inst_SHR_Aux<int>();
		break;
	case BRIG_TYPE_S64:
		Inst_SHR_Aux<long long>();
		break;
	case BRIG_TYPE_U32:
		Inst_SHR_Aux<unsigned int>();
		break;
	case BRIG_TYPE_U64:
		Inst_SHR_Aux<unsigned int>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_AND_Aux()
{
	// Retrieve the inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 & src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_AND()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getType())
	{
	case BRIG_TYPE_B1:
		break;
	case BRIG_TYPE_B32:
		Inst_AND_Aux<unsigned int>();
		break;
	case BRIG_TYPE_B64:
		Inst_AND_Aux<unsigned long long>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_NOT_Aux()
{
	// Retrieve the inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T des = ~src0;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_NOT()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getType())
	{
	case BRIG_TYPE_B1:
		break;
	case BRIG_TYPE_B32:
		Inst_NOT_Aux<unsigned int>();
		break;
	case BRIG_TYPE_B64:
		Inst_NOT_Aux<unsigned long long>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_OR_Aux()
{
	// Retrieve the inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 | src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_OR()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getType())
	{
	case BRIG_TYPE_B1:
		break;
	case BRIG_TYPE_B32:
		Inst_OR_Aux<unsigned int>();
		break;
	case BRIG_TYPE_B64:
		Inst_OR_Aux<unsigned long long>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_POPCOUNT_Aux()
{
	// Retrieve the inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	unsigned int des = 0;
	while (src0!=0)
	{
		if (src0 & 1) des++;
		src0 >>= 1;
	}
	storeOperandValue<unsigned int>(0, des);
}


void WorkItem::ExecuteInst_POPCOUNT()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getSourceType())
	{
	case BRIG_TYPE_B32:
		Inst_POPCOUNT_Aux<unsigned int>();
		break;
	case BRIG_TYPE_B64:
		Inst_POPCOUNT_Aux<unsigned long long>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_XOR_Aux()
{
	// Retrieve the inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 ^ src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_XOR()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getType())
	{
	case BRIG_TYPE_B1:
		break;
	case BRIG_TYPE_B32:
		Inst_XOR_Aux<unsigned int>();
		break;
	case BRIG_TYPE_B64:
		Inst_XOR_Aux<unsigned long long>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_BITEXTRACT_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Retrieve operand value
	T src0 = getOperandValue<T>(1);
	unsigned int src1 = getOperandValue<unsigned int>(2);
	unsigned int src2 = getOperandValue<unsigned int>(3);

	// Performance action
	unsigned int length = sizeof(T) * 8;
	unsigned int offset = src1 & (length == 32 ? 31 : 63);
	unsigned int width = src2 & (length == 32 ? 31 : 63);
	T dest = 0;
	if (width != 0)
	{
		dest = (src0 << (length - width - offset)) >> (length - width);
		// signed or unsigned >>, depending on operation.type
	}
	storeOperandValue<T>(0, dest);
}


void WorkItem::ExecuteInst_BITEXTRACT()
{
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Do different action according to the kind of the inst
	switch (inst.getType())
	{
	case BRIG_TYPE_U32:
		Inst_BITEXTRACT_Aux<unsigned int>();
		break;
	case BRIG_TYPE_S32:
		Inst_BITEXTRACT_Aux<int>();
		break;
	case BRIG_TYPE_U64:
		Inst_BITEXTRACT_Aux<unsigned long long>();
		break;
	case BRIG_TYPE_S64:
		Inst_BITEXTRACT_Aux<long long>();
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


void WorkItem::ExecuteInst_BITINSERT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITMASK()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITREV()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITSELECT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_FIRSTBIT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LASTBIT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_COMBINE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_EXPAND()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDA()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDC()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MOV()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SHUFFLE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACKHI()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACKLO()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_PACK()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACK()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CMOV()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CLASS()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NCOS()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NEXP2()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NFMA()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NLOG2()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NRCP()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NRSQRT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NSIN()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NSQRT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITALIGN()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BYTEALIGN()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_PACKCVT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACKCVT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LERP()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SAD()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SADHI()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SEGMENTP()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_FTOS()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_STOF()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CMP()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CVT()
{
	throw misc::Panic("Instruction not implemented");
}


template<typename T>
void WorkItem::Inst_LD_Aux()
{
}


void WorkItem::ExecuteInst_LD()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	//
	switch (inst.getType())
	{
	case BRIG_TYPE_U32:
		break;
	case BRIG_TYPE_U64:
		break;
	case BRIG_TYPE_S32:
		break;
	case BRIG_TYPE_S64:
		break;
	}

	// Move the pc forward
	this->MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_ST_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	BrigInstEntry inst(stack_top->getPc(), binary);
	BrigInstMem *inst_buf = (BrigInstMem *)stack_top->getPc();

	// Retrieve variable name
	BrigOperandAddress *address_operand_buf =
			(BrigOperandAddress *)inst.getOperand(1);
	BrigDirectiveSymbol *symbol =
			(BrigDirectiveSymbol *)BrigDirEntry::GetDirByOffset(
					binary, address_operand_buf->symbol);
	BrigOperandEntry address_operand((char *)address_operand_buf,
			binary, &inst, 1);
	std::string name = BrigStrEntry::GetStringByOffset(
			binary, symbol->name);

	// Get offset
	unsigned long long offset =
			(((unsigned long long)address_operand_buf->offsetHi)
					<< 32)
			+ (unsigned long long)address_operand_buf->offsetLo;

	// Retrieve memory accessing in the host space
	char *host_buffer;
	switch (inst_buf->segment)
	{
	case BRIG_SEGMENT_NONE:
		break;
	case BRIG_SEGMENT_FLAT:
		break;
	case BRIG_SEGMENT_GLOBAL:
		break;
	case BRIG_SEGMENT_GROUP:
		break;
	case BRIG_SEGMENT_PRIVATE:
		break;
	case BRIG_SEGMENT_KERNARG:
		break;
	case BRIG_SEGMENT_READONLY:
		break;
	case BRIG_SEGMENT_SPILL:
		break;
	case BRIG_SEGMENT_ARG:
	{
		ArgScope *arg_scope = stack_top->getArgumentScope();
		host_buffer = arg_scope->getBuffer(name);
		host_buffer += offset;
	}
		break;
	}

	// Move value from register or immediate into memory
	T src0 = this->getOperandValue<T>(0);
	memcpy((void *)host_buffer, (void *)&src0, sizeof(T));

	std::cout << "Result of ST: " << *(T*)host_buffer << "\n";

	// Move the pc forward
	this->MovePcForwardByOne();
}


void WorkItem::ExecuteInst_ST()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Get type
	switch (inst.getType())
	{
	case BRIG_TYPE_U8:
	case BRIG_TYPE_S8:
		Inst_ST_Aux<unsigned char>();
		break;
	case BRIG_TYPE_U16:
	case BRIG_TYPE_S16:
		Inst_ST_Aux<unsigned short>();
		break;
	case BRIG_TYPE_U32:
	case BRIG_TYPE_S32:
	case BRIG_TYPE_F32:
		Inst_ST_Aux<unsigned int>();
		break;
	case BRIG_TYPE_U64:
	case BRIG_TYPE_S64:
	case BRIG_TYPE_F64:
		Inst_ST_Aux<unsigned long long>();
		break;
	}
}


void WorkItem::ExecuteInst_ATOMIC()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_ATOMICNORET()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_RDIMAGE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDIMAGE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_STIMAGE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_ATOMICIMAGE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_ATOMICIMAGENORET()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEARRAY()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEDEPTH()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEFORMAT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEHEIGHT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEORDER()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEWIDTH()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYSAMPLERCOORD()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYSAMPLERFILTER()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CBR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BRN()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BARRIER()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_ARRIVEFBAR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_INITFBAR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_JOINFBAR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LEAVEFBAR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_RELEASEFBAR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WAITFBAR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDF()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SYNC()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_COUNTLANE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_COUNTUPLANE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MASKLANE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SENDLANE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_RECEIVELANE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CALL()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_RET()
{
	ReturnFunction();
}


void WorkItem::ExecuteInst_SYSCALL()
{
	// Get the syscall code
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());
	unsigned int syscall_code = getOperandValue<unsigned int>(1);

	// Retrieve the function to be executed
	ExecuteSyscallFn fn = WorkItem::execute_syscall_fn[syscall_code];
	(this->*fn)();

	// Move pc to next instruction
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_ALLOCA()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CLEARDETECTEXCEPT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CLOCK()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CUID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CURRENTWORKGROUPSIZE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_DEBUGTRAP()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_DIM()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_DISPATCHID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_DISPATCHPTR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_GETDETECTEXCEPT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_GRIDGROUPS()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_GRIDSIZE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_LANEID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAXCUID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAXWAVEID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NULLPTR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_QPTR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SETDETECTEXCEPT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WAVEID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKGROUPID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKGROUPSIZE()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMABSID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMFLATABSID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMFLATID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMID()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_unsupported()
{
}

}  // namespace HSA

