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
#include <heap.h>


int gpu_alu_engine_inst_mem_latency = 2;  /* Latency of instruction memory */
int gpu_alu_engine_inst_queue_size = 4;  /* Number of instructions */
int gpu_alu_engine_fetch_queue_size = 64;  /* Number of bytes */
int gpu_alu_engine_pe_latency = 4;  /* Processing element latency */


void gpu_alu_engine_write(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_uop_t *uop, *consumer;
	uint64_t cycle;

	int odep;
	int i;

	for (;;) {

		/* Extract a new event for this cycle */
		cycle = heap_peek(compute_unit->alu_engine.event_queue, (void **) &uop);
		if (!uop || cycle > gpu->cycle)
			break;
		assert(cycle == gpu->cycle);
		heap_extract(compute_unit->alu_engine.event_queue, NULL);

		/* One more SubWF writes */
		assert(uop->write_subwavefront_count < uop->subwavefront_count);
		uop->write_subwavefront_count++;
		gpu_pipeline_debug("alu a=\"write\" "
			"cu=%d "
			"uop=%lld "
			"subwf=%d\n",
			compute_unit->id,
			(long long) uop->id,
			uop->write_subwavefront_count - 1);

		/* If this is the first SubWF to write, wake up dependent instructions. */
		if (uop->write_subwavefront_count == 1) {

			/* Wake up consumers */
			while (uop->dep_list_head) {
				consumer = uop->dep_list_head;
				consumer->ready = 1;
				DOUBLE_LINKED_LIST_REMOVE(uop, dep, consumer);
			}

			/* Delete producer from 'producers' array */
			for (i = 0; i < uop->odep_count; i++) {
				odep = uop->odep[i];
				if (compute_unit->alu_engine.producers[odep] == uop)
					compute_unit->alu_engine.producers[odep] = NULL;
			}
		}

		/* If this is the last SubWF to write, free uop */
		if (uop->write_subwavefront_count == uop->subwavefront_count) {
			if (uop->last) {
				assert(!heap_count(compute_unit->alu_engine.event_queue));
				assert(!lnlist_count(compute_unit->alu_engine.fetch_queue));
				assert(!lnlist_count(compute_unit->alu_engine.inst_queue));
				compute_unit->alu_engine.wavefront = NULL;
			}
			gpu_uop_free(uop);
		}
	}
}


void gpu_alu_engine_read(struct gpu_compute_unit_t *compute_unit)
{
	struct lnlist_t *inst_queue = compute_unit->alu_engine.inst_queue;
	struct gpu_uop_t *uop;

	/* Get instruction at the head of the instruction queue */
	lnlist_head(inst_queue);
	uop = lnlist_get(inst_queue);

	/* If no instruction, or instruction at the head is not ready, done */
	if (!uop || !uop->ready)
		return;
	
	/* One more SubWF issued for uop */
	assert(uop->read_subwavefront_count < uop->subwavefront_count);
	uop->read_subwavefront_count++;
	gpu_pipeline_debug("alu a=\"read\" "
		"cu=%d "
		"uop=%lld "
		"subwf=%d\n",
		compute_unit->id,
		(long long) uop->id,
		uop->read_subwavefront_count - 1);

	/* Enqueue one writeback event for each SubWF.
	 * Since processing elements in stream cores are pipelined, they
	 * accept a new instruction every cycle, so no contention. */
	heap_insert(compute_unit->alu_engine.event_queue,
		(long long) gpu->cycle + gpu_alu_engine_pe_latency,
		uop);
	
	/* If this is the last subwavefront, remove instruction from IQ */
	if (uop->read_subwavefront_count == uop->subwavefront_count)
		lnlist_remove(inst_queue);
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

	/* If uop is still being fetched from instruction memory, done */
	if (uop->inst_mem_ready > gpu->cycle)
		return;

	/* If there is no space in the instruction queue, done */
	if (lnlist_count(inst_queue) >= gpu_alu_engine_inst_queue_size)
		return;

	/* Extract uop from fetch queue */
	lnlist_remove(fetch_queue);
	compute_unit->alu_engine.fetch_queue_length -= uop->length;
	assert(compute_unit->alu_engine.fetch_queue_length >= 0);

	/* Insert into instruction queue */
	lnlist_out(inst_queue);
	lnlist_insert(inst_queue, uop);

	/* Debug */
	gpu_pipeline_debug("alu a=\"decode\" "
		"cu=%d\n"
		"uop=%lld\n",
		compute_unit->id,
		(long long) uop->id);
}


void gpu_alu_engine_fetch(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->alu_engine.wavefront;
	struct lnlist_t *fetch_queue = compute_unit->alu_engine.fetch_queue;
	struct amd_alu_group_t *alu_group;
	struct gpu_uop_t *uop, *producer;

	struct amd_inst_t *inst;
	char str1[MAX_STRING_SIZE], str2[MAX_STRING_SIZE];

	int idep, odep;
	int i;

	/* If wavefront finished the ALU clause, no more instruction fetch */
	if (!wavefront || wavefront->clause_kind != GPU_CLAUSE_ALU)
		return;

	/* If fetch queue is full, cannot fetch until space is made */
	if (compute_unit->alu_engine.fetch_queue_length >= gpu_alu_engine_fetch_queue_size)
		return;

	/* Emulate instruction and create uop */
	gpu_wavefront_execute(wavefront);
	alu_group = &wavefront->alu_group;
	uop = gpu_uop_create_from_alu_group(alu_group);
	uop->subwavefront_count = (wavefront->work_item_count + gpu_num_stream_cores - 1)
		/ gpu_num_stream_cores;
	uop->last = wavefront->clause_kind != GPU_CLAUSE_ALU;
	uop->length = alu_group->inst_count * 8 + alu_group->literal_count * 4;

	/* Array 'producers' contains those uops in execution that produce each possible
	 * output dependence. Find the youngest producer for this uop. */
	producer = NULL;
	for (i = 0; i < uop->idep_count; i++) {
		idep = uop->idep[i];
		if (!compute_unit->alu_engine.producers[idep])
			continue;
		if (!producer || producer->id < compute_unit->alu_engine.producers[idep]->id)
			producer = compute_unit->alu_engine.producers[idep];
	}

	/* If there was a producer, enqueue uop in its dependence list. Otherwise, uop is ready. */
	if (producer) {
		DOUBLE_LINKED_LIST_INSERT_TAIL(producer, dep, uop);
	} else {
		uop->ready = 1;
	}

	/* Record output dependences of current instruction in 'producers' array. */
	for (i = 0; i < uop->odep_count; i++) {
		odep = uop->odep[i];
		assert(IN_RANGE(odep, 1, GPU_UOP_DEP_COUNT - 1));
		compute_unit->alu_engine.producers[odep] = uop;
	}

	/* Access instruction cache. Record the time when the instruction will have been fetched,
	 * as per the latency of the instruction memory. */
	uop->inst_mem_ready = gpu->cycle + gpu_alu_engine_inst_mem_latency;

	/* Enqueue instruction into fetch queue */
	lnlist_out(fetch_queue);
	lnlist_insert(fetch_queue, uop);
	compute_unit->alu_engine.fetch_queue_length += uop->length;

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category)) {
		gpu_pipeline_debug("alu a=\"fetch\" "
			"cu=%d "
			"uop=%lld "
			"inst=\"",
			compute_unit->id,
			(long long) uop->id);
		for (i = 0; i < wavefront->alu_group.inst_count; i++) {
			inst = &wavefront->alu_group.inst[i];
			amd_inst_dump_buf(inst, -1, 0, str1, MAX_STRING_SIZE);
			str_single_spaces(str2, str1, MAX_STRING_SIZE);
			gpu_pipeline_debug(
				"inst.%s=\"%s\" ",
				map_value(&amd_alu_map, inst->alu),
				str2);
		}
		gpu_pipeline_debug("\" idep=");
		gpu_uop_dump_dep_list(str1, MAX_STRING_SIZE, uop->idep, uop->idep_count);
		gpu_pipeline_debug("%s odep=", str1);
		gpu_uop_dump_dep_list(str1, MAX_STRING_SIZE, uop->odep, uop->odep_count);
		gpu_pipeline_debug("%s", str1);
		if (producer)
			gpu_pipeline_debug(" prod=%lld", (long long) producer->id);
		gpu_pipeline_debug("\n");
	}
}


void gpu_alu_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->alu_engine.wavefront;

	/* If no wavefront assigned to ALU Engine, nothing to do. */
	if (!wavefront)
		return;

	/* ALU Engine stages */
	gpu_alu_engine_write(compute_unit);
	gpu_alu_engine_read(compute_unit);
	gpu_alu_engine_decode(compute_unit);
	gpu_alu_engine_fetch(compute_unit);
}
