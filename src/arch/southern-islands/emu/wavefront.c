/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <hash-table.h>

#include <southern-islands-asm.h>
#include <southern-islands-emu.h>



/*
 * Public Functions
 */


struct si_wavefront_t *si_wavefront_create()
{
	struct si_wavefront_t *wavefront;

	/* Allocate */
	wavefront = calloc(1, sizeof(struct si_wavefront_t));
	if (!wavefront)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	wavefront->pred = bit_map_create(si_emu_wavefront_size);
	si_wavefront_sreg_init(wavefront);

	/* Return */
	return wavefront;
}

/* Helper function for initializing the wavefront. */
void si_wavefront_sreg_init(struct si_wavefront_t *wavefront)
{
	union si_reg_t *sreg = &wavefront->sreg[0];

	/* Integer inline constants. */
	for(int i = 128; i < 193; i++)
		sreg[i].as_int = i - 128;
	for(int i = 193; i < 209; i++)
		sreg[i].as_int = -(i - 192);

	/* Inline floats. */
	sreg[240].as_float = 0.5;
	sreg[241].as_float = -0.5;
	sreg[242].as_float = 1.0;
	sreg[243].as_float = -1.0;
	sreg[244].as_float = 2.0;
	sreg[245].as_float = -2.0;
	sreg[246].as_float = 4.0;
	sreg[247].as_float = -4.0;
}



void si_wavefront_free(struct si_wavefront_t *wavefront)
{
	/* Free wavefront */
	bit_map_free(wavefront->pred);
	free(wavefront);
}


void si_wavefront_dump(struct si_wavefront_t *wavefront, FILE *f)
{
	struct si_ndrange_t *ndrange = wavefront->ndrange;
	struct si_work_group_t *work_group = wavefront->work_group;

	if (!f)
		return;
	
	/* Dump wavefront statistics in GPU report */
	fprintf(f, "[ NDRange[%d].Wavefront[%d] ]\n\n", ndrange->id, wavefront->id);

	fprintf(f, "Name = %s\n", wavefront->name);
	fprintf(f, "WorkGroup = %d\n", work_group->id);
	fprintf(f, "WorkItemFirst = %d\n", wavefront->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", wavefront->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", wavefront->work_item_count);
	fprintf(f, "\n");

	fprintf(f, "Inst_Count = %lld\n", wavefront->inst_count);
	fprintf(f, "Global_Mem_Inst_Count = %lld\n", wavefront->global_mem_inst_count);
	fprintf(f, "Local_Mem_Inst_Count = %lld\n", wavefront->local_mem_inst_count);
	fprintf(f, "\n");

	/* FIXME Count instruction statistics here */

	fprintf(f, "\n");
}


/* Execute one instruction in the wavefront */
void si_wavefront_execute(struct si_wavefront_t *wavefront)
{
	extern struct si_ndrange_t *si_isa_ndrange;
	extern struct si_work_group_t *si_isa_work_group;
	extern struct si_wavefront_t *si_isa_wavefront;
	extern struct si_work_item_t *si_isa_work_item;
	extern struct si_inst_t *si_isa_inst;

	struct si_ndrange_t *ndrange = wavefront->ndrange;

	int work_item_id;

	/* Get current work-group */
	si_isa_ndrange = wavefront->ndrange;
	si_isa_wavefront = wavefront;
	si_isa_work_group = wavefront->work_group;
	si_isa_work_item = NULL;
	si_isa_inst = NULL;
	assert(!DOUBLE_LINKED_LIST_MEMBER(si_isa_work_group, finished, si_isa_wavefront));

	/* Reset instruction flags */
	wavefront->global_mem_write = 0;
	wavefront->global_mem_read = 0;
	wavefront->local_mem_write = 0;
	wavefront->local_mem_read = 0;
	wavefront->pred_mask_update = 0;
	
	/* Grab the next instruction and update the pointer */
	si_isa_wavefront->inst_size = si_inst_decode(si_isa_wavefront->inst_buf, &si_isa_wavefront->inst);

	/* Increment the instruction pointer */
	si_isa_wavefront->inst_buf += si_isa_wavefront->inst_size;

	/* Stats */
	si_emu->inst_count++;
	si_isa_wavefront->emu_inst_count++;
	si_isa_wavefront->inst_count++;

	/* Set the current instruction */
	si_isa_inst = &si_isa_wavefront->inst;

	/* Execute the current instruction */
	switch (si_isa_inst->info->fmt)
	{

	/* Scalar Memory Instructions */
	case SI_FMT_SOP1:
	case SI_FMT_SOP2:
	case SI_FMT_SOPP:
	case SI_FMT_SMRD:
	{
		si_emu->scalar_inst_count++;
		si_isa_wavefront->scalar_inst_count++;

		/* Only one work item executes the instruction */
		si_isa_debug("\n");
		si_isa_work_item = si_isa_wavefront->scalar_work_item;
		(*si_isa_inst_func[si_isa_inst->info->inst])();
		si_isa_debug("\n");

		break;
	}

	/* Vector ALU Instructions */
	case SI_FMT_VOP2:
	case SI_FMT_VOP1:
	case SI_FMT_VOPC:
	case SI_FMT_VOP3a:
	case SI_FMT_VOP3b:
	{
		si_emu->vector_inst_count++;
		si_isa_wavefront->vector_inst_count++;
	
		si_isa_debug("\n");
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(si_isa_wavefront, work_item_id)
		{
			si_isa_work_item = ndrange->work_items[work_item_id];
			(*si_isa_inst_func[si_isa_inst->info->inst])();
		}
		si_isa_debug("\n");

		break;
	}

	/* Vector Memory Instructions */
	case SI_FMT_MTBUF:
	{
		si_emu->vector_inst_count++;
		si_isa_wavefront->vector_inst_count++;
	
		si_isa_debug("\n");
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(si_isa_wavefront, work_item_id)
		{
			si_isa_work_item = ndrange->work_items[work_item_id];
			(*si_isa_inst_func[si_isa_inst->info->inst])();
		}
		si_isa_debug("\n");

		break;
	}

	default:
	{
		fatal("%s: instruction type not implemented", __FUNCTION__);
		break;
	}

	}

	/* Check if wavefront finished kernel execution */
	if (si_isa_wavefront->finished)
	{
		assert(DOUBLE_LINKED_LIST_MEMBER(si_isa_work_group, running, si_isa_wavefront));
		assert(!DOUBLE_LINKED_LIST_MEMBER(si_isa_work_group, finished, si_isa_wavefront));
		DOUBLE_LINKED_LIST_REMOVE(si_isa_work_group, running, si_isa_wavefront);
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_isa_work_group, finished, si_isa_wavefront);

		/* Check if work-group finished kernel execution */
		if (si_isa_work_group->finished_list_count == si_isa_work_group->wavefront_count)
		{
			assert(DOUBLE_LINKED_LIST_MEMBER(ndrange, running, si_isa_work_group));
			assert(!DOUBLE_LINKED_LIST_MEMBER(ndrange, finished, si_isa_work_group));
			si_work_group_clear_status(si_isa_work_group, si_work_group_running);
			si_work_group_set_status(si_isa_work_group, si_work_group_finished);
		}
	}
}

void si_wavefront_bitmask_sreg(int sreg, int id_in_wavefront, union si_reg_t value)
{
	unsigned int mask = 1;
	if (id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		si_isa_wavefront->sreg[sreg].as_uint = (value.as_uint) ? si_isa_wavefront->sreg[sreg].as_uint | mask:
								 si_isa_wavefront->sreg[sreg].as_uint & ~mask;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		si_isa_wavefront->sreg[sreg + 1].as_uint = (value.as_uint) ? si_isa_wavefront->sreg[sreg + 1].as_uint | mask:
								     si_isa_wavefront->sreg[sreg + 1].as_uint & ~mask;
	}
}

void si_wavefront_init_sreg_with_value(struct si_wavefront_t *wavefront, int sreg, unsigned int value)
{
	wavefront->sreg[sreg].as_uint = value;
}

void si_wavefront_init_sreg_with_cb(struct si_wavefront_t *wavefront, int first_reg, int num_regs, 
	int cb)
{
	struct si_buffer_resource_t res_desc;

	assert(num_regs == 4);
	assert(sizeof(struct si_buffer_resource_t) == 16);

	/* FIXME Populate rest of resource descriptor? */
	res_desc.base_addr = CONSTANT_MEMORY_START + cb*CONSTANT_BUFFER_SIZE;

	memcpy(&wavefront->sreg[first_reg], &res_desc, 16);
}

void si_wavefront_init_sreg_with_uav_table(struct si_wavefront_t *wavefront, int first_reg, 
	int num_regs)
{
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(struct si_mem_ptr_t) == 8);

	mem_ptr.unused = 0;
	mem_ptr.addr = UAV_TABLE_START;

	memcpy(&wavefront->sreg[first_reg], &mem_ptr, 8);
}
