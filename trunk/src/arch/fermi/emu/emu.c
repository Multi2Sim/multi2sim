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
#include <lib/util/misc.h>
#include <memory/memory.h>

#include "emu.h"
#include "grid.h"
#include "machine.h"
#include "thread-block.h"
#include "warp.h"


/*
 * Class 'FrmEmu'
 */

void FrmEmuCreate(FrmEmu *self, struct FrmAsmWrap *as)
{
	/* Parent */
	EmuCreate(asEmu(self), "Fermi");

	/* Initialize */
	self->as = as;
	self->grids = list_create();
	self->pending_grids = list_create();
	self->running_grids = list_create();
	self->finished_grids = list_create();
#define DEFINST(_op, _fmt_str, _opcode) \
		self->inst_func[FrmInstId##_op] = frm_isa_##_op##_impl;
#include <arch/fermi/asm/asm.dat>
#undef DEFINST
	self->global_mem = mem_create();
	self->global_mem->safe = 0;
	self->global_mem_top = 0;
	self->total_global_mem_size = 1 << 30; /* 1GB */
	self->free_global_mem_size = self->total_global_mem_size;
	self->const_mem = mem_create();
	self->const_mem->safe = 0;

	/* Virtual functions */
	asObject(self)->Dump = FrmEmuDump;
	asEmu(self)->DumpSummary = FrmEmuDumpSummary;
	asEmu(self)->Run = FrmEmuRun;
}


void FrmEmuDestroy(FrmEmu *self)
{
	list_free(self->grids);
	list_free(self->pending_grids);
	list_free(self->running_grids);
	list_free(self->finished_grids);
	mem_free(self->global_mem);
	mem_free(self->const_mem);
}


void FrmEmuDump(Object *self, FILE *f)
{
	EmuDump(self, f);
}


void FrmEmuDumpSummary(Emu *self, FILE *f)
{
	EmuDumpSummary(self, f);
}


int FrmEmuRun(Emu *self)
{
	FrmEmu *emu = asFrmEmu(self);

	FrmGrid *grid;
	FrmThreadBlock *thread_block;
	FrmWarp *warp;

	int thread_block_id, warp_id;

	/* Stop emulation if no grids */
	if (! list_count(emu->grids))
		return FALSE;

	/* Remove all pending grids and their thread-blocks from the pending list,
	 * and add them to running list */
	while (list_count(emu->pending_grids))
	{
		grid = list_dequeue(emu->pending_grids);
		while (list_count(grid->pending_thread_blocks))
		{
			thread_block_id = (long) list_dequeue(grid->pending_thread_blocks);
			list_enqueue(grid->running_thread_blocks,
					(void *)((long)thread_block_id));
		}
		list_enqueue(emu->running_grids, grid);
	}

	/* Run one instruction in the whole grid */
	while (list_count(emu->running_grids))
	{
		grid = list_dequeue(emu->running_grids);
		while (list_count(grid->running_thread_blocks))
		{
			thread_block_id = (long) list_dequeue(grid->running_thread_blocks);
			thread_block = new(FrmThreadBlock, thread_block_id, grid);
			while (! thread_block->finished)
			{
				for (warp_id = 0; warp_id <	list_count(thread_block->
						running_warps); ++warp_id)
				{
					warp = list_get(thread_block->running_warps, warp_id);
					if (warp->finished)
						continue;
					FrmWarpExecute(warp);
					if (warp->at_barrier)
					{
						thread_block->at_barrier_warp_count++;
						continue;
					}
				}

				/* Continue execution when all the warps in a thread-block reach
				 * a barrier*/
				if (thread_block->at_barrier_warp_count ==
						thread_block->warp_count)
				{
					for (warp_id = 0; warp_id < thread_block->warp_count;
							warp_id++)
						thread_block->warps[warp_id]->at_barrier = 0;

					thread_block->at_barrier_warp_count = 0;
				}

				if (list_count(thread_block->finished_warps) == thread_block->
						warp_count)
				{
					list_enqueue(grid->finished_thread_blocks, (void *)
							((long) thread_block_id));
					thread_block->finished = 1;
				}
			}
			delete(thread_block);
		}
		list_enqueue(emu->finished_grids, grid);
	}

	/* Free finished grids */
	assert(!list_count(emu->pending_grids) && !list_count(emu->running_grids));
	while (list_count(emu->finished_grids))
	{
		grid = list_dequeue(emu->finished_grids);
		delete(grid);
	}

	/* Continue emulation */
	return TRUE;
}


/*
 * Non-Class Stuff
 */

long long frm_emu_max_cycles;
long long frm_emu_max_inst;
int frm_emu_max_functions;

const int frm_emu_warp_size = 32;
