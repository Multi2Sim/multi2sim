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

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <list>
#include <map>
#include <memory>

#include <lib/cpp/String.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/CommandLine.h>

#include "Memory.h"


namespace mem
{

// A delegate of a memory object for memory allocation and deallocation
class Manager
{
protected:

	// Debug file name, as set by user
	static std::string debug_file;

	// Memory object it manage
	Memory *memory;

	// Page allocation base address
	static const unsigned MapBaseAddress = 0xb7fb0000;
	
	// The error to throw of memory manager
	class Error : public misc::Error
	{
	public:

		/// Constructor
		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Memory manager");
		}
	};

	// A chunk holds information of a piece of memory.
	// A chunk is called a hole if it is free.
	// A chunk is called a pointer if it allocated and can be freed.
	class Chunk
	{
		// Base address of the chunk
		unsigned address;

		// Size of the chunk
		unsigned size;

		// If the chunk is allocated.
		bool is_allocated;

		// Iterator to the chunk in chunks map
		std::map<unsigned, std::unique_ptr<Chunk>>::iterator it_chunks;

		// Iterator to the chunk in holes multimap
		std::map<unsigned, Chunk*>::iterator it_holes;

	public:

		/// Constructor
		Chunk(unsigned address, unsigned size,
				bool is_allocated = false) :
				address(address),
				size(size),
				is_allocated(is_allocated)
		{}

		/// Return the address
		unsigned getAddress() const { return address; };

		/// Set the address 
		void setAddress(unsigned address) { this->address = address; }

		/// Return the end address
		unsigned getEndAddress() const { return address + size - 1; }

		/// Return the size
		unsigned getSize() const { return size; }

		/// Set the size
		void setSize(unsigned size) { this->size = size; }

		/// Set is_allocated field
		void setAllocated(bool is_allocated)
		{ 
			this->is_allocated = is_allocated;
		}

		/// Return is_allocated field
		bool isAllocated() const {return is_allocated;}

		/// Return the iterator in the chunks map
		std::map<unsigned, std::unique_ptr<Chunk>>::iterator
				getChunksIterator() const { return it_chunks; }

		/// set the iterator in the chunks map
		void setChunksIterator(std::map<unsigned, 
				std::unique_ptr<Chunk>>::iterator it)
		{
			it_chunks = it;
		}
		
		/// Return the iterator in the holes map
		std::multimap<unsigned, Chunk*>::iterator
				getHolesIterator() const
		{
			assert(!is_allocated);
			return it_holes; 
		}

		/// Set the iterator in the chunks map
		void setHolesIterator(std::multimap<unsigned, 
				Chunk*>::iterator it)
		{
			it_holes = it;
		}

		/// Dump the chunk information
		void Dump(std::ostream &os = std::cout) const
		{
			std::string chunk_allocation_status = 
					isAllocated() ? "Occupied": "Free";
			os << misc::fmt("    0x%x - 0x%0x (size %d) --- %s\n",
					getAddress(),
					getEndAddress(),
					getSize(), 
					chunk_allocation_status.c_str());
		}

		/// Operator \c << invoking the function Dump) on an output 
		/// stream
		friend std::ostream &operator<<(std::ostream &os,
				const Chunk &chunk)
		{
			chunk.Dump(os);
			return os;
		}

	};

	// Memory chunks, map address to chunk information
	std::map<unsigned, std::unique_ptr<Chunk>> chunks;

	// Memory holes, map the size of the holes to chunks
	std::multimap<unsigned, Chunk*> holes;

	// Request a memory page
	//
	// \return
	//	The hole created 
	Chunk *RequestOnePage(unsigned base_address);

	// Deallocate a memory page
	void DeallocatePage(unsigned size);

	// Merge memory holes in a certain page
	void MergeHoles(Chunk *hole);

	// Merge 2 consecutive holes
	void Merge2Holes(Chunk *hole1, Chunk *hole2);

	// Allocate memory in a big enough hole, return the allocated base
	// address
	unsigned AllocateIn(Chunk *hole, unsigned size, unsigned alignment);

	// Allocate memory chunk larger than a page size
	unsigned AllocateLarge(unsigned size);

	// Free memory chunk larger than a page size 
	void FreeLarge(Chunk *pointer);

	// Determine if a size is aligned
	bool isAlignedSize(unsigned size, unsigned alignment) const
	{
		return !(size % alignment);
	}

	// Determine if the hole can hold the size requested
	bool canHoleContain(Chunk *hole, unsigned size,
			unsigned alignment) const;

	// Create a hole
	Chunk *CreateHole(unsigned addr, unsigned size);

	// Remove a hole
	void RemoveHole(Chunk *hole);

	// Create a pointer
	Chunk *CreatePointer(unsigned addr, unsigned size);

	// Remove a pointer
	void RemovePointer(Chunk *pointer);

	// Determine if two address are in same page
	bool isInSamePage(unsigned addr1, unsigned addr2);

	/// Return next closest aligned address
	unsigned getNextAlignedAddress(unsigned address,
			unsigned alignment) const;

public:

	/// Constructor, assign the memory to manager
	Manager(Memory *memory);

	/// Destroctur
	virtual ~Manager() { };

	/// Debugger
	static misc::Debug debug;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Allocate a piece of memory
	/// 
	/// \param size
	///	size of the address requested
	/// 
	/// \return
	///	base address to the allocated address
	virtual unsigned Allocate(unsigned size, unsigned alignment = 1);

	/// Free allocated memory.
	///
	/// \param address
	///     Base address of a chunk of memory, as returned by a previous
	///     call to Allocate().
	///
	/// \throws
	///     This function throws a Manager::Error exception if the address is
	///     not a valid address returned by a previous call to Allocate() or
	///     if it has been freed before.
	virtual void Free(unsigned address);

	/// Determines if the address falls in an allocated region
	bool isValidAddress(unsigned address);

	/// Get allocated size
	unsigned getAllocatedSize() const;

	/// Get occupied size, equals to the number of pages occupied
	unsigned getOccupiedSize() const;

	/// Dump how chunk is allocated in the managed memory
	void DumpChunks(std::ostream &os) const;

	/// Dump the memory managing status for debug purpose
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking the function Dump) on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Manager &manager)
	{
		manager.Dump(os);
		return os;
	}

};

}  // namespace mem

#endif

