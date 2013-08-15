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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


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


unsigned int SIWorkItemReadSReg(SIWorkItem *self, int sreg)
{
	unsigned int value;

	assert(sreg >= 0);
	assert(sreg != 104);
	assert(sreg != 105);
	assert(sreg != 125);
	assert((sreg < 209) || (sreg > 239));
	assert((sreg < 248) || (sreg > 250));
	assert(sreg != 254);
	assert(sreg < 256);

	if (sreg == SI_VCCZ)
	{
		if (self->wavefront->sreg[SI_VCC].as_uint == 0 && 
			self->wavefront->sreg[SI_VCC+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	if (sreg == SI_EXECZ)
	{
		if (self->wavefront->sreg[SI_EXEC].as_uint == 0 && 
			self->wavefront->sreg[SI_EXEC+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	else
	{
		value = self->wavefront->sreg[sreg].as_uint;
	}

	/* Statistics */
	self->work_group->sreg_read_count++;

	return value;
}


void SIWorkItemWriteSReg(SIWorkItem *self, int sreg, 
	unsigned int value)
{
	assert(sreg >= 0);
	assert(sreg != 104);
	assert(sreg != 105);
	assert(sreg != 125);
	assert((sreg < 209) || (sreg > 239));
	assert((sreg < 248) || (sreg > 250));
	assert(sreg != 254);
	assert(sreg < 256);

	self->wavefront->sreg[sreg].as_uint = value;

	/* Update VCCZ and EXECZ if necessary. */
	if (sreg == SI_VCC || sreg == SI_VCC + 1)
	{
		self->wavefront->sreg[SI_VCCZ].as_uint = 
			!self->wavefront->sreg[SI_VCC].as_uint &
			!self->wavefront->sreg[SI_VCC + 1].as_uint;
	}
	if (sreg == SI_EXEC || sreg == SI_EXEC + 1)
	{
		self->wavefront->sreg[SI_EXECZ].as_uint = 
			!self->wavefront->sreg[SI_EXEC].as_uint &
			!self->wavefront->sreg[SI_EXEC + 1].as_uint;
	}

	/* Statistics */
	self->work_group->sreg_write_count++;
}


unsigned int SIWorkItemReadVReg(SIWorkItem *self, int vreg)
{
	assert(vreg >= 0);
	assert(vreg < 256);

	/* Statistics */
	self->work_group->vreg_read_count++;

	return self->vreg[vreg].as_uint;
}


void SIWorkItemWriteVReg(SIWorkItem *self, int vreg, 
	unsigned int value)
{
	assert(vreg >= 0);
	assert(vreg < 256);
	self->vreg[vreg].as_uint = value;

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
		return SIWorkItemReadVReg(self, reg - 256);
	}
}


void SIWorkItemWriteBitmaskSReg(SIWorkItem *self, int sreg, 
	unsigned int value)
{
	unsigned int mask = 1;
	unsigned int bitfield;
	SIInstReg new_field;
	if (self->id_in_wavefront < 32)
	{
		mask <<= self->id_in_wavefront;
		bitfield = SIWorkItemReadSReg(self, sreg);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		SIWorkItemWriteSReg(self, sreg, new_field.as_uint);
	}
	else
	{
		mask <<= (self->id_in_wavefront - 32);
		bitfield = SIWorkItemReadSReg(self, sreg + 1);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		SIWorkItemWriteSReg(self, sreg + 1, new_field.as_uint);
	}
}


int SIWorkItemReadBitmaskSReg(SIWorkItem *self, int sreg)
{
	unsigned int mask = 1;
	if (self->id_in_wavefront < 32)
	{
		mask <<= self->id_in_wavefront;
		return (SIWorkItemReadSReg(self, sreg) & mask) >> 
			self->id_in_wavefront;
	}
	else
	{
		mask <<= (self->id_in_wavefront - 32);
		return (SIWorkItemReadSReg(self, sreg + 1) & mask) >> 
			(self->id_in_wavefront - 32);
	}
}


/* Initialize a buffer resource descriptor */
void SIWorkItemReadBufferResource(SIWorkItem *self, 
	struct si_buffer_desc_t *buf_desc, int sreg)
{
	assert(buf_desc);

	memcpy(buf_desc, &self->wavefront->sreg[sreg].as_uint, 
		sizeof(unsigned int)*4);
}


/* Initialize a buffer resource descriptor */
void SIWorkItemReadMemPtr(SIWorkItem *self, 
	struct si_mem_ptr_t *mem_ptr, int sreg)
{
	assert(mem_ptr);

	memcpy(mem_ptr, &self->wavefront->sreg[sreg].as_uint, 
		sizeof(unsigned int)*2);
}


