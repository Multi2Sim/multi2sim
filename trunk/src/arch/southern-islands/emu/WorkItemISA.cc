/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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
 *  along with self program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cassert>
#include <limits>
#include <math.h>
#include <lib/cpp/Misc.h>

#include "Emu.h"
#include "NDRange.h"
#include "Wavefront.h"
#include "WorkGroup.h"
#include "WorkItem.h"


using namespace misc;

namespace SI
{

// Macros for instruction format interpretation
#define INST_SMRD  inst->getBytes()->smrd
#define INST_SOPP  inst->getBytes()->sopp
#define INST_SOPK  inst->getBytes()->sopk
#define INST_SOPC  inst->getBytes()->sopc
#define INST_SOP1  inst->getBytes()->sop1
#define INST_SOP2  inst->getBytes()->sop2
#define INST_VOP1  inst->getBytes()->vop1
#define INST_VOP2  inst->getBytes()->vop2
#define INST_VOPC  inst->getBytes()->vopc
#define INST_VOP3b  inst->getBytes()->vop3b
#define INST_VOP3a  inst->getBytes()->vop3a
#define INST_VINTRP  inst->getBytes()->vintrp
#define INST_DS  inst->getBytes()->ds
#define INST_MTBUF  inst->getBytes()->mtbuf
#define INST_MUBUF  inst->getBytes()->mubuf
#define INST_EXP  inst->getBytes()->exp

// Macros for special registers
#define SI_M0 124
#define SI_VCC 106
#define SI_VCCZ 251
#define SI_EXEC 126
#define SI_EXECZ 252
#define SI_SCC 253

const char *err_si_isa_note =
	"\tThe AMD Southern Islands instruction set is partially supported by\n"
	"\tMulti2Sim. If your program is using an unimplemented instruction,\n"
	"\tplease email development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
		inst->getName(), err_si_isa_note)


// FIXME: Move to somewhere else

/* 
 * Float32 <-> Float16
 * Reference: http://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion 
 */

union Bits
{
	float f;
	int32_t si;
	uint32_t ui;
};

#define F_shift 13
#define F_shiftSign 16

#define F_infN 0x7F800000 // flt32 infinity
#define F_maxN 0x477FE000 // max flt16 normal as a flt32
#define F_minN 0x38800000 // min flt16 normal as a flt32
#define F_signN 0x80000000 // flt32 sign bit

#define F_infC (F_infN >> F_shift)
#define F_nanN ((F_infC + 1) << F_shift) // minimum flt16 nan as a flt32
#define F_maxC (F_maxN >> F_shift)
#define F_minC (F_minN >> F_shift)
#define F_signC (F_signN >> F_shiftSign) // flt16 sign bit

#define F_mulN 0x52000000 // (1 << 23) / F_minN
#define F_mulC 0x33800000 // F_minN / (1 << (23 - F_shift))

#define F_subC 0x003FF // max flt32 subnormal down shifted
#define F_norC 0x00400 // min flt32 normal down shifted

#define F_maxD (F_infC - F_maxC - 1)
#define F_minD (F_minC - F_subC - 1)

union hfpack
{
	uint32_t as_uint32;
	struct
	{
		uint16_t s1f;
		uint16_t s0f;
	} as_f16f16;
};

uint16_t Float32to16(float value)
{
	union Bits v, s;
	v.f = value;
	uint32_t sign = v.si & F_signN;
	v.si ^= sign;
	sign >>= F_shiftSign; // logical F_shift
	s.si = F_mulN;
	s.si = s.f * v.f; // correct subnormals
	v.si ^= (s.si ^ v.si) & -(F_minN > v.si);
	v.si ^= (F_infN ^ v.si) & -((F_infN > v.si) & (v.si > F_maxN));
	v.si ^= (F_nanN ^ v.si) & -((F_nanN > v.si) & (v.si > F_infN));
	v.ui >>= F_shift; // logical F_shift
	v.si ^= ((v.si - F_maxD) ^ v.si) & -(v.si > F_maxC);
	v.si ^= ((v.si - F_minD) ^ v.si) & -(v.si > F_subC);
	return v.ui | sign;
}

float Float16to32(uint16_t value)
{
	union Bits v;
	v.ui = value;
	int32_t sign = v.si & F_signC;
	v.si ^= sign;
	sign <<= F_shiftSign;
	v.si ^= ((v.si + F_minD) ^ v.si) & -(v.si > F_subC);
	v.si ^= ((v.si + F_maxD) ^ v.si) & -(v.si > F_maxC);
	union Bits s;
	s.si = F_mulC;
	s.f *= v.si;
	int32_t mask = -(F_norC > v.si);
	v.si <<= F_shift;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;
	return v.f;
}


/*
 * SMRD
 */

#define INST INST_SMRD
void WorkItem::ISA_S_BUFFER_LOAD_DWORD_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();
	int sbase = INST.sbase << 1;

	// sbase holds the first of 4 registers containing the buffer
	// resource descriptor
	EmuBufferDesc buf_desc;
	ReadBufferResource(sbase, buf_desc);

	// Calculate effective address
	unsigned m_base = buf_desc.base_addr;
	unsigned m_offset = INST.imm ? INST.offset * 4 : ReadSReg(INST.offset);
	unsigned addr = m_base + m_offset;

	// Read value from global memory
	InstReg value;
	global_mem->Read(addr, 4, (char *)&value);

	// Store the data in the destination register
	WriteSReg(INST.sdst, value.as_uint);

	// Debug
	if (Emu::debug)
		Emu::debug << StringFmt("wf%d: S%u<=(%u)(%u,%gf)", 
				wavefront->getId(), INST.sdst, addr, 
				value.as_uint, value.as_float);

	// Record last memory access for timing simulation purposes
	global_mem_access_addr = addr;
	global_mem_access_size = 4;
}
#undef INST

#define INST INST_SMRD
void WorkItem::ISA_S_BUFFER_LOAD_DWORDX2_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();
	int sbase = INST.sbase << 1;

	EmuMemPtr mem_ptr;
	ReadMemPtr(sbase, mem_ptr);

	// Calculate effective address
	unsigned m_base = mem_ptr.addr;
	unsigned m_offset = (INST.imm) ? (INST.offset * 4) : ReadSReg(INST.offset);
	unsigned addr = m_base + m_offset;

	int i;
	InstReg value[2];
	for (i = 0; i < 2; i++)
	{
		// Read value from global memory
		global_mem->Read(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(INST.sdst + i, value[i].as_uint);
	}

	// Debug
	if (Emu::debug)
	{
		Emu::debug << StringFmt("wf%d: ", wavefront->getId());
		for (i = 0; i < 2; i++)
		{
			Emu::debug << StringFmt("S%u<=(%u)(%u,%gf) ", INST.sdst + i, 
				addr + i * 4, value[i].as_uint, 
				value[i].as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = 4 * 2;
}
#undef INST


#define INST INST_SMRD
void WorkItem::ISA_S_BUFFER_LOAD_DWORDX4_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();
	int sbase = INST.sbase << 1;

	EmuMemPtr mem_ptr;
	ReadMemPtr(sbase, mem_ptr);

	// Calculate effective address
	unsigned m_base = mem_ptr.addr;
	unsigned m_offset = (INST.imm) ? (INST.offset * 4) : ReadSReg(INST.offset);
	unsigned addr = m_base + m_offset;

	int i;
	InstReg value[4];
	for (i = 0; i < 4; i++)
	{
		// Read value from global memory
		global_mem->Read(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(INST.sdst + i, value[i].as_uint);
	}

	// Debug
	if (Emu::debug)
	{
		Emu::debug << StringFmt("wf%d: ", wavefront->getId());
		for (i = 0; i < 4; i++)
		{
			Emu::debug << StringFmt("S%u<=(%u)(%u,%gf) ", INST.sdst + i, 
				addr + i*4, value[i].as_uint, 
				value[i].as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = 4 * 4;
}
#undef INST

#define INST INST_SMRD
void WorkItem::ISA_S_BUFFER_LOAD_DWORDX8_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();
	int sbase = INST.sbase << 1;

	EmuMemPtr mem_ptr;
	ReadMemPtr(sbase, mem_ptr);

	// Calculate effective address
	unsigned m_base = mem_ptr.addr;
	unsigned m_offset = (INST.imm) ? (INST.offset * 4) : ReadSReg(INST.offset);
	unsigned addr = m_base + m_offset;

	int i;
	InstReg value[8];
	for (i = 0; i < 8; i++)
	{
		// Read value from global memory
		global_mem->Read(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(INST.sdst + i, value[i].as_uint);
	}

	// Debug
	if (Emu::debug)
	{
		Emu::debug << StringFmt("wf%d: ", wavefront->getId());
		for (i = 0; i < 8; i++)
		{
			Emu::debug << StringFmt("S%u<=(%u)(%u,%gf) ", INST.sdst + i, 
				addr + i*4, value[i].as_uint, 
				value[i].as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = 4 * 8;
}
#undef INST

#define INST INST_SMRD
void WorkItem::ISA_S_BUFFER_LOAD_DWORDX16_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();
	int sbase = INST.sbase << 1;

	EmuMemPtr mem_ptr;
	ReadMemPtr(sbase, mem_ptr);

	// Calculate effective address
	unsigned m_base = mem_ptr.addr;
	unsigned m_offset = (INST.imm) ? (INST.offset * 4) : ReadSReg(INST.offset);
	unsigned addr = m_base + m_offset;

	int i;
	InstReg value[16];
	for (i = 0; i < 16; i++)
	{
		// Read value from global memory
		global_mem->Read(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(INST.sdst + i, value[i].as_uint);
	}

	// Debug
	if (Emu::debug)
	{
		Emu::debug << StringFmt("wf%d: ", wavefront->getId());
		for (i = 0; i < 16; i++)
		{
			Emu::debug << StringFmt("S%u<=(%u)(%u,%gf) ", INST.sdst + i, 
				addr + i*4, value[i].as_uint, 
				value[i].as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = 4 * 16;
}
#undef INST

#define INST INST_SMRD
void WorkItem::ISA_S_LOAD_DWORDX2_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();

	assert(INST.imm);

	int sbase = INST.sbase << 1;

	EmuMemPtr mem_ptr;
	ReadMemPtr(sbase, mem_ptr);

	// Calculate effective address
	unsigned m_base = mem_ptr.addr;
	unsigned m_offset = INST.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	int i;
	InstReg value[2];
	for (i = 0; i < 2; i++)
	{
		// Read value from global memory		
		global_mem->Read(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(INST.sdst + i, value[i].as_uint);
	}

	// Debug
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S[%u,%u] <= (addr %u): ", INST.sdst, INST.sdst+3, 
			m_addr);
		for (i = 0; i < 2; i++)
		{
			Emu::debug << StringFmt("S%u<=(%u,%gf) ", INST.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = m_addr;
	global_mem_access_size = 4 * 2;
}

#define INST INST_SMRD
void WorkItem::ISA_S_LOAD_DWORDX4_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();

	assert(INST.imm);

	int sbase = INST.sbase << 1;

	EmuMemPtr mem_ptr;
	ReadMemPtr(sbase, mem_ptr);

	// Calculate effective address
	unsigned m_base = mem_ptr.addr;
	unsigned m_offset = INST.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	int i;
	InstReg value[4];
	for (i = 0; i < 4; i++)
	{
		// Read value from global memory		
		global_mem->Read(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(INST.sdst + i, value[i].as_uint);
	}

	// Debug
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S[%u,%u] <= (addr %u): ", INST.sdst, INST.sdst+3, 
			m_addr);
		for (i = 0; i < 4; i++)
		{
			Emu::debug << StringFmt("S%u<=(%u,%gf) ", INST.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = m_addr;
	global_mem_access_size = 4 * 4;
}
#undef INST

#define INST INST_SMRD
void WorkItem::ISA_S_LOAD_DWORDX8_Impl(Inst *inst)
{
	// Record access
	wavefront->setScalarMemRead();

	assert(INST.imm);

	int sbase = INST.sbase << 1;

	EmuMemPtr mem_ptr;
	ReadMemPtr(sbase, mem_ptr);

	// Calculate effective address
	unsigned m_base = mem_ptr.addr;
	unsigned m_offset = INST.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	int i;
	InstReg value[8];
	for (i = 0; i < 8; i++)
	{
		// Read value from global memory		
		global_mem->Read(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(INST.sdst + i, value[i].as_uint);
	}

	// Debug
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S[%u,%u] <= (addr %u): ", INST.sdst, INST.sdst+3, 
			m_addr);
		for (i = 0; i < 8; i++)
		{
			Emu::debug << StringFmt("S%u<=(%u,%gf) ", INST.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = m_addr;
	global_mem_access_size = 4 * 8;
}
#undef INST


/*
 * SOP2
 */

// D.u = S0.u + S1.u. SCC = carry out.
#define INST INST_SOP2
void WorkItem::ISA_S_ADD_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg sum;
	InstReg carry;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Calculate the sum and carry out.
	sum.as_uint = s0.as_uint + s1.as_uint;
	carry.as_uint = ((unsigned long long) s0.as_uint + 
		(unsigned long long) s1.as_uint) >> 32;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, sum.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, carry.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%u) ", INST.sdst, sum.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", carry.as_uint);
	}
}
#undef INST


// D.u = S0.i + S1.i. scc = overflow.
#define INST INST_SOP2
void WorkItem::ISA_S_ADD_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg sum;
	InstReg ovf;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Calculate the sum and overflow.
	sum.as_int = s0.as_int + s1.as_int;
	ovf.as_uint = (s0.as_int >> 31 != s1.as_int >> 31) ? 0 : 
		((s0.as_int > 0 && sum.as_int < 0) || 
		(s0.as_int < 0 && sum.as_int > 0));

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, sum.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, ovf.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%u) ", INST.sdst, sum.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", ovf.as_uint);
	}
}
#undef INST

// D.u = S0.i - S1.i. scc = overflow.
#define INST INST_SOP2
void WorkItem::ISA_S_SUB_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg diff;
	InstReg ovf;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Calculate the sum and overflow.
	diff.as_int = s0.as_int - s1.as_int;
	ovf.as_uint = (s0.as_int >> 31 != s1.as_int >> 31) ? 
		((s0.as_int > 0 && diff.as_int < 0) ||
		(s0.as_int < 0 && diff.as_int > 0)) : 0;

	// Write the results.
		// Store the data in the destination register
	WriteSReg(INST.sdst, diff.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, ovf.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%d) ", INST.sdst, diff.as_int);
		Emu::debug << StringFmt("scc<=(%u) ", ovf.as_uint);
	}
}
#undef INST

// D.u = (S0.u < S1.u) ? S0.u : S1.u, scc = 1 if S0 is min.
#define INST INST_SOP2
void WorkItem::ISA_S_MIN_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg min;
	InstReg s0_min;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Calculate the minimum operand.
	if (s0.as_uint < s1.as_uint)
	{
		min.as_uint = s0.as_uint;
		s0_min.as_uint = 1;
	}
	else
	{
		min.as_uint = s1.as_uint;
		s0_min.as_uint = 0;
	}

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, min.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, s0_min.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%u) ", INST.sdst, min.as_uint);
		Emu::debug << StringFmt("scc<=(%d) ", s0_min.as_uint);
	}
}
#undef INST

// D.i = (S0.i > S1.i) ? S0.i : S1.i, scc = 1 if S0 is max.
#define INST INST_SOP2
void WorkItem::ISA_S_MAX_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg max;
	InstReg s0_max;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Calculate the maximum operand.
	// Is max defined as GT or GTE?
	if (s0.as_int > s1.as_int)
	{
		max.as_int = s0.as_int;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_int = s1.as_int;
		s0_max.as_uint = 0;
	}

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, max.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, s0_max.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%d) ", INST.sdst, max.as_int);
		Emu::debug << StringFmt("scc<=(%u) ", s0_max.as_uint);
	}
}
#undef INST

// D.u = (S0.u > S1.u) ? S0.u : S1.u, scc = 1 if S0 is max.
#define INST INST_SOP2
void WorkItem::ISA_S_MAX_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg max;
	InstReg s0_max;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Calculate the maximum operand.
	if (s0.as_uint > s1.as_uint)
	{
		max.as_uint = s0.as_uint;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_uint = s1.as_uint;
		s0_max.as_uint = 0;
	}

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, max.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, s0_max.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%u) ", INST.sdst, max.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", s0_max.as_uint);
	}
}
#undef INST

// D.u = SCC ? S0.u : S1.u
#define INST INST_SOP2
void WorkItem::ISA_S_CSELECT_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg scc;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);
	scc.as_uint = ReadSReg(SI_SCC);

	// Calculate the result
	result.as_uint = scc.as_uint ? s0.as_uint : s1.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result.as_uint);
	}
}
#undef INST

// D.u = S0.u & S1.u. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_AND_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;
	InstReg nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0.as_uint & s1.as_uint;
	nonzero.as_uint = ! !result.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

// D.u = S0.u & S1.u. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_AND_B64_Impl(Inst *inst)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	InstReg s0_lo;
	InstReg s0_hi;
	InstReg s1_lo;
	InstReg s1_hi;
	InstReg result_lo;
	InstReg result_hi;
	InstReg nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(INST.ssrc1);
	s1_hi.as_uint = ReadSReg(INST.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint & s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint & s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

// D.u = S0.u | S1.u. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_OR_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;
	InstReg nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0.as_uint | s1.as_uint;
	nonzero.as_uint = ! !result.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

// D.u = S0.u | S1.u. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_OR_B64_Impl(Inst *inst)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	InstReg s0_lo;
	InstReg s0_hi;
	InstReg s1_lo;
	InstReg s1_hi;
	InstReg result_lo;
	InstReg result_hi;
	InstReg nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(INST.ssrc1);
	s1_hi.as_uint = ReadSReg(INST.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint | s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint | s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

// D.u = S0.u ^ S1.u. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_XOR_B64_Impl(Inst *inst)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	InstReg s0_lo;
	InstReg s0_hi;
	InstReg s1_lo;
	InstReg s1_hi;
	InstReg result_lo;
	InstReg result_hi;
	InstReg nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(INST.ssrc1);
	s1_hi.as_uint = ReadSReg(INST.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint ^ s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint ^ s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

// D.u = S0.u & ~S1.u. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_ANDN2_B64_Impl(Inst *inst)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	InstReg s0_lo;
	InstReg s0_hi;
	InstReg s1_lo;
	InstReg s1_hi;
	InstReg result_lo;
	InstReg result_hi;
	InstReg nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(INST.ssrc1);
	s1_hi.as_uint = ReadSReg(INST.ssrc1 + 1);

	/* Bitwise AND the first operand with the negation of the second and
	 * determine if the result is non-zero. */
	result_lo.as_uint = s0_lo.as_uint & ~s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint & ~s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		Emu::debug << StringFmt("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

// D.u = ~(S0.u & S1.u). scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_NAND_B64_Impl(Inst *inst)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	InstReg s0_lo;
	InstReg s0_hi;
	InstReg s1_lo;
	InstReg s1_hi;
	InstReg result_lo;
	InstReg result_hi;
	InstReg nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(INST.ssrc1);
	s1_hi.as_uint = ReadSReg(INST.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = ~(s0_lo.as_uint & s1_lo.as_uint);
	result_hi.as_uint = ~(s0_hi.as_uint & s1_hi.as_uint);
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		Emu::debug << StringFmt("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

// D.u = S0.u << S1.u[4:0]. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_LSHL_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;
	InstReg nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = ReadSReg(INST.ssrc0);
	}
	if (INST.ssrc1 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s1.as_uint = INST.lit_cnst;
	}
	else
	{
		s1.as_uint = ReadSReg(INST.ssrc1) & 0x1F;
	}

	/* Left shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0.as_uint << s1.as_uint;
	nonzero.as_uint = result.as_uint != 0;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		Emu::debug << StringFmt("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

// D.u = S0.u >> S1.u[4:0]. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_LSHR_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;
	InstReg nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = ReadSReg(INST.ssrc0);
	}
	if (INST.ssrc1 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s1.as_uint = INST.lit_cnst;
	}
	else
	{
		s1.as_uint = ReadSReg(INST.ssrc1) & 0x1F;
	}

	/* Right shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0.as_uint >> s1.as_uint;
	nonzero.as_uint = result.as_uint != 0;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		Emu::debug << StringFmt("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

// D.i = signext(S0.i) >> S1.i[4:0]. scc = 1 if result is non-zero.
#define INST INST_SOP2
void WorkItem::ISA_S_ASHR_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;
	InstReg nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = ReadSReg(INST.ssrc0);
	}
	if (INST.ssrc1 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s1.as_uint = INST.lit_cnst;
	}
	else
	{
		s1.as_uint = ReadSReg(INST.ssrc1) & 0x1F;
	}

	/* Right shift the first operand sign extended by the second and
	 * determine if the result is non-zero. */
	result.as_int = s0.as_int >> s1.as_int;
	nonzero.as_uint = result.as_uint != 0;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%d) ", INST.sdst, result.as_int);
		Emu::debug << StringFmt("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

// D.i = S0.i * S1.i.
#define INST INST_SOP2
void WorkItem::ISA_S_MUL_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Multiply the two operands.
	result.as_int = s0.as_int * s1.as_int;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%d)", INST.sdst, result.as_int);
	}
}
#undef INST

/* D.i = (S0.i >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST INST_SOP2
void WorkItem::ISA_S_BFE_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg result;
	InstReg full_reg;
	InstReg nonzero;

	// Load operands from registers.
	s0.as_uint = ReadSReg(INST.ssrc0);
	full_reg.as_uint = ReadSReg(INST.ssrc1);

	/* s1 (offset) should be [4:0] of ssrc1 and s2 (width) should be [22:16] of ssrc1*/
	s1.as_uint = full_reg.as_uint & 0x1F;
	s2.as_uint = (full_reg.as_uint >> 16) & 0x7F;

	// Calculate the result.
	if (s2.as_uint == 0)
	{
		result.as_int = 0;
	}
	else if (s2.as_uint + s1.as_uint < 32)
	{
		result.as_int = (s0.as_int << (32 - s1.as_uint - s2.as_uint)) >> 
			(32 - s2.as_uint);
	}
	else
	{
		result.as_int = s0.as_int >> s1.as_uint;
	}

	nonzero.as_uint = result.as_uint != 0;
	
	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);


	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		Emu::debug << StringFmt("scc<=(%u)", nonzero.as_uint);
	}
	
}
#undef INST



/*
 * SOPK
 */

// D.i = signext(simm16).
#define INST INST_SOPK
void WorkItem::ISA_S_MOVK_I32_Impl(Inst *inst)
{
	InstReg simm16;
	InstReg result;

	// Load constant operand from instruction.
	simm16.as_ushort[0] = INST.simm16;

	// Sign extend the short constant to an integer.
	result.as_int = (int) simm16.as_short[0];

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%d)", INST.sdst, result.as_int);
	}
}
#undef INST

//
#define INST INST_SOPK
void WorkItem::ISA_S_CMPK_LE_U32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.i = D.i + signext(SIMM16). scc = overflow.
#define INST INST_SOPK
void WorkItem::ISA_S_ADDK_I32_Impl(Inst *inst)
{
	InstReg simm16;
	InstReg sum;
	InstReg ovf;
	InstReg dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = INST.simm16;
	se_simm16 = (int) simm16.as_short[0];

	// Load operand from destination register.
	dst.as_uint = ReadSReg(INST.sdst);

	// Add the two operands and determine overflow.
	sum.as_int = dst.as_int + se_simm16;
	ovf.as_uint = (dst.as_int >> 31 != se_simm16 >> 31) ? 0 :
		((dst.as_int > 0 && sum.as_int < 0) || 
		 (dst.as_int < 0 && sum.as_int > 0));

	// Write the results.
		// Store the data in the destination register
	WriteSReg(INST.sdst, sum.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, ovf.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%d)", INST.sdst, sum.as_int);
		Emu::debug << StringFmt("scc<=(%u)", ovf.as_uint);
	}
}
#undef INST

// D.i = D.i * signext(SIMM16). scc = overflow.
#define INST INST_SOPK
void WorkItem::ISA_S_MULK_I32_Impl(Inst *inst)
{
	InstReg simm16;
	InstReg product;
	InstReg ovf;
	InstReg dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = INST.simm16;
	se_simm16 = (int) simm16.as_short[0];

	// Load operand from destination register.
	dst.as_uint = ReadSReg(INST.sdst);

	// Multiply the two operands and determine overflow.
	product.as_int = dst.as_int * se_simm16;
	ovf.as_uint = ((long long) dst.as_int * (long long) se_simm16) > 
		(long long) product.as_int;

	// Write the results.
		// Store the data in the destination register
	WriteSReg(INST.sdst, product.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, ovf.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(%d)", INST.sdst, product.as_int);
		Emu::debug << StringFmt("scc<=(%u)", ovf.as_uint);
	}
}
#undef INST

// D.u = S0.u.
#define INST INST_SOP1
void WorkItem::ISA_S_MOV_B64_Impl(Inst *inst)
{
	// Assert no literal constant with a 64 bit instruction.
	assert(!(INST.ssrc0 == 0xFF));

	InstReg s0_lo;
	InstReg s0_hi;

	// Load operand from registers.
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, s0_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, s0_hi.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, s0_lo.as_uint);
		Emu::debug << StringFmt("S%u<=(0x%x)", INST.sdst + 1, s0_hi.as_uint);
	}
}
#undef INST

/*
 * SOP1
 */

// D.u = S0.u.
#define INST INST_SOP1
void WorkItem::ISA_S_MOV_B32_Impl(Inst *inst)
{
	InstReg s0;

	// Load operand from registers or as a literal constant.
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, s0.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, s0.as_uint);
	}
}
#undef INST

// D.u = ~S0.u SCC = 1 if result non-zero.
#define INST INST_SOP1
void WorkItem::ISA_S_NOT_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg nonzero;

	// Load operand from registers or as a literal constant.
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = ~INST.lit_cnst;
	else
		s0.as_uint = ~ReadSReg(INST.ssrc0);
	nonzero.as_uint = ! !s0.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, s0.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, s0.as_uint);
	}
}
#undef INST

#define INST INST_SOP1
// D.u = WholeQuadMode(S0.u). SCC = 1 if result is non-zero.
void WorkItem::ISA_S_WQM_B64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.u = PC + 4, PC = S0.u
#define INST INST_SOP1
void WorkItem::ISA_S_SWAPPC_B64_Impl(Inst *inst)
{
	unsigned pc;
	InstReg s0_lo;
	InstReg s0_hi;

	// FIXME: cuurently PC is implemented as 32-bit offset
	// Load operands from registers
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);

	// Write the results
	// Store the data in the destination register
	WriteSReg(INST.sdst, wavefront->getPC() + 4);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, 0);

	// Set the new PC
	pc = wavefront->getPC();
	wavefront->setPC(s0_lo.as_uint - 4);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, pc + 4);
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst + 1, s0_hi.as_uint);
		Emu::debug << StringFmt("PC<=(0x%x)", wavefront->getPC());
	}
}
#undef INST

/* D.u = EXEC, EXEC = S0.u & EXEC. scc = 1 if the new value of EXEC is
 * non-zero. */
#define INST INST_SOP1
void WorkItem::ISA_S_AND_SAVEEXEC_B64_Impl(Inst *inst)
{
	// Assert no literal constant with a 64 bit instruction.
	assert(!(INST.ssrc0 == 0xFF));

	InstReg exec_lo;
	InstReg exec_hi;
	InstReg s0_lo;
	InstReg s0_hi;
	InstReg exec_new_lo;
	InstReg exec_new_hi;
	InstReg nonzero;

	// Load operands from registers.
	exec_lo.as_uint = ReadSReg(SI_EXEC);
	exec_hi.as_uint = ReadSReg(SI_EXEC + 1);
	s0_lo.as_uint = ReadSReg(INST.ssrc0);
	s0_hi.as_uint = ReadSReg(INST.ssrc0 + 1);

	/* Bitwise AND exec and the first operand and determine if the result 
	 * is non-zero. */
	exec_new_lo.as_uint = s0_lo.as_uint & exec_lo.as_uint;
	exec_new_hi.as_uint = s0_hi.as_uint & exec_hi.as_uint;
	nonzero.as_uint = exec_new_lo.as_uint || exec_new_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.sdst, exec_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(INST.sdst + 1, exec_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_EXEC, exec_new_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_EXEC + 1, exec_new_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(SI_SCC, nonzero.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst, exec_lo.as_uint);
		Emu::debug << StringFmt("S%u<=(0x%x) ", INST.sdst + 1, exec_hi.as_uint);
		Emu::debug << StringFmt("exec_lo<=(0x%x) ", exec_new_lo.as_uint);
		Emu::debug << StringFmt("exec_hi<=(0x%x) ", exec_new_hi.as_uint);
		Emu::debug << StringFmt("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

/*
 * SOPC
 */

// scc = (S0.i == S1.i).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_EQ_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("wf%d: scc<=(%u) (%u ==? %u)", 
			wavefront->getId(), result.as_uint, s0.as_int,
			s1.as_int);
	}
}
#undef INST

// scc = (S0.i > S1.i).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_GT_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("scc<=(%u) (%u >? %u) ", result.as_uint,
			s0.as_uint, s1.as_uint);
	}
}
#undef INST

// scc = (S0.i >= S1.i).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_GE_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

// scc = (S0.i < S1.i).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_LT_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

// scc = (S0.i <= S1.i).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_LE_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

// scc = (S0.u > S1.u).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_GT_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

// scc = (S0.u >= S1.u).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_GE_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint >= s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

// scc = (S0.u <= S1.u).
#define INST INST_SOPC
void WorkItem::ISA_S_CMP_LE_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadSReg(INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = ReadSReg(INST.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(SI_SCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/*
 * SOPP
 */

// End the program.
void WorkItem::ISA_S_ENDPGM_Impl(Inst *inst)
{
	wavefront->setFinished();
	work_group->incWavefrontsCompletedEmu();
}

// PC = PC + signext(SIMM16 * 4) + 4
#define INST INST_SOPP
void WorkItem::ISA_S_BRANCH_Impl(Inst *inst)
{
	short simm16;
	int se_simm16;

	// Load the short constant operand and sign extend into an integer.
	simm16 = INST.simm16;
	se_simm16 = simm16;

	// Relative jump
	wavefront->incPC(se_simm16 * 4 + 4 - inst->getSize());
}
#undef INST

// if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
#define INST INST_SOPP
void WorkItem::ISA_S_CBRANCH_SCC0_Impl(Inst *inst)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(SI_SCC))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		wavefront->incPC(
			se_simm16 * 4 + 4 - inst->getSize());
	}
}
#undef INST


// if(SCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
#define INST INST_SOPP
void WorkItem::ISA_S_CBRANCH_SCC1_Impl(Inst *inst)
{
	short simm16;
	int se_simm16;

	InstReg scc;

	scc.as_uint = ReadSReg(SI_SCC);

	if (scc.as_uint)
	{
		assert(ReadSReg(SI_SCC) == 1);

		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		wavefront->incPC(
			se_simm16 * 4 + 4 - inst->getSize());

		// Print isa debug information.
		if (Emu::debug)
		{
			Emu::debug << StringFmt("wf%d: SCC=%u (taken)", 
				wavefront->getId(), scc.as_uint);
		}
	}
	else
	{
		// Print isa debug information.
		if (Emu::debug)
		{
			Emu::debug << StringFmt("wf%d: SCC=%u (not taken)", 
				wavefront->getId(), scc.as_uint);
		}
	}
}
#undef INST

// if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
#define INST INST_SOPP
void WorkItem::ISA_S_CBRANCH_VCCZ_Impl(Inst *inst)
{
	short simm16;
	int se_simm16;

	if (ReadSReg(SI_VCCZ))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		wavefront->incPC(
			se_simm16 * 4 + 4 - inst->getSize());
	}
}
#undef INST

// if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
#define INST INST_SOPP
void WorkItem::ISA_S_CBRANCH_VCCNZ_Impl(Inst *inst)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(SI_VCCZ))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		wavefront->incPC(
			se_simm16 * 4 + 4 - inst->getSize());
	}
}
#undef INST

// if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
#define INST INST_SOPP
void WorkItem::ISA_S_CBRANCH_EXECZ_Impl(Inst *inst)
{
	short simm16;
	int se_simm16;

	InstReg exec;
	InstReg execz;

	exec.as_uint = ReadSReg(SI_EXEC);
	execz.as_uint = ReadSReg(SI_EXECZ);

	if (execz.as_uint)
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		wavefront->incPC(
			se_simm16 * 4 + 4 - inst->getSize());

		// Print isa debug information.
		if (Emu::debug)
		{
			Emu::debug << StringFmt("wf%d: EXEC=0x%x, EXECZ=%u (taken)", 
				wavefront->getId(), exec.as_uint, 
				execz.as_uint);
		}
	}
	else
	{
		// Print isa debug information.
		if (Emu::debug)
		{
			Emu::debug << StringFmt("wf%d: EXEC=0x%x, EXECZ=%u " 
				"(not taken)", wavefront->getId(), 
				exec.as_uint, execz.as_uint);
		}
	}
}
#undef INST


// if(EXEC != 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
#define INST INST_SOPP
void WorkItem::ISA_S_CBRANCH_EXECNZ_Impl(Inst *inst)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(SI_EXECZ))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		wavefront->incPC(
			se_simm16 * 4 + 4 - inst->getSize());
	}
}
#undef INST

/* Suspend current wavefront at the barrier. If all wavefronts in work-group
 * reached the barrier, wake them up */
void WorkItem::ISA_S_BARRIER_Impl(Inst *inst)
{
	// Suspend current wavefront at the barrier
	wavefront->setBarrierInst();
	wavefront->setAtBarrier();
	work_group->incWavefrontsAtBarrier();

	Emu::debug << StringFmt("Group %d wavefront %d reached barrier "
		"(%d reached, %d left)\n",
		work_group->getId(), wavefront->getId(), 
		work_group->getWavefrontsAtBarrier(),
		work_group->getWavefrontsInWorkgroup() - 
		work_group->getWavefrontsAtBarrier());


	// If all wavefronts in work-group reached the barrier, wake them up
	if (work_group->getWavefrontsAtBarrier() == work_group->getWavefrontsInWorkgroup())
	{
		// FIXME: friend workitem in wavefront?
		// for( auto &wavefront : work_group->wavefronts)
		// {
		//		wavefront->unsetAtBarrier();
		// }

		work_group->setWavefrontsAtBarrier(0);

		Emu::debug << StringFmt("Group %d completed barrier\n", work_group->getId());
	}
}

void WorkItem::ISA_S_WAITCNT_Impl(Inst *inst)
{
	// Nothing to do in emulation
	wavefront->setMemWait();
}


/*
 * VOP1
 */

// Do nothing
#define INST INST_VOP1
void WorkItem::ISA_V_NOP_Impl(Inst *inst)
{
	// Do nothing
}
#undef INST

// D.u = S0.u.
#define INST INST_VOP1
void WorkItem::ISA_V_MOV_B32_Impl(Inst *inst)
{
	InstReg value;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		value.as_uint = INST.lit_cnst;
	else
		value.as_uint = ReadReg(INST.src0);

	// Write the results.
	WriteVReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) ", id, INST.vdst,
			value.as_uint);
	}
}
#undef INST

// Copy one VGPR value to one SGPR.
#define INST INST_VOP1
void WorkItem::ISA_V_READFIRSTLANE_B32_Impl(Inst *inst)
{
	InstReg value;

	// Load operand from register.
	assert(INST.src0 >= 256 || INST.src0 == SI_M0);
	value.as_uint = ReadReg(INST.src0);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) ", id, INST.vdst,
			value.as_uint);
	}
}
#undef INST

// D.i = (int)S0.d.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_I32_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.f = (double)S0.i.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_F64_I32_Impl(Inst *inst)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} value;
	InstReg s0;
	InstReg result_lo;
	InstReg result_hi;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	// Convert and separate value.
	value.as_double = (double) s0.as_int;

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(INST.vdst, result_lo.as_uint);
	WriteVReg(INST.vdst + 1, result_hi.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V[%u:+1]<=(%lgf) ", id,
			INST.vdst, value.as_double);
	}
}
#undef INST

// D.f = (float)S0.i.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_F32_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg value;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	value.as_float = (float) s0.as_int;

	// Write the results.
	WriteVReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			value.as_float);
	}
}
#undef INST

// D.f = (float)S0.u.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_F32_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg value;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	value.as_float = (float) s0.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			value.as_float);
	}
}
#undef INST

// D.i = (uint)S0.f.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_U32_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg value;

	float fvalue;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	fvalue = s0.as_float;

	// Handle special number cases and cast to an unsigned

	// -inf, NaN, 0, -0 --> 0
	if ((isinf(fvalue) && fvalue < 0.0f) || isnan(fvalue)
		|| fvalue == 0.0f || fvalue == -0.0f)
		value.as_uint = 0;
	// inf, > max_uint --> max_uint
	else if (isinf(fvalue) || fvalue >= std::numeric_limits<unsigned int>::max())
		value.as_uint = std::numeric_limits<unsigned int>::max();
	else
		value.as_uint = (unsigned) fvalue;

	// Write the results.
	WriteVReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u) ", id, INST.vdst,
			value.as_uint);
	}
}
#undef INST

// D.i = (int)S0.f.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_I32_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg value;

	float fvalue;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	fvalue = s0.as_float;

	// Handle special number cases and cast to an int

	// inf, > max_int --> max_int
	if ((isinf(fvalue) && fvalue > 0.0f) || fvalue >= std::numeric_limits<int>::max())
		value.as_int = std::numeric_limits<int>::max();
	// -inf, < -max_int --> -max_int
	else if (isinf(fvalue) || fvalue < std::numeric_limits<int>::min())
		value.as_int = std::numeric_limits<int>::min();
	// NaN, 0, -0 --> 0
	else if (isnan(fvalue) || fvalue == 0.0f || fvalue == -0.0f)
		value.as_int = 0;
	else
		value.as_int = (int) fvalue;

	// Write the results.
	WriteVReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d) ", id, INST.vdst,
			value.as_int);
	}
}
#undef INST

// D.f = (float)S0.d.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_F32_F64_Impl(Inst *inst)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} s0;
	InstReg value;

	assert(INST.src0 != 0xFF);

	// Load operand from registers.
	s0.as_reg[0] = ReadReg(INST.src0);
	s0.as_reg[1] = ReadReg(INST.src0 + 1);

	// Cast to a single precision float
	value.as_float = (float) s0.as_double;

	// Write the results.
	WriteVReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			value.as_float);
	}
}
#undef INST


// D.d = (double)S0.f.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_F64_F32_Impl(Inst *inst)
{
	InstReg s0;
	union
	{
		double as_double;
		unsigned as_reg[2];

	} value;
	InstReg value_lo;
	InstReg value_hi;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	// Cast to a single precision float
	value.as_double = (double) s0.as_float;

	// Write the results.
	value_lo.as_uint = value.as_reg[0];
	value_hi.as_uint = value.as_reg[1];
	WriteVReg(INST.vdst, value_lo.as_uint);
	WriteVReg(INST.vdst + 1, value_hi.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V[%u:+1]<=(%lgf) ", id,
			INST.vdst, value.as_double);
	}
}
#undef INST

// D.d = (double)S0.u.
#define INST INST_VOP1
void WorkItem::ISA_V_CVT_F64_U32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.f = trunc(S0.f), return integer part of S0.
#define INST INST_VOP1
void WorkItem::ISA_V_TRUNC_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg value;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	// Truncate decimal portion
	value.as_float = (float)((int)s0.as_float);

	// Write the results.
	WriteVReg(INST.vdst, value.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			value.as_float);
	}
}
#undef INST

// D.f = trunc(S0); if ((S0 < 0.0) && (S0 != D)) D += -1.0.
#define INST INST_VOP1
void WorkItem::ISA_V_FLOOR_F32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.f = log2(S0.f).
#define INST INST_VOP1
void WorkItem::ISA_V_LOG_F32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.f = 1.0 / S0.f.
#define INST INST_VOP1
void WorkItem::ISA_V_RCP_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg rcp;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	rcp.as_float = 1.0f / s0.as_float;

	// Write the results.
	WriteVReg(INST.vdst, rcp.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			rcp.as_float);
	}
}
#undef INST

// D.d = 1.0 / (S0.d).
#define INST INST_VOP1
void WorkItem::ISA_V_RCP_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.f = 1.0 / sqrt(S0.f).
#define INST INST_VOP1
void WorkItem::ISA_V_RSQ_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.f = sqrt(S0.f).
#define INST INST_VOP1
void WorkItem::ISA_V_SQRT_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg srt;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	srt.as_float = sqrtf(s0.as_float);

	// Write the results.
	WriteVReg(INST.vdst, srt.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			srt.as_float);
	}
}
#undef INST

// D.f = sin(S0.f)
#define INST INST_VOP1
void WorkItem::ISA_V_SIN_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg result;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	// Normalize input
	// XXX Should it be module instead of dividing?
	s0.as_float = s0.as_float * (2 * M_PI);

	if (InRange(s0.as_float, -256, 256))
	{
		result.as_float = sinf(s0.as_float);
	}
	else
	{
		assert(0); // Haven't debugged this yet
		result.as_float = 0;
	}

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) (sin %gf) ", id, 
			INST.vdst, result.as_float, s0.as_float);
	}
}
#undef INST

// D.f = cos(S0.f)
#define INST INST_VOP1
void WorkItem::ISA_V_COS_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg result;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	// Normalize input
	// XXX Should it be module instead of dividing?
	s0.as_float = s0.as_float * (2 * M_PI);

	if (InRange(s0.as_float, -256, 256))
	{
		result.as_float = cosf(s0.as_float);
	}
	else
	{
		assert(0); // Haven't debugged this yet
		result.as_float = 1;
	}

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) (cos %gf) ", id, 
			INST.vdst, result.as_float, s0.as_float);
	}
}
#undef INST

// D.u = ~S0.u.
#define INST INST_VOP1
void WorkItem::ISA_V_NOT_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg result;

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);

	// Bitwise not
	result.as_uint = ~s0.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) ", id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = position of first 1 in S0 from MSB; D=0xFFFFFFFF if S0==0.
#define INST INST_VOP1
void WorkItem::ISA_V_FFBH_U32_Impl(Inst *inst)
{
	NOT_IMPL();
}

// D.d = FRAC64(S0.d);
#define INST INST_VOP1
void WorkItem::ISA_V_FRACT_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// VGPR[D.u + M0.u] = VGPR[S0.u].
#define INST INST_VOP1
void WorkItem::ISA_V_MOVRELD_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg m0;
	
	assert(INST.src0 != 0xFF);

	// Load operand from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	m0.as_uint = ReadReg(SI_M0);

	// Write the results.
	WriteVReg(INST.vdst+m0.as_uint, s0.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V[%u+%u]<=(0x%x) ", id, 
			INST.vdst, m0.as_uint, s0.as_uint);
	}
}
#undef INST


// VGPR[D.u] = VGPR[S0.u + M0.u].
#define INST INST_VOP1
void WorkItem::ISA_V_MOVRELS_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg m0;
	
	assert(INST.src0 != 0xFF);

	// Load operand from register or as a literal constant.
	m0.as_uint = ReadReg(SI_M0);
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0 + m0.as_uint);

	// Write the results.
	WriteVReg(INST.vdst, s0.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V[%u]<=(0x%x) ", id, 
			INST.vdst, s0.as_uint);
	}
}
#undef INST


/*
 * VOP2
 */

/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a
 * scalar GPR in S2. */
#define INST INST_VOP2
void WorkItem::ISA_V_CNDMASK_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	int vcci;

	// Load operands from register or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);
	vcci = ReadBitmaskSReg(SI_VCC);

	// Calculate the result.
	result.as_uint = (vcci) ? s1.as_uint : s0.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u) ", id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.f = S0.f + S1.f.
#define INST INST_VOP2
void WorkItem::ISA_V_ADD_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg sum;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the sum.
	sum.as_float = s0.as_float + s1.as_float;

	// Write the results.
	WriteVReg(INST.vdst, sum.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			sum.as_float);
	}
}
#undef INST

// D.f = S0.f - S1.f.
#define INST INST_VOP2
void WorkItem::ISA_V_SUB_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg dif;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the difference.
	dif.as_float = s0.as_float - s1.as_float;

	// Write the results.
	WriteVReg(INST.vdst, dif.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			dif.as_float);
	}
}
#undef INST

// D.f = S1.f - S0.f.
#define INST INST_VOP2
void WorkItem::ISA_V_SUBREV_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg dif;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the difference.
	dif.as_float = s1.as_float - s0.as_float;

	// Write the results.
	WriteVReg(INST.vdst, dif.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			dif.as_float);
	}
}
#undef INST

// D.f = S0.F * S1.f + D.f.
#define INST INST_VOP2
void WorkItem::ISA_V_MAC_LEGACY_F32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST


// D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0).
#define INST INST_VOP2
void WorkItem::ISA_V_MUL_LEGACY_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg product;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the product.
	if (s0.as_float == 0.0f || s1.as_float == 0.0f)
	{
		product.as_float = 0.0f;
	}
	else
	{
		product.as_float = s0.as_float * s1.as_float;
	}

	// Write the results.
	WriteVReg(INST.vdst, product.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) (%gf * %gf) ", id, 
			INST.vdst, product.as_float, s0.as_float, s1.as_float);
	}
}
#undef INST

// D.f = S0.f * S1.f.
#define INST INST_VOP2
void WorkItem::ISA_V_MUL_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg product;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the product.
	product.as_float = s0.as_float * s1.as_float;

	// Write the results.
	WriteVReg(INST.vdst, product.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) (%gf*%gf)", id, 
			INST.vdst, product.as_float, s0.as_float, s1.as_float);
	}
}
#undef INST

// D.i = S0.i[23:0] * S1.i[23:0].
#define INST INST_VOP2
void WorkItem::ISA_V_MUL_I32_I24_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg product;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Truncate operands to 24-bit signed integers
	s0.as_uint = SignExtend32(s0.as_uint, 24);
	s1.as_uint = SignExtend32(s1.as_uint, 24);

	// Calculate the product.
	product.as_int = s0.as_int * s1.as_int;

	// Write the results.
	WriteVReg(INST.vdst, product.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d)", id, INST.vdst,
			product.as_int);
	}
}
#undef INST

// D.f = min(S0.f, S1.f).
#define INST INST_VOP2
void WorkItem::ISA_V_MIN_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg min;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the minimum operand.
	if (s0.as_float < s1.as_float)
	{
		min.as_float = s0.as_float;
	}
	else
	{
		min.as_float = s1.as_float;
	}

	// Write the results.
	WriteVReg(INST.vdst, min.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf)", id, INST.vdst,
			min.as_float);
	}
}
#undef INST

// D.f = max(S0.f, S1.f).
#define INST INST_VOP2
void WorkItem::ISA_V_MAX_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg max;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the minimum operand.
	if (s0.as_float > s1.as_float)
	{
		max.as_float = s0.as_float;
	}
	else
	{
		max.as_float = s1.as_float;
	}

	// Write the results.
	WriteVReg(INST.vdst, max.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf)", id, INST.vdst,
			max.as_float);
	}
}
#undef INST

// D.i = max(S0.i, S1.i).
#define INST INST_VOP2
void WorkItem::ISA_V_MAX_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg max;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the minimum operand.
	if (s0.as_int > s1.as_int)
	{
		max.as_int = s0.as_int;
	}
	else
	{
		max.as_int = s1.as_int;
	}

	// Write the results.
	WriteVReg(INST.vdst, max.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d)", id, INST.vdst,
			max.as_int);
	}
}
#undef INST

// D.i = min(S0.i, S1.i).
#define INST INST_VOP2
void WorkItem::ISA_V_MIN_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg min;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the minimum operand.
	if (s0.as_int < s1.as_int)
	{
		min.as_int = s0.as_int;
	}
	else
	{
		min.as_int = s1.as_int;
	}

	// Write the results.
	WriteVReg(INST.vdst, min.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d)", id, INST.vdst,
			min.as_int);
	}
}
#undef INST

// D.u = min(S0.u, S1.u).
#define INST INST_VOP2
void WorkItem::ISA_V_MIN_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg min;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the minimum operand.
	if (s0.as_uint < s1.as_uint)
	{
		min.as_uint = s0.as_uint;
	}
	else
	{
		min.as_uint = s1.as_uint;
	}

	// Write the results.
	WriteVReg(INST.vdst, min.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)", id, INST.vdst,
			min.as_uint);
	}
}
#undef INST

// D.u = max(S0.u, S1.u).
#define INST INST_VOP2
void WorkItem::ISA_V_MAX_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg max;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the maximum operand.
	if (s0.as_uint > s1.as_uint)
	{
		max.as_uint = s0.as_uint;
	}
	else
	{
		max.as_uint = s1.as_uint;
	}

	// Write the results.
	WriteVReg(INST.vdst, max.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)", id, INST.vdst,
			max.as_uint);
	}
}
#undef INST

// D.u = S1.u >> S0.u[4:0].
#define INST INST_VOP2
void WorkItem::ISA_V_LSHRREV_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = ReadReg(INST.src0) & 0x1F;
	}
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Right shift s1 by s0.
	result.as_uint = s1.as_uint >> s0.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u) (%u >> %u) ", id,
			INST.vdst, result.as_uint, s1.as_uint, s0.as_uint);
	}
}
#undef INST

// D.i = S1.i >> S0.i[4:0].
#define INST INST_VOP2
void WorkItem::ISA_V_ASHRREV_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s0.as_uint = INST.lit_cnst & 0x1F;
	}
	else
	{
		s0.as_uint = ReadReg(INST.src0) & 0x1F;
	}
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Right shift s1 by s0.
	result.as_int = s1.as_int >> s0.as_int;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d, %u) ", id, INST.vdst,
			result.as_int, result.as_uint);
	}
}
#undef INST

// D.u = S0.u << S1.u[4:0].
#define INST INST_VOP2
void WorkItem::ISA_V_LSHL_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1) & 0x1F;

	// Left shift s1 by s0.
	result.as_uint = s0.as_uint << s1.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) ", id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = S1.u << S0.u[4:0].
#define INST INST_VOP2
void WorkItem::ISA_V_LSHLREV_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = ReadReg(INST.src0) & 0x1F;
	}
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Left shift s1 by s0.
	result.as_uint = s1.as_uint << s0.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) (%u << %u) ", id,
			INST.vdst, result.as_uint, s1.as_uint, s0.as_uint);
	}
}
#undef INST

// D.u = S0.u & S1.u.
#define INST INST_VOP2
void WorkItem::ISA_V_AND_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = ReadReg(INST.src0);
	}
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint & s1.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) (%u & %u) ", id, 
			INST.vdst, result.as_uint, s0.as_uint, s1.as_uint);
	}
}
#undef INST

// D.u = S0.u | S1.u.
#define INST INST_VOP2
void WorkItem::ISA_V_OR_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint | s1.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) (%u | %u) ", 
			id, INST.vdst, result.as_uint, s0.as_uint,
			s1.as_uint);
	}
}
#undef INST

// D.u = S0.u ^ S1.u.
#define INST INST_VOP2
void WorkItem::ISA_V_XOR_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint ^ s1.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)(%u ^ %u) ", id, 
			INST.vdst, result.as_uint, s0.as_uint, s1.as_uint);
	}
}
#undef INST

//D.u = ((1<<S0.u[4:0])-1) << S1.u[4:0]; S0=bitfield_width, S1=bitfield_offset.
#define INST INST_VOP2
void WorkItem::ISA_V_BFM_B32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.f = S0.f * S1.f + D.f.
#define INST INST_VOP2
void WorkItem::ISA_V_MAC_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg dst;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);
	dst.as_uint = ReadVReg(INST.vdst);

	// Calculate the result.
	result.as_float = s0.as_float * s1.as_float + dst.as_float;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			result.as_float);
	}
}
#undef INST

// D.f = S0.f * K + S1.f; K is a 32-bit inline constant
#define INST INST_VOP2
void WorkItem::ISA_V_MADMK_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg K;
	InstReg dst;

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);
	K.as_uint = INST.lit_cnst;
	
	// Calculate the result
	dst.as_float = s0.as_float * K.as_float + s1.as_float;

	// Write the results.
	WriteVReg(INST.vdst, dst.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%f) (%f * %f + %f)", id, 
			INST.vdst, dst.as_float, s0.as_float, K.as_float, 
			s1.as_float);
	}
}
#undef INST

// D.u = S0.u + S1.u, vcc = carry-out.
#define INST INST_VOP2
void WorkItem::ISA_V_ADD_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg sum;
	InstReg carry;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the sum and carry.
	sum.as_int = s0.as_int + s1.as_int;
	carry.as_uint = 
		! !(((long long) s0.as_int + (long long) s1.as_int) >> 32);

	// Write the results.
	WriteVReg(INST.vdst, sum.as_uint);
	WriteBitmaskSReg(SI_VCC, carry.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d) (%d + %d) ", id, 
			INST.vdst, sum.as_int, s0.as_int, s1.as_int);
		Emu::debug << StringFmt("vcc<=(%u) ", carry.as_uint);
	}
}
#undef INST

// D.u = S0.u - S1.u; vcc = carry-out.
#define INST INST_VOP2
void WorkItem::ISA_V_SUB_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg dif;
	InstReg carry;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the difference and carry.
	dif.as_uint = s0.as_int - s1.as_int;
	carry.as_uint = (s1.as_int > s0.as_int);

	// Write the results.
	WriteVReg(INST.vdst, dif.as_uint);
	WriteBitmaskSReg(SI_VCC, carry.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d) ", id, INST.vdst,
			dif.as_int);
		Emu::debug << StringFmt("vcc<=(%u) ", carry.as_uint);
	}
}
#undef INST

// D.u = S1.u - S0.u; vcc = carry-out.
#define INST INST_VOP2
void WorkItem::ISA_V_SUBREV_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg dif;
	InstReg carry;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Calculate the difference and carry.
	dif.as_int = s1.as_int - s0.as_int;
	carry.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	WriteVReg(INST.vdst, dif.as_uint);
	WriteBitmaskSReg(SI_VCC, carry.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d) ", id, INST.vdst,
			dif.as_int);
		Emu::debug << StringFmt("vcc<=(%u) ", carry.as_uint);
	}
}
#undef INST

// D = {flt32_to_flt16(S1.f),flt32_to_flt16(S0.f)}, with round-toward-zero.
#define INST INST_VOP2
void WorkItem::ISA_V_CVT_PKRTZ_F16_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	uint16_t s0f;
	uint16_t s1f;
	union hfpack float_pack;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Convert to half float
	s0f = Float32to16(s0.as_float);
	s1f = Float32to16(s1.as_float);
	float_pack.as_f16f16.s0f = s0f;
	float_pack.as_f16f16.s1f = s1f;

	// Write the results.
	WriteVReg(INST.vdst, float_pack.as_uint32);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d) ", id, INST.vdst,
			float_pack.as_uint32);
	}	
}
#undef INST


/*
 * VOPC
 */

// vcc = (S0.f < S1.f).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_LT_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_float < s1.as_float);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.f > S1.f).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_GT_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_float > s1.as_float);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.f >= S1.f).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_GE_F32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = !(S0.f > S1.f).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_NGT_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = !(s0.as_float > s1.as_float);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = !(S0.f == S1.f).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_NEQ_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = !(s0.as_float == s1.as_float);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.d < S1.d).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_LT_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = (S0.d == S1.d).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_EQ_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = (S0.d <= S1.d).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_LE_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = (S0.d > S1.d).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_GT_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = !(S0.d >= S1.d).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_NGE_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = !(S0.d == S1.d).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_NEQ_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = (S0.i < S1.i).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_LT_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.i == S1.i).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_EQ_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.i <= S1.i).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_LE_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.i > S1.i).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_GT_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.i <> S1.i).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_NE_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int != s1.as_int);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// D.u = (S0.i >= S1.i).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_GE_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// D = IEEE numeric class function specified in S1.u, performed on S0.d.
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_CLASS_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// vcc = (S0.u < S1.u).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_LT_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint < s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.u == S1.u).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_EQ_U32_Impl(Inst *inst)
{
	NOT_IMPL();
}

// vcc = (S0.u <= S1.u).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_LE_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// vcc = (S0.u > S1.u).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_GT_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	// Load operands from registers or as a literal constant.
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadVReg(INST.vsrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(SI_VCC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: vcc<=(%u) ",
			id_in_wavefront, result.as_uint);
	}
}
#undef INST

// D.u = (S0.f < S1.f).
#define INST INST_VOPC
void WorkItem::ISA_V_CMP_NE_U32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST


#define INST INST_VOPC
void WorkItem::ISA_V_CMP_GE_U32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST



/*
 * VOP3a
 */

/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a
 * scalar GPR in S2. */
#define INST INST_VOP3a
void WorkItem::ISA_V_CNDMASK_B32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	int vcci;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	vcci = ReadBitmaskSReg(INST.src2);

	// Perform "floating-point negation"
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Calculate the result.
	result.as_uint = (vcci) ? s1.as_uint : s0.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u, %gf) (s1=%u, s0=%u)", 
			id, INST.vdst, result.as_uint, 
			result.as_float, s1.as_uint, s0.as_uint);
	}
}
#undef INST

// D.f = S0.f + S1.f.
#define INST INST_VOP3a
void WorkItem::ISA_V_ADD_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg sum;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Calculate the sum.
	sum.as_float = s0.as_float + s1.as_float;

	// Write the results.
	WriteVReg(INST.vdst, sum.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			sum.as_float);
	}
}
#undef INST

// D.f = S1.f - S0.f
#define INST INST_VOP3a
void WorkItem::ISA_V_SUBREV_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg diff;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Calculate the diff.
	diff.as_float = s1.as_float - s0.as_float;

	// Write the results.
	WriteVReg(INST.vdst, diff.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			diff.as_float);
	}
}
#undef INST

// D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0).
#define INST INST_VOP3a
void WorkItem::ISA_V_MUL_LEGACY_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg product;

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Calculate the product.
	if (s0.as_float == 0.0f || s1.as_float == 0.0f)
	{
		product.as_float = 0.0f;
	}
	else
	{
		product.as_float = s0.as_float * s1.as_float;
	}

	// Write the results.
	WriteVReg(INST.vdst, product.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) (%gf * %gf) ", id, 
			INST.vdst, product.as_float, s0.as_float, s1.as_float);
	}
}
#undef INST

// D.f = S0.f * S1.f.
#define INST INST_VOP3a
void WorkItem::ISA_V_MUL_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Calculate the result.
	result.as_float = s0.as_float * s1.as_float;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) (%gf*%gf)", id, 
			INST.vdst, result.as_float, s0.as_float, s1.as_float);
	}
}
#undef INST

// D.f = S0. * S1..
#define INST INST_VOP3a
void WorkItem::ISA_V_MUL_I32_I24_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg product;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Truncate operands to 24-bit signed integers
	s0.as_uint = SignExtend32(s0.as_uint, 24);
	s1.as_uint = SignExtend32(s1.as_uint, 24);

	// Calculate the product.
	product.as_int = s0.as_int * s1.as_int;

	// Write the results.
	WriteVReg(INST.vdst, product.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d)", id, INST.vdst,
			product.as_int);
	}
}
#undef INST

// D.f = max(S0.f, S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_MAX_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Calculate the result.
	result.as_float = (s0.as_float > s1.as_float) ? 
		s0.as_float : s1.as_float;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			result.as_float);
	}
}
#undef INST

// D.f = S0.f * S1.f + S2.f.
#define INST INST_VOP3a
void WorkItem::ISA_V_MAD_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	if (INST.abs & 4)
		s2.as_float = fabsf(s2.as_float);

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	if (INST.neg & 4)
		s2.as_float = -s2.as_float;

	// Calculate the result.
	result.as_float = s0.as_float * s1.as_float + s2.as_float;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			result.as_float);
	}
}
#undef INST

// D.u = S0.u[23:0] * S1.u[23:0] + S2.u[31:0].
#define INST INST_VOP3a
void WorkItem::ISA_V_MAD_U32_U24_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	s0.as_uint = s0.as_uint & 0x00FFFFFF;
	s1.as_uint = s1.as_uint & 0x00FFFFFF;

	// Calculate the result.
	result.as_uint = s0.as_uint * s1.as_uint + s2.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u) ", id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.u >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST INST_VOP3a
void WorkItem::ISA_V_BFE_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	s1.as_uint = s1.as_uint & 0x1F;
	s2.as_uint = s2.as_uint & 0x1F;

	// Calculate the result.
	result.as_uint = (s0.as_uint >> s1.as_uint) & ((1 << s2.as_uint) - 1);

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: value:s0:%u, offset:s1:%u, width:s2:%u ",
			id, s0.as_uint, s1.as_uint, s2.as_uint);
		Emu::debug << StringFmt("V%u<=(0x%x) ", INST.vdst, result.as_uint);
	}
}
#undef INST

/* D.i = (S0.i >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST INST_VOP3a
void WorkItem::ISA_V_BFE_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	s1.as_uint = s1.as_uint & 0x1F;
	s2.as_uint = s2.as_uint & 0x1F;

	// Calculate the result.
	if (s2.as_uint == 0)
	{
		result.as_int = 0;
	}
	else if (s2.as_uint + s1.as_uint < 32)
	{
		result.as_int = (s0.as_int << (32 - s1.as_uint - s2.as_uint)) >> 
			(32 - s2.as_uint);
	}
	else
	{
		result.as_int = s0.as_int >> s1.as_uint;
	}

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d) ", id, INST.vdst,
			result.as_int);
	}
}
#undef INST

// D.u = (S0.u & S1.u) | (~S0.u & S2.u).
#define INST INST_VOP3a
void WorkItem::ISA_V_BFI_B32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	// Calculate the result.
	result.as_uint = (s0.as_uint & s1.as_uint) | 
		(~s0.as_uint & s2.as_uint);

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x) ", id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.f = S0.f * S1.f + S2.f
#define INST INST_VOP3a
void WorkItem::ISA_V_FMA_F32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	if (INST.abs & 4)
		s2.as_float = fabsf(s2.as_float);

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	if (INST.neg & 4)
		s2.as_float = -s2.as_float;

	// FMA
	result.as_float = (s0.as_float * s1.as_float) + s2.as_float;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) (%gf*%gf + %gf) ", 
			id, INST.vdst, result.as_float, 
			s0.as_float, s1.as_float, s2.as_float);
	}
}
#undef INST

// D.d = S0.d * S1.d + S2.d
#define INST INST_VOP3a
void WorkItem::ISA_V_FMA_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.u = ({S0,S1} >> S2.u[4:0]) & 0xFFFFFFFF.
#define INST INST_VOP3a
void WorkItem::ISA_V_ALIGNBIT_B32_Impl(Inst *inst)
{
	InstReg src2;
	InstReg result;

	union
	{
		unsigned long long as_b64;
		unsigned as_reg[2];

	} src;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	src.as_reg[0] = ReadReg(INST.src1);
	src.as_reg[1] = ReadReg(INST.src0);
	src2.as_uint = ReadReg(INST.src2);
	src2.as_uint = src2.as_uint & 0x1F;

	// ({S0,S1} >> S2.u[4:0]) & 0xFFFFFFFF.
	result.as_uint = (src.as_b64 >> src2.as_uint) & 0xFFFFFFFF;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x, %u) ({0x%x,0x%x} >> %u) ",
			id, INST.vdst, result.as_uint, 
			result.as_uint, src.as_reg[1], src.as_reg[0], 
			src2.as_uint);
	}
}
#undef INST

/* 
 *D.d = Special case divide fixup and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator).
 */
#define INST INST_VOP3a
void WorkItem::ISA_V_DIV_FIXUP_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.d = min(S0.d, S1.d).
#define INST INST_VOP3a
void WorkItem::ISA_V_MIN_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.d = max(S0.d, S1.d).
#define INST INST_VOP3a
void WorkItem::ISA_V_MAX_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.u = S0.u * S1.u.
#define INST INST_VOP3a
void WorkItem::ISA_V_MUL_LO_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Calculate the product.
	result.as_uint = s0.as_uint * s1.as_uint;

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u) ", id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* 
 *D.d = Special case divide FMA with scale and flags(s0.d = Quotient, s1.d = Denominator,
 *s2.d = Numerator).
 */
#define INST INST_VOP3a
void WorkItem::ISA_V_DIV_FMAS_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.d = Look Up 2/PI (S0.d) with segment select S1.u[4:0].
#define INST INST_VOP3a
void WorkItem::ISA_V_TRIG_PREOP_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.u = (S0.u * S1.u)>>32
#define INST INST_VOP3a
void WorkItem::ISA_V_MUL_HI_U32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Calculate the product and shift right.
	result.as_uint = (unsigned) 
		(((unsigned long long)s0.as_uint * 
		(unsigned long long)s1.as_uint) >> 32);

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u) ", id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.i = S0.i * S1.i.
#define INST INST_VOP3a
void WorkItem::ISA_V_MUL_LO_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Calculate the product.
	result.as_int = s0.as_int * s1.as_int;


	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%d)(%d*%d) ", id, 
			INST.vdst, result.as_int, s0.as_int, s1.as_int);
	}
}
#undef INST

// D.f = S0.f - floor(S0.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_FRACT_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);

	// Apply negation modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	assert (!(INST.abs & 2));
	assert (!(INST.abs & 4));

	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	assert (!(INST.neg & 2));
	assert (!(INST.neg & 4));

	// Calculate the product.
	result.as_float = s0.as_float - floorf(s0.as_float);

	// Write the results.
	WriteVReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%gf) ", id, INST.vdst,
			result.as_float);
	}
}
#undef INST

// D.u = (S0.f < S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LT_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_float < s1.as_float);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.f == S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_EQ_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_float == s1.as_float);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// vcc = (S0.f <= S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LE_F32_VOP3a_Impl(Inst *inst)
{
	NOT_IMPL();
}

// D.u = (S0.f > S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_GT_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_float > s1.as_float);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = !(S0.f <= S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_NLE_F32_VOP3a_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.u = !(S0.f == S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_NEQ_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = !(s0.as_float == s1.as_float);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = !(S0.f < S1.f).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_NLT_F32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = !(s0.as_float < s1.as_float);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// Comparison Operations
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_OP16_F64_VOP3a_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.u = (S0.i < S1.i).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LT_I32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.i == S1.i).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_EQ_I32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.i <= S1.i).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LE_I32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.i > S1.i).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_GT_I32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:%u]<=(%u) ",
			id_in_wavefront, INST.vdst, INST.vdst + 1, 
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.i <> S1.i).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_NE_I32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = !(s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.i >= S1.i).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_GE_I32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.i == S1.i). Also write EXEC
#define INST INST_VOP3a
void WorkItem::ISA_V_CMPX_EQ_I32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Apply absolute value modifiers.
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	// Apply negation modifiers.
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Write EXEC
	WriteBitmaskSReg(SI_EXEC, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:%u],EXEC<=(%u) ",
			id_in_wavefront, INST.vdst, INST.vdst+1,
			result.as_uint);
	}
}
#undef INST

// D = IEEE numeric class function specified in S1.u, performed on S0.d.
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_CLASS_F64_VOP3a_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// D.u = (S0.u < S1.u).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LT_U32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint < s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.u <= S1.u).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LE_U32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.u > S1.u).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_GT_U32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LG_U32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Calculate result.
	result.as_uint = ((s0.as_uint < s1.as_uint) || 
		(s0.as_uint > s1.as_uint));

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0.u >= S1.u).
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_GE_U32_VOP3a_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint >= s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(INST.vdst, result.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%u) ",
			id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

// D.u = (S0 < S1)
#define INST INST_VOP3a
void WorkItem::ISA_V_CMP_LT_U64_VOP3a_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// Max of three numbers.
#define INST INST_VOP3a
void WorkItem::ISA_V_MAX3_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg max;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	// Determine the max.
	// max3(s0, s1, s2) == s0
	if (s0.as_int >= s1.as_int && s0.as_int >= s2.as_int)
	{
		max.as_int = s0.as_int;
	}
	// max3(s0, s1, s2) == s1
	else if (s1.as_int >= s0.as_int && s1.as_int >= s2.as_int)
	{
		max.as_int = s1.as_int;
	}
	// max3(s0, s1, s2) == s2
	else if (s2.as_int >= s0.as_int && s2.as_int >= s1.as_int)
	{
		max.as_int = s2.as_int;
	}
	else
	{
		fatal("%s: Max algorithm failed\n", __FUNCTION__);
	}

	// Write the results.
	WriteVReg(INST.vdst, max.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V[%u]<=(%d) ",
			id_in_wavefront, INST.vdst, max.as_int);
	}
}
#undef INST

// Median of three numbers.
#define INST INST_VOP3a
void WorkItem::ISA_V_MED3_I32_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg s2;
	InstReg median;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	s2.as_uint = ReadReg(INST.src2);

	// Determine the median.
	// max3(s0, s1, s2) == s0
	if (s0.as_int >= s1.as_int && s0.as_int >= s2.as_int)
	{
		// max(s1, s2)
		median.as_int = (s1.as_int >= s2.as_int) ? 
			s1.as_int : s2.as_int;
	}
	// max3(s0, s1, s2) == s1
	else if (s1.as_int >= s0.as_int && s1.as_int >= s2.as_int)
	{
		// max(s0, s2)
		median.as_int = (s0.as_int >= s2.as_int) ? 
			s0.as_int : s2.as_int;
	}
	// max3(s0, s1, s2) == s2
	else if (s2.as_int >= s0.as_int && s2.as_int >= s1.as_int)
	{
		// max(s0, s1)
		median.as_int = (s0.as_int >= s1.as_int) ? 
			s0.as_int : s1.as_int;
	}
	else
	{
		fatal("%s: Median algorithm failed\n", __FUNCTION__);
	}

	// Write the results.
	WriteVReg(INST.vdst, median.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V[%u]<=(%d) ",
			id_in_wavefront, INST.vdst, median.as_int);
	}
}
#undef INST

// D = S0.u >> S1.u[4:0].
#define INST INST_VOP3a
void WorkItem::ISA_V_LSHR_B64_Impl(Inst *inst)
{
	union
	{
		unsigned long long as_b64;
		unsigned as_reg[2];

	} s0, value;

	InstReg s1;
	InstReg result_lo;
	InstReg result_hi;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(INST.src0);
	s0.as_reg[1] = ReadReg(INST.src0 + 1);
	s1.as_uint = ReadReg(INST.src1);
	s1.as_uint = s1.as_uint & 0x1F;

	// Shift s0.
	value.as_b64 = s0.as_b64 >> s1.as_uint;

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(INST.vdst, result_lo.as_uint);
	WriteVReg(INST.vdst + 1, result_hi.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u]<=(0x%x) ",
			id_in_wavefront, INST.vdst,
			result_lo.as_uint);
		Emu::debug << StringFmt("S[%u]<=(0x%x) ", INST.vdst + 1,
			result_hi.as_uint);
	}
}
#undef INST

// D = S0.u >> S1.u[4:0] (Arithmetic shift)
#define INST INST_VOP3a
void WorkItem::ISA_V_ASHR_I64_Impl(Inst *inst)
{
	union
	{
		long long as_i64;
		unsigned as_reg[2];

	} s0, value;

	InstReg s1;
	InstReg result_lo;
	InstReg result_hi;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(INST.src0);
	s0.as_reg[1] = ReadReg(INST.src0 + 1);
	s1.as_uint = ReadReg(INST.src1);
	s1.as_uint = s1.as_uint & 0x1F;

	// Shift s0.
	value.as_i64 = s0.as_i64 >> s1.as_uint;

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(INST.vdst, result_lo.as_uint);
	WriteVReg(INST.vdst + 1, result_hi.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u]<=(0x%x) ",
			id_in_wavefront, INST.vdst,
			result_lo.as_uint);
		Emu::debug << StringFmt("S[%u]<=(0x%x) ", INST.vdst + 1,
			result_hi.as_uint);
	}
}
#undef INST

// D.d = S0.d + S1.d.
#define INST INST_VOP3a
void WorkItem::ISA_V_ADD_F64_Impl(Inst *inst)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} s0, s1, value;

	InstReg result_lo;
	InstReg result_hi;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(INST.src0);
	s0.as_reg[1] = ReadReg(INST.src0 + 1);
	s1.as_reg[0] = ReadReg(INST.src1);
	s1.as_reg[1] = ReadReg(INST.src1 + 1);

	// Add the operands, take into account special number cases.

	// s0 == NaN64 || s1 == NaN64
	if (fpclassify(s0.as_double) == FP_NAN ||
		fpclassify(s1.as_double) == FP_NAN)
	{
		// value <-- NaN64
		value.as_double = NAN;
	}
	// s0,s1 == infinity
	else if (fpclassify(s0.as_double) == FP_INFINITE &&
		fpclassify(s1.as_double) == FP_INFINITE)
	{
		// value <-- NaN64
		value.as_double = NAN;
	}
	// s0,!s1 == infinity
	else if (fpclassify(s0.as_double) == FP_INFINITE)
	{
		// value <-- s0(+-infinity)
		value.as_double = s0.as_double;
	}
	// s1,!s0 == infinity
	else if (fpclassify(s1.as_double) == FP_INFINITE)
	{
		// value <-- s1(+-infinity)
		value.as_double = s1.as_double;
	}
	// s0 == +-denormal, +-0
	else if (fpclassify(s0.as_double) == FP_SUBNORMAL ||
		fpclassify(s0.as_double) == FP_ZERO)
	{
		// s1 == +-denormal, +-0
		if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO)
			// s0 && s1 == -denormal, -0
			if (! !signbit(s0.as_double)
				&& ! !signbit(s1.as_double))
				// value <-- -0
				value.as_double = -0;
			else
				// value <-- +0
				value.as_double = +0;
		// s1 == F
		else
			// value <-- s1
			value.as_double = s1.as_double;
	}
	// s1 == +-denormal, +-0
	else if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
		fpclassify(s1.as_double) == FP_ZERO)
	{
		// s0 == +-denormal, +-0
		if (fpclassify(s0.as_double) == FP_SUBNORMAL ||
			fpclassify(s0.as_double) == FP_ZERO)
			// s0 && s1 == -denormal, -0
			if (! !signbit(s0.as_double)
				&& ! !signbit(s1.as_double))
				// value <-- -0
				value.as_double = -0;
			else
				// value <-- +0
				value.as_double = +0;
		// s0 == F
		else
			// value <-- s1
			value.as_double = s0.as_double;
	}
	// s0 && s1 == F
	else
	{
		value.as_double = s0.as_double + s1.as_double;
	}

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(INST.vdst, result_lo.as_uint);
	WriteVReg(INST.vdst + 1, result_hi.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%lgf) ",
			id_in_wavefront, INST.vdst,
			value.as_double);
	}
}
#undef INST

// D.d = S0.d * S1.d.
#define INST INST_VOP3a
void WorkItem::ISA_V_MUL_F64_Impl(Inst *inst)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} s0, s1, value;

	InstReg result_lo;
	InstReg result_hi;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(INST.src0);
	s0.as_reg[1] = ReadReg(INST.src0 + 1);
	s1.as_reg[0] = ReadReg(INST.src1);
	s1.as_reg[1] = ReadReg(INST.src1 + 1);

	// Multiply the operands, take into account special number cases.

	// s0 == NaN64 || s1 == NaN64
	if (fpclassify(s0.as_double) == FP_NAN ||
		fpclassify(s1.as_double) == FP_NAN)
	{
		// value <-- NaN64
		value.as_double = NAN;
	}
	// s0 == +denormal, +0
	else if ((fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO) &&
		!signbit(s0.as_double))
	{
		// s1 == +-infinity
		if (isinf(s1.as_double))
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!signbit(s1.as_double))
			// value <-- +0
			value.as_double = +0;
		// s1 < 0
		else if (! !signbit(s1.as_double))
			// value <-- -0
			value.as_double = -0;
	}
	// s0 == -denormal, -0
	else if ((fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO) &&
		! !signbit(s0.as_double))
	{
		// s1 == +-infinity
		if (isinf(s1.as_double))
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!signbit(s1.as_double))
			// value <-- -0
			value.as_double = -0;
		// s1 < 0
		else if (! !signbit(s1.as_double))
			// value <-- +0
			value.as_double = +0;
	}
	// s0 == +infinity
	else if (fpclassify(s0.as_double) == FP_INFINITE &&
		!signbit(s0.as_double))
	{
		// s1 == +-denormal, +-0
		if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO)
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!signbit(s1.as_double))
			// value <-- +infinity
			value.as_double = +INFINITY;
		// s1 < 0
		else if (! !signbit(s1.as_double))
			// value <-- -infinity
			value.as_double = -INFINITY;
	}
	// s0 == -infinity
	else if (fpclassify(s0.as_double) == FP_INFINITE &&
		! !signbit(s0.as_double))
	{
		// s1 == +-denormal, +-0
		if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO)
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!signbit(s1.as_double))
			// value <-- -infinity
			value.as_double = -INFINITY;
		// s1 < 0
		else if (! !signbit(s1.as_double))
			// value <-- +infinity
			value.as_double = +INFINITY;
	}
	else
	{
		value.as_double = s0.as_double * s1.as_double;
	}

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(INST.vdst, result_lo.as_uint);
	WriteVReg(INST.vdst + 1, result_hi.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: S[%u:+1]<=(%lgf) ",
			id_in_wavefront, INST.vdst,
			value.as_double);
	}
}
#undef INST

// D.d = Look Up 2/PI (S0.d) with segment select S1.u[4:0].
#define INST INST_VOP3a
void WorkItem::ISA_V_LDEXP_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

/*
 * VOP3b
 */

/* D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carry-out,
 * S2.u=carry-in). */
#define INST INST_VOP3b
void WorkItem::ISA_V_ADDC_U32_VOP3b_Impl(Inst *inst)
{
	InstReg s0;
	InstReg s1;
	InstReg sum;
	InstReg carry_in;
	InstReg carry_out;

	assert(!INST.omod);
	assert(!INST.neg); 

	// Load operands from registers.
	s0.as_uint = ReadReg(INST.src0);
	s1.as_uint = ReadReg(INST.src1);
	carry_in.as_uint = ReadBitmaskSReg(INST.src2);

	// Calculate sum and carry.
	sum.as_uint = s0.as_uint + s1.as_uint + carry_in.as_uint;
	carry_out.as_uint =
		! !(((unsigned long long) s0.as_uint + 
			(unsigned long long) s1.as_uint +
			(unsigned long long) carry_in.as_uint) >> 32);

	// Write the results.
	WriteVReg(INST.vdst, sum.as_uint);
	WriteBitmaskSReg(INST.sdst, carry_out.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u) ", id, INST.vdst,
			sum.as_uint);
		Emu::debug << StringFmt("vcc<=(%u) ", carry_out.as_uint);
	}
}
#undef INST

/* 
 *D.d = Special case divide preop and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator)
 *s0 must equal s1 or s2.
 */
#define INST INST_VOP3b
void WorkItem::ISA_V_DIV_SCALE_F64_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST


/* 
 * VINTRP
 */

// FIXME: move this to some header file
// M0 must be intialized before VINTRP instructions
struct si_m0_for_vintrp_t
{
	unsigned b0 : 1;
	unsigned new_prim_mask : 15;
	unsigned lds_param_offset : 16;
}__attribute__((packed));

union si_isa_v_interp_m0_t
{
	unsigned as_uint;
	struct si_m0_for_vintrp_t for_vintrp;
};

// D = P10 * S + P0
#define INST INST_VINTRP
void WorkItem::ISA_V_INTERP_P1_F32_Impl(Inst *inst)
{
	InstReg s;
	InstReg p0;
	InstReg p10;
	InstReg data;

	union si_isa_v_interp_m0_t m0_vintrp;

	// Get lds offset and primitive mask information
	m0_vintrp.as_uint = ReadReg(SI_M0);

	// Read barycentric coordinates stored in VGPR
	s.as_uint = ReadVReg(INST.vsrc);

	// 12 successive dwords contain P0 P10 P20
	// 4dwords P0: X Y Z W, INST.attrchan decides which 1dword to be loaded
	lds->Read( 
		m0_vintrp.for_vintrp.lds_param_offset + 0 + 4 * INST.attrchan ,
		 4, (char *)&p0.as_uint);
	// 4dwords P10: X Y Z W, INST.attrchan decides which 1dword to be loaded
	lds->Read( 
		m0_vintrp.for_vintrp.lds_param_offset + 16 + 4 * INST.attrchan,
		 4, (char *)&p10.as_uint);

	// D = P10 * S + P0
	data.as_float = p10.as_float * s.as_float + p0.as_float;
	
	// Write the result
	WriteVReg(INST.vdst, data.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%f = P10(%f) * Lamda2(%f) + P0(%f)) \n", 
			id, INST.vdst, data.as_float, p10.as_float, 
			s.as_float, p0.as_float);
	}
}
#undef INST

// D = P20 * S + D
#define INST INST_VINTRP
void WorkItem::ISA_V_INTERP_P2_F32_Impl(Inst *inst)
{
	InstReg s;
	InstReg p20;
	InstReg data;

	union si_isa_v_interp_m0_t m0_vintrp;

	// Get lds offset and primitive mask information
	m0_vintrp.as_uint = ReadReg(SI_M0);

	// Read barycentric coordinates stored in VGPR
	s.as_uint = ReadVReg(INST.vsrc);

	// Read data stores in VGPR for later acclumulation
	data.as_uint = ReadVReg(INST.vdst);

	// 12 successive dwords contain P0 P10 P20
	// 4dwords P20: X Y Z W, INST.attrchan decides which 1dword to be loaded 
	lds->Read( m0_vintrp.for_vintrp.lds_param_offset + 32 + 4 * INST.attrchan,
		 4, (char *)&p20.as_uint);

	// D = P20 * S + D
	data.as_float += p20.as_float * s.as_float;

	// Write the result
	WriteVReg(INST.vdst, data.as_uint);

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%f += P20(%f) * Lamda2(%f)) \n", 
			id, INST.vdst, data.as_float, p20.as_float, 
			s.as_float);
	}
}
#undef INST

// D = {P10,P20,P0}[S]
#define INST INST_VINTRP
void WorkItem::ISA_V_INTERP_MOV_F32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST


/* 
 * DS
 */

// DS[A] = (DS[A] >= D0 ? 0 : DS[A] + 1); uint increment.
#define INST INST_DS
void WorkItem::ISA_DS_INC_U32_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

// DS[ADDR+offset0*4] = D0; DS[ADDR+offset1*4] = D1; Write 2 Dwords
#define INST INST_DS
void WorkItem::ISA_DS_WRITE2_B32_Impl(Inst *inst)
{
	InstReg addr0;
	InstReg addr1;
	InstReg data0;
	InstReg data1;

	assert(!INST.gds);

	// Load address and data from registers.
	addr0.as_uint = ReadVReg(INST.addr);
	addr0.as_uint += INST.offset0*4;
	addr1.as_uint = ReadVReg(INST.addr);
	addr1.as_uint += INST.offset1*4;
	data0.as_uint = ReadVReg(INST.data0);
	data1.as_uint = ReadVReg(INST.data1);

	if (addr0.as_uint > std::min(work_group->getNDRange()->getLocalMemTop(),
		ReadSReg(SI_M0)))
	{
		fatal("%s: invalid address\n", __FUNCTION__);
	}
	if (addr1.as_uint > std::min(work_group->getNDRange()->getLocalMemTop(),
		ReadSReg(SI_M0)))
	{
		fatal("%s: invalid address\n", __FUNCTION__);
	}

	// Write Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Write(addr0.as_uint, 4,
			(char *)&data0.as_uint);
		lds->Write(addr1.as_uint, 4,
			(char *)&data1.as_uint);
	}

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		// If offset1 != 1, then the following is incorrect
		assert(INST.offset0 == 0);
		assert(INST.offset1 == 1);
		lds_access_count = 2;
		lds_access[0].type = MemoryAccessWrite;
		lds_access[0].addr = addr0.as_uint;
		lds_access[0].size = 4;
		lds_access[1].type = MemoryAccessWrite;
		lds_access[1].addr = addr0.as_uint + 4;
		lds_access[1].size = 4;
	}

	// Print isa debug information.
	if (Emu::debug && INST.gds)
	{
		Emu::debug << StringFmt("t%d: GDS[%u]<=(%u,%f) ", id, 
			addr0.as_uint, data0.as_uint, data0.as_float);
		Emu::debug << StringFmt("GDS[%u]<=(%u,%f) ", addr1.as_uint, data0.as_uint,
			data0.as_float);
	}
	else
	{
		Emu::debug << StringFmt("t%d: LDS[%u]<=(%u,%f) ", id, 
			addr0.as_uint, data0.as_uint, data0.as_float);
		Emu::debug << StringFmt("LDS[%u]<=(%u,%f) ", addr1.as_uint, data1.as_uint, 
			data1.as_float);
	}
}
#undef INST

// DS[A] = D0; write a Dword.
#define INST INST_DS
void WorkItem::ISA_DS_WRITE_B32_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data0;

	assert(!INST.offset0);
	//assert(!INST.offset1);
	assert(!INST.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(INST.addr);
	data0.as_uint = ReadVReg(INST.data0);

	if (addr.as_uint > std::min(work_group->getNDRange()->getLocalMemTop(), 
		ReadSReg(SI_M0)))
	{
		fatal("%s: invalid address\n", __FUNCTION__);
	}

	// Global data store not supported
	assert(!INST.gds);

	// Write Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Write(addr.as_uint, 4, 
			(char *)&data0.as_uint);
	}

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessWrite;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 4;
	}

	// Print isa debug information.
	if (Emu::debug && INST.gds)
	{
		Emu::debug << StringFmt("t%d: GDS[%u]<=(%u,%f) ", id, 
			addr.as_uint, data0.as_uint, data0.as_float);
	}
	else
	{
		Emu::debug << StringFmt("t%d: LDS[%u]<=(%u,%f) ", id, 
			addr.as_uint, data0.as_uint, data0.as_float);
	}
}
#undef INST

// DS[A] = D0[7:0]; byte write. 
#define INST INST_DS
void WorkItem::ISA_DS_WRITE_B8_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data0;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(INST.addr);
	data0.as_uint = ReadVReg(INST.data0);

	// Global data store not supported
	assert(!INST.gds);

	// Write Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Write(addr.as_uint, 1, 
			(char *)data0.as_ubyte);
	}

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessWrite;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 1;
	}

	// Print isa debug information.
	if (Emu::debug && INST.gds)
	{
		Emu::debug << StringFmt("t%d: GDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ubyte[0]);
	}
	else
	{
		Emu::debug << StringFmt("t%d: LDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ubyte[0]);
	}
}
#undef INST

// DS[A] = D0[15:0]; short write. 
#define INST INST_DS
void WorkItem::ISA_DS_WRITE_B16_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data0;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(INST.addr);
	data0.as_uint = ReadVReg(INST.data0);

	// Global data store not supported
	assert(!INST.gds);

	// Write Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Write(addr.as_uint, 2, 
			(char *)data0.as_ushort);
	}

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessWrite;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 2;
	}

	// Print isa debug information.
	if (Emu::debug && INST.gds)
	{
		Emu::debug << StringFmt("t%d: GDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ushort[0]);
	}
	else
	{
		Emu::debug << StringFmt("t%d: LDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ushort[0]);
	}

}
#undef INST

// R = DS[A]; Dword read.
#define INST INST_DS
void WorkItem::ISA_DS_READ_B32_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data;

	assert(!INST.offset0);
	//assert(!INST.offset1);
	assert(!INST.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(INST.addr);

	// Global data store not supported
	assert(!INST.gds);

	// Read Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Read(addr.as_uint, 4,
			(char *)&data.as_uint);
	}

	// Write results.
	WriteVReg(INST.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessRead;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 4;
	}

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x)(0x%x) ", id, 
			INST.vdst, addr.as_uint, data.as_uint);
	}

}
#undef INST

// R = DS[ADDR+offset0*4], R+1 = DS[ADDR+offset1*4]. Read 2 Dwords.
#define INST INST_DS
void WorkItem::ISA_DS_READ2_B32_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data0;
	InstReg data1;

	assert(!INST.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(INST.addr);

	// Global data store not supported
	assert(!INST.gds);

	// Read Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Read(
			addr.as_uint + INST.offset0*4, 4, (char *)&data0.as_uint);
		lds->Read(
			addr.as_uint + INST.offset1*4, 4, (char *)&data1.as_uint);
	}

	// Write results.
	WriteVReg(INST.vdst, data0.as_uint);
	WriteVReg(INST.vdst+1, data1.as_uint);

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		// If offset1 != 1, then the following is incorrect
		assert(INST.offset0 == 0);
		assert(INST.offset1 == 1);
		lds_access_count = 2;
		lds_access[0].type = MemoryAccessRead;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 4;
		lds_access[1].type = MemoryAccessRead;
		lds_access[1].addr = addr.as_uint + 4;
		lds_access[1].size = 4;
	}

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x)(0x%x) ", id, 
			INST.vdst, addr.as_uint+INST.offset0*4, 
			data0.as_uint);
		Emu::debug << StringFmt("V%u<=(0x%x)(0x%x) ", INST.vdst+1, 
			addr.as_uint+INST.offset1*4, data1.as_uint);
	}
}
#undef INST

// R = signext(DS[A][7:0]}; signed byte read.
#define INST INST_DS
void WorkItem::ISA_DS_READ_I8_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(INST.addr);

	// Global data store not supported
	assert(!INST.gds);

	// Read Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Read(addr.as_uint, 1,
			&data.as_byte[0]);
	}

	// Extend the sign.
	data.as_int = (int) data.as_byte[0];

	// Write results.
	WriteVReg(INST.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessRead;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 1;
	}

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x)(%d) ", id, INST.vdst,
			addr.as_uint, data.as_int);
	}
}
#undef INST

// R = {24’h0,DS[A][7:0]}; unsigned byte read.
#define INST INST_DS
void WorkItem::ISA_DS_READ_U8_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(INST.addr);

	// Global data store not supported
	assert(!INST.gds);

	// Read Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Read(addr.as_uint, 1,
			(char *)&data.as_ubyte[0]);
	}

	// Make sure to use only bits [7:0].
	data.as_uint = (unsigned) data.as_ubyte[0];

	// Write results.
	WriteVReg(INST.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessRead;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 1;
	}

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x)(%u) ", id, INST.vdst,
			addr.as_uint, data.as_uint);
	}
}
#undef INST

// R = signext(DS[A][15:0]}; signed short read.
#define INST INST_DS
void WorkItem::ISA_DS_READ_I16_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(INST.addr);

	// Global data store not supported
	assert(!INST.gds);

	// Read Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Read(addr.as_uint, 2, (char *)&data.as_short[0]);
	}

	// Extend the sign.
	data.as_int = (int) data.as_short[0];

	// Write results.
	WriteVReg(INST.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessRead;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 2;
	}

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x)(%d) ", id, INST.vdst,
			addr.as_uint, data.as_int);
	}

}
#undef INST

// R = {16’h0,DS[A][15:0]}; unsigned short read.
#define INST INST_DS
void WorkItem::ISA_DS_READ_U16_Impl(Inst *inst)
{
	InstReg addr;
	InstReg data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(INST.addr);

	// Global data store not supported
	assert(!INST.gds);

	// Read Dword.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds->Read(addr.as_uint, 2,
			(char *)&data.as_ushort[0]);
	}

	// Make sure to use only bits [15:0].
	data.as_uint = (unsigned) data.as_ushort[0];

	// Write results.
	WriteVReg(INST.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		lds_access_count = 1;
		lds_access[0].type = MemoryAccessRead;
		lds_access[0].addr = addr.as_uint;
		lds_access[0].size = 2;
	}

	// Print isa debug information.
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(0x%x)(%u) ", id, INST.vdst,
			addr.as_uint, data.as_uint);
	}
}
#undef INST


/*
 * MUBUF
 */

#define INST INST_MUBUF
void WorkItem::ISA_BUFFER_LOAD_SBYTE_Impl(Inst *inst)
{

	assert(!INST.addr64);
	assert(!INST.glc);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	EmuBufferDesc buf_desc;
	InstReg value;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_read = 1;

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	
	global_mem->Read(addr, bytes_to_read, (char *)&value);
	
	// Sign extend
	value.as_int = (int) value.as_byte[0];

	WriteVReg(INST.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_read;

	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)(%d) ", id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST

#define INST INST_MUBUF
void WorkItem::ISA_BUFFER_LOAD_DWORD_Impl(Inst *inst)
{

	assert(!INST.addr64);
	assert(!INST.glc);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	EmuBufferDesc buf_desc;
	InstReg value;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_read = 4;

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	
	global_mem->Read(addr, bytes_to_read, (char *)&value);
	
	// Sign extend
	value.as_int = (int) value.as_byte[0];

	WriteVReg(INST.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_read;

	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)(%d) ", id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST

#define INST INST_MUBUF
void WorkItem::ISA_BUFFER_STORE_BYTE_Impl(Inst *inst)
{

	assert(!INST.addr64);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	EmuBufferDesc buf_desc;
	InstReg value;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_write = 1;

	if (INST.glc)
	{
		wavefront->setVectorMemGlobalCoherency(); // FIXME redundant
	}

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	value.as_int = ReadVReg(INST.vdata);

	global_mem->Write(addr, bytes_to_write, (char *)&value);
	
	// Sign extend
	//value.as_int = (int) value.as_byte[0];

	WriteVReg(INST.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_write;

	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)(%d) ", id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST

#define INST INST_MUBUF
void WorkItem::ISA_BUFFER_STORE_DWORD_Impl(Inst *inst)
{

	assert(!INST.addr64);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	EmuBufferDesc buf_desc;
	InstReg value;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_write = 4;

	if (INST.glc)
	{
		wavefront->setVectorMemGlobalCoherency(); // FIXME redundant
	}

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	value.as_int = ReadVReg(INST.vdata);

	global_mem->Write(addr, bytes_to_write, (char *)&value);
	
	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_write;

	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: (%u)<=V%u(%d) ", id,
			addr, INST.vdata, value.as_int);
	}
}
#undef INST

#define INST INST_MUBUF
void WorkItem::ISA_BUFFER_ATOMIC_ADD_Impl(Inst *inst)
{

	assert(!INST.addr64);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	EmuBufferDesc buf_desc;
	InstReg value;
	InstReg prev_value;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_read = 4;
	int bytes_to_write = 4;

	if (INST.glc)
	{
		wavefront->setVectorMemGlobalCoherency();
	}
	else
	{
		/* NOTE Regardless of whether the glc bit is set by the AMD 
		 * compiler, for the NMOESI protocol correctness , the glc bit
		 * must be set. */
		wavefront->setVectorMemGlobalCoherency();
	}

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	// Read existing value from global memory
	
	global_mem->Read(addr, bytes_to_read, prev_value.as_byte);

	// Read value to add to existing value from a register
	value.as_int = ReadVReg(INST.vdata);

	// Compute and store the updated value
	value.as_int += prev_value.as_int;
	global_mem->Write(addr, bytes_to_write, (char *)&value);
	
	// If glc bit set, retturn the previous value in a register
	if (INST.glc)
	{
		WriteVReg(INST.vdata, prev_value.as_uint);
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_write;

	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)(%d) ", id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST


/*
 * MTBUF 
 */

#define INST INST_MTBUF
void WorkItem::ISA_TBUFFER_LOAD_FORMAT_X_Impl(Inst *inst)
{

	assert(!INST.addr64);
	assert(!INST.tfe);
	assert(!INST.slc);

	EmuBufferDesc buf_desc;
	InstReg value;

	int elem_size;
	int num_elems;
	int bytes_to_read;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	elem_size = IsaGetElemSize(INST.dfmt);
	num_elems = IsaGetNumElems(INST.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 1);
	assert(elem_size == 4);

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	// Calculate the address
	// XXX Need to know when to enable id_in_wavefront
	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + 0/*work_item->id_in_wavefront*/);

	
	global_mem->Read(addr, bytes_to_read, (char *)&value);

	WriteVReg(INST.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_read;

	// TODO Print value based on type
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: V%u<=(%u)(%u,%gf) ", id,
			INST.vdata, addr, value.as_uint, value.as_float);
		if (INST.offen)
			Emu::debug << StringFmt("offen ");
		if (INST.idxen)
			Emu::debug << StringFmt("idxen ");
		Emu::debug << StringFmt("%u,%u,%u,%u,%u,%u ", base, mem_offset, 
			inst_offset, off_vgpr, idx_vgpr, stride);
	}
}
#undef INST

#define INST INST_MTBUF
void WorkItem::ISA_TBUFFER_LOAD_FORMAT_XY_Impl(Inst *inst)
{

	assert(!INST.addr64);

	EmuBufferDesc buf_desc;
	InstReg value;

	int i;
	int elem_size;
	int num_elems;
	int bytes_to_read;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	elem_size = IsaGetElemSize(INST.dfmt);
	num_elems = IsaGetNumElems(INST.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 2);
	assert(elem_size == 4);

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	// Calculate the address
	// XXX Need to know when to enable id_in_wavefront
	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + 0/*work_item->id_in_wavefront*/);

	for (i = 0; i < 2; i++)
	{
		
		global_mem->Read(addr+4*i, 4, (char *)&value);

		WriteVReg(INST.vdata + i, value.as_uint);

		// TODO Print value based on type
		if (Emu::debug)
		{
			Emu::debug << StringFmt("t%d: V%u<=(%u)(%u,%gf) ", id,
				INST.vdata + i, addr+4*i, value.as_uint,
				value.as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_read;
}
#undef INST

#define INST INST_MTBUF
void WorkItem::ISA_TBUFFER_LOAD_FORMAT_XYZ_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

#define INST INST_MTBUF
void WorkItem::ISA_TBUFFER_LOAD_FORMAT_XYZW_Impl(Inst *inst)
{

	assert(!INST.addr64);

	EmuBufferDesc buf_desc;
	InstReg value;

	int i;
	int elem_size;
	int num_elems;
	int bytes_to_read;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;
	unsigned id_in_wavefront = 0;

	elem_size = IsaGetElemSize(INST.dfmt);
	num_elems = IsaGetNumElems(INST.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 4);
	assert(elem_size == 4);

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	// XXX Need to know when to enable id_in_wavefront
	id_in_wavefront = buf_desc.add_tid_enable ?  id_in_wavefront : 0;
	
	// Calculate the address
	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	for (i = 0; i < 4; i++)
	{
		
		global_mem->Read(addr+4*i, 4, (char *)&value);

		WriteVReg(INST.vdata + i, value.as_uint);

		// TODO Print value based on type
		if (Emu::debug)
		{
			Emu::debug << StringFmt("t%d: V%u<=(%u)(%u,%gf) ", id,
				INST.vdata + i, addr+4*i, value.as_uint,
				value.as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_read;
}
#undef INST

#define INST INST_MTBUF
void WorkItem::ISA_TBUFFER_STORE_FORMAT_X_Impl(Inst *inst)
{

	assert(!INST.addr64);

	EmuBufferDesc buf_desc;
	InstReg value;

	int elem_size;
	int num_elems;
	int bytes_to_write;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	elem_size = IsaGetElemSize(INST.dfmt);
	num_elems = IsaGetNumElems(INST.dfmt);
	bytes_to_write = elem_size * num_elems;

	assert(num_elems == 1);
	assert(elem_size == 4);

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	value.as_uint = ReadVReg(INST.vdata);

	global_mem->Write(addr, bytes_to_write, (char *)&value);

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_write;

	// TODO Print value based on type
	if (Emu::debug)
	{
		Emu::debug << StringFmt("t%d: (%u)<=V%u(%u,%gf) ", id,
			addr, INST.vdata, value.as_uint,
			value.as_float);
	}
}
#undef INST

#define INST INST_MTBUF
void WorkItem::ISA_TBUFFER_STORE_FORMAT_XY_Impl(Inst *inst)
{

	assert(!INST.addr64);

	EmuBufferDesc buf_desc;
	InstReg value;

	int elem_size;
	int num_elems;
	int bytes_to_write;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	elem_size = IsaGetElemSize(INST.dfmt);
	num_elems = IsaGetNumElems(INST.dfmt);
	bytes_to_write = elem_size * num_elems;

	assert(num_elems == 2);
	assert(elem_size == 4);

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	for (unsigned i = 0; i < 2; i++)
	{
		value.as_uint = ReadVReg(INST.vdata + i);

		global_mem->Write(addr+4*i, 4, (char *)&value);

		// TODO Print value based on type
		if (Emu::debug)
		{
			Emu::debug << StringFmt("t%d: (%u)<=V%u(%u,%gf) ", id,
				addr, INST.vdata+i, value.as_uint,
				value.as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_write;
}
#undef INST

#define INST INST_MTBUF
void WorkItem::ISA_TBUFFER_STORE_FORMAT_XYZW_Impl(Inst *inst)
{

	assert(!INST.addr64);

	EmuBufferDesc buf_desc;
	InstReg value;

	int elem_size;
	int num_elems;
	int bytes_to_write;

	unsigned base;
	unsigned mem_offset = 0;
	unsigned inst_offset = 0;
	unsigned off_vgpr = 0;
	unsigned stride = 0;
	unsigned idx_vgpr = 0;

	elem_size = IsaGetElemSize(INST.dfmt);
	num_elems = IsaGetNumElems(INST.dfmt);
	bytes_to_write = elem_size * num_elems;

	assert(num_elems == 4);
	assert(elem_size == 4);

	// srsrc is in units of 4 registers
	ReadBufferResource(INST.srsrc * 4, buf_desc);

	// Figure 8.1 from SI ISA defines address calculation
	base = buf_desc.base_addr;
	mem_offset = ReadSReg(INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	// Table 8.3 from SI ISA
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = ReadVReg(INST.vaddr);
		off_vgpr = ReadVReg(INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	// Calculate the address
	unsigned addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + id_in_wavefront);

	for (unsigned i = 0; i < 4; i++)
	{
		value.as_uint = ReadVReg(INST.vdata + i);

		global_mem->Write(addr+4*i, 4, (char *)&value);

		// TODO Print value based on type
		if (Emu::debug)
		{
			Emu::debug << StringFmt("t%d: (%u)<=V%u(%u,%gf) ", id,
				addr, INST.vdata+i, value.as_uint,
				value.as_float);
		}
	}

	// Record last memory access for the detailed simulator.
	global_mem_access_addr = addr;
	global_mem_access_size = bytes_to_write;
}
#undef INST

/*
 * MIMG
 */

#define INST INST_MIMG
void WorkItem::ISA_IMAGE_STORE_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST

#define INST INST_MIMG
void WorkItem::ISA_IMAGE_SAMPLE_Impl(Inst *inst)
{
	NOT_IMPL();
}
#undef INST


/*
 * EXPORT
 */

#define INST INST_EXP
void WorkItem::ISA_EXPORT_Impl(Inst *inst)
{
	// FIXME: Shader Export module to be implemented
	// SISX *sx = emu->sx;
	
	// unsigned compr_en;
	// unsigned target_id;
	// unsigned en_bitmask;
	// InstReg x;
	// InstReg y;
	// InstReg z;
	// InstReg w;

	// compr_en = INST.compr;
	// en_bitmask = INST.en;
	// target_id = INST.tgt;

	// if (!compr_en)
	// {
	// 	if ((en_bitmask & 0x1))
	// 		x.as_uint = ReadVReg(INST.vsrc0);
	// 	else
	// 		x.as_uint = 0;
	// 	if ((en_bitmask & 0x2))
	// 		y.as_uint = ReadVReg(INST.vsrc1);
	// 	else 
	// 		y.as_uint = 0;
	// 	if ((en_bitmask & 0x4))
	// 		z.as_uint = ReadVReg(INST.vsrc2);
	// 	else
	// 		z.as_uint = 0;
	// 	if ((en_bitmask & 0x8))
	// 		w.as_uint = ReadVReg(INST.vsrc3);
	// 	else
	// 		w.as_uint = 0;
	// }
	// else
	// {
	// 	if ((en_bitmask & 0x1))
	// 		x.as_uint = ReadVReg(INST.vsrc0);
	// 	else
	// 		x.as_uint = 0;
	// 	if ((en_bitmask & 0x2))
	// 		y.as_uint = ReadVReg(INST.vsrc1);
	// 	else 
	// 		y.as_uint = 0;
	// 	z.as_uint = 0;
	// 	w.as_uint = 0;
	// }

	// if (target_id >=0 && target_id <= 7)
	// {
	// 	// Export to MRT 0-7
	// 	SISXExportMRT(sx, target_id, work_item, compr_en, x, y, z, w);
	// }
	// else if (target_id == 8)
	// {
	// 	// Export to Z
	// }
	// else if (target_id == 9)
	// {
	// 	// NULL
	// }
	// else if (target_id >= 12 && target_id <= 15)
	// {
	// 	// Position 0-3
	// 	SISXExportPosition(sx, target_id - 12, id, x.as_float, y.as_float, z.as_float, w.as_float);
	// }
	// else if (target_id >= 32 && target_id <= 63)
	// {
	// 	// Parameter 0 - 31
	// 	SISXExportParam(sx, target_id - 32, id, x.as_float, y.as_float, z.as_float, w.as_float);
	// } else
	// 	fatal("Export target %d is not valid!\n", target_id);
}
#undef INST

}  // namespace SI

