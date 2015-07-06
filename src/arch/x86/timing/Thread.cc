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
	if (TraceCache::isPresent())
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


Uop *Thread::FetchInstruction(bool fetch_from_trace_cache)
{
	throw misc::Panic("Not implemented");
}


bool Thread::FetchFromTraceCache()
{
	return true;
}


void Thread::Fetch()
{
	// Sanity
	assert(context);

	// Try to fetch from trace cache first
	if (TraceCache::isPresent() && FetchFromTraceCache())
		return;
	
	// If new block to fetch is not the same as the previously fetched (and
	// stored) block, access the instruction cache.
	unsigned block_address = fetch_neip & ~(instruction_module->getBlockSize() - 1);
	if (block_address != fetch_block_address)
	{
		// Translate address
		unsigned physical_address = context->mmu->TranslateVirtualAddress(
				context->mmu_space,
				fetch_neip);

		// Save last fetched block
		fetch_block_address = block_address;
		fetch_address = physical_address;
		
		// Access instruction cache
		assert(instruction_module->canAccess(physical_address));
		fetch_address = instruction_module->Access(
				mem::Module::AccessLoad,
				physical_address);
		
		// Stats
		btb_reads++;
	}

	// Fetch all instructions within the block up to the first predict-taken
	// branch
	while ((fetch_neip & ~(instruction_module->getBlockSize() - 1))
			== block_address)
	{
		// If instruction caused context to suspend or finish
		if (!context->getState(Context::StateRunning))
			break;
	
		// If fetch queue is full, stop fetching
		if (fetch_queue_occupancy >= Cpu::getFetchQueueSize())
			break;
		
		// Insert macro-instruction into the fetch queue. Since the
		// macro-instruction information is only available at this
		// point, we use it to decode instruction now and insert uops
		// into the fetch queue. However, the fetch queue occupancy is
		// increased with the macro-instruction size.
		Uop *uop = FetchInstruction(false);

		// Invalid x86 instruction, no forward progress in loop
		if (!context->getInstruction()->getSize())
			break;
		
		// No uop was produced by this macro-instruction
		if (!uop)
			continue;

		// Instructions detected as branches by the BTB are checked for
		// branch direction in the branch predictor. If they are
		// predicted taken, stop fetching from this block and set new
		// fetch address.
		if (uop->getFlags() & UInstFlagCtrl)
		{
			// Look up BTB
			unsigned target = branch_predictor->LookupBTB(*uop);

			// Look up branch predictor
			BranchPredictor::Prediction prediction = branch_predictor
					->LookupBranchPrediction(*uop);
			bool taken = prediction == BranchPredictor::PredictionTaken
					&& target;

			// Set next instruction pointer
			if (taken)
			{
				fetch_neip = target;
				uop->predicted_neip = target;
				break;
			}
		}
	}
}

}
