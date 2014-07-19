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

#include <list>
#include <map>
#include <memory>

#include <lib/cpp/String.h>

#include "Memory.h"

namespace mem
{
	
class Manager
{
	
	// Memory object it manage
	Memory *memory;

	// Page allocation base address
	static const unsigned map_base_address = 0xb7fb0000;

	// Memory align unit, default to be 8 bytes long
	static unsigned align_size;
	
	// Abstract a chunk of memory, including pointer and hole
	class Chunk
	{

		// pointer address
		unsigned address;

		// size
		unsigned size;

		// if the chunk is allocated 
		bool is_allocated;

		// Iterator to the chunk in chunks map
		std::map<unsigned, std::unique_ptr<Chunk>>::iterator it_chunks;

		// Iterator to the chunk in holes multimap
		std::map<unsigned, Chunk*>::iterator it_holes;

	public:

		/// Constructor
		Chunk(unsigned addr, unsigned size, bool is_allocated = false)
		{
			this->address = addr;
			this->size = size;
			this->is_allocated = is_allocated;
		}

		/// Returns the address
		unsigned getAddress() const { return address; };

		/// Set the address 
		void setAddress(unsigned addr) { address = addr; }

		/// Returns the end address
		unsigned getEndAddress() const { return address + size - 1; }

		/// Returns the size
		unsigned getSize() const { return size; }

		/// Set the size
		void setSize(unsigned size) { this->size = size; }

		/// Set is_allocated field
		void setIsAllocated(bool is_allocated)
		{ 
			this->is_allocated = is_allocated;
		}

		/// Returns is_allocated field
		bool IsAllocated() const {return is_allocated;}

		/// returns the iterator in the chunks map
		std::map<unsigned, std::unique_ptr<Chunk>>::iterator
				getChunksIterator() const { return it_chunks; }

		/// sets the iterator in the chunks map
		void setChunksIterator(std::map<unsigned, 
				std::unique_ptr<Chunk>>::iterator it)
		{
			it_chunks = it;
		}
		
		/// returns the iterator in the holes map
		std::multimap<unsigned, Chunk*>::iterator
			getHolesIterator() const 
		{ 
			if (is_allocated)
				throw misc::Error("Chunk is not a hole.");
			return it_holes; 
		}

		/// sets the iterator in the chunks map
		void setHolesIterator(std::multimap<unsigned, 
				Chunk*>::iterator it)
		{
			it_holes = it;
		}

		/// Dump the chunk information
		void Dump(std::ostream &os = std::cout) const
		{
			std::string chunk_allocation_status = 
					IsAllocated() ? "Occupied": "Free";
			os << misc::fmt("0x%x - 0x%0x (size %d) --- %s\n",
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
	std::map<unsigned int, std::unique_ptr<Chunk>> chunks;

	// Memory holes, map the size of the holes to chunks
	std::multimap<unsigned int, Chunk*> holes;

	// request a memory page 
	//
	// \return
	//	The hole created 
	Chunk* requestOnePage();

	// Deallocate a memory page
	void deallocatePage(unsigned size);

	// Merge memory holes in a certain page
	void MergeHoles(Chunk *hole);

	// Merge 2 consecutive holes
	void Merge2Holes(Chunk *hole1, Chunk*hole2);

	// Allocate memory at a certain address 
	// 
	// \param hole
	//	the hole for the new pointer to be allocated in
	//
	// \param size
	//	size of the memory requested
	void AllocateIn(Chunk *hole, unsigned int size);

	// Allocate memory chunk larger than a page size
	unsigned int AllocateLarge(unsigned int size);

	// Free memory chunk larger than a page size 
	void FreeLarge(unsigned int size);

	// Returns the hole or pointer next to address 
	unsigned int getNextAddress(unsigned int address);

	// Returns the aligned size
	unsigned toAlignedSize(unsigned size);

	// Judges if a size is aligned
	bool isAlignedSize(unsigned size) const { return size % align_size; }

	// Judges if the hole can hold the size requested
	bool canHoleContain(Chunk *hole, unsigned int size);

	// Create a hole
	Chunk *createHole(unsigned addr, unsigned size);

	// Remove a hole
	void removeHole(Chunk *hole);

	// Create a pointer
	Chunk *createPointer(unsigned addr, unsigned size);

	// Judge if two address are in same page
	bool isInSamePage(unsigned addr1, unsigned addr2);

	// Remove a pointer
	void removePointer(Chunk *pointer);

public:

	/// Constructor, assign the memory to manange
	Manager(Memory *memory);

	/// Allocate a piece of memory
	/// 
	/// \param size
	///		size of the address requested
	/// 
	/// \return
	///		base address to the allocated address
	unsigned int Allocate(unsigned int size);

	/// Free the memory at a certain address
	void Free(unsigned int address);

	/// Dump the memory managing status for debug purpose
	void Dump(std::ostream &os) const;

	/// Dump how chunk is allocated in the managed memory
	void DumpChunks(std::ostream &os = std::cout) const
	{
		os << "Chunks *****\n";
		for (auto it = chunks.begin(); it != chunks.end(); it++)
		{
			os << *(it->second.get());
		}
		os << "***** *****\n\n";
	}

	/// Operator \c << invoking the function Dump) on an output stream
	friend std::ostream &operator<<(std::ostream &os,
		const Manager &manager)
	{
		manager.Dump(os);
		return os;
	}

};

}  // namespace mem

