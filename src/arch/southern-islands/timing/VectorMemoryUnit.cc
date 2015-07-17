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
#include <memory/MMU.h>

#include "CycleIntervalReport.h"
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
	VectorMemoryUnit::Complete();
	VectorMemoryUnit::Write();
	VectorMemoryUnit::Memory();
	VectorMemoryUnit::Read();
	VectorMemoryUnit::Decode();
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

void VectorMemoryUnit::Issue(std::shared_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();

	// One more instruction of this kind
	compute_unit->num_vector_memory_instructions++;
	uop->getWavefrontPoolEntry()->lgkm_cnt++;

	// Issue it
	ExecutionUnit::Issue(uop);
}

void VectorMemoryUnit::Complete()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	Gpu *gpu = compute_unit->getGpu();

	// Sanity check the write buffer
	assert((int) write_buffer.size() <= width);

	// Process completed instructions
	for (auto it = write_buffer.begin(),
			e = write_buffer.end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// Continue if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->write_ready)
			continue;
	
		// Access complete, remove the uop from the queue
		assert(uop->getWavefrontPoolEntry()->lgkm_cnt > 0);
		uop->getWavefrontPoolEntry()->lgkm_cnt--;
		
		// Record trace
		Timing::trace << misc::fmt("si.end_inst "
				"id=%lld "
				"cu=%d\n ",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex());

		// Access complete, remove the uop from the queue
		write_buffer.erase(it);

		// Statistics
		this->inst_count++;
		gpu->last_complete_cycle = compute_unit->getTiming()->getCycle();
	}
}

void VectorMemoryUnit::Write()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check the mem buffer
	assert((int) mem_buffer.size() <= max_inflight_mem_accesses);
	
	// Process completed instructions
	for (auto it = mem_buffer.begin(),
			e = mem_buffer.end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Uop is not ready yet
		if (uop->global_mem_witness)
			continue;
	
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
			continue;
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
			continue;                                 
		}      


		// Update Uop write ready cycle
		uop->write_ready = compute_unit->getTiming()->
			getCycle() + write_latency;

		// In the above context, access means any of the                 
		// mod_access calls in si_vector_mem_mem. Means all              
		// inflight accesses for uop are done
		if (CycleIntervalReport::spatial_report_active)
		{                                                                
			if (uop->vector_mem_write)                               
			{        
				inflight_mem_accesses -= 
						uop->num_global_mem_write;
			}                                                        
			else if (uop->vector_mem_read)                           
			{                                                        
				inflight_mem_accesses -= 
						uop->num_global_mem_read;
			}                                                        
			else                                                     
			{
				throw Timing::Error(misc::fmt(
						"%s: invalid access kind",
						__FUNCTION__));  
			}                                                        
		}

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

		// Move uop to write buffer
		write_buffer.push_back(std::move(*it));
		mem_buffer.erase(it);
	}
}

void VectorMemoryUnit::Memory()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;

	// Module access type enum
	mem::Module::AccessType module_access_type;
	mem::MMU::AccessType mmu_access_type;

	
	// Sanity check read buffer
	assert((int) read_buffer.size() <= read_buffer_size);
	
	// Process completed instructions
	for (auto it = read_buffer.begin(),
			e = read_buffer.end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Continue if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->read_ready)
			continue;
	
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
			continue;
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
			continue;                                 
		}

		// Set the access type
		if (uop->vector_mem_write && !uop->vector_mem_global_coherency)
		{
			module_access_type = mem::Module::AccessType::AccessNCStore;
			mmu_access_type = mem::MMU::AccessType::AccessWrite;
		}
		else if (uop->vector_mem_write && 
				uop->vector_mem_global_coherency)
		{
			module_access_type = mem::Module::AccessType::AccessStore;
			mmu_access_type = mem::MMU::AccessType::AccessWrite;
		}
		else if (uop->vector_mem_read)
		{
			module_access_type = mem::Module::AccessType::AccessLoad;
			mmu_access_type = mem::MMU::AccessType::AccessRead;
		}
		else if (uop->vector_mem_atomic)
		{
			module_access_type = mem::Module::AccessType::AccessStore;
			mmu_access_type = mem::MMU::AccessType::AccessWrite;
		}
		else
		{
			throw Timing::Error(misc::fmt("%s: invalid access kind", 
					__FUNCTION__));
		}

		// Access global memory
		assert(!uop->global_mem_witness);
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

				// Translate virtual address to a physical 
				// address
				unsigned phys_addr = compute_unit->getGpu()->
						getMmu()->
						TranslateVirtualAddress(
						uop->getWorkGroup()->
						getNDRange()->
						getAddressSpace(),
						work_item_info->
						global_mem_access_addr);

				// Submit the access
				compute_unit->scalar_cache->Access(
						module_access_type,
						phys_addr, 
						&uop->global_mem_witness);

				// MMU statistics
				compute_unit->getGpu()->getMmu()->AccessPage(
						phys_addr,
						mmu_access_type);

				// Decrement global_mem_witness
				uop->global_mem_witness--;
			}
		}

		// Spatial Report
		if (CycleIntervalReport::spatial_report_active)
		{                                                                
			if (uop->vector_mem_write)                               
			{                                                        
				uop->num_global_mem_write +=                     
					uop->global_mem_witness;                 
				inflight_mem_accesses += 
						uop->num_global_mem_write;
			}                                                        
			else if (uop->vector_mem_read)                           
			{                                                        
				uop->num_global_mem_read +=                      
					uop->global_mem_witness;                 
				inflight_mem_accesses += 
						uop->num_global_mem_read;
			}                                                        
			else
			{
				throw Timing::Error(misc::fmt(
						"%s: invalid access kind",
						__FUNCTION__));  
			}
		}

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

		// Move uop to exec buffer
		write_buffer.push_back(std::move(*it));
		mem_buffer.erase(it);
	}
}

void VectorMemoryUnit::Read()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check decode buffer
	assert((int) decode_buffer.size() <= decode_buffer_size);
	
	// Process completed instructions
	for (auto it = decode_buffer.begin(),
			e = decode_buffer.end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Continue if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->decode_ready)
			continue;
	
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
			continue;
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
			continue;                                 
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

		// Move uop to read buffer
		read_buffer.push_back(std::move(*it));
		decode_buffer.erase(it);
	}
}

void VectorMemoryUnit::Decode()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check exec buffer
	assert((int) issue_buffer.size() <= issue_buffer_size);
	
	// Process completed instructions
	for (auto it = issue_buffer.begin(),
			e = issue_buffer.end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Continue if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->issue_ready)
			continue;
	
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
			continue;
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
			continue;                                 
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

		// Move uop to write buffer
		decode_buffer.push_back(std::move(*it));
		issue_buffer.erase(it);
	}
}


}

