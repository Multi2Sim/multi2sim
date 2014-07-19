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

#include <cmath>

#include <lib/cpp/String.h>

#include "Manager.h"

namespace mem
{

unsigned Manager::align_size = 8;

Manager::Manager(Memory *memory)
{
	this->memory = memory;
}


unsigned Manager::Allocate(unsigned size)
{
	// If requested size is larger than a page, allocate whole pages for it
	if (size > Memory::PageSize)
	{
		return AllocateLarge(size);
	}

	// Traverse all holes to find an available slot
	for (auto it = holes.lower_bound(size); it != holes.end(); it++)
	{
		if (canHoleContain(it->second, size))
		{
			unsigned addr = it->second->getAddress();
			AllocateIn(it->second, size);
			return addr;
		}
	}
	
	// No available space, need a new page
	Chunk *hole = requestOnePage();
	unsigned addr = hole->getAddress();
	AllocateIn(hole, size);

	return addr;
}


unsigned Manager::toAlignedSize(unsigned size)
{
	return ceil((double)size / align_size) * align_size;
}


bool Manager::canHoleContain(Chunk *hole, unsigned size)
{
	return hole->getSize() >= toAlignedSize(size);
}


unsigned Manager::AllocateLarge(unsigned size)
{

	unsigned int addr = map_base_address;
	unsigned int page_aligned_size = 
			ceil((double)size / Memory::PageSize) 
			* Memory::PageSize;

	// Find a good place to allocate a new page
	addr = memory->MapSpaceDown(addr, page_aligned_size);
	if (addr == (unsigned)-1)
		throw misc::Error("Guest program out of memory.");

	// Use map to allocate a new page
	memory->Map(addr, size, 0x06);

	// Allocate the chunk
	createPointer(addr, page_aligned_size);

	return addr;
}


void Manager::AllocateIn(Chunk *hole, unsigned int size)
{
	// Assert the hole can contain the size
	if (!canHoleContain(hole, size))
		throw misc::Panic("Memory hole cannot contain size");

	// Remove the hole  
	unsigned addr = hole->getAddress();
	unsigned hole_size = hole->getSize();
	unsigned aligned_size = toAlignedSize(size);
	removeHole(hole);

	// Create a new pointer
	createPointer(addr, aligned_size);

	// If required, create another hole
	if (hole_size > aligned_size)
	{
		createHole(addr + aligned_size, hole_size - aligned_size);
	}
}


void Manager::Free(unsigned address)
{
	// Get the chunk to be freed
	auto it = chunks.find(address);
	if (it == chunks.end())
	{
		throw misc::Panic("Trying to free a pointer not created by malloc.");
	}

	// Remove the pointer
	unsigned size = it->second->getSize();
	if (size > Memory::PageSize)
	{
		FreeLarge(address);
		return;
	}

	removePointer(it->second.get());

	// Add the hole
	Chunk *hole = createHole(address, size);

	// Merge Holes
	MergeHoles(hole);
}


void Manager::FreeLarge(unsigned address)
{

	auto it = chunks.find(address);
	if (it == chunks.end())
	{
		throw misc::Panic("Trying to free a pointer not created by malloc");
	}

	// Remove the page
	unsigned num_pages = it->second->getSize() / Memory::PageSize;
	for (unsigned i = 0; i < num_pages; i++)
	{
		deallocatePage(address);
		address += Memory::PageSize;
	}


	// Remove chunk
	removePointer(it->second.get());

}


void Manager::MergeHoles(Chunk *hole)
{
	// Assert the chunk passed in as argument is a hole
	if (hole->IsAllocated())
		throw misc::Panic("Trying to merge around a allocated pointer");

	// Get hole address and size
	unsigned addr = hole->getAddress();
	unsigned merged_addr = addr;
	
	// Get the iterator
	auto it = chunks.lower_bound(addr);

	// Try to merge forward
	it++;
	if (it!=chunks.end() && 
			isInSamePage(addr, it->second->getAddress()) &&
			(!it->second->IsAllocated()))	
	{
		Merge2Holes(hole, it->second.get());
	}

	// Try to merge back
	it = chunks.lower_bound(addr);
	if (it != chunks.begin())
	{
		Chunk *chunk1 = it->second.get();
		it--;
		Chunk *chunk2 = it->second.get();
		if (isInSamePage(addr, chunk2->getAddress()) &&
				(!chunk2->IsAllocated()))
		{
			Merge2Holes(chunk2, chunk1);
			merged_addr = chunk2->getAddress();
		}
	}



	// Check if the page is free
	it = chunks.lower_bound(merged_addr);
	if (it->second->getSize() == Memory::PageSize)	
	{
		deallocatePage(merged_addr);
		removeHole(it->second.get());
	}
}


void Manager::Merge2Holes(Chunk *hole1, Chunk *hole2)
{
	// Assert hole1 and hole2 are not allocated and they are consecutive
	if (hole1->IsAllocated())
	{
		throw misc::Panic("Hole 1 is not a hole.");
	}
	if (hole2->IsAllocated())
	{
		throw misc::Panic("Hole 2 is not a hole.");
	}
	if (hole1->getAddress() + hole1->getSize() != hole2->getAddress())
	{
		throw misc::Panic("Hole1 and hole2 are not adjacent.");
	}
	
	// Get information
	unsigned size1 = hole1->getSize();
	unsigned size2 = hole2->getSize();

	// Erase the second hole
	removeHole(hole2);

	// Enlarge first hole
	hole1->setSize(size1 + size2);
}


bool Manager::isInSamePage(unsigned addr1, unsigned addr2)
{
	unsigned pageIndex1 = addr1 / Memory::PageSize;
	unsigned pageIndex2 = addr2 / Memory::PageSize;
	return pageIndex1 == pageIndex2;
}


void Manager::deallocatePage(unsigned addr)
{
	// Deallocate page
	memory->Unmap(addr, Memory::PageSize);
}


Manager::Chunk *Manager::requestOnePage()
{
	unsigned int addr = map_base_address;

	// Find a good place to allocate a new page
	addr = memory->MapSpaceDown(addr, Memory::PageSize);
	if (addr == (unsigned)-1)
		throw misc::Error("Guest program out of memory.");

	// Use map to allocate a new page
	memory->Map(addr, memory->PageSize, 0x06);

	// Make the whole page a big hole
	Chunk *hole = createHole(addr, memory->PageSize);

	return hole;
}


Manager::Chunk *Manager::createHole(unsigned addr, unsigned size)
{
	// Create the chunk object
	Chunk *hole = new Chunk(addr, size);

	// Insert it into the chunks map
	auto pair = chunks.emplace(addr, std::unique_ptr<Chunk>(hole));
	hole->setChunksIterator(pair.first);

	// Insert it into the holes map
	auto it = holes.emplace(size, hole);
	hole->setHolesIterator(it);	

	// return it	
	return hole;
}


void Manager::removeHole(Chunk *hole)
{
	holes.erase(hole->getHolesIterator());
	chunks.erase(hole->getChunksIterator());
}


Manager::Chunk *Manager::createPointer(unsigned addr, unsigned size)
{
	// Create the chunk object
	Chunk *pointer = new Chunk(addr, size, true);

	// Insert it into the chunks map
	auto pair = chunks.emplace(addr, std::unique_ptr<Chunk>(pointer));
	pointer->setChunksIterator(pair.first);

	// return the pointer
	return pointer;
}


void Manager::removePointer(Chunk *pointer)
{
	chunks.erase(pointer->getChunksIterator());
}


void Manager::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n***** Memory *****\n");
	DumpChunks(os);
	os << misc::fmt("******************\n");
}

}  // namespace mem

