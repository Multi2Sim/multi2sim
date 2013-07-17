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


#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <mem-system/memory.h>

#include "thread-block.h"
#include "thread.h"
#include "warp.h"


/*
 * Public Functions
 */


struct frm_thread_block_t *frm_thread_block_create()
{
	struct frm_thread_block_t *thread_block;

	/* Initialize */
	thread_block = xcalloc(1, sizeof(struct frm_thread_block_t));
	thread_block->shared_mem = mem_create();
	thread_block->shared_mem->safe = 0;
	thread_block->num_warps_at_barrier = 0;

	/* Return */
	return thread_block;
}


void frm_thread_block_free(struct frm_thread_block_t *thread_block)
{
	int i;

	for (i = 0; i < thread_block->warp_count; i++)
                frm_warp_free(thread_block->warps[i]);
	free(thread_block->warps);
	list_free(thread_block->running_warps);
	list_free(thread_block->finished_warps);

	for (i = 0; i < thread_block->thread_count; i++)
                frm_thread_free(thread_block->threads[i]);
	free(thread_block->threads);

	mem_free(thread_block->shared_mem);
	free(thread_block);
}


void frm_thread_block_dump(struct frm_thread_block_t *thread_block, FILE *f)
{
//	struct frm_grid_t *grid = thread_block->grid;
//	struct frm_warp_t *warp;
//	int warp_id;
//
//	if (!f)
//		return;
//	
//	fprintf(f, "[ Grid[%d].ThreadBlock[%d] ]\n\n", grid->id, thread_block->id);
//	fprintf(f, "Name = %s\n", thread_block->name);
//	fprintf(f, "WarpCount = %d\n", thread_block->warp_count);
//	fprintf(f, "ThreadCount = %d\n", thread_block->thread_count);
//	fprintf(f, "\n");
//
//	/* Dump warps */
//	FRM_FOREACH_WARP_IN_THREADBLOCK(thread_block, warp_id)
//	{
//		warp = grid->warps[warp_id];
//		frm_warp_dump(warp, f);
//	}
}

