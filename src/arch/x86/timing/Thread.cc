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

#include "Thread.h"
#include "CPU.h"
#include "Timing.h"

namespace x86
{

Thread::Thread(const std::string &name, Timing *timing,
	CPU *cpu, Core *core, int id_in_core)
		:
		name(name),
		timing(timing),
		cpu(cpu),
		core(core),
		id_in_core(id_in_core)
{
	// Initialize Uop queue
	uop_queue.resize(CPU::getUopQueueSize());

	// Initialize fetch queue
	fetch_queue.resize(CPU::getFetchQueueSize());

	// Initialize reorder buffer
	reorder_buffer_left_bound = this->id_in_core * CPU::getReorderBufferSize();
	reorder_buffer_right_bound = (this->id_in_core + 1) * CPU::getReorderBufferSize() - 1;
	reorder_buffer_head = reorder_buffer_left_bound;
	reorder_buffer_tail = reorder_buffer_left_bound;

	// Initialize branch predictor
	branch_predictor = misc::new_unique<BranchPredictor>(name +
			".BranchPredictor");

	// Initialize trace cache
	if (TraceCache::getPresent())
		trace_cache = misc::new_unique<TraceCache>(name +
				".TraceCache");

	// Initialize register file
	reg_file = misc::new_unique<RegisterFile>(core, this);
	reg_file->InitRegisterFile();
}


void Thread::RecoverFetchQueue()
{
	// Local variable declaration
	Uop *uop;

	// Recover fetch queue
	while (fetch_queue.size())
	{
		uop = fetch_queue.back().get();
		assert(uop->getThread() == this);
		if (!uop->getSpeculativeMode())
			break;
		uop->setInFetchQueue(false);
		if (!uop->isFromTraceCache() && !uop->getMopIndex())
		{
			fetch_queue_occupied -= uop->getMopSize();
			assert(fetch_queue_occupied >= 0);
		}
		if (uop->isFromTraceCache())
		{
			trace_cache_queue_occupied--;
			assert(trace_cache_queue_occupied >= 0);
		}
		if (fetch_queue.size() > 0)
		{
			assert(fetch_queue_occupied > 0);
			assert(trace_cache_queue_occupied > 0);
		}
		fetch_queue.pop_back();

		// Trace FIXME
	}
}


bool Thread::CanInsertInInstructionQueue()
{
	// Local variable
	int count;
	int size;

	// Get the size of the queue and the Uop count in the queue
	if (CPU::getInstructionQueueKind() == CPU::InstructionQueueKindPrivate)
	{
		size = CPU::getInstructionQueueSize();
		count = uop_count_in_instruction_queue;
	}
	else
	{
		size = CPU::getInstructionQueueSize() * CPU::getNumThreads();
		count = core->getInstructionQueueCount();
	}

	// return the flag
	return count < size;
}


void Thread::InsertInInstructionQueue(std::shared_ptr<Uop> &uop)
{
	// Make sure the Uop is not in the instruction queue
	assert(!uop->isInInstructionQueue());

	// Insert
	instruction_queue.push_back(uop);

	// Set the flag to true to indicate that the Uop is in the instruction queue
	uop->setInInstructionQueue(true);

	// Increment the Uop count both for thread and core
	core->incInstructionQueueCount();
	uop_count_in_instruction_queue++;
}


void Thread::RemoveFromInstructionQueue(int index)
{
	// Make sure there is Uop in the instruction queue
	assert(instruction_queue.size() > 0);

	// Set the flag to false to indicate that
	// The Uop is not in the instruction queue anymore
	Uop *uop = instruction_queue[index].get();
	uop->setInInstructionQueue(false);

	// Remove
	instruction_queue.erase(instruction_queue.begin() + index);

	// Decrement the Uop count both for thread and core
	assert(core->getInstructionQueueCount() > 0 && uop_count_in_instruction_queue > 0);
	core->decInstructionQueueCount();
	uop_count_in_instruction_queue--;
}


void Thread::RecoverInstructionQueue()
{
	// Local variable declaration
	Uop *uop;

	// Remove the Uop in speculative mode
	for (unsigned int index = 0; index < instruction_queue.size(); index++)
	{
		// Get Uop from instruction queue
		uop = instruction_queue[index].get();

		// If Uop is in speculative mode, then remove it from the queue
		if (uop->getSpeculativeMode())
		{
			// Remove
			RemoveFromInstructionQueue(index);

			// Keep the index unchanged and loop
			continue;
		}

		// Increment the queue index
		index++;
	}
}


bool Thread::CanInsertInLoadStoreQueue()
{
	// Local variable
	int count;
	int size;

	// Get the size of the queue and the Uop count in the queue
	if (CPU::getLoadStoreQueueKind() == CPU::LoadStoreQueueKindPrivate)
	{
		size = CPU::getLoadStoreQueueSize();
		count = uop_count_in_load_store_queue;
	}
	else
	{
		size = CPU::getLoadStoreQueueSize() * CPU::getNumThreads();
		count = core->getLoadStoreQueueCount();
	}

	// return the flag
	return count < size;
}


void Thread::InsertInLoadStoreQueue(std::shared_ptr<Uop> &uop)
{
	// Make sure there is Uop in the load/store queue
	assert(!uop->isInLoadQueue() && !uop->isInStoreQueue());

	// Make sure the Opcode is correct
	assert(uop->getUinst()->getOpcode() == UInstLoad || uop->getUinst()->getOpcode() == UInstStore ||
			uop->getUinst()->getOpcode() == UInstPrefetch);

	// Insert and set flag accordingly
	if (uop->getUinst()->getOpcode() == UInstLoad)
	{
		load_queue.push_back(uop);
		uop->setInLoadQueue(true);
	}
	else if (uop->getUinst()->getOpcode() == UInstStore)
	{
		store_queue.push_back(uop);
		uop->setInStoreQueue(true);
	}

	// Increment the Uop count both for thread and core
	core->incLoadStoreQueueCount();
	uop_count_in_load_store_queue++;
}


void Thread::RemoveFromLoadQueue(int index)
{
	// Make sure there is Uop in the load queue
	assert(load_queue.size() > 0);

	// Set the flag to false to indicate that
	// The Uop is not in the load queue anymore
	Uop *uop = load_queue[index].get();
	uop->setInLoadQueue(false);

	// Remove
	load_queue.erase(load_queue.begin() + index);

	// Decrement the Uop count both for thread and core
	assert(core->getLoadStoreQueueCount() > 0 && uop_count_in_load_store_queue > 0);
	core->decLoadStoreQueueCount();
	uop_count_in_load_store_queue--;
}


void Thread::RemoveFromStoreQueue(int index)
{
	// Make sure there is Uop in the store queue
	assert(store_queue.size() > 0);

	// Set the flag to false to indicate that
	// The Uop is not in the store queue anymore
	Uop *uop = store_queue[index].get();
	uop->setInStoreQueue(false);

	// Remove
	store_queue.erase(store_queue.begin() + index);

	// Decrement the Uop count both for thread and core
	assert(core->getLoadStoreQueueCount() > 0 && uop_count_in_load_store_queue > 0);
	core->decLoadStoreQueueCount();
	uop_count_in_load_store_queue--;
}


void Thread::RecoverLoadStoreQueue()
{
	// Local variable declaration
	Uop *uop;

	// Remove the Uop in speculative mode from load queue
	for (unsigned int index = 0; index < load_queue.size(); index++)
	{
		// Get Uop from load queue
		uop = load_queue[index].get();

		// If Uop is in speculative mode, then remove it from the queue
		if (uop->getSpeculativeMode())
		{
			// Remove
			RemoveFromLoadQueue(index);

			// Keep the index unchanged and loop
			continue;
		}

		// Increment the queue index
		index++;
	}

	// Remove the Uop in speculative mode from store queue
	for (unsigned int index = 0; index < store_queue.size(); index++)
	{
		// Get Uop from store queue
		uop = store_queue[index].get();

		// If Uop is in speculative mode, then remove it from the queue
		if (uop->getSpeculativeMode())
		{
			// Remove
			RemoveFromStoreQueue(index);

			// Keep the index unchanged and loop
			continue;
		}

		// Increment the queue index
		index++;
	}
}


void Thread::RecoverUopQueue()
{
	// Local variable declaration
	Uop *uop;

	// Recover
	while (uop_queue.size() > 0)
	{
		// Get the last Uop from Uop queue
		uop = uop_queue.back().get();

		// Check whether the thread is correct
		assert(uop->getThread() == this);

		// If the Uop is not in speculative mode, then exit
		if (!uop->getSpeculativeMode())
			break;

		// Set the flag to false to indicate that
		// The Uop is not in the Uop queue anymore
		uop->setInUopQueue(false);

		// Remove
		uop_queue.pop_back();
	}
}


bool Thread::isLongLatencyInEventQueue()
{
	for (auto uop : core->getEventQueue())
	{
		// Only check the uop that belong to the current thread
		if (uop->getThread() != this)
			continue;

		// Check whether there is uop that have waited for more than 20 cycles
		if (timing->getCycle() - uop->getIssueWhen() > 20)
			return true;
	}
	return false;
}


bool Thread::isCacheMissInEventQueue()
{
	for (auto uop : core->getEventQueue())
	{
		// Only check the uop that belong to the current thread
		// Only check the uop that load memory
		if (uop->getThread() != this ||uop->getUinst()->getOpcode() != UInstLoad)
			continue;

		// Check whether there is uop that have waited for more than 5 cycles
		// Assume that a load that take more than 5 cycles is means that cache miss happened
		if (timing->getCycle() - uop->getIssueWhen() > 5)
			return true;
	}
	return false;
}


void Thread::RecoverEventQueue()
{

}


bool Thread::CanFetch()
{
	return true;
}


std::shared_ptr<Uop> Thread::FetchInstruction(bool fetch_from_trace_cache)
{
	return nullptr;
}


bool Thread::FetchFromTraceCache()
{
	return true;
}


void Thread::Fetch()
{

}

}
