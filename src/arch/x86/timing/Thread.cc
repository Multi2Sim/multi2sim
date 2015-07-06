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


namespace x86
{

Thread::Thread(Core *core,
		int id_in_core) :
		core(core),
		id_in_core(id_in_core)
{
	// Assign name
	name = misc::fmt("Core %d Thread %d", core->getId(), id_in_core);

	// Global index in CPU
	id_in_cpu = core->getId() * Cpu::getNumThreads() + id_in_core;

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


bool Thread::canFetch()
{
	// There must be a context mapped to this thread
	if (!context)
		return false;
	
	// The context must be running
	if (!context->getState(Context::StateRunning))
		return false;

	// Fetch queue must have not exceeded the limit of stored bytes to be
	// able to store new macro-instructions.
	if (fetch_queue_occupancy >= Cpu::getFetchQueueSize())
		return false;

	// If the next fetch address belongs to a new block, cache system
	// must be accessible to read it.
	unsigned block_address = fetch_neip & ~(instruction_module->getBlockSize() - 1);
	if (block_address != fetch_block_address)
	{
		unsigned physical_address = context->mmu->TranslateVirtualAddress(
				context->mmu_space,
				fetch_neip);
		if (!instruction_module->canAccess(physical_address))
			return false;
	}
	
	// We can fetch
	return true;
}


std::shared_ptr<Uop> Thread::FetchInstruction(bool fetch_from_trace_cache)
{
	throw misc::Panic("Not implemented");
}


bool Thread::FetchFromTraceCache()
{
	return true;
}


void Thread::Fetch()
{

}

}
