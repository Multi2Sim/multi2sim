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

#include "Timing.h"
#include "CPU.h"
#include "Core.h"


namespace x86
{

Core::Core(const std::string &name, CPU *cpu, int id)
	:
	name(name),
	cpu(cpu),
	id(id)
{
	// The prefix for each core
	std::string prefix = name + "Thread";
	std::string thread_name;

	// Initialize ROB
	InitializeReorderBuffer();

	// Initialize functional unit
	functional_unit = misc::new_unique<FunctionalUnit>();

	// Create threads
	for (int i = 0; i < CPU::getNumThreads(); i++)
	{
		thread_name = prefix + misc::fmt("%d", i);
		threads.emplace_back(misc::new_unique<Thread>(thread_name,
				this->cpu, this, i));
	}
}


void Core::InitializeReorderBuffer()
{
	// Create empty reorder buffer
	reorder_buffer_total_size = CPU::getReorderBufferSize() * CPU::getNumThreads();
	reorder_buffer.resize(reorder_buffer_total_size);
}


void Core::TrimReorderBuffer()
{
	// Local variable
	int idx;
	Uop *uop;

	// Trim head
	while (uop_count_in_rob > 0)
	{
		uop = reorder_buffer[reorder_buffer_head].get();
		if (uop != nullptr)
			break;
		reorder_buffer_head = reorder_buffer_head == reorder_buffer_total_size - 1 ?
				0 : reorder_buffer_head + 1;
		uop_count_in_rob--;
	}

	// Trim tail
	while (uop_count_in_rob > 0)
	{
		idx = reorder_buffer_tail ? reorder_buffer_tail - 1 : reorder_buffer_total_size - 1;
		uop = reorder_buffer[idx].get();
		if (uop)
			break;
		reorder_buffer_tail = idx;
		uop_count_in_rob--;
	}
}


bool Core::CanEnqueueInReorderBuffer(Uop *uop)
{
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		if (uop->getThread()->getUopCountInRob() < CPU::getReorderBufferSize())
			return true;
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		if (uop_count_in_rob < reorder_buffer_total_size)
			return true;
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	return false;
}


void Core::EnqueueInReorderBuffer(Uop *uop)
{
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		assert(uop->getThread()->getUopCountInRob() < CPU::getReorderBufferSize());
		assert(!reorder_buffer[uop->getThread()->getReorderBufferTail()]);
		reorder_buffer[uop->getThread()->getReorderBufferTail()].reset(uop);
		uop->getThread()->incReorderBufferTail();
		uop->getThread()->incUopCountInRob();
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		assert(uop_count_in_rob < reorder_buffer_total_size);
		assert(!reorder_buffer[reorder_buffer_tail]);
		reorder_buffer[reorder_buffer_tail].reset(uop);
		reorder_buffer_tail = reorder_buffer_tail == reorder_buffer_total_size - 1 ?
				0 : reorder_buffer_tail + 1;
		uop_count_in_rob++;
		uop->getThread()->incUopCountInRob();
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	//Instruction is in the ROB
	uop->setInReorderBuffer(true);
}


bool Core::CanDequeueFromReorderBuffer(int thread_id)
{
	// Local variable declaration
	Uop *uop;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id);

	// Check according to ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		if (thread->getUopCountInRob() > 0)
			return true;

		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		if (uop_count_in_rob == 0)
			return false;

		uop = reorder_buffer[reorder_buffer_head].get();
		if (uop->getThread() == thread)
			return true;
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return false;
}

Uop *Core::getReorderBufferHead(int thread_id)
{
	// Local variable declaration
	Uop *uop;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id);

	// Get head according to ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		if (thread->getUopCountInRob() > 0)
		{
			uop = reorder_buffer[thread->getReorderBufferHead()].get();
			return uop;
		}
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		if (thread->getUopCountInRob() == 0)
			return nullptr;

		for (int i = 0; i < uop_count_in_rob; i++)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->getThread() == thread)
				return uop;
		}
		throw misc::Panic("NO head found\n");
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return nullptr;
}

void Core::RemoveReorderBufferHead(int thread_id)
{
	// Local variable declaration
	Uop *uop = nullptr;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id);

	// Remove Uop according to the ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		assert(thread->getUopCountInRob() > 0);
		uop = reorder_buffer[thread->getReorderBufferHead()].get();
		assert(uop->getThread() == thread);
		reorder_buffer[thread->getReorderBufferHead()].reset();
		thread->incReorderBufferHead();
		thread->decUopCountInRob();
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		assert(thread->getUopCountInRob());
		for (int i = 0; i < uop_count_in_rob; i++)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->getThread() == thread)
			{
				reorder_buffer[idx].reset(nullptr);;
				thread->decUopCountInRob();
				break;
			}
		}
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	// Free instruction
	uop->setInReorderBuffer(false);
}


Uop *Core::getReorderBufferTail(int thread_id)
{
	// Local variable declaration
	Uop *uop;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id);

	// Get tail according to ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		if (thread->getUopCountInRob() > 0)
		{
			if (thread->getReorderBufferTail() == thread->getReorderBufferLeftBound())
				idx = thread->getReorderBufferRightBound();
			else
				idx = thread->getReorderBufferTail() - 1;
			uop = reorder_buffer[idx].get();
			return uop;
		}
		break;

	case CPU::ReorderBufferKindShared:

		// in core domain
		TrimReorderBuffer();
		if (!thread->getUopCountInRob())
			return nullptr;
		for (int i = uop_count_in_rob - 1; i >= 0; i--)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->getThread() == thread)
				return uop;
		}
		throw misc::Panic("reorder_buffer_tail: no tail found");
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return nullptr;
}

void Core::RemoveReorderBufferTail(int thread_id)
{
	// Local variable declaration
	Uop *uop = nullptr;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id);

	// Remove tail according to ROB kind
	switch (CPU::getReorderBufferKind())
	{

	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		assert(thread->getUopCountInRob() > 0);
		if (thread->getReorderBufferTail() == thread->getReorderBufferLeftBound())
			idx = thread->getReorderBufferRightBound();
		else
			idx = thread->getReorderBufferTail() - 1;
		uop = reorder_buffer[idx].get();
		assert(uop->getThread() == thread);
		reorder_buffer[idx].reset();
		thread->setReorderBufferTail(idx);
		thread->decUopCountInRob();;
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		assert(thread->getUopCountInRob());
		for (int i = uop_count_in_rob - 1; i >= 0; i--)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->getThread() == thread)
			{
				reorder_buffer[idx].reset(nullptr);;
				thread->decUopCountInRob();
				break;
			}
		}
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	// Free instruction
	uop->setInReorderBuffer(false);
}

Uop *Core::getReorderBufferEntry(int index, int thread_id)
{
	// Local declaration
	Uop *uop;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id);

	// Check that index is in bounds
	if (index < 0 || index >= thread->getUopCountInRob())
		return nullptr;

	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		index += thread->getReorderBufferHead();
		if (index > thread->getReorderBufferRightBound())
			index = index - thread->getReorderBufferRightBound()
				+ thread->getReorderBufferLeftBound() - 1;
		uop = reorder_buffer[index].get();
		assert(uop);
		return uop;

	case CPU::ReorderBufferKindShared:

		TrimReorderBuffer();
		index = (reorder_buffer_head + index) % reorder_buffer_total_size;
		uop = reorder_buffer[index].get();
		assert(uop);
		return uop;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return nullptr;
}


void Core::InsertInEventQueue(std::shared_ptr<Uop> uop)
{
	// Make sure the uop is not in the event queue at the moment
	assert(!uop->isInEventQueue());

	// Traverse the list to find the proper position
	for (auto it = event_queue.begin(); it != event_queue.end(); it++)
	{
		// Check whether the current item should be handled after the uop
		if (uop->Compare(it->get()) < 0)
		{
			// Insert the Uop
			event_queue.insert(it, uop);

			// Exit the loop
			break;
		}
	}

	// Set flat to indicate the uop is in the event queue
	uop->setInEventQueue(true);
}


std::shared_ptr<Uop> Core::ExtractFromEventQueue()
{
	// If the queue is empty, return a null pointer
	if (event_queue.size() <= 0)
		return nullptr;

	// Get the first element of the queue
	std::shared_ptr<Uop> uop = event_queue.front();

	// Make sure the uop is currently in the event queue
	assert(uop->isInEventQueue());

	// Remove the uop
	event_queue.pop_front();

	// Set flag to indicate that the uop is not in the queue anymore
	uop->setInEventQueue(false);

	// Return the uop
	return uop;
}


void Core::Fetch()
{
	// Invoke fetch stage function according to the kind
	switch (CPU::getFetchKind())
	{

	case CPU::FetchKindShared:
	{
		// Fetch from all threads
		for (int i = 0; i < CPU::getNumThreads(); i++)
			if (threads[i]->CanFetch())
				threads[i]->Fetch();
		break;
	}

	case CPU::FetchKindTimeslice:
	{
		// Round-robin fetch
		Thread *thread;
		for (int i = 0; i < CPU::getNumThreads(); i++)
		{
			current_fetch_thread = (current_fetch_thread + 1) % CPU::getNumThreads();
			thread = threads[current_fetch_thread].get();
			if (thread->CanFetch())
			{
				thread->Fetch();
				break;
			}
		}
		break;
	}

	case CPU::FetchKindSwitchonevent:
	{
		// If current thread is stalled, it means that we just switched to it.
		// No fetching and no switching either.
		Thread *thread = threads[current_fetch_thread].get();
		if (thread->getFetchStallUntil() >= Timing::getInstance()->getCycle())
			break;

		// Switch thread if:
		// - Quantum expired for current thread.
		// - Long latency instruction is in progress.
		bool must_switch = !thread->CanFetch();
		must_switch = must_switch || Timing::getInstance()->getCycle() - fetch_switch_when >
				CPU::getThreadQuantum() + CPU::getThreadSwitchPenalty();
		must_switch = must_switch || thread->isLongLatencyInEventQueue();

		// Switch thread
		if (must_switch)
		{
			// Find a new thread to switch to
			Thread *new_thread;
			int new_index;
			for (new_index = (thread->getIDInCore() + 1) % CPU::getNumThreads();
					new_index != thread->getIDInCore();
					new_index = (new_index + 1) % CPU::getNumThreads())
			{
				// Do not choose it if it is not eligible for fetching
				new_thread = threads[new_index].get();
				if (!new_thread->CanFetch())
					continue;

				// Do not choose it if it is unfair
				if (new_thread->getNumFetchedUinst() >
					thread->getNumFetchedUinst() + 100000)
					continue;

				// Do not choose it if it is stalled
				if (new_thread->isLongLatencyInEventQueue())
					continue;

				// Choose it if we need to switch
				if (new_thread->CanFetch())
					break;
			}

			if (new_index != thread->getIDInCore())
			{
				// Thread switch successful
				current_fetch_thread = new_index;
				fetch_switch_when = Timing::getInstance()->getCycle();
				new_thread->setFetchStallUntil(Timing::getInstance()->getCycle() +
						CPU::getThreadSwitchPenalty() - 1);
			}
			else
			{
				// Thread switch fail
				break;
			}
		}

		// Fetch
		thread = threads[current_fetch_thread].get();
		if (thread->CanFetch())
			thread->Fetch();
		break;
	}

	default:

		throw misc::Panic("wrong fetch policy");
	}
}

}
