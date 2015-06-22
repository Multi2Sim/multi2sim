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

#include <arch/common/Arch.h>
#include <lib/cpp/CommandLine.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <memory/Memory.h>

#include "Emulator.h"
#include "Grid.h"
#include "Thread.h"
#include "Warp.h"
#include "ThreadBlock.h"


namespace Kepler
{
//
// Configuration options
//

// Debugger file
std::string Emulator::isa_debug_file;

// Simulation kind
comm::Arch::SimKind Emulator::sim_kind = comm::Arch::SimFunctional;



//
// Static variables
//

// Debugger
misc::Debug Emulator::isa_debug;

std::unique_ptr<Emulator> Emulator::instance;

Emulator *Emulator::getInstance()
{
	if (instance)
		return instance.get();
	instance.reset(new Emulator());
	return instance.get();
}

Emulator::Emulator() : comm::Emulator("kpl")
{
    // Initialize disassembler
	this->as = as->getInstance();

	// Global memory initialization
	global_mem.reset(new mem::Memory());
	global_mem->setSafe(false);
	global_mem_top = 0;
	global_mem_total_size = 1 << 30; /* 2GB */
	global_mem_free_size = this->global_mem_total_size;

	// Constant memory initialization
    const_mem.reset(new mem::Memory());
	const_mem->setSafe(false);

	emu_max_inst = 0xffffffff;
	emu_max_cycles = 0xffffffff;
	emu_max_functions = 0xffffffff;
}

void Emulator::Dump(std::ostream &os) const
{
	std::cout <<"\n[ Kepler ]\nInstructions = "
			<< alu_inst_count << std::endl;
}


void Emulator::DumpSummary(std::ostream &os)
{
	// Call parent
	Dump();
}


bool Emulator::Run()
{

	Grid *grid;
	std::unique_ptr<ThreadBlock> thread_block;
	int thread_block_id;

	// Stop emulation if no grids
	if (!this->grids.size())
		return FALSE;

	// Stop if no pending grids
	if (!pending_grids.size())
		return false;

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

			while (thread_block.get()->getWarpsCompletedEmuCount()
					!= thread_block.get()->getWarpCount())
			{

				for (auto wp_p = thread_block.get()->WarpsBegin(); wp_p <
					thread_block.get()->WarpsEnd(); ++wp_p)
				{
					if ((*wp_p)->getFinishedEmu() || (*wp_p)->getAtBarrier())
					{
						continue;
					}
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

	// Continue emulation
	return true;
}

void Emulator::ReadConstMem(unsigned addr, unsigned size, char *buf)
{
	const_mem->Read(addr, size, buf);
}

void Emulator::ReadGlobalMem(unsigned addr, unsigned size, char *buf)
{
	global_mem->Read(addr, size, buf);
}

void Emulator::WriteGlobalMem(unsigned addr, unsigned size, const char *buf)
{
	global_mem->Write(addr, size, buf);
}


void Emulator::WriteConstMem(unsigned addr, unsigned size, const char *buf)
{
	const_mem->Write(addr, size, buf);
}


void Emulator::PushPendingGrid(Grid *grid)
{
	pending_grids.push_back(grid);
}


Grid *Emulator::addGrid(Function *function)
{
	// Create the grid and add it to the grid list
	grids.emplace_back(new Grid(function));
	return grids.back().get();
}


void Emulator::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Kepler");

	// Option --kpl-sim <kind>
	command_line->RegisterEnum("--kpl-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::Arch::SimKindMap,
			"Level of accuracy of kepler simulation");

	// Option --kpl-debug-isa <kind>
	command_line->RegisterString("--kpl-debug-isa <file>",isa_debug_file,
			"Dump debug information about Kepler isa implementation");
}


void Emulator::ProcessOptions()
{
	isa_debug.setPath(isa_debug_file);
	isa_debug.setPrefix("[Kepler emulator]");
}

}	//namespace

