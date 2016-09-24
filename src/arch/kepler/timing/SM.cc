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

#include <arch/kepler/disassembler/Instruction.h>
#include <arch/kepler/emulator/Grid.h>
#include <arch/kepler/emulator/ThreadBlock.h>
#include <arch/kepler/emulator/Thread.h>
#include <arch/kepler/emulator/Warp.h>
#include <memory/Module.h>

#include "ExecutionUnit.h"
#include "SM.h"
#include "Timing.h"
#include "WarpPool.h"



namespace Kepler
{

int SM::num_warp_pools = 4;
int SM::num_shared_spus = 2 * num_warp_pools;
int SM::num_shared_dpus = 2 * num_warp_pools;
int SM::num_shared_imus = 1 * num_warp_pools;
int SM::num_shared_sfus = 1 * num_warp_pools;
int SM::num_shared_lsus = 1 * num_warp_pools;
int SM::max_blocks_per_sm = 16;
int SM::max_blocks_per_warp_pool = 4;
int SM::max_warps_per_sm = 64;
int SM::max_warps_per_warp_pool = 16;
int SM::fetch_latency = 2;//8;
int SM::fetch_width = 2;
int SM::fetch_buffer_size = 10;
int SM::dispatch_latency = 2;//13;
int SM::dispatch_width = 5;
int SM::max_instructions_dispatched_per_type = 2;


SM::SM(int id, GPU *gpu) :
		gpu(gpu),
		id(id)
{
	this->timing = Timing::getInstance();
	// Create warp pools
	warp_pools.resize(num_warp_pools);
	is_warp_pool_available.resize(num_warp_pools, true);

	// Create fetch buffer
	fetch_buffers.resize(num_warp_pools);

	// Create all private units
	private_spus.resize(num_warp_pools);
	private_brus.resize(num_warp_pools);

	// Create all shared units
	shared_spus.resize(num_shared_spus);
	shared_dpus.resize(num_shared_dpus);
	shared_lsus.resize(num_shared_lsus);
	shared_imus.resize(num_shared_imus);
	shared_sfus.resize(num_shared_sfus);

	is_private_spus_available.resize(num_warp_pools, 1);
	is_private_brus_available.resize(num_warp_pools, 1);

	private_spus_next_available_cycle.resize(num_warp_pools, 1);
	private_brus_next_available_cycle.resize(num_warp_pools, 1);

	is_shared_spus_available.resize(num_shared_spus, 1);
	is_shared_dpus_available.resize(num_shared_dpus, 1);
	is_shared_lsus_available.resize(num_shared_lsus, 1);
	is_shared_imus_available.resize(num_shared_imus, 1);
	is_shared_sfus_available.resize(num_shared_sfus, 1);

	shared_spus_next_available_cycle.resize(num_shared_spus, -1);
	shared_dpus_next_available_cycle.resize(num_shared_dpus, -1);
	shared_lsus_next_available_cycle.resize(num_shared_lsus, -1);
	shared_imus_next_available_cycle.resize(num_shared_imus, -1);
	shared_sfus_next_available_cycle.resize(num_shared_sfus, -1);


	scoreboard.resize(num_warp_pools);

	// Initial all units warp pool and ...
	for (int i = 0; i < num_warp_pools; i++)
	{
		private_brus[i] = misc::new_unique<BRU>(i,this);
		private_spus[i] = misc::new_unique<SPU>(i,this);
		shared_lsus[i] = misc::new_unique<LSU>(i,this);
		shared_imus[i] = misc::new_unique<IMU>(i,this);
		shared_sfus[i] = misc::new_unique<SFU>(i,this);
		warp_pools[i] = misc::new_unique<WarpPool>(i,this);
		fetch_buffers[i] = misc::new_unique<FetchBuffer>(i, this);
		scoreboard[i] = misc::new_unique<ScoreBoard>(i, this);

		shared_spus[2 * i] = misc::new_unique<SPU>(2 * i,this);
		shared_spus[2 * i + 1] = misc::new_unique<SPU>(2 * i + 1, this);
		shared_dpus[2 * i] = misc::new_unique<DPU>(2 * i,this);
		shared_dpus[2 * i + 1] = misc::new_unique<DPU>(2 * i + 1,this);
	}

	// Initialize last id of last warp pool was mapped to
	last_mapped_warp_pool_id = -1;
}


void SM::DispatchToExecutionUnit(std::unique_ptr<Uop> uop,
		ExecutionUnit *execution_unit)
{
	//long long sm_id = uop->getIdInSM();
	//int warp_id = uop->getWarp()->getId();
	//long long id_in_warp = uop->getIdInWarp();

	// Dispatch to execution unit. Take unique pointer ownership directly.
	execution_unit->Dispatch(std::move(uop));

	// Trace
	/*
	Timing::trace << misc::fmt("kpl.instruction "
			"id=%lld "
			"sm=%d "
			"warp=%d "
			"uop_id=%lld "
			"stage=\"i\"\n",
			sm_id,
			id,
			warp_id,
			id_in_warp);
	*/
}


void SM::Dispatch(FetchBuffer *fetch_buffer, WarpPool *warp_pool)
{

	// Number of total dispatched instructions
	int number_dispatched = 0;

	// Number of instructions dispatched to private SPU
	int dispatched_to_pspu = 0;

	// Number of instructions dispatched to private bru
	int dispatched_to_bru = 0;

	// A round robin scheduler chooses a warp from the fetch buffer to dispatch
	for (int i = 0; i < max_warps_per_warp_pool; i++)
	{
		// Get warp pool entry index
		unsigned index = (fetch_buffer->getLastDispatchedWarpIndex() + i + 1) %
				max_warps_per_warp_pool;

		// Get warp pool entry
		WarpPoolEntry *warp_pool_entry = (warp_pool->begin() + index)
				->get();

		// Get fetch buffer entry id which equals the warp pool entry id
		int fetch_buffer_entry_index = warp_pool_entry->getIdInWarpPool();

		// Dispatch fixed number of instructions per cycle
		for (int j = 0; j < fetch_width; j++)
		{
			// Get the index in the valid buffer. There are two instructions per
			// fetch buffer entry.
			int buffer_index = fetch_buffer_entry_index *
					fetch_buffer->num_instructions_per_entry + j;

			// Check if the slot in the fetch buffer entry is valid for dispatch.
			// InValid means there are no fetched instructions in the slot.
			if (!fetch_buffer->IsValidEntry(buffer_index))
				continue;

			// Get Warp
			Warp *warp = warp_pool_entry->getWarp();

			if (!warp)
				continue;

			// Check if the warp is at barrier
			if (warp->getAtBarrier())
				continue;

			// Get the fetch buffer uop iterator
			auto it = fetch_buffer->begin() + buffer_index;

			// If there is control hazard
			// This part of code can be used in
			// future. For now, there is a branch pending bit in fetch, when there
			// is a branch instruction dispatched, the branch pending bit is set
			// to true so SM stops fetch this warp until the branch instruction
			// is finished.
		/*	if (warp->getPC() != (*it)->getInstructionPC())
			{
				fetch_buffer->FlushFetchBufferEntrySet(buffer_index);

				// Also need to delete the uop
				(*it).reset();

				// skip the dispatch
				continue;
			}
		*/
			// Get Uop warp and instruction
			Uop *uop = (*it).get();

			// Check id the warp pc is the same as uop pc, if not, check next
			// fetch buffer entry
			if(warp->getPC() != uop->getInstructionPC())
				break;

			// Skip uops that have not completed fetch
			if (timing->getCycle() < uop->fetch_ready)
				continue;

			// Get the instruction
			//Instruction *instruction = uop->getInstruction();

			// Record if the slot in the entry is dispatched
			bool is_dispatched = false;

			// Decode the instruction before scoreboard
			Instruction::Opcode opcode = warp->Decode();

			// Get current instruction of the warp
			Instruction *instruction = warp->getInstruction();


			// Read registers index used and instruction format in new
			// instruction
			instruction->ReadRegistersIndex(opcode, uop);

			uop->setInstructionOpcode(opcode);

			// Check collision
			if (scoreboard[warp_pool->getId()]->
					CheckCollision(warp, uop) &&
					(instruction->getFormat() != Instruction::FormatInvalid))
					break;

			if(instruction->getFormat() == Instruction::FormatInteger1)
			{
				if (dispatched_to_pspu < 1)
				{
					DispatchToExecutionUnit(std::move(*it),
							private_spus[warp_pool->getId()].get());
					dispatched_to_pspu++;
					is_dispatched = true;
				}
				else if (is_shared_imus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_imus[warp_pool->getId()].get());
					is_shared_imus_available[warp_pool->getId()] = 0;
					shared_imus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatInteger2)
			{
				// Check the first shared spu
				if (is_shared_spus_available[2 * warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId()].get());
					is_shared_spus_available[2 * warp_pool->getId()] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				// Check the second shared spu
				else if (is_shared_spus_available[2 * warp_pool->getId() + 1])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId() + 1].get());
					is_shared_spus_available[2 * warp_pool->getId() + 1] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId() + 1]
						=  timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatInteger3)
			{
				if (is_shared_imus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_imus[warp_pool->getId()].get());
					is_shared_imus_available[warp_pool->getId()] = 0;
					shared_imus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatFP32_1)
			{
				if (dispatched_to_pspu < 1)
				{
					DispatchToExecutionUnit(std::move(*it),
							private_spus[warp_pool->getId()].get());
					dispatched_to_pspu++;
					is_dispatched = true;
				}
				// Check the first shared spu
				else if (is_shared_spus_available[2 * warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId()].get());
					is_shared_spus_available[2 * warp_pool->getId()] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				// Check the second shared spu
				else if (is_shared_spus_available[2 * warp_pool->getId() + 1])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId() + 1].get());
					is_shared_spus_available[2 * warp_pool->getId() + 1] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId() + 1]
						=  timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatFP32_2)
			{
				// Check the first shared spu
				if (is_shared_spus_available[2 * warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId()].get());
					is_shared_spus_available[2 * warp_pool->getId()] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				// Check the second shared spu
				else if (is_shared_spus_available[2 * warp_pool->getId() + 1])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId() + 1].get());
					is_shared_spus_available[2 * warp_pool->getId() + 1] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId() + 1]
						=  timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatFP64)
			{
				// Check the first shared dpu
				if (is_shared_dpus_available[2 * warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_dpus[2 * warp_pool->getId()].get());
					is_shared_dpus_available[2 * warp_pool->getId()] = 0;
					shared_dpus_next_available_cycle[2 * warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				// Check the second shared dpu
				else if (is_shared_dpus_available[2 * warp_pool->getId() + 1])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_dpus[2 * warp_pool->getId() + 1].get());
					is_shared_dpus_available[2 * warp_pool->getId() + 1] = 0;
					shared_dpus_next_available_cycle[2 * warp_pool->getId() + 1]
						=  timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatControl)
			{
				if (dispatched_to_bru < 1)
				{
					DispatchToExecutionUnit(std::move(*it),
							private_brus[warp_pool->getId()].get());
					dispatched_to_bru++;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatLS)
			{
				if (is_shared_lsus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_lsus[warp_pool->getId()].get());
					is_shared_lsus_available[warp_pool->getId()] = 0;
					shared_lsus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatConversion)
			{
				// Check the first shared dpu
				if (is_shared_dpus_available[2 * warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_dpus[2 * warp_pool->getId()].get());
					is_shared_dpus_available[2 * warp_pool->getId()] = 0;
					shared_dpus_next_available_cycle[2 * warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				// Check the second shared dpu
				else if (is_shared_dpus_available[2 * warp_pool->getId() + 1])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_dpus[2 * warp_pool->getId() + 1].get());
					is_shared_dpus_available[2 * warp_pool->getId() + 1] = 0;
					shared_dpus_next_available_cycle[2 * warp_pool->getId() + 1]
						=  timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatBit1)
			{
				if (dispatched_to_pspu < 1)
				{

					DispatchToExecutionUnit(std::move(*it),
							private_spus[warp_pool->getId()].get());
					dispatched_to_pspu++;
					is_dispatched = true;
				}
				// Check the first shared spu
				else if (is_shared_spus_available[2 * warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId()].get());
					is_shared_spus_available[2 * warp_pool->getId()] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				// Check the second shared spu
				else if (is_shared_spus_available[2 * warp_pool->getId() + 1])
				{
					DispatchToExecutionUnit(std::move(*it),
						shared_spus[2 * warp_pool->getId() + 1].get());
					is_shared_spus_available[2 * warp_pool->getId() + 1] = 0;
					shared_spus_next_available_cycle[2 * warp_pool->getId() + 1]
						=  timing->getCycle() + 4;
					is_dispatched = true;
				}
				else if (is_shared_imus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_imus[warp_pool->getId()].get());
					is_shared_imus_available[warp_pool->getId()] = 0;
					shared_imus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				else if (is_shared_sfus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_sfus[warp_pool->getId()].get());
					is_shared_sfus_available[warp_pool->getId()] = 0;
					shared_sfus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatBit2)
			{
				if (dispatched_to_pspu < 1)
				{
					this->DispatchToExecutionUnit(std::move(*it),
							private_spus[warp_pool->getId()].get());
					dispatched_to_pspu++;
					is_dispatched = true;
				}
				else if (is_shared_imus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_imus[warp_pool->getId()].get());
					is_shared_imus_available[warp_pool->getId()] = 0;
					shared_imus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
				else if (is_shared_sfus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_sfus[warp_pool->getId()].get());
					is_shared_sfus_available[warp_pool->getId()] = 0;
					shared_sfus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatBit3)
			{
				if (is_shared_imus_available[warp_pool->getId()])
				{
					DispatchToExecutionUnit(std::move(*it),
							shared_imus[warp_pool->getId()].get());
					is_shared_imus_available[warp_pool->getId()] = 0;
					shared_imus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatSFU)
			{
				 if (is_shared_sfus_available[warp_pool->getId()])
				 {
					DispatchToExecutionUnit(std::move(*it),
							shared_sfus[warp_pool->getId()].get());
					is_shared_sfus_available[warp_pool->getId()] = 0;
					shared_sfus_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 4;
					is_dispatched = true;
				}
			}
			else if (instruction->getFormat() == Instruction::FormatInvalid)
				is_dispatched = true;

			if (is_dispatched)
			{
				number_dispatched++;
				fetch_buffer->FlushFetchBufferEntrySet(buffer_index);
				scoreboard[warp_pool->getId()]->ReserveRegisters(warp, uop);

				// Reset is dispatched in every slot in the entry
				is_dispatched = false;

				// Emulate instructions
				warp->Execute(opcode);

				// Increase number of instructions executed in
				// SM
				num_total_instructions++;

				// Increase number of instructions executed in
				// GPU
				Timing::getInstance()->num_SIMD_instructions++;
				// Update memory accesses
				if (instruction->getFormat() == Instruction::FormatLS)
				{
					for (auto it = warp->ThreadsBegin(), e = warp->ThreadsEnd();
						it != e; ++it)
					{
						// Get Thread
						Thread * thread = it->get();

						// Get uop thread info
						Uop::ThreadInfo *thread_info;
						thread_info =
								&uop->thread_info_list[thread->getIdInWarp()];

						// Global memory
						thread_info->global_memory_access_address =
								thread->global_memory_access_address;
						thread_info->global_memory_access_size =
								thread->global_memory_access_size;
					}
				}

				// Check if warp emulation is finished. If finished, we can also
				// set the finished timing to be true.
				if (warp->getFinishedEmu())
				{
					uop->last_instruction_in_warp = warp->getFinishedEmu();
				}

				// Convert instruction name to string
				std::string instruction_name = warp->getInstruction()->getName();
				misc::StringSingleSpaces(instruction_name);

				// Trace
				Timing::trace << misc ::fmt("kpl.instruction "
						"id=%lld "
						"sm=%d "
						"warp=%d "
						"uop_id=%lld "
						"stg=\"d\" "
						"cycle=%lld "
						"asm=\"%s\"\n",
						uop->getIdInSM(),
						id,
						uop->getWarp()->getId(),
						uop->getIdInWarp(),
						timing->getCycle(),
						instruction_name.c_str());

				// Access instruction cache. Record the time when the
				// instruction will have been fetched, as per the latency of the
				// instrution memory
				uop->dispatch_ready = timing->getCycle() + dispatch_latency;
			}

			// If the first instruction in the entry is a branch and it is
			// dispatched. Flash the other slot in the fetch buffer entry
			// (for future leave the second one for next cycle to
			// avoid control hazard)
			if (dispatched_to_bru == 1)
			{
				// If the instruction reside in the first of the entry
				if (j == 0)
				{
					// Flush fetch buffer valid bit
					fetch_buffer->FlushFetchBufferEntrySet(buffer_index + 1);

					// Also need to delete the associated uop
					auto index = fetch_buffer->begin() + buffer_index + 1;
					(*index).reset();
				}

				// Set the branch pending bit
				fetch_buffer->setBranchPendingBit(fetch_buffer_entry_index,
						true);
				break;
			}

			// Added for different number of lanes in a unit.
			if (dispatched_to_pspu)
			{
				is_warp_pool_available[warp_pool->getId()] = false;
				warp_pool_next_available_cycle[warp_pool->getId()] =
							timing->getCycle() + 8;
			}
			if ((opcode == Instruction::INST_EXIT) && (j == 0))
				break;
		}


/*
		if (number_dispatched)
		{
			fetch_buffer->setLastDispatchedWarpIndex(fetch_buffer_entry_index);
			break;
		}
*/


		///////
		fetch_buffer->setLastDispatchedWarpIndex(index);
		break;
	}
}

void SM::Fetch(FetchBuffer *fetch_buffer, WarpPool *warp_pool)
{
	// Check
	assert(fetch_buffer);
	assert(warp_pool);
	assert(fetch_buffer->getId() == warp_pool->getId());

	// Flag shows whether there is instruction fetched for the warp
	bool instructions_fetched = false;

	// Fetch the instructions
	for (int i = 0; i < max_warps_per_warp_pool; i++)
	{
		// Get warp pool entry index
		unsigned index = (fetch_buffer->getLastFetchedWarpIndex() + i + 1) %
				max_warps_per_warp_pool;

		// Get warp pool entry
		WarpPoolEntry *warp_pool_entry = (*(warp_pool->begin() + index)).get();

		// Get Warp
		Warp *warp = warp_pool_entry->getWarp();

		// Check if the warp is valid
		if (!warp)
			continue;

		if (warp->getFinishedEmu())
			continue;

		// Get fetch buffer entry id which equals the warp pool entry id
		int fetch_buffer_entry_index = warp_pool_entry->getIdInWarpPool();

		// Check if the fetch buffer is valid for fetch. Valid means there are
		// still fetched instructions haven't been dispatched. So continue to
		// check next entry
		if (fetch_buffer->IsValidEntry(fetch_buffer_entry_index * 2) ||
				fetch_buffer->IsValidEntry(fetch_buffer_entry_index * 2 + 1))
			continue;

		// Check the fetch buffer pending bit. If the pending bit is true,
		// skip this warp
		if(fetch_buffer->getBranchPendingBit(fetch_buffer_entry_index))
			continue;

		int fetch_pc = warp->getPC();

		// Fetch instructions
		// Create uop
		for (int i = 0; i < fetch_width; i++)
		{
			if (!warp->getFinishedEmu())
			//if (fetch_pc < warp->getLastInstructionPC())
			{
				auto uop = misc::new_unique<Uop>(
						fetch_buffer,
						warp,
						warp_pool_entry,
						timing->getCycle(),
						warp->getThreadBlock(),
						fetch_buffer->getId(),
						warp->getPC() + i * 8);

				// Check
				assert(warp->getThreadBlock() && uop->getThreadBlock());

				// Convert instruction name to string
				//std::string instruction_name = warp->getInstruction()->getName();
				//misc::StringSingleSpaces(instruction_name);

				// Trace
				Timing::trace << misc::fmt("kpl.new_inst "
						"id=%lld "
						"sm=%d "
						"WarpPool=%d "
						"warp=%d "
						"uop_id=%lld "
						"stage=\"f\" "
						"cycle=%lld \n",
						uop->getIdInSM(),
						id,
						warp_pool->getId(),
						uop->getWarp()->getId(),
						uop->getIdInWarp(),
						timing->getCycle());

				// Access instruction cache. Record the time when the
				// instruction will have been fetched, as per the latency of the
				// instruction memory
				uop->fetch_ready = timing->getCycle() + fetch_latency;

				// Check the instruction memory latency for every cache line
/*
				if (fetch_pc % 64 == 0)
					uop->fetch_ready += 25;

				if ((fetch_pc % 64 == 8) && i == 1)
					uop->fetch_ready += 25;
*/
				// Insert uop into fetch buffer
				fetch_buffer->AddToFetchBuffer(fetch_buffer_entry_index *2 + i,
						std::move(uop));
				fetch_pc += 8;
				instructions_fetched = true;
			}
		}

		if (instructions_fetched)
		{
			fetch_buffer->setLastFatchedWarpIndex(fetch_buffer_entry_index);
			break;
		}
	}
}


void SM::MapBlock(ThreadBlock *thread_block)
{
	//Check
	assert(thread_block);
	assert((int)thread_blocks.size() < gpu->getNumBlocksAllowedPerSM());

	// Find an available slot
	while (thread_block->id_in_sm < gpu->getNumBlocksAllowedPerSM()
			&& thread_block->id_in_sm < (int) thread_blocks.size())
		thread_block->id_in_sm++;

	// Check
	assert(thread_block->id_in_sm < gpu->getNumBlocksAllowedPerSM());

	// Insert block into the list
	AddBlock(thread_block);

	//Check
	assert((int) thread_blocks.size() <= gpu->getNumBlocksAllowedPerSM());

	// If SM is not full, add it back to the available list
	if ((int) thread_blocks.size() < gpu->getNumBlocksAllowedPerSM())
	{
		if (!in_available_sms)
			gpu->InsertInAvailableSMs(this);
	}

	// Assign warp identifiers in SM
	int warp_id = 0;

	for (auto it = thread_block->WarpsBegin(); it != thread_block->WarpsEnd();
			++it)
	{
		// Get warp pool id for the current warp
		int warp_pool_id = (last_mapped_warp_pool_id + 1) % num_warp_pools;

		// Number of warps the given thread block contains
		int num_warps = thread_block->getWarpCount();

		// Get Warp pointer
		Warp *warp = it->get();

		// Set Warp Id
		warp->id_in_sm = thread_block->id_in_sm * num_warps
				+ warp_id;

		// Map the given thread block to the warp pools
		warp_pools[warp_pool_id]->MapWarp(warp);

		// Update internal counter
		warp_id++;

		// Update last mapped warp pool id
		last_mapped_warp_pool_id = (last_mapped_warp_pool_id + 1) %
				num_warp_pools;
	}

	// Increment count of mapped blocks
	num_mapped_blocks++;

	Timing::trace << misc::fmt("kpl.map.blk "
								"sm=%d "
								"b=%d "
								"t_first=%d "
								"t_count=%d "
								"b_first=%d "
								"b_count=%d\n",
								id,thread_block->getId(),
								thread_block->getThread(0)->getId(),
								thread_block->getThreadsCount(),
								thread_block->getWarp(0)->getId(),
								thread_block->getWarpCount());
}


void SM::AddBlock(ThreadBlock *thread_block)
{
	// Add thread block
	thread_blocks.push_back(thread_block);

	// Save iterator
	auto it = thread_blocks.begin();
	std::advance(it,thread_block->id_in_sm);
	thread_block->sm_thread_block_iterator = it;
}


void SM::RemoveBlock(ThreadBlock *thread_block)
{
	// Erase thread block
	assert(thread_block->sm_thread_block_iterator != thread_blocks.end());
	thread_blocks.erase(thread_block->sm_thread_block_iterator);
}


void SM::UnmapBlock(ThreadBlock *thread_block)
{
	// Get GPU object
	GPU *gpu = getGPU();

	// Remove the thread block from the list
	assert(thread_blocks.size() > 0);
	RemoveBlock(thread_block);

	// Unmap warps
	for (auto it = thread_block->WarpsBegin(); it != thread_block->WarpsEnd();
			++it)
	{
		Warp *warp = it->get();
		WarpPoolEntry *warp_pool_entry = warp->getWarpPoolEntry();

		// Get id of warp pool the entry belongs to
		int warp_pool_id = warp_pool_entry->getWarpPool()->getId();

		// Unmap warp from the pool
		warp_pools[warp_pool_id]->UnmapWarp(warp);
	}

	// If SM is not already in the available list, place it there
	if(!in_available_sms)
		gpu->InsertInAvailableSMs(this);

	// Trace
	Timing::trace << misc::fmt("kpl.unmap_blk sm=%d tb=%d\n", id,
			thread_block->getId());
}


void SM::Run()
{
	// Return if no block is mapped to this compute
	if(!thread_blocks.size())
		return;

	// Update shared units available
	for (int i = 0; i < num_shared_spus; i++)
	{
		if (!is_shared_spus_available[i])
		{
			if ( timing->getCycle() == shared_spus_next_available_cycle[i] )
				is_shared_spus_available[i] = 1;
		}
	}

	for (int i = 0; i < num_shared_dpus; i++)
	{
		if (!is_shared_dpus_available[i])
		{
			if ( timing->getCycle() == shared_dpus_next_available_cycle[i] )
				is_shared_dpus_available[i] = 1;
		}
	}

	for (int i = 0; i < num_shared_lsus; i++)
	{
		if (!is_shared_lsus_available[i])
		{
			if ( timing->getCycle() == shared_lsus_next_available_cycle[i] )
				is_shared_lsus_available[i] = 1;
		}
	}

	for (int i = 0; i < num_shared_imus; i++)
	{
		if (!is_shared_imus_available[i])
		{
			if ( timing->getCycle() == shared_imus_next_available_cycle[i] )
				is_shared_imus_available[i] = 1;
		}
	}

	for (int i = 0; i < num_shared_sfus; i++)
	{
		if (!is_shared_sfus_available[i])
		{
			if ( timing->getCycle() == shared_sfus_next_available_cycle[i] )
				is_shared_sfus_available[i] = 1;
		}
	}

	// Add for different number of lanes in a unit.
	for (int i = 0; i < num_warp_pools; i++)
	{
		if (!is_warp_pool_available[i])
		{
			if (timing->getCycle() == warp_pool_next_available_cycle[i])
				is_warp_pool_available[i] = true;
		}
	}
	// Private Spus
	for (auto &pspu : private_spus)
		pspu->Run();

	// Private Bru
	for (auto &sbru : private_brus)
		sbru->Run();

	// Shared Spus
	for (auto &sspu : shared_spus)
		sspu->Run();

	// Shared DPU
	for (auto &sdpu : shared_dpus)
		sdpu->Run();

	// Shared LSU
	for (auto &slsu : shared_lsus)
		slsu->Run();


	// Shared IMU
	for (auto &simu : shared_imus)
		simu->Run();

	// Shared SFU
	for (auto &ssfu : shared_sfus)
		ssfu->Run();

	// Dispatch
	for (int i = 0; i < num_warp_pools; i++)
	{
		if (is_warp_pool_available[i])
			Dispatch(fetch_buffers[i].get(), warp_pools[i].get());
	}

	// Fetch
	for (int i = 0; i < num_warp_pools; i++)
		Fetch(fetch_buffers[i].get(), warp_pools[i].get());
}


} // namespace Kepler
