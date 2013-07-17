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
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "cf-engine.h"
#include "compute-unit.h"
#include "gpu.h"
#include "instruction-interval-report.h"
#include "cycle-interval-report.h"
#include "sched.h"


/* Configuration parameters */
int evg_gpu_cf_engine_inst_mem_latency = 2;  /* Instruction memory latency */


static void evg_cf_engine_fetch(struct evg_compute_unit_t *compute_unit)
{
	struct evg_ndrange_t *ndrange = evg_gpu->ndrange;
	struct evg_wavefront_t *wavefront;

	char str[MAX_LONG_STRING_SIZE];
	char str_trimmed[MAX_LONG_STRING_SIZE];

	struct evg_inst_t *inst;

	struct evg_uop_t *uop;
	struct evg_work_item_uop_t *work_item_uop;
	struct evg_work_item_t *work_item;
	int work_item_id;

	/* Schedule wavefront */
	wavefront = evg_schedule(compute_unit);
	if (!wavefront)
		return;

	/* Emulate CF instruction */
	evg_wavefront_execute(wavefront);
	inst = &wavefront->cf_inst;


	/* Create uop */
	uop = evg_uop_create();
	uop->wavefront = wavefront;
	uop->work_group = wavefront->work_group;
	uop->compute_unit = compute_unit;
	uop->id_in_compute_unit = compute_unit->gpu_uop_id_counter++;
	uop->alu_clause_trigger = wavefront->clause_kind == EVG_CLAUSE_ALU;
	uop->tex_clause_trigger = wavefront->clause_kind == EVG_CLAUSE_TEX;
	uop->no_clause_trigger = wavefront->clause_kind == EVG_CLAUSE_CF;
	uop->last = DOUBLE_LINKED_LIST_MEMBER(wavefront->work_group, finished, wavefront);
	uop->wavefront_last = uop->last && uop->no_clause_trigger;
	uop->global_mem_read = wavefront->global_mem_read;
	uop->global_mem_write = wavefront->global_mem_write;
	uop->active_mask_update = wavefront->active_mask_update;
	uop->active_mask_push = wavefront->active_mask_push;
	uop->active_mask_pop = wavefront->active_mask_pop;
	uop->active_mask_stack_top = wavefront->stack_top;
	uop->vliw_slots = 1;

	/* If debugging active mask, store active state for work-items */
	if (debug_status(evg_stack_debug_category))
		evg_uop_save_active_mask(uop);

	/* If instruction is a global memory write, record addresses */
	if (uop->global_mem_write)
	{
		assert((inst->info->flags & EVG_INST_FLAG_MEM_WRITE));
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			work_item_uop->global_mem_access_addr = work_item->global_mem_access_addr;
			work_item_uop->global_mem_access_size = work_item->global_mem_access_size;
		}
	}

	/* Access instruction cache. Record the time when the instruction will have been fetched,
	 * as per the latency of the instruction memory. */
	uop->inst_mem_ready = asTiming(evg_gpu)->cycle + evg_gpu_cf_engine_inst_mem_latency;

	/* Insert uop to fetch buffer */
	assert(!compute_unit->cf_engine.fetch_buffer[wavefront->id_in_compute_unit]);
	compute_unit->cf_engine.fetch_buffer[wavefront->id_in_compute_unit] = uop;

	/* Stats */
	compute_unit->inst_count++;
	compute_unit->cf_engine.inst_count++;

	if(evg_spatial_report_active)
		evg_cf_report_new_inst(compute_unit);


	if (uop->global_mem_write)
		compute_unit->cf_engine.global_mem_write_count++;
	if (uop->alu_clause_trigger)
		compute_unit->cf_engine.alu_clause_trigger_count++;
	if (uop->tex_clause_trigger)
		compute_unit->cf_engine.tex_clause_trigger_count++;
	if (evg_periodic_report_active)
		evg_periodic_report_new_inst(uop);

	/* Trace */
	if (evg_tracing())
	{
		evg_inst_dump_buf(inst, -1, 0, str, sizeof str);
		str_single_spaces(str_trimmed, sizeof str_trimmed, str);
		evg_trace("evg.new_inst id=%lld cu=%d wg=%d wf=%d cat=\"cf\" stg=\"cf-fe\" asm=\"%s\"\n",
			uop->id_in_compute_unit, compute_unit->id, uop->work_group->id, wavefront->id, str_trimmed);
	}
}


static void evg_cf_engine_decode(struct evg_compute_unit_t *compute_unit)
{
	struct evg_uop_t *uop;
	int index;

	/* Search entry in fetch buffer to decode */
	index = compute_unit->cf_engine.decode_index;
	for (;;)
	{
		/* There must be a fetch buffer slot ready (instruction fetch finished),
		 * and the corresponding slot in the instruction buffer must be free. */
		uop = compute_unit->cf_engine.fetch_buffer[index];
		if (uop && uop->inst_mem_ready && !compute_unit->cf_engine.inst_buffer[index])
			break;

		/* Current candidate is not valid - go to next.
		 * If we went through the whole fetch buffer, no decode. */
		index = (index + 1) % evg_gpu->wavefronts_per_compute_unit;
		if (index == compute_unit->cf_engine.decode_index)
			return;
	}

	/* Decode instruction */
	compute_unit->cf_engine.fetch_buffer[index] = NULL;
	compute_unit->cf_engine.inst_buffer[index] = uop;

	/* Set next decode candidate */
	compute_unit->cf_engine.decode_index = (index + 1)
		% evg_gpu->wavefronts_per_compute_unit;

	/* Trace */
	evg_trace("evg.inst id=%lld cu=%d stg=\"cf-de\"\n",
		uop->id_in_compute_unit, compute_unit->id);
}


static void evg_cf_engine_execute(struct evg_compute_unit_t *compute_unit)
{
	struct evg_wavefront_t *wavefront;
	struct evg_ndrange_t *ndrange;

	struct evg_uop_t *uop;
	int index;

	struct evg_work_item_uop_t *work_item_uop;
	struct evg_work_item_t *work_item;
	int work_item_id;

	/* Complete queue must be empty. If it is not, it means that a write access is trying to allocate
	 * a cache port, so compute unit needs to stall. */
	if (linked_list_count(compute_unit->cf_engine.complete_queue))
		return;

	/* Search entry in instruction buffer to execute. */
	index = compute_unit->cf_engine.execute_index;
	for (;;)
	{
		/* There must be an instruction buffer slot ready. */
		uop = compute_unit->cf_engine.inst_buffer[index];
		if (uop)
			break;

		/* Current candidate is not valid - go to next.
		 * If we went through the whole instruction buffer, no execute. */
		index = (index + 1) % evg_gpu->wavefronts_per_compute_unit;
		if (index == compute_unit->cf_engine.execute_index)
			return;
	}

	/* Remove entry from instruction buffer */
	assert(uop);
	wavefront = uop->wavefront;
	ndrange = wavefront->ndrange;
	compute_unit->cf_engine.inst_buffer[index] = NULL;

	/* Execute instruction */
	if (uop->alu_clause_trigger)
	{
		/* Trigger secondary ALU clause */
		linked_list_add(compute_unit->alu_engine.pending_queue, uop);
		compute_unit->alu_engine.wavefront_count++;
	}
	else if (uop->tex_clause_trigger)
	{
		/* Trigger secondary TEX clause */
		linked_list_add(compute_unit->tex_engine.pending_queue, uop);
		compute_unit->tex_engine.wavefront_count++;
	}
	else
	{
		/* Execute instruction in CF Engine - completed next cycle */
		linked_list_out(compute_unit->cf_engine.complete_queue);
		linked_list_insert(compute_unit->cf_engine.complete_queue, uop);

		/* Global memory write - execute asynchronously */
		if (uop->global_mem_write)
		{
			EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
			{
				work_item = ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				mod_access(compute_unit->global_memory, mod_access_nc_store, 
					work_item_uop->global_mem_access_addr,
					&uop->global_mem_witness, NULL, NULL, NULL);
				uop->global_mem_witness--;
			}
		}
	}

	/* Set next execute candidate */
	compute_unit->cf_engine.execute_index = (index + 1)
		% evg_gpu->wavefronts_per_compute_unit;
	
	/* Trace */
	evg_trace("evg.inst id=%lld cu=%d stg=\"cf-ex\"\n",
		uop->id_in_compute_unit, compute_unit->id);
}


static void evg_cf_engine_complete(struct evg_compute_unit_t *compute_unit)
{
	struct linked_list_t *complete_queue = compute_unit->cf_engine.complete_queue;
	struct linked_list_t *wavefront_pool = compute_unit->wavefront_pool;
	struct evg_work_group_t *work_group;
	struct evg_uop_t *uop;

	/* Process all uops in the complete queue */
	while (linked_list_count(complete_queue))
	{
		/* Get instruction at the head of complete queue */
		linked_list_head(complete_queue);
		uop = linked_list_get(complete_queue);
		work_group = uop->work_group;

		/* If it is a memory write that still couldn't allocate a write port, stall */
		if (uop->global_mem_write && uop->global_mem_witness)
			break;

		/* Extract from complete queue */
		linked_list_remove(complete_queue);

		/* Instruction finishes a wavefront */
		if (uop->last)
		{
			work_group->compute_unit_finished_count++;
		}
		else
		{
			/* Insert wavefront into wavefront pool */
			linked_list_insert(wavefront_pool, uop->wavefront);
			linked_list_next_circular(wavefront_pool);
		}

		/* Debug */
		if (debug_status(evg_stack_debug_category))
			evg_uop_debug_active_mask(uop);

		/* Trace */
		evg_trace("evg.inst id=%lld cu=%d stg=\"cf-co\"\n",
			uop->id_in_compute_unit, compute_unit->id);

		/* Free uop. If a trace is being generated, defer the instruction
		 * release to the next cycle to allow for the last stage to be shown. */
		if (evg_tracing())
			evg_gpu_uop_trash_add(uop);
		else
			evg_uop_free(uop);

		/* Wavefront finishes a work-group */
		assert(work_group->compute_unit_finished_count <= work_group->wavefront_count);
		if (work_group->compute_unit_finished_count == work_group->wavefront_count)
			evg_compute_unit_unmap_work_group(compute_unit, work_group);

		/* Statistics */
		evg_gpu->last_complete_cycle = asTiming(evg_gpu)->cycle;
	}
}


void evg_cf_engine_run(struct evg_compute_unit_t *compute_unit)
{
	/* Call CF Engine stages */
	evg_cf_engine_complete(compute_unit);
	evg_cf_engine_execute(compute_unit);
	evg_cf_engine_decode(compute_unit);
	evg_cf_engine_fetch(compute_unit);
}

