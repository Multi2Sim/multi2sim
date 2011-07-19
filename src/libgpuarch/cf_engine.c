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


void gpu_cf_engine_complete(struct gpu_compute_unit_t *compute_unit)
{
	struct lnlist_t *complete_queue = compute_unit->cf_engine.complete_queue;
	struct lnlist_t *wavefront_pool = compute_unit->cf_engine.wavefront_pool;
	struct gpu_work_group_t *work_group;
	struct gpu_uop_t *uop;

	/* Process all uops in the complete queue */
	while (lnlist_count(complete_queue)) {
		
		/* Extract uop from complete queue */
		lnlist_head(complete_queue);
		uop = lnlist_get(complete_queue);
		work_group = uop->work_group;
		lnlist_remove(complete_queue);

		/* Instruction finishes a wavefront */
		if (uop->last)
			work_group->compute_unit_finished_count++;
	
		/* Insert wavefront into wavefront pool */
		else {
			lnlist_insert(wavefront_pool, uop->wavefront);
			lnlist_next_circular(wavefront_pool);
		}

		/* Debug */
		gpu_pipeline_debug("cf a=\"complete\" "
			"cu=%d "
			"uop=%lld\n",
			compute_unit->id,
			(long long) uop->id);

		/* Free uop */
		gpu_uop_free(uop);

		/* Wavefront finishes a work-group */
		assert(work_group->compute_unit_finished_count <= work_group->wavefront_count);
		if (work_group->compute_unit_finished_count == work_group->wavefront_count)
			gpu_compute_unit_unmap_work_group(compute_unit, work_group);
	}
}


void gpu_cf_engine_execute(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_work_group_t *work_group;
	struct gpu_ndrange_t *ndrange;
	struct gpu_uop_t *uop;
	int index;

	struct gpu_work_item_uop_t *work_item_uop;
	struct gpu_work_item_t *work_item;
	int work_item_id;

	/* Search entry in instruction buffer to decode */
	index = compute_unit->cf_engine.execute_index;
	for (;;) {
		
		/* There must be an instruction buffer slot ready, and the resource
		 * required by the instruction must be available. */
		uop = compute_unit->cf_engine.inst_buffer[index];
		if (uop) {
			work_group = uop->work_group;
			ndrange = work_group->ndrange;
			if (uop->alu_clause_trigger && !compute_unit->alu_engine.wavefront)
				break;
			if (uop->tex_clause_trigger && !compute_unit->tex_engine.wavefront)
				break;
			if (uop->no_clause_trigger)
				break;
		}

		/* Current candidate is not valid - go to next.
		 * If we went through the whole instruction buffer, no execute. */
		index = (index + 1) % gpu->wavefronts_per_compute_unit;
		if (index == compute_unit->cf_engine.execute_index)
			return;
	}

	/* Remove entry from instruction buffer */
	compute_unit->cf_engine.inst_buffer[index] = NULL;

	/* Execute instruction */
	if (uop->alu_clause_trigger) {
		
		/* Trigger secondary ALU clause */
		assert(!compute_unit->alu_engine.wavefront);
		compute_unit->alu_engine.cf_uop = uop;
		compute_unit->alu_engine.wavefront = uop->wavefront;
		compute_unit->alu_engine.wavefront_count++;

	} else if (uop->tex_clause_trigger) {
		
		/* Trigger secondary TEX clause */
		assert(!compute_unit->tex_engine.wavefront);
		compute_unit->tex_engine.cf_uop = uop;
		compute_unit->tex_engine.wavefront = uop->wavefront;
		compute_unit->tex_engine.wavefront_count++;

	} else {
		
		/* Execute instruction in CF Engine - completed next cycle */
		lnlist_out(compute_unit->cf_engine.complete_queue);
		lnlist_insert(compute_unit->cf_engine.complete_queue, uop);

		/* Global memory write - execute asynchronously */
		if (uop->global_mem_write) {
			FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id) {
				work_item = ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				gpu_cache_access(compute_unit->data_cache, 2, work_item_uop->global_mem_access_addr,
					work_item_uop->global_mem_access_size, NULL);
			}
		}
	}

	/* Set next execute candidate */
	compute_unit->cf_engine.execute_index = (index + 1)
		% gpu->wavefronts_per_compute_unit;
	
	/* Debug */
	gpu_pipeline_debug("cf a=\"execute\" "
		"cu=%d "
		"uop=%lld\n",
		compute_unit->id,
		(long long) uop->id);
}


void gpu_cf_engine_decode(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_uop_t *uop;
	int index;

	/* Search entry in fetch buffer to decode */
	index = compute_unit->cf_engine.decode_index;
	for (;;) {
		
		/* There must be a fetch buffer slot ready (instruction fetch finished),
		 * and the corresponding slot in the instruction buffer must be free. */
		uop = compute_unit->cf_engine.fetch_buffer[index];
		if (uop && uop->inst_mem_ready && !compute_unit->cf_engine.inst_buffer[index])
			break;

		/* Current candidate is not valid - go to next.
		 * If we went through the whole fetch buffer, no decode. */
		index = (index + 1) % gpu->wavefronts_per_compute_unit;
		if (index == compute_unit->cf_engine.decode_index)
			return;
	}

	/* Decode instruction */
	compute_unit->cf_engine.fetch_buffer[index] = NULL;
	compute_unit->cf_engine.inst_buffer[index] = uop;

	/* Set next decode candidate */
	compute_unit->cf_engine.decode_index = (index + 1)
		% gpu->wavefronts_per_compute_unit;
	
	/* Debug */
	gpu_pipeline_debug("cf a=\"decode\" "
		"cu=%d "
		"uop=%lld\n",
		compute_unit->id,
		(long long) uop->id);
}


void gpu_cf_engine_fetch(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_ndrange_t *ndrange = gpu->ndrange;
	struct lnlist_t *wavefront_pool = compute_unit->cf_engine.wavefront_pool;
	struct gpu_wavefront_t *wavefront, *temp_wavefront;

	char str1[MAX_STRING_SIZE], str2[MAX_STRING_SIZE];
	struct amd_inst_t *inst;
	int inst_num;

	struct gpu_uop_t *uop;
	struct gpu_work_item_uop_t *work_item_uop;
	struct gpu_work_item_t *work_item;
	int work_item_id;


	/* If there is no wavefront in the pool, done */
	if (!lnlist_count(wavefront_pool))
		return;
	
	/* Select current position in pool as initial candidate wavefront */
	if (!lnlist_get(wavefront_pool))
		lnlist_head(wavefront_pool);
	wavefront = lnlist_get(wavefront_pool);
	temp_wavefront = wavefront;

	/* Look for a valid candidate */
	for (;;) {
		
		/* Wavefront must be running,
		 * and the corresponding slot in fetch buffer must be free. */
		assert(wavefront->id_in_compute_unit < gpu->wavefronts_per_compute_unit);
		if (DOUBLE_LINKED_LIST_MEMBER(wavefront->work_group, running, wavefront) &&
			!compute_unit->cf_engine.fetch_buffer[wavefront->id_in_compute_unit])
			break;

		/* Current candidate is not valid - go to next.
		 * If we went through the whole pool, no fetch. */
		lnlist_next_circular(wavefront_pool);
		wavefront = lnlist_get(wavefront_pool);
		if (wavefront == temp_wavefront)
			return;
	}

	/* Wavefront found, remove from pool. */
	assert(wavefront->clause_kind == GPU_CLAUSE_CF);
	lnlist_remove(wavefront_pool);

	/* Emulate CF instruction */
	inst_num = (wavefront->cf_buf - ndrange->kernel->cal_abi->text_buffer) / 8;
	gpu_wavefront_execute(wavefront);
	inst = &wavefront->cf_inst;

	/* Create uop */
	uop = gpu_uop_create();
	uop->wavefront = wavefront;
	uop->work_group = wavefront->work_group;
	uop->alu_clause_trigger = wavefront->clause_kind == GPU_CLAUSE_ALU;
	uop->tex_clause_trigger = wavefront->clause_kind == GPU_CLAUSE_TEX;
	uop->no_clause_trigger = wavefront->clause_kind == GPU_CLAUSE_CF;
	uop->last = DOUBLE_LINKED_LIST_MEMBER(wavefront->work_group, finished, wavefront);
	uop->global_mem_read = wavefront->global_mem_read;
	uop->global_mem_write = wavefront->global_mem_write;

	/* If instruction is a global memory write, record addresses */
	if (uop->global_mem_write) {
		assert((inst->info->flags & AMD_INST_FLAG_MEM_WRITE));
		FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id) {
			work_item = ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			work_item_uop->global_mem_access_addr = work_item->global_mem_access_addr;
			work_item_uop->global_mem_access_size = work_item->global_mem_access_size;
		}
	}

	/* Access instruction cache. Record the time when the instruction will have been fetched,
	 * as per the latency of the instruction memory. */
	uop->inst_mem_ready = gpu->cycle + gpu_cf_engine_inst_mem_latency;

	/* Insert uop to fetch buffer */
	assert(!compute_unit->cf_engine.fetch_buffer[wavefront->id_in_compute_unit]);
	compute_unit->cf_engine.fetch_buffer[wavefront->id_in_compute_unit] = uop;

	/* Stats */
	compute_unit->inst_count++;
	compute_unit->cf_engine.inst_count++;
	if (uop->global_mem_write)
		compute_unit->cf_engine.global_mem_write_count++;
	if (uop->alu_clause_trigger)
		compute_unit->cf_engine.alu_clause_trigger_count++;
	if (uop->tex_clause_trigger)
		compute_unit->cf_engine.tex_clause_trigger_count++;

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category)) {
		amd_inst_dump_buf(inst, inst_num, 0, str1, MAX_STRING_SIZE);
		str_single_spaces(str2, str1, MAX_STRING_SIZE);
		gpu_pipeline_debug("cf a=\"fetch\" "
			"cu=%d "
			"wf=%d "
			"uop=%lld "
			"inst=\"%s\"\n",
			compute_unit->id,
			wavefront->id,
			(long long) uop->id,
			str2);
	}
}


void gpu_cf_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	/* Call CF Engine stages */
	gpu_cf_engine_complete(compute_unit);
	gpu_cf_engine_execute(compute_unit);
	gpu_cf_engine_decode(compute_unit);
	gpu_cf_engine_fetch(compute_unit);
}

