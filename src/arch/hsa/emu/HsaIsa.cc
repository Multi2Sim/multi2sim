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

void WorkItem::ExecuteInst_NOP(){}
void WorkItem::ExecuteInst_ABS(){}
void WorkItem::ExecuteInst_ADD(){}
void WorkItem::ExecuteInst_BORROW(){}
void WorkItem::ExecuteInst_CARRY(){}
void WorkItem::ExecuteInst_CEIL(){}
void WorkItem::ExecuteInst_COPYSIGN(){}
void WorkItem::ExecuteInst_DIV(){}
void WorkItem::ExecuteInst_FLOOR(){}
void WorkItem::ExecuteInst_FMA(){}
void WorkItem::ExecuteInst_FRACT(){}
void WorkItem::ExecuteInst_MAD(){}
void WorkItem::ExecuteInst_MAX(){}
void WorkItem::ExecuteInst_MIN(){}
void WorkItem::ExecuteInst_MUL(){}
void WorkItem::ExecuteInst_MULHI(){}
void WorkItem::ExecuteInst_NEG(){}
void WorkItem::ExecuteInst_REM(){}
void WorkItem::ExecuteInst_RINT(){}
void WorkItem::ExecuteInst_SQRT(){}
void WorkItem::ExecuteInst_SUB(){}
void WorkItem::ExecuteInst_TRUNC(){}
void WorkItem::ExecuteInst_MAD24(){}
void WorkItem::ExecuteInst_MAD24HI(){}
void WorkItem::ExecuteInst_MUL24(){}
void WorkItem::ExecuteInst_MUL24HI(){}
void WorkItem::ExecuteInst_SHL(){}
void WorkItem::ExecuteInst_SHR(){}
void WorkItem::ExecuteInst_AND(){}
void WorkItem::ExecuteInst_NOT(){}
void WorkItem::ExecuteInst_OR(){}
void WorkItem::ExecuteInst_POPCOUNT(){}
void WorkItem::ExecuteInst_XOR(){}
void WorkItem::ExecuteInst_BITEXTRACT(){}
void WorkItem::ExecuteInst_BITINSERT(){}
void WorkItem::ExecuteInst_BITMASK(){}
void WorkItem::ExecuteInst_BITREV(){}
void WorkItem::ExecuteInst_BITSELECT(){}
void WorkItem::ExecuteInst_FIRSTBIT(){}
void WorkItem::ExecuteInst_LASTBIT(){}
void WorkItem::ExecuteInst_COMBINE(){}
void WorkItem::ExecuteInst_EXPAND(){}
void WorkItem::ExecuteInst_LDA(){}
void WorkItem::ExecuteInst_LDC(){}
void WorkItem::ExecuteInst_MOV(){}
void WorkItem::ExecuteInst_SHUFFLE(){}
void WorkItem::ExecuteInst_UNPACKHI(){}
void WorkItem::ExecuteInst_UNPACKLO(){}
void WorkItem::ExecuteInst_PACK(){}
void WorkItem::ExecuteInst_UNPACK(){}
void WorkItem::ExecuteInst_CMOV(){}
void WorkItem::ExecuteInst_CLASS(){}
void WorkItem::ExecuteInst_NCOS(){}
void WorkItem::ExecuteInst_NEXP2(){}
void WorkItem::ExecuteInst_NFMA(){}
void WorkItem::ExecuteInst_NLOG2(){}
void WorkItem::ExecuteInst_NRCP(){}
void WorkItem::ExecuteInst_NRSQRT(){}
void WorkItem::ExecuteInst_NSIN(){}
void WorkItem::ExecuteInst_NSQRT(){}
void WorkItem::ExecuteInst_BITALIGN(){}
void WorkItem::ExecuteInst_BYTEALIGN(){}
void WorkItem::ExecuteInst_PACKCVT(){}
void WorkItem::ExecuteInst_UNPACKCVT(){}
void WorkItem::ExecuteInst_LERP(){}
void WorkItem::ExecuteInst_SAD(){}
void WorkItem::ExecuteInst_SADHI(){}
void WorkItem::ExecuteInst_SEGMENTP(){}
void WorkItem::ExecuteInst_FTOS(){}
void WorkItem::ExecuteInst_STOF(){}
void WorkItem::ExecuteInst_CMP(){}
void WorkItem::ExecuteInst_CVT(){}
void WorkItem::ExecuteInst_LD(){}
void WorkItem::ExecuteInst_ST(){}
void WorkItem::ExecuteInst_ATOMIC(){}
void WorkItem::ExecuteInst_ATOMICNORET(){}
void WorkItem::ExecuteInst_RDIMAGE(){}
void WorkItem::ExecuteInst_LDIMAGE(){}
void WorkItem::ExecuteInst_STIMAGE(){}
void WorkItem::ExecuteInst_ATOMICIMAGE(){}
void WorkItem::ExecuteInst_ATOMICIMAGENORET(){}
void WorkItem::ExecuteInst_QUERYIMAGEARRAY(){}
void WorkItem::ExecuteInst_QUERYIMAGEDEPTH(){}
void WorkItem::ExecuteInst_QUERYIMAGEFORMAT(){}
void WorkItem::ExecuteInst_QUERYIMAGEHEIGHT(){}
void WorkItem::ExecuteInst_QUERYIMAGEORDER(){}
void WorkItem::ExecuteInst_QUERYIMAGEWIDTH(){}
void WorkItem::ExecuteInst_QUERYSAMPLERCOORD(){}
void WorkItem::ExecuteInst_QUERYSAMPLERFILTER(){}
void WorkItem::ExecuteInst_CBR(){}
void WorkItem::ExecuteInst_BRN(){}
void WorkItem::ExecuteInst_BARRIER(){}
void WorkItem::ExecuteInst_ARRIVEFBAR(){}
void WorkItem::ExecuteInst_INITFBAR(){}
void WorkItem::ExecuteInst_JOINFBAR(){}
void WorkItem::ExecuteInst_LEAVEFBAR(){}
void WorkItem::ExecuteInst_RELEASEFBAR(){}
void WorkItem::ExecuteInst_WAITFBAR(){}
void WorkItem::ExecuteInst_LDF(){}
void WorkItem::ExecuteInst_SYNC(){}
void WorkItem::ExecuteInst_COUNTLANE(){}
void WorkItem::ExecuteInst_COUNTUPLANE(){}
void WorkItem::ExecuteInst_MASKLANE(){}
void WorkItem::ExecuteInst_SENDLANE(){}
void WorkItem::ExecuteInst_RECEIVELANE(){}
void WorkItem::ExecuteInst_CALL(){}
void WorkItem::ExecuteInst_RET(){}
void WorkItem::ExecuteInst_SYSCALL(){}
void WorkItem::ExecuteInst_ALLOCA(){}
void WorkItem::ExecuteInst_CLEARDETECTEXCEPT(){}
void WorkItem::ExecuteInst_CLOCK(){}
void WorkItem::ExecuteInst_CUID(){}
void WorkItem::ExecuteInst_CURRENTWORKGROUPSIZE(){}
void WorkItem::ExecuteInst_DEBUGTRAP(){}
void WorkItem::ExecuteInst_DIM(){}
void WorkItem::ExecuteInst_DISPATCHID(){}
void WorkItem::ExecuteInst_DISPATCHPTR(){}
void WorkItem::ExecuteInst_GETDETECTEXCEPT(){}
void WorkItem::ExecuteInst_GRIDGROUPS(){}
void WorkItem::ExecuteInst_GRIDSIZE(){}
void WorkItem::ExecuteInst_LANEID(){}
void WorkItem::ExecuteInst_MAXCUID(){}
void WorkItem::ExecuteInst_MAXWAVEID(){}
void WorkItem::ExecuteInst_NULLPTR(){}
void WorkItem::ExecuteInst_QID(){}
void WorkItem::ExecuteInst_QPTR(){}
void WorkItem::ExecuteInst_SETDETECTEXCEPT(){}
void WorkItem::ExecuteInst_WAVEID(){}
void WorkItem::ExecuteInst_WORKGROUPID(){}
void WorkItem::ExecuteInst_WORKGROUPSIZE(){}
void WorkItem::ExecuteInst_WORKITEMABSID(){}
void WorkItem::ExecuteInst_WORKITEMFLATABSID(){}
void WorkItem::ExecuteInst_WORKITEMFLATID(){}
void WorkItem::ExecuteInst_WORKITEMID(){}


} // namespace HSA