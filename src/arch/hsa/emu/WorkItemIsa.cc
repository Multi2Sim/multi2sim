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

#include <cstring>

//#include <arch/hsa/driver/Driver.h>
#include <arch/hsa/asm/AsmService.h>

#include "WorkItem.h"


namespace HSA
{

WorkItem::ExecuteInstFn WorkItem::execute_inst_fn[InstOpcodeCount + 1] =
{
#define DEFINST(name, opcode, opstr) \
		&WorkItem::ExecuteInst_##name ,
#include <arch/hsa/asm/Inst.def>
#undef DEFINST
	&WorkItem::ExecuteInst_unsupported
};


void WorkItem::ExecuteInst_NOP()
{
	throw misc::Panic("Instruction not implemented\n");
}


template<typename T>
void WorkItem::Inst_ABS_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T des = abs(src0);
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_ABS()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
		{
		case BRIG_TYPE_S32:

			Inst_ABS_Aux<int>();
			break;

		case BRIG_TYPE_S64:

			Inst_ABS_Aux<long long>();
			break;

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst ABS, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_ADD_Aux()
{
	//Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 + src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_ADD()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst ADD, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_BORROW_Aux()
{
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
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst BORROW, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_CARRY_Aux()
{
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
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

				throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst ADD, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_CEIL()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_COPYSIGN()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


template<typename T>
void WorkItem::Inst_DIV_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 / src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_DIV()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst DIV, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_FLOOR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_FMA()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_FRACT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


template<typename T>
void WorkItem::Inst_MAD_Aux()
{
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
	BrigCodeEntry *inst = stack_top->getPc();

	// Perform different action according to the type
	switch (inst->getType())
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

	default:

		throw Error("Illegal type.");
	}


	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_MAX_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = (src0 > src1) ? src0 : src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MAX()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst MAX, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_MIN_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = (src0 < src1) ? src0 : src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MIN()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst MIN, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}

template<typename T> void WorkItem::Inst_MUL_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 * src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_MUL()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst MUL, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_MULHI_Aux(int half_width, T lo_mask)
{
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
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst MULHI, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T> void WorkItem::Inst_NEG_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T des = -src0;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_NEG()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
		{
		case BRIG_TYPE_S32:

			Inst_NEG_Aux<int>();
			break;

		case BRIG_TYPE_S64:

			Inst_NEG_Aux<long long>();
			break;

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst NEG, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_REM_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 % src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_REM()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action accoding to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst REM, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_RINT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SQRT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


template<typename T> void WorkItem::Inst_SUB_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 - src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_SUB()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	if (inst->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (inst->getType())
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

		default:

			throw Error("Illegal type.");
		}
	}
	else if (inst->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst SUB, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_TRUNC()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_MAD24()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_MAD24HI()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_MUL24()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_MUL24HI()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


template<typename T>
void WorkItem::Inst_SHL_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	unsigned int src1 = getOperandValue<unsigned int>(2);
	T des = src0 << src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_SHL()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getType())
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

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_SHR_Aux()
{
	// FIXME:  Logic right shift
	// Perform action
	T src0 = getOperandValue<T>(1);
	unsigned int src1 = getOperandValue<unsigned int>(2);
	T des = src0 >> src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_SHR()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getType())
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

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_AND_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 & src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_AND()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getType())
	{
	case BRIG_TYPE_B1:

		throw misc::Panic("Unimplemented Inst AND, type B1.");
		break;

	case BRIG_TYPE_B32:

		Inst_AND_Aux<unsigned int>();
		break;

	case BRIG_TYPE_B64:

		Inst_AND_Aux<unsigned long long>();
		break;

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_NOT_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T des = ~src0;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_NOT()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getType())
	{
	case BRIG_TYPE_B1:

		throw misc::Panic("Unimplemented Inst NOT, kind B1.");
		break;

	case BRIG_TYPE_B32:

		Inst_NOT_Aux<unsigned int>();
		break;

	case BRIG_TYPE_B64:

		Inst_NOT_Aux<unsigned long long>();
		break;

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_OR_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 | src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_OR()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getType())
	{
	case BRIG_TYPE_B1:

		throw misc::Panic("Unimplemented Inst OR, type B1.");
		break;

	case BRIG_TYPE_B32:

		Inst_OR_Aux<unsigned int>();
		break;

	case BRIG_TYPE_B64:

		Inst_OR_Aux<unsigned long long>();
		break;

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_POPCOUNT_Aux()
{
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
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getSourceType())
	{
	case BRIG_TYPE_B32:

		Inst_POPCOUNT_Aux<unsigned int>();
		break;

	case BRIG_TYPE_B64:

		Inst_POPCOUNT_Aux<unsigned long long>();
		break;

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_XOR_Aux()
{
	// Perform action
	T src0 = getOperandValue<T>(1);
	T src1 = getOperandValue<T>(2);
	T des = src0 ^ src1;
	storeOperandValue<T>(0, des);
}


void WorkItem::ExecuteInst_XOR()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getType())
	{
	case BRIG_TYPE_B1:

		throw misc::Panic("Unimplemented Inst XOR, type B1.");
		break;

	case BRIG_TYPE_B32:

		Inst_XOR_Aux<unsigned int>();
		break;

	case BRIG_TYPE_B64:

		Inst_XOR_Aux<unsigned long long>();
		break;

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_BITEXTRACT_Aux()
{
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
	BrigCodeEntry *inst = stack_top->getPc();

	// Do different action according to the kind of the inst
	switch (inst->getType())
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

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	// FIXME Loader is not a singleton, should be per context
	// FIXME Create class Context. Class Emu will have a list of contexts
	// and maybe other sub-lists of running/suspended contexts. Use
	// intrusive linked lists for this (src/lib/cpp/List.h).
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_BITINSERT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_BITMASK()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_BITREV()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_BITSELECT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_FIRSTBIT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LASTBIT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_COMBINE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_EXPAND()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


template<typename T>
void WorkItem::Inst_LDA_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Retrieve operand
	auto address_operand = inst->getOperand(1);
	auto symbol = address_operand->getSymbol();
	std::string name = symbol->getName();

	// Get offset
	unsigned long long offset = address_operand->getOffset();

	// Declare address
	unsigned address;

	// FIXME remove duplicated code

	// First, try argument scope first
	VariableScope *variable_scope = stack_top->getArgumentScope();

	// Argument scope exist
	if (variable_scope)
	{
		address = variable_scope->getAddress(name);

		// If address found
		if (address)
		{
			storeOperandValue<T>(0, (T)(address + offset));
			return;
		}
	}

	// Second, try variable scope
	variable_scope = stack_top->getVariableScope();
	if (variable_scope)
	{
		address = variable_scope->getAddress(name);

		// If address found
		if (address)
		{
			storeOperandValue<T>(0, (T)(address + offset));
			return;
		}
	}

	// Third, try variable scope
	variable_scope = stack_top->getFunctionArguments();
	if (variable_scope)
	{
		address = variable_scope->getAddress(name);

		// If address found
		if (address)
		{
			storeOperandValue<T>(0, (T)(address + offset));
			return;
		}
	}

	throw misc::Panic(misc::fmt("Variable %s not found!\n", name.c_str()));

	// Store the address in dest
	storeOperandValue<T>(0, 0);

}


void WorkItem::ExecuteInst_LDA()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();
	
	switch(inst->getType())
	{
	case BRIG_TYPE_U32:
		Inst_LDA_Aux<unsigned int>();
		break;
	case BRIG_TYPE_U64:
		Inst_LDA_Aux<unsigned long long>();
		break;
	default:
		throw misc::Error("Unsupported type for instruction LDA.");
		break;
	}

	// Move PC forward.
	MovePcForwardByOne();

}


template<typename T>
void WorkItem::Inst_MOV_Aux()
{
	// Retrieve inst
	// StackFrame *stack_top = stack.back().get();
	// BrigInstEntry inst(stack_top->getPc(),
	//		ProgramLoader::getInstance()->getBinary());
	// BrigInstBasic *inst_buf = (BrigInstBasic *)stack_top->getPc();

	// Retrieve src value
	T src0 = getOperandValue<T>(1);

	// Move to dst value
	T dst = src0;
	storeOperandValue<T>(0, dst);
}


void WorkItem::ExecuteInst_MOV()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Call auxiliary function on different type
	switch (inst->getType()){
	case BRIG_TYPE_B1:

		Inst_MOV_Aux<unsigned char>();
		break;

	case BRIG_TYPE_B32:
	case BRIG_TYPE_U32:

		Inst_MOV_Aux<unsigned int>();
		break;

	case BRIG_TYPE_B64:
	case BRIG_TYPE_U64:

		Inst_MOV_Aux<unsigned long long>();
		break;

	case BRIG_TYPE_S32:

		Inst_MOV_Aux<int>();
		break;

	case BRIG_TYPE_S64:

		Inst_MOV_Aux<long long>();
		break;

	case BRIG_TYPE_F32:

		Inst_MOV_Aux<float>();
		break;

	case BRIG_TYPE_F64:

		Inst_MOV_Aux<double>();
		break;

	default:

		throw misc::Panic("Unsupported type for opcode MOV.");
		break;

	}

	// Move PC forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_SHUFFLE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_UNPACKHI()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_UNPACKLO()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_PACK()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_UNPACK()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CMOV()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CLASS()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NCOS()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NEXP2()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NFMA()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NLOG2()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NRCP()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NRSQRT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NSIN()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NSQRT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_BITALIGN()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_BYTEALIGN()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_PACKCVT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_UNPACKCVT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LERP()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SAD()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SADHI()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SEGMENTP()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_FTOS()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_STOF()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


template<typename SrcType, typename DstType>
void WorkItem::Inst_CMP_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	//Get source value
	SrcType src1 = getOperandValue<SrcType>(1);
	SrcType src2 = getOperandValue<SrcType>(2);
	DstType dst = 0;

	switch (inst->getCompareOperation()){
	case BRIG_COMPARE_EQ:

		if (src1 == src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_NE:

		if (src1 != src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_LT:

		if (src1 < src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_LE:

		if (src1 <= src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_GT:

		if (src1 > src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_GE:

		if (src1 >= src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_EQU:
	case BRIG_COMPARE_NEU:
	case BRIG_COMPARE_LTU:
	case BRIG_COMPARE_LEU:
	case BRIG_COMPARE_GTU:
	case BRIG_COMPARE_GEU:
	case BRIG_COMPARE_NUM:
	case BRIG_COMPARE_NAN:
	case BRIG_COMPARE_SEQ:
	case BRIG_COMPARE_SNE:
	case BRIG_COMPARE_SLT:
	case BRIG_COMPARE_SLE:
	case BRIG_COMPARE_SGT:
	case BRIG_COMPARE_SGE:
	case BRIG_COMPARE_SGEU:
	case BRIG_COMPARE_SEQU:
	case BRIG_COMPARE_SNEU:
	case BRIG_COMPARE_SLTU:
	case BRIG_COMPARE_SLEU:
	case BRIG_COMPARE_SNUM:
	case BRIG_COMPARE_SNAN:
	case BRIG_COMPARE_SGTU:
	default:
		throw misc::Panic("Unimplemented compare operation.");

	}

	// Store result value
	storeOperandValue<DstType>(0, dst);

	// Move PC forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_CMP()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	switch (inst->getSourceType()){
	case BRIG_TYPE_B1:
	case BRIG_TYPE_B8:

		Inst_CMP_Aux<unsigned char, unsigned char>();
		break;

	case BRIG_TYPE_B16:
	case BRIG_TYPE_U16:

		Inst_CMP_Aux<unsigned short, unsigned char>();
		break;

	case BRIG_TYPE_B32:
	case BRIG_TYPE_U32:

		Inst_CMP_Aux<unsigned int, unsigned char>();
		break;

	case BRIG_TYPE_B64:
	case BRIG_TYPE_U64:

		Inst_CMP_Aux<unsigned long long, unsigned char>();
		break;

	case BRIG_TYPE_S32:

		Inst_CMP_Aux<int, unsigned char>();
		break;

	case BRIG_TYPE_S64:

		Inst_CMP_Aux<long long, unsigned char>();
		break;

	default:

		throw misc::Panic("Unsupported source type for CMP.");
		break;

	}
}


template<typename SrcType, typename DstType> void WorkItem::Inst_CVT_chop_Aux()
{
	SrcType src = getOperandValue<SrcType>(1);
	DstType dst = 0;

	// Use memcpy to chop
	memcpy(&dst, &src, sizeof(DstType));

	storeOperandValue<DstType>(0, dst);
}


template<typename SrcType, typename DstType> void WorkItem::Inst_CVT_zext_Aux()
{
	SrcType src = getOperandValue<SrcType>(1);
	storeOperandValue<DstType>(0, src);
}


void WorkItem::ExecuteInst_CVT()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Get src type and dst type
	BrigTypeX src_type = inst->getSourceType();
	BrigTypeX dst_type = inst->getType();

	if (src_type == BRIG_TYPE_U16 && dst_type == BRIG_TYPE_U32)
	{
		Inst_CVT_zext_Aux<unsigned short, unsigned int>();
	}
	else if (src_type == BRIG_TYPE_U32 && dst_type == BRIG_TYPE_U64)
	{
		Inst_CVT_zext_Aux<unsigned int, unsigned long long>();
	}
	else
	{
		throw misc::Panic(misc::fmt("Conversion between %s and %s "
				"is not supported\n",
				AsmService::TypeToString(src_type).c_str(),
				AsmService::TypeToString(dst_type).c_str()));
	}

	// Move PC forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_LD_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Get variable name
	auto address_operand = inst->getOperand(1);
	if (address_operand->getSymbol().get())
	{
		auto symbol = address_operand->getSymbol();
		std::string name = symbol->getName();

		// Get offset
		unsigned long long offset = address_operand->getOffset();

		// Get buffer in host
		char *host_buffer = getVariableBuffer(inst->getSegment(), name);
		host_buffer += offset;

		// Move value from register or immediate into memory
		storeOperandValue<T>(0, *((T *)host_buffer));
	}
	// If the address is stored in a register
	else
	{
		std::string register_name = address_operand->getReg()
					->getRegisterName();
		unsigned address = stack_top->getRegisterValue<unsigned>(
				register_name);

		unsigned long long offset = address_operand->getOffset();
		address += offset;

		// Get buffer in host
		char *host_buffer = Emu::getInstance()->getMemory()->getBuffer(
				address, sizeof(T), mem::Memory::AccessRead);

		// Move value from register or immediate into memory
		storeOperandValue<T>(0, *((T *)host_buffer));

	}
}


void WorkItem::ExecuteInst_LD()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	//
	switch (inst->getType())
	{
	case BRIG_TYPE_U32:

		Inst_LD_Aux<unsigned int>();
		break;

	case BRIG_TYPE_S32:

		Inst_LD_Aux<int>();
		break;

	case BRIG_TYPE_F32:

		Inst_LD_Aux<float>();
		break;

	case BRIG_TYPE_U64:

		Inst_LD_Aux<unsigned long long>();
		break;

	case BRIG_TYPE_S64:

		Inst_LD_Aux<long long>();
		break;

	case BRIG_TYPE_F64:

		Inst_LD_Aux<double>();
		break;

	default:

		throw misc::Panic("Unimplemented type for Inst LD.");
		break;
	}

	// Move the pc forward
	MovePcForwardByOne();
}


template<typename T>
void WorkItem::Inst_ST_Aux()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Retrieve variable name
	auto address_operand = inst->getOperand(1);
	if (address_operand->getSymbol().get())
	{
		auto symbol = address_operand->getSymbol();
		std::string name = symbol->getName();
		// Get offset
		unsigned long long offset = address_operand->getOffset();\

		// Retrieve memory accessing in the host space
		char *host_buffer = this->getVariableBuffer(inst->getSegment(),
				name);
		host_buffer += offset;

		// Move value from register or immediate into memory
		T src0 = getOperandValue<T>(0);
		memcpy((void *)host_buffer, (void *)&src0, sizeof(T));
	}
	else
	{
		std::string register_name = address_operand->getReg()
				->getRegisterName();
		unsigned address = stack_top->getRegisterValue<unsigned>(
				register_name);

		unsigned long long offset = address_operand->getOffset();
		address += offset;

		// Get buffer in host
		char *host_buffer = Emu::getInstance()->getMemory()->getBuffer(
				address, sizeof(T), mem::Memory::AccessWrite);

		// Move value from register or immediate into memory
		T src0 = getOperandValue<T>(0);
		memcpy((void *)host_buffer, (void *)&src0, sizeof(T));
	}

}


void WorkItem::ExecuteInst_ST()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Get type
	switch (inst->getType())
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

	default:
		throw misc::Panic("Type is not supported");
		break;
	}

	// Move the pc forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_ATOMIC()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ATOMICNORET()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SIGNAL()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SIGNALNORET()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_MEMFENCE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_RDIMAGE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LDIMAGE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_STIMAGE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_QUERYIMAGE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_QUERYSAMPLER()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CBR()
{
	// Retrieve
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Retrieve condition
	unsigned char condition = getOperandValue<unsigned char>(0);

	// Jump if condition is true
	if (condition){
		// Retrieve 1st operand
		auto operand1 = inst->getOperand(1);
		if (operand1->getKind() == BRIG_KIND_OPERAND_CODE_REF)
		{
			auto label = operand1->getRef();

			// If the label is an the end or beyond the end of the
			// function, return the function
			/*
			char *code = BrigInstEntry::GetInstByOffset(
					binary, label->code);
			if (code >= stack_top->getFunction()->getLastInst())
			{
				ReturnFunction();
				return;
			}
			*/

			// Redirect pc to a certain label
			stack_top->setPc(std::move(label));
			return;
		}else{
			throw misc::Panic("Unsupported operand type for CBR.");
		}
	}

	// Move PC forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_BR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SBR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_BARRIER()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WAVEBARRIER()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ARRIVEFBAR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_INITFBAR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_JOINFBAR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LEAVEFBAR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_RELEASEFBAR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WAITFBAR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LDF()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ACTIVELANECOUNT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ACTIVELANEID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ACTIVELANEMASK()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ACTIVELANESHUFFLE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CALL()
{
	// Retrieve instruction
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Retrieve the function name
	auto function_operand = inst->getOperand(1);
	std::string function_name = function_operand->getRef()->getName();

	// Try to intercept the function execution if the function is runtime
	// function
	if (Driver::getInstance()->Intercept(function_name, stack_top))
	{
		// If a call back stack frame is implanted do not move pc
		// forward
		StackFrame *new_stack_top = stack.back().get();
		if (new_stack_top != stack_top)
			return;

		// Otherwize, move PC forward
		MovePcForwardByOne();
		return;
	}


	// Retrieve the function
	Function *function = loader->getFunction(function_name);

	// Prepare stack frame and pass the argument by value
	auto new_frame = misc::new_unique<StackFrame>(function, this);
	function->PassByValue(stack_top, new_frame.get(), inst);

	// Push frame in stack
	stack.push_back(std::move(new_frame));

	// Dump backtrace information for debugging purpose
	if (Emu::isa_debug)
		Backtrace(Emu::isa_debug);

}


void WorkItem::ExecuteInst_SCALL()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ICALL()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


template<typename T> void WorkItem::Inst_LDI_Aux()
{
	// Retrieve instruction
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Get function operand
	auto operand1 = inst->getOperand(1);

	// Store the offset of the function
	storeOperandValue<T>(0, operand1->getRef()->getOffset());

	// Move PC forward
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_LDI()
{
	// Retrieve instruction
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Perform action according to type
	switch(inst->getType())
	{
	case BRIG_TYPE_U32:
		Inst_LDI_Aux<unsigned int>();
		break;
	case BRIG_TYPE_U64:
		Inst_LDI_Aux<unsigned long long>();
		break;
	default:
		std::string type_str = AsmService::TypeToString(
				inst->getType());
		throw misc::Panic(misc::fmt("Unsupported type %s for inst LDI",
				type_str.c_str()));
	}

}


void WorkItem::ExecuteInst_RET()
{
	// Return the function
	ReturnFunction();
	if (Emu::isa_debug)
		Backtrace(Emu::isa_debug);
}



void WorkItem::ExecuteInst_ALLOCA()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CURRENTWORKGROUPSIZE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_DIM()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_GRIDGROUPS()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_GRIDSIZE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_PACKETCOMPLETIONSIG()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_PACKETID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKGROUPID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKGROUPSIZE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKITEMABSID()
{
	unsigned int dim = getOperandValue<unsigned int>(1);
	switch(dim)
	{
	case 0:
		storeOperandValue<unsigned int>(0, abs_id_x);
		break;
	case 1:
		storeOperandValue<unsigned int>(0, abs_id_y);
		break;
	case 2:
		storeOperandValue<unsigned int>(0, abs_id_z);
		break;
	default:
		throw misc::Error("Trying to getting work item absolute id "
				"other than x, y and z axis.");
	}

	// Move pc to next instruction
	MovePcForwardByOne();
}


void WorkItem::ExecuteInst_WORKITEMFLATABSID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKITEMFLATID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKITEMID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CLEARDETECTEXCEPT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_GETDETECTEXCEPT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_SETDETECTEXCEPT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ADDQUEUEWRITEINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_AGENTCOUNT()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_AGENTID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CASQUEUEWRITEINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LDK()
{
	ExecuteInst_LDI();
}


void WorkItem::ExecuteInst_LDQUEUEREADINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LDQUEUEWRITEINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_QUEUEID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_QUEUEPTR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_STQUEUEREADINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_STQUEUEWRITEINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CLOCK()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CUID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_DEBUGTRAP()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_GROUPBASEPTR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_KERNARGBASEPTR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LANEID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_MAXCUID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_MAXWAVEID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_NULLPTR()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WAVEID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}



void WorkItem::ExecuteInst_unsupported()
{
}

}  // namespace HSA

