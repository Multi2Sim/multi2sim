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

#include "ComputeUnit.h"
#include "VectorMemoryUnit.h"
#include "WavefrontPool.h"


namespace SI
{


void WavefrontPoolEntry::Clear()
{
	// Make sure all the counters have been reset to zero
	assert(!vm_cnt);
	assert(!exp_cnt);
	assert(!lgkm_cnt);
	assert(!mem_wait);
	assert(!wait_for_barrier);

	// Reset the wavefront flags
	wavefront = nullptr;
	uop = nullptr;
	valid = false;
	ready = false;
	ready_next_cycle = false;
	wavefront_finished = false;
}


WavefrontPool::WavefrontPool(int id, ComputeUnit *compute_unit) :
		id(id),
		compute_unit(compute_unit)
{
	// Populate wavefront_pool_entries vector
	wavefront_pool_entries.resize(
			ComputeUnit::max_wavefronts_per_wavefront_pool);
	for (int i = 0; i < ComputeUnit::max_wavefronts_per_wavefront_pool; i++)
		wavefront_pool_entries[i] = 
				misc::new_unique<WavefrontPoolEntry>(i, this);	
}


void WavefrontPool::MapWavefronts(WorkGroup *work_group)
{
	// Determine starting ID for wavefronts in the instruction buffer
	int wg_id_in_wfp = work_group->getIdInComputeUnit() /
			ComputeUnit::num_wavefront_pools;
	int first_entry = wg_id_in_wfp * work_group->getWavefrontsInWorkgroup();

	// Initialize entry index within wavefront pool
	int entry_index = 0;

	// Assign wavefronts to the wavefront pool
	for (auto it = work_group->getWavefrontsBegin(), 
			e = work_group->getWavefrontsEnd();
			it != e;
			++it)
	{
		// Get the wavefront object
		Wavefront *wavefront = it->get();

		// Set entry pointer to an entry in the wavefront pool
		WavefrontPoolEntry *wavefront_pool_entry = 
			wavefront_pool_entries[first_entry + entry_index].get();

		// Make sure the entry was set and that it is not yet valid.
		// Having the valid field set would indicate that it was 
		// already assigned to another wavefront
		assert(wavefront_pool_entry);
		assert(!wavefront_pool_entry->valid);

		// Set initial state
		wavefront_pool_entry->valid = true;
		wavefront_pool_entry->ready = true;
		wavefront_pool_entry->setWavefront(wavefront);
		wavefront->setWavefrontPoolEntry(wavefront_pool_entry);
		
		// Increment the number of wavefronts associated with the 
		// wavefront pool
		num_wavefronts++;

		// Increment the entry index
		entry_index++;
	}
}

void WavefrontPool::UnmapWavefronts(WorkGroup *work_group)
{
	// Reset mapped wavefronts
	assert(num_wavefronts >= (int) work_group->getWavefrontsInWorkgroup());

	// Remove wavefronts from the wavefront pool
	for (auto it = work_group->getWavefrontsBegin(), 
			e = work_group->getWavefrontsEnd();
			it != e;
			++it)
	{
		// Get the wavefront object and associated wavefront pool entry
		Wavefront *wavefront = it->get();
		WavefrontPoolEntry *wavefront_pool_entry = 
				wavefront->getWavefrontPoolEntry();
		
		// Get the ID of the entry in the wavefront pool
		int wf_id_in_wfp = wavefront_pool_entry->getIdInWavefrontPool();

		// Make sure the wavefront in the entry has been properly set
		assert(wavefront_pool_entries[wf_id_in_wfp]->getWavefront());
		assert(wavefront_pool_entries[wf_id_in_wfp]->valid);
		assert(wavefront_pool_entries[wf_id_in_wfp]->
				getWavefront()->getId() == wavefront->getId());

		// Clear wavefront pool entry
		wavefront_pool_entries[wf_id_in_wfp]->Clear();
	}
	
	// Adjust the number of wavefronts mapped to the wavefront pool
	num_wavefronts -= work_group->getWavefrontsInWorkgroup();
}


} // SI namespace

