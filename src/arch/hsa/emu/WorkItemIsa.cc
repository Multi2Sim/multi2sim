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
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ABS()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ADD()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BORROW()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CARRY()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CEIL()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_COPYSIGN()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_DIV()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_FLOOR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_FMA()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_FRACT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAD()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAX()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MIN()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MUL()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MULHI()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NEG()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_REM()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_RINT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SQRT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SUB()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_TRUNC()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAD24()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAD24HI()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MUL24()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MUL24HI()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SHL()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SHR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_AND()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NOT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_OR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_POPCOUNT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_XOR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITEXTRACT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITINSERT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITMASK()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITREV()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITSELECT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_FIRSTBIT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LASTBIT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_COMBINE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_EXPAND()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDA()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDC()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MOV()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SHUFFLE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACKHI()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACKLO()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_PACK()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACK()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CMOV()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CLASS()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NCOS()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NEXP2()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NFMA()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NLOG2()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NRCP()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NRSQRT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NSIN()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NSQRT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BITALIGN()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BYTEALIGN()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_PACKCVT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_UNPACKCVT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LERP()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SAD()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SADHI()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SEGMENTP()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_FTOS()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_STOF()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CMP()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CVT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LD()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ST()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ATOMIC()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ATOMICNORET()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_RDIMAGE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDIMAGE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_STIMAGE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ATOMICIMAGE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ATOMICIMAGENORET()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEARRAY()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEDEPTH()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEFORMAT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEHEIGHT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEORDER()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYIMAGEWIDTH()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYSAMPLERCOORD()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QUERYSAMPLERFILTER()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CBR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BRN()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_BARRIER()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ARRIVEFBAR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_INITFBAR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_JOINFBAR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LEAVEFBAR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_RELEASEFBAR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WAITFBAR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LDF()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SYNC()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_COUNTLANE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_COUNTUPLANE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MASKLANE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SENDLANE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_RECEIVELANE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CALL()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_RET()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SYSCALL()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_ALLOCA()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CLEARDETECTEXCEPT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CLOCK()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CUID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_CURRENTWORKGROUPSIZE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_DEBUGTRAP()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_DIM()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_DISPATCHID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_DISPATCHPTR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_GETDETECTEXCEPT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_GRIDGROUPS()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_GRIDSIZE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_LANEID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAXCUID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_MAXWAVEID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_NULLPTR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_QPTR()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_SETDETECTEXCEPT()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WAVEID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKGROUPID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKGROUPSIZE()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMABSID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMFLATABSID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMFLATID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_WORKITEMID()
{
	throw std::logic_error("Instruction not implemented");
}


void WorkItem::ExecuteInst_unsupported()
{
}

}  // namespace HSA

