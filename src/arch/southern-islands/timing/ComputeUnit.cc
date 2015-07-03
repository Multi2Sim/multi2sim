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

#include <arch/southern-islands/disassembler/Instruction.h>
#include <arch/southern-islands/emulator/Wavefront.h>

#include "ComputeUnit.h"
#include "Timing.h"


namespace SI
{

int ComputeUnit::num_wavefront_pools = 4;
int ComputeUnit::fetch_latency = 1;
int ComputeUnit::fetch_width = 1;
int ComputeUnit::fetch_buffer_size = 10;
int ComputeUnit::issue_latency = 1;
int ComputeUnit::issue_width = 5;
int ComputeUnit::max_instructions_issued_per_type = 1;
	

ComputeUnit::ComputeUnit(int index) :
		index(index),
		scalar_unit(this),
		branch_unit(this),
		lds_unit(this),
		vector_memory_unit(this)
{
	// Create wavefront pools, and SIMD units
	wavefront_pools.reserve(num_wavefront_pools);
	fetch_buffers.reserve(num_wavefront_pools);
	simd_units.reserve(num_wavefront_pools);
	for (int i = 0; i < num_wavefront_pools; i++)
	{
		wavefront_pools[i] = misc::new_unique<WavefrontPool>(this);
		fetch_buffers[i] = misc::new_unique<FetchBuffer>(this);
		simd_units[i] = misc::new_unique<SimdUnit>(this);
	}
}


void ComputeUnit::IssueToExecutionUnit(FetchBuffer *fetch_buffer,
		ExecutionUnit *execution_unit)
{
	// Issue at most 'max_instructions_per_type'
	for (int num_issued_instructions = 0;
			num_issued_instructions < max_instructions_issued_per_type;
			num_issued_instructions++)
	{
		// Nothing if execution unit cannot absorb more instructions
		if (!execution_unit->canIssue())
			break;

		// Find oldest uop
		auto oldest_uop_iterator = fetch_buffer->end();
		for (auto it = fetch_buffer->begin(),
				e = fetch_buffer->end();
				it != e;
				++it)
		{
			// Discard uop if it is not suitable for this execution
			// unit
			Uop *uop = it->get();
			if (!execution_unit->isValidUop(uop))
				continue;

			// Skip uops that have not completed fetch
			if (timing->getCycle() < uop->fetch_ready)
				continue;

			// Save oldest uop
			if (oldest_uop_iterator == fetch_buffer->end() ||
					uop->getWavefront()->getId() <
					(*oldest_uop_iterator)->getWavefront()->getId())
				oldest_uop_iterator = it;
		}

		// Stop if no instruction found
		if (oldest_uop_iterator == fetch_buffer->end())
			break;

		// Erase from fetch buffer, issue to execution unit
		std::shared_ptr<Uop> uop = fetch_buffer->Remove(oldest_uop_iterator);
		execution_unit->Issue(uop);

		// Trace
		Timing::trace << misc::fmt("si.instruction "
				"id=%lld "
				"cu=%d "
				"wf=%d "
				"uop_id=%lld "
				"stg=\"i\"\n", 
				uop->getIdInComputeUnit(),
				index,
				uop->getWavefront()->getId(),
				uop->getIdInWavefront());
	}
}


void ComputeUnit::Issue(int fetch_buffer_id)
{
	// Get fetch buffer
	assert(misc::inRange(fetch_buffer_id, 0, num_wavefront_pools - 1));
	FetchBuffer *fetch_buffer = fetch_buffers[fetch_buffer_id].get();

	// Issue instructions to branch unit
	IssueToExecutionUnit(fetch_buffer, &branch_unit);

	// Issue instructions to scalar unit
	IssueToExecutionUnit(fetch_buffer, &scalar_unit);

#if 0
	/* SIMD unit */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate SIMD instructions */
			if (SIInstWrapGetFormat(uop->instruction) != SIInstFormatVOP2 && 
				SIInstWrapGetFormat(uop->instruction) != SIInstFormatVOP1 && 
				SIInstWrapGetFormat(uop->instruction) != SIInstFormatVOPC && 
				SIInstWrapGetFormat(uop->instruction) != SIInstFormatVOP3a && 
				SIInstWrapGetFormat(uop->instruction) != SIInstFormatVOP3b)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest SIMD instruction */
		if (oldest_uop &&
			list_count(self->simd_units[active_fb]->issue_buffer) < 
				si_gpu_simd_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->simd_units[active_fb]->issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.instruction id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->simd_inst_count++;
		}
	}

	/* Vector memory */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate memory instructions */
			if (SIInstWrapGetFormat(uop->instruction) != SIInstFormatMTBUF && 
				SIInstWrapGetFormat(uop->instruction) != SIInstFormatMUBUF)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest memory instruction */
		if (oldest_uop &&
			list_count(self->vector_mem_unit.issue_buffer) < 
				si_gpu_vector_mem_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->vector_mem_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.instruction id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->vector_mem_inst_count++;
			oldest_uop->wavefront_pool_entry->lgkm_cnt++;
		}
	}

	/* LDS */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate LDS instructions */
			if (SIInstWrapGetFormat(uop->instruction) != SIInstFormatDS)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest LDS instruction */
		if (oldest_uop &&
			list_count(self->lds_unit.issue_buffer) < 
			si_gpu_lds_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->lds_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.instruction id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->lds_inst_count++;
			oldest_uop->wavefront_pool_entry->lgkm_cnt++;
		}
	}

	/* Update visualization states for all instructions not issued */
	list_entries = list_count(self->fetch_buffers[active_fb]);
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(self->fetch_buffers[active_fb], i);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(gpu)->cycle < uop->fetch_ready)
		{
			continue;
		}

		si_trace("si.instruction id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
			uop->id_in_compute_unit, self->id, 
			uop->wavefront->id, uop->id_in_wavefront);
	}
#endif
}


void ComputeUnit::Fetch(int wavefront_pool_id)
{
}


void ComputeUnit::Run()
{
	// Return if no work groups are mapped to this compute unit
	if (!work_groups.size())
		return;
	
	// Save timing simulator
	timing = Timing::getInstance();

	// Issue buffer chosen to issue this cycle
	int active_issue_buffer = timing->getCycle() % num_wavefront_pools;
	assert(active_issue_buffer >= 0 && active_issue_buffer < num_wavefront_pools);

	// SIMDs
	for (auto &simd_unit : simd_units)
		simd_unit->Run();

	// Vector memory
	vector_memory_unit.Run();

	// LDS unit
	lds_unit.Run();

	// Scalar unit
	scalar_unit.Run();

	// Branch unit
	branch_unit.Run();

	// Issue from the active issue buffer
	Issue(active_issue_buffer);

	// Update visualization in non-active issue buffers
	/*
	FIXME
	for (i = 0; i < num_simd_units; i++)
	{
		if (i != active_issue_buffer)
		{
			SIComputeUnitUpdateFetchVisualization(self, i);
		}
	}
	*/

	// Fetch
	for (int i = 0; i < num_wavefront_pools; i++)
		Fetch(i);
}

}

