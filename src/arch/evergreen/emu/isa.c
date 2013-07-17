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
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/repos.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "machine.h"
#include "work-item.h"
#include "work-group.h"


/*
 * Global Variables
 */

/* Repository of deferred tasks */
struct repos_t *evg_isa_write_task_repos;

/* Instruction execution table */
evg_isa_inst_func_t *evg_isa_inst_func;

/* Debug */
int evg_isa_debug_category;




/*
 * Initialization, finalization
 */


/* Initialization */
void evg_isa_init()
{
	/* Initialize */
	evg_isa_inst_func = xcalloc(EVG_INST_COUNT, sizeof(evg_isa_inst_func_t));
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _opcode, _flags) \
	evg_isa_inst_func[EVG_INST_##_name] = evg_isa_##_name##_impl;
#include <arch/evergreen/asm/asm.dat>
#undef DEFINST

	/* Repository of deferred tasks */
	evg_isa_write_task_repos = repos_create(sizeof(struct evg_isa_write_task_t),
		"gpu_isa_write_task_repos");
}


void evg_isa_done()
{
	/* Instruction execution table */
	free(evg_isa_inst_func);

	/* Repository of deferred tasks */
	repos_free(evg_isa_write_task_repos);
}




/*
 * Constant Memory
 */

void evg_isa_const_mem_write(int bank, int vector, int elem, void *pvalue)
{
	unsigned int addr;

	/* Mark CB0[0..8].{x,y,z,w} positions as initialized */
	if (!bank && vector < 9)
		evg_emu->const_mem_cb0_init[vector * 4 + elem] = 1;

	/* Write */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_write(evg_emu->const_mem, addr, 4, pvalue);
}


void evg_isa_const_mem_read(int bank, int vector, int elem, void *pvalue)
{
	unsigned int addr;

	/* Warn if a position within CB[0..8].{x,y,z,w} is used uninitialized */
	if (!bank && vector < 9 && !evg_emu->const_mem_cb0_init[vector * 4 + elem])
		warning("CB0[%d].%c is used uninitialized", vector, "xyzw"[elem]);
	
	/* Read */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_read(evg_emu->const_mem, addr, 4, pvalue);
}





/*
 * ALU Clauses
 */

/* Called before and ALU clause starts for a wavefront */
void evg_isa_alu_clause_start(struct evg_wavefront_t *wavefront)
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
void evg_isa_alu_clause_end(struct evg_wavefront_t *wavefront)
{
	/* If CF inst was ALU_POP_AFTER, pop the stack */
	if (wavefront->cf_inst.info->inst == EVG_INST_ALU_POP_AFTER)
		evg_wavefront_stack_pop(wavefront, 1);
}




/*
 * TEX Clauses
 */

/* Called before and TEX clause starts in wavefront */
void evg_isa_tc_clause_start(struct evg_wavefront_t *wavefront)
{
	/* Stats */
	wavefront->tc_clause_count++;
}


/* Called after a TEX clause completed in a wavefront */
void evg_isa_tc_clause_end(struct evg_wavefront_t *wavefront)
{
}




/*
 * Instruction operands
 */

/* Dump a destination value depending on the format of the destination operand
 * in the current instruction, as specified by its flags. */
void gpu_isa_dest_value_dump(struct evg_inst_t *inst, void *value_ptr, FILE *f)
{
	if (inst->info->flags & EVG_INST_FLAG_DST_INT)
		fprintf(f, "%d", * (int *) value_ptr);
	
	else if (inst->info->flags & EVG_INST_FLAG_DST_UINT)
		fprintf(f, "0x%x", * (unsigned int *) value_ptr);
	
	else if (inst->info->flags & EVG_INST_FLAG_DST_FLOAT)
		fprintf(f, "%gf", * (float *) value_ptr);
	
	else
		fprintf(f, "(0x%x,%gf)", * (unsigned int *) value_ptr, * (float *) value_ptr);
}


/* Read source GPR, checking for valid ranges. */
unsigned int evg_isa_read_gpr(struct evg_work_item_t *work_item,
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
float evg_isa_read_gpr_float(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, int im)
{
	union evg_reg_t reg;

	reg.as_uint = evg_isa_read_gpr(work_item, gpr, rel, chan, im);
	return reg.as_float;
}


void evg_isa_write_gpr(struct evg_work_item_t *work_item,
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


void evg_isa_write_gpr_float(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, float value)
{
	union evg_reg_t reg;

	reg.as_float = value;
	evg_isa_write_gpr(work_item, gpr, rel, chan, reg.as_uint);
}


/* Read source operand in ALU instruction.
 * This is a common function for both integer and float formats. */
static unsigned int evg_isa_read_op_src_common(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int src_idx, int *neg_ptr, int *abs_ptr)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	int sel;
	int rel;
	int chan;

	int value = 0;  /* Signed, for negative constants and abs operations */

	/* Get the source operand parameters */
	evg_inst_get_op_src(inst, src_idx, &sel, &rel, &chan, neg_ptr, abs_ptr);

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
		unsigned int kcache_mode;
		unsigned int kcache_addr;

		assert(wavefront->cf_inst.info->fmt[0] == EVG_FMT_CF_ALU_WORD0
			&& wavefront->cf_inst.info->fmt[1] == EVG_FMT_CF_ALU_WORD1);
		kcache_bank = wavefront->cf_inst.words[0].cf_alu_word0.kcache_bank0;
		kcache_mode = wavefront->cf_inst.words[0].cf_alu_word0.kcache_mode0;
		kcache_addr = wavefront->cf_inst.words[1].cf_alu_word1.kcache_addr0;

		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		EVG_ISA_ARG_NOT_SUPPORTED_RANGE(chan, 0, 3);
		evg_isa_const_mem_read(kcache_bank, kcache_addr * 16 + sel - 128, chan, &value);

		return value;
	}

	/* 160..191: Kcache 1 constant */
	if (IN_RANGE(sel, 160, 191))
	{

		unsigned int kcache_bank, kcache_mode, kcache_addr;

		assert(wavefront->cf_inst.info->fmt[0] == EVG_FMT_CF_ALU_WORD0
			&& wavefront->cf_inst.info->fmt[1] == EVG_FMT_CF_ALU_WORD1);
		kcache_bank = wavefront->cf_inst.words[0].cf_alu_word0.kcache_bank1;
		kcache_mode = wavefront->cf_inst.words[1].cf_alu_word1.kcache_mode1;
		kcache_addr = wavefront->cf_inst.words[1].cf_alu_word1.kcache_addr1;

		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		EVG_ISA_ARG_NOT_SUPPORTED_RANGE(chan, 0, 3);
		evg_isa_const_mem_read(kcache_bank, kcache_addr * 16 + sel - 160, chan, &value);
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
		union evg_reg_t reg;

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
		union evg_reg_t reg;

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


unsigned int evg_isa_read_op_src_int(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int src_idx)
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


float evg_isa_read_op_src_float(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int src_idx)
{
	union evg_reg_t reg;
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
struct evg_inst_t *evg_isa_get_alu_inst(struct evg_alu_group_t *alu_group,
	enum evg_alu_enum alu)
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

void evg_isa_enqueue_write_lds(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, unsigned int addr, unsigned int value,
	int value_size)
{
	struct evg_isa_write_task_t *wt;

	/* Inactive pixel not enqueued */
	if (!evg_work_item_get_pred(work_item))
		return;
	
	/* Create task */
	wt = repos_create_object(evg_isa_write_task_repos);
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
void evg_isa_enqueue_write_dest(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, unsigned int value)
{
	struct evg_isa_write_task_t *wt;

	/* If pixel is inactive, do not enqueue the task */
	assert(inst->info->fmt[0] == EVG_FMT_ALU_WORD0);
	if (!evg_work_item_get_pred(work_item))
		return;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * EVG_ALU_WORD1_OP2 and EVG_ALU_WORD1_OP3 formats. */
	wt = repos_create_object(evg_isa_write_task_repos);
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
	if (inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 && !EVG_ALU_WORD1_OP2.write_mask)
		wt->write_mask = 0;

	/* Enqueue task */
	linked_list_add(work_item->write_task_list, wt);
}


void evg_isa_enqueue_write_dest_float(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, float value)
{
	union evg_reg_t reg;

	reg.as_float = value;
	evg_isa_enqueue_write_dest(work_item, inst, reg.as_uint);
}


void evg_isa_enqueue_push_before(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_isa_write_task_t *wt;

	/* Do only if instruction initiating ALU clause is ALU_PUSH_BEFORE */
	if (wavefront->cf_inst.info->inst != EVG_INST_ALU_PUSH_BEFORE)
		return;

	/* Create and enqueue task */
	wt = repos_create_object(evg_isa_write_task_repos);
	wt->work_item = work_item;
	wt->kind = EVG_ISA_WRITE_TASK_PUSH_BEFORE;
	wt->inst = inst;
	linked_list_add(work_item->write_task_list, wt);
}


void evg_isa_enqueue_pred_set(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int cond)
{
	struct evg_isa_write_task_t *wt;

	/* If pixel is inactive, predicate is not changed */
	assert(inst->info->fmt[0] == EVG_FMT_ALU_WORD0);
	assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2);
	if (!evg_work_item_get_pred(work_item))
		return;
	
	/* Create and enqueue task */
	wt = repos_create_object(evg_isa_write_task_repos);
	wt->work_item = work_item;
	wt->kind = EVG_ISA_WRITE_TASK_SET_PRED;
	wt->inst = inst;
	wt->cond = cond;
	linked_list_add(work_item->write_task_list, wt);
}


void evg_isa_write_task_commit(struct evg_work_item_t *work_item)
{
	struct linked_list_t *task_list = work_item->write_task_list;
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_work_group_t *work_group = work_item->work_group;

	struct evg_isa_write_task_t *wt;
	struct evg_inst_t *inst;

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
					str_map_value(&evg_pv_map, wt->inst->alu));
				if (wt->write_mask)
				{
					evg_isa_debug(",");
					evg_inst_dump_gpr(wt->gpr, wt->rel, wt->chan, 0,
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
			union evg_reg_t lds_value;

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
		repos_free_object(evg_isa_write_task_repos, wt);
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
				evg_wavefront_stack_push(wavefront);
			wavefront->push_before_done = 1;
			break;
		}

		case EVG_ISA_WRITE_TASK_SET_PRED:
		{
			int update_pred = EVG_ALU_WORD1_OP2.update_pred;
			int update_exec_mask = EVG_ALU_WORD1_OP2.update_exec_mask;

			assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2);
			if (update_pred)
				evg_work_item_set_pred(work_item, wt->cond);
			if (update_exec_mask)
				evg_work_item_set_active(work_item, wt->cond);

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
		repos_free_object(evg_isa_write_task_repos, wt);
		linked_list_remove(task_list);
	}

	/* List should be empty */
	assert(!linked_list_count(task_list));
}

