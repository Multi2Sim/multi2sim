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

#include <arch/southern-islands/emulator/WorkGroup.h>
#include <arch/southern-islands/emulator/Wavefront.h>

#include "SimdUnit.h"
#include "ComputeUnit.h"
#include "Gpu.h"
#include "Timing.h"


namespace SI
{

int SimdUnit::width = 1;
int SimdUnit::num_simd_lanes = 16;
int SimdUnit::issue_buffer_size = 1;
int SimdUnit::decode_latency = 1;
int SimdUnit::decode_buffer_size = 1;
int SimdUnit::read_exec_write_latency = 8;
int SimdUnit::exec_buffer_size = 2;
int SimdUnit::read_exec_write_buffer_size = 2;


void SimdUnit::Run()
{
	SimdUnit::Complete();
	SimdUnit::Execute();
	SimdUnit::Decode();
}

bool SimdUnit::isValidUop(Uop *uop) const
{
	// Get instruction
	Instruction *instruction = uop->getInstruction();

	// Determine if simd instruction
	if (instruction->getFormat() != Instruction::FormatVOP2 &&
			instruction->getFormat() != Instruction::FormatVOP1 &&
			instruction->getFormat() != Instruction::FormatVOPC &&
			instruction->getFormat() != Instruction::FormatVOP3a &&
			instruction->getFormat() != Instruction::FormatVOP3b)
		return false;

	return true;
}

void SimdUnit::Issue(std::unique_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();
	compute_unit->num_simd_instructions++;

	// Issue it
	ExecutionUnit::Issue(std::move(uop));
}


void SimdUnit::Complete()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();
	Gpu *gpu = compute_unit->getGpu();

	// Sanity check exec buffer
	assert(int(exec_buffer.size()) <= exec_buffer_size);

	// Initialize iterator
	auto it = exec_buffer.begin();

	// Process completed instructions
	while (it != exec_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// Checks
		assert(uop);

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->execute_ready)
			break;

		// Trace
		Timing::trace << misc::fmt("si.end_inst "
				"id=%lld "
				"cu=%d\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex());

		// Statistics
		num_instructions++;
		gpu->last_complete_cycle = compute_unit->getTiming()->getCycle();

		// Remove uop from the exec buffer and get the iterator to the
		// next element
		it = exec_buffer.erase(it);
		assert(uop->getWorkGroup()
				->inflight_instructions > 0);
		uop->getWorkGroup()->
				inflight_instructions--;
	}

}


void SimdUnit::Execute()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();

	// Sanity check decode buffer
	assert(int(decode_buffer.size()) <= decode_buffer_size);

	// Internal counter
	int instructions_processed = 0;

	// Initialize iterator
	auto it = decode_buffer.begin();

	// Process completed instructions
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

		// Sanity check exec buffer
		assert(int(exec_buffer.size()) <= exec_buffer_size);

		// Stall if there is not room in the exec buffer
		if (int(exec_buffer.size()) == exec_buffer_size)
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

		// Includes time for pipelined read-exec-write of all
		// subwavefronts
		uop->execute_ready = compute_unit->getTiming()->getCycle() +
				read_exec_write_latency;

		// Update wavefront pool entry
		uop->getWavefrontPoolEntry()->ready_next_cycle = true;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"simd-e\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to exec buffer and get the iterator for
		// the next element
		exec_buffer.push_back(std::move(*it));
		it = decode_buffer.erase(it);
	}

}


void SimdUnit::Decode()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();

	// Sanity check issue buffer
	assert(int(issue_buffer.size()) <= issue_buffer_size);

	// Internal counter
	int instructions_processed = 0;

	// Initialize iterator
	auto it = issue_buffer.begin();

	// Process completed instructions
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

		// Sanity check decode buffer
		assert(int(decode_buffer.size()) <= decode_buffer_size);

		// Stall if there is not room in the decode buffer
		if (int(decode_buffer.size()) == decode_buffer_size)
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

		// Update uop
		uop->decode_ready = compute_unit->getTiming()->getCycle() +
				decode_latency;

		//if (si_spatial_report_active)
		//	SIComputeUnitReportNewALUInst(simd->compute_unit);

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"simd-d\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to decode buffer and get the iterator for
		// the next element
		decode_buffer.push_back(std::move(*it));
		it = issue_buffer.erase(it);
	}

}

} // namespace SI

