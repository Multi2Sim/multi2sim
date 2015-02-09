/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include "Emu.h"

#include "SegmentManager.h"

namespace HSA
{

SegmentManager::SegmentManager(mem::Memory* memory, unsigned size) :
		Manager(memory)
{
	this->size = size;

	if (size > 0)
	{
		// Request the memory of certain size from the main memory manager
		mem::Manager *main_manager = Emu::getInstance()->getMemoryManager();
		base_address = main_manager->Allocate(size);

		// Create a hole for the entire segment
		CreateHole(base_address, size);
	}
}


unsigned SegmentManager::Allocate(unsigned size, unsigned alignment)
{
	for (auto it = holes.lower_bound(size); it != holes.end(); it++)
	{
		if (canHoleContain(it->second, size, alignment))
		{
			debug << misc::fmt("Allocating in hole 0x%x\n",
					it->second->getAddress());
			unsigned address = AllocateIn(it->second,
					size, alignment);
			if (debug)
			{
				Dump(debug);
			}

			return address - base_address;
		}
	}

	// No enough space in the segment
	throw Error("No enough space in segment\n");
	return 0;
}


SegmentManager::~SegmentManager()
{
	if (size == 0)
		return;

	// Clear the memory occupied by the segment
	mem::Manager *main_manager = Emu::getInstance()->getMemoryManager();
	main_manager->Free(base_address);
}


void SegmentManager::Free(unsigned address)
{
	// Dump information into debug file
	debug << misc::fmt("Free pointer at 0x%x.\n", address);

	// Get the chunk to be freed
	auto it = chunks.find(address + base_address);
	if (it == chunks.end())
	{
		throw Error("Trying to free a pointer not created by Allocate "
				"or has already been freed.");
	}

	// Get the pointer size when it is allocated
	unsigned pointer_size = it->second->getSize();

	// Remove the pointer
	RemovePointer(it->second.get());

	// Add the hole
	Chunk *hole = CreateHole(address + base_address, pointer_size);

	// Merge Holes
	MergeHoles(hole);

	// Dump free result
	if (debug) Dump(debug);
}


unsigned SegmentManager::getFlatAddress(unsigned address)
{
	return address + base_address;
}

}  // namespace HSA
