/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
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

#include <iostream>
#include <list>

#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "Emu.h"
#include "Grid.h"
#include "machine.h"
#include "Warp.h"
#include "ThreadBlock.h"

namespace Kepler
{
/*
 * Class 'Emu'
 */

Emu::Emu(Asm *as)
{
    /* Initialize */
	this->as = as;

	this->grids = new std::list<Grid*>;
	this->pending_grids = new std::list<Grid*>;
	this->running_grids = new std::list<Grid*>;
	this->finished_grids = new std::list<Grid*>;

#define DEFINST(_name, _fmt_str, ...) \
	this->inst_func[INST_##_name] = kpl_isa_##_name##_impl;
#include <arch/kepler/asm/asm.dat>
#undef DEFINST
	this->global_mem = new Memory::Memory();
    this->global_mem->safe = false;
    this->global_mem_top = 0;
    this->global_mem_total_size = 1 << 30; /* 2GB */
    this->global_mem_free_size = this->global_mem_total_size;
    this->const_mem = new Memory::Memory();
    this->const_mem->safe = false;
/*

        asObject(this)->Dump = KplEmuDump;
        asEmu(this)->DumpSummary = KplEmuDumpSummary;
        asEmu(this)->Run = KplEmuRun;
*/
}

void Emu::Dump(std::ostream &os = std::cout) const
{

}


void Emu::DumpSummary(std::ostream &os = std::cout)
{
	/* Call parent */

}


bool Emu::Run()
{
	/*
	Grid *grid;
	ThreadBlock *thread_block;
	Warp *warp;

	int thread_block_id, warp_id;

	// Stop emulation if no grids
	if (!this->grids.size())
		return FALSE;

	// Remove grid and its thread blocks from pending list, and add them to
	 // running list
	while (this->pending_grids.size())
	{
		grid = this->pending_grids.pop_front();
		while (grid->pending_thread_blocks.size())
		{
			thread_block_id = (long) grid->running_thread_blocks.pop_back();
			thread_block = new ThreadBlock(grid);
			while (!thread_block->finished_emu)
			{
				for (warp_id = 0; warp_id <	list_count(thread_block->
						running_warps); ++warp_id)
				{
					warp = list_get(thread_block->running_warps, warp_id);
					if (warp->finished_emu || warp->at_barrier)
						continue;
					warp->Execute();
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

	// Free finished grids
	assert(!list_count(emu->pending_grids) && !list_count(emu->running_grids));
	while (list_count(emu->finished_grids))
	{
		grid = list_dequeue(emu->finished_grids);
		delete(grid);
	}
*/
	/* Continue emulation */
	return TRUE;
}

/*
 * Non-Class Stuff
 */
/*
long long kpl_emu_max_cycles;
long long kpl_emu_max_inst;
int kpl_emu_max_functions;

const int kpl_emu_warp_size = 32;
*/
}

