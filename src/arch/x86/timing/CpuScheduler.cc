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

#include "Cpu.h"
#include "Timing.h"


namespace x86
{

void Cpu::AllocateContext(Context *context)
{
	// Get core and thread that the context is mapped to
	Core *core = context->core;
	Thread *thread = context->thread;
	assert(core && thread);

	// Sanity
	assert(!thread->context);
	assert(context->getState(Context::StateMapped));
	assert(!context->getState(Context::StateAlloc));
	assert(!context->evict_signal);

	// Update context state
	context->allocate_cycle = getCycle();
	context->setState(Context::StateAlloc);

	// Update thread state
	thread->context = context;
	thread->setFetchNeip(context->getRegs().getEip());

	// Debug
	Emulator::context_debug << misc::fmt("@%lld Context %d "
			"allocated in Core %d Thread %d\n",
			getCycle(),
			context->getId(),
			core->getId(),
			thread->getIdInCore());

	// Trace
	Timing::trace << misc::fmt("x86.map_ctx "
			"ctx=%d "
			"core=%d "
			"thread=%d "
			"ppid=%d\n",
			context->getId(),
			core->getId(),
			thread->getIdInCore(),
			context->getParentId());
}


void Cpu::MapContext(Context *context)
{
	// Sanity
	assert(!context->getState(Context::StateAlloc));
	assert(!context->getState(Context::StateMapped));
	assert(!context->evict_signal);

	// From the hardware threads that the context has affinity with, find
	// the one with the smalled number of contexts mapped.
	Thread *found_thread = nullptr;
	for (int i = 0; i < getNumCores(); i++)
	{
		// Get core
		Core *core = getCore(i);

		// Traverse threads
		for (int j = 0; j < core->getNumThreads(); j++)
		{
			// Get thread
			Thread *thread = core->getThread(j);

			// Context does not have affinity with this thread
			if (!context->thread_affinity->Test(thread->getIdInCpu()))
				continue;

			// Check if this thread is better
			if (!found_thread || thread->getNumMappedContexts() <
					found_thread->getNumMappedContexts())
				found_thread = thread;
		}
	}

	// Final thread
	if (!found_thread)
		throw misc::Panic("No thread found with affinity to the context");
	
	// Map it to the thread
	found_thread->MapContext(context);
}


void Cpu::UpdateContextAllocationCycle()
{
	// Set it to the current cycle initially
	min_context_allocate_cycle = getCycle();

	// Traverse all cores
	for (int i = 0; i < getNumCores(); i++)
	{
		// Get core
		Core *core = getCore(i);

		// Traverse all threads
		for (int j = 0; j < core->getNumThreads(); j++)
		{
			// Get thread
			Thread *thread = core->getThread(j);
			Context *context = thread->context;

			// Update if necessary
			if (context && !context->evict_signal &&
					context->allocate_cycle <
					min_context_allocate_cycle)
				min_context_allocate_cycle =
						context->allocate_cycle;
		}
	}
}


void Cpu::Schedule()
{
	// Check if any context quantum could have expired
	bool quantum_expired = getCycle() >= min_context_allocate_cycle +
			context_quantum;

	// Check for quick scheduler end. The only way to effectively execute
	// the scheduler is that either a quantum expired or a signal to
	// reschedule has been flagged.
	Emulator *emulator = Emulator::getInstance();
	if (!quantum_expired && !emulator->schedule_signal)
		return;

	// Debug
	if (Emulator::context_debug)
	{
		Emulator::context_debug << misc::fmt("@%lld Schedule: ",
				getCycle());
		if (quantum_expired)
			Emulator::context_debug << misc::fmt("quantum expired "
					"(set at %lld)\n",
					min_context_allocate_cycle);
		else if (emulator->schedule_signal)
			Emulator::context_debug << "Emulator sent scheduling "
					"signal\n";
	}

	// We have to schedule. Uncheck the schedule signal here, since
	// upcoming actions might set it again for a second scheduler call.
	emulator->schedule_signal = false;

	// Check if there is any running context that is currently not mapped
	// to any node (core/thread); for example, a new context, or a
	// context that has changed its affinity.
	for (auto it = emulator->getRunningContextsBegin(),
			e = emulator->getRunningContextsEnd();
			it != e;
			++it)
	{
		// Get context
		Context *context = *it;

		// Check if not mapped
		if (!context->getState(Context::StateMapped))
			MapContext(context);
	}

	// Scheduling done individually for each hardware thread
	for (int i = 0; i < getNumCores(); i++)
	{
		Core *core = getCore(i);
		for (int j = 0; j < core->getNumThreads(); j++)
		{
			Thread *thread = core->getThread(j);
			thread->Schedule();
		}
	}

	// Update oldest allocation time of allocated contexts to determine
	// when is the next time the scheduler should be invoked.
	UpdateContextAllocationCycle();
}

}

