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
#include <lib/util/debug.h>
#include <lib/util/repos.h>

#include "emu.h"
#include "isa.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"


/* Repository of deferred tasks */
struct repos_t *si_isa_write_task_repos;

/* Instruction execution table */
si_isa_inst_func_t *si_isa_inst_func;

/* Debug */
int si_isa_debug_category;




/*
 * Initialization, finalization
 */


/* Initialization */
void si_isa_init()
{
	/* Initialize */
	si_isa_inst_func = xcalloc(SI_INST_COUNT, sizeof(si_isa_inst_func_t));
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	extern void si_isa_##_name##_impl(struct si_work_item_t *work_item, struct si_inst_t *inst); \
	si_isa_inst_func[SI_INST_##_name] = si_isa_##_name##_impl;
#include <arch/southern-islands/asm/asm.dat>
#undef DEFINST

	/* Repository of deferred tasks */
	si_isa_write_task_repos = repos_create(sizeof(struct si_isa_write_task_t),
		"gpu_isa_write_task_repos");
}

void si_isa_done()
{
	/* Instruction execution table */
	free(si_isa_inst_func);

	/* Repository of deferred tasks */
	repos_free(si_isa_write_task_repos);
}


/* Helper functions */

unsigned int si_isa_read_sreg(struct si_work_item_t *work_item, int sreg)
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
		if (work_item->wavefront->sreg[SI_VCC].as_uint == 0 && 
			work_item->wavefront->sreg[SI_VCC+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	if (sreg == SI_EXECZ)
	{
		if (work_item->wavefront->sreg[SI_EXEC].as_uint == 0 && 
			work_item->wavefront->sreg[SI_EXEC+1].as_uint == 0)
			value = 1;
		else 
			value = 0;
	}
	else
	{
		value = work_item->wavefront->sreg[sreg].as_uint;
	}

	/* Statistics */
	work_item->work_group->sreg_read_count++;

	return value;
}

void si_isa_write_sreg(struct si_work_item_t *work_item, int sreg, 
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

	work_item->wavefront->sreg[sreg].as_uint = value;

	/* Update VCCZ and EXECZ if necessary. */
	if (sreg == SI_VCC || sreg == SI_VCC + 1)
	{
		work_item->wavefront->sreg[SI_VCCZ].as_uint = 
			!work_item->wavefront->sreg[SI_VCC].as_uint &
			!work_item->wavefront->sreg[SI_VCC + 1].as_uint;
	}
	if (sreg == SI_EXEC || sreg == SI_EXEC + 1)
	{
		work_item->wavefront->sreg[SI_EXECZ].as_uint = 
			!work_item->wavefront->sreg[SI_EXEC].as_uint &
			!work_item->wavefront->sreg[SI_EXEC + 1].as_uint;
	}

	/* Statistics */
	work_item->work_group->sreg_write_count++;
}

unsigned int si_isa_read_vreg(struct si_work_item_t *work_item, int vreg)
{
	assert(vreg >= 0);
	assert(vreg < 256);

	/* Statistics */
	work_item->work_group->vreg_read_count++;

	return work_item->vreg[vreg].as_uint;
}

void si_isa_write_vreg(struct si_work_item_t *work_item, int vreg, 
	unsigned int value)
{
	assert(vreg >= 0);
	assert(vreg < 256);
	work_item->vreg[vreg].as_uint = value;

	/* Statistics */
	work_item->work_group->vreg_write_count++;
}

unsigned int si_isa_read_reg(struct si_work_item_t *work_item, int reg)
{
	if (reg < 256)
	{
		return si_isa_read_sreg(work_item, reg);
	}
	else
	{
		return si_isa_read_vreg(work_item, reg - 256);
	}
}

void si_isa_bitmask_sreg(struct si_work_item_t *work_item, int sreg, 
	unsigned int value)
{
	unsigned int mask = 1;
	unsigned int bitfield;
	union si_reg_t new_field;
	if (work_item->id_in_wavefront < 32)
	{
		mask <<= work_item->id_in_wavefront;
		bitfield = si_isa_read_sreg(work_item, sreg);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		si_isa_write_sreg(work_item, sreg, new_field.as_uint);
	}
	else
	{
		mask <<= (work_item->id_in_wavefront - 32);
		bitfield = si_isa_read_sreg(work_item, sreg + 1);
		new_field.as_uint = (value) ? bitfield | mask: bitfield & ~mask;
		si_isa_write_sreg(work_item, sreg + 1, new_field.as_uint);
	}
}

int si_isa_read_bitmask_sreg(struct si_work_item_t *work_item, int sreg)
{
	unsigned int mask = 1;
	if (work_item->id_in_wavefront < 32)
	{
		mask <<= work_item->id_in_wavefront;
		return (si_isa_read_sreg(work_item, sreg) & mask) >> 
			work_item->id_in_wavefront;
	}
	else
	{
		mask <<= (work_item->id_in_wavefront - 32);
		return (si_isa_read_sreg(work_item, sreg + 1) & mask) >> 
			(work_item->id_in_wavefront - 32);
	}
}

/* Initialize a buffer resource descriptor */
void si_isa_read_buf_res(struct si_work_item_t *work_item, 
	struct si_buffer_desc_t *buf_desc, int sreg)
{
	assert(buf_desc);

	memcpy(buf_desc, &work_item->wavefront->sreg[sreg].as_uint, 
		sizeof(unsigned int)*4);
}

/* Initialize a buffer resource descriptor */
void si_isa_read_mem_ptr(struct si_work_item_t *work_item, 
	struct si_mem_ptr_t *mem_ptr, int sreg)
{
	assert(mem_ptr);

	memcpy(mem_ptr, &work_item->wavefront->sreg[sreg].as_uint, 
		sizeof(unsigned int)*2);
}



/*
 * Constant Memory
 */

/* Used for allocating CB0 and CB1 */
unsigned int si_isa_const_mem_allocate(unsigned int size)
{
        unsigned int ptr;

        /* Assign position in device global memory */
        ptr = si_emu->video_mem_top;
        si_emu->video_mem_top += size;

        return ptr;
}


/* 
 * Southern Islands data types
 */
int si_isa_get_num_elems(int data_format)
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

int si_isa_get_elem_size(int data_format)
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
