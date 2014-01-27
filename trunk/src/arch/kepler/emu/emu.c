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
#include <mem-system/memory.h>

#include "emu.h"
#include "grid.h"
#include "machine.h"
#include "warp.h"
#include "thread-block.h"

/*
 * Class 'KplEmu'
 */

void KplEmuCreate(KplEmu *self, struct KplAsm *as)
{
	/* Parent */
	EmuCreate(asEmu(self), "Kepler");

        /* Initialize */
	self->as = as;
	self->grids = list_create();
	self->pending_grids = list_create();
	self->running_grids = list_create();
	self->finished_grids = list_create();
#define DEFINST(_name, _fmt_str, ...) \
		self->inst_func[INST_##_name] = kpl_isa_##_name##_impl;
#include <arch/kepler/asm/asm.dat>
#undef DEFINST
	self->global_mem = mem_create();
        self->global_mem->safe = 0;
        self->global_mem_top = 0;
        self->total_global_mem_size = 1 << 30; /* 2GB */
        self->free_global_mem_size = self->total_global_mem_size;
        self->const_mem = mem_create();
        self->const_mem->safe = 0;

        /* Virtual functions */
        asObject(self)->Dump = KplEmuDump;
        asEmu(self)->DumpSummary = KplEmuDumpSummary;
        asEmu(self)->Run = KplEmuRun;
}


void KplEmuDestroy(KplEmu *self)
{
	list_free(self->grids);
	list_free(self->pending_grids);
	list_free(self->running_grids);
 	list_free(self->finished_grids);
	mem_free(self->global_mem);
        mem_free(self->const_mem);
}


void KplEmuDump(Object *self, FILE *f)
{
	EmuDump(self, f);
}


void KplEmuDumpSummary(Emu *self, FILE *f)
{
	/* Call parent */
	EmuDumpSummary(self, f);
}


int KplEmuRun(Emu *self)
{
	KplEmu *emu = asKplEmu(self);

	KplGrid *grid;
	KplThreadBlock *thread_block;
	KplWarp *warp;

	int thread_block_id, warp_id;

	/* Stop emulation if no grids */
	if (!list_count(emu->grids))
		return FALSE;

	/* Remove grid and its thread blocks from pending list, and add them to
	 * running list */
	while ((list_count(emu->pending_grids)))
	{
		grid = list_dequeue(emu->pending_grids);
		while (list_count(grid->pending_thread_blocks))
		{
			thread_block_id = (long) list_dequeue(grid->running_thread_blocks);
			thread_block = new(KplThreadBlock, thread_block_id, grid);
			while (!thread_block->finished)
			{
				for (warp_id = 0; warp_id <	list_count(thread_block->
						running_warps); ++warp_id)
				{
					warp = list_get(thread_block->running_warps, warp_id);
					if (warp->finished || warp->at_barrier)
						continue;
					KplWarpExecute(warp);
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


void KplEmuConstMemWrite(KplEmu *self, unsigned addr, void *value_ptr)
{
	mem_write(self->const_mem, addr, sizeof(unsigned), value_ptr);
}


void KplEmuConstMemRead(KplEmu *self, unsigned addr, void *value_ptr)
{
	mem_read(self->const_mem, addr, sizeof(unsigned), value_ptr);
}


/*
 * Non-Class Stuff
 */

long long kpl_emu_max_cycles;
long long kpl_emu_max_inst;
int kpl_emu_max_functions;

const int kpl_emu_warp_size = 32;

