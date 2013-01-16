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

#include <assert.h>

#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/emu/wavefront.h>
#include <arch/southern-islands/emu/work-group.h>
#include <lib/mhandle/mhandle.h>

#include "gpu.h"
#include "wavefront-pool.h"
#include "cycle-interval-report.h"

struct si_wavefront_pool_t *si_wavefront_pool_create()
{
	struct si_wavefront_pool_t *wavefront_pool;
	int i;

	/* Initialize */
	wavefront_pool = xcalloc(1, sizeof(struct si_wavefront_pool_t));
	wavefront_pool->entries = xcalloc(si_gpu_max_wavefronts_per_wavefront_pool, 
		sizeof(struct si_wavefront_pool_entry_t*));

	for (i = 0; i < si_gpu_max_wavefronts_per_wavefront_pool; i++) 
	{
		wavefront_pool->entries[i] = xcalloc(1, sizeof(struct si_wavefront_pool_entry_t));
		wavefront_pool->entries[i]->id_in_wavefront_pool = i;
		wavefront_pool->entries[i]->wavefront_pool = wavefront_pool;
	}

	/* Return */
	return wavefront_pool;
}

void si_wavefront_pool_free(struct si_wavefront_pool_t *wavefront_pool)
{
	int i;

	for (i = 0; i < si_gpu_max_wavefronts_per_wavefront_pool; i++) 
		free(wavefront_pool->entries[i]);

	free(wavefront_pool->entries);
	free(wavefront_pool);
}

void si_wavefront_pool_map_wavefronts(struct si_wavefront_pool_t *wavefront_pool, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wg_id_in_ib;
	int first_entry;
	int i;

	/* Determine starting ID for wavefronts in the instruction buffer */
	wg_id_in_ib = work_group->id_in_compute_unit/si_gpu_num_wavefront_pools;
	first_entry = wg_id_in_ib * ndrange->wavefronts_per_work_group;

	/* Assign wavefronts a slot in the instruction buffer */
	for (i = 0; i < ndrange->wavefronts_per_work_group; i++) 
	{
		wavefront = work_group->wavefronts[i];
		wavefront->wavefront_pool_entry = wavefront_pool->entries[first_entry + i];
		assert(!wavefront->wavefront_pool_entry->valid);

		/* Set initial state */
		wavefront->wavefront_pool_entry->valid = 1;
		wavefront->wavefront_pool_entry->ready = 1;
		wavefront->wavefront_pool_entry->wavefront = wavefront;

		wavefront_pool->wavefront_count++;
	}
}

void si_wavefront_pool_unmap_wavefronts(struct si_wavefront_pool_t *wavefront_pool, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wf_id_in_ib;
	int i;

	/* Reset mapped wavefronts */
	assert(wavefront_pool->wavefront_count >= ndrange->wavefronts_per_work_group);

	for (i = 0; i < ndrange->wavefronts_per_work_group; i++) 
	{
		wavefront = work_group->wavefronts[i];
		wf_id_in_ib = wavefront->wavefront_pool_entry->id_in_wavefront_pool;

		/* TODO Add complete flag to slots in instruction buffer */
		/* TODO Check that all slots are complete before setting to NULL */
		assert(wavefront_pool->entries[wf_id_in_ib]->wavefront);
		assert(wavefront_pool->entries[wf_id_in_ib]->wavefront->id == wavefront->id);
		wavefront_pool->entries[wf_id_in_ib]->valid = 0;
		wavefront_pool->entries[wf_id_in_ib]->wavefront_finished = 0;
		wavefront_pool->entries[wf_id_in_ib]->wavefront = NULL;
	}
	wavefront_pool->wavefront_count -= ndrange->wavefronts_per_work_group;
}
