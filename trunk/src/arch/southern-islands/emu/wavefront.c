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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>

#include "emu.h"
#include "wavefront.h"
#include "isa.h"
#include "ndrange.h"
#include "work-group.h"
#include "work-item.h"



/*
 * Public Functions
 */


struct si_wavefront_t *si_wavefront_create()
{
	struct si_wavefront_t *wavefront;

	/* Initialize */
	wavefront = xcalloc(1, sizeof(struct si_wavefront_t));
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
	struct si_ndrange_t *ndrange;
	struct si_work_group_t *work_group;
	struct si_work_item_t *work_item;
	struct si_inst_t *inst;

	char inst_dump[MAX_INST_STR_SIZE];
	unsigned int pc;

	ndrange = wavefront->ndrange;

	int work_item_id;

	/* Get current work-group */
	ndrange = wavefront->ndrange;
	work_group = wavefront->work_group;
	work_item = NULL;
	inst = NULL;
	assert(!DOUBLE_LINKED_LIST_MEMBER(work_group, finished, wavefront));

	/* Reset instruction flags */
	wavefront->vector_mem_write = 0;
	wavefront->vector_mem_read = 0;
	wavefront->scalar_mem_read = 0;
	wavefront->local_mem_write = 0;
	wavefront->local_mem_read = 0;
	wavefront->pred_mask_update = 0;
	wavefront->mem_wait = 0;
	wavefront->barrier = 0;

	assert(!wavefront->finished);
	
	/* Grab the next instruction and update the pointer */
	wavefront->inst_size = si_inst_decode(wavefront->wavefront_pool, &wavefront->inst);

	/* Stats */
	si_emu->inst_count++;
	wavefront->emu_inst_count++;
	wavefront->inst_count++;

	/* Set the current instruction */
	inst = &wavefront->inst;
	pc = wavefront->wavefront_pool - wavefront->wavefront_pool_start;

	/* Execute the current instruction */
	switch (inst->info->fmt)
	{

	/* Scalar ALU Instructions */
	case SI_FMT_SOP1:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_sop1(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->scalar_alu_inst_count++;
		wavefront->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = wavefront->scalar_work_item;
		(*si_isa_inst_func[inst->info->inst])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_SOP2:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_sop2(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->scalar_alu_inst_count++;
		wavefront->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = wavefront->scalar_work_item;
		(*si_isa_inst_func[inst->info->inst])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_SOPP:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_sopp(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		if (wavefront->inst.micro_inst.sopp.op > 1 &&
						wavefront->inst.micro_inst.sopp.op < 10)
		{
			si_emu->branch_inst_count++;
			wavefront->branch_inst_count++;
		} else
		{
			si_emu->scalar_alu_inst_count++;
			wavefront->scalar_alu_inst_count++;
		}

		/* Only one work item executes the instruction */
		work_item = wavefront->scalar_work_item;
		(*si_isa_inst_func[inst->info->inst])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_SOPC:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_sopc(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->scalar_alu_inst_count++;
		wavefront->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = wavefront->scalar_work_item;
		(*si_isa_inst_func[inst->info->inst])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_SOPK:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_sopk(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->scalar_alu_inst_count++;
		wavefront->scalar_alu_inst_count++;

		/* Only one work item executes the instruction */
		work_item = wavefront->scalar_work_item;
		(*si_isa_inst_func[inst->info->inst])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	/* Scalar Memory Instructions */
	case SI_FMT_SMRD:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_smrd(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->scalar_mem_inst_count++;
		wavefront->scalar_mem_inst_count++;

		/* Only one work item executes the instruction */
		work_item = wavefront->scalar_work_item;
		(*si_isa_inst_func[inst->info->inst])(work_item, inst);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	/* Vector ALU Instructions */
	case SI_FMT_VOP2:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_vop2(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_VOP1:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_vop1(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;

		if (inst->micro_inst.vop1.op == 2)
		{
			/* Instruction ignores execution mask and is only executed on one work item.
			 * Execute on the first active work item from the least significant bit in EXEC.
			 * (if exec is 0, execute work item 0) */
			work_item = ndrange->work_items[wavefront->work_item_id_first];
			if (si_isa_read_sreg(work_item, SI_EXEC) == 0 && si_isa_read_sreg(work_item, SI_EXEC + 1) == 0)
			{
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
			}
			else {
				SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
				{
					work_item = ndrange->work_items[work_item_id];
					if(si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
					{
						(*si_isa_inst_func[inst->info->inst])(work_item, inst);
						break;
					}
				}
			}
		}
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_VOPC:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_vopc(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}
	
	case SI_FMT_VOP3a:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_vop3(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_VOP3b:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_vop3(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_DS:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_ds(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->local_mem_inst_count++;
		wavefront->local_mem_inst_count++;

		/* Record access type */
		if ((inst->info->opcode >= 13 && inst->info->opcode < 16) ||
			(inst->info->opcode >= 30 && inst->info->opcode < 32) ||
			(inst->info->opcode >= 77 && inst->info->opcode < 80))
		{
			wavefront->local_mem_write = 1;
		}
		else if ((inst->info->opcode >= 54 && inst->info->opcode < 61) ||
			(inst->info->opcode >= 118 && inst->info->opcode < 120))
		{
			wavefront->local_mem_read = 1;
		}
		else 
		{
			fatal("%s: unimplemented LDS opcode", __FUNCTION__);
		}

		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	/* Vector Memory Instructions */
	case SI_FMT_MTBUF:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump_mtbuf(inst, wavefront->inst_size, pc, wavefront->wavefront_pool, inst_dump,
				MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_mem_inst_count++;
		wavefront->vector_mem_inst_count++;

		/* Record access type */
		if (inst->info->opcode >= 0 && inst->info->opcode < 4)
			wavefront->vector_mem_read = 1;
		else if (inst->info->opcode >= 4 && inst->info->opcode < 8)
			wavefront->vector_mem_write = 1;
		else 
			fatal("%s: invalid mtbuf opcode", __FUNCTION__);
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			if (si_wavefront_work_item_active(wavefront, work_item->id_in_wavefront))
				(*si_isa_inst_func[inst->info->inst])(work_item, inst);
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	default:
	{
		fatal("%s: instruction type not implemented", __FUNCTION__);
		break;
	}

	}

	/* Check if wavefront finished kernel execution */
	if (wavefront->finished)
	{
		assert(DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront));
		assert(!DOUBLE_LINKED_LIST_MEMBER(work_group, finished, wavefront));
		DOUBLE_LINKED_LIST_REMOVE(work_group, running, wavefront);
		DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, finished, wavefront);

		/* Check if work-group finished kernel execution */
		if (work_group->finished_list_count == work_group->wavefront_count)
		{
			assert(DOUBLE_LINKED_LIST_MEMBER(ndrange, running, work_group));
			assert(!DOUBLE_LINKED_LIST_MEMBER(ndrange, finished, work_group));
			si_work_group_clear_status(work_group, si_work_group_running);
			si_work_group_set_status(work_group, si_work_group_finished);

			/* Check if ND-Range finished kernel execution */
			if (ndrange->finished_list_count == ndrange->work_group_count)
			{
				assert(DOUBLE_LINKED_LIST_MEMBER(si_emu, running_ndrange, ndrange));
				assert(!DOUBLE_LINKED_LIST_MEMBER(si_emu, finished_ndrange, ndrange));
				si_ndrange_clear_status(ndrange, si_ndrange_running);
				si_ndrange_set_status(ndrange, si_ndrange_finished);
			}
		}
	}

	/* Increment the instruction pointer */
	wavefront->wavefront_pool += wavefront->inst_size;
}

int si_wavefront_work_item_active(struct si_wavefront_t *wavefront, int id_in_wavefront) {
	int mask = 1;
	if(id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (wavefront->sreg[SI_EXEC].as_uint & mask) >> id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (wavefront->sreg[SI_EXEC + 1].as_uint & mask) >> (id_in_wavefront - 32);
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
