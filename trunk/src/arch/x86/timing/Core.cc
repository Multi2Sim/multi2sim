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

	// Create threads
	threads.reserve(Cpu::getNumThreads());
	for (int i = 0; i < Cpu::getNumThreads(); i++)
		threads.emplace_back(misc::new_unique<Thread>(this, i));
}


void Core::InsertInEventQueue(std::shared_ptr<Uop> uop)
{
	// Sanity
	assert(!uop->in_event_queue);
	assert(uop->event_queue_iterator == event_queue.end());

	// Find position in event queue
	auto it = event_queue.begin();
	auto e = event_queue.end();
	while (it != e)
	{
		// Check if position found
		if (uop->Compare(it->get()) < 0)
			break;

		// Next
		++it;
	}

	// Insert
	uop->event_queue_iterator = event_queue.insert(it, uop);
	uop->in_event_queue = true;
}


std::shared_ptr<Uop> Core::ExtractFromEventQueue()
{
	// If the queue is empty, return a null pointer
	if (event_queue.size() <= 0)
		return nullptr;

	// Get the first element of the queue
	std::shared_ptr<Uop> uop = event_queue.front();
	assert(uop->in_event_queue);
	assert(uop->event_queue_iterator == event_queue.end());

	// Remove the uop
	event_queue.pop_front();

	// Set flag to indicate that the uop is not in the queue anymore
	uop->in_event_queue = false;
	uop->event_queue_iterator = event_queue.end();

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
			if (threads[i]->canFetch())
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
			if (thread->canFetch())
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


void Core::Decode()
{
	// Run decode stage for all threads
	for (auto &thread : threads)
		thread->Decode();
}


void Core::Run()
{
	Decode();
	Fetch();
}

}

