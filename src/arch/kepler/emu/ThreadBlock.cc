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

#include "ThreadBlock.h"
#include "Thread.h"
#include "Warp.h"


/*
 * Public Functions
 */

void KplThreadBlockCreate(KplThreadBlock *self, KplGrid *grid)
{
	/* Initialize */
	self->grid = grid;
	self->shared_mem = mem_create();
	self->shared_mem->safe = 0;
	self->num_warps_at_barrier = 0;
}


void KplThreadBlockDestroy(KplThreadBlock *self)
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


