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
#include <mem-system/memory.h>

#include "grid.h"
#include "threadblock.h"
#include "warp.h"


/*
 * Public Functions
 */


struct frm_threadblock_t *frm_threadblock_create()
{
	struct frm_threadblock_t *threadblock;

	/* Initialize */
	threadblock = xcalloc(1, sizeof(struct frm_threadblock_t));
	threadblock->local_mem = mem_create();
	threadblock->local_mem->safe = 0;

	/* Return */
	return threadblock;
}


void frm_threadblock_free(struct frm_threadblock_t *threadblock)
{
	mem_free(threadblock->local_mem);
	free(threadblock);
}


int frm_threadblock_get_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status)
{
	return (threadblock->status & status) > 0;
}


void frm_threadblock_set_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status)
{
	struct frm_grid_t *grid = threadblock->grid;

	/* Get only the new bits */
	status &= ~threadblock->status;

	/* Add threadblock to lists */
	if (status & frm_threadblock_pending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(grid, pending, threadblock);
	if (status & frm_threadblock_running)
		DOUBLE_LINKED_LIST_INSERT_TAIL(grid, running, threadblock);
	if (status & frm_threadblock_finished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(grid, finished, threadblock);

	/* Update it */
	threadblock->status |= status;
}


void frm_threadblock_clear_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status)
{
	struct frm_grid_t *grid = threadblock->grid;

	/* Get only the bits that are set */
	status &= threadblock->status;

	/* Remove threadblock from lists */
	if (status & frm_threadblock_pending)
		DOUBLE_LINKED_LIST_REMOVE(grid, pending, threadblock);
	if (status & frm_threadblock_running)
		DOUBLE_LINKED_LIST_REMOVE(grid, running, threadblock);
	if (status & frm_threadblock_finished)
		DOUBLE_LINKED_LIST_REMOVE(grid, finished, threadblock);
	
	/* Update status */
	threadblock->status &= ~status;
}


void frm_threadblock_dump(struct frm_threadblock_t *threadblock, FILE *f)
{
	struct frm_grid_t *grid = threadblock->grid;
	struct frm_warp_t *warp;
	int warp_id;

	if (!f)
		return;
	
	fprintf(f, "[ Grid[%d].ThreadBlock[%d] ]\n\n", grid->id, threadblock->id);
	fprintf(f, "Name = %s\n", threadblock->name);
	fprintf(f, "WarpFirst = %d\n", threadblock->warp_id_first);
	fprintf(f, "WarpLast = %d\n", threadblock->warp_id_last);
	fprintf(f, "WarpCount = %d\n", threadblock->warp_count);
	fprintf(f, "ThreadFirst = %d\n", threadblock->thread_id_first);
	fprintf(f, "ThreadLast = %d\n", threadblock->thread_id_last);
	fprintf(f, "ThreadCount = %d\n", threadblock->thread_count);
	fprintf(f, "\n");

	/* Dump warps */
	FRM_FOREACH_WARP_IN_THREADBLOCK(threadblock, warp_id)
	{
		warp = grid->warps[warp_id];
		frm_warp_dump(warp, f);
	}
}

