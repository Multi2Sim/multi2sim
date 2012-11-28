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

#include <lib/util/repos.h>
#include <mem-system/memory.h>

#include <arch/southern-islands/emu/isa.h>
#include <arch/southern-islands/emu/work-item.h>
#include <arch/southern-islands/emu/wavefront.h>

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
	/* Allocate instruction execution table */
	si_isa_inst_func = calloc(SI_INST_COUNT, sizeof(si_isa_inst_func_t));
	if (!si_isa_inst_func)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size) \
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

union si_reg_t si_isa_read_sreg(struct si_work_item_t *work_item, int sreg)
{
	return work_item->wavefront->sreg[sreg];
}

void si_isa_write_sreg(struct si_work_item_t *work_item, int sreg, union si_reg_t value)
{
	work_item->wavefront->sreg[sreg] = value;

	/* Update VCCZ and EXECZ if necessary. */
	if (sreg == SI_VCC || sreg == SI_VCC + 1)
		work_item->wavefront->sreg[SI_VCCZ].as_uint = !work_item->wavefront->sreg[SI_VCC].as_uint &
														!work_item->wavefront->sreg[SI_VCC + 1].as_uint;
	if (sreg == SI_EXEC || sreg == SI_EXEC + 1)
		work_item->wavefront->sreg[SI_EXECZ].as_uint = !work_item->wavefront->sreg[SI_EXEC].as_uint &
														!work_item->wavefront->sreg[SI_EXEC + 1].as_uint;
}

union si_reg_t si_isa_read_vreg(struct si_work_item_t *work_item, int vreg)
{
	return work_item->vreg[vreg];
}

void si_isa_write_vreg(struct si_work_item_t *work_item, int vreg, union si_reg_t value)
{
	work_item->vreg[vreg] = value;
}

union si_reg_t si_isa_read_reg(struct si_work_item_t *work_item, int reg)
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

void si_isa_bitmask_sreg(struct si_work_item_t *work_item, int sreg, union si_reg_t value)
{
	unsigned int mask = 1;
	unsigned int bitfield;
	union si_reg_t new_field;
	if (work_item->id_in_wavefront < 32)
	{
		mask <<= work_item->id_in_wavefront;
		bitfield = si_isa_read_sreg(work_item, sreg).as_uint;
		new_field.as_uint = (value.as_uint) ? bitfield | mask: bitfield & ~mask;
		si_isa_write_sreg(work_item, sreg, new_field);
	}
	else
	{
		mask <<= (work_item->id_in_wavefront - 32);
		bitfield = si_isa_read_sreg(work_item, sreg + 1).as_uint;
		new_field.as_uint = (value.as_uint) ? bitfield | mask: bitfield & ~mask;
		si_isa_write_sreg(work_item, sreg + 1, new_field);
	}
}

int si_isa_read_bitmask_sreg(struct si_work_item_t *work_item, int sreg)
{
	unsigned int mask = 1;
	if (work_item->id_in_wavefront < 32)
	{
		mask <<= work_item->id_in_wavefront;
		return (si_isa_read_sreg(work_item, sreg).as_uint & mask) >> work_item->id_in_wavefront;
	}
	else
	{
		mask <<= (work_item->id_in_wavefront - 32);
		return (si_isa_read_sreg(work_item, sreg + 1).as_uint & mask) >> (work_item->id_in_wavefront - 32);
	}
}

/* Initialize a buffer resource descriptor */
void si_isa_read_buf_res(struct si_work_item_t *work_item, struct si_buffer_resource_t *buf_desc, int sreg)
{
	assert(buf_desc);

	memcpy(buf_desc, &work_item->wavefront->sreg[sreg].as_uint, sizeof(unsigned int)*4);
}

/* Initialize a buffer resource descriptor */
void si_isa_read_mem_ptr(struct si_work_item_t *work_item, struct si_mem_ptr_t *mem_ptr, int sreg)
{
	assert(mem_ptr);

	memcpy(mem_ptr, &work_item->wavefront->sreg[sreg].as_uint, sizeof(unsigned int)*2);
}


/*
 * Constant Memory
 */

void si_isa_const_mem_write(int buffer, int offset, void *pvalue)
{
	uint32_t addr; 

	assert(buffer < CONSTANT_BUFFERS);
	assert(offset <= CONSTANT_BUFFER_SIZE - 4);

	addr = CONSTANT_MEMORY_START + buffer*CONSTANT_BUFFER_SIZE + offset;

	/* Write */
    mem_write(si_emu->global_mem, addr, 4, pvalue);
}


void si_isa_const_mem_read(int buffer, int offset, void *pvalue)
{
	uint32_t addr; 

	assert(buffer < CONSTANT_BUFFERS);
	
	addr = CONSTANT_MEMORY_START + buffer*CONSTANT_BUFFER_SIZE + offset;

        /* Read */
        mem_read(si_emu->global_mem, addr, 4, pvalue);
}

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
	{
		fatal("%s: Invalid or unsupported data format", __FUNCTION__);
	}
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
