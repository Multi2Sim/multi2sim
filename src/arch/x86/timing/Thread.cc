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

namespace x86
{

Thread::Thread(const std::string &name, CPU *cpu, Core *core, int id_in_core) :
		name(name),
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
		if (!uop->IsFromTraceCache() && !uop->getMopIndex())
		{
			fetch_queue_occupied -= uop->getMopSize();
			assert(fetch_queue_occupied >= 0);
		}
		if (uop->IsFromTraceCache())
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
		count = instruction_queue_count;
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
	assert(!uop.get()->IsInInstructionQueue());

	// Insert
	instruction_queue.push_back(uop);

	// Set the flag to true to indicate that the Uop is in the instruction queue
	uop.get()->setInInstructionQueue(true);

	// Increment the Uop count both for thread and core
	core->incInstructionQueueCount();
	instruction_queue_count++;
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
	assert(core->getInstructionQueueCount() > 0 && instruction_queue_count > 0);
	core->decInstructionQueueCount();
	instruction_queue_count--;
}


void Thread::RecoverInstructionQueue()
{
	// Local variable declaration
	Uop *uop;

	// Remove the Uop in speculative mode
	for (unsigned int index = 0; index < instruction_queue.size(); index++)
	{
		uop = instruction_queue[index].get();
		if (uop->getSpeculativeMode())
		{
			// Remove
			RemoveFromInstructionQueue(index);

			// Keep the index unchanged and loop
			continue;
		}
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
		count = load_store_queue_count;
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
	assert(!uop->IsInLoadQueue() && !uop->IsInStoreQueue());

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
	else if (uop->getUinst()->getOpcode() == UInstPrefetch)
	{
		prefetch_queue.push_back(uop);
		uop->setInPrefetchQueue(true);
	}

	// Increment the Uop count both for thread and core
	core->incLoadStoreQueueCount();
	load_store_queue_count++;
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
	assert(core->getLoadStoreQueueCount() > 0 && load_store_queue_count > 0);
	core->decLoadStoreQueueCount();
	load_store_queue_count--;
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
	assert(core->getLoadStoreQueueCount() > 0 && load_store_queue_count > 0);
	core->decLoadStoreQueueCount();
	load_store_queue_count--;
}


void Thread::RemoveFromPrefetchQueue(int index)
{
	// Make sure there is Uop in the prefetch queue
	assert(prefetch_queue.size() > 0);

	// Set the flag to false to indicate that
	// The Uop is not in the prefetch queue anymore
	Uop *uop = prefetch_queue[index].get();
	uop->setInPrefetchQueue(false);

	// Remove
	prefetch_queue.erase(prefetch_queue.begin() + index);

	// Decrement the Uop count both for thread and core
	assert(core->getLoadStoreQueueCount() > 0 && load_store_queue_count > 0);
	core->decLoadStoreQueueCount();
	load_store_queue_count--;
}


void Thread::RecoverLoadStoreQueue()
{
	// Local variable declaration
	Uop *uop;

	// Remove the Uop in speculative mode from load queue
	for (unsigned int index = 0; index < load_queue.size(); index++)
	{
		uop = load_queue[index].get();
		if (uop->getSpeculativeMode())
		{
			// Remove
			RemoveFromLoadQueue(index);

			// Keep the index unchanged and loop
			continue;
		}
		index++;
	}

	// Remove the Uop in speculative mode from store queue
	for (unsigned int index = 0; index < store_queue.size(); index++)
	{
		uop = store_queue[index].get();
		if (uop->getSpeculativeMode())
		{
			// Remove
			RemoveFromStoreQueue(index);

			// Keep the index unchanged and loop
			continue;
		}
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
		uop = uop_queue.back().get();
		assert(uop->getThread() == this);
		if (!uop->getSpeculativeMode())
			break;
		uop->setInUopQueue(false);
		uop_queue.pop_back();
	}
}

}
