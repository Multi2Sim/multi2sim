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
#include "Grid.h"
#include "isa.h"


void Grid::Grid(KplEmu *emu, struct cuda_function_t *function)
{
	/* Initialization */
	this->emu = emu;
	this->id = list_count(emu->grids);
	this->function = function;
	this->num_gpr = function->num_gpr;

	/* Add to list */
	list_add(emu->grids, this);
}

void Grid::Dump(FILE *f)
{
}


/* Setup sizes of a grid. Used by driver. */
void Grid::SetupSize(unsigned *thread_block_count,
		unsigned *thread_block_size)
{
	int i;

	/* Setup thread-block counts */
	for (i = 0; i < 3; i++)
		this->thread_block_count3[i] = thread_block_count[i];
	this->thread_block_count = this->thread_block_count3[0] *
			this->thread_block_count3[1] * this->thread_block_count3[2];

	/* Setup thread-block sizes */
	for (i = 0; i < 3; i++)
		this->thread_block_size3[i] = thread_block_size[i];
	this->thread_block_size = this->thread_block_size3[0] *
			this->thread_block_size3[1] * this->thread_block_size3[2];

	/* Calculate thread counts */
	for (i = 0; i < 3; i++)
		this->thread_count3[i] = thread_block_count[i] * thread_block_size[i];
	this->thread_count = this->thread_count3[0] * this->thread_count3[1] *
			this->thread_count3[2];

	/* Create lists */
	this->pending_thread_blocks = list_create();
	for (i = 0; i < this->thread_block_count; ++i)
		list_enqueue(this->pending_thread_blocks, (void *)((long)i));
	this->running_thread_blocks = list_create();
	this->finished_thread_blocks = list_create();

	/* Debug */
	kpl_isa_debug("%s:%d: block count = (%d,%d,%d)\n",
			__FILE__, __LINE__, this->thread_block_count3[0],
			this->thread_block_count3[1],
			this->thread_block_count3[2]);
	kpl_isa_debug("%s:%d: block size = (%d,%d,%d)\n",
			__FILE__, __LINE__, this->thread_block_size3[0],
			this->thread_block_size3[1],
			this->thread_block_size3[2]);
	kpl_isa_debug("%s:%d: grid size = (%d,%d,%d)\n",
			__FILE__, __LINE__, this->thread_count3[0],
			this->thread_count3[1], this->thread_count3[2]);
}


/* Write initial values into constant memory. Used by driver. */
void Grid::SetupConstantMemory()
{
	KplEmuConstMemWrite(this->emu, 0x8, this->thread_block_size3);
}

