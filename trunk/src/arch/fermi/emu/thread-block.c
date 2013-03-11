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
#include "thread-block.h"
#include "warp.h"


/*
 * Public Functions
 */


struct frm_thread_block_t *frm_thread_block_create()
{
	struct frm_thread_block_t *thread_block;

	/* Initialize */
	thread_block = xcalloc(1, sizeof(struct frm_thread_block_t));
	thread_block->local_mem = mem_create();
	thread_block->local_mem->safe = 0;

	/* Return */
	return thread_block;
}


void frm_thread_block_free(struct frm_thread_block_t *thread_block)
{
	mem_free(thread_block->local_mem);
	free(thread_block);
}


int frm_thread_block_get_status(struct frm_thread_block_t *thread_block, enum frm_thread_block_status_t status)
{
	return (thread_block->status & status) > 0;
}


void frm_thread_block_set_status(struct frm_thread_block_t *thread_block, enum frm_thread_block_status_t status)
{
	struct frm_grid_t *grid = thread_block->grid;

	/* Get only the new bits */
	status &= ~thread_block->status;

	/* Add thread_block to lists */
	if (status & frm_thread_block_pending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(grid, pending, thread_block);
	if (status & frm_thread_block_running)
		DOUBLE_LINKED_LIST_INSERT_TAIL(grid, running, thread_block);
	if (status & frm_thread_block_finished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(grid, finished, thread_block);

	/* Update it */
	thread_block->status |= status;
}


void frm_thread_block_clear_status(struct frm_thread_block_t *thread_block, enum frm_thread_block_status_t status)
{
	struct frm_grid_t *grid = thread_block->grid;

	/* Get only the bits that are set */
	status &= thread_block->status;

	/* Remove thread_block from lists */
	if (status & frm_thread_block_pending)
		DOUBLE_LINKED_LIST_REMOVE(grid, pending, thread_block);
	if (status & frm_thread_block_running)
		DOUBLE_LINKED_LIST_REMOVE(grid, running, thread_block);
	if (status & frm_thread_block_finished)
		DOUBLE_LINKED_LIST_REMOVE(grid, finished, thread_block);
	
	/* Update status */
	thread_block->status &= ~status;
}


void frm_thread_block_dump(struct frm_thread_block_t *thread_block, FILE *f)
{
	struct frm_grid_t *grid = thread_block->grid;
	struct frm_warp_t *warp;
	int warp_id;

	if (!f)
		return;
	
	fprintf(f, "[ Grid[%d].ThreadBlock[%d] ]\n\n", grid->id, thread_block->id);
	fprintf(f, "Name = %s\n", thread_block->name);
	fprintf(f, "WarpFirst = %d\n", thread_block->warp_id_first);
	fprintf(f, "WarpLast = %d\n", thread_block->warp_id_last);
	fprintf(f, "WarpCount = %d\n", thread_block->warp_count);
	fprintf(f, "ThreadFirst = %d\n", thread_block->thread_id_first);
	fprintf(f, "ThreadLast = %d\n", thread_block->thread_id_last);
	fprintf(f, "ThreadCount = %d\n", thread_block->thread_count);
	fprintf(f, "\n");

	/* Dump warps */
	FRM_FOREACH_WARP_IN_THREADBLOCK(thread_block, warp_id)
	{
		warp = grid->warps[warp_id];
		frm_warp_dump(warp, f);
	}
}

