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

#include "Cpu.h"
#include "Thread.h"
#include "Timing.h"


namespace x86
{

const char *Thread::commit_stall_error =
	"The CPU commit stage has not received any instruction for 1M "
	"cycles. Most likely, this means that a deadlock condition "
	"occurred in the management of some modeled structure (network, "
	"cache system, core queues, etc.).\n";


bool Thread::canCommit()
{
	// Get current cycle
	long long cycle = cpu->getCycle();

	// Sanity check - If the context is running, we assume that something is
	// going wrong if more than 1M cycles go by without committing a uop.
	if (!context || !context->getState(Context::StateRunning))
		last_commit_cycle = cycle;
	if (cycle - last_commit_cycle > 1000000)
	{
		// Show warning
		misc::Warning("[x86] %s: simulation ended due to a commit "
				"stall.\n\t%s",
				name.c_str(),
				commit_stall_error);

		// Print state of the core
		std::cerr << *core;

		// Finish simulation
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->Finish("Stall");
	}

	// If there is no instruction in the reorder buffer, cannot commit
	if (reorder_buffer.empty())
		return false;

	// Get instruction from reorder buffer head
	assert(reorder_buffer.size());
	std::shared_ptr<Uop> uop = reorder_buffer.front();
	assert(uop->getThread() == this);

	// Stores must be ready in order to commit
	if (uop->getOpcode() == Uinst::OpcodeStore)
		return register_file->isUopReady(uop.get());
	
	// Instructions other than stores must be completed
	return uop->completed;
}


void Thread::Commit(int quantum)
{
	// Sanity: context must be mapped
	assert(context);

	// Commit stage for thread
	while (quantum && canCommit())
	{
		// Get instruction at the head of the reorder buffer
		assert(reorder_buffer.size());
		std::shared_ptr<Uop> uop = reorder_buffer.front();
		assert(uop->getThread() == this);

		// Recover from mispeculation if this is the first uop of a
		// sequence of mispredicted instructions.
		//
		// NOTE - Even when the recovery kind (RecoverKind) has been
		// set to the writeback stage, there is a chance that a
		// leading mispeculated instruction has made it here. This is
		// the case of a mispeculated store, which did not have the
		// chance to get to the writeback stage before it commits.
		//
		if (uop->first_speculative_mode)
		{
			// Clear thread structures
			Recover();

			// Nothing left to do
			return;
		}
	
		// Free physical registers
		assert(!uop->speculative_mode);
		register_file->CommitUop(uop.get());
		
		// Branches update branch predictor and BTB
		if (uop->getFlags() & Uinst::FlagCtrl)
		{
			branch_predictor->Update(uop.get());
			branch_predictor->UpdateBtb(uop.get());
			num_btb_writes++;
		}

		// Trace cache
		if (TraceCache::isPresent())
			trace_cache->RecordUop(uop.get());

		// Save last commit cycle
		last_commit_cycle = cpu->getCycle();

		// Record committed uops of each kind
		incNumCommittedUinsts(uop->getOpcode());
		core->incNumCommittedUinsts(uop->getOpcode());
		cpu->incNumCommittedUinsts(uop->getOpcode());
		if (!uop->mop_index)
			cpu->incNumCommittedInstructions();

		// Trace cache statistics
		if (uop->from_trace_cache)
			trace_cache->incNumCommittedUinsts();
		
		// Statistics for branch instructions
		if (uop->getFlags() & Uinst::FlagCtrl)
		{
			// Number of branches
			num_branches++;
			core->incNumBranches();
			cpu->incNumBranches();

			// Mispredicted branches
			if (uop->neip != uop->predicted_neip)
			{
				num_mispredicted_branches++;
				core->incNumMispredictedBranches();
				cpu->incNumMispredictedBranches();
			}
		}

		// Trace
		if (Timing::trace)
		{
			// Output
			Timing::trace << misc::fmt("x86.inst "
					"id=%lld "
					"core=%d "
					"stg=\"co\"\n",
					uop->getIdInCore(),
					core->getId());

			// Keep uop for later
			cpu->InsertInTraceList(uop);
		}

		// Remove uop from reorder buffer
		ExtractFromReorderBuffer(uop.get());

		// Consume quantum
		quantum--;

		// Statistics
		num_reorder_buffer_reads++;
		core->incNumReorderBufferReads();
	}

	// If context eviction signal is activated and pipeline is empty,
	// deallocate context.
	if (context->evict_signal && isPipelineEmpty())
		EvictContext();
}

}

