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

#include <lib/cpp/Misc.h>

#include "Wavefront.h"
#include "WorkItem.h"
#include "WorkGroup.h"


namespace SI
{

void WorkItem::ISAUnimplemented(Instruction *inst)
{
	throw misc::Panic(misc::fmt("%s: Unimplemented Southern Islands "
			"instruction\n"
			"\n\t"
			"The AMD Southern Islands instruction set is partially "
			"supported by Multi2Sim. If your program is using an "
			"unimplemented instruction, please report a bug on "
			"www.multi2sim.org requesting support for it.",
			inst->getName()));

}

int WorkItem::ISAGetNumElems(int data_format)
{
	int num_elems;

	switch (data_format)
	{

	case 1:
	case 2:
	case 4:
	{
		num_elems = 1;
		break;
	}

	case 3:
	case 5:
	case 11:
	{
		num_elems = 2;
		break;
	}

	case 13:
	{
		num_elems = 3;	
		break;
	}

	case 10:
	case 12:
	case 14:
	{
		num_elems = 4;
		break;
	}

	default:

		throw misc::Panic("Invalid data format");

	}

	return num_elems;
}

int WorkItem::ISAGetElemSize(int data_format)
{
	int elem_size;

	switch (data_format)
	{

	// 8-bit data
	case 1:
	case 3:
	case 10:
	{
		elem_size = 1;
		break;
	}

	// 16-bit data
	case 2:
	case 5:
	case 12:
	{
		elem_size = 2;
		break;
	}

	// 32-bit data
	case 4:
	case 11:
	case 13:
	case 14:
	{
		elem_size = 4;	
		break;
	}

	default:

		throw misc::Panic("Invalid data format");

	}

	return elem_size;
}

union hfpack
{
	unsigned as_uint32;
	struct
	{
		unsigned short s1f;
		unsigned short s0f;
	} as_f16f16;
};

unsigned short WorkItem::Float32to16(float value)
{
	union Bits
	{
		float f;
		int si;
		unsigned ui;
	};

	const unsigned F_shift= 13;
	const unsigned F_shiftSign = 16;

	const int F_infN = 0x7F800000; // flt32 infinity
	const int F_maxN = 0x477FE000; // max flt16 normal as a flt32
	const int F_minN = 0x38800000; // min flt16 normal as a flt32
	const int F_signN = 0x80000000; // flt32 sign bit

	const int F_infC = F_infN >> F_shift;
	const int F_nanN = (F_infC + 1) << F_shift; // minimum flt16 nan as a flt32
	const int F_maxC = F_maxN >> F_shift;
	const int F_minC = F_minN >> F_shift;
	const int F_mulN = 0x52000000; // (1 << 23) / F_minN
	const int F_subC = 0x003FF; // max flt32 subnormal down shifted
	const int F_maxD = F_infC - F_maxC - 1;
	const int F_minD = F_minC - F_subC - 1;

	union Bits v, s;
	v.f = value;
	unsigned sign = v.si & F_signN;
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

float WorkItem::Float16to32(unsigned short value)
{
	union Bits
	{
		float f;
		int si;
		unsigned ui;
	};

	const unsigned F_shift = 13;
	const unsigned F_shiftSign = 16;

	const int F_infN = 0x7F800000; // flt32 infinity
	const int F_maxN = 0x477FE000; // max flt16 normal as a flt32
	const int F_minN = 0x38800000; // min flt16 normal as a flt32
	const int F_signN = 0x80000000; // flt32 sign bit

	const int F_infC = F_infN >> F_shift;
	const int F_maxC = F_maxN >> F_shift;
	const int F_minC = F_minN >> F_shift;
	const int F_signC = F_signN >> F_shiftSign; // flt16 sign bit

	const int F_mulC =  0x33800000; // F_minN / (1 << (23 - F_shift))

	const int F_subC = 0x003FF; // max flt32 subnormal down shifted
	const int F_norC = 0x00400; // min flt32 normal down shifted

	const int F_maxD = F_infC - F_maxC - 1;
	const int F_minD = F_minC - F_subC - 1;

	union Bits v;
	v.ui = value;
	int sign = v.si & F_signC;
	v.si ^= sign;
	sign <<= F_shiftSign;
	v.si ^= ((v.si + F_minD) ^ v.si) & -(v.si > F_subC);
	v.si ^= ((v.si + F_maxD) ^ v.si) & -(v.si > F_maxC);
	union Bits s;
	s.si = F_mulC;
	s.f *= v.si;
	int mask = -(F_norC > v.si);
	v.si <<= F_shift;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;
	return v.f;
}


WorkItem::WorkItem(Wavefront *wavefront, int id)
{
	// Initialization
	this->id = id;
	this->wavefront = wavefront;

	// Work-group that is belongs to
	work_group = wavefront->getWorkGroup();

	ISAInstFuncTable[Instruction::OpcodeInvalid] =  nullptr;
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	ISAInstFuncTable[Instruction::Opcode_##_name] = &WorkItem::ISA_##_name##_Impl;
#include <arch/southern-islands/disassembler/Instruction.def>
#undef DEFINST
	ISAInstFuncTable[Instruction::OpcodeCount] = nullptr;
}

void WorkItem::setWorkGroup(WorkGroup *wg)
{ 
	work_group = wg; 
	lds = wg->getLocalMemory(); 
}

void WorkItem::Execute(Instruction::Opcode opcode, Instruction *inst)
{
	(this->*(ISAInstFuncTable[opcode]))(inst);
}


unsigned WorkItem::ReadSReg(int sreg)
{
	return wavefront->getSregUint(sreg);
}


void WorkItem::WriteSReg(int sreg, 
	unsigned value)
{
	// Set scalar register and update VCCZ and EXECZ if necessary.
	wavefront->setSregUint(sreg, value);
}


unsigned WorkItem::ReadVReg(int vreg)
{
	assert(vreg >= 0);
	assert(vreg < 256);

	// Statistics
	work_group->incVregReadCount();

	return this->vreg[vreg].as_uint;
}


void WorkItem::WriteVReg(int vreg, 
	unsigned value)
{
	assert(vreg >= 0);
	assert(vreg < 256);
	this->vreg[vreg].as_uint = value;

	// Statistics
	work_group->incVregWriteCount();
}


unsigned WorkItem::ReadReg(int reg)
{
	if (reg < 256)
	{
		return ReadSReg(reg);
	}
	else
	{
		return ReadVReg(reg - 256);
	}
}


void WorkItem::WriteBitmaskSReg(int sreg, 
	unsigned value)
{
	unsigned mask = 1;
	unsigned bitfield;
	Instruction::Register new_field;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		bitfield = ReadSReg(sreg);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		WriteSReg(sreg, new_field.as_uint);
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		bitfield = ReadSReg(sreg + 1);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		WriteSReg(sreg + 1, new_field.as_uint);
	}
}


int WorkItem::ReadBitmaskSReg(int sreg)
{
	unsigned mask = 1;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (ReadSReg(sreg) & mask) >> 
			id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (ReadSReg(sreg + 1) & mask) >> 
			(id_in_wavefront - 32);
	}
}


// Initialize a buffer resource descriptor
void WorkItem::ReadBufferResource(
	int sreg,
	WorkItem::BufferDescriptor &buf_desc)
{
	// Buffer resource descriptor is stored in 4 succesive scalar registers
	((unsigned *) &buf_desc)[0] = wavefront->getSregUint(sreg);
	((unsigned *) &buf_desc)[1] = wavefront->getSregUint(sreg + 1);
	((unsigned *) &buf_desc)[2] = wavefront->getSregUint(sreg + 2);
	((unsigned *) &buf_desc)[3] = wavefront->getSregUint(sreg + 3);
}


// Initialize a mempry pointer descriptor
void WorkItem::ReadMemPtr(
	int sreg,
	WorkItem::MemoryPointer &mem_ptr)
{
	// Memory pointer descriptor is stored in 2 succesive scalar registers
	((unsigned *)&mem_ptr)[0] = wavefront->getSregUint(sreg);
	((unsigned *)&mem_ptr)[1] = wavefront->getSregUint(sreg + 1);
}


}  // namespace SI
