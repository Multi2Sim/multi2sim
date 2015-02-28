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
#include <memory/memory.h>

#include "emu.h"
#include "grid.h"
#include "thread-block.h"
#include "thread.h"
#include "warp.h"


/*
 * Public Functions
 */

void FrmThreadBlockCreate(FrmThreadBlock *self, int id, FrmGrid *grid)
{
	FrmWarp *warp;
	FrmThread *thread;
	int i;

	/* Initialization */
	self->id = id;
	self->grid = grid;

	/* Create warps */
	self->warp_count = (grid->thread_block_size + frm_emu_warp_size - 1) /
			frm_emu_warp_size;
	self->warps = xcalloc(self->warp_count, sizeof(FrmWarp *));
	self->running_warps = list_create();
	self->finished_warps = list_create();
	for (i = 0; i < self->warp_count; ++i)
	{
		warp = new(FrmWarp, i, self, grid);
		self->warps[i] = warp;
		list_add(self->running_warps, warp);
	}

	/* Create threads */
	self->thread_count = grid->thread_block_size;
	self->threads = xcalloc(self->thread_count, sizeof(FrmThread *));
	for (i = 0; i < self->thread_count; ++i)
	{
		thread = new(FrmThread, i, self->warps[i / frm_emu_warp_size]);
		self->threads[i] = thread;
	}

	/* Create shared memory */
	self->shared_mem = mem_create();
	self->shared_mem->safe = 0;

	/* Flags */
	self->finished = 0;
}


void FrmThreadBlockDestroy(FrmThreadBlock *self)
{
	int i;

	mem_free(self->shared_mem);

	for (i = 0; i < self->warp_count; i++)
		delete(self->warps[i]);
	list_free(self->finished_warps);
	list_free(self->running_warps);
	free(self->warps);

	for (i = 0; i < self->thread_count; i++)
		delete(self->threads[i]);
	free(self->threads);
}


void FrmThreadBlockDump(FrmThreadBlock *self, FILE *f)
{
}

