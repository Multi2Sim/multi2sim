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

#include "ComputeUnit.h"
#include "ScalarUnit.h"
#include "Gpu.h"
#include "Timing.h"


namespace SI
{

int ScalarUnit::width = 1;
int ScalarUnit::issue_buffer_size = 4;
int ScalarUnit::decode_latency = 1;
int ScalarUnit::decode_buffer_size = 1;
int ScalarUnit::read_latency = 1;
int ScalarUnit::read_buffer_size = 1;
int ScalarUnit::exec_latency = 4;
int ScalarUnit::exec_buffer_size = 32;
int ScalarUnit::write_latency = 1;
int ScalarUnit::write_buffer_size = 1;


void ScalarUnit::Run()
{
	// Run pipeline stages in reverse order
	ScalarUnit::Complete();
	ScalarUnit::Write();
	ScalarUnit::Execute();
	ScalarUnit::Read();
	ScalarUnit::Decode();
}


bool ScalarUnit::isValidUop(Uop *uop) const
{
	Instruction *instruction = uop->getInstruction();
	if (instruction->getFormat() != Instruction::FormatSOPP &&
			instruction->getFormat() != Instruction::FormatSOP1 &&
			instruction->getFormat() != Instruction::FormatSOP2 &&
			instruction->getFormat() != Instruction::FormatSOPC &&
			instruction->getFormat() != Instruction::FormatSOPK &&
			instruction->getFormat() != Instruction::FormatSMRD)
		return false;
	
	if (instruction->getFormat() == Instruction::FormatSOPP && 
			instruction->getBytes()->sopp.op > 1 && 
			instruction->getBytes()->sopp.op < 10)
		return false;

	return true;
}


void ScalarUnit::Issue(std::unique_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();
	if (uop->getInstruction()->getFormat() == Instruction::FormatSMRD)
	{
		// The wavefront will be ready next cycle
		uop->getWavefrontPoolEntry()->ready_next_cycle = true;
		
		// Keep track of statistics
		compute_unit->num_scalar_memory_instructions++;
		uop->getWavefrontPoolEntry()->lgkm_cnt++;
	}
	else
	{
		// Scalar ALU instructions must complete before the next
		// instruction can be fetched.
		compute_unit->num_scalar_alu_instructions++;
	}

	// Issue it
	ExecutionUnit::Issue(std::move(uop));
}

void ScalarUnit::Complete()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();
	Gpu *gpu = compute_unit->getGpu();

	// Initialize iterator
	auto it = write_buffer.begin();

	// Process completed instructions
	while (it != write_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// Get work group
		WorkGroup *work_group = uop->getWorkGroup();

		// Break if uop is not ready
		if (compute_unit->getTiming()->getCycle() < uop->write_ready)
			break;

		// If this is the last instruction and there are outstanding
		// memory operations, wait for them to complete
		if (uop->wavefront_last_instruction &&
			(uop->getWavefrontPoolEntry()->lgkm_cnt ||
			 uop->getWavefrontPoolEntry()->vm_cnt ||
			 uop->getWavefrontPoolEntry()->exp_cnt))
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

		// Decrement the outstanding memory access count
		if (uop->scalar_memory_read)
		{
			assert(uop->getWavefrontPoolEntry()->lgkm_cnt > 0);
			uop->getWavefrontPoolEntry()->lgkm_cnt--;
		}

		if (!uop->scalar_memory_read)
		{
			// Update wavefront pool entry
			uop->getWavefrontPoolEntry()->ready = true;
		}

		// Check for "wait" instruction
		// If a wait instruction was executed and there are outstanding
		// memory accesses, set the wavefront to waiting
		if (uop->memory_wait)
		{
			uop->getWavefrontPoolEntry()->mem_wait = true;
		}

		// Check for "barrier" instruction
		if (uop->at_barrier)
		{
			// Set a flag to wait until all wavefronts have
			// reached the barrier
			assert(!uop->getWavefrontPoolEntry()->wait_for_barrier);
			uop->getWavefrontPoolEntry()->wait_for_barrier = true;

			// Check if all wavefronts have reached the barrier
			bool barrier_complete = true;
			for (auto it = work_group->getWavefrontsBegin(),
					e = work_group->getWavefrontsEnd();
					it != e;
					++it)
			{
				Wavefront *wavefront = it->get();
				assert(wavefront->getWavefrontPoolEntry());
				if (!wavefront->getWavefrontPoolEntry()->
						wait_for_barrier)
					barrier_complete = false;
			}

			// If all wavefronts have reached the barrier,
			// clear their flags
			if (barrier_complete)
			{
				for (auto it = work_group->getWavefrontsBegin(),
						e = work_group->getWavefrontsEnd();
						it != e;
						++it)
				{
					Wavefront *wavefront = it->get();
					assert(wavefront->getWavefrontPoolEntry()->
							wait_for_barrier);
					wavefront->getWavefrontPoolEntry()->
							wait_for_barrier = false;
				}

				Timing::pipeline_debug << misc::fmt(
						"wg=%d id_in_wf=%lld "
						"Barrier:Finished (last wf=%d)\n",
						work_group->getId(),
						uop->getIdInWavefront(),
						uop->getWavefront()->getId());
			}
		}

		if (uop->wavefront_last_instruction)
		{
			// If the Uop completes the wavefront, set a bit
			// so that the hardware wont try to fetch any
			// more instructions for it
			uop->getWavefrontPoolEntry()->wavefront_finished = true;
			work_group->incWavefrontsCompletedTiming();

			// Set the work group as finished with timing simulation
			// if all the wavefonts in the work group are complete
			if (work_group->getWavefrontsCompletedTiming() ==
					work_group->getWavefrontsInWorkgroup())
				work_group->finished_timing = true;

			// Check if wavefront finishes a work-group
			assert(work_group->getWavefrontsCompletedTiming() <=
					work_group->getWavefrontsInWorkgroup());

			// Check if the work group is finished. If so, unmap
			// the work group
			if (work_group->finished_timing &&
					work_group->inflight_instructions == 1)
			{
				Timing::pipeline_debug << misc::fmt(
						"wg=%d "
						"WGFinished\n",
						work_group->getId());
				compute_unit->UnmapWorkGroup(uop->getWorkGroup());
			}
		}

		// Trace
		Timing::trace << misc::fmt("si.end_inst "
				"id=%lld "
				"cu=%d\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex());

		// Access complete, remove the uop from the queue
		it = write_buffer.erase(it);
		assert(uop->getWorkGroup()->inflight_instructions > 0);
		uop->getWorkGroup()->inflight_instructions--;

		// Statistics
		num_instructions++;
		gpu->last_complete_cycle = compute_unit->getTiming()->getCycle();
	}
}


void ScalarUnit::Write()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();

	// Internal counter
	int instructions_processed = 0;

	// Sanity check exec buffer
	assert((int) exec_buffer.size() <= exec_buffer_size);

	// Initialize iterator
	auto it = exec_buffer.begin();

	// Process completed instructions
	while (it != exec_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		if (uop->scalar_memory_read)
		{
			// Check if access is complete
			if (uop->global_memory_witness)
			{
				break;
			}

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

			// Stall if there is not room in the exec buffer
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
					"stg=\"su-w\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());

			// Move uop to write buffer and get the iterator for
			// the next element
			write_buffer.push_back(std::move(*it));
			it = exec_buffer.erase(it);
		}

		// ALU instruction
		else
		{
			// Uop is not ready yet
			if (compute_unit->getTiming()->getCycle() < uop->
					execute_ready)
						break;

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
				break;
			}

			// Sanity check write buffer
			assert((int) write_buffer.size() <= write_buffer_size);

			// Stall if the write buffer is full
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

			// Sanity check write buffer
			assert((int) write_buffer.size() <= write_buffer_size);

			// Stall if the write buffer is full
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

			// Update uop write ready
			uop->write_ready = compute_unit->getTiming()->
					getCycle() + write_latency;

			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"su-w\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());

			// Move uop to write buffer and get the iterator for
			// the next element
			write_buffer.push_back(std::move(*it));
			it = exec_buffer.erase(it);
		}
	}
}

void ScalarUnit::Execute()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();

	// Internal variables
	int instructions_processed = 0;

	// Sanity check read buffer
	assert((int) read_buffer.size() <= read_buffer_size);

	// Initialize iterator
	auto it = read_buffer.begin();

	// Process completed instructions
	while (it != read_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Uop is not ready yet
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
		assert(int(exec_buffer.size()) <= exec_buffer_size);

		// Stall if there is no room in the exec buffer
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

		// Scalar mem read
		if (uop->scalar_memory_read)
		{
			// Access global memory
			uop->global_memory_witness--;

			// FIXME Get rid of dependence on wavefront here
			uop->global_memory_access_address = uop->getWavefront()->
					getScalarWorkItem()->global_memory_access_address;

			// Translate virtual address to physical address
			unsigned phys_addr = compute_unit->getGpu()->
					getMmu()->TranslateVirtualAddress(
							uop->getWorkGroup()->
							getNDRange()->
							address_space,
						uop->global_memory_access_address);

			// Submit the access
			compute_unit->scalar_cache->Access(
					mem::Module::AccessType::AccessLoad,
					phys_addr, &uop->global_memory_witness);

			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"su-m\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());

			// Move uop to the execution buffer and get the
			// iterator for the next element
			exec_buffer.push_back(std::move(*it));
			it = read_buffer.erase(it);
		}

		// ALU instruction
		else
		{
			uop->execute_ready = compute_unit->getTiming()->
					getCycle() + exec_latency;

			// Trace
			Timing::trace << misc::fmt("si.inst "
					"id=%lld "
					"cu=%d "
					"wf=%d "
					"uop_id=%lld "
					"stg=\"su-e\"\n",
					uop->getIdInComputeUnit(),
					compute_unit->getIndex(),
					uop->getWavefront()->getId(),
					uop->getIdInWavefront());

			// Move uop to the execution buffer and get the
			// iterator for the next element
			exec_buffer.push_back(std::move(*it));
			it = read_buffer.erase(it);
		}
	}
}

void ScalarUnit::Read()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();

	// Internal variables
	int instructions_processed = 0;

	// Sanity check decode buffer
	assert((int) decode_buffer.size() <= decode_buffer_size);

	// Initialize iterator
	auto it = decode_buffer.begin();

	// Process completed instructions
	while (it != decode_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Uop is not ready yet
		if (compute_unit->getTiming()->getCycle() < uop->decode_ready)
			break;

		// Stall if the decode width has been reached
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

		// Sanity check
		assert((int) read_buffer.size() <= read_buffer_size);

		// Stall if read buffer is full
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

		// Update uop read ready
		uop->read_ready = compute_unit->getTiming()->getCycle() +
				read_latency;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"su-r\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to the read buffer and get the iterator to the
		// next element
		read_buffer.push_back(std::move(*it));
		it = decode_buffer.erase(it);
	}
}

void ScalarUnit::Decode()
{
	// Get useful objects
	ComputeUnit *compute_unit = getComputeUnit();

	// Internal variables
	int instructions_processed = 0;

	// Sanity check decode buffer
	assert((int) issue_buffer.size() <= issue_buffer_size);

	// Initialize iterator
	auto it = issue_buffer.begin();

	// Process completed instructions
	while (it != issue_buffer.end())
	{
		// Get Uop
		Uop *uop = it->get();

		// One more instruction processed
		instructions_processed++;

		// Uop is not ready yet
		if (compute_unit->getTiming()->getCycle() < uop->issue_ready)
			break;

		// Stall if the issue width has been reached
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

		// Sanity check
		assert((int) decode_buffer.size() <= decode_buffer_size);

		// Stall if decode buffer is full
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

		// Update uop decode ready
		uop->decode_ready = compute_unit->getTiming()->getCycle() +
				decode_latency;

		// Trace
		Timing::trace << misc::fmt("si.inst "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"su-d\"\n",
				uop->getIdInComputeUnit(),
				compute_unit->getIndex(),
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());

		// Move uop to the decode buffer and get the iterator
		// to the next element
		decode_buffer.push_back(std::move(*it));
		it = issue_buffer.erase(it);
	}
}


} // namespace SI

