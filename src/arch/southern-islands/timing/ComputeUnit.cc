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
#include <arch/southern-islands/emulator/Emulator.h>
#include <arch/southern-islands/emulator/NDRange.h>
#include <arch/southern-islands/emulator/Wavefront.h>
#include <arch/southern-islands/emulator/WorkGroup.h>
#include <memory/Module.h>

#include "ComputeUnit.h"
#include "Timing.h"
#include "WavefrontPool.h"


namespace SI
{

int ComputeUnit::num_wavefront_pools = 4;
int ComputeUnit::max_work_groups_per_wavefront_pool = 10;
int ComputeUnit::max_wavefronts_per_wavefront_pool = 10; 
int ComputeUnit::fetch_latency = 1;
int ComputeUnit::fetch_width = 1;
int ComputeUnit::fetch_buffer_size = 10;
int ComputeUnit::issue_latency = 1;
int ComputeUnit::issue_width = 5;
int ComputeUnit::max_instructions_issued_per_type = 1;
int ComputeUnit::lds_size = 65536;
int ComputeUnit::lds_alloc_size = 64;
int ComputeUnit::lds_latency = 2;                                                      
int ComputeUnit::lds_block_size = 64;                                                  
int ComputeUnit::lds_num_ports = 2; 
	

ComputeUnit::ComputeUnit(int index, Gpu *gpu) :
		gpu(gpu),
		index(index),
		scalar_unit(this),
		branch_unit(this),
		lds_unit(this),
		vector_memory_unit(this)
{
	// Create the Lds module
	lds_module = misc::new_unique<mem::Module>(
			misc::fmt("LDS[%d]", index), 
			mem::Module::TypeLocalMemory,
			lds_num_ports, 
			lds_block_size,
			lds_latency);

	// Create wavefront pools, and SIMD units
	wavefront_pools.resize(num_wavefront_pools);
	fetch_buffers.resize(num_wavefront_pools);
	simd_units.resize(num_wavefront_pools);
	for (int i = 0; i < num_wavefront_pools; i++)
	{
		wavefront_pools[i] = misc::new_unique<WavefrontPool>(i, this);
		fetch_buffers[i] = misc::new_unique<FetchBuffer>(i, this);
		simd_units[i] = misc::new_unique<SimdUnit>(this);
	}
}


void ComputeUnit::IssueToExecutionUnit(std::unique_ptr<Uop> uop,
		ExecutionUnit *execution_unit)
{
	// Check
	assert(uop.get());

	// Get ids
	long long compute_unit_id = uop->getIdInComputeUnit();
	int wavefront_id = uop->getWavefront()->getId();
	long long id_in_wavefront = uop->getIdInWavefront();

	// Erase from fetch buffer, issue to execution unit
	execution_unit->Issue(std::move(uop));

	// Increase the uop counter for current cycle
	auto it = num_uop_issued_this_cycle.find(execution_unit);
	it->second++;

	// Trace
	Timing::trace << misc::fmt("si.inst "
			"id=%lld "
			"cu=%d "
			"wf=%d "
			"uop_id=%lld "
			"stg=\"i\"\n",
			compute_unit_id,
			index,
			wavefront_id,
			id_in_wavefront);
}


bool ComputeUnit::CanExecutionUnitIssue(Uop *uop, ExecutionUnit *execution_unit)
{
	assert(uop);

	// Create an entry in the uop counter on demand
	auto it = num_uop_issued_this_cycle.find(execution_unit);
	if (it == num_uop_issued_this_cycle.end()) {
		auto ret = num_uop_issued_this_cycle.emplace(execution_unit, 0);
		it = ret.first;
	}

	// Checks if the maximum value reached
	if (it->second >= max_instructions_issued_per_type)
		return false;

	// Check if the exeuction unit can issue
	if (!execution_unit->canIssue())
		return false;

	// Check if the uop type matches the execution unit
	if (!execution_unit->isValidUop(uop))
		return false;

	return true;
}


void ComputeUnit::Issue(FetchBuffer *fetch_buffer)
{
	// Clear uop counter
	num_uop_issued_this_cycle.clear();

	// Iterate uops to launch
	auto it = fetch_buffer->begin();
	while(it != fetch_buffer->end())
	{
		// Get Uop
		Uop *uop = it->get();
		assert(uop);

		// Skip uops that have not completed fetch
		if (timing->getCycle() < uop->fetch_ready)
			break;

		// Issue to branch unit
		if (CanExecutionUnitIssue(uop, &branch_unit))
		{
			IssueToExecutionUnit(std::move(*it), &branch_unit);
			it = fetch_buffer->Remove(it);
			continue;
		}

		// Issue to scalar unit
		if (CanExecutionUnitIssue(uop, &scalar_unit))
		{
			IssueToExecutionUnit(std::move(*it), &scalar_unit);
			it = fetch_buffer->Remove(it);
			continue;
		}

		// Issue instructions to SIMD units
		bool issued_to_simd_unit = false;
		for (auto &simd_unit : simd_units)
		{
			if (CanExecutionUnitIssue(uop, simd_unit.get()))
			{
				IssueToExecutionUnit(std::move(*it),
						simd_unit.get());
				it = fetch_buffer->Remove(it);
				issued_to_simd_unit = true;
				break;
			}
		}
		if (issued_to_simd_unit)
			continue;

		// Issue to vector memory unit
		if (CanExecutionUnitIssue(uop, &vector_memory_unit))
		{
			IssueToExecutionUnit(std::move(*it), &vector_memory_unit);
			it = fetch_buffer->Remove(it);
			continue;
		}

		// Issue to vector lds unit
		if (CanExecutionUnitIssue(uop, &lds_unit))
		{
			IssueToExecutionUnit(std::move(*it), &lds_unit);
			it = fetch_buffer->Remove(it);
			continue;
		}

		// Not issued anywhere, stop issuing
		break;
	}

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
		if (!wavefront)
			continue;

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
		// workgroup remain. There may still be outstanding
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
		wavefront_pool_entry->ready = false;

		// Create uop
		auto uop = misc::new_unique<Uop>(
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
		fetch_buffer->addUop(std::move(uop));

		instructions_processed++;
		num_total_instructions++;
	}
}


void ComputeUnit::MapWorkGroup(WorkGroup *work_group)
{
	// Checks
	assert(work_group);
	assert((int) work_groups.size() <= gpu->getWorkGroupsPerComputeUnit());
	assert(!work_group->id_in_compute_unit);

	// Find an available slot
	while (work_group->id_in_compute_unit < gpu->getWorkGroupsPerComputeUnit()
			&& (work_group->id_in_compute_unit < 
			(int) work_groups.size()) && 
			(work_groups[work_group->id_in_compute_unit] != nullptr))
		work_group->id_in_compute_unit++;

	// Checks
	assert(work_group->id_in_compute_unit <
			gpu->getWorkGroupsPerComputeUnit());

	// Save timing simulator
	timing = Timing::getInstance();

	// Debug
	Emulator::scheduler_debug << misc::fmt("@%lld available slot %d "
			"found in compute unit %d\n",
			timing->getCycle(),
			work_group->id_in_compute_unit,
			index);

	// Insert work group into the list
	AddWorkGroup(work_group);

	// Checks
	assert((int) work_groups.size() <= gpu->getWorkGroupsPerComputeUnit());
	
	// If compute unit is not full, add it back to the available list
	if ((int) work_groups.size() < gpu->getWorkGroupsPerComputeUnit())
	{
		if (!in_available_compute_units)
			gpu->InsertInAvailableComputeUnits(this);
	}

	// Assign wavefront identifiers in compute unit
	int wavefront_id = 0;
	for (auto it = work_group->getWavefrontsBegin();
			it != work_group->getWavefrontsEnd();
			++it)
	{
		// Get wavefront pointer
		Wavefront *wavefront = it->get();

		// Set wavefront Id
		wavefront->id_in_compute_unit = work_group->id_in_compute_unit *
				work_group->getWavefrontsInWorkgroup() +
				wavefront_id;

		// Update internal counter
		wavefront_id++;
	}

	// Set wavefront pool for work group
	int wavefront_pool_id = work_group->id_in_compute_unit %
			num_wavefront_pools;
	work_group->wavefront_pool = wavefront_pools[wavefront_pool_id].get();

	// Check if the wavefronts in the work group can fit into the wavefront
	// pool
	assert((int) work_group->getWavefrontsInWorkgroup() <=
			max_wavefronts_per_wavefront_pool);

	// Insert wavefronts into an instruction buffer
	work_group->wavefront_pool->MapWavefronts(work_group);

	// Increment count of mapped work groups
	num_mapped_work_groups++;

	// Debug info
	Emulator::scheduler_debug << misc::fmt("\t\tfirst wavefront=%d, "
			"count=%d\n"
			"\t\tfirst work-item=%d, count=%d\n",
			work_group->getWavefront(0)->getId(),
			work_group->getNumWavefronts(),
			work_group->getWorkItem(0)->getId(),
			work_group->getNumWorkItems());

	// Trace info
	Timing::trace << misc::fmt("si.map_wg "
				   "cu=%d "
				   "wg=%d "
				   "wi_first=%d "
				   "wi_count=%d "
				   "wf_first=%d "
				   "wf_count=%d\n",
				   index, work_group->getId(),
				   work_group->getWorkItem(0)->getId(),
				   work_group->getNumWorkItems(),
				   work_group->getWavefront(0)->getId(),
				   work_group->getNumWavefronts());
}

void ComputeUnit::AddWorkGroup(WorkGroup *work_group)
{
	// Add a work group only if the id in compute unit is the id for a new 
	// work group in the compute unit's list
	int index = work_group->id_in_compute_unit;
	if (index == (int) work_groups.size() &&
			(int) work_groups.size() < 
			gpu->getWorkGroupsPerComputeUnit())
	{
		work_groups.push_back(work_group);
	}
	else
	{
		// Make sure an entry is emptied up
		assert(work_groups[index] == nullptr);

		// Set the new work group to the empty entry
		work_groups[index] = work_group;
	}

	// Save iterator 
	auto it = work_groups.begin();
	std::advance(it, work_group->getIdInComputeUnit()); 
	work_group->compute_unit_work_groups_iterator = it;

	// Debug info
	Emulator::scheduler_debug << misc::fmt("\twork group %d "
			"added\n",
			work_group->getId());
}


void ComputeUnit::RemoveWorkGroup(WorkGroup *work_group)
{
	// Debug info
	Emulator::scheduler_debug << misc::fmt("@%lld work group %d "
			"removed from compute unit %d slot %d\n",
			timing->getCycle(),
			work_group->getId(),
			index,
			work_group->id_in_compute_unit);

	// Erase work group                                                      
	assert(work_group->compute_unit_work_groups_iterator != 
			work_groups.end());
	work_groups[work_group->id_in_compute_unit] = nullptr;
}


void ComputeUnit::UnmapWorkGroup(WorkGroup *work_group)
{
	// Get Gpu object
	Gpu *gpu = getGpu();

	// Add work group register access statistics to compute unit
	num_sreg_reads += work_group->getSregReadCount();
	num_sreg_writes += work_group->getSregWriteCount();
	num_vreg_reads += work_group->getVregReadCount();
	num_vreg_writes += work_group->getVregWriteCount();

	// Remove the work group from the list
	assert(work_groups.size() > 0);
	RemoveWorkGroup(work_group);

	// Unmap wavefronts from instruction buffer
	work_group->wavefront_pool->UnmapWavefronts(work_group);
	
	// Remove the work group from the running work groups list
	NDRange *ndrange = work_group->getNDRange();
	ndrange->RemoveWorkGroup(work_group);

	// If compute unit is not already in the available list, place
	// it there. The vector list of work groups does not shrink,
	// when we unmap a workgroup.
	assert((int) work_groups.size() <= gpu->getWorkGroupsPerComputeUnit());
	if (!in_available_compute_units)
		gpu->InsertInAvailableComputeUnits(this);

	// Trace
	Timing::trace << misc::fmt("si.unmap_wg cu=%d wg=%d\n", index,
			work_group->getId());
}


void ComputeUnit::UpdateFetchVisualization(FetchBuffer *fetch_buffer)
{
	for (auto it = fetch_buffer->begin(),
			e = fetch_buffer->end();
			it != e;
			++it)
	{
		// Get uop
		Uop *uop = it->get();
		assert(uop);

		// Skip all uops that have not yet completed the fetch
		if (timing->getCycle() < uop->fetch_ready)
			break;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"s\"\n",
				uop->getIdInComputeUnit(),
				index,
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
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
	for (int i = 0; i < (int) simd_units.size(); i++)
	{
		if (i != active_issue_buffer)
		{
			UpdateFetchVisualization(fetch_buffers[i].get());
		}
	}

	// Fetch
	for (int i = 0; i < num_wavefront_pools; i++)
		Fetch(fetch_buffers[i].get(), wavefront_pools[i].get());
}


} // Namespace SI

