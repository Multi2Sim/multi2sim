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

#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"



/*
 * Public Functions
 */


struct evg_work_group_t *evg_work_group_create(void)
{
	struct evg_work_group_t *work_group;

	/* Initialize */
	work_group = xcalloc(1, sizeof(struct evg_work_group_t));
	work_group->local_mem = mem_create();
	work_group->local_mem->safe = 0;

	/* Return */
	return work_group;
}


void evg_work_group_free(struct evg_work_group_t *work_group)
{
	mem_free(work_group->local_mem);
	str_free(work_group->name);
	free(work_group);
}


int evg_work_group_get_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status)
{
	return (work_group->status & status) > 0;
}


void evg_work_group_set_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status)
{
	struct evg_ndrange_t *ndrange = work_group->ndrange;

	/* Get only the new bits */
	status &= ~work_group->status;

	/* Add work-group to lists */
	if (status & evg_work_group_pending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, pending, work_group);
	if (status & evg_work_group_running)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, running, work_group);
	if (status & evg_work_group_finished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, finished, work_group);

	/* Update it */
	work_group->status |= status;
}


void evg_work_group_clear_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status)
{
	struct evg_ndrange_t *ndrange = work_group->ndrange;

	/* Get only the bits that are set */
	status &= work_group->status;

	/* Remove work-group from lists */
	if (status & evg_work_group_pending)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, pending, work_group);
	if (status & evg_work_group_running)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, running, work_group);
	if (status & evg_work_group_finished)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, finished, work_group);
	
	/* Update status */
	work_group->status &= ~status;
}


void evg_work_group_dump(struct evg_work_group_t *work_group, FILE *f)
{
	struct evg_ndrange_t *ndrange = work_group->ndrange;
	struct evg_wavefront_t *wavefront;
	int wavefront_id;

	if (!f)
		return;
	
	fprintf(f, "[ NDRange[%d].WorkGroup[%d] ]\n\n", ndrange->id, work_group->id);
	fprintf(f, "Name = %s\n", work_group->name);
	fprintf(f, "WaveFrontFirst = %d\n", work_group->wavefront_id_first);
	fprintf(f, "WaveFrontLast = %d\n", work_group->wavefront_id_last);
	fprintf(f, "WaveFrontCount = %d\n", work_group->wavefront_count);
	fprintf(f, "WorkItemFirst = %d\n", work_group->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", work_group->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", work_group->work_item_count);
	fprintf(f, "\n");

	/* Dump wavefronts */
	EVG_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = ndrange->wavefronts[wavefront_id];
		evg_wavefront_dump(wavefront, f);
	}
}


void evg_work_group_set_name(struct evg_work_group_t *work_group, char *name)
{
	work_group->name = str_set(work_group->name, name);
}
