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
		sizeof(struct si_fetch_buffer_t*));
	compute_unit->simds = calloc(compute_unit->num_wavefront_pools, sizeof(struct si_simd_t*));

	compute_unit->scalar_unit.mem_queue = linked_list_create();
	compute_unit->scalar_unit.exec_queue = heap_create(10);
	compute_unit->scalar_unit.compute_unit = compute_unit;

	compute_unit->vector_mem_unit.mem_queue = linked_list_create();
	compute_unit->vector_mem_unit.exec_queue = heap_create(10);
	compute_unit->vector_mem_unit.compute_unit = compute_unit;

	for (i = 0; i < compute_unit->num_wavefront_pools; i++) 
	{
		/* Allocate and initialize wavefront pools */
		compute_unit->wavefront_pools[i] = si_wavefront_pool_create();
		compute_unit->wavefront_pools[i]->id = i;
		compute_unit->wavefront_pools[i]->compute_unit = compute_unit;

		/* Allocate and initialize fetch buffers */
		compute_unit->fetch_buffers[i] = calloc(si_gpu_max_wavefronts_per_wavefront_pool, 
			sizeof(struct si_fetch_buffer_t));
		if (!compute_unit->fetch_buffers[i])
			fatal("%s: out of memory", __FUNCTION__);
		compute_unit->fetch_buffers[i]->uops = calloc(
			si_gpu_max_wavefronts_per_wavefront_pool, sizeof(struct si_uop_t *));
		if (!compute_unit->fetch_buffers[i]->uops)
			fatal("%s: out of memory", __FUNCTION__);
		compute_unit->fetch_buffers[i]->cycle_fetched = calloc(
			si_gpu_max_wavefronts_per_wavefront_pool, sizeof(long long int));
		if (!compute_unit->fetch_buffers[i]->cycle_fetched)
			fatal("%s: out of memory", __FUNCTION__);
		compute_unit->fetch_buffers[i]->entries = si_gpu_max_wavefronts_per_wavefront_pool;

		/* Allocate SIMD structures */
		compute_unit->simds[i] = calloc(1, sizeof(struct si_simd_t));
		if (!compute_unit->simds[i])
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

	linked_list_free(compute_unit->scalar_unit.mem_queue);
	heap_free(compute_unit->scalar_unit.exec_queue);

	linked_list_free(compute_unit->vector_mem_unit.mem_queue);
	heap_free(compute_unit->vector_mem_unit.exec_queue);

	/* Compute unit */
	for (i = 0; i < compute_unit->num_wavefront_pools; i++)
	{
		free(compute_unit->simds[i]);

		si_wavefront_pool_free(compute_unit->wavefront_pools[i]);

		free(compute_unit->fetch_buffers[i]->uops);
		free(compute_unit->fetch_buffers[i]->cycle_fetched);
		free(compute_unit->fetch_buffers[i]);
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
	int wfp_id;

	assert(compute_unit->work_group_count < si_gpu->work_groups_per_compute_unit);
	assert(!work_group->id_in_compute_unit);

	/* Find an available slot */
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
	{
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_ready, compute_unit);
	}
	
	/* If this is the first scheduled work-group, insert to 'compute_unit_busy' list. */
	if (!DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_busy, compute_unit))
	{
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_busy, compute_unit);
	}

	/* Assign wavefronts identifiers in compute unit */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = ndrange->wavefronts[wavefront_id];
		wavefront->id_in_compute_unit = work_group->id_in_compute_unit *
			ndrange->wavefronts_per_work_group + wavefront->id_in_work_group;
	}

	/* Set wavefront pool for work group */
	wfp_id = work_group->id_in_compute_unit % si_gpu_num_wavefront_pools;
	work_group->wavefront_pool = compute_unit->wavefront_pools[wfp_id];

	/* Insert wavefronts into a wavefront pool */
	si_wavefront_pool_map_wavefronts(work_group->wavefront_pool, work_group);

	/* Change work-group status to running */
	si_work_group_clear_status(work_group, si_work_group_pending);
	si_work_group_set_status(work_group, si_work_group_running);

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

	/* Unmap wavefronts from wavefront pool */
	si_wavefront_pool_unmap_wavefronts(work_group->wavefront_pool, work_group);

	/* If compute unit accepts work-groups again, insert into 'compute_unit_ready' list */
	if (!DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_ready, compute_unit))
	{
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_ready, compute_unit);
	}
	
	/* If compute unit is not compute_unit_busy anymore, remove it from 
	 * 'compute_unit_busy' list */
	if (!compute_unit->work_group_count && DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_busy,
		compute_unit))
	{
		DOUBLE_LINKED_LIST_REMOVE(si_gpu, compute_unit_busy, compute_unit);
	}

	/* Trace */
	si_trace("si.unmap_wg cu=%d wg=%d\n", compute_unit->id, work_group->id);
}

#include <limits.h>
int *si_fetch_buffer_get_oldest(struct si_fetch_buffer_t *fetch_buffer)
{
	int *id_list;
	long long int *age_list;
	unsigned int i;

	id_list = calloc(fetch_buffer->entries, sizeof(int));
	if (!id_list)
		fatal("%s: out of memory", __FUNCTION__);
	age_list = calloc(fetch_buffer->entries, sizeof(long long int));
	if (!age_list)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize lists */
	for (i = 0; i < fetch_buffer->entries; i++)
	{
		id_list[i] = i;
		if (!fetch_buffer->uops[i])
			age_list[i] = LLONG_MAX;
		else
			age_list[i] = fetch_buffer->cycle_fetched[i];
	}

	/* Sort */
	for (i = 1; i < fetch_buffer->entries; i++)
	{
		long long int cur_age = age_list[i];
		int cur_idx = i;

		while (cur_idx > 0 && cur_age < age_list[cur_idx-1])
		{
			long long int tmp_age = age_list[cur_idx-1];
			age_list[cur_idx-1] = age_list[cur_idx];
			age_list[cur_idx] = tmp_age;

			int tmp_id = id_list[cur_idx-1];
			id_list[cur_idx-1] = id_list[cur_idx];
			id_list[cur_idx] = tmp_id;
			
			cur_idx--;
		}
	}

	free(age_list);

	return id_list;
}


void si_compute_unit_fetch(struct si_compute_unit_t *compute_unit, int active_wfp)
{
	int i;
	struct si_wavefront_t *wavefront;
	struct si_uop_t *uop;

	assert(active_wfp < compute_unit->num_wavefront_pools);

	for (i = 0; i < si_gpu_max_wavefronts_per_wavefront_pool; i++)
	{
		wavefront = compute_unit->wavefront_pools[active_wfp]->wavefronts[i];

		/* If there isn't a wavefront or the wavefront isn't ready, continue */
		if (!wavefront || !wavefront->ready)
			continue;

		/* If there fetch buffer isn't empty, continue */
		if(compute_unit->fetch_buffers[active_wfp]->uops[i])
			continue;

		/* The fetch buffer is empty, so fetch another instruction */

		/* Emulate instruction */
		si_wavefront_execute(wavefront);
		wavefront->ready = 0;

		/* Create uop */
		uop = si_uop_create();
		uop->wavefront = wavefront;
		uop->work_group = wavefront->work_group;
		uop->compute_unit = compute_unit;
		uop->id_in_compute_unit = compute_unit->gpu_uop_id_counter++;
		uop->local_mem_read = wavefront->local_mem_read;
		uop->local_mem_write = wavefront->local_mem_write;
		
		/* Access instruction cache. Record the time when the instruction will have 
		 * been fetched, as per the latency of the instruction memory. */
		uop->fetch_ready = si_gpu->cycle + si_gpu_fetch_latency;

		/* Insert uop to fetch buffer */
		compute_unit->fetch_buffers[active_wfp]->uops[i] = uop;
		compute_unit->fetch_buffers[active_wfp]->cycle_fetched[i] = si_gpu->cycle;
	}
}


/* Decode one instruction per hardware unit */
void si_compute_unit_decode(struct si_compute_unit_t *compute_unit, int active_wfp)
{
	assert(active_wfp < compute_unit->num_wavefront_pools);

	struct si_fetch_buffer_t *fetch_buffer;
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int *search_order;
	int i;

	fetch_buffer = compute_unit->fetch_buffers[active_wfp];

	search_order = si_fetch_buffer_get_oldest(fetch_buffer);

	for (i = 0; i < compute_unit->fetch_buffers[active_wfp]->entries; i++)
	{
		int next_oldest = search_order[i];

		uop = compute_unit->fetch_buffers[active_wfp]->uops[next_oldest];

		/* If the uop is NULL, no more instructions are ready to be decoded */
		if (!uop)
			break;

		/* Skip any uops that have not yet completed the fetch */
		if (si_gpu->cycle < uop->fetch_ready)
			continue;

		/* Decode */
		wavefront = compute_unit->wavefront_pools[active_wfp]->wavefronts[next_oldest];
		assert(wavefront);

		switch (wavefront->inst.info->fmt)
		{

		/* Scalar ALU */
		case SI_FMT_SOP1:
		case SI_FMT_SOP2:
		case SI_FMT_SOPP:
		case SI_FMT_SOPC:
		case SI_FMT_SOPK:
		{
			/* FIXME consider branch unit */

			/* Continue if scalar unit instruction buffer is not free */
			if (compute_unit->scalar_unit.inst_buffer)
				continue;

			/* Decode uop and place it in scalar unit instruction buffer */
			uop->decode_ready = si_gpu->cycle + si_gpu_decode_latency;
			compute_unit->scalar_unit.inst_buffer = uop;

			/* Remove uop from fetch buffer */
			compute_unit->fetch_buffers[active_wfp]->uops[next_oldest] = NULL;
			compute_unit->fetch_buffers[active_wfp]->cycle_fetched[next_oldest] = 
				LLONG_MAX;

			break;
		}

		/* Scalar memory */
		case SI_FMT_SMRD:
		{
			/* Continue if scalar unit instruction buffer is not free */
			if (compute_unit->scalar_unit.inst_buffer)
				continue;

			/* Decode uop and place it in scalar unit instruction buffer */
			uop->decode_ready = si_gpu->cycle + si_gpu_decode_latency;
			compute_unit->scalar_unit.inst_buffer = uop;

			/* Remove uop from fetch buffer */
			compute_unit->fetch_buffers[active_wfp]->uops[next_oldest] = NULL;
			compute_unit->fetch_buffers[active_wfp]->cycle_fetched[next_oldest] = 
				LLONG_MAX;

			break;
		}

		/* Vector ALU */
		case SI_FMT_VOP2:
		case SI_FMT_VOP1:
		case SI_FMT_VOPC:
		case SI_FMT_VOP3a:
		case SI_FMT_VOP3b:
		{
			/* Continue if vector memory unit instruction buffer is not free */
			if (compute_unit->simds[active_wfp]->inst_buffer)
				continue;

			/* Decode uop and place it in vector memory unit instruction buffer */
			uop->decode_ready = si_gpu->cycle + si_gpu_decode_latency;
			compute_unit->simds[active_wfp]->inst_buffer = uop;

			/* Remove uop from fetch buffer */
			compute_unit->fetch_buffers[active_wfp]->uops[next_oldest] = NULL;
			compute_unit->fetch_buffers[active_wfp]->cycle_fetched[next_oldest] = 
				LLONG_MAX;

			break;
		}

		/* Vector memory */
		case SI_FMT_MTBUF:
		{
			/* Continue if vector memory unit instruction buffer is not free */
			if (compute_unit->vector_mem_unit.inst_buffer)
				continue;

			/* Decode uop and place it in vector memory unit instruction buffer */
			uop->decode_ready = si_gpu->cycle + si_gpu_decode_latency;
			compute_unit->vector_mem_unit.inst_buffer = uop;

			/* Remove uop from fetch buffer */
			compute_unit->fetch_buffers[active_wfp]->uops[next_oldest] = NULL;
			compute_unit->fetch_buffers[active_wfp]->cycle_fetched[next_oldest] = 
				LLONG_MAX;

			break;
		}

		/* Local memory */
		case SI_FMT_DS:
		{
			abort();

			/* FIXME Add instruction buffer */

			break;
		}

		default:
		{
			fatal("%s: instruction type not implemented", __FUNCTION__);
		}
		}
	}


	free(search_order);

	/* Trace */
	si_trace("si.inst id=%lld cu=%d stg=\"cf-de\"\n", uop->id_in_compute_unit, 
		compute_unit->id);
}


/* Advance one cycle in the compute unit by running every stage from last to first */
void si_compute_unit_run(struct si_compute_unit_t *compute_unit)
{
	int i;
	int num_simds;
	int active_fetch_wfp;  /* Wavefront pool chosen to fetch this cycle */
	int active_decode_wfp;  /* Wavefront pool chosen to decode this cycle */

	active_fetch_wfp = si_gpu->cycle % compute_unit->num_wavefront_pools;
	active_decode_wfp = (si_gpu->cycle - si_gpu_fetch_latency) % 
		compute_unit->num_wavefront_pools;

	/* Run Engines */
	num_simds = compute_unit->num_wavefront_pools;
	for (i = 0; i < num_simds; i++)
		si_simd_run(compute_unit->simds[i]);

	si_vector_mem_run(&compute_unit->vector_mem_unit);

	si_scalar_unit_run(&compute_unit->scalar_unit);

	//si_branch_unit_run(compute_unit->branch_unit);

	/* Run Decode */
	si_compute_unit_decode(compute_unit, active_decode_wfp);

	/* Run Fetch */
	si_compute_unit_fetch(compute_unit, active_fetch_wfp);

	/* Stats */
	compute_unit->cycle++;
}

