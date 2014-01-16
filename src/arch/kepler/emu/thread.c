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


#include "emu.h"
#include "grid.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"


/*
 * Public Functions
 */

void KplThreadCreate(KplThread *self, int id, KplWarp *warp)
{
	int i;

	/* Initialization */
	self->id = id + warp->thread_block->id * warp->thread_block->thread_count;
	self->id_in_warp = id % kpl_emu_warp_size;
	self->warp = warp;
	self->thread_block = warp->thread_block;
	self->grid = warp->thread_block->grid;

	/* General purpose registers */
	for (i = 0; i < 64; ++i)
		self->gpr[i].u32 = 0;

	/* Special registers */
	for (i = 0; i < 82; ++i)
		self->sr[i].u32 = 0;
	self->sr[33].u32 = id % self->grid->thread_block_size3[0];
	self->sr[34].u32 = (id / self->grid->thread_block_size3[0]) %
			self->grid->thread_block_size3[1];
	self->sr[35].u32 = id / (self->grid->thread_block_size3[0] *
			self->grid->thread_block_size3[1]);
	self->sr[37].u32 = self->thread_block->id %
			self->grid->thread_block_count3[0];
	self->sr[38].u32 = (self->thread_block->id /
			self->grid->thread_block_count3[0]) %
			self->grid->thread_block_count3[1];
	self->sr[39].u32 = self->thread_block->id /
			(self->grid->thread_block_count3[0] *
					self->grid->thread_block_count3[1]);

	/* Predicate registers */
	for (i = 0; i < 7; ++i)
		self->pr[i] = 0;
	self->pr[7] = 1;

	/* Add thread to warp */
	warp->threads[self->id_in_warp] = self;
}


void KplThreadDestroy(KplThread *self)
{
}

