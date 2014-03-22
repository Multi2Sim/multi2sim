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
#include <memory>

#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <mem-system/Memory.h>

#include "Emu.h"
#include "Grid.h"
#include "Machine.h"
#include "Warp.h"
#include "ThreadBlock.h"

namespace Kepler
{
/*
 * Class 'Emu'
 */
std::unique_ptr<Emu> Emu::instance;

Emu *Emu::getInstance()
{
	if (instance)
		return instance.get();
	instance.reset(new Emu);
	return instance.get();
}

Emu::Emu()
{
    /* Initialize */
	this->as = as->getInstance();

#define DEFINST(_name, _fmt_str, ...) \
	inst_func[INST_##_name] = kpl_isa_##_name##_impl;
#include <arch/kepler/asm/asm.dat>
#undef DEFINST

	// Global memory initialization
	global_mem = new mem::Memory();
	global_mem->setSafe(false);
    global_mem_top = 0;
    global_mem_total_size = 1 << 30; /* 2GB */
    global_mem_free_size = this->global_mem_total_size;

	// Global memory initialization
    const_mem = new mem::Memory();
	const_mem->setSafe(false);

	alu_inst_count = 0;
	branch_inst_count = 0;
	shared_mem_inst_count = 0;
	global_mem_inst_count = 0;
}

void Emu::Dump(std::ostream &os) const
{
	std::cout <<"\n[ Kepler ]\nInstructions = "
			<< alu_inst_count << std::endl;
}


void Emu::DumpSummary(std::ostream &os)
{
	/* Call parent */
	Dump();
}


bool Emu::Run()
{

	Grid *grid;
	std::unique_ptr<ThreadBlock> thread_block;
	int thread_block_id;

	// Stop emulation if no grids
	if (!this->grids.size())
		return FALSE;

	// Remove grid and its thread blocks from pending list, and add them to
	 // running list
	while (pending_grids.size())
	{
		grid = pending_grids.front();
		pending_grids.pop_front();
		thread_block_id = 0;

		while (grid->getPendThreadBlocksize())
		{
			grid->WaitingToRunning(thread_block_id);
			thread_block_id ++;
			thread_block.reset(grid->getRunningThreadBlocksBegin()->release());
			while (thread_block.get()->getWarpsCompletedEmu() != thread_block.
					get()->getWarpsInWorkgroup())
			{
				for (auto wp_p = thread_block.get()->WarpsBegin(); wp_p <
					thread_block.get()->WarpsEnd(); ++wp_p)
				{
					if ((*wp_p)->getFinishedEmu() || (*wp_p)->getAtBarrier())
						continue;
					(*wp_p)->Execute();
				}
			}
			thread_block.get()->setFinishedEmu(true);
			grid->PopRunningThreadBlock();
			grid->PushFinishedThreadBlock
				(std::move(std::unique_ptr<ThreadBlock>(thread_block.release())));
		}
		finished_grids.push_back(grid);
	}

	// Free finished grids
	assert(!running_grids.size() && !pending_grids.size());
	finished_grids.clear();

	/* Continue emulation */
	return TRUE;
}

// push back an element in grids list
void Emu::GridsPushBack(Grid * grid)
{
	grids.push_back(grid);
}

void Emu::ReadConstMem(unsigned addr, unsigned size, char *buf)
{
	const_mem->Read(addr, size, buf);
}

void Emu::ReadGlobalMem(unsigned addr, unsigned size, char *buf)
{
	global_mem->Read(addr, size, buf);
}

void Emu::WriteGlobalMem(unsigned addr, unsigned size, const char *buf)
{
	global_mem->Write(addr, size, buf);
}



void Emu::WriteConstMem(unsigned addr, unsigned size, const char *buf)
{
	const_mem->Write(addr, size, buf);
}

void Emu::PushPendingGrid(Grid *grid)
{
	pending_grids.push_back(grid);
}

}	//namespace

