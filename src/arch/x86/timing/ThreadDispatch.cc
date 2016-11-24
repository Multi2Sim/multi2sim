/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include "Core.h"
#include "Cpu.h"
#include "Thread.h"
#include "Timing.h"


namespace x86
{

Thread::DispatchStall Thread::canDispatch()
{
	// Uop queue is empty
	if (uop_queue.empty())
		return !context || !context->getState(Context::StateRunning) ?
				DispatchStallContext :
				DispatchStallUopQueue;
	
	// Reorder buffer is full
	if (!canInsertInReorderBuffer())
		return DispatchStallReorderBuffer;

	// Instruction queue is full
	Uop *uop = uop_queue.front().get();
	if (!(uop->getFlags() & Uinst::FlagMem) && !canInsertInInstructionQueue())
		return DispatchStallInstructionQueue;

	// Load-store queue is full
	if ((uop->getFlags() & Uinst::FlagMem) && !canInsertInLoadStoreQueue())
		return DispatchStallLoadStoreQueue;

	// No physical registers available
	if (!register_file->canRename(uop))
		return DispatchStallRename;

	// Dispatch slot available
	return DispatchStallUsed;
}


int Thread::Dispatch(int quantum)
{
	// Repeat while there is quantum left
	while (quantum)
	{
		// Check if we can dispatch
		DispatchStall stall = canDispatch();
		if (stall != DispatchStallUsed)
		{
			core->incDispatchStall(stall, quantum);
			break;
		}

		// Get uop at the head of the uop queue
		assert(uop_queue.size());
		std::shared_ptr<Uop> uop = uop_queue.front();
	
		// Extract uop from uop queue
		ExtractFromUopQueue(uop.get());
		
		// Register renaming
		register_file->Rename(uop.get());
		
		// Insert in reorder buffer
		InsertInReorderBuffer(uop);
		core->incNumReorderBufferWrites();
		num_reorder_buffer_writes++;

		// Mark instruction as dispatched
		uop->dispatched = true;
		uop->dispatch_when = cpu->getCycle();
		
		// Insert non-memory instruction into instruction queue
		if (!(uop->getFlags() & Uinst::FlagMem))
		{
			InsertInInstructionQueue(uop);
			core->incNumInstructionQueueWrites();
			num_instruction_queue_writes++;
		}
		
		// Memory instructions into the load-store queue
		if ((uop->getFlags() & Uinst::FlagMem))
		{
			InsertInLoadStoreQueue(uop);
			core->incNumLoadStoreQueueWrites();
			num_load_store_queue_writes++;
		}

		// Increment dispatch slot
		core->incDispatchStall(uop->speculative_mode ?
				DispatchStallSpeculative :
				DispatchStallUsed, 1);

		// Increment number of dispatched micro-instructions of each
		// kind
		incNumDispatchedUinsts(uop->getOpcode());
		core->incNumDispatchedUinsts(uop->getOpcode());
		cpu->incNumDispatchedUinsts(uop->getOpcode());
		
		// Increment number of dispatched micro-instructions coming from
		// the trace cache
		if (uop->from_trace_cache)
			trace_cache->incNumDispatchedUinsts();
		
		// Another instruction dispatched, update quantum
		quantum--;

		// Trace
		Timing::trace << misc::fmt("x86.inst "
				"id=%lld "
				"core=%d "
				"stg=\"di\"\n",
				uop->getIdInCore(),
				core->getId());
	}

	// Return remaining unused quantum
	return quantum;
}

}

