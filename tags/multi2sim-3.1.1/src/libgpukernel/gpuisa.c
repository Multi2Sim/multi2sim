/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel.h>
#include <cpukernel.h>
#include <repos.h>


/* Some globals */

struct gpu_ndrange_t *gpu_isa_ndrange;  /* Current ND-Range */
struct gpu_work_group_t *gpu_isa_work_group;  /* Current work-group */
struct gpu_wavefront_t *gpu_isa_wavefront;  /* Current wavefront */
struct gpu_work_item_t *gpu_isa_work_item;  /* Current work-item */
struct amd_inst_t *gpu_isa_cf_inst;  /* Current CF instruction */
struct amd_inst_t *gpu_isa_inst;  /* Current instruction */
struct amd_alu_group_t *gpu_isa_alu_group;  /* Current ALU group */

/* Repository of deferred tasks */
struct repos_t *gpu_isa_write_task_repos;

/* Instruction execution table */
amd_inst_impl_t *amd_inst_impl;

/* Debug */
int gpu_isa_debug_category;




/*
 * Initialization, finalization
 */


/* Initialization */
void gpu_isa_init()
{
	/* Initialize instruction execution table */
	amd_inst_impl = calloc(AMD_INST_COUNT, sizeof(amd_inst_impl_t));
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _opcode, _flags) \
	extern void amd_inst_##_name##_impl(); \
	amd_inst_impl[AMD_INST_##_name] = amd_inst_##_name##_impl;
#include <gpudisasm.dat>
#undef DEFINST

	/* Repository of deferred tasks */
	gpu_isa_write_task_repos = repos_create(sizeof(struct gpu_isa_write_task_t),
		"gpu_isa_write_task_repos");
}


void gpu_isa_done()
{
	/* Instruction execution table */
	free(amd_inst_impl);

	/* Repository of deferred tasks */
	repos_free(gpu_isa_write_task_repos);
}




/*
 * Constant Memory
 */

void gpu_isa_const_mem_write(int bank, int vector, int elem, void *pvalue)
{
	uint32_t addr;

	/* Mark CB0[0..8].{x,y,z,w} positions as initialized */
	if (!bank && vector < 9)
		gk->const_mem_cb0_init[vector * 4 + elem] = 1;

	/* Write */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_write(gk->const_mem, addr, 4, pvalue);
}


void gpu_isa_const_mem_read(int bank, int vector, int elem, void *pvalue)
{
	uint32_t addr;

	/* Warn if a position within CB[0..8].{x,y,z,w} is used uninitialized */
	if (!bank && vector < 9 && !gk->const_mem_cb0_init[vector * 4 + elem])
		warning("CB0[%d].%c is used uninitialized", vector, "xyzw"[elem]);
	
	/* Read */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_read(gk->const_mem, addr, 4, pvalue);
}





/*
 * ALU Clauses
 */

/* Called before and ALU clause starts in 'gpu_isa_wavefront' */
void gpu_isa_alu_clause_start()
{
	/* Copy 'active' mask at the top of the stack to 'pred' mask */
	bit_map_copy(gpu_isa_wavefront->pred, 0, gpu_isa_wavefront->active_stack,
		gpu_isa_wavefront->stack_top * gpu_isa_wavefront->work_item_count, gpu_isa_wavefront->work_item_count);
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:pred=", gpu_isa_wavefront->name);
		bit_map_dump(gpu_isa_wavefront->pred, 0, gpu_isa_wavefront->work_item_count,
			debug_file(gpu_isa_debug_category));
	}

	/* Flag 'push_before_done' will be set by the first PRED_SET* inst */
	gpu_isa_wavefront->push_before_done = 0;

	/* Stats */
	gpu_isa_wavefront->alu_clause_count++;
}


/* Called after an ALU clause completed in a wavefront */
void gpu_isa_alu_clause_end()
{
	/* If CF inst was ALU_POP_AFTER, pop the stack */
	if (gpu_isa_cf_inst->info->inst == AMD_INST_ALU_POP_AFTER)
		gpu_wavefront_stack_pop(gpu_isa_wavefront, 1);
}




/*
 * TEX Clauses
 */

/* Called before and TEX clause starts in wavefront */
void gpu_isa_tc_clause_start()
{
	/* Stats */
	gpu_isa_wavefront->tc_clause_count++;
}


/* Called after a TEX clause completed in a wavefront */
void gpu_isa_tc_clause_end()
{
}




/*
 * Instruction operands
 */

/* Dump a destination value depending on the format of the destination operand
 * in the current instruction, as specified by its flags. */
void gpu_isa_dest_value_dump(void *pvalue, FILE *f)
{
	if (gpu_isa_inst->info->flags & AMD_INST_FLAG_DST_INT)
		fprintf(f, "%d", * (int *) pvalue);
	
	else if (gpu_isa_inst->info->flags & AMD_INST_FLAG_DST_UINT)
		fprintf(f, "0x%x", * (unsigned int *) pvalue);
	
	else if (gpu_isa_inst->info->flags & AMD_INST_FLAG_DST_FLOAT)
		fprintf(f, "%gf", * (float *) pvalue);
	
	else
		fprintf(f, "(0x%x,%gf)", * (unsigned int *) pvalue, * (float *) pvalue);
}


/* Read source GPR */
uint32_t gpu_isa_read_gpr(int gpr, int rel, int chan, int im)
{
	GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 4);
	GPU_PARAM_NOT_SUPPORTED_OOR(gpr, 0, 127);
	GPU_PARAM_NOT_SUPPORTED_NEQ(rel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(im, 0);
	return GPU_GPR_ELEM(gpr, chan);
}


/* Read source GPR in float format */
float gpu_isa_read_gpr_float(int gpr, int rel, int chan, int im)
{
	uint32_t value;
	float value_float;

	value = gpu_isa_read_gpr(gpr, rel, chan, im);
	value_float = * (float *) &value;
	return value_float;
}


void gpu_isa_write_gpr(int gpr, int rel, int chan, uint32_t value)
{
	GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 4);
	GPU_PARAM_NOT_SUPPORTED_OOR(gpr, 0, 127);
	GPU_PARAM_NOT_SUPPORTED_NEQ(rel, 0);
	GPU_GPR_ELEM(gpr, chan) = value;
}


void gpu_isa_write_gpr_float(int gpr, int rel, int chan, float value_float)
{
	uint32_t value;

	value = * (uint32_t *) &value_float;
	gpu_isa_write_gpr(gpr, rel, chan, value);
}


/* Read source operand in ALU instruction.
 * This is a common function for both integer and float formats. */
static uint32_t gpu_isa_read_op_src_common(int src_idx, int *neg_ptr, int *abs_ptr)
{
	int sel, rel, chan;
	int32_t value = 0;  /* Signed, for negative constants and abs operations */

	/* Get the source operand parameters */
	amd_inst_get_op_src(gpu_isa_inst, src_idx, &sel, &rel, &chan, neg_ptr, abs_ptr);

	/* 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127)) {
		int index_mode;
		index_mode = gpu_isa_inst->words[0].alu_word0.index_mode;
		value = gpu_isa_read_gpr(sel, rel, chan, index_mode);
		return value;
	}

	/* 128..159: Kcache 0 constant */
	if (IN_RANGE(sel, 128, 159)) {

		uint32_t kcache_bank, kcache_mode, kcache_addr;

		assert(gpu_isa_cf_inst->info->fmt[0] == FMT_CF_ALU_WORD0 && gpu_isa_cf_inst->info->fmt[1] == FMT_CF_ALU_WORD1);
		kcache_bank = gpu_isa_cf_inst->words[0].cf_alu_word0.kcache_bank0;
		kcache_mode = gpu_isa_cf_inst->words[0].cf_alu_word0.kcache_mode0;
		kcache_addr = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_addr0;

		GPU_PARAM_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		gpu_isa_const_mem_read(kcache_bank, kcache_addr * 16 + sel - 128, chan, &value);
		return value;
	}

	/* 160..191: Kcache 1 constant */
	if (IN_RANGE(sel, 160, 191)) {

		uint32_t kcache_bank, kcache_mode, kcache_addr;

		assert(gpu_isa_cf_inst->info->fmt[0] == FMT_CF_ALU_WORD0 && gpu_isa_cf_inst->info->fmt[1] == FMT_CF_ALU_WORD1);
		kcache_bank = gpu_isa_cf_inst->words[0].cf_alu_word0.kcache_bank1;
		kcache_mode = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_mode1;
		kcache_addr = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_addr1;

		GPU_PARAM_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		gpu_isa_const_mem_read(kcache_bank, kcache_addr * 16 + sel - 160, chan, &value);
		return value;
	}

	/* QA and QA.pop */
	if (sel == 219 || sel == 221) {
		uint32_t *pvalue;
		pvalue = (uint32_t *) list_dequeue(gpu_isa_work_item->lds_oqa);
		if (!pvalue)
			fatal("%s: LDS queue A is empty", __FUNCTION__);
		value = *pvalue;
		if (sel == 219)
			list_enqueue(gpu_isa_work_item->lds_oqa, pvalue);
		else
			free(pvalue);
		return value;
	}

	/* QB and QB.pop */
	if (sel == 220 || sel == 222) {
		uint32_t *pvalue;
		pvalue = (uint32_t *) list_dequeue(gpu_isa_work_item->lds_oqb);
		if (!pvalue)
			fatal("%s: LDS queue B is empty", __FUNCTION__);
		value = *pvalue;
		if (sel == 220)
			list_enqueue(gpu_isa_work_item->lds_oqb, pvalue);
		else
			free(pvalue);
		return value;
	}

	/* ALU_SRC_0 */
	if (sel == 248) {
		value = 0;
		return value;
	}

	/* ALU_SRC_1 */
	if (sel == 249) {
		float f = 1.0f;
		value = * (uint32_t *) &f;
		return value;
	}

	/* ALU_SRC_1_INT */
	if (sel == 250) {
		value = 1;
		return value;
	}

	/* ALU_SRC_M_1_INT */
	if (sel == 251) {
		value = -1;
		return value;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253) {
		assert(gpu_isa_inst->alu_group);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		value = * (uint32_t *) &gpu_isa_inst->alu_group->literal[chan];
		return value;
	}

	/* ALU_SRC_PV */
	if (sel == 254) {
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		value = gpu_isa_work_item->pv.elem[chan];
		return value;
	}

	/* ALU_SRC_PS */
	if (sel == 255) {
		value = gpu_isa_work_item->pv.elem[4];
		return value;
	}

	/* Not implemented 'sel' field */
	fatal("gpu_isa_read_op_src: src_idx=%d, not implemented for sel=%d", src_idx, sel);
	return 0;
}


uint32_t gpu_isa_read_op_src_int(int src_idx)
{
	int neg, abs;
	int32_t value; /* Signed */

	value = gpu_isa_read_op_src_common(src_idx, &neg, &abs);

	/* Absolute value and negation */
	if (abs && value < 0)
		value = -value;
	if (neg)
		value = -value;
	
	/* Return as unsigned */
	return value;
}


float gpu_isa_read_op_src_float(int src_idx)
{
	uint32_t value;
	float value_float;
	int neg, abs;

	value = gpu_isa_read_op_src_common(src_idx, &neg, &abs);
	value_float = * (float *) &value;

	/* Absolute value and negation */
	if (abs && value_float < 0.0)
		value_float = -value_float;
	if (neg)
		value_float = -value_float;
	return value_float;
}




/*
 * Deferred tasks for ALU group
 */


void gpu_isa_enqueue_write_lds(uint32_t addr, uint32_t value)
{
	struct gpu_isa_write_task_t *wt;

	/* Inactive pixel not enqueued */
	if (!gpu_work_item_get_pred(gpu_isa_work_item))
		return;
	
	/* Create task */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_WRITE_LDS;
	wt->inst = gpu_isa_inst;
	wt->lds_addr = addr;
	wt->lds_value = value;

	/* Enqueue task */
	lnlist_add(gpu_isa_work_item->write_task_list, wt);
}


/* Write to destination operand in ALU instruction */
void gpu_isa_enqueue_write_dest(uint32_t value)
{
	struct gpu_isa_write_task_t *wt;

	/* If pixel is inactive, do not enqueue the task */
	assert(gpu_isa_inst->info->fmt[0] == FMT_ALU_WORD0);
	if (!gpu_work_item_get_pred(gpu_isa_work_item))
		return;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * ALU_WORD1_OP2 and ALU_WORD1_OP3 formats. */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_WRITE_DEST;
	wt->inst = gpu_isa_inst;
	wt->gpr = ALU_WORD1_OP2.dst_gpr;
	wt->rel = ALU_WORD1_OP2.dst_rel;
	wt->chan = ALU_WORD1_OP2.dst_chan;
	wt->index_mode = ALU_WORD0.index_mode;
	wt->value = value;

	/* For ALU_WORD1_OP2, check 'write_mask' field */
	wt->write_mask = 1;
	if (gpu_isa_inst->info->fmt[1] == FMT_ALU_WORD1_OP2 && !ALU_WORD1_OP2.write_mask)
		wt->write_mask = 0;

	/* Enqueue task */
	lnlist_add(gpu_isa_work_item->write_task_list, wt);
}


void gpu_isa_enqueue_write_dest_float(float value_float)
{
	uint32_t value;

	value = * (uint32_t *) &value_float;
	gpu_isa_enqueue_write_dest(value);
}


void gpu_isa_enqueue_push_before(void)
{
	struct gpu_isa_write_task_t *wt;

	/* Do only if instruction initiating ALU clause is ALU_PUSH_BEFORE */
	if (gpu_isa_cf_inst->info->inst != AMD_INST_ALU_PUSH_BEFORE)
		return;

	/* Create and enqueue task */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_PUSH_BEFORE;
	wt->inst = gpu_isa_inst;
	lnlist_add(gpu_isa_work_item->write_task_list, wt);
}


void gpu_isa_enqueue_pred_set(int cond)
{
	struct gpu_isa_write_task_t *wt;

	/* If pixel is inactive, predicate is not changed */
	assert(gpu_isa_inst->info->fmt[0] == FMT_ALU_WORD0);
	assert(gpu_isa_inst->info->fmt[1] == FMT_ALU_WORD1_OP2);
	if (!gpu_work_item_get_pred(gpu_isa_work_item))
		return;
	
	/* Create and enqueue task */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_SET_PRED;
	wt->inst = gpu_isa_inst;
	wt->cond = cond;
	lnlist_add(gpu_isa_work_item->write_task_list, wt);
}


void gpu_isa_write_task_commit(void)
{
	struct lnlist_t *task_list = gpu_isa_work_item->write_task_list;
	struct gpu_isa_write_task_t *wt;

	/* Process first tasks of type:
	 *  - GPU_ISA_WRITE_TASK_WRITE_DEST
	 *  - GPU_ISA_WRITE_TASK_WRITE_LDS
	 */
	for (lnlist_head(task_list); !lnlist_eol(task_list); )
	{

		/* Get task */
		wt = lnlist_get(task_list);
		gpu_isa_inst = wt->inst;

		switch (wt->kind) {
		
		case GPU_ISA_WRITE_TASK_WRITE_DEST:
		{
			if (wt->write_mask)
				gpu_isa_write_gpr(wt->gpr, wt->rel, wt->chan, wt->value);
			gpu_isa_work_item->pv.elem[wt->inst->alu] = wt->value;

			/* Debug */
			if (gpu_isa_debugging()) {
				gpu_isa_debug("  i%d:%s", gpu_isa_work_item->id,
					map_value(&amd_pv_map, wt->inst->alu));
				if (wt->write_mask) {
					gpu_isa_debug(",");
					amd_inst_dump_gpr(wt->gpr, wt->rel, wt->chan, 0,
						debug_file(gpu_isa_debug_category));
				}
				gpu_isa_debug("<=");
				gpu_isa_dest_value_dump(&wt->value, debug_file(gpu_isa_debug_category));
			}

			break;
		}

		case GPU_ISA_WRITE_TASK_WRITE_LDS:
		{
			struct mem_t *local_mem;

			local_mem = gpu_isa_work_group->local_mem;
			assert(local_mem);
			mem_write(local_mem, wt->lds_addr, 4, &wt->lds_value);
			gpu_isa_debug("  i%d:LDS[0x%x]<=(%u,%gf)", gpu_isa_work_item->id, wt->lds_addr,
				wt->lds_value, * (float *) &wt->lds_value);
			break;
		}

		default:
			lnlist_next(task_list);
			continue;
		}

		/* Done with this task */
		repos_free_object(gpu_isa_write_task_repos, wt);
		lnlist_remove(task_list);
	}

	/* Process PUSH_BEFORE, PRED_SET */
	for (lnlist_head(task_list); !lnlist_eol(task_list); ) {
		
		/* Get task */
		wt = lnlist_get(task_list);
		gpu_isa_inst = wt->inst;

		/* Process */
		switch (wt->kind) {

		case GPU_ISA_WRITE_TASK_PUSH_BEFORE:
		{
			if (!gpu_isa_wavefront->push_before_done)
				gpu_wavefront_stack_push(gpu_isa_wavefront);
			gpu_isa_wavefront->push_before_done = 1;
			break;
		}

		case GPU_ISA_WRITE_TASK_SET_PRED:
		{
			int update_pred = ALU_WORD1_OP2.update_pred;
			int update_exec_mask = ALU_WORD1_OP2.update_exec_mask;

			assert(gpu_isa_inst->info->fmt[1] == FMT_ALU_WORD1_OP2);
			if (update_pred)
				gpu_work_item_set_pred(gpu_isa_work_item, wt->cond);
			if (update_exec_mask)
				gpu_work_item_set_active(gpu_isa_work_item, wt->cond);

			/* Debug */
			if (debug_status(gpu_isa_debug_category)) {
				if (update_pred && update_exec_mask)
					gpu_isa_debug("  i%d:act/pred<=%d", gpu_isa_work_item->id, wt->cond);
				else if (update_pred)
					gpu_isa_debug("  i%d:pred=%d", gpu_isa_work_item->id, wt->cond);
				else if (update_exec_mask)
					gpu_isa_debug("  i%d:pred=%d", gpu_isa_work_item->id, wt->cond);
			}
			break;
		}

		default:
			abort();
		}
		
		/* Done with task */
		repos_free_object(gpu_isa_write_task_repos, wt);
		lnlist_remove(task_list);
	}

	/* List should be empty */
	assert(!lnlist_count(task_list));
}

