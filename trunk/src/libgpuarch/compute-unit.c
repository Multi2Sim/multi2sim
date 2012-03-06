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




/*
 * Compute Unit
 */

struct gpu_compute_unit_t *gpu_compute_unit_create()
{
	struct gpu_compute_unit_t *compute_unit;
	char buf[MAX_STRING_SIZE];

	/* Create compute unit */
	compute_unit = calloc(1, sizeof(struct gpu_compute_unit_t));
	compute_unit->wavefront_pool = linked_list_create();

	/* Local memory */
	snprintf(buf, sizeof buf, "LocalMemory[%d]", compute_unit->id);
	compute_unit->local_mod = mod_create(buf, mod_kind_main_memory,
		gpu_local_mem_read_ports + gpu_local_mem_write_ports,  /* FIXME */
		gpu_local_mem_banks, gpu_local_mem_read_ports, gpu_local_mem_write_ports,
		gpu_local_mem_block_size, gpu_local_mem_latency);

	/* Initialize CF Engine */
	compute_unit->cf_engine.fetch_buffer = calloc(gpu_max_wavefronts_per_compute_unit, sizeof(void *));
	compute_unit->cf_engine.inst_buffer = calloc(gpu_max_wavefronts_per_compute_unit, sizeof(void *));
	compute_unit->cf_engine.complete_queue = linked_list_create();

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

	/* List of mapped work-groups */
	compute_unit->work_groups = calloc(gpu_max_work_groups_per_compute_unit, sizeof(void *));

	/* Return */
	return compute_unit;
}


void gpu_compute_unit_free(struct gpu_compute_unit_t *compute_unit)
{
	struct heap_t *event_queue;
	struct gpu_uop_t *uop;
	int i;

	/* CF Engine - free uops in fetch buffer, instruction buffer, and complete queue */
	for (i = 0; i < gpu_max_wavefronts_per_compute_unit; i++)
	{
		gpu_uop_free(compute_unit->cf_engine.fetch_buffer[i]);
		gpu_uop_free(compute_unit->cf_engine.inst_buffer[i]);
	}
	gpu_uop_list_free(compute_unit->cf_engine.complete_queue);

	/* CF Engine - free structures */
	free(compute_unit->cf_engine.fetch_buffer);
	free(compute_unit->cf_engine.inst_buffer);
	linked_list_free(compute_unit->cf_engine.complete_queue);

	/* ALU Engine - free uops in event queue (heap) */
	event_queue = compute_unit->alu_engine.event_queue;
	while (heap_count(event_queue)) {
		heap_extract(event_queue, (void **) &uop);
		uop->write_subwavefront_count++;
		if (uop->write_subwavefront_count == uop->subwavefront_count)
			gpu_uop_free(uop);
	}

	/* ALU Engine - free uops in fetch queue, instruction buffer, execution buffer,
	 * and event queue. Also free CF instruction currently running. */
	gpu_uop_list_free(compute_unit->alu_engine.pending_queue);
	gpu_uop_list_free(compute_unit->alu_engine.finished_queue);
	gpu_uop_list_free(compute_unit->alu_engine.fetch_queue);
	gpu_uop_free(compute_unit->alu_engine.inst_buffer);
	gpu_uop_free(compute_unit->alu_engine.exec_buffer);

	/* ALU Engine - structures */
	linked_list_free(compute_unit->alu_engine.pending_queue);
	linked_list_free(compute_unit->alu_engine.finished_queue);
	linked_list_free(compute_unit->alu_engine.fetch_queue);
	heap_free(compute_unit->alu_engine.event_queue);

	/* TEX Engine - free uop in fetch queue, instruction buffer, write buffer. */
	gpu_uop_list_free(compute_unit->tex_engine.pending_queue);
	gpu_uop_list_free(compute_unit->tex_engine.finished_queue);
	gpu_uop_list_free(compute_unit->tex_engine.fetch_queue);
	gpu_uop_free(compute_unit->tex_engine.inst_buffer);
	gpu_uop_list_free(compute_unit->tex_engine.load_queue);

	/* TEX Engine - structures */
	linked_list_free(compute_unit->tex_engine.pending_queue);
	linked_list_free(compute_unit->tex_engine.finished_queue);
	linked_list_free(compute_unit->tex_engine.fetch_queue);
	linked_list_free(compute_unit->tex_engine.load_queue);

	/* Compute unit */
	linked_list_free(compute_unit->wavefront_pool);
	free(compute_unit->work_groups);  /* List of mapped work-groups */
	mod_free(compute_unit->local_mod);
	free(compute_unit);
}


void gpu_compute_unit_map_work_group(struct gpu_compute_unit_t *compute_unit, struct gpu_work_group_t *work_group)
{
	struct gpu_ndrange_t *ndrange = work_group->ndrange;
	struct gpu_wavefront_t *wavefront;
	int wavefront_id;

	/* Map work-group */
	assert(compute_unit->work_group_count < gpu->work_groups_per_compute_unit);
	assert(!work_group->id_in_compute_unit);
	while (work_group->id_in_compute_unit < gpu->work_groups_per_compute_unit
		&& compute_unit->work_groups[work_group->id_in_compute_unit])
		work_group->id_in_compute_unit++;
	assert(work_group->id_in_compute_unit < gpu->work_groups_per_compute_unit);
	compute_unit->work_groups[work_group->id_in_compute_unit] = work_group;
	compute_unit->work_group_count++;

	/* If compute unit reached its maximum load, remove it from 'ready' list.
	 * Otherwise, move it to the end of the 'ready' list. */
	assert(DOUBLE_LINKED_LIST_MEMBER(gpu, ready, compute_unit));
	DOUBLE_LINKED_LIST_REMOVE(gpu, ready, compute_unit);
	if (compute_unit->work_group_count < gpu->work_groups_per_compute_unit)
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, ready, compute_unit);
	
	/* If this is the first scheduled work-group, insert to 'busy' list. */
	if (!DOUBLE_LINKED_LIST_MEMBER(gpu, busy, compute_unit))
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, busy, compute_unit);

	/* Assign wavefronts identifiers in compute unit */
	FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id) {
		wavefront = ndrange->wavefronts[wavefront_id];
		wavefront->id_in_compute_unit = work_group->id_in_compute_unit *
			ndrange->wavefronts_per_work_group + wavefront->id_in_work_group;
	}

	/* Change work-group status to running */
	gpu_work_group_clear_status(work_group, gpu_work_group_pending);
	gpu_work_group_set_status(work_group, gpu_work_group_running);

	/* Insert all wavefronts into the CF Engine's wavefront pool */
	FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id) {
		wavefront = ndrange->wavefronts[wavefront_id];
		linked_list_add(compute_unit->wavefront_pool, wavefront);
	}

	/* Debug */
	gpu_pipeline_debug("cu a=\"map\" "
		"cu=%d "
		"wg=%d\n",
		compute_unit->id,
		work_group->id);
	
	/* Stats */
	compute_unit->mapped_work_groups++;
}


void gpu_compute_unit_unmap_work_group(struct gpu_compute_unit_t *compute_unit, struct gpu_work_group_t *work_group)
{
	/* Reset mapped work-group */
	assert(compute_unit->work_group_count > 0);
	assert(compute_unit->work_groups[work_group->id_in_compute_unit]);
	compute_unit->work_groups[work_group->id_in_compute_unit] = NULL;
	compute_unit->work_group_count--;

	/* If compute unit accepts work-groups again, insert into 'ready' list */
	if (!DOUBLE_LINKED_LIST_MEMBER(gpu, ready, compute_unit))
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, ready, compute_unit);
	
	/* If compute unit is not busy anymore, remove it from 'busy' list */
	if (!compute_unit->work_group_count && DOUBLE_LINKED_LIST_MEMBER(gpu, busy, compute_unit))
		DOUBLE_LINKED_LIST_REMOVE(gpu, busy, compute_unit);

	/* Debug */
	gpu_pipeline_debug("cu a=\"unmap\" "
		"cu=%d "
		"wg=%d\n",
		compute_unit->id,
		work_group->id);
}


/* Advance one cycle in the compute unit by running every stage from last to first */
void gpu_compute_unit_run(struct gpu_compute_unit_t *compute_unit)
{
	/* Run Engines */
	gpu_alu_engine_run(compute_unit);
	gpu_tex_engine_run(compute_unit);
	gpu_cf_engine_run(compute_unit);

	/* Stats */
	compute_unit->cycle++;
}

