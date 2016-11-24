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


void Core::Dump(std::ostream &os) const
{
	// Dump all threads
	for (auto &thread : threads)
		os << *thread;
}


void Core::InsertInEventQueue(std::shared_ptr<Uop> uop, int latency)
{
	// Sanity
	assert(!uop->in_event_queue);

	// Set completion time for the instruction
	assert(!uop->completed);
	uop->complete_when = cpu->getCycle() + latency;

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


void Core::ExtractFromEventQueue(Uop *uop)
{
	// Uop must be in the queue
	assert(uop->in_event_queue);

	// Save iterator
	auto it = uop->event_queue_iterator;

	// Indicate that the uop is not in the queue anymore
	uop->in_event_queue = false;
	uop->event_queue_iterator = event_queue.end();

	// Remove it as the last step, as this may free the uop
	event_queue.erase(it);
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
			if (threads[i]->canFetch() == Thread::FetchStallUsed)
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
			if (thread->canFetch() == Thread::FetchStallUsed)
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


void Core::Dispatch()
{
	// Maximum number of threads to skip
	int skip = Cpu::getNumThreads();

	// Initially full quantum
	int quantum = Cpu::getDispatchWidth();

	// Invoke stage according to kind
	switch (Cpu::getDispatchKind())
	{

	case Cpu::DispatchKindShared:
	{
		do
		{
			// Next thread
			current_dispatch_thread = (current_dispatch_thread + 1)
					% Cpu::getNumThreads();
			Thread *thread = getThread(current_dispatch_thread);

			// Dispatch thread
			int remaining = thread->Dispatch(1);

			// Update remaining quantum and skipped threads
			if (remaining)
			{
				skip--;
			}
			else
			{
				skip = Cpu::getNumThreads();
				quantum--;
			}
		} while (quantum && skip);
		break;
	}

	case Cpu::DispatchKindTimeslice:
	{
		// Find a thread
		Thread *thread;
		do
		{
			// Next thread
			current_dispatch_thread = (current_dispatch_thread + 1)
					% Cpu::getNumThreads();
			thread = getThread(current_dispatch_thread);

			// Update skipped threads
			skip--;
		} while (skip && thread->canDispatch() != Thread::DispatchStallUsed);

		// Dispatch
		thread->Dispatch(quantum);
		break;
	}

	default:

		throw misc::Panic("Invalid dispatch kind");
	}
}


void Core::Issue()
{
	switch (Cpu::getIssueKind())
	{
	
	case Cpu::IssueKindShared:
	{
		// Issue load-store queue
		int quantum = Cpu::getIssueWidth();
		int skip = Cpu::getNumThreads();
		do
		{
			// Next thread
			current_issue_thread = (current_issue_thread + 1)
					% Cpu::getNumThreads();

			// Issue instruction
			Thread *thread = getThread(current_issue_thread);
			quantum = thread->IssueLoadStoreQueue(quantum);

			// Update skipped threads
			skip--;
		} while (skip && quantum);

		// Issue IQs
		quantum = Cpu::getIssueWidth();
		skip = Cpu::getNumThreads();
		do
		{
			// Next thread
			current_issue_thread = (current_issue_thread + 1)
					% Cpu::getNumThreads();

			// Issue instruction
			Thread *thread = getThread(current_issue_thread);
			quantum = thread->IssueInstructionQueue(quantum);

			// Update skipped threads
			skip--;
		} while (skip && quantum);
	
		// Done
		break;
	}
	
	case Cpu::IssueKindTimeslice:
	{
		// Issue load-store queue
		int quantum = Cpu::getIssueWidth();
		int skip = Cpu::getNumThreads();
		do
		{
			// Next thread
			current_issue_thread = (current_issue_thread + 1)
					% Cpu::getNumThreads();

			// Issue instruction
			Thread *thread = getThread(current_issue_thread);
			quantum = thread->IssueLoadStoreQueue(quantum);

			// Update skipped threads
			skip--;
		} while (skip && quantum == Cpu::getIssueWidth());

		// Issue IQs
		quantum = Cpu::getIssueWidth();
		skip = Cpu::getNumThreads();
		do
		{
			// Next thread
			current_issue_thread = (current_issue_thread + 1)
					% Cpu::getNumThreads();

			// Issue instruction
			Thread *thread = getThread(current_issue_thread);
			quantum = thread->IssueInstructionQueue(quantum);

			// Update skipped threads
			skip--;
		} while (skip && quantum == Cpu::getIssueWidth());
	
		// Done
		break;
	}

	default:

		throw misc::Panic("Invalid issue kind");
	}
}


void Core::Writeback()
{
	// Traverse event queue
	for (;;)
	{
		// No more elements in the event queue
		if (event_queue.empty())
			break;

		// Pick uop from the head of the event queue
		std::shared_ptr<Uop> uop = event_queue.front();

		// If the uop is set to complete later than the current cycle,
		// there is nothing else to extract from the event queue.
		if (uop->complete_when > cpu->getCycle())
			break;

		// Sanity
		assert(uop->ready);
		assert(!uop->completed);

		// Extract element from event queue
		ExtractFromEventQueue(uop.get());

		// If this instruction is the first in speculative mode
		// (typically a mispredicted branch), and recovery is configured
		// to happen at writeback, schedule recovery.
		//
		// NOTE - This used to check whether the current instruction
		// is a mispredicted branch, instead of flag 'first_spec_mode'.
		// This code was modified because the previous approach didn't
		// consider the case where a wrong hit in the trace cache was
		// fetching the wrong instruction.
		//
		bool recover = Cpu::getRecoverKind() == Cpu::RecoverKindWriteback
				&& uop->first_speculative_mode;


		// Trace output. Prevent instructions that are not in the
		// reorder buffer from writing to the trace. These can be either
		// loads that were squashed, or stores that committed before
		// being issued.
		if (uop->in_reorder_buffer)
			Timing::trace << misc::fmt("x86.inst "
					"id=%lld "
					"core=%d "
					"stg=\"wb\"\n",
					uop->getIdInCore(),
					id);

		// Instruction has completed
		uop->completed = true;

		// Write output registers
		Thread *thread = uop->getThread();
		RegisterFile *register_file = thread->getRegisterFile();
		register_file->WriteUop(uop.get());

		// Increment number of writes to core's register counters
		num_integer_register_writes += uop->getNumIntegerOutputs();
		num_floating_point_register_writes += uop->getNumFloatingPointOutputs();
		num_xmm_register_writes += uop->getNumXmmOutputs();

		// Increment number of writes to thread's register counters
		thread->incNumIntegerRegisterWrites(uop->getNumIntegerOutputs());
		thread->incNumFloatingPointRegisterWrites(uop->getNumFloatingPointOutputs());
		thread->incNumXmmRegisterWrites(uop->getNumXmmOutputs());
		
		// Recover from mispeculation
		if (recover)
			thread->Recover();
	}
}


void Core::Commit()
{
	// Check type of commit
	switch (Cpu::getCommitKind())
	{

	case Cpu::CommitKindShared:
	{
		int skip = Cpu::getNumThreads();
		int quantum = Cpu::getCommitWidth();
		while (quantum && skip)
		{
			// Check next thread
			current_commit_thread = (current_commit_thread + 1)
					% Cpu::getNumThreads();
			
			// Check if we can commit from thread
			Thread *thread = threads[current_commit_thread].get();
			if (thread->canCommit())
			{
				// Commit one instruction
				thread->Commit(1);
				quantum--;
				skip = Cpu::getNumThreads();
			}
			else
			{
				// One thread skipped
				skip--;
			}
		}

		// Done
		break;
	}

	case Cpu::CommitKindTimeslice:
	{
		// Find thread to commit instructions from
		int next_thread = current_commit_thread;
		while (true)
		{
			// Go to next thread
			next_thread = (next_thread + 1) % Cpu::getNumThreads();

			// Already cycled through
			if (next_thread == current_commit_thread)
				break;

			// Found a thread to commit instructions from
			if (threads[next_thread]->canCommit())
				break;
		}
		
		// Commit new thread
		current_commit_thread = next_thread;
		Thread *thread = threads[current_commit_thread].get();

		// Commit if we can
		if (thread->canCommit())
			thread->Commit(Cpu::getCommitWidth());

		// Done
		break;
	}

	default:

		throw misc::Panic("Invalid commit kind");
	}
}


void Core::Run()
{
	// Run stages in reverse order
	Commit();
	Writeback();
	Issue();
	Dispatch();
	Decode();
	Fetch();
}

}

