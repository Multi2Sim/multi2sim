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

#include <arch/hsa/asm/BrigOperandEntry.h> 
#include <arch/hsa/asm/BrigImmed.h>

#include "WorkItem.h"

namespace HSA
{

template <typename Type>
Type WorkItem::getOperandValue(unsigned int index)
{
	// Get the operand entry
	BrigInstEntry inst(pc, ProgramLoader::getInstance()->getBinary());
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
		Type value = 0;
		//Type value = *((Type *)registers.getRegister(register_name));
		return value;
	}
	default:
		throw misc::Panic("Unsupported operand type "
				"for getOperandValue");
		break;
	}
	return 0;
}


template <typename Type>
void WorkItem::storeOperandValue(unsigned int index, Type value)
{
	// Get the operand entry
	BrigInstEntry inst(pc, ProgramLoader::getInstance()->getBinary());
	BrigOperandEntry operand(inst.getOperand(index), inst.getFile(), 
			&inst, index);

	// Do corresponding action according to the type of operand
	// I do not think there should be other type except reg
	switch (operand.getKind())
	{
	case BRIG_OPERAND_REG:
	{
		std::string register_name = operand.getRegisterName();
		//registers.setRegister(register_name,
		//		(char *)&value);
		break;
	}
	default:
		throw misc::Panic("Unsupported operand type "
				"for storeOperandValue");
	}
}


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
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_ADD()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_BORROW()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_CARRY()
{
	throw misc::Panic("Instruction not implemented");
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
	throw misc::Panic("Instruction not implemented");
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
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MIN()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MUL()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_MULHI()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_NEG()
{
	BrigInstEntry inst(pc, ProgramLoader::getInstance()->getBinary());

	// Do different action accoding to the kind of the inst
	if (inst.getKind() == BRIG_INST_BASIC)
	{
		switch (inst.getType())
		{
		case BRIG_TYPE_S32:
		{
			int src = getOperandValue<int>(1);
			int des = -src;
			storeOperandValue<int>(0, des);
			//emu->isa_debug << registers;
			break;
		}
		case BRIG_TYPE_S64:
			break;
		}
	}
	else if (inst.getKind() == BRIG_INST_MOD)
	{

	}
	else
	{
		throw Error("Unexpected instruction kind for NEG");
	}
}


void WorkItem::ExecuteInst_REM()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_RINT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SQRT()
{
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SUB()
{
	throw misc::Panic("Instruction not implemented");
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
	throw misc::Panic("Instruction not implemented");
}


void WorkItem::ExecuteInst_SYSCALL()
{
	throw misc::Panic("Instruction not implemented");
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

