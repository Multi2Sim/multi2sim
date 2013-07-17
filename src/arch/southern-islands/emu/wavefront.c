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
#include <mem-system/memory.h>

#include "isa.h"
#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"



/*
 * Public Functions
 */


struct si_wavefront_t *si_wavefront_create(int wavefront_id, 
	struct si_work_group_t *work_group)
{
	struct si_wavefront_t *wavefront;

	int work_item_id;

	/* Initialize */
	wavefront = xcalloc(1, sizeof(struct si_wavefront_t));
	wavefront->id = wavefront_id;
	si_wavefront_sreg_init(wavefront);

	/* Create work items */
	wavefront->work_items = xcalloc(si_emu_wavefront_size, sizeof(void *));
	SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
	{
		wavefront->work_items[work_item_id] = si_work_item_create();

		wavefront->work_items[work_item_id]->wavefront = wavefront;
		wavefront->work_items[work_item_id]->id_in_wavefront = 
			work_item_id;
		wavefront->work_items[work_item_id]->work_group = work_group;
	}

	/* Create scalar work item */
	wavefront->scalar_work_item = si_work_item_create();
	wavefront->scalar_work_item->wavefront = wavefront;
	wavefront->scalar_work_item->work_group = work_group;

	/* Assign the work group */
	wavefront->work_group = work_group;

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
	assert(wavefront);

	/* Free wavefront */
	
	free(wavefront->work_items);

	memset(wavefront, 0, sizeof(struct si_wavefront_t));
	free(wavefront);

	wavefront = NULL;
}

/* Execute the next instruction for the wavefront */
void si_wavefront_execute(struct si_wavefront_t *wavefront)
{
	struct si_ndrange_t *ndrange;
	struct si_work_group_t *work_group;
	struct si_work_item_t *work_item;
	struct si_inst_t *inst;

	char inst_dump[MAX_INST_STR_SIZE];

	ndrange = wavefront->work_group->ndrange;

	int work_item_id;

	/* Get current work-group */
	ndrange = wavefront->work_group->ndrange;
	work_group = wavefront->work_group;
	work_item = NULL;
	inst = NULL;

	/* Reset instruction flags */
	wavefront->vector_mem_write = 0;
	wavefront->vector_mem_read = 0;
	wavefront->scalar_mem_read = 0;
	wavefront->lds_write = 0;
	wavefront->lds_read = 0;
	wavefront->mem_wait = 0;
	wavefront->at_barrier = 0;
	wavefront->barrier_inst = 0;
	wavefront->vector_mem_glc = 0;

	assert(!wavefront->finished);
	
	/* Grab the instruction at PC and update the pointer */
	wavefront->inst_size = si_inst_decode(
		ndrange->inst_buffer + wavefront->pc,
		&wavefront->inst, 0);

	/* Stats */
	asEmu(si_emu)->instructions++;
	wavefront->emu_inst_count++;
	wavefront->inst_count++;

	/* Set the current instruction */
	inst = &wavefront->inst;

	/* Execute the current instruction */
	switch (inst->info->fmt)
	{

	/* Scalar ALU Instructions */
	case SI_FMT_SOP1:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])(work_item,
					inst);
			}
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;

		/* Special case: V_READFIRSTLANE_B32 */
		if (inst->micro_inst.vop1.op == 2)
		{
			/* Instruction ignores execution mask and is only 
			 * executed on one work item. Execute on the first 
			 * active work item from the least significant bit 
			 * in EXEC. (if exec is 0, execute work item 0) */
			work_item = wavefront->work_items[0];
			if (si_isa_read_sreg(work_item, SI_EXEC) == 0 && 
				si_isa_read_sreg(work_item, SI_EXEC + 1) == 0)
			{
				(*si_isa_inst_func[inst->info->inst])(work_item,
					inst);
			}
			else {
				SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, 
					work_item_id)
				{
					work_item = wavefront->
						work_items[work_item_id];
					if(si_wavefront_work_item_active(
						wavefront, 
						work_item->id_in_wavefront))
					{
						(*si_isa_inst_func[
						 	inst->info->inst])(
							work_item, inst);
						break;
					}
				}
			}
		}
		else
		{
			/* Execute the instruction */
			SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, 
				work_item_id)
			{
				work_item = wavefront->work_items[work_item_id];
				if(si_wavefront_work_item_active(wavefront, 
					work_item->id_in_wavefront))
				{
					(*si_isa_inst_func[inst->info->inst])(
						work_item, inst);
				}
			}
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])(work_item,
					inst);
			}
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])(work_item,
					inst);
			}
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_alu_inst_count++;
		wavefront->vector_alu_inst_count++;
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])(work_item,
					inst);
			}
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->lds_inst_count++;
		wavefront->lds_inst_count++;

		/* Record access type */
		if ((inst->info->opcode >= 13 && inst->info->opcode < 16) ||
			(inst->info->opcode >= 30 && inst->info->opcode < 32) ||
			(inst->info->opcode >= 77 && inst->info->opcode < 80))
		{
			wavefront->lds_write = 1;
		}
		else if (
			(inst->info->opcode >= 54 && inst->info->opcode < 61) ||
			(inst->info->opcode >= 118 && inst->info->opcode < 120))
		{
			wavefront->lds_read = 1;
		}
		else 
		{
			fatal("%s: unimplemented LDS opcode", __FUNCTION__);
		}

		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			if(si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])(work_item,
					inst);
			}
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
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
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
			work_item = wavefront->work_items[work_item_id];
			if (si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])(work_item,
					inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_MUBUF:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->vector_mem_inst_count++;
		wavefront->vector_mem_inst_count++;

		/* Record access type */
		if ((inst->info->opcode >= 0 && inst->info->opcode < 4) ||
			(inst->info->opcode >= 8 && inst->info->opcode < 15))
		{
			wavefront->vector_mem_read = 1;
		}
		else if ((inst->info->opcode >= 4 && inst->info->opcode < 8) ||
			(inst->info->opcode >= 24 && inst->info->opcode < 30))
		{
			wavefront->vector_mem_write = 1;
		}
		else 
		{
			fatal("%s: unsupported mtbuf opcode", __FUNCTION__);
		}
	
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			if (si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])
					(work_item, inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;
	}

	case SI_FMT_EXP:
	{
		/* Dump instruction string when debugging */
		if (debug_status(si_isa_debug_category))
		{
			si_inst_dump(inst, wavefront->inst_size, wavefront->pc,
					ndrange->inst_buffer + wavefront->pc,
					inst_dump, MAX_INST_STR_SIZE);
			si_isa_debug("\n%s", inst_dump);
		}

		/* Stats */
		si_emu->export_inst_count++;
		wavefront->export_inst_count++;

		/* Record access type */
		/* FIXME */
			
		/* Execute the instruction */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			if (si_wavefront_work_item_active(wavefront, 
				work_item->id_in_wavefront))
			{
				(*si_isa_inst_func[inst->info->inst])
					(work_item, inst);
			}
		}

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("\n");
		}

		break;

	}

	default:
	{
		fatal("%s: instruction type not implemented (%d)", 
			__FUNCTION__, inst->info->fmt);
		break;
	}

	}

	/* Check if wavefront finished kernel execution */
	if (!wavefront->finished)
	{
		/* Increment the PC */
		wavefront->pc += wavefront->inst_size;
	}
	else
	{
		/* Check if work-group finished kernel execution */
		if (work_group->wavefronts_completed_emu == 
			work_group->wavefront_count)
		{
			work_group->finished_emu = 1;
		}
	}
}

int si_wavefront_work_item_active(struct si_wavefront_t *wavefront, 
	int id_in_wavefront) 
{
	int mask = 1;
	if(id_in_wavefront < 32)
	{
		mask <<= id_in_wavefront;
		return (wavefront->sreg[SI_EXEC].as_uint & mask) >> 
			id_in_wavefront;
	}
	else
	{
		mask <<= (id_in_wavefront - 32);
		return (wavefront->sreg[SI_EXEC + 1].as_uint & mask) >> 
			(id_in_wavefront - 32);
	}
}

void si_wavefront_init_sreg_with_value(struct si_wavefront_t *wavefront, 
	int sreg, unsigned int value)
{
	wavefront->sreg[sreg].as_uint = value;
}

/* Puts a memory descriptor for a constant buffer (e.g. CB0) into sregs
 * (requires 4 consecutive registers to store the 128-bit structure) */
void si_wavefront_init_sreg_with_cb(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs, int const_buf_num)
{
	struct si_buffer_desc_t buf_desc;
	struct si_ndrange_t *ndrange = wavefront->work_group->ndrange;

	unsigned int buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buf_desc) == 16);
	assert(const_buf_num < SI_EMU_MAX_NUM_CONST_BUFS);
	assert(ndrange->const_buf_table_entries[const_buf_num].valid);

	buf_desc_addr = ndrange->const_buf_table +
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE;

	/* Read a descriptor from the constant buffer table (located 
	 * in global memory) */
	mem_read(si_emu->global_mem, buf_desc_addr, sizeof(buf_desc), 
		&buf_desc);

	/* Store the descriptor in 4 scalar registers */
	memcpy(&wavefront->sreg[first_reg], &buf_desc, sizeof(buf_desc));
}

/* Put a pointer to the constant buffer table into 2 consecutive sregs */
void si_wavefront_init_sreg_with_cb_table(struct si_wavefront_t *wavefront,
        int first_reg, int num_regs)
{
	struct si_ndrange_t *ndrange = wavefront->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->const_buf_table;

	memcpy(&wavefront->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

/* Puts a memory descriptor for a UAV into sregs
 * (requires 4 consecutive registers to store the 128-bit structure) */
void si_wavefront_init_sreg_with_uav(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs, int uav)
{
	struct si_buffer_desc_t buf_desc;
	struct si_ndrange_t *ndrange = wavefront->work_group->ndrange;

	unsigned int buf_desc_addr;

	assert(num_regs == 4);
	assert(sizeof(buf_desc) == 16);
	assert(uav < SI_EMU_MAX_NUM_UAVS);
	assert(ndrange->uav_table_entries[uav].valid);

	buf_desc_addr = ndrange->uav_table + uav*SI_EMU_UAV_TABLE_ENTRY_SIZE;

	/* Read a descriptor from the constant buffer table (located 
	 * in global memory) */
	mem_read(si_emu->global_mem, buf_desc_addr, sizeof(buf_desc), 
		&buf_desc);

	/* Store the descriptor in 4 scalar registers */
	memcpy(&wavefront->sreg[first_reg], &buf_desc, sizeof(buf_desc));
}

/* Put a pointer to the UAV table into 2 consecutive sregs */
void si_wavefront_init_sreg_with_uav_table(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs)
{
	struct si_ndrange_t *ndrange = wavefront->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->uav_table;

	memcpy(&wavefront->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

/* Put a pointer to the Vertex Buffer table into 2 consecutive sregs */
void si_wavefront_init_sreg_with_vertex_buffer_table(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs)
{
	struct si_ndrange_t *ndrange = wavefront->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->vertex_buffer_table;

	memcpy(&wavefront->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

/* Put a pointer to the Fetch Shader into 2 consecutive sregs */
void si_wavefront_init_sreg_with_fetch_shader(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs)
{
	struct si_ndrange_t *ndrange = wavefront->work_group->ndrange;
	struct si_mem_ptr_t mem_ptr;

	assert(num_regs == 2);
	assert(sizeof(mem_ptr) == 8);

	mem_ptr.addr = (unsigned int)ndrange->inst_buffer_size;

	memcpy(&wavefront->sreg[first_reg], &mem_ptr, sizeof(mem_ptr));
}

