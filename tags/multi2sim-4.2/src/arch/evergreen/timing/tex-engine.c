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
#include <lib/util/linked-list.h>
#include <lib/util/string.h>

#include "compute-unit.h"
#include "gpu.h"
#include "instruction-interval-report.h"
#include "cycle-interval-report.h"

#include "tex-engine.h"


/* Configuration parameters */
int evg_gpu_tex_engine_inst_mem_latency = 2;  /* Instruction memory latency */
int evg_gpu_tex_engine_fetch_queue_size = 32;  /* Number of bytes */
int evg_gpu_tex_engine_load_queue_size = 8;  /* Maximum number of in-flight global memory reads */


static void evg_tex_engine_fetch(struct evg_compute_unit_t *compute_unit)
{
	struct linked_list_t *pending_queue = compute_unit->tex_engine.pending_queue;
	struct linked_list_t *finished_queue = compute_unit->tex_engine.finished_queue;

	struct evg_wavefront_t *wavefront;
	struct evg_uop_t *cf_uop, *uop;
	struct evg_work_item_uop_t *work_item_uop;
	struct evg_inst_t *inst;
	int inst_num;

	struct evg_work_item_t *work_item;
	int work_item_id;

	char str[MAX_LONG_STRING_SIZE];
	char str_trimmed[MAX_LONG_STRING_SIZE];

	/* Get wavefront to fetch from */
	linked_list_head(pending_queue);
	cf_uop = linked_list_get(pending_queue);
	if (!cf_uop)
		return;
	wavefront = cf_uop->wavefront;
	assert(wavefront->clause_kind == EVG_CLAUSE_TEX);


	/* If fetch queue is full, cannot fetch until space is made */
	if (compute_unit->tex_engine.fetch_queue_length >= evg_gpu_tex_engine_fetch_queue_size)
		return;
	
	/* Emulate instruction and create uop */
	inst_num = (wavefront->clause_buf - wavefront->clause_buf_start) / 16;
	evg_wavefront_execute(wavefront);
	inst = &wavefront->tex_inst;
	uop = evg_uop_create();
	uop->wavefront = wavefront;
	uop->work_group = wavefront->work_group;
	uop->cf_uop = cf_uop;
	uop->compute_unit = compute_unit;
	uop->id_in_compute_unit = compute_unit->gpu_uop_id_counter++;
	uop->last = wavefront->clause_kind != EVG_CLAUSE_TEX;
	uop->global_mem_read = wavefront->global_mem_read;
	uop->global_mem_write = wavefront->global_mem_write;
	uop->vliw_slots = 1;

	/* If TEX clause finished, extract CF uop from 'pending_queue' and
	 * insert it into 'finished_queue'. */
	if (uop->last)
	{
		linked_list_remove(pending_queue);
		linked_list_add(finished_queue, cf_uop);
	}

	/* If instruction is a global memory read (should be), record addresses */
	if (uop->global_mem_read)
	{
		assert((inst->info->flags & EVG_INST_FLAG_MEM_READ));
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = evg_gpu->ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			work_item_uop->global_mem_access_addr = work_item->global_mem_access_addr;
			work_item_uop->global_mem_access_size = work_item->global_mem_access_size;
		}
	}

	/* Access instruction cache. Record the time when the instruction will have been fetched,
	 * as per the latency of the instruction memory. */
	uop->inst_mem_ready = asTiming(evg_gpu)->cycle + evg_gpu_tex_engine_inst_mem_latency;

	/* Enqueue uop into fetch queue */
	linked_list_out(compute_unit->tex_engine.fetch_queue);
	linked_list_insert(compute_unit->tex_engine.fetch_queue, uop);
	compute_unit->tex_engine.fetch_queue_length += uop->length;

	/* Stats */
	compute_unit->inst_count++;
	compute_unit->tex_engine.inst_count++;

	if(evg_spatial_report_active)
		evg_tex_report_new_inst(compute_unit);

	if (evg_periodic_report_active)
		evg_periodic_report_new_inst(uop);

	/* Trace */
	if (evg_tracing())
	{
		evg_inst_dump_buf(inst, inst_num, 0, str, sizeof str);
		str_single_spaces(str_trimmed, sizeof str_trimmed, str);
		evg_trace("evg.new_inst id=%lld cu=%d wg=%d wf=%d cat=\"tex\" stg=\"tex-fe\" asm=\"%s\"\n",
			uop->id_in_compute_unit, compute_unit->id, uop->work_group->id,
			wavefront->id, str_trimmed);
	}
}


static void evg_tex_engine_decode(struct evg_compute_unit_t *compute_unit)
{
	struct linked_list_t *fetch_queue = compute_unit->tex_engine.fetch_queue;
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
	if (compute_unit->tex_engine.inst_buffer)
		return;

	/* Extract uop from fetch queue */
	linked_list_remove(fetch_queue);
	compute_unit->tex_engine.fetch_queue_length -= uop->length;
	assert(compute_unit->tex_engine.fetch_queue_length >= 0);

	/* Insert into instruction buffer */
	assert(!compute_unit->tex_engine.inst_buffer);
	compute_unit->tex_engine.inst_buffer = uop;

	/* Trace */
	evg_trace("evg.inst id=%lld cu=%d stg=\"tex-de\"\n",
		uop->id_in_compute_unit, compute_unit->id);
}


static void evg_tex_engine_read(struct evg_compute_unit_t *compute_unit)
{
	struct evg_work_item_t *work_item;
	int work_item_id;

	struct evg_uop_t *uop;
	struct evg_work_item_uop_t *work_item_uop;

	/* If there is no instruction in instruction buffer, done */
	uop = compute_unit->tex_engine.inst_buffer;
	if (!uop)
		return;

	/* If there is no space in the load queue, done. */
	if (linked_list_count(compute_unit->tex_engine.load_queue) >= evg_gpu_tex_engine_load_queue_size)
		return;
	
	/* Extract uop from instruction buffer and insert into load queue. */
	compute_unit->tex_engine.inst_buffer = NULL;
	linked_list_out(compute_unit->tex_engine.load_queue);
	linked_list_insert(compute_unit->tex_engine.load_queue, uop);

	/* Global memory read  */
	if (uop->global_mem_read)
	{
		assert(!uop->global_mem_witness);
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
		{
			work_item = evg_gpu->ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			mod_access(compute_unit->global_memory, mod_access_load, 
				work_item_uop->global_mem_access_addr,
				&uop->global_mem_witness, NULL, NULL, NULL);
			uop->global_mem_witness--;
		}
		uop->num_global_mem_read = uop->global_mem_witness;
		if(evg_spatial_report_active)
			evg_tex_report_global_mem_inflight(compute_unit,uop->num_global_mem_read);

	}


	/* Trace */
	evg_trace("evg.inst id=%lld cu=%d stg=\"tex-rd\"\n",
		uop->id_in_compute_unit, compute_unit->id);
}


static void evg_tex_engine_write(struct evg_compute_unit_t *compute_unit)
{
	struct linked_list_t *finished_queue = compute_unit->tex_engine.finished_queue;
	struct evg_uop_t *cf_uop, *uop;

	/* Get instruction at the head of the load queue. */
	linked_list_head(compute_unit->tex_engine.load_queue);
	uop = linked_list_get(compute_unit->tex_engine.load_queue);
	if (!uop)
		return;


	/* If the memory read did not complete, done. */
	if (uop->global_mem_witness)
		return;

	/* Subtracted in this function which denotes accesses finished*/
	if(evg_spatial_report_active)
		evg_tex_report_global_mem_finish(compute_unit,uop->num_global_mem_read);

	/* Extract from load queue. */
	linked_list_remove(compute_unit->tex_engine.load_queue);

	/* Trace */
	evg_trace("evg.inst id=%lld cu=%d stg=\"tex-wr\"\n",
		uop->id_in_compute_unit, compute_unit->id);

	/* Last uop in clause */
	if (uop->last)
	{
		/* Extract CF uop from 'finished_queue' */
		linked_list_head(finished_queue);
		cf_uop = linked_list_get(finished_queue);
		assert(cf_uop == uop->cf_uop);
		linked_list_remove(finished_queue);

		/* Enqueue CF uop into complete queue in CF Engine */
		linked_list_add(compute_unit->cf_engine.complete_queue, cf_uop);
	}

	/* Free uop */
	if (evg_tracing())
		evg_gpu_uop_trash_add(uop);
	else
		evg_uop_free(uop);
	
	/* Statistics */
	evg_gpu->last_complete_cycle = asTiming(evg_gpu)->cycle;
}


void evg_tex_engine_run(struct evg_compute_unit_t *compute_unit)
{
	/* If no wavefront to run, avoid entering loop */
	if (!linked_list_count(compute_unit->tex_engine.pending_queue) &&
		!linked_list_count(compute_unit->tex_engine.finished_queue))
		return;

	/* TEX Engine stages */
	evg_tex_engine_write(compute_unit);
	evg_tex_engine_read(compute_unit);
	evg_tex_engine_decode(compute_unit);
	evg_tex_engine_fetch(compute_unit);

	/* Stats */
	compute_unit->tex_engine.cycle++;

	/* Interval Reset Actions */
	//evg_tex_engine_interval_update(compute_unit);
}

