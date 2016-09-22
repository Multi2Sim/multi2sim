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

#include "Spu.h"
#include "SM.h"
#include "Gpu.h"
#include "Timing.h"
#include "ScoreBoard.h"


namespace Kepler
{

int SPU::width = 1;
int SPU::num_spu_lanes = 32;
int SPU::dispatch_buffer_size = SM::dispatch_latency;
int SPU::read_latency = 1;//5;
int SPU::read_buffer_size = read_latency;
int SPU::execute_latency = 1;//5;
int SPU::execute_buffer_size = execute_latency;
int SPU::write_latency = 1;//4;
int SPU::write_buffer_size = write_latency;

void SPU::Run()
{
	SPU::Complete();
	SPU::Write();
	SPU::Execute();
	SPU::Read();
}


void SPU::Dispatch(std::unique_ptr<Uop> uop)
{
	// Get sm
	SM *sm = getSM();

	// One more instruction of this kind
	sm->num_sfu_instructions++;

	// Dispatch it
	ExecutionUnit::Dispatch(std::move(uop));
}


void SPU::Complete()
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

void SPU::Write()
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
				"std=\"pspu-w\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to write buffer and get the iterator for next
		write_buffer.push_back(std::move(*it));
		it = execute_buffer.erase(it);
	}
}


void SPU::Execute()
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
				"std=\"pspu-e\"\n",
				uop->getIdInSM(),
				sm->getId(),
				uop->getWarp()->getId(),
				uop->getIdInSM());

		// Move uop to write buffer and get the iterator for next
		execute_buffer.push_back(std::move(*it));
		it = read_buffer.erase(it);
	}
}


void SPU::Read()
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
				"std=\"pspu-r\"\n",
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
