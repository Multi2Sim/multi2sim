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


/* FIXME: configurable */
int gpu_alu_engine_inst_queue_size = 4;  /* Number of instructions */
int gpu_alu_engine_fetch_queue_size = 64;  /* Number of bytes */


void gpu_alu_engine_read(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->alu_engine.wavefront;
	struct lnlist_t *inst_queue = compute_unit->alu_engine.inst_queue;
	struct gpu_uop_t *uop;

	/* Get instruction at the head of the instruction queue */
	lnlist_head(inst_queue);
	uop = lnlist_get(inst_queue);

	/////// FIXME //////////
	if (!lnlist_count(inst_queue) && !lnlist_count(compute_unit->alu_engine.fetch_queue)
			&& wavefront->clause_kind != GPU_CLAUSE_ALU)
	{
		compute_unit->alu_engine.wavefront = NULL;
		return;
	}
	///////////////////////

	/* If there was no uop in the queue, done */
	if (!uop)
		return;

	/* Extract uop from instruction queue */
	lnlist_remove(inst_queue);

	///////// FIXME ////////////
	gpu_uop_free(uop);//////
	////////////////////////////
}


void gpu_alu_engine_decode(struct gpu_compute_unit_t *compute_unit)
{
	struct lnlist_t *fetch_queue = compute_unit->alu_engine.fetch_queue;
	struct lnlist_t *inst_queue = compute_unit->alu_engine.inst_queue;
	struct gpu_uop_t *uop;

	/* Get instruction at the head of the fetch queue */
	lnlist_head(fetch_queue);
	uop = lnlist_get(fetch_queue);

	/* If there was no uop in the queue, done */
	if (!uop)
		return;

	/* If uop is still being fetch from instruction memory, done */
	if (!uop->inst_cache_witness)
		return;

	/* If there is no space in the instruction queue, done */
	if (lnlist_count(inst_queue) >= gpu_alu_engine_inst_queue_size)
		return;

	/* Extract uop from fetch queue and insert it into instruction queue */
	lnlist_remove(fetch_queue);
	lnlist_out(inst_queue);
	lnlist_insert(inst_queue, uop);

	/* Debug */
	gpu_pipeline_debug("alu a=\"decode\" "
		"uop=%lld\n",
		uop->id);
}


void gpu_alu_engine_fetch(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->alu_engine.wavefront;
	struct lnlist_t *fetch_queue = compute_unit->alu_engine.fetch_queue;
	struct gpu_uop_t *uop;

	struct amd_inst_t *inst;
	char str1[MAX_STRING_SIZE], str2[MAX_STRING_SIZE];
	int i;

	/* If wavefront finished the ALU clause, no more instruction fetch */
	if (!wavefront || wavefront->clause_kind != GPU_CLAUSE_ALU)
		return;

	/* If fetch queue is full, cannot fetch until space is made */
	/* FIXME */
	if (lnlist_count(fetch_queue) > 1)
		return;

	/* Emulate instruction */
	gpu_wavefront_execute(wavefront);

	/* Create new uop */
	uop = gpu_uop_create();
	amd_alu_group_copy(&uop->alu_group, &wavefront->alu_group);

	/* Access instruction cache */
	/* FIXME */
	uop->inst_cache_witness = 1;

	/* Enqueue instruction into fetch queue */
	lnlist_out(fetch_queue);
	lnlist_insert(fetch_queue, uop);

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category)) {
		gpu_pipeline_debug("alu a=\"fetch\" "
			"uop=%lld "
			"inst=\"",
			uop->id);
		for (i = 0; i < wavefront->alu_group.inst_count; i++) {
			inst = &wavefront->alu_group.inst[i];
			amd_inst_dump_buf(inst, -1, 0, str1, MAX_STRING_SIZE);
			str_single_spaces(str2, str1, MAX_STRING_SIZE);
			gpu_pipeline_debug(
				"inst.%s=\"%s\" ",
				map_value(&amd_alu_map, inst->alu),
				str2);
		}
		gpu_pipeline_debug("\"\n");
	}
}


void gpu_alu_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->alu_engine.wavefront;

	/* If no wavefront assigned to ALU Engine, nothing to do. */
	if (!wavefront)
		return;

	/* ALU Engine stages */
	gpu_alu_engine_read(compute_unit);
	gpu_alu_engine_decode(compute_unit);
	gpu_alu_engine_fetch(compute_unit);
}
