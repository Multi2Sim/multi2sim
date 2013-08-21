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

void FrmThreadBlockCreate(FrmThreadBlock *self, FrmGrid *grid)
{
	/* Initialize */
	self->grid = grid;
	self->shared_mem = mem_create();
	self->shared_mem->safe = 0;
	self->num_warps_at_barrier = 0;
}


void FrmThreadBlockDestroy(FrmThreadBlock *self)
{
	int i;

	for (i = 0; i < self->warp_count; i++)
                delete(self->warps[i]);
	free(self->warps);
	list_free(self->running_warps);
	list_free(self->finished_warps);

	for (i = 0; i < self->thread_count; i++)
                delete(self->threads[i]);
	free(self->threads);

	mem_free(self->shared_mem);
}


void FrmThreadBlockDump(FrmThreadBlock *self, FILE *f)
{
//	struct frm_grid_t *grid = self->grid;
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

