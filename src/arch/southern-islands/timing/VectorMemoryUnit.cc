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

#include <arch/southern-islands/emulator/Wavefront.h>
#include <arch/southern-islands/emulator/WorkGroup.h>
#include <arch/southern-islands/emulator/NDRange.h>
#include <memory/Module.h>
#include <memory/Mmu.h>

#include "VectorMemoryUnit.h"
#include "ComputeUnit.h"
#include "Timing.h"


namespace SI
{

int VectorMemoryUnit::width = 1;
int VectorMemoryUnit::issue_buffer_size = 1;
int VectorMemoryUnit::decode_latency = 1;
int VectorMemoryUnit::decode_buffer_size = 1;
int VectorMemoryUnit::read_latency = 1;
int VectorMemoryUnit::read_buffer_size = 1;
int VectorMemoryUnit::max_inflight_mem_accesses = 32;
int VectorMemoryUnit::write_latency = 1;
int VectorMemoryUnit::write_buffer_size = 1;


void VectorMemoryUnit::Run()
{
	Complete();
	Write();
	Memory();
	Read();
	Decode();
}

bool VectorMemoryUnit::isValidUop(Uop *uop) const
{
	// Get instruction
	Instruction *instruction = uop->getInstruction();

	// Determine if vector memory instruction
	if (instruction->getFormat() != Instruction::FormatMTBUF &&
			instruction->getFormat() != Instruction::FormatMUBUF)
		return false;

	return true;
}

void VectorMemoryUnit::Issue(std::unique_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();

	// The wavefront will be ready next cycle
	uop->getWavefrontPoolEntry()->ready_next_cycle = true;

	// One more instruction of this kind
	compute_unit->num_vector_memory_instructions++;
	uop->getWavefrontPoolEntry()->lgkm_cnt++;

	// Issue it
	ExecutionUnit::Issue(std::move(uop));
}

void VectorMemoryUnit::Complete()
{
	// Get compute unit and GPU objects
	ComputeUnit *compute_unit = getComputeUnit();
	Gpu *gpu = compute_unit->getGpu();

	// Sanity check the write buffer
	assert((int) write_buffer.size() <= width);

	// Process completed instructions
	auto it = write_buffer.begin();
	while (it != write_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->write_ready)
			break;
	
		// Access complete, remove the uop from the queue
		assert(uop->getWavefrontPoolEntry()->lgkm_cnt > 0);
		uop->getWavefrontPoolEntry()->lgkm_cnt--;
		
		// Record trace
		Timing::trace << misc::fmt("si.end_inst "
				"id=%lld "
				"cu=%d\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex());

		// Access complete, remove the uop from the queue and get the 
		// iterator for the next element
		it = write_buffer.erase(it);
		assert(uop->getWorkGroup()
				->inflight_instructions > 0);
		uop->getWorkGroup()->
				inflight_instructions--;

		// Statistics
		num_instructions++;
		gpu->last_complete_cycle = compute_unit->getTiming()->getCycle();
	}
}

void VectorMemoryUnit::Write()
{
	// Get compute unit object
	ComputeUnit *compute_unit = getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check the mem buffer
	assert((int) mem_buffer.size() <= max_inflight_mem_accesses);
	
	// Process completed instructions
	auto it = mem_buffer.begin();
	while (it != mem_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Uop is not ready yet
		if (uop->global_memory_witness)
			break;
	
		// Stall if width has been reached
		if (instructions_processed > width)
		{
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;
		}

		// Sanity check write buffer
		assert((int) write_buffer.size() <= write_buffer_size);

		// Stall if the write buffer is full.
		if ((int) write_buffer.size() == write_buffer_size) 
		{ 		
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;                                 
		}      


		// Update Uop write ready cycle
		uop->write_ready = compute_unit->getTiming()->
			getCycle() + write_latency;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"mem-w\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to write buffer and get the iterator for the next 
		// element
		write_buffer.push_back(std::move(*it));
		it = mem_buffer.erase(it);
	}
}

void VectorMemoryUnit::Memory()
{
	// Get compute unit object
	ComputeUnit *compute_unit = getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;

	// Module access type enum
	mem::Module::AccessType module_access_type;
	
	// Sanity check read buffer
	assert((int) read_buffer.size() <= read_buffer_size);
	
	// Process completed instructions
	auto it = read_buffer.begin();
	while (it != read_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->read_ready)
			break;
	
		// Stall if width has been reached
		if (instructions_processed > width)
		{
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;
		}

		// Sanity check mem buffer
		assert((int) mem_buffer.size() <= max_inflight_mem_accesses);

		// Stall if there is no room in the memory buffer
		if ((int) mem_buffer.size() == max_inflight_mem_accesses)
		{ 		
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;                                 
		}

		// Set the access type
		if (uop->vector_memory_write && !uop->vector_memory_global_coherency)
		{
			module_access_type = mem::Module::AccessType::AccessNCStore;
		}
		else if (uop->vector_memory_write &&
				uop->vector_memory_global_coherency)
		{
			module_access_type = mem::Module::AccessType::AccessStore;
		}
		else if (uop->vector_memory_read)
		{
			module_access_type = mem::Module::AccessType::AccessLoad;
		}
		else if (uop->vector_memory_atomic)
		{
			module_access_type = mem::Module::AccessType::AccessStore;
		}
		else
		{
			throw Timing::Error(misc::fmt("%s: invalid access kind", 
					__FUNCTION__));
		}

		// This variable keeps track if any work items are unsuccessful
		// in making an access to the vector cache.
		bool all_work_items_accessed = true;

		// Access global memory
		assert(!uop->global_memory_witness);
		Timing::pipeline_debug << misc::fmt(
				"\t\t@%lld inst=%lld "
				"id_in_wf=%lld wg=%d/wf=%d (VecMem)\n",
				compute_unit->getTiming()->getCycle(),
				uop->getId(),
				uop->getIdInWavefront(),
				uop->getWorkGroup()->getId(),
				uop->getWavefront()->getId());
		for (auto wi_it = uop->getWavefront()->getWorkItemsBegin(),
				wi_e = uop->getWavefront()->getWorkItemsEnd();
				wi_it != wi_e;
				++wi_it)
		{
			// Get work item
			WorkItem *work_item = wi_it->get();

			// Access memory for each active work-item
			if (uop->getWavefront()->isWorkItemActive(
					work_item->getIdInWavefront()))
			{
				// Get the work item uop
				Uop::WorkItemInfo *work_item_info = 
						&uop->work_item_info_list[
						work_item->getIdInWavefront()];

				// Check if the work item info struct has
				// already made a successful vector cache
				// access. If so, move on to the next work item.
				if (work_item_info->accessed_cache)	
					continue;

				// Translate virtual address to a physical 
				// address
				unsigned physical_address = compute_unit->
						getGpu()->
						getMmu()->
						TranslateVirtualAddress(
						uop->getWorkGroup()->
						getNDRange()->
						address_space,
						work_item_info->
						global_memory_access_address);
		

				// Make sure we can access the vector cache. If 
				// so, submit the access. If we can access the
				// cache, mark the accessed flag of the work 
				// item info struct.
				if (compute_unit->vector_cache->
						canAccess(physical_address))
				{
					compute_unit->vector_cache->Access(
							module_access_type,
							physical_address, 
							&uop->global_memory_witness);
					work_item_info->accessed_cache = true;

					// Access global memory
					uop->global_memory_witness--;
				}
				else
				{
					all_work_items_accessed = false;
				}
			}
		}

		// Make sure that all the work items in the wavefront have 
		// successfully accessed the vector cache. If not, the uop
		// is not moved to the write buffer. Instead, the uop will
		// be re-processed next cycle. Once all work items access
		// the vector cache, the uop will be moved to the write buffer.
		if (!all_work_items_accessed)
			continue;


		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"mem-m\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to exec buffer and get the iterator for the next
		// element
		mem_buffer.push_back(std::move(*it));
		it = read_buffer.erase(it);
	}
}

void VectorMemoryUnit::Read()
{
	// Get compute unit object
	ComputeUnit *compute_unit = getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check decode buffer
	assert((int) decode_buffer.size() <= decode_buffer_size);
	
	// Process completed instructions
	auto it = decode_buffer.begin();
	while (it != decode_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->decode_ready)
			break;
	
		// Stall if width has been reached
		if (instructions_processed > width)
		{
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;
		}

		// Sanity check the read buffer
		assert((int) read_buffer.size() <= read_buffer_size);

		// Stall if the read buffer is full.
		if ((int) read_buffer.size() == read_buffer_size)
		{ 		
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;                                 
		}      


		// Update Uop read ready cycle
		uop->read_ready = compute_unit->getTiming()->
			getCycle() + read_latency;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"mem-r\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to read buffer and get the iterator for the next
		// element
		read_buffer.push_back(std::move(*it));
		it = decode_buffer.erase(it);
	}
}

void VectorMemoryUnit::Decode()
{
	// Get compute unit object
	ComputeUnit *compute_unit = getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check exec buffer
	assert((int) issue_buffer.size() <= issue_buffer_size);
	
	// Process completed instructions
	auto it = issue_buffer.begin();
	while (it != issue_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->issue_ready)
			break;
	
		// Stall if width has been reached
		if (instructions_processed > width)
		{
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;
		}

		// Sanity check the decode buffer
		assert((int) decode_buffer.size() <= decode_buffer_size);

		// Stall if the decode buffer is full.
		if ((int) decode_buffer.size() == decode_buffer_size)
		{ 		
			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"s\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());
			break;                                 
		}      


		// Update Uop write ready cycle
		uop->decode_ready = compute_unit->getTiming()->
			getCycle() + decode_latency;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"mem-d\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to write buffer and get the iterator for the next 
		// element
		decode_buffer.push_back(std::move(*it));
		it = issue_buffer.erase(it);
	}
}


}

