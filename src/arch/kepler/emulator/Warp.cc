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


//#include <driver/cuda/function.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/hash-table.h>
#include <arch/kepler/disassembler/Disassembler.h>

#include "Emulator.h"
#include "Grid.h"
#include "Thread.h"
#include "ThreadBlock.h"
#include "Warp.h"


namespace Kepler
{

Warp::Warp(ThreadBlock *thread_block, unsigned id):inst(Instruction())
{

	unsigned am = 0;

	// Initialization
	// Calculate the warp ID in grid
	this->id = id + thread_block->getId() * thread_block->getWarpCount();

	// Get the ID in thread block
	id_in_thread_block = id;

	// Get the Grid it belongs to
	grid = thread_block->getGrid();

	// Get the thread block it belongs to
	this->thread_block = thread_block;

	int thread_block_size = thread_block->getGrid()->getThreadBlockSize();

	// Get threads from grid
	if (thread_block_size <= (int)warp_size)
		thread_count = thread_block_size;
	else if (id < thread_block->getWarpCount() - 1)
		thread_count = warp_size;
	else
		thread_count = grid->getThreadBlockSize() -
		(thread_block->getWarpCount() - 1) * warp_size;

	// Return Address Stack
	return_stack = std::unique_ptr<ReturnAddressStack>(new ReturnAddressStack());
	// SyncStack
	std::unique_ptr<SyncStack> sync_stack(new SyncStack(thread_count));

	// Initialize active mask
	if (thread_count == warp_size)
            am = unsigned(-1);
	else
            am = (1u << thread_count) - 1;

	// Push the default sync stack for main kernel function.
	return_stack->push(0, am, sync_stack);

	this->getSyncStack()->get()->setActiveMask(am);

	// Function name
	kernel_function_name = grid->getKernelFunctionName();

	// Instruction
	pc = 0;
	target_pc = 0;
	inst_size = 8;
	instruction_buffer = grid->getInstructionBuffer();
	instruction_buffer_size = grid->getInstructionBufferSize();


	// Reset flags
	at_barrier_thread_count = 0;
	at_barrier = 0;
	finished_thread = 0;
	finished_emu = false;

	// simulation performance
	emu_inst_count = 0;
	emu_time_start = 0;
	emu_time_end = 0;

	// Statistics
	inst_count = 0;
	num_global_memory_instructions = 0;
	num_shared_memory_instructions = 0;
}


void Warp::Dump(std::ostream &os) const
{
}


void Warp::Execute()
{
	// Get emu instance
	Emulator *emulator = Emulator::getInstance();

	// Instruction binary
	Instruction::Bytes inst_bytes;

	// Instruction opcode
	Instruction::Opcode inst_op;

	// Read instruction binary
	inst_bytes.as_uint[0] = instruction_buffer[pc / inst_size] >> 32;
	inst_bytes.as_uint[1] = instruction_buffer[pc / inst_size];

	// Decode instruction
	if( pc % 64)
	{
			inst.Decode((const char *) &inst_bytes, pc);

			// Execute instruction
			inst_op = (Instruction::Opcode) inst.getOpcode();

			if (!inst_op)
			{
				std::cerr << __FILE__ << ":" << __LINE__ << ": unrecognized instruction "
					<< std::hex << " pc " << pc << std::endl;
				misc::Panic("Simulation exits with exception.\n");
			}

			for (auto thread_id = threads_begin; thread_id < threads_end; ++thread_id)
			{
				/*
				if (thread_id->get()->getId() == 0)
				std::cerr << inst.getName() << " id " << thread_id->get()->getId()
						<< " warp_id " << id << " thread count " << this->thread_count
						<< " block id " << thread_block->getId()
						<< " warp count " << thread_block->getWarpCount()
						<< " pc " << pc << std::endl;
						*/
				thread_id->get()->Execute(inst_op,&inst);
			}
	}
	else
	{
			for (auto thread_id = threads_begin; thread_id < threads_end; ++thread_id)
			{
				thread_id->get()->ExecuteSpecial();
			}
	}

	// Finish
	if (finished_emu)
	{

		thread_block->incWarpsCompletedEmu();

		// Check if thread block finished kernel execution
		if (thread_block->getNumWarpsCompletedEmu() ==
				thread_block->getWarpCount())
		{
			thread_block->setFinishedEmu(true);
		}
		return;

	}

    inst_count++;
    emu_inst_count++;
    emulator->incNumAluInstructions();
    pc = this->target_pc;

    if(pc >= instruction_buffer_size - 8)
    {
    	finished_emu = true;
    	thread_block->incWarpsCompletedEmu();
    }
	// Stats

	//asEmu(emu)->instructions++; // no parent class any more
}


unsigned Warp::getFinishedThreadCount() const
{
        unsigned num = 0;

        for (unsigned i = 0; i < thread_count; i++)
                num += (finished_thread >> i) & 1u;

        assert(num <= thread_count);

        return num;
}



} //namespace
