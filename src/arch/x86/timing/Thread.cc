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
#include "Thread.h"
#include "TraceCache.h"


namespace x86
{

const misc::StringMap Thread::fetch_stall_map =
{
	{ "Invalid", FetchStallInvalid },
	{ "Used", FetchStallUsed },
	{ "Context", FetchStallContext },
	{ "Suspended", FetchStallSuspended },
	{ "FetchQueue", FetchStallFetchQueue },
	{ "InstructionMemory", FetchStallInstructionMemory }
};


const misc::StringMap Thread::dispatch_stall_map =
{
	{ "Invalid", DispatchStallInvalid },
	{ "Used", DispatchStallUsed },
	{ "Speculative", DispatchStallSpeculative },
	{ "UopQueue", DispatchStallUopQueue },
	{ "ReorderBuffer", DispatchStallReorderBuffer },
	{ "InstructionQueue", DispatchStallInstructionQueue },
	{ "LoadStoreQueue", DispatchStallLoadStoreQueue },
	{ "Rename", DispatchStallRename },
	{ "Context", DispatchStallContext }
};


Thread::Thread(Core *core,
		int id_in_core) :
		core(core),
		id_in_core(id_in_core)
{
	// Assign name
	name = misc::fmt("Core %d Thread %d", core->getId(), id_in_core);

	// Assign CPU
	cpu = core->getCpu();

	// Global index in CPU
	id_in_cpu = core->getId() * Cpu::getNumThreads() + id_in_core;

	// Initialize branch predictor
	branch_predictor = misc::new_unique<BranchPredictor>(name +
			".BranchPredictor");

	// Initialize trace cache
	if (TraceCache::isPresent())
		trace_cache = misc::new_unique<TraceCache>(name +
				".TraceCache");

	// Initialize register file
	register_file = misc::new_unique<RegisterFile>(this);
}


void Thread::Dump(std::ostream &os) const
{
	// Title
	std::string title = misc::fmt("Core %d - Thread %d",
			id_in_core, core->getId());
	os << title << '\n';
	os << std::string(title.length(), '=') << "\n\n";

	// Register file
	if (register_file)
		os << *register_file;

	// Queues
	DumpFetchQueue(os);
	DumpUopQueue(os);
	DumpReorderBuffer(os);
	DumpInstructionQueue(os);
	DumpLoadStoreQueue(os);

	// Done
	os << "\n\n";
}


void Thread::InsertInFetchQueue(std::shared_ptr<Uop> uop)
{
	// Sanity
	assert(!uop->in_fetch_queue);

	// Insert in queue
	uop->in_fetch_queue = true;
	uop->fetch_queue_iterator = fetch_queue.insert(fetch_queue.end(), uop);

	// Increase occupancy of fetch queue or trace queue
	if (uop->from_trace_cache)
	{
		// Trace queue occupancy is increased by 1 for each uop
		// inserted in it.
		trace_cache_queue_occupancy++;
	}
	else
	{
		// Fetch queue occupancy is increased by the number of bytes
		// of the associated macro-instruction, only for the first uop
		// in each macro-instruction.
		if (uop->mop_index == 0)
			fetch_queue_occupancy += uop->mop_size;
	}
}


void Thread::ExtractFromFetchQueue(Uop *uop)
{
	// Sanity: uop must be in the fetch queue, and must be either the first
	// or the last element in it.
	assert(uop->in_fetch_queue);
	assert(uop->fetch_queue_iterator != fetch_queue.end());
	assert(fetch_queue.size() > 0);
	assert(uop == fetch_queue.front().get() ||
			uop == fetch_queue.back().get());
	
	// Save iterator
	auto it = uop->fetch_queue_iterator;

	// Mark uop as extracted
	uop->in_fetch_queue = false;
	uop->fetch_queue_iterator = fetch_queue.end();

	// Decrease occupancy of fetch queue or trace queue
	if (uop->from_trace_cache)
	{
		// Trace queue occupancy is decreased by 1 for each uop
		// extracted from it.
		assert(trace_cache_queue_occupancy > 0);
		trace_cache_queue_occupancy--;
	}
	else
	{
		// Fetch queue occupancy is decreased by the number of bytes
		// of the associated macro-instruction, only for the first
		// uop in each macro-instruction.
		if (uop->mop_index == 0)
		{
			assert(fetch_queue_occupancy >= uop->mop_size);
			fetch_queue_occupancy -= uop->mop_size;
		}
	}

	// Extract uop as last step, since uop may be freed here
	fetch_queue.erase(it);
}


void Thread::DumpFetchQueue(std::ostream &os) const
{
	// Title
	std::string title = "Fetch queue";
	os << title << '\n';
	os << std::string(title.size(), '-') << "\n\n";

	// Dump content
	int index = 0;
	for (auto &uop : fetch_queue)
	{
		os << misc::fmt("%3d. ", index);
		os << *uop << '\n';
		index++;
	}

	// Empty list
	if (fetch_queue.empty())
		os << "-Empty-\n";

	// End
	os << '\n';
}


void Thread::InsertInUopQueue(std::shared_ptr<Uop> uop)
{
	assert(!uop->in_uop_queue);
	uop->in_uop_queue = true;
	uop->uop_queue_iterator = uop_queue.insert(uop_queue.end(), uop);
}


void Thread::ExtractFromUopQueue(Uop *uop)
{
	// Sanity: uop must be in the uop queue, and must be either the first
	// or the last element in it.
	assert(uop->in_uop_queue);
	assert(uop_queue.size() > 0);
	assert(uop == uop_queue.front().get() || uop_queue.back().get());

	// Save iterator
	auto it = uop->uop_queue_iterator;

	// Mark uop as extracted
	uop->in_uop_queue = false;
	uop->uop_queue_iterator = uop_queue.end();

	// Extract uop as last step, since this may free it
	uop_queue.erase(it);
}


void Thread::DumpUopQueue(std::ostream &os) const
{
	// Title
	std::string title = "Uop queue";
	os << title << '\n';
	os << std::string(title.size(), '-') << "\n\n";

	// Dump content
	int index = 0;
	for (auto &uop : uop_queue)
	{
		os << misc::fmt("%3d. ", index);
		os << *uop << '\n';
		index++;
	}

	// Empty list
	if (uop_queue.empty())
		os << "-Empty-\n";

	// End
	os << '\n';
}


bool Thread::canInsertInReorderBuffer()
{
	switch (Cpu::getReorderBufferKind())
	{

	case Cpu::ReorderBufferKindPrivate:

		// Return whether the number of instructions in this thread's
		// ROB is smaller than the ROB size configured by the user,
		// which is specified as a per-thread ROB size.
		return (int) reorder_buffer.size() <
				Cpu::getReorderBufferSize();

	case Cpu::ReorderBufferKindShared:

		// Return whether the number of instructions in all threads'
		// ROBs is smaller than the ROB size selected by the user,
		// which is specified as the total ROB size.
		return core->getReorderBufferOccupancy() <
				Cpu::getReorderBufferSize();

	default:

		throw misc::Panic("Invalid reorder buffer kind");
	}
}


void Thread::InsertInReorderBuffer(std::shared_ptr<Uop> uop)
{
	// Sanity
	assert(!uop->in_reorder_buffer);

	// Insert into reorder buffer
	uop->in_reorder_buffer = true;
	uop->reorder_buffer_iterator = reorder_buffer.insert(reorder_buffer.end(), uop);

	// Increase per-core counter
	core->incReorderBufferOccupancy();
}


void Thread::ExtractFromReorderBuffer(Uop *uop)
{
	// Sanity: uop must be in the reorder buffer, and must be either the
	// first or the last instruction in that queue.
	assert(uop->in_reorder_buffer);
	assert(reorder_buffer.size() > 0);
	assert(uop == reorder_buffer.front().get() || reorder_buffer.back().get());

	// Save iterator
	auto it = uop->reorder_buffer_iterator;

	// Mark uop as extracted
	uop->in_reorder_buffer = false;
	uop->reorder_buffer_iterator = reorder_buffer.end();

	// Extract uop as last step, since this may free it
	reorder_buffer.erase(it);

	// Decrease per-core counter
	core->decReorderBufferOccupancy();
}


void Thread::DumpReorderBuffer(std::ostream &os) const
{
	// Current cycle
	long long cycle = cpu->getCycle();

	// Title
	std::string title = "Reorder buffer";
	os << title << '\n';
	os << std::string(title.size(), '-') << "\n\n";

	// Dump content
	int index = 0;
	for (auto &uop : reorder_buffer)
	{
		// Instruction
		os << misc::fmt("%3d. ", index);
		os << *uop << '\n';
		index++;

		// Dispatched
		if (uop->dispatched)
			os << misc::fmt("\t\tDispatched in cycle %lld (%lld ago)\n",
					uop->dispatch_when,
					cycle - uop->dispatch_when);
		else
			os << "\t\tNot dispatched\n";

		// Issued
		if (uop->issued)
			os << misc::fmt("\t\tIssued in cycle %lld (%lld ago)\n",
					uop->issue_when,
					cycle - uop->issue_when);
		else
			os << "\t\tNot issued\n";

		// Issued
		if (uop->completed)
			os << misc::fmt("\t\tCompleted in cycle %lld (%lld ago)\n",
					uop->complete_when,
					cycle - uop->complete_when);
		else
			os << "\t\tNot completed\n";
	}

	// Empty list
	if (reorder_buffer.empty())
		os << "-Empty-\n";

	// End
	os << '\n';
}


bool Thread::canInsertInInstructionQueue()
{
	switch (Cpu::getInstructionQueueKind())
	{

	case Cpu::InstructionQueueKindPrivate:

		// Return whether the number of instructions in this thread's IQ
		// is smaller than the IQ size configured by the user, which is
		// specified as a per-thread IQ size.
		return (int) instruction_queue.size() <
				Cpu::getInstructionQueueSize();

	case Cpu::InstructionQueueKindShared:

		// Return whether the number of instructions in all threads'
		// IQs is smaller than the IQ size selected by the user,
		// which is specified as the total IQ size.
		return core->getInstructionQueueOccupancy() <
				Cpu::getInstructionQueueSize();
	
	default:

		throw misc::Panic("Invalid instruction queue kind");
	}
}


void Thread::InsertInInstructionQueue(std::shared_ptr<Uop> uop)
{
	// Sanity
	assert(!uop->in_instruction_queue);
	assert(!uop->in_load_queue);
	assert(!uop->in_store_queue);

	// Insert into instruction queue
	uop->in_instruction_queue = true;
	uop->instruction_queue_iterator = instruction_queue.insert(
			instruction_queue.end(), uop);

	// Increase per-core counter
	core->incInstructionQueueOccupancy();
}


void Thread::ExtractFromInstructionQueue(Uop *uop)
{
	// Sanity: instruction must be in the queue
	assert(!uop->in_load_queue);
	assert(!uop->in_store_queue);
	assert(uop->in_instruction_queue);

	// Save iterator
	auto it = uop->instruction_queue_iterator;

	// Mark uop as not present
	uop->in_instruction_queue = false;
	uop->instruction_queue_iterator = instruction_queue.end();
	
	// Remove from queue as the last step, as this may free the uop
	instruction_queue.erase(it);

	// Decrease per-core counter
	core->decInstructionQueueOccupancy();
}


void Thread::DumpInstructionQueue(std::ostream &os) const
{
	// Title
	std::string title = "Instruction queue";
	os << title << '\n';
	os << std::string(title.size(), '-') << "\n\n";

	// Dump content
	int index = 0;
	for (auto &uop : instruction_queue)
	{
		os << misc::fmt("%3d. ", index);
		os << *uop << '\n';
		index++;
	}

	// Empty list
	if (instruction_queue.empty())
		os << "-Empty-\n";

	// End
	os << '\n';
}


bool Thread::canInsertInLoadStoreQueue()
{
	switch (Cpu::getLoadStoreQueueKind())
	{

	case Cpu::LoadStoreQueueKindPrivate:

		// Return whether the number of instructions in this thread's
		// LSQ is smaller than the IQ size configured by the user, which
		// is specified as a per-thread LSQ size
		return (int) (load_queue.size() + store_queue.size()) <
				Cpu::getLoadStoreQueueSize();

	case Cpu::LoadStoreQueueKindShared:

		// Return whether the number of instructions in all threads'
		// LSQs is smaller than the LSQ size selected by the user,
		// which is specified as the total LSQ size.
		return core->getLoadStoreQueueOccupancy() <
				Cpu::getLoadStoreQueueSize();
	
	default:

		throw misc::Panic("Invalid load-store queue kind");
	}
}


void Thread::InsertInLoadStoreQueue(std::shared_ptr<Uop> uop)
{
	// Sanity
	assert(!uop->in_load_queue);
	assert(!uop->in_store_queue);

	// Insert into load or store queue depending on instruction type
	switch (uop->getUinst()->getOpcode())
	{

	case Uinst::OpcodeLoad:

		uop->load_queue_iterator = load_queue.insert(load_queue.end(), uop);
		uop->in_load_queue = true;
		break;

	case Uinst::OpcodeStore:

		uop->store_queue_iterator = store_queue.insert(store_queue.end(), uop);
		uop->in_store_queue = true;
		break;
	
	default:

		throw misc::Panic("Invalid micro-instruction opcode");
	}

	// Increase per-core counter
	core->incLoadStoreQueueOccupancy();
}


void Thread::ExtractFromLoadQueue(Uop *uop)
{
	// Uop must be in the queue
	assert(uop->in_load_queue);
	assert(!uop->in_store_queue);
	assert(!uop->in_instruction_queue);

	// Save iterator
	auto it = uop->load_queue_iterator;

	// Mark as not present in the queue
	uop->in_load_queue = false;
	uop->load_queue_iterator = load_queue.end();
	
	// Remove from queue as last step, as this may free uop
	load_queue.erase(it);

	// Decrease per-core counter
	core->decLoadStoreQueueOccupancy();
}


void Thread::ExtractFromStoreQueue(Uop *uop)
{
	// Uop must be in the queue
	assert(!uop->in_instruction_queue);
	assert(!uop->in_load_queue);
	assert(uop->in_store_queue);

	// Save iterator
	auto it = uop->store_queue_iterator;

	// Mark as not present in the queue
	uop->in_store_queue = false;
	uop->store_queue_iterator = store_queue.end();

	// Remove from queue as last step, as this may free uop
	store_queue.erase(it);

	// Decrease per-core counter
	core->decLoadStoreQueueOccupancy();
}


void Thread::DumpLoadStoreQueue(std::ostream &os) const
{
	// Load queue
	std::string title = "Load queue";
	os << title << '\n';
	os << std::string(title.size(), '-') << "\n\n";

	// Dump content
	int index = 0;
	for (auto &uop : load_queue)
	{
		os << misc::fmt("%3d. ", index);
		os << *uop << '\n';
		index++;
	}

	// Empty list
	if (load_queue.empty())
		os << "-Empty-\n";

	// End
	os << '\n';

	// Store queue
	title = "Store queue";
	os << title << '\n';
	os << std::string(title.size(), '-') << "\n\n";

	// Dump content
	index = 0;
	for (auto &uop : store_queue)
	{
		os << misc::fmt("%3d. ", index);
		os << *uop << '\n';
		index++;
	}

	// Empty list
	if (store_queue.empty())
		os << "-Empty-\n";

	// End
	os << '\n';
}


}

