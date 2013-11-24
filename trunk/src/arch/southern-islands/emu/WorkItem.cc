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

using namespace misc;

namespace SI
{

/*
 * Private functions
 */

void WorkItem::ISAUnimplemented(Inst *inst)
{
	static const char*err_si_isa_note =
	"The AMD Southern Islands instruction set is partially supported by"
	"Multi2Sim. If your program is using an unimplemented instruction,"
	"please email development@multi2sim.org' to request support for it.";

	fatal("GPU instruction '%s' not implemented\n%s",
		inst->getName(), err_si_isa_note);

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
		fatal("%s: Invalid or unsupported data format", __FUNCTION__);

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
	{
		fatal("%s: Invalid or unsupported data format", __FUNCTION__);
	}
	}

	return elem_size;
}

union hfpack
{
	uint32_t as_uint32;
	struct
	{
		uint16_t s1f;
		uint16_t s0f;
	} as_f16f16;
};

uint16_t WorkItem::Float32to16(float value)
{
	union Bits
	{
		float f;
		int32_t si;
		uint32_t ui;
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

float WorkItem::Float16to32(uint16_t value)
{
	union Bits
	{
		float f;
		int32_t si;
		uint32_t ui;
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
 * Public functions
 */

WorkItem::WorkItem(Wavefront *wavefront, int id)
{
	// Initialization
	this->id = id;
	this->wavefront = wavefront;
}

unsigned WorkItem::ReadSReg(int sreg_id)
{
	unsigned value;

	assert(sreg_id >= 0);
	assert(sreg_id != 104);
	assert(sreg_id != 105);
	assert(sreg_id != 125);
	assert((sreg_id < 209) || (sreg_id > 239));
	assert((sreg_id < 248) || (sreg_id > 250));
	assert(sreg_id != 254);
	assert(sreg_id < 256);

	if (sreg_id == SI_VCCZ)
	{
		if (wavefront->getSReg(SI_VCC).as_uint == 0 && 
			wavefront->getSReg(SI_VCC+1).as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	if (sreg_id == SI_EXECZ)
	{
		if (wavefront->getSReg(SI_EXEC).as_uint == 0 && 
			wavefront->getSReg(SI_EXEC+1).as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	else
	{
		value = wavefront->getSReg(sreg_id).as_uint;
	}

	// Statistics
	work_group->getSregReadCount()++;

	return value;
}


void WorkItem::WriteSReg(int sreg_id, 
	unsigned value)
{
	assert(sreg_id >= 0);
	assert(sreg_id != 104);
	assert(sreg_id != 105);
	assert(sreg_id != 125);
	assert((sreg_id < 209) || (sreg_id > 239));
	assert((sreg_id < 248) || (sreg_id > 250));
	assert(sreg_id != 254);
	assert(sreg_id < 256);

	wavefront->getSReg(sreg_id).as_uint = value;

	// Update VCCZ and EXECZ if necessary.
	if (sreg_id == SI_VCC || sreg_id == SI_VCC + 1)
	{
		wavefront->getSReg(SI_VCCZ).as_uint = 
			!wavefront->getSReg(SI_VCC).as_uint &
			!wavefront->getSReg(SI_VCC + 1).as_uint;
	}
	if (sreg_id == SI_EXEC || sreg_id == SI_EXEC + 1)
	{
		wavefront->getSReg(SI_EXECZ).as_uint = 
			!wavefront->getSReg(SI_EXEC).as_uint &
			!wavefront->getSReg(SI_EXEC + 1).as_uint;
	}

	// Statistics
	work_group->getSregWriteCount()++;
}


unsigned WorkItem::ReadVReg(int vreg_idx)
{
	assert(vreg_idx >= 0);
	assert(vreg_idx < 256);

	// Statistics
	work_group->getVregReadCount()++;

	return vreg[vreg_idx].as_uint;
}


void WorkItem::WriteVReg(int vreg_idx, 
	unsigned value)
{
	assert(vreg_idx >= 0);
	assert(vreg_idx < 256);
	vreg[vreg_idx].as_uint = value;

	// Statistics
	work_group->getVregWriteCount()++;
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


void WorkItem::WriteBitmaskSReg(int sreg_id, 
	unsigned value)
{
	unsigned mask = 1;
	unsigned bitfield;
	InstReg new_field;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		bitfield = ReadSReg(sreg_id);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		WriteSReg(sreg_id, new_field.as_uint);
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		bitfield = ReadSReg(sreg_id + 1);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		WriteSReg(sreg_id + 1, new_field.as_uint);
	}
}


int WorkItem::ReadBitmaskSReg(int sreg_id)
{
	unsigned mask = 1;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (ReadSReg(sreg_id) & mask) >> 
			id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (ReadSReg(sreg_id + 1) & mask) >> 
			(id_in_wavefront - 32);
	}
}


// Initialize a buffer resource descriptor
void WorkItem::ReadBufferResource(
	int sreg_id, EmuBufferDesc &buf_desc)
{
	// ((unsigned *)&buf_desc)[0] = wavefront->getSReg(sreg_id);
}


// Initialize a buffer resource descriptor
void WorkItem::ReadMemPtr(
	int sreg_id, EmuMemPtr &mem_ptr)
{
	// FIXME
	// assert(mem_ptr);

	// memcpy(mem_ptr, &wavefront->getSReg(sreg_id).as_uint, 
	// 	sizeof(unsigned int)*2);
}


}  // namespace SI
