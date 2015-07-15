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
#include <arch/southern-islands/emulator/WorkItem.h>

#include "LdsUnit.h"
#include "ComputeUnit.h"
#include "Timing.h"


namespace SI
{
int LdsUnit::width = 1;
int LdsUnit::issue_buffer_size = 4;
int LdsUnit::decode_latency = 1;
int LdsUnit::decode_buffer_size = 1;
int LdsUnit::read_latency = 1;
int LdsUnit::read_buffer_size = 1;
int LdsUnit::write_latency = 1;
int LdsUnit::write_buffer_size = 1;
int LdsUnit::max_in_flight_mem_accesses = 32;


void LdsUnit::Run()
{
	LdsUnit::Complete();
	LdsUnit::Write();
	LdsUnit::Mem();
	LdsUnit::Read();
	LdsUnit::Decode();
}


bool LdsUnit::isValidUop(Uop *uop) const
{
	// Get instruction
	Instruction *instruction = uop->getInstruction();

	// Determine if lds instruction
	if (instruction->getFormat() != Instruction::FormatDS)
		return false;

	return true;
}


void LdsUnit::Issue(std::shared_ptr<Uop> uop)
{
	// Get compute unit
	ComputeUnit *compute_unit = getComputeUnit();

	// One more instruction of this kind
	compute_unit->num_lds_instructions++;
	uop->getWavefrontPoolEntry()->lgkm_cnt++;

	// Issue it
	ExecutionUnit::Issue(uop);
}

void LdsUnit::Complete()
{
	// Get useful objects
	ComputeUnit *compute_unit = this->getComputeUnit();

	// Sanity check write buffer
	assert(int(write_buffer.size()) <= write_buffer_size);

	// Process completed instructions
	for (auto it = write_buffer.begin(),
			e = write_buffer.end();
			it != e;
			++it)
	{
		// Get Uop
		Uop *uop = it->get();

		// Uop is not ready yet
		if (compute_unit->getTiming()->getCycle() < uop->write_ready)
			continue;

	// Access complete, remove the uop from the queue
	write_buffer.erase(it);

	// Statistics
	assert(uop->getWavefrontPoolEntry()->lgkm_cnt > 0);
	uop->getWavefrontPoolEntry()->lgkm_cnt--;

	// Trace
	Timing::trace << misc::fmt("si.end_inst "
			           "id=%lld "
			           "cu=%d\n",
				    uop->getIdInComputeUnit(),
		                    compute_unit->getIndex());

	// Free uop
	// SI_uop_free(uop)

	// Statistics
	inst_count++;
	compute_unit->getGpu()->last_complete_cycle = compute_unit->
			getTiming()->getCycle();
	}
}

void LdsUnit::Write()
{
	// Get useful objects
	ComputeUnit *compute_unit = this->getComputeUnit();

	// Internal counter
	int instructions_processed;

	// Sanity check write buffer
	assert(int(mem_buffer.size()) <= max_in_flight_mem_accesses);

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
		if (uop->lds_witness)
			continue;

		// Stall if the width has been reached
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

		// Sanity check the write buffer
		assert(int(write_buffer.size()) <= write_buffer_size);

		// Stop if the write buffer is full
		if (int(write_buffer.size()) == write_buffer_size)
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
		}

		// Access complete, update write ready
		uop->write_ready = compute_unit->getTiming()->getCycle() +
				write_latency;

		// Remove the uop from the queue
		mem_buffer.erase(it);
		write_buffer.push_back(std::move(*it));

		// Update wavefront pool entry
		uop->getWavefrontPoolEntry()->ready_next_cycle = true;

		// One more instruction processed
		instructions_processed++;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"lds-w\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
	}
}

void LdsUnit::Mem()
{
	// Get useful objects
	ComputeUnit *compute_unit = this->getComputeUnit();

	// Internal counter
	int instructions_processed;

	// Sanity check write buffer
	assert(int(read_buffer.size()) <= read_buffer_size);

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

		// Uop is not ready yet
		if (compute_unit->getTiming()->getCycle() < uop->read_ready)
			continue;

		// Stall if the width has been reached
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

		// Sanity check uop
		assert(uop->lds_read || uop->lds_write);

		// Sanity check the mem buffer
		assert(int(mem_buffer.size()) <= max_in_flight_mem_accesses);

		// Stop if the memory buffer is full
		if (int(mem_buffer.size()) == max_in_flight_mem_accesses)
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

		// Access local memory
		for (auto it = uop->getWavefront()->getWorkItemsBegin(),
				e = uop->getWavefront()->getWorkItemsEnd();
				it != e;
				++it)
		{
			// Get work item
			WorkItem *work_item = it->get();

			// Get uop work item info
			Uop::WorkItemInfo *work_item_info;
			work_item_info =
				&uop->work_item_info_list[work_item->getIdInWavefront()];

			// Access type
			mem::Module::AccessType access_type;

			for (int i = 0; i < work_item_info->lds_access_count; i++)
			{
				if (work_item->lds_access[i].type ==
						WorkItem::MemoryAccessType::MemoryAccessRead)
				{
					access_type = mem::Module::AccessType::AccessLoad;
				}

				if (work_item->lds_access[i].type ==
						WorkItem::MemoryAccessType::MemoryAccessWrite)
				{
					access_type = mem::Module::AccessType::AccessStore;
				}

				else
				{
					throw misc::Panic("Invalid lds access");
				}

				// Start access
				compute_unit->getLdsModule()->Access(
						access_type,
						work_item_info->lds_access[i].addr,
						&uop->lds_witness);
			}
		}

		// Move uop to the mem buffer
		read_buffer.erase(it);
		mem_buffer.push_back(std::move(*it));

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"lds-m\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
	}
}

void LdsUnit::Read()
{
	// Get useful objects
	ComputeUnit *compute_unit = this->getComputeUnit();

	// Internal counter
	int instructions_processed;

	// Sanity check write buffer
	assert(int(decode_buffer.size()) <= decode_buffer_size);

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

		// Stall if the width has been reached
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

		// Stop if the read buffer is full
		if (int(read_buffer.size()) == read_buffer_size)
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

		// Uop is not ready yet
		if (compute_unit->getTiming()->getCycle() < uop->decode_ready)
			continue;

		// Update uop
		uop->read_ready = compute_unit->getTiming()->getCycle() +
				read_latency;

		// Move uop to read buffer
		decode_buffer.erase(it);
		read_buffer.push_back(std::move(*it));

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"lds-r\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
	}
}

void LdsUnit::Decode()
{
	// Get useful objects
	ComputeUnit *compute_unit = this->getComputeUnit();

	// Internal counter
	int instructions_processed;

	// Sanity check write buffer
	assert(int(issue_buffer.size()) <= issue_buffer_size);

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

		// Uop is not ready yet
		if (compute_unit->getTiming()->getCycle() < uop->issue_ready)
			continue;

		// Stall if the width has been reached
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
		assert(int(decode_buffer.size()) <= decode_buffer_size);

		// Stop if the decode buffer is full
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
			continue;
		}

		// Update uop
		uop->decode_ready = compute_unit->getTiming()->getCycle() +
				decode_latency;

		// Mode uop to decode buffer
		issue_buffer.erase(it);
		decode_buffer.push_back(std::move(*it));

		// if (si_spatial_report_active)
		//	SIComputeUnitReportNewLDSInst(lds->compute_unit);

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"lds-d\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
	}
}


} // namespace SI

