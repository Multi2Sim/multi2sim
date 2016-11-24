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

#include <memory/Module.h>

#include "Core.h"
#include "Thread.h"
#include "Timing.h"


namespace x86
{

int Thread::IssueLoadQueue(int quantum)
{
	// List iterators
	auto it = load_queue.begin();
	auto e = load_queue.end();

	// Traverse list
	while (it != e && quantum > 0)
	{
		// Get the uop and forward iterator
		std::shared_ptr<Uop> uop = *it;
		++it;

		// If the uop is not ready, skip it
		if (!register_file->isUopReady(uop.get()))
			continue;

		// Check that memory system is accessible
		if (!data_module->canAccess(uop->physical_address))
			continue;

		// Remove uop from load queue
		ExtractFromLoadQueue(uop.get());

		// Access memory system
		cpu->MemoryAccess(data_module,
				mem::Module::AccessLoad,
				uop->physical_address,
				uop);

		// Mark uop as issued
		uop->issued = true;
		uop->issue_when = cpu->getCycle();
		
		// Increment the number of issued instructions of this kind
		incNumIssuedUinsts(uop->getOpcode());
		core->incNumIssuedUinsts(uop->getOpcode());
		cpu->incNumIssuedUinsts(uop->getOpcode());

		// Increment number of reads from load-store-queue
		num_load_store_queue_reads++;
		core->incNumLoadStoreQueueReads();

		// Increment per-thread number of reads from registers
		num_integer_register_reads += uop->getNumIntegerInputs();
		num_floating_point_register_reads += uop->getNumFloatingPointInputs();
		num_xmm_register_reads += uop->getNumXmmInputs();

		// Increment per-core number of reads from registers
		core->incNumIntegerRegisterReads(uop->getNumIntegerInputs());
		core->incNumFloatingPointRegisterReads(uop->getNumFloatingPointInputs());
		core->incNumXmmRegisterReads(uop->getNumXmmInputs());

		// Increment number of issued micro-instructions coming from
		// the trace cache
		if (uop->from_trace_cache)
			trace_cache->incNumIssuedUinsts();
	
		// One more instruction issued, update quantum
		quantum--;

		// Trace
		Timing::trace << misc::fmt("x86.inst "
				"id=%lld "
				"core=%d "
				"stg=\"i\"\n",
				uop->getIdInCore(),
				core->getId());
	}

	// Return remaining quantum
	return quantum;
}


int Thread::IssueStoreQueue(int quantum)
{
	// List iterators
	auto it = store_queue.begin();
	auto e = store_queue.end();

	// Traverse list
	while (it != e && quantum > 0)
	{
		// Get the uop and forward iterator
		std::shared_ptr<Uop> uop = *it;
		++it;

		// Sanity
		assert(uop->getOpcode() == Uinst::OpcodeStore);

		// Only committed stores can issue
		if (uop->in_reorder_buffer)
			break;

		// Check that memory system is ready
		if (!data_module->canAccess(uop->physical_address))
			break;

		// Remove store from store queue
		ExtractFromStoreQueue(uop.get());

		// Issue store to memory system
		cpu->MemoryAccess(data_module,
				mem::Module::AccessStore,
				uop->physical_address,
				uop);

		// Mark uop as issued
		uop->issued = true;
		uop->issue_when = cpu->getCycle();
		
		// Increment the number of issued instructions of this kind
		incNumIssuedUinsts(uop->getOpcode());
		core->incNumIssuedUinsts(uop->getOpcode());
		cpu->incNumIssuedUinsts(uop->getOpcode());

		// Increment number of reads from load-store-queue
		num_load_store_queue_reads++;
		core->incNumLoadStoreQueueReads();

		// Increment per-thread number of reads from registers
		num_integer_register_reads += uop->getNumIntegerInputs();
		num_floating_point_register_reads += uop->getNumFloatingPointInputs();
		num_xmm_register_reads += uop->getNumXmmInputs();

		// Increment per-core number of reads from registers
		core->incNumIntegerRegisterReads(uop->getNumIntegerInputs());
		core->incNumFloatingPointRegisterReads(uop->getNumFloatingPointInputs());
		core->incNumXmmRegisterReads(uop->getNumXmmInputs());

		// Increment number of issued micro-instructions coming from
		// the trace cache
		if (uop->from_trace_cache)
			trace_cache->incNumIssuedUinsts();
	
		// One more instruction issued, update quantum
		quantum--;
	}

	// Return remaining quantum
	return quantum;
}


int Thread::IssueLoadStoreQueue(int quantum)
{
	// Give priority to loads versus stores
	quantum = IssueLoadQueue(quantum);
	quantum = IssueStoreQueue(quantum);

	// Return remaining unused quantum
	return quantum;
}


int Thread::IssueInstructionQueue(int quantum)
{
	// List iterators
	auto it = instruction_queue.begin();
	auto e = instruction_queue.end();

	// Traverse list
	while (it != e && quantum > 0)
	{
		// Get the uop and forward iterator
		std::shared_ptr<Uop> uop = *it;
		++it;

		// Sanity
		assert(!(uop->getFlags() & Uinst::FlagMem));

		// If the uop is not ready, skip it
		if (!register_file->isUopReady(uop.get()))
			continue;

		// Run the instruction in its corresponding functional unit in
		// the ALU. If the instruction does not require a functional
		// unit, a latency of 1 is returned by ALU::Reserve(). If there
		// is no functional unit available, it returns 0.
		Alu *alu = core->getAlu();
		int latency = alu->Reserve(uop.get());
		if (!latency)
			continue;

		// Instruction was successfully issued, remove from instruction
		// queue.
		ExtractFromInstructionQueue(uop.get());

		// Instruction has been issued
		uop->issued = true;
		uop->issue_when = cpu->getCycle();

		// Schedule instruction in event queue
		assert(latency > 0);
		core->InsertInEventQueue(uop, latency);
		
		// Increment the number of issued instructions of this kind
		incNumIssuedUinsts(uop->getOpcode());
		core->incNumIssuedUinsts(uop->getOpcode());
		cpu->incNumIssuedUinsts(uop->getOpcode());

		// Increment number of reads from instruction queue
		num_instruction_queue_reads++;
		core->incNumInstructionQueueReads();

		// Increment per-thread number of reads from registers
		num_integer_register_reads += uop->getNumIntegerInputs();
		num_floating_point_register_reads += uop->getNumFloatingPointInputs();
		num_xmm_register_reads += uop->getNumXmmInputs();

		// Increment per-core number of reads from registers
		core->incNumIntegerRegisterReads(uop->getNumIntegerInputs());
		core->incNumFloatingPointRegisterReads(uop->getNumFloatingPointInputs());
		core->incNumXmmRegisterReads(uop->getNumXmmInputs());

		// Increment number of issued micro-instructions coming from
		// the trace cache
		if (uop->from_trace_cache)
			trace_cache->incNumIssuedUinsts();
	
		// One more instruction issued, update quantum
		quantum--;
		
		// Trace
		Timing::trace << misc::fmt("x86.inst "
				"id=%lld "
				"core=%d "
				"stg=\"i\"\n",
				uop->getIdInCore(),
				core->getId());
	}
	
	// Return remaining unused quantum
	return quantum;
}

}

