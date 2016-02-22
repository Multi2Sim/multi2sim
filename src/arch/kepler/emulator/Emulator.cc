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

Emulator::Emulator() : comm::Emulator("Kepler")
{
	// Initialize disassembler
	disassembler = Disassembler::getInstance();

	// Global memory initialization
	global_memory = misc::new_unique<mem::Memory>();
	global_memory->setSafe(false);
	global_memory_top = 0;
	global_memory_total_size = 1 << 30; /* 2GB */
	global_memory_free_size = this->global_memory_total_size;

	// Shared memory initialization
	shared_memory_total_size = 16 * (1 << 20) * 20; // 20 blocks. 16MB each

	// Constant memory initialization
	constant_memory = misc::new_unique<mem::Memory>();
	constant_memory->setSafe(false);

	max_instructions = 0x0;
	max_cycles = 0x0;
	max_functions = 0x0;
}

void Emulator::Dump(std::ostream &os) const
{
	std::cout <<"\n[ Kepler ]\nInstructions = "
			<< num_alu_instructions << std::endl;
}


void Emulator::DumpSummary(std::ostream &os)
{
	Dump();
}


bool Emulator::Run()
{
	// Stop emulation if no grids
	if (!this->grids.size())
		return false;

	// Stop if no pending grids
	if (!pending_grids.size())
		return false;

	// Remove grid and its thread blocks from pending list, and add them to
	// running list
	Grid *grid;
	std::unique_ptr<ThreadBlock> thread_block;
	int thread_block_id;
	unsigned thread_block_3d_id[3];
	while (pending_grids.size())
	{
		grid = pending_grids.front();
		pending_grids.pop_front();
		thread_block_id = 0;
		while (grid->getPendThreadBlocksize())
		{
			// Threadblock.X
			thread_block_3d_id[0] = thread_block_id /
					(grid->getThreadBlockCount3(1) *
							grid->getThreadBlockCount3(2));

			// Threadblock.Y
			thread_block_3d_id[1] = (thread_block_id %
					(grid->getThreadBlockCount3(1) *
							grid->getThreadBlockCount3(2))) /
								grid->getThreadBlockCount3(2);

			// ThreadBlock.Z
			thread_block_3d_id[2] = ((thread_block_id %
					(grid->getThreadBlockCount3(1) *
							grid->getThreadBlockCount3(2))) %
								grid->getThreadBlockCount3(2))  ;
			grid->WaitingToRunning(thread_block_id, thread_block_3d_id);
			thread_block_id ++;
			thread_block.reset(grid->getRunningThreadBlocksBegin()->release());
			while (thread_block.get()->getNumWarpsCompletedEmu()
					!= thread_block.get()->getWarpCount())
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
			thread_block.reset(); // free the memory of thread block
		}
		finished_grids.push_back(grid);
	}

	// Free finished grids
	assert(!running_grids.size() && !pending_grids.size());
	finished_grids.clear();

	// Continue emulation
	return true;
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
	// Throw an error for now if the si-sim detailed is invoked
	if (sim_kind == comm::Arch::SimDetailed)
		throw misc::Error("The detailed Kepler simulation is not currently "
				"supported in Multi2Sim.");

	// Set the path for the debug files
	isa_debug.setPath(isa_debug_file);
	isa_debug.setPrefix("[Kepler emulator]");
}

}	//namespace

