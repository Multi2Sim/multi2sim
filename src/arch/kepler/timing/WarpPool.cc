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

#include "SM.h"
#include "WarpPool.h"


namespace Kepler
{


void WarpPoolEntry::Clear()
{
	// Reset the warp flags
	warp = nullptr;
	uop = nullptr;
	valid = false;
	ready = false;
	ready_next_cycle = false;
	warp_finished = false;
}


WarpPool::WarpPool(int id, SM *sm) :
		id(id),
		sm(sm)
{
	// Populate warp_pool_entries vector
	warp_pool_entries.resize(SM::max_warps_per_warp_pool);
	for (int i = 0; i < SM::max_warps_per_warp_pool; i++)
		warp_pool_entries[i] = misc::new_unique<WarpPoolEntry>(i,this);
}


void WarpPool::MapWarp(Warp *warp)
{
	// Find first available warp pool entry
	int index = FindFirstAvailableEntryIndex();
	assert(index >= 0);

	// Set entry pointer point to an entry in the warp pool
	WarpPoolEntry *warp_pool_entry =
			warp_pool_entries[index].get();

	// Set initial state for the entry
	warp_pool_entry->valid = true;
	warp_pool_entry->ready = true;
	warp_pool_entry->setWarp(warp);

	// Set warp pool entry to the associated warp
	warp->setWarpPoolEntry(warp_pool_entry);
	warp->setWarpPoolEntryIndex(index);

	//Debug
	//std::cout<<"Function::MapWarp"<<std::endl;
	//std::cout<<"warp id is"<<warp_pool_entries[index].get()->getWarp()->getId()
			//<<std::endl;


	// Increment the number of warps associated with the warp pool
	num_warps++;

	// Debug
	//std::cout<<"num of warps mapped" << num_warps << std::endl;
}


void WarpPool::UnmapWarp(Warp *warp)
{
	// Get the warp pool entry associated with the warp
	WarpPoolEntry *warp_pool_entry = warp->getWarpPoolEntry();

	// Get the ID of the entry in the warp pool
	int id = warp_pool_entry->getIdInWarpPool();

	//Debug
	//std::cout<<"Function::UnmMapWarp"<<std::endl;
	//std::cout<<"warp id is"<<warp_pool_entries[id]->getWarp()->getId()<<std::endl;
	//std::cout<<"warp index is" <<warp->getId()<<std::endl;

	// Check
	assert(warp_pool_entries[id]->getWarp());
	assert(warp_pool_entries[id]->valid);
	assert(warp_pool_entries[id]->getWarp()->getId() == warp->getId());

	// Clear warp pool entry
	warp_pool_entries[id]->Clear();

	// Adjust the number of warps mapped to the warp pool
	num_warps--;
}

int WarpPool::FindFirstAvailableEntryIndex()
{
	for (int i = 0; i < SM::max_warps_per_warp_pool; i++)
	{
		if (!warp_pool_entries[i]->valid)
			return i;
	}
	return -1;
}


} // Kepler namespace
