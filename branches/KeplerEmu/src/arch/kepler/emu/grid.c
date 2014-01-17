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


#include <driver/cuda/function.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"


void KplGridCreate(KplGrid *self, KplEmu *emu, struct cuda_function_t *function)
{
	/* Initialization */
	self->emu = emu;
	self->id = list_count(emu->grids);
	self->function = function;
	self->num_gpr = function->num_gpr;

	/* Add to list */
	list_add(emu->grids, self);
}


void KplGridDestroy(KplGrid *self)
{
	KplEmu *emu = self->emu;

	/* Run free notify call-back */
	if (self->free_notify_func)
		self->free_notify_func(self->free_notify_data);

        /* Free thread_blocks */
	list_free(self->pending_thread_blocks);
	list_free(self->running_thread_blocks);
	list_free(self->finished_thread_blocks);

	/* Remove from lists */
	list_remove(emu->pending_grids, self);
	list_remove(emu->running_grids, self);
	list_remove(emu->finished_grids, self);
	list_remove(emu->grids, self);
}

void KplGridDump(KplGrid *self, FILE *f)
{
}


/* Setup sizes of a grid. Used by driver. */
void KplGridSetupSize(KplGrid *self, unsigned *thread_block_count,
		unsigned *thread_block_size)
{
	int i;

	/* Setup thread-block counts */
	for (i = 0; i < 3; i++)
		self->thread_block_count3[i] = thread_block_count[i];
	self->thread_block_count = self->thread_block_count3[0] *
			self->thread_block_count3[1] * self->thread_block_count3[2];

	/* Setup thread-block sizes */
	for (i = 0; i < 3; i++)
		self->thread_block_size3[i] = thread_block_size[i];
	self->thread_block_size = self->thread_block_size3[0] *
			self->thread_block_size3[1] * self->thread_block_size3[2];

	/* Calculate thread counts */
	for (i = 0; i < 3; i++)
		self->thread_count3[i] = thread_block_count[i] * thread_block_size[i];
	self->thread_count = self->thread_count3[0] * self->thread_count3[1] *
			self->thread_count3[2];

	/* Create lists */
	self->pending_thread_blocks = list_create();
	for (i = 0; i < self->thread_block_count; ++i)
		list_enqueue(self->pending_thread_blocks, (void *)((long)i));
	self->running_thread_blocks = list_create();
	self->finished_thread_blocks = list_create();

	/* Debug */
	kpl_isa_debug("%s:%d: block count = (%d,%d,%d)\n", 
			__FILE__, __LINE__, self->thread_block_count3[0],
			self->thread_block_count3[1],
			self->thread_block_count3[2]);
	kpl_isa_debug("%s:%d: block size = (%d,%d,%d)\n", 
			__FILE__, __LINE__, self->thread_block_size3[0],
			self->thread_block_size3[1],
			self->thread_block_size3[2]);
	kpl_isa_debug("%s:%d: grid size = (%d,%d,%d)\n", 
			__FILE__, __LINE__, self->thread_count3[0],
			self->thread_count3[1], self->thread_count3[2]);
}


/* Write initial values into constant memory. Used by driver. */
void KplGridSetupConstantMemory(KplGrid *self)
{
	KplEmuConstMemWrite(self->emu, 0x8, self->thread_block_size3);
}

