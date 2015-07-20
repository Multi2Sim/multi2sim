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

#include <arch/southern-islands/disassembler/Instruction.h>
#include <arch/southern-islands/emulator/Wavefront.h>

#include "ComputeUnit.h"
#include "Timing.h"
#include "WavefrontPool.h"


namespace SI
{

int ComputeUnit::num_wavefront_pools = 4;
int ComputeUnit::fetch_latency = 1;
int ComputeUnit::fetch_width = 1;
int ComputeUnit::fetch_buffer_size = 10;
int ComputeUnit::issue_latency = 1;
int ComputeUnit::issue_width = 5;
int ComputeUnit::max_instructions_issued_per_type = 1;
	

ComputeUnit::ComputeUnit(int index) :
		index(index),
		scalar_unit(this),
		branch_unit(this),
		lds_unit(this),
		vector_memory_unit(this)
{
	// Create wavefront pools, and SIMD units
	wavefront_pools.reserve(num_wavefront_pools);
	fetch_buffers.reserve(num_wavefront_pools);
	simd_units.reserve(num_wavefront_pools);
	for (int i = 0; i < num_wavefront_pools; i++)
	{
		wavefront_pools[i] = misc::new_unique<WavefrontPool>(i, this);
		fetch_buffers[i] = misc::new_unique<FetchBuffer>(i, this);
		simd_units[i] = misc::new_unique<SimdUnit>(this);
	}
}


void ComputeUnit::IssueToExecutionUnit(FetchBuffer *fetch_buffer,
		ExecutionUnit *execution_unit)
{
	// Issue at most 'max_instructions_per_type'
	for (int num_issued_instructions = 0;
			num_issued_instructions < max_instructions_issued_per_type;
			num_issued_instructions++)
	{
		// Nothing if execution unit cannot absorb more instructions
		if (!execution_unit->canIssue())
			break;

		// Find oldest uop
		auto oldest_uop_iterator = fetch_buffer->end();
		for (auto it = fetch_buffer->begin(),
				e = fetch_buffer->end();
				it != e;
				++it)
		{
			// Discard uop if it is not suitable for this execution
			// unit
			Uop *uop = it->get();
			if (!execution_unit->isValidUop(uop))
				continue;

			// Skip uops that have not completed fetch
			if (timing->getCycle() < uop->fetch_ready)
				continue;

			// Save oldest uop
			if (oldest_uop_iterator == fetch_buffer->end() ||
					uop->getWavefront()->getId() <
					(*oldest_uop_iterator)->getWavefront()->getId())
				oldest_uop_iterator = it;
		}

		// Stop if no instruction found
		if (oldest_uop_iterator == fetch_buffer->end())
			break;

		// Erase from fetch buffer, issue to execution unit
		std::shared_ptr<Uop> uop = fetch_buffer->Remove(oldest_uop_iterator);
		execution_unit->Issue(uop);

		// Trace
		Timing::trace << misc::fmt("si.instruction "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"i\"\n", 
				uop->getIdInComputeUnit(),
				index,
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
	}
}


void ComputeUnit::Issue(FetchBuffer *fetch_buffer)
{
	// Issue instructions to branch unit
	IssueToExecutionUnit(fetch_buffer, &branch_unit);

	// Issue instructions to scalar unit
	IssueToExecutionUnit(fetch_buffer, &scalar_unit);

	// Issue instructions to SIMD units
	for (auto &simd_unit : simd_units)
		IssueToExecutionUnit(fetch_buffer, simd_unit.get());

	// Issue instructions to vector memory unit
	IssueToExecutionUnit(fetch_buffer, &vector_memory_unit);

	// Issue instructions to LDS unit
	IssueToExecutionUnit(fetch_buffer, &lds_unit);

	// Update visualization states for all instructions not issued
	for (auto it = fetch_buffer->begin(),
			e = fetch_buffer->end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// Skip uops that have not completed fetch
		if (timing->getCycle() < uop->fetch_ready)
			continue;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"i\"\n",
				uop->getIdInComputeUnit(),
				index,
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
	}
}


void ComputeUnit::Fetch(FetchBuffer *fetch_buffer,
		WavefrontPool *wavefront_pool)
{
	// Checks
	assert(fetch_buffer);
	assert(wavefront_pool);
	assert(fetch_buffer->getId() == wavefront_pool->getId());

	// Set up variables
	int instructions_processed = 0;

	// Fetch the instructions
	for (auto it = wavefront_pool->begin(),
			e = wavefront_pool->end();
			it != e;
			++it)
	{
		// Get wavefront pool entry
		WavefrontPoolEntry *wavefront_pool_entry = it->get();

		// Get wavefront
		Wavefront *wavefront = wavefront_pool_entry->getWavefront();

		// Checks
		assert(wavefront);

		// This should always be checked, regardless of how many
		// instructions have been fetched
		if (wavefront_pool_entry->ready_next_cycle)
		{
			wavefront_pool_entry->ready = true;
			wavefront_pool_entry->ready_next_cycle = false;
			continue;
		}

		// Only fetch a fixed number of instructions per cycle
		if (instructions_processed == fetch_width)
			continue;

		// Wavefront is not ready (previous instructions is still
		// in flight
		if (!wavefront_pool_entry->ready)
			continue;

		// If the wavefront finishes, there still may be outstanding
		// memory operations, so if the entry is marked finished
		// the wavefront must also be finished, but not vice-versa
		if (wavefront_pool_entry->wavefront_finished)
		{
			assert(wavefront->getFinished());
			continue;
		}

		// Wavefront is finished but other wavefronts from the
		// workgroup remain.  There may still be outstanding
		// memory operations, but no more instructions should
		// be fetched.
		if (wavefront->getFinished())
			continue;

		// Wavefront is ready but waiting on outstanding
		// memory instructions
		if (wavefront->isMemoryWait())
		{
			// No outstanding accesses
			if (!wavefront_pool_entry->lgkm_cnt &&
				!wavefront_pool_entry->exp_cnt &&
				!wavefront_pool_entry->vm_cnt)
					wavefront->setMemoryWait(false);
			else
				continue;
		}

		// Wavefront is ready but waiting at barrier
		if (wavefront_pool_entry->wait_for_barrier)
			continue;

		// Stall if fetch buffer is full
		assert(fetch_buffer->getSize() <= fetch_buffer_size);
		if (fetch_buffer->getSize() == fetch_buffer_size)
			continue;

		// Emulate instructions
		wavefront->Execute();
		wavefront_pool_entry->ready = true;

		// Create uop
		auto uop = misc::new_shared<Uop>(
				wavefront,
				wavefront_pool_entry,
				timing->getCycle(),
				wavefront->getWorkGroup(),
				fetch_buffer->getId());
		uop->vector_memory_read = wavefront->vector_memory_read;
		uop->vector_memory_write = wavefront->vector_memory_write;
		uop->vector_memory_atomic = wavefront->vector_memory_atomic;
		uop->scalar_memory_read = wavefront->scalar_memory_read;
		uop->lds_read = wavefront->lds_read;
		uop->lds_write = wavefront->lds_write;
		uop->wavefront_last_instruction = wavefront->finished;
		uop->memory_wait = wavefront->memory_wait;
		uop->at_barrier = wavefront->at_barrier;
		uop->setInstruction(wavefront->getInstruction());
		uop->vector_memory_global_coherency =
				wavefront->vector_memory_global_coherency;

		// Checks
		assert(wavefront->getWorkGroup() && uop->getWorkGroup());

		// Convert instruction name to string
		std::string instruction_name = wavefront->getInstruction()->getName();
		misc::StringSingleSpaces(instruction_name);

		// Trace
		Timing::trace << misc::fmt("si.new_inst "
				"id=%lld "
				"cu=%d "
				"ib=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"i\" "
				"asm=\"%s\"\n",
				uop->getIdInComputeUnit(),
				index,
				uop->getWavefrontPoolId(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront(),
				instruction_name.c_str());


		// Update last memory accesses
		for (auto it = wavefront->getWorkItemsBegin(),
				e = wavefront->getWorkItemsEnd();
				it != e;
				++it)
		{
			// Get work item
			WorkItem *work_item = it->get();

			// Get uop work item info
			Uop::WorkItemInfo *work_item_info;
			work_item_info =
				&uop->work_item_info_list[work_item->getIdInWavefront()];

			// Global memory
			work_item_info->global_memory_access_address =
					work_item->global_memory_access_address;
			work_item_info->global_memory_access_size =
					work_item->global_memory_access_size;

			// LDS
			work_item_info->lds_access_count =
				work_item->lds_access_count;
			for (int j = 0; j < work_item->lds_access_count; j++)
			{
				work_item_info->lds_access[j].type =
					work_item->lds_access[j].type;
				work_item_info->lds_access[j].addr =
					work_item->lds_access[j].addr;
				work_item_info->lds_access[j].size =
					work_item->lds_access[j].size;
			}
		}

		// Access instruction cache. Record the time when the
		// instruction will have been fetched, as per the latency
		// of the instruction memory.
		uop->fetch_ready = timing->getCycle() + fetch_latency;

		// Insert uop into fetch buffer
		fetch_buffer->addUop(uop);

		instructions_processed++;
		num_total_instructions++;
	}
}


void ComputeUnit::Run()
{
	// Return if no work groups are mapped to this compute unit
	if (!work_groups.size())
		return;
	
	// Save timing simulator
	timing = Timing::getInstance();

	// Issue buffer chosen to issue this cycle
	int active_issue_buffer = timing->getCycle() % num_wavefront_pools;
	assert(active_issue_buffer >= 0 && active_issue_buffer < num_wavefront_pools);

	// SIMDs
	for (auto &simd_unit : simd_units)
		simd_unit->Run();

	// Vector memory
	vector_memory_unit.Run();

	// LDS unit
	lds_unit.Run();

	// Scalar unit
	scalar_unit.Run();

	// Branch unit
	branch_unit.Run();

	// Issue from the active issue buffer
	Issue(fetch_buffers[active_issue_buffer].get());

	// Update visualization in non-active issue buffers
	/*
	FIXME
	for (i = 0; i < num_simd_units; i++)
	{
		if (i != active_issue_buffer)
		{
			SIComputeUnitUpdateFetchVisualization(self, i);
		}
	}
	*/

	// Fetch
	//for (int i = 0; i < num_wavefront_pools; i++)
	//	Fetch(i);
}

}

