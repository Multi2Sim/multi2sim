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


#include <gpuarch.h>


/* Configuration parameters */
int gpu_cf_engine_inst_mem_latency = 2;  /* Instruction memory latency */
int gpu_cf_engine_inst_queue_size = 8;  /* Number of instructions */
int gpu_cf_engine_fetch_queue_size = 64;  /* Number of bytes */


#if 0
/* Based on the previous value of 'compute_unit->cf_engine.wavefront', assign the
 * new wavefront to run in the CF Engine. */
void gpu_cf_engine_schedule(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_work_group_t *work_group = compute_unit->work_group;
	struct gpu_wavefront_t *wavefront;

	/* If there is no running wavefront, nothing to do */
	if (!work_group->running_count) {
		compute_unit->cf_engine.wavefront = NULL;
		return;
	}

	/* If there was no previously running wavefront, or if previously running wavefront is not
	 * running anymore, select with first wavefront from running list. */
	wavefront = compute_unit->cf_engine.wavefront;
	if (!wavefront || !DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront))
		wavefront = work_group->running_list_head;

	/* Otherwise, move to next running wavefront. */
	else
		wavefront = wavefront->running_next ? wavefront->running_next
			: work_group->running_list_head;

	/* This wavefront is temporarily selected as next running wavefront */
	compute_unit->cf_engine.wavefront = wavefront;

	/* Now make sure that the wavefront is not executing in the ALU or TEX engine.
	 * Move through the running list until one is found. */
	if (compute_unit->alu_engine.wavefront == wavefront || compute_unit->tex_engine.wavefront == wavefront) {
		do {
			wavefront = wavefront->running_next ? wavefront->running_next
				: work_group->running_list_head;
			if (wavefront != compute_unit->alu_engine.wavefront && wavefront != compute_unit->tex_engine.wavefront)
				break;
			if (wavefront == compute_unit->cf_engine.wavefront)
				wavefront = NULL;
		} while (wavefront);
	}

	/* Final assignment of current CF wavefront. Exit if none is ready. */
	compute_unit->cf_engine.wavefront = wavefront;
	assert(!wavefront || wavefront->clause_kind == GPU_CLAUSE_CF);
}
#endif


void gpu_cf_engine_execute(struct gpu_compute_unit_t *compute_unit)
{
}


void gpu_cf_engine_decode(struct gpu_compute_unit_t *compute_unit)
{
}


void gpu_cf_engine_fetch(struct gpu_compute_unit_t *compute_unit)
{
}


void gpu_cf_engine_schedule(struct gpu_compute_unit_t *compute_unit)
{
	struct lnlist_t *wavefront_pool = compute_unit->cf_engine.wavefront_pool;
	struct gpu_work_group_t *work_group = compute_unit->work_group;
	struct gpu_wavefront_t *wavefront;

	/* If there is no wavefront in the pool, done */
	if (!lnlist_count(wavefront_pool))
		return;
	
	/* If fetch stage still didn't process the last scheduled wavefront, done */
	if (compute_unit->cf_engine.sched_wavefront)
		return;
	
	/* Make sure that current wavefront is a valid position in the list. */
	if (!lnlist_get(wavefront_pool))
		lnlist_head(wavefront_pool);
	
	/* Assign temporarily current wavefront as scheduled candidate */
	wavefront = lnlist_get(wavefront_pool);
	compute_unit->cf_engine.sched_wavefront = wavefront;

	/* Search for a candidate that is a running wavefront.
	 * If there is no running wavefront, done. */
	if (!DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront)) {
		do {
			lnlist_next_circular(wavefront_pool);
			wavefront = lnlist_get(wavefront_pool);
		} while (!DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront) &&
			wavefront != compute_unit->cf_engine.sched_wavefront);
		if (wavefront == compute_unit->cf_engine.sched_wavefront) {
			compute_unit->cf_engine.sched_wavefront = NULL;
			return;
		}
	}

	/* Assign final candidate, and extract it from wavefront pool. */
	compute_unit->cf_engine.sched_wavefront = wavefront;
	lnlist_remove(wavefront_pool);

	/* Debug */
	gpu_pipeline_debug("cf a=\"sched\" "
		"cu=%d "
		"wf=%d\n",
		compute_unit->id,
		wavefront->id);
}


void gpu_cf_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	/* Call CF Engine stages */
	assert(compute_unit->work_group);
	gpu_cf_engine_execute(compute_unit);
	gpu_cf_engine_decode(compute_unit);
	gpu_cf_engine_fetch(compute_unit);
	gpu_cf_engine_schedule(compute_unit);
}


#if 0
void gpu_cf_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_work_group_t *work_group = compute_unit->work_group;
	struct gpu_ndrange_t *ndrange = work_group->ndrange;
	struct gpu_wavefront_t *wavefront;

	char str1[MAX_STRING_SIZE], str2[MAX_STRING_SIZE];
	struct amd_inst_t *inst;
	int inst_num;

	/* Get next wavefront to run in the CF Engine */
	gpu_cf_engine_schedule(compute_unit);
	wavefront = compute_unit->cf_engine.wavefront;
	if (!wavefront) {
		if (!compute_unit->alu_engine.wavefront && !compute_unit->tex_engine.wavefront)
			gpu_compute_unit_unmap_work_group(compute_unit);
		return;
	}

	/* Emulate CF instruction */
	inst_num = (wavefront->cf_buf - ndrange->kernel->cal_abi->text_buffer) / 8;
	gpu_wavefront_execute(wavefront);
	inst = &wavefront->cf_inst;

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category)) {
		amd_inst_dump_buf(inst, inst_num, 0, str1, MAX_STRING_SIZE);
		str_single_spaces(str2, str1, MAX_STRING_SIZE);
		gpu_pipeline_debug("cf a=\"exec\" "
			"cu=%d "
			"wf=%d "
			"inst=\"%s\"\n",
			compute_unit->id,
			wavefront->id,
			str2);
	}

	/* CF instruction is triggering a secondary ALU clause */
	if (wavefront->clause_kind == GPU_CLAUSE_ALU) {
		compute_unit->alu_engine.wavefront = wavefront;
		gpu_pipeline_debug("alu a=\"start\" "
			"cu=%d "
			"wf=%d\n",
			compute_unit->id,
			wavefront->id);
	}

	/* CF instruction is triggering a secondary TEX clause */
	else if (wavefront->clause_kind == GPU_CLAUSE_TEX) {
		compute_unit->tex_engine.wavefront = wavefront;
		gpu_pipeline_debug("tex a=\"start\" "
			"cu=%d "
			"wf=%d\n",
			compute_unit->id,
			wavefront->id);
	}
}
#endif

