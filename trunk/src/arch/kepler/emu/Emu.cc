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

	grids = new std::list<Grid*>;
	pending_grids = new std::list<Grid*>;
	running_grids = new std::list<Grid*>;
	finished_grids = new std::list<Grid*>;

#define DEFINST(_name, _fmt_str, ...) \
	inst_func[INST_##_name] = kpl_isa_##_name##_impl;
#include <arch/kepler/asm/asm.dat>
#undef DEFINST
	global_mem = new Memory::Memory();
    global_mem->safe = false;
    global_mem_top = 0;
    global_mem_total_size = 1 << 30; /* 2GB */
    global_mem_free_size = this->global_mem_total_size;
    const_mem = new Memory::Memory();
    const_mem->safe = false;
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

	Grid *grid;
	Warp *warp;
	std::list<std::unique_ptr<ThreadBlock>>::iterator thread_block;

	// Stop emulation if no grids
	if (!this->grids.size())
		return FALSE;

	// Remove grid and its thread blocks from pending list, and add them to
	 // running list
	while (pending_grids.size())
	{
		grid = pending_grids.front();
		pending_grids.pop_front();

		while (grid->getPendThreadBlocksize())
		{
			grid->WaitingToRunning();
			thread_block = grid->getRunningThreadBlocksBegin();
			while (thread_block->getWarpsCompletedEmu() != thread_block->
					getWarpsInWorkgroup())
			{
				for (auto wp_p = thread_block->WarpsBegin(); wp_p <	thread_block->
						WarpsEnd(); ++wp_p)
				{
					if ((*wp_p)->finished_emu || (*wp_p)->at_barrier)
						continue;
					(*wp_p)->Execute();
				}
			}
			thread_block->setFinishedEmu(true);
			grid->finished_thread_blocks.push_back(thread_block);
		}
		finished_grids.push_back(grid);
	}

	// Free finished grids
	assert(!grid->getPendThreadBlocksize() && !grid->getRunThreadBlocksize());
	finished_grids.clear();

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

