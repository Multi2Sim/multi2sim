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

#include "timing.h"


struct si_wavefront_pool_t *si_wavefront_pool_create()
{
	struct si_wavefront_pool_t *wavefront_pool;

	/* Create */
	wavefront_pool = calloc(1, sizeof(struct si_wavefront_pool_t));
	if (!wavefront_pool)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	wavefront_pool->wavefronts = calloc(si_gpu_max_wavefronts_per_wavefront_pool, 
		sizeof(struct si_wavefront_t));
	if (!wavefront_pool->wavefronts)
		fatal("%s: out of memory", __FUNCTION__);

	wavefront_pool->wavefront_count = 0;

	/* Return */
	return wavefront_pool;
}

void si_wavefront_pool_free(struct si_wavefront_pool_t *wavefront_pool)
{
	free(wavefront_pool->wavefronts);
	free(wavefront_pool);
}

void si_wavefront_pool_map_wavefronts(struct si_wavefront_pool_t *wavefront_pool, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wg_id_in_wfp;
	int first_wavefront;
	int i;

	/* Determine starting ID for wavefronts in the wavefront pool */
	wg_id_in_wfp = work_group->id_in_compute_unit/si_gpu_num_wavefront_pools;
	first_wavefront = wg_id_in_wfp * ndrange->wavefronts_per_work_group;

	/* Insert wavefronts into the wavefront pool */
	for (i = 0; i < ndrange->wavefronts_per_work_group; i++) 
	{
		wavefront = work_group->wavefronts[i];
		wavefront->id_in_wavefront_pool = first_wavefront + i;
		assert(!wavefront_pool->wavefronts[wavefront->id_in_wavefront_pool]);

		wavefront->ready = 1;
		wavefront->wavefront_pool = wavefront_pool;

		wavefront_pool->wavefront_count++;
		wavefront_pool->wavefronts[wavefront->id_in_wavefront_pool] = wavefront;
	}
}

void si_wavefront_pool_unmap_wavefronts(struct si_wavefront_pool_t *wavefront_pool, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wf_id_in_wfp;
	int i;

	/* Reset mapped wavefronts */
	assert(wavefront_pool->wavefront_count >= ndrange->wavefronts_per_work_group);

	for (i = 0; i < ndrange->wavefronts_per_work_group; i++) 
	{
		wavefront = work_group->wavefronts[i];
		wf_id_in_wfp = wavefront->id_in_wavefront_pool;

		/* TODO Add complete flag to slots in wavefront pool */
		/* TODO Check that all slots are complete before setting to NULL */
		assert(wavefront_pool->wavefronts[wf_id_in_wfp]);
		assert(wavefront_pool->wavefronts[wf_id_in_wfp]->id == wavefront->id);
		wavefront_pool->wavefronts[wf_id_in_wfp] = NULL;
	}
	wavefront_pool->wavefront_count -= ndrange->wavefronts_per_work_group;
}
