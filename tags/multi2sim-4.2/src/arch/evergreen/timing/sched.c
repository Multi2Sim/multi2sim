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


#include <arch/evergreen/emu/wavefront.h>
#include <arch/evergreen/emu/work-group.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "compute-unit.h"
#include "gpu.h"
#include "sched.h"


struct str_map_t evg_gpu_sched_policy_map =
{
		2, {
			{ "RoundRobin", evg_gpu_sched_round_robin },
			{ "Greedy", evg_gpu_sched_greedy }
		}
	};
enum evg_gpu_sched_policy_t evg_gpu_sched_policy;




/*
 * Private Functions
 */

static struct evg_wavefront_t *evg_schedule_round_robin(struct evg_compute_unit_t *compute_unit)
{
	struct evg_wavefront_t *wavefront, *temp_wavefront;
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
		assert(wavefront->id_in_compute_unit < evg_gpu->wavefronts_per_compute_unit);
		if (DOUBLE_LINKED_LIST_MEMBER(wavefront->work_group, running, wavefront) &&
			!compute_unit->cf_engine.fetch_buffer[wavefront->id_in_compute_unit])
			break;

		/* Current candidate is not valid - go to next.
		 * If we went through the whole pool, no fetch. */
		linked_list_next_circular(wavefront_pool);
		wavefront = linked_list_get(wavefront_pool);
		if (wavefront == temp_wavefront)
			return NULL;
	}

	/* Wavefront found, remove from pool and return. */
	assert(wavefront->clause_kind == EVG_CLAUSE_CF);
	linked_list_remove(wavefront_pool);
	return wavefront;
}


static struct evg_wavefront_t *evg_schedule_greedy(struct evg_compute_unit_t *compute_unit)
{
	struct evg_wavefront_t *wavefront, *temp_wavefront;
	struct linked_list_t *wavefront_pool = compute_unit->wavefront_pool;

	/* Check all candidates */
	temp_wavefront = NULL;
	LINKED_LIST_FOR_EACH(wavefront_pool)
	{
		/* Get wavefront from list */
		wavefront = linked_list_get(wavefront_pool);
		
		/* Wavefront must be running,
		 * and the corresponding slot in fetch buffer must be free. */
		assert(wavefront->id_in_compute_unit < evg_gpu->wavefronts_per_compute_unit);
		if (!DOUBLE_LINKED_LIST_MEMBER(wavefront->work_group, running, wavefront) ||
			compute_unit->cf_engine.fetch_buffer[wavefront->id_in_compute_unit])
			continue;

		/* Select current wavefront temporarily */
		if (!temp_wavefront || temp_wavefront->sched_when < wavefront->sched_when)
			temp_wavefront = wavefront;
	}

	/* No wavefront found */
	wavefront = NULL;
	if (!temp_wavefront)
		return NULL;

	/* Wavefront found, remove from pool and return. */
	assert(temp_wavefront->clause_kind == EVG_CLAUSE_CF);
	linked_list_find(wavefront_pool, temp_wavefront);
	assert(!wavefront_pool->error_code);
	linked_list_remove(wavefront_pool);
	temp_wavefront->sched_when = asTiming(evg_gpu)->cycle;
	return temp_wavefront;
}




/*
 * Global Functions
 */

/* Return a wavefront from the wavefront pool in the compute unit.
 * If a wavefront was found, it will be extracted from the wavefront pool.
 * If no valid candidate is found in the wavefront pool, the function returns NULL. */
struct evg_wavefront_t *evg_schedule(struct evg_compute_unit_t *compute_unit)
{
	struct evg_wavefront_t *wavefront;

	/* If there is no wavefront in the pool, return NULL. */
	if (!linked_list_count(compute_unit->wavefront_pool))
		return NULL;

	/* Run different scheduling algorithm depending on configured policy */
	switch (evg_gpu_sched_policy)
	{

	case evg_gpu_sched_round_robin:

		wavefront = evg_schedule_round_robin(compute_unit);
		break;

	case evg_gpu_sched_greedy:

		wavefront = evg_schedule_greedy(compute_unit);
		break;

	default:
		panic("%s: invalid policy", __FUNCTION__);
	}

	/* Scheduled wavefront */
	return wavefront;
}

