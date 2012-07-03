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


struct string_map_t si_gpu_sched_policy_map =
{
		2, {
			{ "RoundRobin", si_gpu_sched_round_robin },
			{ "Greedy", si_gpu_sched_greedy }
		}
	};
enum si_gpu_sched_policy_t si_gpu_sched_policy;




/*
 * Private Functions
 */

#if 0
static struct si_wavefront_t *si_schedule_round_robin(struct si_compute_unit_t *compute_unit)
{
	struct si_wavefront_t *wavefront, *temp_wavefront;
	struct linked_list_t *wavefront_pool = compute_unit->wavefront_pool;

	/* Select current position in pool as initial candidate wavefront */
	if (!linked_list_get(wavefront_pool))
		linked_list_head(wavefront_pool);
	wavefront = linked_list_get(wavefront_pool);
	temp_wavefront = wavefront;

	/* Look for a valid candidate */
	for (;;)
	{
		/* Wavefront must be running,
		 * and the corresponding slot in fetch buffer must be free. */
		/* FIXME */
		/*
		assert(wavefront->id_in_compute_unit < si_gpu->wavefronts_per_compute_unit);
		if (DOUBLE_LINKED_LIST_MEMBER(wavefront->work_group, running, wavefront) &&
			!compute_unit->simd->fetch_buffer[wavefront->id_in_compute_unit])
			break;
			*/

		/* Current candidate is not valid - go to next.
		 * If we went through the whole pool, no fetch. */
		linked_list_next_circular(wavefront_pool);
		wavefront = linked_list_get(wavefront_pool);
		if (wavefront == temp_wavefront)
			return NULL;
	}

	/* Wavefront found, remove from pool and return. */
	linked_list_remove(wavefront_pool);
	return wavefront;
}
#endif






/*
 * Global Functions
 */

/* Return a wavefront from the wavefront pool in the compute unit.
 * If a wavefront was found, it will be extracted from the wavefront pool.
 * If no valid candidate is found in the wavefront pool, the function returns NULL. */
#if 0
struct si_wavefront_t *si_compute_unit_schedule(struct si_compute_unit_t *compute_unit)
{
	struct si_wavefront_t *wavefront;

	/* If there is no wavefront in the pool, return NULL. */
	if (!linked_list_count(compute_unit->wavefront_pool))
		return NULL;

	/* Run different scheduling algorithm depending on configured policy */
	switch (si_gpu_sched_policy)
	{

	case si_gpu_sched_round_robin:

		wavefront = si_schedule_round_robin(compute_unit);
		break;

	default:
		panic("%s: invalid policy", __FUNCTION__);
	}

	/* Scheduled wavefront */
	return wavefront;
}
#endif
