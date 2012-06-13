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

	/* Return */
	return wavefront;
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

void si_wavefront_bitmask_cc(unsigned long long *cc, int id_in_wavefront, unsigned int value)
{
	unsigned long long mask = 1 << id_in_wavefront;
	*cc = (value) ? *cc | mask: *cc & ~mask;
}

void si_wavefront_init_sreg_with_value(struct si_wavefront_t *wavefront, int sreg, uint32_t value)
{
	wavefront->sgpr[sreg] = value;
}

void si_wavefront_init_sreg_with_cb(struct si_wavefront_t *wavefront, int first_reg, int num_regs, 
	int cb)
{
	struct si_buffer_resource_t res_desc;

	assert(num_regs == 4);
	assert(sizeof(struct si_buffer_resource_t) == 16);

	/* FIXME Populate rest of resource descriptor? */
	res_desc.base_addr = CONSTANT_MEMORY_START + cb*CONSTANT_BUFFER_SIZE;

	memcpy(&wavefront->sgpr[first_reg], &res_desc, 16);
}

void si_wavefront_init_sreg_with_uav_table(struct si_wavefront_t *wavefront, int first_reg, 
	int num_regs)
{
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(struct si_mem_ptr_t) == 8);

	mem_ptr.unused = 0;
	mem_ptr.addr = UAV_TABLE_START;

	memcpy(&wavefront->sgpr[first_reg], &mem_ptr, 8);
}
