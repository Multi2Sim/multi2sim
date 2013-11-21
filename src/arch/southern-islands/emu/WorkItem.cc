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

int WorkItem::IsaGetNumElems(int data_format) const
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

int WorkItem::IsaGetElemSize(int data_format) const
{
	int elem_size;

	switch (data_format)
	{

	/* 8-bit data */
	case 1:
	case 3:
	case 10:
	{
		elem_size = 1;
		break;
	}

	/* 16-bit data */
	case 2:
	case 5:
	case 12:
	{
		elem_size = 2;
		break;
	}

	/* 32-bit data */
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

/*
 * Public functions
 */

WorkItem::WorkItem(Wavefront *wavefront, int id)
{
	/* Initialization */
	this->id = id;
	this->wavefront = wavefront;
}

unsigned WorkItem::ReadSReg(int sreg_idx)
{
	unsigned value;

	assert(sreg_idx >= 0);
	assert(sreg_idx != 104);
	assert(sreg_idx != 105);
	assert(sreg_idx != 125);
	assert((sreg_idx < 209) || (sreg_idx > 239));
	assert((sreg_idx < 248) || (sreg_idx > 250));
	assert(sreg_idx != 254);
	assert(sreg_idx < 256);

	if (sreg_idx == SI_VCCZ)
	{
		if (wavefront->getSReg(SI_VCC).as_uint == 0 && 
			wavefront->getSReg(SI_VCC+1).as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	if (sreg_idx == SI_EXECZ)
	{
		if (wavefront->getSReg(SI_EXEC).as_uint == 0 && 
			wavefront->getSReg(SI_EXEC+1).as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	else
	{
		value = wavefront->getSReg(sreg_idx).as_uint;
	}

	/* Statistics */
	work_group->getSregReadCount()++;

	return value;
}


void WorkItem::WriteSReg(int sreg_idx, 
	unsigned value)
{
	assert(sreg_idx >= 0);
	assert(sreg_idx != 104);
	assert(sreg_idx != 105);
	assert(sreg_idx != 125);
	assert((sreg_idx < 209) || (sreg_idx > 239));
	assert((sreg_idx < 248) || (sreg_idx > 250));
	assert(sreg_idx != 254);
	assert(sreg_idx < 256);

	wavefront->getSReg(sreg_idx).as_uint = value;

	/* Update VCCZ and EXECZ if necessary. */
	if (sreg_idx == SI_VCC || sreg_idx == SI_VCC + 1)
	{
		wavefront->getSReg(SI_VCCZ).as_uint = 
			!wavefront->getSReg(SI_VCC).as_uint &
			!wavefront->getSReg(SI_VCC + 1).as_uint;
	}
	if (sreg_idx == SI_EXEC || sreg_idx == SI_EXEC + 1)
	{
		wavefront->getSReg(SI_EXECZ).as_uint = 
			!wavefront->getSReg(SI_EXEC).as_uint &
			!wavefront->getSReg(SI_EXEC + 1).as_uint;
	}

	/* Statistics */
	work_group->getSregWriteCount()++;
}


unsigned WorkItem::ReadVReg(int vreg_idx)
{
	assert(vreg_idx >= 0);
	assert(vreg_idx < 256);

	/* Statistics */
	work_group->getVregReadCount()++;

	return vreg[vreg_idx].as_uint;
}


void WorkItem::WriteVReg(int vreg_idx, 
	unsigned value)
{
	assert(vreg_idx >= 0);
	assert(vreg_idx < 256);
	vreg[vreg_idx].as_uint = value;

	/* Statistics */
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


void WorkItem::WriteBitmaskSReg(int sreg_idx, 
	unsigned value)
{
	unsigned mask = 1;
	unsigned bitfield;
	InstReg new_field;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		bitfield = ReadSReg(sreg_idx);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		WriteSReg(sreg_idx, new_field.as_uint);
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		bitfield = ReadSReg(sreg_idx + 1);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		WriteSReg(sreg_idx + 1, new_field.as_uint);
	}
}


int WorkItem::ReadBitmaskSReg(int sreg_idx)
{
	unsigned mask = 1;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (ReadSReg(sreg_idx) & mask) >> 
			id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (ReadSReg(sreg_idx + 1) & mask) >> 
			(id_in_wavefront - 32);
	}
}


/* Initialize a buffer resource descriptor */
void WorkItem::ReadBufferResource(
	int sreg_idx, EmuBufferDesc &buf_desc)
{
	// FIXME
	// assert(buf_desc);

	// memcpy(buf_desc, &wavefront->getSReg(sreg_idx).as_uint, 
	// 	sizeof(unsigned int)*4);
}


/* Initialize a buffer resource descriptor */
void WorkItem::ReadMemPtr(
	int sreg_idx, EmuMemPtr &mem_ptr)
{
	// FIXME
	// assert(mem_ptr);

	// memcpy(mem_ptr, &wavefront->getSReg(sreg_idx).as_uint, 
	// 	sizeof(unsigned int)*2);
}


#if 0

#include <lib/mhandle/mhandle.h>

#include "isa.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"



/*
 * Class 'SIWorkItem'
 */

void SIWorkItemCreate(SIWorkItem *self, int id, SIWavefront *wavefront)
{
	/* Initialize */
	self->id = id;
	self->wavefront = wavefront;
}


void SIWorkItemDestroy(SIWorkItem *self)
{
}


unsigned int SIWorkItemReadSReg(SIWorkItem *self, int sreg_idx)
{
	unsigned int value;

	assert(sreg_idx >= 0);
	assert(sreg_idx != 104);
	assert(sreg_idx != 105);
	assert(sreg_idx != 125);
	assert((sreg_idx < 209) || (sreg_idx > 239));
	assert((sreg_idx < 248) || (sreg_idx > 250));
	assert(sreg_idx != 254);
	assert(sreg_idx < 256);

	if (sreg_idx == SI_VCCZ)
	{
		if (self->wavefront->sreg_idx[SI_VCC].as_uint == 0 && 
			self->wavefront->sreg_idx[SI_VCC+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	if (sreg_idx == SI_EXECZ)
	{
		if (self->wavefront->sreg_idx[SI_EXEC].as_uint == 0 && 
			self->wavefront->sreg_idx[SI_EXEC+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	else
	{
		value = self->wavefront->sreg_idx[sreg_idx].as_uint;
	}

	/* Statistics */
	self->work_group->sreg_idx_read_count++;

	return value;
}


void SIWorkItemWriteSReg(SIWorkItem *self, int sreg_idx, 
	unsigned int value)
{
	assert(sreg_idx >= 0);
	assert(sreg_idx != 104);
	assert(sreg_idx != 105);
	assert(sreg_idx != 125);
	assert((sreg_idx < 209) || (sreg_idx > 239));
	assert((sreg_idx < 248) || (sreg_idx > 250));
	assert(sreg_idx != 254);
	assert(sreg_idx < 256);

	self->wavefront->sreg_idx[sreg_idx].as_uint = value;

	/* Update VCCZ and EXECZ if necessary. */
	if (sreg_idx == SI_VCC || sreg_idx == SI_VCC + 1)
	{
		self->wavefront->sreg_idx[SI_VCCZ].as_uint = 
			!self->wavefront->sreg_idx[SI_VCC].as_uint &
			!self->wavefront->sreg_idx[SI_VCC + 1].as_uint;
	}
	if (sreg_idx == SI_EXEC || sreg_idx == SI_EXEC + 1)
	{
		self->wavefront->sreg_idx[SI_EXECZ].as_uint = 
			!self->wavefront->sreg_idx[SI_EXEC].as_uint &
			!self->wavefront->sreg_idx[SI_EXEC + 1].as_uint;
	}

	/* Statistics */
	self->work_group->sreg_idx_write_count++;
}


unsigned int SIWorkItemReadVReg(SIWorkItem *self, int vreg)
{
	assert(vreg >= 0);
	assert(vreg < 256);

	/* Statistics */
	self->work_group->vreg_read_count++;

	return self->vreg[vreg_idx].as_uint;
}


void SIWorkItemWriteVReg(SIWorkItem *self, int vreg_idx, 
	unsigned int value)
{
	assert(vreg >= 0);
	assert(vreg < 256);
	self->vreg[vreg_idx].as_uint = value;

	/* Statistics */
	self->work_group->vreg_write_count++;
}


unsigned int SIWorkItemReadReg(SIWorkItem *self, int reg)
{
	if (reg < 256)
	{
		return SIWorkItemReadSReg(self, reg);
	}
	else
	{
		return SIWorkItemReadVReg(reg - 256);
	}
}


void SIWorkItemWriteBitmaskSReg(SIWorkItem *self, int sreg_idx, 
	unsigned int value)
{
	unsigned int mask = 1;
	unsigned int bitfield;
	SIInstReg new_field;
	if (self->id_in_wavefront < 32)
	{
		mask <<= self->id_in_wavefront;
		bitfield = SIWorkItemReadSReg(self, sreg_idx);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		SIWorkItemWriteSReg(self, sreg_idx, new_field.as_uint);
	}
	else
	{
		mask <<= (self->id_in_wavefront - 32);
		bitfield = SIWorkItemReadSReg(self, sreg_idx + 1);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		SIWorkItemWriteSReg(self, sreg_idx + 1, new_field.as_uint);
	}
}


int SIWorkItemReadBitmaskSReg(SIWorkItem *self, int sreg_idx)
{
	unsigned int mask = 1;
	if (self->id_in_wavefront < 32)
	{
		mask <<= self->id_in_wavefront;
		return (SIWorkItemReadSReg(self, sreg_idx) & mask) >> 
			self->id_in_wavefront;
	}
	else
	{
		mask <<= (self->id_in_wavefront - 32);
		return (SIWorkItemReadSReg(self, sreg_idx + 1) & mask) >> 
			(self->id_in_wavefront - 32);
	}
}


/* Initialize a buffer resource descriptor */
void SIWorkItemReadBufferResource(SIWorkItem *self, 
	struct si_buffer_desc_t *buf_desc, int sreg_idx)
{
	assert(buf_desc);

	memcpy(buf_desc, &self->wavefront->sreg_idx[sreg_idx].as_uint, 
		sizeof(unsigned int)*4);
}


/* Initialize a buffer resource descriptor */
void SIWorkItemReadMemPtr(SIWorkItem *self, 
	struct si_mem_ptr_t *mem_ptr, int sreg_idx)
{
	assert(mem_ptr);

	memcpy(mem_ptr, &self->wavefront->sreg_idx[sreg_idx].as_uint, 
		sizeof(unsigned int)*2);
}


#endif

}  /* namespace SI */
