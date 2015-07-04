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

#include "Core.h"
#include "Cpu.h"
#include "Timing.h"


namespace x86
{

Core::Core(Cpu *cpu,
		int id) :
		cpu(cpu),
		id(id)
{
	// Assign name
	name = misc::fmt("Core %d", id);

	// Initialize ROB
	InitializeReorderBuffer();

	// Initialize functional unit
	functional_unit = misc::new_unique<FunctionalUnit>();

	// Create threads
	threads.reserve(Cpu::getNumThreads());
	for (int i = 0; i < Cpu::getNumThreads(); i++)
		threads.emplace_back(misc::new_unique<Thread>(this, i));
}


void Core::InitializeReorderBuffer()
{
	// Create empty reorder buffer
	reorder_buffer_total_size = Cpu::getReorderBufferSize() * Cpu::getNumThreads();
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
	switch (Cpu::getReorderBufferKind())
	{
	case Cpu::ReorderBufferKindPrivate:

		// In thread domain
		if (uop->getThread()->getUopCountInRob() < Cpu::getReorderBufferSize())
			return true;
		break;

	case Cpu::ReorderBufferKindShared:

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
	switch (Cpu::getReorderBufferKind())
	{

	case Cpu::ReorderBufferKindPrivate:

		// In thread domain
		assert(uop->getThread()->getUopCountInRob() < Cpu::getReorderBufferSize());
		assert(!reorder_buffer[uop->getThread()->getReorderBufferTail()]);
		reorder_buffer[uop->getThread()->getReorderBufferTail()].reset(uop);
		uop->getThread()->incReorderBufferTail();
		uop->getThread()->incUopCountInRob();
		break;

	case Cpu::ReorderBufferKindShared:

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

	// Instruction is in the ROB
	uop->setInReorderBuffer(true);
}


bool Core::CanDequeueFromReorderBuffer(int thread_id)
{
	// Local variable declaration
	Uop *uop;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id);

	// Check according to ROB kind
	switch (Cpu::getReorderBufferKind())
	{

	case Cpu::ReorderBufferKindPrivate:

		// In thread domain
		if (thread->getUopCountInRob() > 0)
			return true;

		break;

	case Cpu::ReorderBufferKindShared:

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
	switch (Cpu::getReorderBufferKind())
	{
	case Cpu::ReorderBufferKindPrivate:

		// In thread domain
		if (thread->getUopCountInRob() > 0)
		{
			uop = reorder_buffer[thread->getReorderBufferHead()].get();
			return uop;
		}
		break;

	case Cpu::ReorderBufferKindShared:

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
	switch (Cpu::getReorderBufferKind())
	{
	case Cpu::ReorderBufferKindPrivate:

		// In thread domain
		assert(thread->getUopCountInRob() > 0);
		uop = reorder_buffer[thread->getReorderBufferHead()].get();
		assert(uop->getThread() == thread);
		reorder_buffer[thread->getReorderBufferHead()].reset();
		thread->incReorderBufferHead();
		thread->decUopCountInRob();
		break;

	case Cpu::ReorderBufferKindShared:

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
	switch (Cpu::getReorderBufferKind())
	{
	case Cpu::ReorderBufferKindPrivate:

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

	case Cpu::ReorderBufferKindShared:

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
	switch (Cpu::getReorderBufferKind())
	{

	case Cpu::ReorderBufferKindPrivate:

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

	case Cpu::ReorderBufferKindShared:

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

	switch (Cpu::getReorderBufferKind())
	{
	case Cpu::ReorderBufferKindPrivate:

		index += thread->getReorderBufferHead();
		if (index > thread->getReorderBufferRightBound())
			index = index - thread->getReorderBufferRightBound()
				+ thread->getReorderBufferLeftBound() - 1;
		uop = reorder_buffer[index].get();
		assert(uop);
		return uop;

	case Cpu::ReorderBufferKindShared:

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
	switch (Cpu::getFetchKind())
	{

	case Cpu::FetchKindShared:
	{
		// Fetch from all threads
		for (int i = 0; i < Cpu::getNumThreads(); i++)
			if (threads[i]->CanFetch())
				threads[i]->Fetch();
		break;
	}

	case Cpu::FetchKindTimeslice:
	{
		// Round-robin fetch
		for (int i = 0; i < Cpu::getNumThreads(); i++)
		{
			// Next thread
			current_fetch_thread = (current_fetch_thread + 1)
					% Cpu::getNumThreads();

			// Try to fetch from this thread
			Thread *thread = threads[current_fetch_thread].get();
			if (thread->CanFetch())
			{
				thread->Fetch();
				break;
			}
		}

		// Done
		break;
	}

	default:

		throw misc::Panic("Invalid fetch policy");
	}
}


void Core::Run()
{
	Fetch();
}

}

