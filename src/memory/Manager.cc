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


// Debug file name, as set by user
std::string Manager::debug_file;

// Debugger
misc::Debug Manager::debug;


void Manager::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Memory");

	// Option '--hsa-debug-driver <file>'
	command_line->RegisterString("--mem-manager-debug <file>", debug_file,
			"Dump debug information for the memory manager, "
			"including how much memory allocated, fragmentation.");
}


void Manager::ProcessOptions()
{
	debug.setPath(debug_file);
	//debug.setPrefix("[Memory Manager]");
}


unsigned Manager::Allocate(unsigned size, unsigned alignment)
{
	// Assert the alignment is smaller than page size
	assert(alignment<=Memory::PageSize);

	// Log memory allocation request in debug file
	debug << misc::fmt("%d bytes of memory requested, align to %d byte\n",
			size, alignment);

	// If requested size is larger than a page, allocate whole pages for it
	if (size > Memory::PageSize)
	{
		unsigned address = AllocateLarge(size);
		if (debug) Dump(debug);
		return address;
	}

	// Traverse all holes that have a bigger size than the required space to
	// find an available slot
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
			return address;
		}
	}
	
	// No available space, need a new page
	Chunk *hole = RequestOnePage(MapBaseAddress);
	unsigned addr = hole->getAddress();
	AllocateIn(hole, size, alignment);

	// After allocating memory space, dump summary of managed memory
	if(debug) Dump(debug);
	return addr;
}


bool Manager::canHoleContain(Chunk *hole, unsigned size,
		unsigned alignment) const
{
	// Get next aligned address
	unsigned aligned_address = getNextAlignedAddress(hole->getAddress(),
			alignment);

	// Calculate the size in bytes of the gap between the base address
	// of the hole to the first aligned address in the hole. It is the
	// space wasted in this allocation
	//
	//  | Beginning of the hole
	// -----------------
	// | | | | | | | | |
	// -----------------
	//      | First aligned address
	//
	// In this example the aligned size is at the third byte of the hole.
	// Therefore, the gap is 2. Assuming the hole is 8 bytes large,
	// only if the requested size is smaller than 6, can the hole contain
	// the request size of memory/
	unsigned gap = aligned_address - hole->getAddress();

	// Returns true if the hole's size is greater than the requested size
	// plus the gap.
	bool can_hold = hole->getSize() >= size + gap;

	// Dump information into debug file
	/*
	debug << misc::fmt("Checking if hole 0x%x, size %d, "
			"fit variable size %d, align to %d. "
			"Aligned size would be 0x%x. \n",
			hole->getAddress(), hole->getSize(),
			size, alignment, aligned_address);
	*/

	// Return result
	return can_hold;
}


unsigned Manager::getNextAlignedAddress(unsigned address,
			unsigned alignment) const
{
	return ceil((double)address / alignment) * alignment;
}


unsigned Manager::AllocateLarge(unsigned size)
{

	unsigned int address = Manager::MapBaseAddress;
	unsigned int page_aligned_size = 
			ceil((double)size / Memory::PageSize) 
			* Memory::PageSize;

	// Find a good place to allocate a new page
	address = memory->MapSpaceDown(address, page_aligned_size);
	if (address == (unsigned)-1)
		throw misc::Error("Guest program out of memory.");

	// Use map to allocate a new page
	memory->Map(address, size, 0x07);

	// Allocate the chunk
	CreatePointer(address, page_aligned_size);

	// Return the address to the newly allocated pointer
	return address;
}


unsigned Manager::AllocateIn(Chunk *hole, unsigned size, unsigned alignment)
{
	// Assert the hole can contain the size
	// assert(canHoleContain(hole, size, alignment));

	// Remove the hole  
	unsigned address = hole->getAddress();
	unsigned hole_size = hole->getSize();
	RemoveHole(hole);

	// Get the address to allocate memory
	unsigned aligned_address = getNextAlignedAddress(address, alignment);

	// Allocating a piece of memory in a hole may create a pointer and
	// two holes. For example, if the hole starts at address 30, and it is
	// 8 bytes long, we want to allocate a 3 bytes, 8 aligned address.
	//
	//
	// | Address 30
	// -----------------
	// | | | | | | | | |
	// -----------------
	//     | First aligned address 32
	//
	//
	// Then it become 3 small chunks
	//
	//
	// | Address 30       | Address 32       | Address 35
	// -----              -------            -------
	// | | |              |x|x|x|            | | | |
	// -----              -------            -------
	// Hole 1             Pointer            Hole 2
	//
	//
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
	// Dump information into debug file
	debug << misc::fmt("Free pointer at 0x%x.\n", address);

	// Get the chunk to be freed
	auto it = chunks.find(address);
	if (it == chunks.end())
	{
		throw Error("Trying to free a pointer not created by Allocate "
				"or has already been freed.");
	}

	// If the chunk is larger than a page size, use special rule for it.
	unsigned size = it->second->getSize();
	if (size > Memory::PageSize)
	{
		FreeLarge(it->second.get());
		if (debug) Dump(debug);
		return;
	}

	// Remove the pointer
	RemovePointer(it->second.get());

	// Add the hole
	Chunk *hole = CreateHole(address, size);

	// Merge Holes
	MergeHoles(hole);

	// Dump free result
	if (debug) Dump(debug);
}


void Manager::FreeLarge(Chunk *pointer)
{
	// Remove the page
	unsigned num_pages = pointer->getSize() / Memory::PageSize;
	unsigned address = pointer->getAddress();
	for (unsigned i = 0; i < num_pages; i++)
	{
		DeallocatePage(address);
		address += Memory::PageSize;
	}


	// Remove chunk
	RemovePointer(pointer);

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
	assert(!hole1->isAllocated());
	assert(!hole2->isAllocated());
	assert(hole1->getAddress() + hole1->getSize() == hole2->getAddress());

	// Get information
	unsigned addr = hole1->getAddress();
	unsigned size1 = hole1->getSize();
	unsigned size2 = hole2->getSize();

	// Erase the both holes
	RemoveHole(hole2);
	RemoveHole(hole1);

	// Add the large hole
	this->CreateHole(addr, size1 + size2);
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


Manager::Chunk *Manager::RequestOnePage(unsigned base_address)
{
	unsigned int addr = base_address;

	// Find a good place to allocate a new page
	addr = memory->MapSpace(addr, Memory::PageSize);
	if (addr == (unsigned)-1)
		throw misc::Error("Guest program out of memory.");

	// Use map to allocate a new page
	memory->Map(addr, memory->PageSize, 0x07);

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

	/*
	debug << misc::fmt("Hole created at: 0x%x, size: %d\n", addr, size);
	for (auto it = holes.begin(); it != holes.end(); it++)
	{
		debug << misc::fmt("Hole 0x%x, %d\n",
				it->second->getAddress(),
				it->first);
	}
	*/

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

bool Manager::isValidAddress(unsigned address)
{
	// Find the possible chunk it can locate in
	auto it = chunks.lower_bound(address);

	// Determine if the address falls in the chunk and if the chunk is
	// allocated
	if (it->second->getAddress() <= address &&
			it->second->getEndAddress() >= address &&
			it->second->isAllocated())
		return true;
	else
		return false;
}


unsigned Manager::getAllocatedSize() const
{
	unsigned size_allocated = 0;
	for (auto it = chunks.begin(); it != chunks.end(); it++)
	{
		Chunk *chunk = it->second.get();
		if (chunk->isAllocated())
		{
			size_allocated += chunk->getSize();
		}
	}
	return size_allocated;
}


unsigned Manager::getOccupiedSize() const
{
	unsigned size_occupied = 0;
	for (auto it = chunks.begin(); it != chunks.end(); it++)
	{
		Chunk *chunk = it->second.get();
		size_occupied += chunk->getSize();
	}
	return size_occupied;
}


void Manager::DumpChunks(std::ostream &os = std::cout) const
{
	os << "\n  ***** Chunks *****";

	unsigned prev_page = 0;

	for (auto it = chunks.begin(); it != chunks.end(); it++)
	{
		Chunk *chunk = it->second.get();

		// If in a new page, print page information
		unsigned curr_page = chunk->getAddress() / Memory::PageSize;
		if(curr_page != prev_page)
		{
			// Close previous page
			if (prev_page > 0)
				os << "  ***** **** *****\n";

			// Start a new page
			prev_page = curr_page;
			os << "\n  ***** Page *****\n";
		}

		// Dump chunk information
		os << *(chunk);
	}

	// Close last page
	if (prev_page > 0)
		os << "  ***** **** *****\n";

	// Close dumping chunks
	os << "\n  ***** ****** *****\n";
}


void Manager::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n***** Memory *****\n");
	DumpChunks(os);
	os << misc::fmt("  Allocated Size: %d,\n", getAllocatedSize());
	os << misc::fmt("  Occupied Size: %d,\n", getOccupiedSize());
	os << misc::fmt("  Fragmentation: %f\n",
			(double)getAllocatedSize()/(double)getOccupiedSize() );
	os << misc::fmt("\n***** ****** *****\n\n");
}

}  // namespace mem

