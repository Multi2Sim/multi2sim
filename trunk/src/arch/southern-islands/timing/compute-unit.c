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

#include <southern-islands-timing.h>
#include <heap.h>




/*
 * Compute Unit
 */

struct si_compute_unit_t *si_compute_unit_create()
{
	struct si_compute_unit_t *compute_unit;
	char buf[MAX_STRING_SIZE];
	int i;

	/* Create */
	compute_unit = calloc(1, sizeof(struct si_compute_unit_t));
	if (!compute_unit)
		fatal("%s: out of memory", __FUNCTION__);

	/* Local memory */
	snprintf(buf, sizeof buf, "LocalMemory[%d]", compute_unit->id);
	compute_unit->local_memory = mod_create(buf, mod_kind_local_memory,
		si_gpu_local_mem_num_ports, si_gpu_local_mem_block_size, si_gpu_local_mem_latency);

	/* Hardware structures */
	compute_unit->num_wavefront_pools = si_gpu_num_wavefront_pools;
	compute_unit->wavefront_pools = calloc(compute_unit->num_wavefront_pools, 
		sizeof(struct si_wavefront_pool_t*));
	compute_unit->fetch_buffers = calloc(compute_unit->num_wavefront_pools, 
		sizeof(struct si_uop_t*));
	compute_unit->simds = calloc(compute_unit->num_wavefront_pools, sizeof(struct si_simd_t*));
	for (i = 0; i < compute_unit->num_wavefront_pools; i++) 
	{
		compute_unit->wavefront_pools[i] = si_wavefront_pool_create();
		compute_unit->wavefront_pools[i]->id = i;
		compute_unit->wavefront_pools[i]->compute_unit = compute_unit;
		DOUBLE_LINKED_LIST_INSERT_TAIL(compute_unit, wavefront_pool_ready, 
			compute_unit->wavefront_pools[i]);

		compute_unit->simds[i] = calloc(1, sizeof(struct si_simd_t));
		if (!compute_unit->simds[i])
			fatal("%s: out of memory", __FUNCTION__);
		compute_unit->fetch_buffers[i] = calloc(si_gpu_max_wavefronts_per_wavefront_pool,
			sizeof(struct si_uop_t));
		if (!compute_unit->fetch_buffers[i])
			fatal("%s: out of memory", __FUNCTION__);
	}

	compute_unit->work_groups = calloc(si_gpu_max_work_groups_per_wavefront_pool * 
		si_gpu_num_wavefront_pools, sizeof(void *));
	if (!compute_unit->work_groups)
		fatal("%s: out of memory", __FUNCTION__);

	/* Return */
	return compute_unit;
}


void si_compute_unit_free(struct si_compute_unit_t *compute_unit)
{
	int i;

	/* SIMD - free uops in fetch buffer, instruction buffer, and complete queue */
	for (i = 0; i < compute_unit->num_wavefront_pools; i++)
	{
		si_uop_free(compute_unit->simds[i]->inst_buffer);
		si_uop_free(compute_unit->simds[i]->exec_buffer);
	}

	/* Scalar Unit - free uops in fetch buffer, instruction buffer, and complete queue */
	si_uop_free(compute_unit->scalar_unit.inst_buffer);
	si_uop_free(compute_unit->scalar_unit.exec_buffer);

	/* Branch Unit - free uops in fetch buffer, instruction buffer, and complete queue */
	si_uop_free(compute_unit->branch_unit.inst_buffer);
	si_uop_free(compute_unit->branch_unit.exec_buffer);

	/* Compute unit */
	for (i = 0; i < compute_unit->num_wavefront_pools; i++)
	{
		free(compute_unit->simds[i]);
		si_wavefront_pool_free(compute_unit->wavefront_pools[i]);
		si_uop_free(compute_unit->fetch_buffers[i]);
	}
	free(compute_unit->simds);
	free(compute_unit->wavefront_pools);
	free(compute_unit->fetch_buffers);
	free(compute_unit->work_groups);  /* List of mapped work-groups */
	mod_free(compute_unit->local_memory);
	free(compute_unit);
}


void si_compute_unit_map_work_group(struct si_compute_unit_t *compute_unit, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wavefront_id;

	printf("mapping work group %d to compute unit %d\n", work_group->id, compute_unit->id);

	/* Map work-group */
	assert(compute_unit->work_group_count < si_gpu->work_groups_per_compute_unit);
	assert(!work_group->id_in_compute_unit);
	while (work_group->id_in_compute_unit < si_gpu->work_groups_per_compute_unit &&
		compute_unit->work_groups[work_group->id_in_compute_unit])
		work_group->id_in_compute_unit++;
	assert(work_group->id_in_compute_unit < si_gpu->work_groups_per_compute_unit);
	compute_unit->work_groups[work_group->id_in_compute_unit] = work_group;
	compute_unit->work_group_count++;

	/* If compute unit reached its maximum load, remove it from 'compute_unit_ready' list.
	 * Otherwise, move it to the end of the 'compute_unit_ready' list. */
	assert(DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_ready, compute_unit));
	DOUBLE_LINKED_LIST_REMOVE(si_gpu, compute_unit_ready, compute_unit);
	if (compute_unit->work_group_count < si_gpu->work_groups_per_compute_unit)
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_ready, compute_unit);
	
	/* If this is the first scheduled work-group, insert to 'compute_unit_busy' list. */
	if (!DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_busy, compute_unit))
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_busy, compute_unit);

	/* Assign wavefronts identifiers in compute unit */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = ndrange->wavefronts[wavefront_id];
		wavefront->id_in_compute_unit = work_group->id_in_compute_unit *
			ndrange->wavefronts_per_work_group + wavefront->id_in_work_group;
	}

	/* Change work-group status to running */
	si_work_group_clear_status(work_group, si_work_group_pending);
	si_work_group_set_status(work_group, si_work_group_running);

	/* Insert wavefronts into a wavefront pool */
	assert(compute_unit->wavefront_pool_ready_list_head);
	si_wavefront_pool_map_wavefronts(compute_unit->wavefront_pool_ready_list_head, work_group);

	/* Trace */
	si_trace("si.map_wg cu=%d wg=%d wi_first=%d wi_count=%d wf_first=%d wf_count=%d\n",
		compute_unit->id, work_group->id, work_group->work_item_id_first,
		work_group->work_item_count, work_group->wavefront_id_first,
		work_group->wavefront_count);

	/* Stats */
	compute_unit->mapped_work_groups++;
}


void si_compute_unit_unmap_work_group(struct si_compute_unit_t *compute_unit, struct si_work_group_t *work_group)
{
	/* Reset mapped work-group */
	assert(compute_unit->work_group_count > 0);
	assert(compute_unit->work_groups[work_group->id_in_compute_unit]);
	compute_unit->work_groups[work_group->id_in_compute_unit] = NULL;
	compute_unit->work_group_count--;

	/* If compute unit accepts work-groups again, insert into 'compute_unit_ready' list */
	if (!DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_ready, compute_unit))
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_ready, compute_unit);
	
	/* If compute unit is not compute_unit_busy anymore, remove it from 
	 * 'compute_unit_busy' list */
	if (!compute_unit->work_group_count && DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_busy,
		compute_unit))
	{
		DOUBLE_LINKED_LIST_REMOVE(si_gpu, compute_unit_busy, compute_unit);
	}

	/* Trace */
	si_trace("si.unmap_wg cu=%d wg=%d\n",
		compute_unit->id, work_group->id);
}


/* Advance one cycle in the compute unit by running every stage from last to first */
void si_compute_unit_run(struct si_compute_unit_t *compute_unit)
{
	/* Run Engines */
	//si_compute_unit_run_simd(compute_unit);
	//si_compute_unit_run_scalar_unit(compute_unit);
	//si_compute_unit_run_branch_unit(compute_unit);

	/* Stats */
	compute_unit->cycle++;
}

