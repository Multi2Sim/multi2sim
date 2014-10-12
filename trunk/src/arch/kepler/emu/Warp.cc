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
#include <arch/kepler/asm/Asm.h>

#include "Emu.h"
#include "Grid.h"
#include "Machine.h"
#include "Thread.h"
#include "ThreadBlock.h"
#include "Warp.h"


namespace Kepler
{

Warp::Warp(ThreadBlock *thread_block, unsigned id):inst(Inst())
{

	// Initialization
	// Calculate the warp ID in grid
	this->id = id + thread_block->getId() * thread_block->getWarpCount();

	// Get the ID in thread block
	id_in_thread_block = id;

	// Get the Grid it belongs to
	grid = thread_block->getGrid();

	// Get the thread block it belongs to
	this->thread_block = thread_block;

	// Get threads from grid
	if (id < thread_block->getWarpCount() - 1)
		thread_count = warp_size;
	else
		thread_count = grid->getThreadBlockSize() -
		(thread_block->getWarpCount() - 1) * warp_size;

	// SyncStack
	sync_stack = std::unique_ptr<SyncStack>(new SyncStack());

	// Instruction
	pc = 0;
	inst_size = 8;
	inst_buffer = grid->getInstBuffer();
	inst_buffer_size = grid->getInstBufferSize();

	// Initialize active mask
	if (thread_count == warp_size)
            active_mask = unsigned(-1);
	else
		active_mask = (1u << thread_count) - 1;

	// Initialize temp_entry
	resetTempMask();

	// Reset flags
	at_barrier_thread_count = 0;
	finished_thread = 0;
	finished_emu = false;
	taken_thread = 0;

	// simulation performance
	emu_inst_count = 0;
	emu_time_start = 0;
	emu_time_end = 0;

	// Statistics
	inst_count = 0;
	global_mem_inst_count = 0;
	shared_mem_inst_count = 0;
}


void Warp::Dump(std::ostream &os) const
{
}


void Warp::Execute()
{
	// Get emu instance
	Emu *emu = Emu::getInstance();

	// Instruction binary
	InstBytes inst_bytes;

	// Instruction opcode
	InstOpcode inst_op;

	// Read instruction binary
	inst_bytes.as_uint[0] = inst_buffer[pc / inst_size] >> 32;
	inst_bytes.as_uint[1] = inst_buffer[pc / inst_size];

	// Decode instruction
	if( pc % 64)
	{
			inst.Decode((const char *) &inst_bytes, pc);

			// Execute instruction
			inst_op = (InstOpcode) inst.getOpcode();

			if (!inst_op)
			std::cerr << __FILE__ << ":" << __LINE__ << ": unrecognized instruction "
					<< std::hex << " pc " << pc << std::endl
					<< inst_bytes.as_uint[0]<< inst_bytes.as_uint[1] << std::endl;

			for (auto thread_id = threads_begin; thread_id < threads_end; ++thread_id)
			{
				thread_id->get()->Execute(inst_op,&inst);
				//std::cerr << inst.getName() << " id " << thread_id->get()->getId()
				//		<< " warp_id " << id << std::endl;
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
		if (thread_block->getWarpsCompletedEmuCount() ==
				thread_block->getWarpCount())
		{
			thread_block->setFinishedEmu(true);
		}
		return;

	}

    inst_count++;					//other counter?
    emu_inst_count++;
    emu->incAluInstCount();
    pc = this->target_pc;

    if(pc >= inst_buffer_size - 8)
    {
    	finished_emu = true;
    	thread_block->incWarpsCompletedEmu();
    }
	// Stats

	//asEmu(emu)->instructions++; // no parent class any more
}

/*
unsigned Warp::getStackPCNTReconvPc() const
{
		for(auto i = sync_stack.rbegin(); i < sync_stack.rend(); ++i)
		{
				if (i->entry_type == PCNT)
					return i->reconv_pc;
		}

		throw misc::Error("Instruction PCNT error."
				"No PBK entry in synchronization stack");
		return 0;
}


void Warp::CONTStack(unsigned id_in_warp)
{
		for(auto i = sync_stack.rbegin(); i < sync_stack.rend(); ++i)
		{
				if (i->entry_type == PCNT)
				{
                        i->active_thread_mask |= (1u << id_in_warp);
						// clear the current active mask
						active_mask &= ~(0x1u << id_in_warp);
						return;
				}
				// clear the bit in every entry before a PCNT
				i->active_thread_mask &= ~(0x1u << id_in_warp);
		}

		throw misc::Error("Instruction PCNT error."
				"No PBK entry in synchronization stack");
}
*/

unsigned Warp::getFinishedThreadCount() const
{
        unsigned num = 0;

        for (unsigned i = 0; i < thread_count; i++)
                num += (finished_thread >> i) & 1u;

        assert(num <= thread_count);

        return num;
}


unsigned Warp::getActiveMaskBitCount() const
{
        unsigned num = 0;

        for (unsigned i = 0; i < thread_count; i++)
                num += (active_mask >> i) & 1u;

        assert(num <= thread_count);

        return num;
}


} //namespace
