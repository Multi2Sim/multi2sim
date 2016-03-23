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


// This file contains the implementation of the x86 context scheduler. The
// following definitions are assumed in the description of the algorithm:
//
//   - A node is a pair core/thread where a context can run.
//   - Map a context to a node: associate a context with a node (core/thread).
//     This association is done once when the context is created, or during
//     context migration (e.g., when it changes its thread affinity bitmap). A
//     node has a list of mapped contexts.
//   - Unmap a context: remove association between a context and a node. The
//     context is removed from the node's list of mapped contexts.
//   - Allocate a context: select the context from the node's list of mapped
//     context and start effectively executing it, allocating pipeline resources
//     in the node.
//   - Evict a context: Deallocate pipeline resources from the executing
//     context, but still keep it in the node's list of mapped contexts.
//
// The following is a description of the implemented scheduling algorithm,
// implemented in function Cpu::Schedule():
//
//   - Only start running if a schedule signal has been received (variable
//     'emu->schedule_signal' is set, or one of the allocated contexts has
//     exhaused its quantum. A schedule signal is triggered every time a
//     context changes any of its state bits.
//
//   - Uncheck 'Emulator::schedule_signal' at this point to allow any of the
//     subsequent actions to force the scheduler to run in the next cycle.
//
//   - Check the list of running contexts for any unmapped context. Map them by
//     selecting the node that has the lowest number of contexts currently mapped
//     to it. The context will always execute on that node, unless it changes its
//     affinity.
//
//   - For each node:
//
//         - If the allocated context is not in 'Running' state, signal eviction.
//
//         - If the allocated context has exhausted its quantum, signal eviction.
//           As an exception, the context will continue running if there is no
//           other candidate in the mapped context list; in this case, the running
//           context resets its quantum to keep the scheduler from trying to
//           evict the context right away.
//
//         - If the allocated context lost affinity with the node, signal
//           eviction.
//
//         - If any mapped context other than the allocated context lost
//           affinity with the node, unmap it.
//
//         - If any mapped context other than the allocated context finished
//           execution, unmap it and free it.
//
//         - If there is no allocated context, search the node's list of mapped
//           contexts for the context in state running and with valid affinity
//           that was evicted least recently. Allocate it, if found.
//
//   - Update global variable 'min_alloc_cycle' with the cycle of the least
//     recently allocated context. The scheduler needs to be called again due
//     to quantum expiration only when this variable indicates so.
//

#include "Cpu.h"
#include "Thread.h"
#include "Timing.h"


namespace x86
{

void Thread::MapContext(Context *context)
{
	// Sanity
	assert(context);
	assert(!context->core);
	assert(!context->thread);
	assert(!context->getState(Context::StateMapped));
	assert(!context->getState(Context::StateAlloc));

	// Update context state
	context->core = core;
	context->thread = this;
	context->setState(Context::StateMapped);

	// Add context to the node's mapped list
	context->mapped_contexts_iterator = mapped_contexts.insert(
			mapped_contexts.end(), context);

	// Debug
	Emulator::context_debug << misc::fmt("@%lld Context %d mapped "
			"to Core %d Thread %d\n",
			cpu->getCycle(),
			context->getId(),
			core->getId(),
			getIdInCore());
}


void Thread::UnmapContext(Context *context)
{
	// Sanity
	assert(context);
	assert(context->thread);
	assert(context->getState(Context::StateMapped));
	assert(!context->getState(Context::StateAlloc));
	assert(this->context != context);

	// Update context state
	context->clearState(Context::StateMapped);

	// Remove context from thread's mapped list
	mapped_contexts.erase(context->mapped_contexts_iterator);
	context->core = nullptr;
	context->thread = nullptr;

	// Debug
	Emulator::context_debug << misc::fmt("@%lld Context %d unmapped "
			"from thread %s\n",
			cpu->getCycle(),
			context->getId(),
			name.c_str());

	// If context has finished, free it
	if (context->getState(Context::StateFinished))
	{
		// Trace
		Timing::trace << misc::fmt("x86.end_ctx "
				"ctx=%d\n",
				context->getId());

		// Free context
		Emulator *emulator = Emulator::getInstance();
		emulator->FreeContext(context);
	}
}


void Thread::EvictContextSignal()
{
	// Sanity
	assert(context);
	assert(context->getState(Context::StateAlloc));
	assert(context->getState(Context::StateMapped));
	assert(!context->evict_signal);
	assert(context->thread == this);
	assert(context->core = core);

	// Set eviction signal
	context->evict_signal = true;

	// Debug
	Emulator::context_debug << misc::fmt("@%lld Context %d signaled for "
			"eviction from thread %s\n",
			cpu->getCycle(),
			context->getId(),
			name.c_str());

	// If pipeline is already empty for the thread, effective eviction can
	// happen right away.
	if (isPipelineEmpty())
		EvictContext();
}


void Thread::EvictContext()
{
	// Sanity
	assert(context);
	assert(context->getState(Context::StateAlloc));
	assert(context->getState(Context::StateMapped));
	assert(!context->getState(Context::StateSpecMode));
	assert(reorder_buffer.empty());
	assert(context->evict_signal);

	// Update context state
	context->clearState(Context::StateAlloc);
	context->evict_cycle = cpu->getCycle();
	context->evict_signal = 0;

	// Debug
	Emulator::context_debug << misc::fmt("@%lld Context %d evicted "
			"from Core %d Thread %d\n",
			cpu->getCycle(),
			context->getId(),
			core->getId(),
			getIdInCore());

	// Trace
	Timing::trace << misc::fmt("x86.unmap_ctx "
			"ctx=%d "
			"core=%d "
			"thread=%d\n",
			context->getId(),
			core->getId(),
			id_in_core);
	
	// Update thread state
	context = nullptr;
	fetch_neip = 0;
}


void Thread::Schedule()
{
	// Actions for the context allocated to this thread
	if (context)
	{
		// Sanity
		assert(context->getState(Context::StateAlloc));
		assert(context->getState(Context::StateMapped));

		// Context not in 'running' state
		if (!context->evict_signal && !context->getState(Context::StateRunning))
		{
			// Debug
			Emulator::context_debug << misc::fmt(
					"@%lld Context %d "
					"in Core %d Thread %d not "
					"in Running state anymore\n",
					cpu->getCycle(),
					context->getId(),
					core->getId(),
					getIdInCore());

			// Evict context
			EvictContextSignal();
		}

		// Context lost affinity with the thread
		if (!context->evict_signal && !context->thread_affinity->Test(id_in_cpu))
		{
			// Debug
			Emulator::context_debug << misc::fmt(
					"@%lld Context %d "
					"lost affinity with Core %d "
					"Thread %d - rescheduling\n",
					cpu->getCycle(),
					context->getId(),
					core->getId(),
					getIdInCore());
			
			// Evict context
			EvictContextSignal();
		}

		// Context quantum expired
		if (!context->evict_signal && cpu->getCycle()
				>= context->allocate_cycle
				+ Cpu::getContextQuantum())
		{
			// Debug
			Emulator::context_debug << misc::fmt("@%lld Context "
					"%d quantum expired\n",
					cpu->getCycle(),
					context->getId());

			// If there are no other contexts to run on this thread,
			// allocate a new quantum and return
			assert(mapped_contexts.size() >= 1);
			if (mapped_contexts.size() == 1)
			{
				// Debug
				Emulator::context_debug << misc::fmt(
						"\tOnly context %d mapped\n",
						context->getId());
				
				// Renew quantum
				assert(mapped_contexts.front() == context);
				context->allocate_cycle = cpu->getCycle();
				return;
			}

			// Find a running context mapped to the same node
			bool found = false;
			for (Context *temp_context : mapped_contexts)
			{
				// Debug
				Emulator::context_debug << misc::fmt(
						"\tCandidate context %d (%s)\n",
						temp_context->getId(),
						Context::StateMap.MapFlags(
						temp_context->getState()).c_str());

				// Check if candidate is valid
				if (temp_context != context &&
						temp_context->getState(Context::StateRunning) &&
						temp_context->sched_priority >= context->sched_priority)
				{
					// Debug
					Emulator::context_debug << "\tFound\n";

					// Found
					found = true;
					break;
				}
			}

			// If a context was found, there are other candidates
			// for allocation in the node. We need to evict the
			// current context. If there are no other running
			// candidates, there is no need to evict. But we
			// update the allocation time, so that the
			// scheduler is not called constantly hereafter.
			if (found)
				EvictContextSignal();
			else
				context->allocate_cycle = cpu->getCycle();
		}

		// Context quantum has not expired, but another thread
		// of higher priority may interrupt it.
		else if (!context->evict_signal && cpu->getCycle()
				< context->allocate_cycle
				+ Cpu::getContextQuantum())
		{
			// Debug
			Emulator::context_debug << misc::fmt("@%lld Context %d "
					"interrupted\n",
					cpu->getCycle(),
					context->getId());

			// Find a running context mapped to the same node
			bool found = false;
			for (Context *temp_context : mapped_contexts)
			{
				// Debug
				Emulator::context_debug << misc::fmt(
						"\tContext %d "
						"is a candidate\n",
						temp_context->getId());
				Emulator::context_debug << misc::fmt(
						"\t\tPriority = %d, "
						"state = %s\n",
						temp_context->sched_priority,
						Context::StateMap.MapFlags(
						temp_context->getState()).c_str());

				// Check if candidate is valid
				if (temp_context != context &&
						temp_context->getState(Context::StateRunning) &&
						temp_context->sched_priority > context->sched_priority)
				{
					// Debug
					Emulator::context_debug << "\tFound\n";

					// Found
					found = true;
					break;
				}
			}

			// If a context was found, there are other candidates
			// for allocation in the node. We need to evict the
			// current context. If there are no other running
			// candidates, there is no need to evict. But we
			// update the allocation time, so that the
			// scheduler is not called constantly hereafter. 
			// Do not update the quantum if the current thread 
			// is not evicted. This would lead to a livelock 
			// in situations where the current thread is always
			// interrupted before its quantum expires and there
			// are only threads of equal priority to run.
			if (found)
			{
				// Debug
				Emulator::context_debug << misc::fmt(
						"\tContext %d begin evicted\n",
						context->getId());

				// Signal eviction
				EvictContextSignal();
			}
			else
			{
				// Debug
				Emulator::context_debug << misc::fmt(
						"\tContext %d continuing\n",
						context->getId());
			}
		}
	}

	// Actions for mapped contexts, other than the allocated context, if any
	for (auto it = mapped_contexts.begin(),
			e = mapped_contexts.end();
			it != e;
			// No increment
			)
	{
		// Get current context. Increment the iterator here, since
		// removing the context from the mapped list will invalidate it
		// Ignore the currently allocated context
		Context *temp_context = *it;
		++it;

		// Ignore currently allocated context
		if (temp_context == context)
			continue;

		// Unmap a context if it lost affinity with the node or if it
		// finished execution.
		if (!temp_context->thread_affinity->Test(id_in_cpu) ||
				temp_context->getState(Context::StateFinished))
			UnmapContext(temp_context);
	}

	// If thread is available, try to allocate a context mapped to it.
	if (!context)
	{
		// Search the mapped context with the oldest 'evict_cycle'
		// that is state 'Running' and has affinity with the node.
		Context *allocate_context = nullptr;
		for (Context *temp_context : mapped_contexts)
		{
			// Debug
			Emulator::context_debug << misc::fmt("@%lld Context %d "
					"(priority %d)\n",
					cpu->getCycle(),
					temp_context->getId(),
					temp_context->sched_priority);
			
			// No affinity
			if (!temp_context->thread_affinity->Test(id_in_cpu))
				continue;

			// Not running
			if (!temp_context->getState(Context::StateRunning))
				continue;

			// Good candidate
			if (!allocate_context || (allocate_context->evict_cycle
					> temp_context->evict_cycle
					&& temp_context->sched_priority
					>= allocate_context->sched_priority))
			{
				// Select candidate
				allocate_context = temp_context;

				// Debug
				Emulator::context_debug << misc::fmt(
						"@%lld Context %d "
						"(priority %d) "
						"is a candidate\n",
						cpu->getCycle(),
						allocate_context->getId(), 
						allocate_context->sched_priority);
			}
			else
			{
				// Debug
				Emulator::context_debug << misc::fmt(
						"@%lld Context %d "
						"(priority %d) "
						"is not a candidate\n",
						cpu->getCycle(),
						temp_context->getId(),
						temp_context->sched_priority);
			}
		}

		// Allocate context if found
		if (allocate_context)
		{
			// Allocate it
			cpu->AllocateContext(allocate_context);

			// Debug
			Emulator::context_debug << misc::fmt(
					"Allocating context %d\n",
					allocate_context->getId());
		}
	}
}

}

