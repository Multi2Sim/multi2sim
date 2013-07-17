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
#include <lib/mhandle/mhandle.h>
#include <lib/util/heap.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "alu-engine.h"
#include "cf-engine.h"
#include "compute-unit.h"
#include "gpu.h"
#include "tex-engine.h"
#include "instruction-interval-report.h"
#include "cycle-interval-report.h"



/*
 * Compute Unit
 */

struct evg_compute_unit_t *evg_compute_unit_create()
{
	struct evg_compute_unit_t *compute_unit;
	char buf[MAX_STRING_SIZE];

	/* Initialize */
	compute_unit = xcalloc(1, sizeof(struct evg_compute_unit_t));
	compute_unit->wavefront_pool = linked_list_create();

	/* Local memory */
	snprintf(buf, sizeof buf, "LocalMemory[%d]", compute_unit->id);
	compute_unit->local_memory = mod_create(buf, mod_kind_local_memory,
		evg_gpu_local_mem_num_ports, evg_gpu_local_mem_block_size, evg_gpu_local_mem_latency);

	/* Initialize CF Engine */
	compute_unit->cf_engine.complete_queue = linked_list_create();
	compute_unit->cf_engine.fetch_buffer = xcalloc(evg_gpu_max_wavefronts_per_compute_unit, sizeof(void *));
	compute_unit->cf_engine.inst_buffer = xcalloc(evg_gpu_max_wavefronts_per_compute_unit, sizeof(void *));

	/* Initialize ALU Engine */
	compute_unit->alu_engine.pending_queue = linked_list_create();
	compute_unit->alu_engine.finished_queue = linked_list_create();
	compute_unit->alu_engine.fetch_queue = linked_list_create();
	compute_unit->alu_engine.event_queue = heap_create(10);

	/* Initialize TEX Engine */
	compute_unit->tex_engine.pending_queue = linked_list_create();
	compute_unit->tex_engine.finished_queue = linked_list_create();
	compute_unit->tex_engine.fetch_queue = linked_list_create();
	compute_unit->tex_engine.load_queue = linked_list_create();

	/* Return */
	compute_unit->work_groups = xcalloc(evg_gpu_max_work_groups_per_compute_unit, sizeof(void *));
	return compute_unit;
}


void evg_compute_unit_free(struct evg_compute_unit_t *compute_unit)
{
	struct heap_t *event_queue;
	struct evg_uop_t *uop;
	int i;

	/* CF Engine - free uops in fetch buffer, instruction buffer, and complete queue */
	for (i = 0; i < evg_gpu_max_wavefronts_per_compute_unit; i++)
	{
		evg_uop_free(compute_unit->cf_engine.fetch_buffer[i]);
		evg_uop_free(compute_unit->cf_engine.inst_buffer[i]);
	}
	evg_uop_list_free(compute_unit->cf_engine.complete_queue);

	/* CF Engine - free structures */
	free(compute_unit->cf_engine.fetch_buffer);
	free(compute_unit->cf_engine.inst_buffer);
	linked_list_free(compute_unit->cf_engine.complete_queue);

	/* ALU Engine - free uops in event queue (heap) */
	event_queue = compute_unit->alu_engine.event_queue;
	while (event_queue->count)
	{
		heap_extract(event_queue, (void **) &uop);
		uop->write_subwavefront_count++;
		if (uop->write_subwavefront_count == uop->subwavefront_count)
			evg_uop_free(uop);
	}

	/* ALU Engine - free uops in fetch queue, instruction buffer, execution buffer,
	 * and event queue. Also free CF instruction currently running. */
	evg_uop_list_free(compute_unit->alu_engine.pending_queue);
	evg_uop_list_free(compute_unit->alu_engine.finished_queue);
	evg_uop_list_free(compute_unit->alu_engine.fetch_queue);
	evg_uop_free(compute_unit->alu_engine.inst_buffer);
	evg_uop_free(compute_unit->alu_engine.exec_buffer);

	/* ALU Engine - structures */
	linked_list_free(compute_unit->alu_engine.pending_queue);
	linked_list_free(compute_unit->alu_engine.finished_queue);
	linked_list_free(compute_unit->alu_engine.fetch_queue);
	heap_free(compute_unit->alu_engine.event_queue);

	/* TEX Engine - free uop in fetch queue, instruction buffer, write buffer. */
	evg_uop_list_free(compute_unit->tex_engine.pending_queue);
	evg_uop_list_free(compute_unit->tex_engine.finished_queue);
	evg_uop_list_free(compute_unit->tex_engine.fetch_queue);
	evg_uop_free(compute_unit->tex_engine.inst_buffer);
	evg_uop_list_free(compute_unit->tex_engine.load_queue);

	/* TEX Engine - structures */
	linked_list_free(compute_unit->tex_engine.pending_queue);
	linked_list_free(compute_unit->tex_engine.finished_queue);
	linked_list_free(compute_unit->tex_engine.fetch_queue);
	linked_list_free(compute_unit->tex_engine.load_queue);

	/* Compute unit */
	linked_list_free(compute_unit->wavefront_pool);
	free(compute_unit->work_groups);  /* List of mapped work-groups */
	mod_free(compute_unit->local_memory);
	free(compute_unit);
}


void evg_compute_unit_map_work_group(struct evg_compute_unit_t *compute_unit, struct evg_work_group_t *work_group)
{
	struct evg_ndrange_t *ndrange = work_group->ndrange;
	struct evg_wavefront_t *wavefront;
	int wavefront_id;

	/* Map work-group */
	assert(compute_unit->work_group_count < evg_gpu->work_groups_per_compute_unit);
	assert(!work_group->id_in_compute_unit);
	while (work_group->id_in_compute_unit < evg_gpu->work_groups_per_compute_unit
		&& compute_unit->work_groups[work_group->id_in_compute_unit])
		work_group->id_in_compute_unit++;
	assert(work_group->id_in_compute_unit < evg_gpu->work_groups_per_compute_unit);
	compute_unit->work_groups[work_group->id_in_compute_unit] = work_group;
	compute_unit->work_group_count++;

	/* If compute unit reached its maximum load, remove it from 'ready' list.
	 * Otherwise, move it to the end of the 'ready' list. */
	assert(DOUBLE_LINKED_LIST_MEMBER(evg_gpu, ready, compute_unit));
	DOUBLE_LINKED_LIST_REMOVE(evg_gpu, ready, compute_unit);
	if (compute_unit->work_group_count < evg_gpu->work_groups_per_compute_unit)
		DOUBLE_LINKED_LIST_INSERT_TAIL(evg_gpu, ready, compute_unit);
	
	/* If this is the first scheduled work-group, insert to 'busy' list. */
	if (!DOUBLE_LINKED_LIST_MEMBER(evg_gpu, busy, compute_unit))
		DOUBLE_LINKED_LIST_INSERT_TAIL(evg_gpu, busy, compute_unit);

	/* Assign wavefronts identifiers in compute unit */
	EVG_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = ndrange->wavefronts[wavefront_id];
		wavefront->id_in_compute_unit = work_group->id_in_compute_unit *
			ndrange->wavefronts_per_work_group + wavefront->id_in_work_group;
	}

	/* Change work-group status to running */
	evg_work_group_clear_status(work_group, evg_work_group_pending);
	evg_work_group_set_status(work_group, evg_work_group_running);

	/* Insert all wavefronts into the CF Engine's wavefront pool */
	EVG_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = ndrange->wavefronts[wavefront_id];
		linked_list_add(compute_unit->wavefront_pool, wavefront);
	}

	/* Initialization of periodic report for wavefronts */
	if (evg_periodic_report_active)
	{
		EVG_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
		{
			wavefront = ndrange->wavefronts[wavefront_id];
			evg_periodic_report_wavefront_init(wavefront);
		}
	}

	/* Trace */
	evg_trace("evg.map_wg cu=%d wg=%d wi_first=%d wi_count=%d wf_first=%d wf_count=%d\n",
		compute_unit->id, work_group->id, work_group->work_item_id_first,
		work_group->work_item_count, work_group->wavefront_id_first,
		work_group->wavefront_count);

	/* Stats */
	compute_unit->mapped_work_groups++;
	evg_gpu->last_complete_cycle = asTiming(evg_gpu)->cycle;
}


void evg_compute_unit_unmap_work_group(struct evg_compute_unit_t *compute_unit, struct evg_work_group_t *work_group)
{
	struct evg_ndrange_t *ndrange = work_group->ndrange;
	struct evg_wavefront_t *wavefront;

	int wavefront_id;

	/* Reset mapped work-group */
	assert(compute_unit->work_group_count > 0);
	assert(compute_unit->work_groups[work_group->id_in_compute_unit]);
	compute_unit->work_groups[work_group->id_in_compute_unit] = NULL;
	compute_unit->work_group_count--;

	/* If compute unit accepts work-groups again, insert into 'ready' list */
	if (!DOUBLE_LINKED_LIST_MEMBER(evg_gpu, ready, compute_unit))
		DOUBLE_LINKED_LIST_INSERT_TAIL(evg_gpu, ready, compute_unit);
	
	/* If compute unit is not busy anymore, remove it from 'busy' list */
	if (!compute_unit->work_group_count && DOUBLE_LINKED_LIST_MEMBER(evg_gpu, busy, compute_unit))
		DOUBLE_LINKED_LIST_REMOVE(evg_gpu, busy, compute_unit);

	/* Finalization of periodic events for all wavefronts in the work-group */
	if (evg_periodic_report_active)
	{
		EVG_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
		{
			wavefront = ndrange->wavefronts[wavefront_id];
			evg_periodic_report_wavefront_done(wavefront);
		}
	}

	/* Trace */
	evg_trace("evg.unmap_wg cu=%d wg=%d\n",
		compute_unit->id, work_group->id);
}


/* Advance one cycle in the compute unit by running every stage from last to first */
void evg_compute_unit_run(struct evg_compute_unit_t *compute_unit)
{
	/* Run Engines */
	evg_alu_engine_run(compute_unit);
	evg_tex_engine_run(compute_unit);
	evg_cf_engine_run(compute_unit);

	/* Stats */
	compute_unit->cycle++;
	if(evg_spatial_report_active)
		evg_cu_interval_update(compute_unit);
}

