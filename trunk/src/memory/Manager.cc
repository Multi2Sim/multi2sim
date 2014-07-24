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

Manager::Manager(Memory *memory)
{
	this->memory = memory;
}


unsigned Manager::Allocate(unsigned size, unsigned alignment=1)
{
	// Assert the alignment is smaller than page size
	assert(alignment<=Memory::PageSize);

	// If requested size is larger than a page, allocate whole pages for it
	if (size > Memory::PageSize)
	{
		return AllocateLarge(size);
	}

	// Traverse all holes to find an available slot
	for (auto it = holes.lower_bound(size); it != holes.end(); it++)
	{
		if (canHoleContain(it->second, size, alignment))
		{
			return AllocateIn(it->second, size, alignment);
		}
	}
	
	// No available space, need a new page
	Chunk *hole = RequestOnePage();
	unsigned addr = hole->getAddress();
	AllocateIn(hole, size, alignment);
	return addr;
}


bool Manager::canHoleContain(Chunk *hole, unsigned size,
		unsigned alignment) const
{
	unsigned aligned_address = getNextAlignedAddress(hole->getAddress(),
			alignment);
	unsigned offset = aligned_address - size;

	return hole->getSize() >= size + offset;
}


unsigned Manager::getNextAlignedAddress(unsigned address,
			unsigned alignment) const
{
	return ceil((double)address / alignment) * alignment;
}


unsigned Manager::AllocateLarge(unsigned size)
{

	unsigned int addr = Manager::MapBaseAddress;
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
	CreatePointer(addr, page_aligned_size);

	return addr;
}


unsigned Manager::AllocateIn(Chunk *hole, unsigned size, unsigned alignment)
{
	// Assert the hole can contain the size
	if (!canHoleContain(hole, size, alignment))
		throw misc::Panic("Memory hole cannot contain size");

	// Remove the hole  
	unsigned address = hole->getAddress();
	unsigned hole_size = hole->getSize();
	RemoveHole(hole);

	// Get the address to allocate memory
	unsigned aligned_address = getNextAlignedAddress(address, alignment);

	// Insert first hole, in front of allocated memory chunk.
	unsigned hole1_size = aligned_address - address;
	if (hole1_size > 0)
	{
		CreateHole(address, hole1_size);
	}

	// Insert second hole, behind the allocated memory chunk
	unsigned hole2_size = hole_size - size - hole1_size;
	if (hole2_size > 0)
	{
		CreateHole(address + hole1_size + size, hole2_size);
	}

	// Create a new pointer
	CreatePointer(aligned_address, size);
	return aligned_address;
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

	RemovePointer(it->second.get());

	// Add the hole
	Chunk *hole = CreateHole(address, size);

	// Merge Holes
	MergeHoles(hole);
}


void Manager::FreeLarge(unsigned address)
{

	auto it = chunks.find(address);
	if (it == chunks.end() && it->second->isAllocated())
	{
		throw Error("Trying to free a pointer not created by Allocate "
				" or it has been freed before");
	}

	// Remove the page
	unsigned num_pages = it->second->getSize() / Memory::PageSize;
	for (unsigned i = 0; i < num_pages; i++)
	{
		DeallocatePage(address);
		address += Memory::PageSize;
	}


	// Remove chunk
	RemovePointer(it->second.get());

}


void Manager::MergeHoles(Chunk *hole)
{
	// Assert the chunk passed in as argument is a hole
	if (hole->isAllocated())
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
			(!it->second->isAllocated()))
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
				(!chunk2->isAllocated()))
		{
			Merge2Holes(chunk2, chunk1);
			merged_addr = chunk2->getAddress();
		}
	}



	// Check if the page is free
	it = chunks.lower_bound(merged_addr);
	if (it->second->getSize() == Memory::PageSize)	
	{
		DeallocatePage(merged_addr);
		RemoveHole(it->second.get());
	}
}


void Manager::Merge2Holes(Chunk *hole1, Chunk *hole2)
{
	// Assert hole1 and hole2 are not allocated and they are consecutive
	if (hole1->isAllocated())
	{
		throw misc::Panic("Hole 1 is not a hole.");
	}
	if (hole2->isAllocated())
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
	RemoveHole(hole2);

	// Enlarge first hole
	hole1->setSize(size1 + size2);
}


bool Manager::isInSamePage(unsigned addr1, unsigned addr2)
{
	unsigned pageIndex1 = addr1 / Memory::PageSize;
	unsigned pageIndex2 = addr2 / Memory::PageSize;
	return pageIndex1 == pageIndex2;
}


void Manager::DeallocatePage(unsigned addr)
{
	// Deallocate page
	memory->Unmap(addr, Memory::PageSize);
}


Manager::Chunk *Manager::RequestOnePage()
{
	unsigned int addr = Manager::MapBaseAddress;

	// Find a good place to allocate a new page
	addr = memory->MapSpaceDown(addr, Memory::PageSize);
	if (addr == (unsigned)-1)
		throw misc::Error("Guest program out of memory.");

	// Use map to allocate a new page
	memory->Map(addr, memory->PageSize, 0x06);

	// Make the whole page a big hole
	Chunk *hole = CreateHole(addr, memory->PageSize);

	return hole;
}


Manager::Chunk *Manager::CreateHole(unsigned addr, unsigned size)
{
	// Create the chunk object
	Chunk *hole = new Chunk(addr, size);

	// Insert it into the chunks map
	auto pair = chunks.insert(std::make_pair(addr,
			std::unique_ptr<Chunk>(hole)));
	hole->setChunksIterator(pair.first);

	// Insert it into the holes map
	auto it = holes.insert(std::make_pair(size, hole));
	hole->setHolesIterator(it);	

	// Return the hole created
	return hole;
}


void Manager::RemoveHole(Chunk *hole)
{
	holes.erase(hole->getHolesIterator());
	chunks.erase(hole->getChunksIterator());
}


Manager::Chunk *Manager::CreatePointer(unsigned addr, unsigned size)
{
	// Create the chunk object
	Chunk *pointer = new Chunk(addr, size, true);

	// Insert it into the chunks map
	auto pair = chunks.insert(std::make_pair(addr,
			std::unique_ptr<Chunk>(pointer)));
	pointer->setChunksIterator(pair.first);

	// return the pointer
	return pointer;
}


void Manager::RemovePointer(Chunk *pointer)
{
	// Remove the chunk in the chunks map
	chunks.erase(pointer->getChunksIterator());
}


void Manager::DumpChunks(std::ostream &os = std::cout) const
{
	os << "Chunks *****\n";
	for (auto it = chunks.begin(); it != chunks.end(); it++)
	{
		os << *(it->second.get());
	}
	os << "***** *****\n\n";
}


void Manager::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n***** Memory *****\n");
	DumpChunks(os);
	os << misc::fmt("******************\n");
}

}  // namespace mem

