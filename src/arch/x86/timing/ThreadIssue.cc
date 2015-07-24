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
		// Get the uop
		std::shared_ptr<Uop> uop = *it;

		// If the uop is not ready, skip it
		if (!register_file->isUopReady(uop.get()))
		{
			++it;
			continue;
		}

		// Check that memory system is accessible
		if (!data_module->canAccess(uop->physical_address))
		{
			++it;
			continue;
		}

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
		load_store_queue_reads++;
		core->incLoadStoreQueueReads();

		// Increment per-thread number of reads from registers
		integer_register_reads += uop->getNumIntegerInputs();
		floating_point_register_reads += uop->getNumFloatingPointInputs();
		xmm_register_reads += uop->getNumXmmInputs();

		// Increment per-core number of reads from registers
		core->incIntegerRegisterReads(uop->getNumIntegerInputs());
		core->incFloatingPointRegisterReads(uop->getNumFloatingPointInputs());
		core->incXmmRegisterReads(uop->getNumXmmInputs());

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
	// Return remaining unused quantum
	return quantum;
}

}

