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

#include "BranchUnit.h"
#include "Timing.h"


namespace SI
{

int BranchUnit::width = 1;
int BranchUnit::issue_buffer_size = 1;
int BranchUnit::decode_latency = 1;
int BranchUnit::decode_buffer_size = 1;
int BranchUnit::read_latency = 1;
int BranchUnit::read_buffer_size = 1;
int BranchUnit::exec_latency = 16;
int BranchUnit::exec_buffer_size = 16;
int BranchUnit::write_latency = 1;
int BranchUnit::write_buffer_size = 1;


void BranchUnit::Run()
{
	Complete();
	Write();
	Execute();
	Read();
	Decode();
}


bool BranchUnit::isValidUop(Uop *uop) const
{
	// Get instruction
	Instruction *instruction = uop->getInstruction();
	
	// Check if the instruction is a branch instruction
	return instruction->getFormat() == Instruction::FormatSOPP &&
			instruction->getBytes()->sopp.op > 1 &&
			instruction->getBytes()->sopp.op < 10;
}


void BranchUnit::Issue(std::unique_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();
	compute_unit->num_branch_instructions++;

	// Issue it
	ExecutionUnit::Issue(std::move(uop));
}

void BranchUnit::Complete()
{
	// Get compute unit and GPU objects
	ComputeUnit *compute_unit = getComputeUnit();
	Gpu *gpu = compute_unit->getGpu();

	// Sanity check the write buffer
	assert((int) write_buffer.size() <= write_latency * width);

	// Process completed instructions
	auto it = write_buffer.begin();
	while (it != write_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->write_ready)
			break;
	
		// Record trace
		Timing::trace << misc::fmt("si.end_inst "
				"id=%lld "
				"cu=%d\n ",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex());

		// Allow next instruction to be fetched
		uop->getWavefrontPoolEntry()->ready = true;

		// Access complete, remove the uop from the queue, and get the
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

void BranchUnit::Write()
{
	// Get compute unit object
	ComputeUnit *compute_unit = getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check exec buffer
	assert((int) exec_buffer.size() <= exec_buffer_size);
	
	// Process completed instructions
	auto it = exec_buffer.begin();
	while (it != exec_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->execute_ready)
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
				"stg=\"bu-w\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to write buffer and get the iterator for the
		// next element
		write_buffer.push_back(std::move(*it));
		it = exec_buffer.erase(it);
	}
}

void BranchUnit::Execute()
{
	// Get compute unit object
	ComputeUnit *compute_unit = getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
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

		// Sanity check exec buffer
		assert((int) exec_buffer.size() <= exec_buffer_size);

		// Stall if the exec buffer is full.
		if ((int) exec_buffer.size() == exec_buffer_size)             
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


		// Update Uop exec ready cycle
		uop->execute_ready = compute_unit->getTiming()->
			getCycle() + exec_latency;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"bu-e\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to exec buffer and get the iterator for the 
		// next element
		exec_buffer.push_back(std::move(*it));
		it = read_buffer.erase(it);
	}
}

void BranchUnit::Read()
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
				"stg=\"bu-r\"\n",
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

void BranchUnit::Decode()
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
				"stg=\"bu-d\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to write buffer
		decode_buffer.push_back(std::move(*it));
		it = issue_buffer.erase(it);
	}
}


} // SI Namespace

