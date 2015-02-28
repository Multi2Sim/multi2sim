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

#include <arch/evergreen/asm/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/repos.h>
#include <lib/util/string.h>
#include <memory/memory.h>

#include "emu.h"
#include "isa.h"
#include "machine.h"
#include "ndrange.h"
#include "work-item.h"
#include "work-group.h"


/*
 * Global Variables
 */

/* Debug */
int evg_isa_debug_category;



/*
 * ALU Clauses
 */

/* Called before and ALU clause starts for a wavefront */
void evg_isa_alu_clause_start(EvgWavefront *wavefront)
{
	/* Copy 'active' mask at the top of the stack to 'pred' mask */
	bit_map_copy(wavefront->pred, 0, wavefront->active_stack,
		wavefront->stack_top * wavefront->work_item_count, wavefront->work_item_count);
	if (debug_status(evg_isa_debug_category))
	{
		evg_isa_debug("  %s:pred=", wavefront->name);
		bit_map_dump(wavefront->pred, 0, wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* Flag 'push_before_done' will be set by the first PRED_SET* inst */
	wavefront->push_before_done = 0;

	/* Stats */
	wavefront->alu_clause_count++;
}


/* Called after an ALU clause completed in a wavefront */
void evg_isa_alu_clause_end(EvgWavefront *wavefront)
{
	/* If CF inst was ALU_POP_AFTER, pop the stack */
	if (wavefront->cf_inst->info->opcode == EVG_INST_ALU_POP_AFTER)
		EvgWavefrontPop(wavefront, 1);
}




/*
 * TEX Clauses
 */

/* Called before and TEX clause starts in wavefront */
void evg_isa_tc_clause_start(EvgWavefront *wavefront)
{
	/* Stats */
	wavefront->tc_clause_count++;
}


/* Called after a TEX clause completed in a wavefront */
void evg_isa_tc_clause_end(EvgWavefront *wavefront)
{
}




/*
 * Instruction operands
 */

/* Dump a destination value depending on the format of the destination operand
 * in the current instruction, as specified by its flags. */
void gpu_isa_dest_value_dump(EvgInst *inst, void *value_ptr, FILE *f)
{
	if (inst->info->flags & EvgInstFlagDstInt)
		fprintf(f, "%d", * (int *) value_ptr);
	
	else if (inst->info->flags & EvgInstFlagDstUint)
		fprintf(f, "0x%x", * (unsigned int *) value_ptr);
	
	else if (inst->info->flags & EvgInstFlagDstFloat)
		fprintf(f, "%gf", * (float *) value_ptr);
	
	else
		fprintf(f, "(0x%x,%gf)", * (unsigned int *) value_ptr, * (float *) value_ptr);
}


/* Read source GPR, checking for valid ranges. */
unsigned int evg_isa_read_gpr(EvgWorkItem *work_item,
	int gpr, int rel, int chan, int im)
{
	/* Check arguments */
	if (!IN_RANGE(chan, 0, 4))
		fatal("%s: invalid value for 'chan'", __FUNCTION__);
	if (!IN_RANGE(gpr, 0, 127))
		fatal("%s: invalid value for 'gpr'", __FUNCTION__);
	if (rel)
		fatal("%s: not supported for 'rel' != 0", __FUNCTION__);
	if (im)
		fatal("%s: not supported for 'im' != 0", __FUNCTION__);

	/* Return value */
	return work_item->gpr[gpr].elem[chan];
}


/* Read source GPR in float format */
float evg_isa_read_gpr_float(EvgWorkItem *work_item,
	int gpr, int rel, int chan, int im)
{
	EvgInstReg reg;

	reg.as_uint = evg_isa_read_gpr(work_item, gpr, rel, chan, im);
	return reg.as_float;
}


void evg_isa_write_gpr(EvgWorkItem *work_item,
	int gpr, int rel, int chan, unsigned int value)
{
	/* Check arguments */
	if (!IN_RANGE(chan, 0, 4))
		fatal("%s: invalid value for 'chan'", __FUNCTION__);
	if (!IN_RANGE(gpr, 0, 127))
		fatal("%s: invalid value for 'gpr'", __FUNCTION__);
	if (rel)
		fatal("%s: not supported for 'rel' != 0", __FUNCTION__);

	/* Write to register */
	work_item->gpr[gpr].elem[chan] = value;
}


void evg_isa_write_gpr_float(EvgWorkItem *work_item,
	int gpr, int rel, int chan, float value)
{
	EvgInstReg reg;

	reg.as_float = value;
	evg_isa_write_gpr(work_item, gpr, rel, chan, reg.as_uint);
}


/* Read source operand in ALU instruction.
 * This is a common function for both integer and float formats. */
static unsigned int evg_isa_read_op_src_common(EvgWorkItem *work_item,
	EvgInst *inst, int src_idx, int *neg_ptr, int *abs_ptr)
{
	EvgWavefront *wavefront = work_item->wavefront;
	EvgNDRange *ndrange = work_item->ndrange;
	EvgEmu *emu = ndrange->emu;

	int sel;
	int rel;
	int chan;

	int value = 0;  /* Signed, for negative constants and abs operations */

	/* Get the source operand parameters */
	EvgInstGetOpSrc(inst, src_idx, &sel, &rel, &chan, neg_ptr, abs_ptr);

	/* 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127))
	{
		int index_mode;

		index_mode = inst->words[0].alu_word0.index_mode;
		value = evg_isa_read_gpr(work_item, sel, rel, chan, index_mode);
		return value;
	}

	/* 128..159: Kcache 0 constant */
	if (IN_RANGE(sel, 128, 159))
	{
		unsigned int kcache_bank;
		//unsigned int kcache_mode;
		unsigned int kcache_addr;

		assert(wavefront->cf_inst->info->fmt[0] == EvgInstFormatCfAluWord0
			&& wavefront->cf_inst->info->fmt[1] == EvgInstFormatCfAluWord1);
		kcache_bank = wavefront->cf_inst->words[0].cf_alu_word0.kcache_bank0;
		//kcache_mode = wavefront->cf_inst->words[0].cf_alu_word0.kcache_mode0;
		kcache_addr = wavefront->cf_inst->words[1].cf_alu_word1.kcache_addr0;

		//EVG_ISA_ARG_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		EVG_ISA_ARG_NOT_SUPPORTED_RANGE(chan, 0, 3);
		EvgEmuConstMemRead(emu, kcache_bank, kcache_addr * 16 + sel - 128, chan, &value);

		return value;
	}

	/* 160..191: Kcache 1 constant */
	if (IN_RANGE(sel, 160, 191))
	{
		unsigned int kcache_bank;
		//unsigned int kcache_mode;
		unsigned int kcache_addr;

		assert(wavefront->cf_inst->info->fmt[0] == EvgInstFormatCfAluWord0
			&& wavefront->cf_inst->info->fmt[1] == EvgInstFormatCfAluWord1);
		kcache_bank = wavefront->cf_inst->words[0].cf_alu_word0.kcache_bank1;
		//kcache_mode = wavefront->cf_inst->words[1].cf_alu_word1.kcache_mode1;
		kcache_addr = wavefront->cf_inst->words[1].cf_alu_word1.kcache_addr1;

		//EVG_ISA_ARG_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		EVG_ISA_ARG_NOT_SUPPORTED_RANGE(chan, 0, 3);
		EvgEmuConstMemRead(emu, kcache_bank, kcache_addr * 16 + sel - 160, chan, &value);
		return value;
	}

	/* QA and QA.pop */
	if (sel == 219 || sel == 221)
	{
		unsigned int *pvalue;
		pvalue = (unsigned int *) list_dequeue(work_item->lds_oqa);
		if (!pvalue)
			fatal("%s: LDS queue A is empty", __FUNCTION__);
		value = *pvalue;
		if (sel == 219)
			list_enqueue(work_item->lds_oqa, pvalue);
		else
			free(pvalue);
		return value;
	}

	/* QB and QB.pop */
	if (sel == 220 || sel == 222)
	{
		unsigned int *pvalue;

		pvalue = (unsigned int *) list_dequeue(work_item->lds_oqb);
		if (!pvalue)
			fatal("%s: LDS queue B is empty", __FUNCTION__);
		value = *pvalue;
		if (sel == 220)
			list_enqueue(work_item->lds_oqb, pvalue);
		else
			free(pvalue);
		return value;
	}

	/* ALU_SRC_0 */
	if (sel == 248)
	{
		value = 0;
		return value;
	}

	/* ALU_SRC_1 */
	if (sel == 249)
	{
		EvgInstReg reg;

		reg.as_float = 1.0f;
		value = reg.as_uint;

		return value;
	}

	/* ALU_SRC_1_INT */
	if (sel == 250)
	{
		value = 1;
		return value;
	}

	/* ALU_SRC_M_1_INT */
	if (sel == 251)
	{
		value = -1;
		return value;
	}

	/* ALU_SRC_0_5 */
	if (sel == 252)
	{
		EvgInstReg reg;

		reg.as_float = 0.5f;
		value = reg.as_uint;

		return value;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253)
	{
		assert(inst->alu_group);
		EVG_ISA_ARG_NOT_SUPPORTED_RANGE(chan, 0, 3);
		value = inst->alu_group->literal[chan].as_uint;
		return value;
	}

	/* ALU_SRC_PV */
	if (sel == 254)
	{
		EVG_ISA_ARG_NOT_SUPPORTED_RANGE(chan, 0, 3);
		value = work_item->pv.elem[chan];
		return value;
	}

	/* ALU_SRC_PS */
	if (sel == 255)
	{
		value = work_item->pv.elem[4];
		return value;
	}

	/* Not implemented 'sel' field */
	fatal("%s: src_idx=%d, not implemented for sel=%d", __FUNCTION__, src_idx, sel);
	return 0;
}


unsigned int evg_isa_read_op_src_int(EvgWorkItem *work_item,
	EvgInst *inst, int src_idx)
{
	int neg, abs;
	int value;

	/* Get value */
	value = evg_isa_read_op_src_common(work_item, inst,
		src_idx, &neg, &abs);

	/* Absolute value and negation */
	if (abs && value < 0)
		value = -value;
	if (neg)
		value = -value;
	
	/* Return as unsigned */
	return value;
}


float evg_isa_read_op_src_float(EvgWorkItem *work_item,
	EvgInst *inst, int src_idx)
{
	EvgInstReg reg;
	int neg;
	int abs;

	/* Get value */
	reg.as_uint = evg_isa_read_op_src_common(work_item, inst,
		src_idx, &neg, &abs);

	/* Absolute value and negation */
	if (abs && reg.as_float < 0.0)
		reg.as_float = -reg.as_float;
	if (neg)
		reg.as_float = -reg.as_float;
	
	/* Return */
	return reg.as_float;
}


/* Return the instruction in slot 'alu' of the VLIW bundle in 'alu_group'.
 * If the VLIW slot requested is not present, this function returns NULL. */
EvgInst *evg_isa_get_alu_inst(EvgALUGroup *alu_group, EvgInstAlu alu)
{
	int i;

	/* Not a VLIW bundle */
	if (!alu_group)
		panic("%s: not a VLIW bundle", __FUNCTION__);

	/* Loop for slot 'alu' */
	for (i = 0; i < alu_group->inst_count; i++)
		if (alu_group->inst[i].alu == alu)
			return &alu_group->inst[i];

	/* VLIW slot not present */
	return NULL;
}




/*
 * Deferred tasks for ALU group
 */

void evg_isa_enqueue_write_lds(EvgWorkItem *work_item,
	EvgInst *inst, unsigned int addr, unsigned int value,
	int value_size)
{
	EvgNDRange *ndrange = work_item->ndrange;
	EvgEmu *emu = ndrange->emu;

	struct evg_isa_write_task_t *wt;

	/* Inactive pixel not enqueued */
	if (!EvgWorkItemGetPred(work_item))
		return;
	
	/* Create task */
	wt = repos_create_object(emu->write_task_repos);
	wt->work_item = work_item;
	wt->kind = EVG_ISA_WRITE_TASK_WRITE_LDS;
	wt->inst = inst;
	wt->lds_addr = addr;
	wt->lds_value = value;
	wt->lds_value_size = value_size;

	/* Enqueue task */
	linked_list_add(work_item->write_task_list, wt);
}


/* Write to destination operand in ALU instruction */
void evg_isa_enqueue_write_dest(EvgWorkItem *work_item,
	EvgInst *inst, unsigned int value)
{
	EvgNDRange *ndrange = work_item->ndrange;
	EvgEmu *emu = ndrange->emu;

	struct evg_isa_write_task_t *wt;

	/* If pixel is inactive, do not enqueue the task */
	assert(inst->info->fmt[0] == EvgInstFormatAluWord0);
	if (!EvgWorkItemGetPred(work_item))
		return;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * EVG_ALU_WORD1_OP2 and EVG_ALU_WORD1_OP3 formats. */
	wt = repos_create_object(emu->write_task_repos);
	wt->work_item = work_item;
	wt->kind = EVG_ISA_WRITE_TASK_WRITE_DEST;
	wt->inst = inst;
	wt->gpr = EVG_ALU_WORD1_OP2.dst_gpr;
	wt->rel = EVG_ALU_WORD1_OP2.dst_rel;
	wt->chan = EVG_ALU_WORD1_OP2.dst_chan;
	wt->index_mode = EVG_ALU_WORD0.index_mode;
	wt->value = value;

	/* For EVG_ALU_WORD1_OP2, check 'write_mask' field */
	wt->write_mask = 1;
	if (inst->info->fmt[1] == EvgInstFormatAluWord1Op2 && !EVG_ALU_WORD1_OP2.write_mask)
		wt->write_mask = 0;

	/* Enqueue task */
	linked_list_add(work_item->write_task_list, wt);
}


void evg_isa_enqueue_write_dest_float(EvgWorkItem *work_item,
	EvgInst *inst, float value)
{
	EvgInstReg reg;

	reg.as_float = value;
	evg_isa_enqueue_write_dest(work_item, inst, reg.as_uint);
}


void evg_isa_enqueue_push_before(EvgWorkItem *work_item,
	EvgInst *inst)
{
	EvgWavefront *wavefront = work_item->wavefront;
	EvgNDRange *ndrange = work_item->ndrange;
	EvgEmu *emu = ndrange->emu;

	struct evg_isa_write_task_t *wt;

	/* Do only if instruction initiating ALU clause is ALU_PUSH_BEFORE */
	if (wavefront->cf_inst->info->opcode != EVG_INST_ALU_PUSH_BEFORE)
		return;

	/* Create and enqueue task */
	wt = repos_create_object(emu->write_task_repos);
	wt->work_item = work_item;
	wt->kind = EVG_ISA_WRITE_TASK_PUSH_BEFORE;
	wt->inst = inst;
	linked_list_add(work_item->write_task_list, wt);
}


void evg_isa_enqueue_pred_set(EvgWorkItem *work_item,
	EvgInst *inst, int cond)
{
	EvgNDRange *ndrange = work_item->ndrange;
	EvgEmu *emu = ndrange->emu;

	struct evg_isa_write_task_t *wt;

	/* If pixel is inactive, predicate is not changed */
	assert(inst->info->fmt[0] == EvgInstFormatAluWord0);
	assert(inst->info->fmt[1] == EvgInstFormatAluWord1Op2);
	if (!EvgWorkItemGetPred(work_item))
		return;
	
	/* Create and enqueue task */
	wt = repos_create_object(emu->write_task_repos);
	wt->work_item = work_item;
	wt->kind = EVG_ISA_WRITE_TASK_SET_PRED;
	wt->inst = inst;
	wt->cond = cond;
	linked_list_add(work_item->write_task_list, wt);
}


void evg_isa_write_task_commit(EvgWorkItem *work_item)
{
	struct linked_list_t *task_list = work_item->write_task_list;
	EvgWavefront *wavefront = work_item->wavefront;
	EvgWorkGroup *work_group = work_item->work_group;
	EvgNDRange *ndrange = work_item->ndrange;
	EvgEmu *emu = ndrange->emu;

	struct evg_isa_write_task_t *wt;
	EvgInst *inst;

	/* Process first tasks of type:
	 *  - EVG_ISA_WRITE_TASK_WRITE_DEST
	 *  - EVG_ISA_WRITE_TASK_WRITE_LDS
	 */
	for (linked_list_head(task_list); !linked_list_is_end(task_list); )
	{

		/* Get task */
		wt = linked_list_get(task_list);
		assert(wt->work_item == work_item);
		inst = wt->inst;

		switch (wt->kind)
		{
		
		case EVG_ISA_WRITE_TASK_WRITE_DEST:
		{
			if (wt->write_mask)
				evg_isa_write_gpr(work_item, wt->gpr, wt->rel, wt->chan, wt->value);
			work_item->pv.elem[wt->inst->alu] = wt->value;

			/* Debug */
			if (evg_isa_debugging())
			{
				evg_isa_debug("  i%d:%s", work_item->id,
					str_map_value(&evg_inst_alu_pv_map, wt->inst->alu));
				if (wt->write_mask)
				{
					evg_isa_debug(",");
					EvgInstDumpGpr(wt->gpr, wt->rel, wt->chan, 0,
						debug_file(evg_isa_debug_category));
				}
				evg_isa_debug("<=");
				gpu_isa_dest_value_dump(inst, &wt->value,
					debug_file(evg_isa_debug_category));
			}

			break;
		}

		case EVG_ISA_WRITE_TASK_WRITE_LDS:
		{
			struct mem_t *local_mem;
			EvgInstReg lds_value;

			local_mem = work_group->local_mem;
			assert(local_mem);
			assert(wt->lds_value_size);
			mem_write(local_mem, wt->lds_addr, wt->lds_value_size, &wt->lds_value);

			/* Debug */
			lds_value.as_uint = wt->lds_value;
			evg_isa_debug("  i%d:LDS[0x%x]<=(%u,%gf) (%d bytes)", work_item->id, wt->lds_addr,
				lds_value.as_uint, lds_value.as_float, (int) wt->lds_value_size);
			break;
		}

		default:
			linked_list_next(task_list);
			continue;
		}

		/* Done with this task */
		repos_free_object(emu->write_task_repos, wt);
		linked_list_remove(task_list);
	}

	/* Process PUSH_BEFORE, PRED_SET */
	for (linked_list_head(task_list); !linked_list_is_end(task_list); )
	{
		/* Get task */
		wt = linked_list_get(task_list);
		inst = wt->inst;

		/* Process */
		switch (wt->kind)
		{

		case EVG_ISA_WRITE_TASK_PUSH_BEFORE:
		{
			if (!wavefront->push_before_done)
				EvgWavefrontPush(wavefront);
			wavefront->push_before_done = 1;
			break;
		}

		case EVG_ISA_WRITE_TASK_SET_PRED:
		{
			int update_pred = EVG_ALU_WORD1_OP2.update_pred;
			int update_exec_mask = EVG_ALU_WORD1_OP2.update_exec_mask;

			assert(inst->info->fmt[1] == EvgInstFormatAluWord1Op2);
			if (update_pred)
				EvgWorkItemSetPred(work_item, wt->cond);
			if (update_exec_mask)
				EvgWorkItemSetActive(work_item, wt->cond);

			/* Debug */
			if (debug_status(evg_isa_debug_category))
			{
				if (update_pred && update_exec_mask)
					evg_isa_debug("  i%d:act/pred<=%d", work_item->id, wt->cond);
				else if (update_pred)
					evg_isa_debug("  i%d:pred=%d", work_item->id, wt->cond);
				else if (update_exec_mask)
					evg_isa_debug("  i%d:pred=%d", work_item->id, wt->cond);
			}
			break;
		}

		default:
			abort();
		}
		
		/* Done with task */
		repos_free_object(emu->write_task_repos, wt);
		linked_list_remove(task_list);
	}

	/* List should be empty */
	assert(!linked_list_count(task_list));
}

