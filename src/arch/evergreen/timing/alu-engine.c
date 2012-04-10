/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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
int gpu_alu_engine_fetch_queue_size = 64;  /* Number of bytes */
int gpu_alu_engine_pe_latency = 4;  /* Processing element latency */


void gpu_alu_engine_fetch(struct gpu_compute_unit_t *compute_unit)
{
	struct linked_list_t *pending_queue = compute_unit->alu_engine.pending_queue;
	struct linked_list_t *finished_queue = compute_unit->alu_engine.finished_queue;

	struct linked_list_t *fetch_queue = compute_unit->alu_engine.fetch_queue;
	struct amd_alu_group_t *alu_group;
	struct gpu_uop_t *cf_uop, *uop, *producer;
	struct gpu_work_item_uop_t *work_item_uop;
	struct gpu_wavefront_t *wavefront;

	struct gpu_work_item_t *work_item;
	int work_item_id;

	int idep, odep;
	int i;

	/* Get wavefront to fetch from */
	linked_list_head(pending_queue);
	cf_uop = linked_list_get(pending_queue);
	if (!cf_uop)
		return;
	wavefront = cf_uop->wavefront;
	assert(wavefront->clause_kind == GPU_CLAUSE_ALU);

	/* If fetch queue is full, cannot fetch until space is made */
	if (compute_unit->alu_engine.fetch_queue_length >= gpu_alu_engine_fetch_queue_size)
		return;

	/* Emulate instruction and create uop */
	gpu_wavefront_execute(wavefront);
	alu_group = &wavefront->alu_group;
	uop = gpu_uop_create_from_alu_group(alu_group);
	uop->wavefront = wavefront;
	uop->work_group = wavefront->work_group;
	uop->cf_uop = cf_uop;
	uop->compute_unit = compute_unit;
	uop->id_in_compute_unit = compute_unit->gpu_uop_id_counter++;
	uop->subwavefront_count = (wavefront->work_item_count + gpu_num_stream_cores - 1)
		/ gpu_num_stream_cores;
	uop->last = wavefront->clause_kind != GPU_CLAUSE_ALU;
	uop->length = alu_group->inst_count * 8 + alu_group->literal_count * 4;
	uop->local_mem_read = wavefront->local_mem_read;
	uop->local_mem_write = wavefront->local_mem_write;
	uop->active_mask_update = wavefront->active_mask_update;
	uop->active_mask_push = wavefront->active_mask_push;
	uop->active_mask_pop = wavefront->active_mask_pop;
	uop->active_mask_stack_top = wavefront->stack_top;
	wavefront->alu_engine_in_flight++;

	/* If ALU clause finished, extract wavefront from pending wavefront queue, and
	 * insert it into finished wavefront queue. */
	if (uop->last)
	{
		linked_list_remove(pending_queue);
		linked_list_add(finished_queue, cf_uop);
	}

	/* If debugging active mask, store active state for work-items */
	if (debug_status(gpu_stack_debug_category))
		gpu_uop_save_active_mask(uop);

	/* Stats */
	compute_unit->inst_count++;
	compute_unit->alu_engine.inst_count++;
	compute_unit->alu_engine.inst_slot_count += alu_group->inst_count;
	if (uop->local_mem_read || uop->local_mem_write)
		compute_unit->alu_engine.local_mem_slot_count += alu_group->inst_count;
	assert(IN_RANGE(alu_group->inst_count, 1, 5));
	compute_unit->alu_engine.vliw_slots[alu_group->inst_count - 1]++;

	/* If instruction accesses local memory, record addresses. */
	if (uop->local_mem_read || uop->local_mem_write)
	{
		FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = gpu->ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			work_item_uop->local_mem_access_count = work_item->local_mem_access_count;
			for (i = 0; i < work_item->local_mem_access_count; i++)
			{
				work_item_uop->local_mem_access_kind[i] = work_item->local_mem_access_type[i];
				work_item_uop->local_mem_access_addr[i] = work_item->local_mem_access_addr[i];
				work_item_uop->local_mem_access_size[i] = work_item->local_mem_access_size[i];
			}
		}
	}

	/* Array 'producers' contains those uops in execution that produce each possible
	 * output dependence. Find the youngest producer for this uop. */
	producer = NULL;
	for (i = 0; i < uop->idep_count; i++)
	{
		idep = uop->idep[i];
		if (!compute_unit->alu_engine.producers[idep])
			continue;
		if (!producer || producer->id < compute_unit->alu_engine.producers[idep]->id)
			producer = compute_unit->alu_engine.producers[idep];
	}

	/* If there was a producer, enqueue uop in its dependence list. Otherwise, uop is ready. */
	if (producer)
	{
		DOUBLE_LINKED_LIST_INSERT_TAIL(producer, dep, uop);
	}
	else
	{
		uop->ready = 1;
	}

	/* Record output dependences of current instruction in 'producers' array. */
	for (i = 0; i < uop->odep_count; i++)
	{
		odep = uop->odep[i];
		assert(IN_RANGE(odep, 1, GPU_UOP_DEP_COUNT - 1));
		compute_unit->alu_engine.producers[odep] = uop;
	}

	/* Access instruction cache. Record the time when the instruction will have been fetched,
	 * as per the latency of the instruction memory. */
	uop->inst_mem_ready = gpu->cycle + gpu_alu_engine_inst_mem_latency;

	/* Enqueue instruction into fetch queue */
	linked_list_out(fetch_queue);
	linked_list_insert(fetch_queue, uop);
	compute_unit->alu_engine.fetch_queue_length += uop->length;

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category))
	{
		char str[MAX_STRING_SIZE];

		gpu_pipeline_debug("alu a=\"fetch\" "
			"cu=%d "
			"wg=%d "
			"wf=%d "
			"uop=%lld ",
			compute_unit->id,
			uop->work_group->id,
			wavefront->id,
			uop->id_in_compute_unit);
		amd_alu_group_dump_debug(&wavefront->alu_group, -1, -1,
			debug_file(gpu_pipeline_debug_category));
		gpu_pipeline_debug(" idep=");
		gpu_uop_dump_dep_list(str, MAX_STRING_SIZE, uop->idep, uop->idep_count);
		gpu_pipeline_debug("%s odep=", str);
		gpu_uop_dump_dep_list(str, MAX_STRING_SIZE, uop->odep, uop->odep_count);
		gpu_pipeline_debug("%s", str);
		if (producer)
			gpu_pipeline_debug(" prod=%lld", producer->id);
		gpu_pipeline_debug("\n");
	}
}


void gpu_alu_engine_decode(struct gpu_compute_unit_t *compute_unit)
{
	struct linked_list_t *fetch_queue = compute_unit->alu_engine.fetch_queue;
	struct gpu_uop_t *uop;

	/* Get instruction at the head of the fetch queue */
	linked_list_head(fetch_queue);
	uop = linked_list_get(fetch_queue);

	/* If there was no uop in the queue, done */
	if (!uop)
		return;

	/* If uop is still being fetched from instruction memory, done */
	if (uop->inst_mem_ready > gpu->cycle)
		return;

	/* If instruction buffer is occupied, done */
	if (compute_unit->alu_engine.inst_buffer)
		return;

	/* Extract uop from fetch queue */
	linked_list_remove(fetch_queue);
	compute_unit->alu_engine.fetch_queue_length -= uop->length;
	assert(compute_unit->alu_engine.fetch_queue_length >= 0);

	/* Insert into instruction buffer */
	assert(!compute_unit->alu_engine.inst_buffer);
	compute_unit->alu_engine.inst_buffer = uop;

	/* Debug */
	gpu_pipeline_debug("alu a=\"decode\" "
		"cu=%d "
		"uop=%lld\n",
		compute_unit->id,
		uop->id_in_compute_unit);
}


void gpu_alu_engine_read(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_work_item_t *work_item;
	int work_item_id;

	struct gpu_uop_t *uop;
	struct gpu_work_item_uop_t *work_item_uop;
	int i;

	/* Get instruction in the instruction buffer.
	 * If no instruction, or instruction at the head is not ready, done */
	uop = compute_unit->alu_engine.inst_buffer;
	if (!uop || !uop->ready)
		return;
	
	/* If there is no space in the execution buffer, done */
	if (compute_unit->alu_engine.exec_buffer)
		return;
	
	/* If instruction reads from local memory, do it here. */
	if (uop->local_mem_read)
	{
		FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
		{
			work_item = gpu->ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			for (i = 0; i < work_item_uop->local_mem_access_count; i++)
			{
				if (work_item_uop->local_mem_access_kind[i] != 1)  /* read access */
					continue;
				mod_access(compute_unit->local_memory, mod_entry_gpu,
					mod_access_read, work_item_uop->local_mem_access_addr[i],
					&uop->local_mem_witness, NULL, NULL);
				uop->local_mem_witness--;
			}
		}
	}

	/* Debug */
	gpu_pipeline_debug("alu a=\"read\" "
		"cu=%d "
		"uop=%lld\n",
		compute_unit->id,
		uop->id_in_compute_unit);

	/* Move uop from instruction buffer into execution buffer */
	compute_unit->alu_engine.inst_buffer = NULL;
	compute_unit->alu_engine.exec_buffer = uop;
}


void gpu_alu_engine_execute(struct gpu_compute_unit_t *compute_unit)
{
	struct mod_t *local_memory = compute_unit->local_memory;
	struct gpu_uop_t *uop;

	/* Get uop from execution buffer.
	 * If no instruction, or instruction at the head is not ready, done */
	uop = compute_unit->alu_engine.exec_buffer;
	if (!uop || uop->local_mem_witness)
		return;
	
	/* If instruction writes to local memory, check that module is available. */
	if (uop->local_mem_write && !mod_can_access(local_memory, 0))
		return;
	
	/* One more SubWF launched for execution for uop.
	 * Enqueue one writeback event for each SubWF.
	 * Since processing elements in stream cores are pipelined, they
	 * accept a new instruction every cycle, so no contention. */
	assert(uop->exec_subwavefront_count < uop->subwavefront_count);
	uop->exec_subwavefront_count++;
	heap_insert(compute_unit->alu_engine.event_queue,
		gpu->cycle + gpu_alu_engine_pe_latency,
		uop);
	
	/* Debug */
	gpu_pipeline_debug("alu a=\"exec\" "
		"cu=%d "
		"uop=%lld "
		"subwf=%d\n",
		compute_unit->id,
		uop->id_in_compute_unit,
		uop->exec_subwavefront_count - 1);

	/* If this is the last subwavefront, remove uop from execution buffer */
	if (uop->exec_subwavefront_count == uop->subwavefront_count)
		compute_unit->alu_engine.exec_buffer = NULL;
}


void gpu_alu_engine_write(struct gpu_compute_unit_t *compute_unit)
{
	struct linked_list_t *finished_queue = compute_unit->alu_engine.finished_queue;

	struct gpu_wavefront_t *wavefront;
	struct gpu_work_item_t *work_item;
	int work_item_id;

	struct gpu_uop_t *cf_uop, *uop, *consumer;
	struct gpu_work_item_uop_t *work_item_uop;
	uint64_t cycle;

	int odep;
	int i;

	for (;;)
	{
		/* Extract a new event for this cycle */
		cycle = heap_peek(compute_unit->alu_engine.event_queue, (void **) &uop);
		if (!uop || cycle > gpu->cycle)
			break;
		assert(cycle == gpu->cycle);
		wavefront = uop->wavefront;
		heap_extract(compute_unit->alu_engine.event_queue, NULL);

		/* If instruction writes to local memory, do it here. */
		if (uop->local_mem_write)
		{
			FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
			{
				work_item = gpu->ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				for (i = 0; i < work_item_uop->local_mem_access_count; i++)
				{
					if (work_item_uop->local_mem_access_kind[i] != mod_access_write)
						continue;
					mod_access(compute_unit->local_memory, mod_entry_gpu,
						mod_access_write, work_item_uop->local_mem_access_addr[i],
						NULL, NULL, NULL);
				}
			}
		}

		/* One more SubWF writes */
		assert(uop->write_subwavefront_count < uop->subwavefront_count);
		uop->write_subwavefront_count++;
		gpu_pipeline_debug("alu a=\"write\" "
			"cu=%d "
			"uop=%lld "
			"subwf=%d\n",
			compute_unit->id,
			uop->id_in_compute_unit,
			uop->write_subwavefront_count - 1);

		/* If this is the first SubWF to write, wake up dependent instructions. */
		if (uop->write_subwavefront_count == 1)
		{
			/* Wake up consumers */
			while (uop->dep_list_head)
			{
				consumer = uop->dep_list_head;
				consumer->ready = 1;
				DOUBLE_LINKED_LIST_REMOVE(uop, dep, consumer);
			}

			/* Delete producer from 'producers' array */
			for (i = 0; i < uop->odep_count; i++)
			{
				odep = uop->odep[i];
				if (compute_unit->alu_engine.producers[odep] == uop)
					compute_unit->alu_engine.producers[odep] = NULL;
			}
		}

		/* If this is the last SubWF to write, free uop */
		if (uop->write_subwavefront_count == uop->subwavefront_count)
		{
			/* Debug info for active mask stack */
			if (debug_status(gpu_stack_debug_category))
				gpu_uop_debug_active_mask(uop);

			/* One less uop in flight */
			assert(wavefront->alu_engine_in_flight > 0);
			wavefront->alu_engine_in_flight--;

			/* Last instruction in clause.
			 * Since uops might get here out of order, the condition to check if
			 * instruction is the last cannot be 'uop->last'. */
			if (wavefront->clause_kind != GPU_CLAUSE_ALU && !wavefront->alu_engine_in_flight)
			{
				/* Extract CF uop from finished queue. Since instruction execution may
				 * vary in latency, finished CF uop may not be the one at the head of the queue. */
				cf_uop = uop->cf_uop;
				linked_list_find(finished_queue, cf_uop);
				assert(!finished_queue->error_code);
				linked_list_remove(finished_queue);

				/* Enqueue CF uop into complete queue in CF Engine */
				linked_list_add(compute_unit->cf_engine.complete_queue, cf_uop);
			}

			/* Free uop */
			gpu_uop_free(uop);
		}
	}
}


void gpu_alu_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	/* If no wavefront to run, avoid entering loop */
	if (!linked_list_count(compute_unit->alu_engine.pending_queue) &&
		!linked_list_count(compute_unit->alu_engine.finished_queue))
		return;

	/* ALU Engine stages */
	gpu_alu_engine_write(compute_unit);
	gpu_alu_engine_execute(compute_unit);
	gpu_alu_engine_read(compute_unit);
	gpu_alu_engine_decode(compute_unit);
	gpu_alu_engine_fetch(compute_unit);

	/* Stats */
	compute_unit->alu_engine.cycle++;
}

