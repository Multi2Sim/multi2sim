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
#include <lib/util/bit-map.h>
#include <lib/util/hash-table.h>

#include "Emu.h"
#include "Grid.h"
#include "isa.h"
#include "Machine.h"
#include "Thread.h"
#include "ThreadBlock.h"
#include "Warp.h"
//#include "Wrapper.h"

/*
 * Public Functions
 */

namespace Kepler
{

Warp::Warp(ThreadBlock *thread_block, unsigned id)
{
	/* Initialization */
	this->id = id + thread_block->getId() * thread_block->getWarpsInWorkgroup();
	id_in_thread_block = id;
	grid = thread_block->getGrid();
	this->thread_block = thread_block;

	/* Allocate threads */
	if (id < thread_block->getWarpsInWorkgroup() - 1)
		thread_count = warp_size;
	else
		thread_count = grid->getThreadBlockSize() -
		(thread_block->getWarpsInWorkgroup() - 1) * warp_size;

	/* Instruction */
	inst = new Inst();
	inst_size = 8;
	inst_buffer = grid->getInstBuffer();
	inst_buffer_size = grid->getInstBufferSize();

	/* Sync stack */
	sync_stack_top = 0;
	//sync_stack.entries[sync_stack_top].active_thread_mask =
	//		bit_map_create(thread_count);
	//bit_map_set(sync_stack.entries[sync_stack_top].
	//		active_thread_mask, 0, thread_count,
	//		((unsigned long long)1 << thread_count) - 1);
	sync_stack.entries[sync_stack_top].active_thread_mask =
			((unsigned long long)1 << thread_count) - 1;

	/* Reset flags */
	at_barrier_thread_count = 0;
	finished_thread_count = 0;
	finished_emu = false;		//make it clear

	/* simulation performance */
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
	Emu *emu = Emu::getInstance();
	InstBytes inst_bytes;
	InstOpcode inst_op;

	InstFunc InstFunction;

	/* Get instruction */
	inst_bytes.as_uint[0] = inst_buffer[pc / inst_size] >> 32;
	inst_bytes.as_uint[1] = inst_buffer[pc / inst_size];
	//kpl_isa_debug("%s:%d: warp[%d] executes instruction [0x%x] 0x%016llx\n",
		//	__FILE__, __LINE__, this->id, this->pc, inst_bytes.as_dword);

	/* Decode instruction */
	if( pc % 64)
	{
		inst->Decode((const char *) &inst_bytes, pc);

		/* Execute instruction */
		inst_op = (InstOpcode) inst->getOpcode();
		if (!inst_op)
			std::cerr << __FILE__ << ":" << __LINE__ << ": unrecognized instruction ("
					<< inst_bytes.as_uint[0]<< inst_bytes.as_uint[1] << std::endl;
		for (auto thread_id = threads_begin; thread_id < threads_end; ++thread_id)
		{
			InstFunction = grid->getInstFunc(inst_op); //make it clear
			(*InstFunction)(thread_id->get(), inst);
		}
	}
	/* Finish */

	if (finished_emu)
	{
		/*
		assert(list_index_of(thread_block->running_warps, this) != -1);
		assert(list_index_of(thread_block->finished_warps, this) == -1);
		list_remove(thread_block->running_warps, this);
		list_add(thread_block->finished_warps, this);
		*/

		// Check if thread block finished kernel execution
		//copied from SI, delete it if not necessary/
		if (thread_block->getFinishedEmu() ==
				thread_block->getWarpsInWorkgroup())
		{
			thread_block->setFinishedEmu(true);
		}
		return;

	}

	/* Update PC */
	if( pc % 64 )
	{
        inst_count++;					//other counter?
        emu_inst_count++;
        emu->incAluInstCount();
	}
/*	if (KplInstWrapGetCategory(inst) != KplInstCategoryCtrl)
		pc += inst_size;
	else
		pc = this->target_pc;
*/
        pc += inst_size;		//make it clear, no jump
    if(pc >= grid->getInstBufferSize() - 8)
    {
    	finished_emu = true;
    	thread_block->incWarpsCompletedEmu();
    }
	/* Stats */

	//asEmu(emu)->instructions++; // no parent class any more
}

} //namespace
