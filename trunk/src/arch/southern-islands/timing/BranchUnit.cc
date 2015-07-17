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
	BranchUnit::Complete();
	BranchUnit::Write();
	BranchUnit::Execute();
	BranchUnit::Read();
	BranchUnit::Decode();
}


bool BranchUnit::isValidUop(Uop *uop) const
{
	Instruction *instruction = uop->getInstruction();
	return instruction->getFormat() == Instruction::FormatSOPP &&
			instruction->getBytes()->sopp.op > 1 &&
			instruction->getBytes()->sopp.op < 10;
}


void BranchUnit::Issue(std::shared_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();
	compute_unit->num_branch_instructions++;

	// Issue it
	ExecutionUnit::Issue(uop);
}

void BranchUnit::Complete()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	Gpu *gpu = compute_unit->getGpu();

	// Sanity check the write buffer
	assert((int) write_buffer.size() <= write_latency * width);

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
	
		// Record trace
		Timing::trace << misc::fmt("si.end_inst "
				"id=%lld "
				"cu=%d\n ",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex());

		// Allow next instruction to be fetched
		uop->getWavefrontPoolEntry()->ready = true;

		// Access complete, remove the uop from the queue
		write_buffer.erase(it);

		// Statistics
		this->inst_count++;
		gpu->last_complete_cycle = compute_unit->getTiming()->getCycle();
	}
}

void BranchUnit::Write()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
	// Sanity check exec buffer
	assert((int) exec_buffer.size() <= exec_buffer_size);
	
	// Process completed instructions
	for (auto it = exec_buffer.begin(),
			e = exec_buffer.end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Continue if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->execute_ready)
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

		// Move uop to write buffer
		write_buffer.push_back(std::move(*it));
		exec_buffer.erase(it);
	}
}

void BranchUnit::Execute()
{
	ComputeUnit *compute_unit = this->getComputeUnit();
	
	// Internal counter
	int instructions_processed = 0;
	
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
			continue;                                 
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

		// Move uop to exec buffer
		exec_buffer.push_back(std::move(*it));
		read_buffer.erase(it);
	}
}

void BranchUnit::Read()
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
				"stg=\"bu-r\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to read buffer
		read_buffer.push_back(std::move(*it));
		decode_buffer.erase(it);
	}
}

void BranchUnit::Decode()
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
				"stg=\"bu-d\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to write buffer
		decode_buffer.push_back(std::move(*it));
		issue_buffer.erase(it);
	}
}


} // SI Namespace

