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

#include "Thread.h"
#include "Timing.h"


namespace x86
{

void Thread::RecoverFetchQueue()
{
	// Keep squashing instructions from tail
	while (fetch_queue.size())
	{
		// Get uop from the tail
		std::shared_ptr<Uop> uop = fetch_queue.back();
		assert(uop->getThread() == this);

		// Stop if this uop is not in speculative mode anymore
		if (!uop->speculative_mode)
			break;

		// Remove from fetch queue
		ExtractFromFetchQueue(uop.get());

		// Trace
		if (Timing::trace)
		{
			// Output
			Timing::trace << misc::fmt("x86.inst "
					"id=%lld "
					"core=%d "
					"stg=\"sq\"\n",
					uop->getIdInCore(),
					core->getId());

			// Keep uop for later
			cpu->InsertInTraceList(uop);
		}
	}

	// Sanity
	assert(fetch_queue_occupancy == 0);
	assert(trace_cache_queue_occupancy == 0);
}


void Thread::RecoverUopQueue()
{
	// Keep squashing uops from the queue
	while (uop_queue.size())
	{
		// Get uop from the back
		std::shared_ptr<Uop> uop = uop_queue.back();
		assert(uop->getThread() == this);

		// Stop if uop is not in speculative mode
		if (!uop->speculative_mode)
			break;

		// Remove it from uop queue
		ExtractFromUopQueue(uop.get());

		// Trace
		if (Timing::trace)
		{
			// Output
			Timing::trace << misc::fmt("x86.inst "
					"id=%lld "
					"core=%d "
					"stg=\"sq\"\n",
					uop->getIdInCore(),
					core->getId());

			// Keep uop for later
			cpu->InsertInTraceList(uop);
		}
	}
}


void Thread::RecoverInstructionQueue()
{
	// Traverse instruction queue
	auto it = instruction_queue.begin();
	auto e = instruction_queue.end();
	while (it != e)
	{
		// Get instruction
		Uop *uop = it->get();
		++it;

		// Remove if it is a speculative uop
		if (uop->speculative_mode)
			ExtractFromInstructionQueue(uop);
	}
}


void Thread::RecoverLoadQueue()
{
	// Traverse load queue
	auto it = load_queue.begin();
	auto e = load_queue.end();
	while (it != e)
	{
		// Get instruction
		Uop *uop = it->get();
		++it;

		// Remove if it is a speculative uop
		if (uop->speculative_mode)
			ExtractFromLoadQueue(uop);
	}
}


void Thread::RecoverStoreQueue()
{
	// Traverse store queue
	auto it = store_queue.begin();
	auto e = store_queue.end();
	while (it != e)
	{
		// Get instruction
		Uop *uop = it->get();
		++it;

		// Remove if it is a speculative uop
		if (uop->speculative_mode)
			ExtractFromStoreQueue(uop);
	}
}


void Thread::RecoverEventQueue()
{
	// Traverse event queue
	auto it = core->getEventQueueBegin();
	auto e = core->getEventQueueEnd();
	while (it != e)
	{
		// Get instruction
		Uop *uop = it->get();
		++it;

		// Remove if it is a speculative uop in the current thread
		if (uop->getThread() == this && uop->speculative_mode)
			core->ExtractFromEventQueue(uop);
	}
}


void Thread::Recover()
{
	// Remove instructions of this thread in fetch queue, uop queue,
	// instruction queue, store queue, load queue, and event queue.
	RecoverFetchQueue();
	RecoverUopQueue();
	RecoverInstructionQueue();
	RecoverLoadQueue();
	RecoverStoreQueue();
	RecoverEventQueue();

	// Remove instructions from ROB, restoring the state of the physical
	// register file.
	while (reorder_buffer.size())
	{
		// Get instruction at the reorder buffer tail
		std::shared_ptr<Uop> uop = reorder_buffer.back();
		assert(uop->getThread() == this);

		// If we already removed all speculative instructions, done
		if (!uop->speculative_mode)
			break;
		
		// Statistics
		num_squashed_uinsts++;
		core->incNumSquashedUinsts();
		cpu->incNumSquashedUinsts();
		if (uop->from_trace_cache)
			trace_cache->incNumSquashedUinsts();

		// Finish register renaming if uop didn't complete yet
		if (!uop->completed)
			register_file->WriteUop(uop.get());

		// Undo register renaming
		register_file->UndoUop(uop.get());

		// Trace
		if (Timing::trace)
		{
			// Output
			Timing::trace << misc::fmt("x86.inst "
					"id=%lld "
					"core=%d "
					"stg=\"sq\"\n",
					uop->getIdInCore(),
					core->getId());

			// Save uop for later
			cpu->InsertInTraceList(uop);
		}

		// Remove reorder buffer entry
		ExtractFromReorderBuffer(uop.get());
	}

	// Check state of fetch stage and mapped context, if still any
	if (context)
	{
		// If we actually fetched wrong instructions, recover emulator
		if (context->getState(Context::StateSpecMode))
			context->Recover();
	
		// Set next program counter to valid address
		fetch_neip = context->getRegs().getEip();
	}
}

}

