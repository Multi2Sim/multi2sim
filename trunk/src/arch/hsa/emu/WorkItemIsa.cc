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
		{
			int src0 = getOperandValue<int>(1);
			int des = abs(src0);
			storeOperandValue<int>(0, des);
			break;
		}
		case BRIG_TYPE_S64:
		{
			long long src0 = getOperandValue<long long>(1);
			long long des = abs(src0);
			storeOperandValue<long long>(0, des);
		}
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
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
		{
			int src0 = getOperandValue<int>(1);
			int src1 = getOperandValue<int>(2);
			int des = src0 + src1;
			storeOperandValue<int>(0, des);
			break;
		}
		case BRIG_TYPE_S64:
		{
			long long src0 = getOperandValue<long long>(1);
			long long src1 = getOperandValue<long long>(2);
			long long des = src0 + src1;
			storeOperandValue<long long>(0, des);
		}
			break;
		case BRIG_TYPE_U32:
		{
			unsigned int src0 = getOperandValue<unsigned int>(1);
			unsigned int src1 = getOperandValue<unsigned int>(2);
			unsigned int des = src0 + src1;
			storeOperandValue<unsigned int>(0, des);
			break;
		}
		case BRIG_TYPE_U64:
		{
			unsigned long long src0 = getOperandValue<unsigned long long>(1);
			unsigned long long src1 = getOperandValue<unsigned long long>(2);
			unsigned long long des = src0 + src1;
			storeOperandValue<unsigned long long>(0, des);
		}
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
		{
			int src0 = getOperandValue<int>(1);
			int src1 = getOperandValue<int>(2);
			int des = src0 / src1;
			storeOperandValue<int>(0, des);
		}
		break;
		case BRIG_TYPE_S64:
		{
			long long src0 = getOperandValue<long long>(1);
			long long src1 = getOperandValue<long long>(2);
			long long des = src0 / src1;
			storeOperandValue<long long>(0, des);
		}
			break;
		case BRIG_TYPE_U32:
		{
			unsigned int src0 = getOperandValue<unsigned int>(1);
			unsigned int src1 = getOperandValue<unsigned int>(2);
			unsigned int des = src0 / src1;
			storeOperandValue<unsigned int>(0, des);
		}
		break;
		case BRIG_TYPE_U64:
		{
			unsigned long long src0 = getOperandValue<unsigned long long>(1);
			unsigned long long src1 = getOperandValue<unsigned long long>(2);
			unsigned long long des = src0 / src1;
			storeOperandValue<unsigned long long>(0, des);
		}
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


void WorkItem::ExecuteInst_MAD()
{
	throw misc::Panic("Instruction not implemented");
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
		{
			int src0 = getOperandValue<int>(1);
			int src1 = getOperandValue<int>(2);
			int des = (src0 > src1) ? src0 : src1;
			storeOperandValue<int>(0, des);
		}
		break;
		case BRIG_TYPE_S64:
		{
			long long src0 = getOperandValue<long long>(1);
			long long src1 = getOperandValue<long long>(2);
			long long des = (src0 > src1) ? src0 : src1;
			storeOperandValue<long long>(0, des);
		}
			break;
		case BRIG_TYPE_U32:
		{
			unsigned int src0 = getOperandValue<unsigned int>(1);
			unsigned int src1 = getOperandValue<unsigned int>(2);
			unsigned int des = (src0 > src1) ? src0 : src1;
			storeOperandValue<unsigned int>(0, des);
		}
		break;
		case BRIG_TYPE_U64:
		{
			unsigned long long src0 = getOperandValue<unsigned long long>(1);
			unsigned long long src1 = getOperandValue<unsigned long long>(2);
			unsigned long long des = (src0 > src1) ? src0 : src1;
			storeOperandValue<unsigned long long>(0, des);
		}
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}

	// Move the pc forward
	this->MovePcForwardByOne();
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
		{
			int src0 = getOperandValue<int>(1);
			int src1 = getOperandValue<int>(2);
			int des = (src0 < src1) ? src0 : src1;
			storeOperandValue<int>(0, des);
		}
		break;
		case BRIG_TYPE_S64:
		{
			long long src0 = getOperandValue<long long>(1);
			long long src1 = getOperandValue<long long>(2);
			long long des = (src0 < src1) ? src0 : src1;
			storeOperandValue<long long>(0, des);
		}
			break;
		case BRIG_TYPE_U32:
		{
			unsigned int src0 = getOperandValue<unsigned int>(1);
			unsigned int src1 = getOperandValue<unsigned int>(2);
			unsigned int des = (src0 < src1) ? src0 : src1;
			storeOperandValue<unsigned int>(0, des);
		}
		break;
		case BRIG_TYPE_U64:
		{
			unsigned long long src0 = getOperandValue<unsigned long long>(1);
			unsigned long long src1 = getOperandValue<unsigned long long>(2);
			unsigned long long des = (src0 < src1) ? src0 : src1;
			storeOperandValue<unsigned long long>(0, des);
		}
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
		{
			int src0 = getOperandValue<int>(1);
			int src1 = getOperandValue<int>(2);
			int des = src0 % src1;
			storeOperandValue<int>(0, des);
		}
		break;
		case BRIG_TYPE_S64:
		{
			long long src0 = getOperandValue<long long>(1);
			long long src1 = getOperandValue<long long>(2);
			long long des = src0 % src1;
			storeOperandValue<long long>(0, des);
		}
			break;
		case BRIG_TYPE_U32:
		{
			unsigned int src0 = getOperandValue<unsigned int>(1);
			unsigned int src1 = getOperandValue<unsigned int>(2);
			unsigned int des = src0 % src1;
			storeOperandValue<unsigned int>(0, des);
		}
		break;
		case BRIG_TYPE_U64:
		{
			unsigned long long src0 = getOperandValue<unsigned long long>(1);
			unsigned long long src1 = getOperandValue<unsigned long long>(2);
			unsigned long long des = src0 % src1;
			storeOperandValue<unsigned long long>(0, des);
		}
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
	int src0 = getOperandValue<int>(1);
	int src1 = getOperandValue<int>(2);
	int des = src0 - src1;
	storeOperandValue<int>(0, des);
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


void WorkItem::ExecuteInst_SHL()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SHR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_AND()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NOT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_OR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_POPCOUNT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_XOR()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITEXTRACT()
{
	throw misc::Panic("Instruction not implemented");
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


void WorkItem::ExecuteInst_LD()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_ST()
{
	throw misc::Panic("Instruction not implemented");
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

