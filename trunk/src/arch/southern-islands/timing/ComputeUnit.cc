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
#include "WavefrontPool.h"


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

	// Issue instructions to SIMD units
	for (auto &simd_unit : simd_units)
		IssueToExecutionUnit(fetch_buffer, simd_unit.get());

	// Issue instructions to vector memory unit
	IssueToExecutionUnit(fetch_buffer, &vector_memory_unit);

	// Issue instructions to LDS unit
	IssueToExecutionUnit(fetch_buffer, &lds_unit);

	// Update visualization states for all instructions not issued
	std::list<std::shared_ptr<Uop>>::iterator it;
	for (it = fetch_buffer->begin(); it != fetch_buffer->end(); it++)
	{
		// Get Uop
		Uop *uop = (*it).get();

		// Skip uops that have not completed fetch
		if (timing->getCycle() < uop->fetch_ready)
			continue;

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


void ComputeUnit::Fetch(WavefrontPool *wavefront_pool)
{
//	SIGpu *gpu = self->gpu;
//	SIWavefront *wavefront;
//	SIWorkItem *work_item;
//
//	int i, j;
//	int instructions_processed = 0;
//	int work_item_id;
//
//	struct si_uop_t *uop;
//	struct si_work_item_uop_t *work_item_uop;
//	struct si_wavefront_pool_entry_t *wavefront_pool_entry;
//
//	char inst_str[MAX_STRING_SIZE];
//	char inst_str_trimmed[MAX_STRING_SIZE];
//
//	assert(active_fb < self->num_wavefront_pools);
//
	// Get wavefront pool
//	WavefrontPool wavefront_pool = wavefront_pools[wavefront_pool_id];
//	WavefrontPoolEntry wavefront_pool_entry = wavefront_pool
//
//	// Iterate through wavefronts in wavefront pool
//	for (Wavefront wavefront : wavefront_pool)
//	{
//
//	}





//	for (i = 0; i < si_gpu_max_wavefronts_per_wavefront_pool; i++)
//	{
//		wavefront = self->wavefront_pools[active_fb]->
//			entries[i]->wavefront;
//
//		/* No wavefront */
//		if (!wavefront)
//			continue;
//
//		/* Sanity check wavefront */
//		assert(wavefront->wavefront_pool_entry);
//		assert(wavefront->wavefront_pool_entry ==
//			self->wavefront_pools[active_fb]->entries[i]);
//
//		/* Regardless of how many instructions have been fetched
//		 * already, this should always be checked */
//		if (wavefront->wavefront_pool_entry->ready_next_cycle)
//		{
//			/* Allow instruction to be fetched next cycle */
//			wavefront->wavefront_pool_entry->ready = 1;
//			wavefront->wavefront_pool_entry->ready_next_cycle = 0;
//			continue;
//		}
//
//		/* Only fetch a fixed number of instructions per cycle */
//		if (instructions_processed == si_gpu_fe_fetch_width)
//			continue;
//
//		/* Wavefront isn't ready (previous instruction is still
//		 * in flight) */
//		if (!wavefront->wavefront_pool_entry->ready)
//			continue;
//
//		/* If the wavefront finishes, there still may be outstanding
//		 * memory operations, so if the entry is marked finished
//		 * the wavefront must also be finished, but not vice-versa */
//		if (wavefront->wavefront_pool_entry->wavefront_finished)
//		{
//			assert(wavefront->finished);
//			continue;
//		}
//
//		/* Wavefront is finished but other wavefronts from workgroup
//		 * remain.  There may still be outstanding memory operations,
//		 * but no more instructions should be fetched. */
//		if (wavefront->finished)
//			continue;
//
//		/* Wavefront is ready but waiting on outstanding
//		 * memory instructions */
//		if (wavefront->wavefront_pool_entry->wait_for_mem)
//		{
//			if (!wavefront->wavefront_pool_entry->lgkm_cnt &&
//				!wavefront->wavefront_pool_entry->exp_cnt &&
//				!wavefront->wavefront_pool_entry->vm_cnt)
//			{
//				wavefront->wavefront_pool_entry->wait_for_mem =
//					0;
//			}
//			else
//			{
//				/* TODO Show a waiting state in visualization
//				 * tool */
//				/* XXX uop is already freed */
//				continue;
//			}
//		}
//
//		/* Wavefront is ready but waiting at barrier */
//		if (wavefront->wavefront_pool_entry->wait_for_barrier)
//		{
//			/* TODO Show a waiting state in visualization tool */
//			/* XXX uop is already freed */
//			continue;
//		}
//
//		/* Stall if fetch buffer full */
//		assert(list_count(self->fetch_buffers[active_fb]) <=
//					si_gpu_fe_fetch_buffer_size);
//		if (list_count(self->fetch_buffers[active_fb]) ==
//					si_gpu_fe_fetch_buffer_size)
//		{
//			continue;
//		}
//
//		/* Emulate instruction */
//		SIWavefrontExecute(wavefront);
//
//		wavefront_pool_entry = wavefront->wavefront_pool_entry;
//		wavefront_pool_entry->ready = 0;
//
//		/* Create uop */
//		uop = si_uop_create();
//		uop->wavefront = wavefront;
//		uop->work_group = wavefront->work_group;
//		uop->compute_unit = self;
//		uop->id_in_compute_unit = self->uop_id_counter++;
//		uop->id_in_wavefront = wavefront->uop_id_counter++;
//		uop->wavefront_pool_id = active_fb;
//		uop->vector_mem_read = wavefront->vector_mem_read;
//		uop->vector_mem_write = wavefront->vector_mem_write;
//		uop->vector_mem_atomic = wavefront->vector_mem_atomic;
//		uop->scalar_mem_read = wavefront->scalar_mem_read;
//		uop->lds_read = wavefront->lds_read;
//		uop->lds_write = wavefront->lds_write;
//		uop->wavefront_pool_entry = wavefront->wavefront_pool_entry;
//		uop->wavefront_last_inst = wavefront->finished;
//		uop->mem_wait_inst = wavefront->mem_wait;
//		uop->barrier_wait_inst = wavefront->barrier_inst;
//		uop->inst = wavefront->inst;
//		uop->cycle_created = asTiming(gpu)->cycle;
//		uop->glc = wavefront->vector_mem_glc;
//		assert(wavefront->work_group && uop->work_group);
//
//		/* Trace */
//		if (si_tracing())
//		{
//			SIInstWrapDumpBuf(wavefront->inst, inst_str, sizeof inst_str);
//			str_single_spaces(inst_str_trimmed,
//				sizeof inst_str_trimmed,
//				inst_str);
//			si_trace("si.new_inst id=%lld cu=%d ib=%d wg=%d "
//				"wf=%d uop_id=%lld stg=\"f\" asm=\"%s\"\n",
//				uop->id_in_compute_unit, self->id,
//				uop->wavefront_pool_id, uop->work_group->id,
//				wavefront->id, uop->id_in_wavefront,
//				inst_str_trimmed);
//		}
//
//		/* Update last memory accesses */
//		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
//		{
//			work_item = wavefront->work_items[work_item_id];
//			work_item_uop =
//				&uop->work_item_uop[work_item->id_in_wavefront];
//
//			/* Global memory */
//			work_item_uop->global_mem_access_addr =
//				work_item->global_mem_access_addr;
//			work_item_uop->global_mem_access_size =
//				work_item->global_mem_access_size;
//
//			/* LDS */
//			work_item_uop->lds_access_count =
//				work_item->lds_access_count;
//			for (j = 0; j < work_item->lds_access_count; j++)
//			{
//				work_item_uop->lds_access_kind[j] =
//					work_item->lds_access_type[j];
//				work_item_uop->lds_access_addr[j] =
//					work_item->lds_access_addr[j];
//				work_item_uop->lds_access_size[j] =
//					work_item->lds_access_size[j];
//			}
//		}
//
//		/* Access instruction cache. Record the time when the
//		 * instruction will have been fetched, as per the latency
//		 * of the instruction memory. */
//		uop->fetch_ready = asTiming(gpu)->cycle + si_gpu_fe_fetch_latency;
//
//		/* Insert into fetch buffer */
//		list_enqueue(self->fetch_buffers[active_fb], uop);
//
//		instructions_processed++;
//		self->inst_count++;
//	}
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
	//for (int i = 0; i < num_wavefront_pools; i++)
	//	Fetch(i);
}

}

