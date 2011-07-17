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
int gpu_tex_engine_inst_mem_latency = 2;  /* Instruction memory latency */
int gpu_tex_engine_fetch_queue_size = 32;  /* Number of bytes */


void gpu_tex_engine_write(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_uop_t *uop;

	/* Get instruction at the write buffer. */
	uop = compute_unit->tex_engine.write_buffer;
	if (!uop)
		return;
	
	/* If memory reads did not complete, done. */
	if (uop->global_mem_witness)
		return;
	
	/* Clear write buffer, and finish instruction. */
	compute_unit->tex_engine.write_buffer = NULL;

	/* Debug */
	gpu_pipeline_debug("tex a=\"write\" "
		"cu=%d "
		"uop=%lld\n",
		compute_unit->id,
		(long long) uop->id);

	/* Last uop in clause */
	if (uop->last) {
		lnlist_out(compute_unit->cf_engine.complete_queue);
		lnlist_insert(compute_unit->cf_engine.complete_queue,
			compute_unit->tex_engine.cf_uop);

		compute_unit->tex_engine.cf_uop = NULL;
		compute_unit->tex_engine.wavefront = NULL;
	}

	/* Free uop */
	gpu_uop_free(uop);
}


void gpu_tex_engine_read(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->tex_engine.wavefront;
	struct gpu_ndrange_t *ndrange = wavefront->ndrange;

	struct gpu_work_item_t *work_item;
	int work_item_id;

	struct gpu_uop_t *uop;
	struct gpu_work_item_uop_t *work_item_uop;

	/* If there is no instruction in instruction buffer, done */
	uop = compute_unit->tex_engine.inst_buffer;
	if (!uop)
		return;
	
	/* If there is no space in write buffer, an uop is currently
	 * being processed in the memory hierarchy, done. */
	if (compute_unit->tex_engine.write_buffer)
		return;
	
	/* Move uop from instruction buffer into write buffer */
	assert(!compute_unit->tex_engine.write_buffer);
	compute_unit->tex_engine.inst_buffer = NULL;
	compute_unit->tex_engine.write_buffer = uop;

	/* Global memory read  */
	if (uop->global_mem_read) {
		assert(!uop->global_mem_witness);
		FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id) {
			work_item = ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			gpu_cache_access(compute_unit->data_cache, 1, work_item_uop->global_mem_access_addr,
					work_item_uop->global_mem_access_size, &uop->global_mem_witness);
			uop->global_mem_witness--;
		}
	}
	
	/* Debug */
	gpu_pipeline_debug("tex a=\"read\" "
		"cu=%d "
		"uop=%lld\n",
		compute_unit->id,
		(long long) uop->id);
}


void gpu_tex_engine_decode(struct gpu_compute_unit_t *compute_unit)
{
	struct lnlist_t *fetch_queue = compute_unit->tex_engine.fetch_queue;
	struct gpu_uop_t *uop;

	/* Get instruction at the head of the fetch queue */
	lnlist_head(fetch_queue);
	uop = lnlist_get(fetch_queue);

	/* If there was no uop in the queue, done */
	if (!uop)
		return;

	/* If uop is still being fetched from instruction memory, done */
	if (uop->inst_mem_ready > gpu->cycle)
		return;

	/* If instruction buffer is occupied, done */
	if (compute_unit->tex_engine.inst_buffer)
		return;

	/* Extract uop from fetch queue */
	lnlist_remove(fetch_queue);
	compute_unit->tex_engine.fetch_queue_length -= uop->length;
	assert(compute_unit->tex_engine.fetch_queue_length >= 0);

	/* Insert into instruction buffer */
	assert(!compute_unit->tex_engine.inst_buffer);
	compute_unit->tex_engine.inst_buffer = uop;

	/* Debug */
	gpu_pipeline_debug("tex a=\"decode\" "
		"cu=%d "
		"uop=%lld\n",
		compute_unit->id,
		(long long) uop->id);
}


void gpu_tex_engine_fetch(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->tex_engine.wavefront;
	struct gpu_ndrange_t *ndrange = wavefront->ndrange;

	struct gpu_uop_t *uop;
	struct gpu_work_item_uop_t *work_item_uop;
	struct amd_inst_t *inst;
	int inst_num;

	struct gpu_work_item_t *work_item;
	int work_item_id;

	char str1[MAX_STRING_SIZE], str2[MAX_STRING_SIZE];

	/* If wavefront finished the TEX clause, no more instruction fetch */
	if (!wavefront || wavefront->clause_kind != GPU_CLAUSE_TEX)
		return;

	/* If fetch queue is full, cannot fetch until space is made */
	if (compute_unit->tex_engine.fetch_queue_length >= gpu_tex_engine_fetch_queue_size)
		return;
	
	/* Emulate instruction and create uop */
	inst_num = (wavefront->clause_buf - wavefront->clause_buf_start) / 16;
	gpu_wavefront_execute(wavefront);
	inst = &wavefront->tex_inst;
	uop = gpu_uop_create();
	uop->last = wavefront->clause_kind != GPU_CLAUSE_TEX;
	uop->global_mem_read = wavefront->global_mem_read;
	uop->global_mem_write = wavefront->global_mem_write;

	/* Stats */
	compute_unit->inst_count++;
	compute_unit->tex_engine.inst_count++;
	
	/* If instruction is a global memory read (should be), record addresses */
	if (uop->global_mem_read) {
		assert((inst->info->flags & AMD_INST_FLAG_MEM_READ));
		FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id) {
			work_item = ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			work_item_uop->global_mem_access_addr = work_item->global_mem_access_addr;
			work_item_uop->global_mem_access_size = work_item->global_mem_access_size;
		}
	}

	/* Access instruction cache. Record the time when the instruction will have been fetched,
	 * as per the latency of the instruction memory. */
	uop->inst_mem_ready = gpu->cycle + gpu_tex_engine_inst_mem_latency;

	/* Enqueue uop into fetch queue */
	lnlist_out(compute_unit->tex_engine.fetch_queue);
	lnlist_insert(compute_unit->tex_engine.fetch_queue, uop);
	compute_unit->tex_engine.fetch_queue_length += uop->length;

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category)) {
		amd_inst_dump_buf(inst, inst_num, 0, str1, MAX_STRING_SIZE);
		str_single_spaces(str2, str1, MAX_STRING_SIZE);
		gpu_pipeline_debug("tex a=\"fetch\" "
			"cu=%d "
			"wf=%d "
			"inst=\"%s\"\n",
			compute_unit->id,
			wavefront->id,
			str2);
	}
}


void gpu_tex_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->tex_engine.wavefront;

	/* If no wavefront assigned to TEX Engine, nothing to do. */
	if (!wavefront)
		return;

	/* TEX Engine stages */
	gpu_tex_engine_write(compute_unit);
	gpu_tex_engine_read(compute_unit);
	gpu_tex_engine_decode(compute_unit);
	gpu_tex_engine_fetch(compute_unit);

	/* Stats */
	compute_unit->tex_engine.cycle++;
}

