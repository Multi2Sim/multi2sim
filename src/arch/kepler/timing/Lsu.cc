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

#include <arch/kepler/emulator/Grid.h>
#include <arch/kepler/emulator/Warp.h>
#include <arch/kepler/emulator/ThreadBlock.h>
#include <arch/kepler/emulator/Thread.h>
#include <memory/Module.h>
#include <memory/Mmu.h>

#include "Lsu.h"
#include "SM.h"
#include "Gpu.h"
#include "ScoreBoard.h"
#include "Timing.h"


namespace Kepler
{

int LSU::width = 1;
int LSU::dispatch_buffer_size = SM::dispatch_latency;
int LSU::read_buffer_size = 100;
int LSU::read_latency = 1;
int LSU::max_inflight_mem_accesses = 1;
int LSU::write_buffer_size = 1;
int LSU::write_latency = 1;

void LSU::Run()
{
	LSU::Complete();
	LSU::Write();
	LSU::Memory();
	LSU::Read();
}


void LSU::Dispatch(std::unique_ptr<Uop> uop)
{
	// Get sm
	SM *sm = getSM();

	// One more instruction of this kind
	sm->num_memory_instructions++;

	// Dispatch it
	ExecutionUnit::Dispatch(std::move(uop));
}


void LSU::Complete()
{
	// Get SM and GPU object
	SM *sm = getSM();
	GPU *gpu = sm->getGPU();

	// Process completed instructions
	auto it = write_buffer.begin();
	while (it != write_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// Break if uop is not ready
		if (sm->getTiming()->getCycle() < uop->write_ready)
			break;

		// Record trace
		Timing::trace << misc::fmt("kpl.end_inst "
							"id=%lld "
							"sm=%d\n",
							uop->getIdInSM(),
							sm->getId());

		// Release Scoreboard
		sm->getScoreboard(uop->getWarpPoolId())->
			ReleaseRegisters(uop->getWarp(), uop);

		// Remove uop from the write buffer
		it = write_buffer.erase(it);

		// Statistics
		this->num_instructions++;
		gpu->last_complete_cycle = sm->getTiming()->getCycle();
	}
}

void LSU::Write()
{
	// Get SM object
	SM *sm = getSM();

	// Internal counter
	int instructions_processed = 0;

	// Sanity check execution buffer
	assert((int) memory_buffer.size() <= max_inflight_mem_accesses);

	// Process completed instructions
	auto it = memory_buffer.begin();
	while (it != memory_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Break if uop is not ready
		if (sm->getTiming()->getCycle() < uop->execute_ready)
			break;

		// Stall if width has been reached
		if (instructions_processed > width)
		{
			// Trace
			Timing::trace << misc::fmt("kpl.inst "
					"id=%lld "
					"sm=%d "
					"bk=%d "
					"uop_id=%lld "
					"std=\"s\"\n",
					uop->getIdInSM(),
					sm->getId(),
					uop->getWarp()->getId(),
					uop->getIdInSM());
			break;
		}

		// Sanity check write buffer
		assert((int) write_buffer.size() <= write_buffer_size);

		// Stall if the write buffer is full
		if((int) write_buffer.size() == write_buffer_size)
		{
			// Trace
			Timing::trace << misc::fmt("kpl.inst "
					"id=%lld "
					"sm=%d "
					"bk=%d "
					"uop_id=%lld "
					"std=\"s\"\n",
					uop->getIdInSM(),
					sm->getId(),
					uop->getWarp()->getId(),
					uop->getIdInSM());
			break;
		}

		// Update Uop write ready cycle
		uop->write_ready = sm->getTiming()->getCycle() + write_latency;

		// Trace
		Timing::trace << misc::fmt("kpl.inst "
				"id=%lld "
				"sm=%d "
				"bk=%d "
				"uop_id=%lld "
				"std=\"lsu-w\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to write buffer and get the iterator for next
		write_buffer.push_back(std::move(*it));
		it = memory_buffer.erase(it);
	}
}


void LSU::Memory()
{
	// Get SM object
	SM *sm = getSM();

	// Internal counter
	int instructions_processed = 0;

	// Module access type enum
	//mem::Module::AccessType module_access_type;

	// Sanity check read buffer
	assert((int) read_buffer.size() <= read_buffer_size);

	// Process complete instructions
	auto it = read_buffer.begin();
	while (it != read_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Break if uop is not ready
		if (sm->getTiming()->getCycle() < uop->read_ready)
			break;

		// Stall if width has been reached
		if (instructions_processed > width)
		{
			// Trace
			Timing::trace << misc::fmt("kpl.inst "
					"id=%lld "
					"sm=%d "
					"bk=%d "
					"uop_id=%lld "
					"std=\"s\"\n",
					uop->getIdInSM(),
					sm->getId(),
					uop->getWarp()->getId(),
					uop->getIdInSM());
			break;
		}

		// Sanity check memory buffer
		assert((int) memory_buffer.size() <= max_inflight_mem_accesses);

		// Stall if the execute buffer is full
		if((int) memory_buffer.size() == max_inflight_mem_accesses)
		{
			// Trace
			Timing::trace << misc::fmt("kpl.inst "
					"id=%lld "
					"sm=%d "
					"bk=%d "
					"uop_id=%lld "
					"std=\"s\"\n",
					uop->getIdInSM(),
					sm->getId(),
					uop->getWarp()->getId(),
					uop->getIdInSM());
			break;
		}

/*
		// Set the access type

		if (uop->memory_read)
			module_access_type = mem::Module::AccessType::AccessLoad;
		else if (uop->memory_write)
			module_access_type = mem::Module::AccessType::AccessNCStore;

		// This available keeps track if any thread are unsuccessful in making an
		// access to the data cache
		bool all_threads_accessed = true;

		// Access memory
		if (uop->memory_witness)
		{

			std::cout << "Access Type" << module_access_type <<std::endl;
			std::cout << "Uop ID" << uop->getId() << std::endl;
			std::cout <<"memory witness" << uop->memory_witness << std::endl;
			std::cout<<"Uop PC"<< uop->getInstructionPC();
			std::cout<<"	Warp ID" << uop->getWarp()->getId();
			std::cout<<"warp instruction" <<uop->getInstructionOpcode();
			std::cout<<std::endl;

		}

	//	assert(!uop->memory_witness);

		// Update Uop write ready cycle
		for (auto it = uop->getWarp()->ThreadsBegin(),
				e = uop->getWarp()->ThreadsEnd();
				it != e;
				++it)
		{
			// Get thread
			Thread *thread = it->get();

			// Access memory for each active thread
			if(uop->getWarp()->getThreadActive(thread->getIdInWarp()))
			{
				// Get the thread uop
				Uop::ThreadInfo *thread_info = &uop->thread_info_list
						[thread->getIdInWarp()];

				// Check if the thread info struct has already made a successful
				// cache access. If so, move on to the next thread
				if (thread_info->accessed_cache)
					continue;

				// Translate virtual address to a physical address
				unsigned physical_address = sm->getGPU()->getMmu()->
						TranslateVirtualAddress(
						uop->getThreadBlock()->getGrid()->address_space,
						thread_info->global_memory_access_address);

				// Make sure we can access the cache if so, submit the access.
				// if not, mark the accessed flag of the thread info struct
				if (sm->cache->canAccess(physical_address))
				{
					sm->cache->Access(
							module_access_type,
							physical_address);
							//&uop->memory_witness);
					thread_info->accessed_cache = true;

					// Access memory
					uop->memory_witness--;
				}
				else
					all_threads_accessed = false;
			}
		}

		// Make sure that all threads in the warp have successfully accessed the
		// cache. If not, the uop is not moved to the write buffer. Instead,
		// the uop will be re processed next cycle. Once all threads access the
		// cache, the uop will be moved to the write buffer.
		if (!all_threads_accessed)
			continue;
*/
		// Trace
		Timing::trace << misc::fmt("kpl.inst "
				"id=%lld "
				"sm=%d "
				"bk=%d "
				"uop_id=%lld "
				"std=\"mem-m\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to write buffer and get the iterator for next
		memory_buffer.push_back(std::move(*it));
		it = read_buffer.erase(it);
	}
}


void LSU::Read()
{
	// Get SM object
	SM *sm = getSM();

	// Internal counter
	int instructions_processed = 0;

	// Sanity check read buffer
	assert((int) dispatch_buffer.size() <= dispatch_buffer_size);

	// Process complete instructions
	auto it = dispatch_buffer.begin();
	while (it != dispatch_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Break if uop is not ready
		if (sm->getTiming()->getCycle() < uop->dispatch_ready)
			break;

		// Stall if width has been reached
		if (instructions_processed > width)
		{
			// Trace
			Timing::trace << misc::fmt("kpl.inst "
					"id=%lld "
					"sm=%d "
					"bk=%d "
					"uop_id=%lld "
					"std=\"s\"\n",
					uop->getIdInSM(),
					sm->getId(),
					uop->getWarp()->getId(),
					uop->getIdInSM());
			break;
		}

		// Sanity check read buffer
		assert((int) read_buffer.size() <= read_buffer_size);

		// Stall if the read buffer is full
		if((int) read_buffer.size() == read_buffer_size)
		{
			// Trace
			Timing::trace << misc::fmt("kpl.inst "
					"id=%lld "
					"sm=%d "
					"bk=%d "
					"uop_id=%lld "
					"std=\"s\"\n",
					uop->getIdInSM(),
					sm->getId(),
					uop->getWarp()->getId(),
					uop->getIdInSM());
			break;
		}

		// Update Uop write ready cycle
		uop->read_ready = sm->getTiming()->getCycle() + read_latency;

		// Trace
		Timing::trace << misc::fmt("kpl.inst "
				"id=%lld "
				"sm=%d "
				"bk=%d "
				"uop_id=%lld "
				"std=\"lsu-r\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to write buffer and get the iterator for next
		read_buffer.push_back(std::move(*it));
		it = dispatch_buffer.erase(it);
	}
}


} // namespace Kepler
