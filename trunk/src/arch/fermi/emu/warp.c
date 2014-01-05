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
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/bit-map.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "machine.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"


/*
 * Public Functions
 */

void FrmWarpCreate(FrmWarp *self, int id, FrmThreadBlock *thread_block,
		FrmGrid *grid)
{
	FrmEmu *emu = grid->emu;

	/* Initialization */
	self->id = id + thread_block->id * thread_block->warp_count;
	self->id_in_thread_block = id;
	self->grid = grid;
	self->thread_block = thread_block;

	/* Allocate threads */
	if (id < thread_block->warp_count - 1)
		self->thread_count = frm_emu_warp_size;
	else
		self->thread_count = grid->thread_block_size -
		(thread_block->warp_count - 1) * frm_emu_warp_size;
	self->threads = (FrmThread **) xcalloc(self->thread_count,
			sizeof(FrmThread *));

	/* Predicate mask */
	self->pred = bit_map_create(frm_emu_warp_size);

	/* Instruction */
	self->inst = FrmInstWrapCreate(emu->as);
	self->inst_size = 8;
	self->inst_buffer = grid->function->inst_bin;
	self->inst_buffer_size = grid->function->inst_bin_size;

	/* Sync stack */
	self->sync_stack.entries[0].active_thread_mask = ((unsigned long long)1 <<
			self->thread_count) - 1;
	self->sync_stack_top = 0;

	/* Reset flags */
	self->at_barrier = 0;
}

void FrmWarpDestroy(FrmWarp *self)
{
	free(self->threads);
	bit_map_free(self->pred);
	FrmInstWrapFree(self->inst);
}

void FrmWarpDump(FrmWarp *self, FILE *f)
{
}

void FrmWarpExecute(FrmWarp *self)
{
	FrmEmu *emu;
	FrmGrid *grid;
	FrmThreadBlock *thread_block;
	FrmThread *thread;
	struct FrmInstWrap *inst;

	FrmInstBytes inst_bytes;
	FrmInstId inst_id;
	int thread_id;

	/* Get current arch, grid, and thread-block */
	thread_block = self->thread_block;
	grid = thread_block->grid;
	emu = grid->emu;

	/* Get instruction */
	inst_bytes.word[0] = self->inst_buffer[self->pc / self->inst_size] >> 32;
	inst_bytes.word[1] = self->inst_buffer[self->pc / self->inst_size];
	frm_isa_debug("%s:%d: warp[%d] executes instruction [0x%x] 0x%016llx\n",
			__FILE__, __LINE__, self->id, self->pc, inst_bytes.dword);

	/* Decode instruction */
	inst = self->inst;
	FrmInstWrapDecode(inst, self->pc, &inst_bytes);

	/* Execute instruction */
	inst_id = FrmInstWrapGetId(inst);
	if (!inst_id)
		fatal("%s:%d: unrecognized instruction (%08x %08x)",
				__FILE__, __LINE__, inst_bytes.word[0], inst_bytes.word[1]);
	for (thread_id = 0; thread_id < self->thread_count; ++thread_id)
	{
		thread = self->threads[thread_id];
		emu->inst_func[inst_id](thread, inst);
	}

	/* Finish */
	if (self->finished)
	{
		/* Check if warp finished kernel execution */
		assert(list_index_of(thread_block->running_warps, self) != -1);
		assert(list_index_of(thread_block->finished_warps, self) == -1);
		list_remove(thread_block->running_warps, self);
		list_add(thread_block->finished_warps, self);

		/* Check if thread-block finished kernel execution */
		if (list_count(thread_block->finished_warps) ==
				thread_block->warp_count)
		{
			assert(list_index_of(grid->running_thread_blocks, thread_block) !=
					-1);
			assert(list_index_of(grid->finished_thread_blocks, thread_block) ==
					-1);
			list_remove(grid->running_thread_blocks, thread_block);
			list_add(grid->finished_thread_blocks, thread_block);

			/* Check if grid finished kernel execution */
			if (list_count(grid->finished_thread_blocks) == 
					grid->thread_block_count)
			{
				assert(list_index_of(emu->running_grids, grid) != -1);
				assert(list_index_of(emu->finished_grids, grid) == -1);
				list_remove(emu->running_grids, grid);
				list_add(emu->finished_grids, grid);
			}
		}
	}

	/* Increment the PC */
	self->pc += self->inst_size;

	/* Stats */
	asEmu(emu)->instructions++;
	self->inst_count++;
}
