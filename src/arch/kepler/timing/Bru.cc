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

#include <arch/kepler/emulator/Warp.h>

#include "Bru.h"
#include "SM.h"
#include "Gpu.h"
#include "Timing.h"
#include "ScoreBoard.h"


namespace Kepler
{

int BRU::width = 1;
int BRU::num_bru_lanes = 32;
int BRU::dispatch_buffer_size = SM::dispatch_latency;
int BRU::read_latency = 5;
int BRU::read_buffer_size = read_latency;
int BRU::execute_latency = 5;
int BRU::execute_buffer_size = execute_latency;
int BRU::write_latency = 5;
int BRU::write_buffer_size = write_latency;

void BRU::Run()
{
	BRU::Complete();
	BRU::Write();
	BRU::Execute();
	BRU::Read();
}


void BRU::Dispatch(std::unique_ptr<Uop> uop)
{
	// One more instruction of this kind
	SM *sm = getSM();

	// One more instruciton of this kind
	sm->num_branch_instructions++;

	// Dispatch it
	ExecutionUnit::Dispatch(std::move(uop));
}


void BRU::Complete()
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

		// Get thread block
		ThreadBlock *thread_block = uop->getThreadBlock();

		// Get grid
		Grid *grid = uop->getThreadBlock()->getGrid();

		if (uop->last_instruction_in_warp)
		{
			uop->getWarpPoolEntry()->warp_finished = true;
			thread_block->incWarpsCompletedTiming();

			if (thread_block->getNumWarpsCompletedTiming() ==
					thread_block->getWarpCount())
			{
				thread_block->finished_timing = true;
				sm->UnmapBlock(uop->getThreadBlock());

				grid->IncThreadBlocksCompletedTiming();
				if (grid->getNumThreadBlocksCompletedTiming() ==
						grid->getThreadBlockCount())
				{
					grid->WakeupContext();
					Timing::getInstance()->grid_finished = true;
					Timing::getInstance()->popGrid();
				}
			}
		}

		// Set branch pending bit to false
		uop->getFetchBuffer()->setBranchPendingBit
				(uop->getWarpPoolEntry()->getIdInWarpPool(), false);

		// Remove uop from the write buffer
		it = write_buffer.erase(it);

		// Statistics
		this->num_instructions++;
		gpu->last_complete_cycle = sm->getTiming()->getCycle();
	}
}

void BRU::Write()
{
	// Get SM object
	SM *sm = getSM();

	// Internal counter
	int instructions_processed = 0;

	// Sanity check execution buffer
	assert((int) execute_buffer.size() <= execute_buffer_size);

	// Process completed instructions
	auto it = execute_buffer.begin();
	while (it != execute_buffer.end())
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
				"std=\"bru-w\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to write buffer and get the iterator for next
		write_buffer.push_back(std::move(*it));
		it = execute_buffer.erase(it);
	}
}


void BRU::Execute()
{
	// Get SM object
	SM *sm = getSM();

	// Internal counter
	int instructions_processed = 0;

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

		// Sanity check write buffer
		assert((int) execute_buffer.size() <= execute_buffer_size);

		// Stall if the execute buffer is full
		if((int) execute_buffer.size() == execute_buffer_size)
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
		uop->execute_ready = sm->getTiming()->getCycle() + execute_latency;

		// Trace
		Timing::trace << misc::fmt("kpl.inst "
				"id=%lld "
				"sm=%d "
				"bk=%d "
				"uop_id=%lld "
				"std=\"bru-e\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to execute buffer and get the iterator for next
		execute_buffer.push_back(std::move(*it));
		it = read_buffer.erase(it);
	}
}


void BRU::Read()
{
	// Get SM object
	SM *sm = getSM();

	// Internal counter
	int instructions_processed = 0;

	// Sanity check read buffer
	if ((int)dispatch_buffer.size() > dispatch_buffer_size)
	{
		auto it = dispatch_buffer.begin();
		Uop *uop = it->get();
		std::cout<< "instruction1 op is:" << uop->getInstructionOpcode() << std::endl;
		std::cout<< "instruction1 pc is:" << uop->getInstructionPC() << std::endl;
		std::cout<< "warp id for instruction1 is:" << uop->getWarp()->getId() << std::endl;
		Uop *uop1 = (it + 1)->get();
		std::cout<< "instruction2 op is:" << uop1->getInstructionOpcode() << std::endl;
		std::cout<< "instruction2 pc is:" << uop1->getInstructionPC() << std::endl;
		std::cout<< "warp id for instruction2 is:" << uop1->getWarp()->getId() << std::endl;
	}
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
				"std=\"bru-r\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to read buffer and get the iterator for next
		read_buffer.push_back(std::move(*it));
		it = dispatch_buffer.erase(it);
	}
}


} // namespace Kepler
