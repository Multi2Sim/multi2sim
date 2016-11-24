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

#include <lib/cpp/Misc.h>
#include <arch/hsa/disassembler/AsmService.h>
#include <arch/hsa/driver/HsaExecutable.h>

#include "WorkItem.h"


namespace HSA
{

/*
WorkItem::ExecuteInstFn WorkItem::execute_inst_fn[InstOpcodeCount + 1] =
{
#define DEFINST(name, opcode, opstr) \
		&WorkItem::ExecuteInst_##name ,
#include <arch/hsa/disassembler/Instruction.def>
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
	T src0;
	getOperandValue(1, &src0);
	T des = abs(src0);
	setOperandValue(0, &des);
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
void WorkItem::Inst_BORROW_Aux()
{
	// Perform action
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);

	// If overflows, set des to 1;
	T des = 0;
	if (src0 < src1)
		des = 1;

	// Store result
	setOperandValue(0, &des);
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
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);

	// If overflows, set des to 1;
	T des = 0;
	T sum = src0 + src1;
	if (sum < src0 || sum < src1)
		des = 1;

	// Store result
	setOperandValue(0, &des);
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
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);
	T des = src0 / src1;
	setOperandValue(0, &des);
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
		switch (inst->getType())
		{
		case BRIG_TYPE_F32:

			Inst_DIV_Aux<float>();
			break;

		case BRIG_TYPE_F64:

			Inst_DIV_Aux<double>();
			break;

		default:

			throw Error("Illegal type.");
		}

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
	T src0;
	T src1;
	T src2;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);
	getOperandValue(3, &src2);
	T des = (src0 * src1) + src2;
	setOperandValue(0, &des);
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
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);
	T des = (src0 > src1) ? src0 : src1;
	setOperandValue(0, &des);
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
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);
	T des = (src0 < src1) ? src0 : src1;
	setOperandValue(0, &des);
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


template<typename T>
void WorkItem::Inst_MULHI_Aux(int half_width, T lo_mask)
{
	// Get source value
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);

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
	setOperandValue(0, &des);
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
	T src0;
	getOperandValue(1, &src0);
	T des = -src0;
	setOperandValue(0, &des);
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
		switch (inst->getType())
		{
		case BRIG_TYPE_F32:

			Inst_NEG_Aux<float>();
			break;

		case BRIG_TYPE_F64:

			Inst_NEG_Aux<double>();
			break;

		default:

			throw Error("Illegal type.");
		}
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
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);
	T des = src0 % src1;
	setOperandValue(0, &des);
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
void WorkItem::Inst_NOT_Aux()
{
	// Perform action
	T src0;
	getOperandValue(1, &src0);
	T des = ~src0;
	setOperandValue(0, &des);
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
void WorkItem::Inst_POPCOUNT_Aux()
{
	// Get operand value
	T src0;
	getOperandValue(1, &src0);

	// Calculate
	unsigned int des = 0;
	while (src0!=0)
	{
		if (src0 & 1) des++;
		src0 >>= 1;
	}
	setOperandValue(0, &des);
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
	T src0;
	T src1;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);
	T des = src0 ^ src1;
	setOperandValue(0, &des);
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
	T src0;
	unsigned int src1;
	unsigned int src2;
	getOperandValue(1, &src0);
	getOperandValue(2, &src1);
	getOperandValue(3, &src2);

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
	setOperandValue(0, &dest);
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

template<typename T> void WorkItem::Inst_CMOV_Aux() {
	// Retrieve src0 value
	unsigned char src0;
	getOperandValue(1, &src0);

	// Retrieve src1, src2
	T src1, src2;
	getOperandValue(2, &src1);
	getOperandValue(3, &src2);

	// Move to dst value
	T dst;
	if (src0) 
		dst = src1;
	else 
		dst = src2;
	setOperandValue(0, &dst);
}


void WorkItem::ExecuteInst_CMOV()
{
	// Retrieve inst
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *inst = stack_top->getPc();

	// Call auxiliary function on different type
	switch (inst->getType()){
	case BRIG_TYPE_B1:

		Inst_CMOV_Aux<unsigned char>();
		break;

	case BRIG_TYPE_B32:
	case BRIG_TYPE_U32:

		Inst_CMOV_Aux<unsigned int>();
		break;

	case BRIG_TYPE_B64:
	case BRIG_TYPE_U64:

		Inst_CMOV_Aux<unsigned long long>();
		break;

	case BRIG_TYPE_S32:

		Inst_CMOV_Aux<int>();
		break;

	case BRIG_TYPE_S64:

		Inst_CMOV_Aux<long long>();
		break;

	case BRIG_TYPE_F32:

		Inst_CMOV_Aux<float>();
		break;

	case BRIG_TYPE_F64:

		Inst_CMOV_Aux<double>();
		break;

	default:

		throw misc::Panic("Unsupported type for opcode CMOV.");
		break;

	}

	// Move PC forward
	MovePcForwardByOne();
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


void WorkItem::ExecuteInst_IMAGEFENCE()
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


void WorkItem::ExecuteInst_SBR()
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


void WorkItem::ExecuteInst_ACTIVELANEPERMUTE()
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

	// Retrieve the function
	// Function *function = executable->getFunction(function_name);

	// Prepare stack frame and pass the argument by value
	// auto new_frame = misc::new_unique<StackFrame>(function, this);
	// function->PassByValue(stack_top, new_frame.get(), inst);

	// Push frame in stack
	// stack.push_back(std::move(new_frame));
}


void WorkItem::ExecuteInst_SCALL()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ICALL()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_ALLOCA()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_CURRENTWORKITEMFLATID()
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


void WorkItem::ExecuteInst_PACKETCOMPLETIONSIG()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_PACKETID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKGROUPSIZE()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKITEMFLATABSID()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_WORKITEMFLATID()
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


void WorkItem::ExecuteInst_CASQUEUEWRITEINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LDQUEUEREADINDEX()
{
	throw misc::Panic(misc::fmt("Instruction not implemented %s\n", __FUNCTION__));
}


void WorkItem::ExecuteInst_LDQUEUEWRITEINDEX()
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

void WorkItem::ExecuteInst_GCNMIN() 
{
	ExecuteInst_MIN();
}


void WorkItem::ExecuteInst_unsupported()
{
}

*/

}  // namespace HSA

