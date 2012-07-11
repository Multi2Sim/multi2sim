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

	wavefront_pool->num_wavefronts = 0;

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
	struct si_compute_unit_t *compute_unit = wavefront_pool->compute_unit;
	struct si_wavefront_t *wavefront;
	int i, j;

	/* Insert wavefronts into a wavefront pool */
	for (i = 0; i < ndrange->wavefronts_per_work_group; i++) 
	{
		assert(wavefront_pool->num_wavefronts < si_gpu_max_wavefronts_per_wavefront_pool);
		wavefront = work_group->wavefronts[i];
		for (j = 0; j < si_gpu_max_wavefronts_per_wavefront_pool; j++)
		{
			if (!wavefront_pool->wavefronts[j])
			{
				wavefront_pool->wavefronts[j] = wavefront;
				wavefront_pool->num_wavefronts++;
				/*printf("  inserting wavefront %d from wg %d into cu %d, "
					"wavefront_pool %d (%d)\n", wavefront->id, work_group->id, 
					compute_unit->id, wavefront_pool->id, 
					wavefront_pool->num_wavefronts);
					*/
				break;
			}
		}
		assert(j < si_gpu_max_wavefronts_per_wavefront_pool);
	}

	/* If wavefront pool reached its maximum load, remove it from 'wavefront_pool_ready' list.
	 * Otherwise, move it to the end of the 'wavefront_pool_ready' list. */
	assert(DOUBLE_LINKED_LIST_MEMBER(compute_unit, wavefront_pool_ready, wavefront_pool));
	DOUBLE_LINKED_LIST_REMOVE(compute_unit, wavefront_pool_ready, wavefront_pool);
	if (wavefront_pool->num_wavefronts < si_gpu->wavefronts_per_wavefront_pool - 
		ndrange->wavefronts_per_work_group);
		DOUBLE_LINKED_LIST_INSERT_TAIL(compute_unit, wavefront_pool_ready, wavefront_pool);
	
	/* If this is the first scheduled work-group, insert to 'wavefront_pool_busy' list. */
	if (!DOUBLE_LINKED_LIST_MEMBER(compute_unit, wavefront_pool_busy, wavefront_pool))
		DOUBLE_LINKED_LIST_INSERT_TAIL(compute_unit, wavefront_pool_busy, wavefront_pool);
}
