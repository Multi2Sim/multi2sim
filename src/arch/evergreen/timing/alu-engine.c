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


#include <arch/evergreen/emu/ndrange.h>
#include <arch/evergreen/emu/wavefront.h>
#include <arch/evergreen/emu/work-group.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/heap.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "alu-engine.h"
#include "compute-unit.h"
#include "gpu.h"
#include "instruction-interval-report.h"
#include "cycle-interval-report.h"


int evg_gpu_alu_engine_inst_mem_latency = 2;  /* Latency of instruction memory */
int evg_gpu_alu_engine_fetch_queue_size = 64;  /* Number of bytes */
int evg_gpu_alu_engine_pe_latency = 4;  /* Processing element latency */


static void evg_alu_engine_fetch(struct evg_compute_unit_t *compute_unit)
{
	struct linked_list_t *pending_queue = compute_unit->alu_engine.pending_queue;
	struct linked_list_t *finished_queue = compute_unit->alu_engine.finished_queue;

	struct linked_list_t *fetch_queue = compute_unit->alu_engine.fetch_queue;
	struct evg_alu_group_t *alu_group;
	struct evg_uop_t *cf_uop, *uop, *producer;
	struct evg_work_item_uop_t *work_item_uop;
	struct evg_wavefront_t *wavefront;

	struct evg_work_item_t *work_item;
	int work_item_id;

	int idep, odep;
	int i;

	/* Get wavefront to fetch from */
	linked_list_head(pending_queue);
	cf_uop = linked_list_get(pending_queue);
	if (!cf_uop)
		return;
	wavefront = cf_uop->wavefront;
	assert(wavefront->clause_kind == EVG_CLAUSE_ALU);

	/* If fetch queue is full, cannot fetch until space is made */
	if (compute_unit->alu_engine.fetch_queue_length >= evg_gpu_alu_engine_fetch_queue_size)
		return;

	/* Emulate instruction and create uop */
	evg_wavefront_execute(wavefront);
	alu_group = &wavefront->alu_group;
	uop = evg_uop_create_from_alu_group(alu_group);
	uop->wavefront = wavefront;
	uop->work_group = wavefront->work_group;
	uop->cf_uop = cf_uop;
	uop->compute_unit = compute_unit;
	uop->id_in_compute_unit = compute_unit->gpu_uop_id_counter++;
	uop->subwavefront_count = (wavefront->work_item_count + evg_gpu_num_stream_cores - 1)
		/ evg_gpu_num_stream_cores;
	uop->last = wavefront->clause_kind != EVG_CLAUSE_ALU;
	uop->length = alu_group->inst_count * 8 + alu_group->literal_count * 4;
	uop->local_mem_read = wavefront->local_mem_read;
	uop->local_mem_write = wavefront->local_mem_write;
	uop->active_mask_update = wavefront->active_mask_update;
	uop->active_mask_push = wavefront->active_mask_push;
	uop->active_mask_pop = wavefront->active_mask_pop;
	uop->active_mask_stack_top = wavefront->stack_top;
	uop->vliw_slots = alu_group->inst_count;
	wavefront->alu_engine_in_flight++;

	/* If ALU clause finished, extract wavefront from pending wavefront queue, and
	 * insert it into finished wavefront queue. */
	if (uop->last)
	{
		linked_list_remove(pending_queue);
		linked_list_add(finished_queue, cf_uop);
	}

	/* If debugging active mask, store active state for work-items */
	if (debug_status(evg_stack_debug_category))
		evg_uop_save_active_mask(uop);

	/* Stats */
	compute_unit->inst_count++;
	compute_unit->alu_engine.inst_count++;
	compute_unit->alu_engine.inst_slot_count += alu_group->inst_count;

	if (uop->local_mem_read || uop->local_mem_write)
		compute_unit->alu_engine.local_mem_slot_count += alu_group->inst_count;
	assert(IN_RANGE(alu_group->inst_count, 1, 5));
	compute_unit->alu_engine.vliw_slots[alu_group->inst_count - 1]++;
	if (evg_periodic_report_active)
		evg_periodic_report_new_inst(uop);
	if(evg_spatial_report_active)
		evg_alu_report_new_inst(compute_unit);


	/* If instruction accesses local memory, record addresses. */
	if (uop->local_mem_read || uop->local_mem_write)
	{
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = evg_gpu->ndrange->work_items[work_item_id];
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
		uop->ready = 1;

	/* Record output dependences of current instruction in 'producers' array. */
	for (i = 0; i < uop->odep_count; i++)
	{
		odep = uop->odep[i];
		assert(IN_RANGE(odep, 1, EVG_UOP_DEP_COUNT - 1));
		compute_unit->alu_engine.producers[odep] = uop;
	}

	/* Access instruction cache. Record the time when the instruction will have been fetched,
	 * as per the latency of the instruction memory. */
	uop->inst_mem_ready = asTiming(evg_gpu)->cycle + evg_gpu_alu_engine_inst_mem_latency;

	/* Enqueue instruction into fetch queue */
	linked_list_out(fetch_queue);
	linked_list_insert(fetch_queue, uop);
	compute_unit->alu_engine.fetch_queue_length += uop->length;

	/* Trace */
	if (evg_tracing())
	{
		char str_inst[MAX_LONG_STRING_SIZE];

		evg_alu_group_dump_buf(&wavefront->alu_group, str_inst, sizeof str_inst);
		evg_trace("evg.new_inst id=%lld cu=%d wg=%d wf=%d cat=\"alu\" stg=\"alu-fe\" %s\n",
			uop->id_in_compute_unit, compute_unit->id,
			uop->work_group->id, wavefront->id, str_inst);
	}
}


static void evg_alu_engine_decode(struct evg_compute_unit_t *compute_unit)
{
	struct linked_list_t *fetch_queue = compute_unit->alu_engine.fetch_queue;
	struct evg_uop_t *uop;

	/* Get instruction at the head of the fetch queue */
	linked_list_head(fetch_queue);
	uop = linked_list_get(fetch_queue);

	/* If there was no uop in the queue, done */
	if (!uop)
		return;

	/* If uop is still being fetched from instruction memory, done */
	if (uop->inst_mem_ready > asTiming(evg_gpu)->cycle)
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

	/* Trace */
	evg_trace("evg.inst id=%lld cu=%d stg=\"alu-de\"\n",
		uop->id_in_compute_unit, compute_unit->id);
}


static void evg_alu_engine_read(struct evg_compute_unit_t *compute_unit)
{
	struct evg_work_item_t *work_item;
	int work_item_id;

	struct evg_uop_t *uop;
	struct evg_work_item_uop_t *work_item_uop;
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
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
		{
			work_item = evg_gpu->ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			for (i = 0; i < work_item_uop->local_mem_access_count; i++)
			{
				if (work_item_uop->local_mem_access_kind[i] != 1)  /* read access */
					continue;
				mod_access(compute_unit->local_memory, mod_access_load, 
					work_item_uop->local_mem_access_addr[i],
					&uop->local_mem_witness, NULL, NULL, NULL);
				uop->local_mem_witness--;
			}
		}
	}

	/* Trace */
	evg_trace("evg.inst id=%lld cu=%d stg=\"alu-rd\"\n",
		uop->id_in_compute_unit, compute_unit->id);

	/* Move uop from instruction buffer into execution buffer */
	compute_unit->alu_engine.inst_buffer = NULL;
	compute_unit->alu_engine.exec_buffer = uop;
}


static void evg_alu_engine_execute(struct evg_compute_unit_t *compute_unit)
{
	struct mod_t *local_memory = compute_unit->local_memory;
	struct evg_uop_t *uop;

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
		asTiming(evg_gpu)->cycle + evg_gpu_alu_engine_pe_latency,
		uop);
	
	/* Trace */
	if (uop->exec_subwavefront_count == 1)
		evg_trace("evg.inst id=%lld cu=%d stg=\"alu-rd\"\n",
			uop->id_in_compute_unit, compute_unit->id);

	/* If this is the last subwavefront, remove uop from execution buffer */
	if (uop->exec_subwavefront_count == uop->subwavefront_count)
		compute_unit->alu_engine.exec_buffer = NULL;
}


static void evg_alu_engine_write(struct evg_compute_unit_t *compute_unit)
{
	struct linked_list_t *finished_queue = compute_unit->alu_engine.finished_queue;

	struct evg_wavefront_t *wavefront;
	struct evg_work_item_t *work_item;
	int work_item_id;

	struct evg_uop_t *cf_uop, *uop, *consumer;
	struct evg_work_item_uop_t *work_item_uop;

	long long cycle;

	int odep;
	int i;

	for (;;)
	{
		/* Extract a new event for this cycle */
		cycle = heap_peek(compute_unit->alu_engine.event_queue, (void **) &uop);
		if (!uop || cycle > asTiming(evg_gpu)->cycle)
			break;
		assert(cycle == asTiming(evg_gpu)->cycle);
		wavefront = uop->wavefront;
		heap_extract(compute_unit->alu_engine.event_queue, NULL);

		/* If instruction writes to local memory, do it here. */
		if (uop->local_mem_write)
		{
			EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
			{
				work_item = evg_gpu->ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				for (i = 0; i < work_item_uop->local_mem_access_count; i++)
				{
					if (work_item_uop->local_mem_access_kind[i] != 
						mod_access_store)
						continue;
					mod_access(compute_unit->local_memory, mod_access_store,
						work_item_uop->local_mem_access_addr[i],
						NULL, NULL, NULL, NULL);
				}
			}
		}

		/* One more SubWF writes */
		assert(uop->write_subwavefront_count < uop->subwavefront_count);
		uop->write_subwavefront_count++;

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
			if (debug_status(evg_stack_debug_category))
				evg_uop_debug_active_mask(uop);

			/* One less uop in flight */
			assert(wavefront->alu_engine_in_flight > 0);
			wavefront->alu_engine_in_flight--;

			/* Last instruction in clause.
			 * Since uops might get here out of order, the condition to check if
			 * instruction is the last cannot be 'uop->last'. */
			if (wavefront->clause_kind != EVG_CLAUSE_ALU && !wavefront->alu_engine_in_flight)
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

			/* Trace */
			evg_trace("evg.inst id=%lld cu=%d stg=\"alu-wr\"\n",
				uop->id_in_compute_unit, compute_unit->id);

			/* Free uop */
			if (evg_tracing())
				evg_gpu_uop_trash_add(uop);
			else
				evg_uop_free(uop);

			/* Statistics */
			evg_gpu->last_complete_cycle = asTiming(evg_gpu)->cycle;
		}
	}
}


void evg_alu_engine_run(struct evg_compute_unit_t *compute_unit)
{
	/* If no wavefront to run, avoid entering loop */
	if (!linked_list_count(compute_unit->alu_engine.pending_queue) &&
		!linked_list_count(compute_unit->alu_engine.finished_queue))
		return;

	/* ALU Engine stages */
	evg_alu_engine_write(compute_unit);
	evg_alu_engine_execute(compute_unit);
	evg_alu_engine_read(compute_unit);
	evg_alu_engine_decode(compute_unit);
	evg_alu_engine_fetch(compute_unit);

	/* Stats */
	compute_unit->alu_engine.cycle++;

}

