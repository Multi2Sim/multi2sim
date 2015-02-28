/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef MEMORY_SPEC_MEM_H
#define MEMORY_SPEC_MEM_H

#include "Memory.h"


namespace mem
{

/// This class represents a temporary memory used during speculative execution
/// of a CPU context. Speculative memory writes are stored here. A subsequent
/// load during speculative execution will read its value from this memory if it
/// exists, and will load from the actual complete Memory object otherwise. When
/// speculative execution ends, the contents of this memory will be just
/// discarded.
class SpecMem
{
	// Page size
	static const unsigned LogPageSize = 4;
	static const unsigned PageSize = 1 << LogPageSize;
	static const unsigned PageMask = ~(PageSize - 1);

	// Number of entries in the page table
	static const unsigned PageTableSize = 32;

	// To prevent an excessive growth of speculative memory, this is a limit
	// of pages. After this limit has reached, no more pages are allocated,
	// reads will be done from the non-speculative memory, and writes will
	// be ignored while in speculative mode.
	static const int MaxPages = 100;

	// Memory page
	struct Page
	{
		unsigned address;
		char data[PageSize];
		Page *next;
	};

	// Associated non-speculative memory
	Memory *memory;

	// Number of pages
	int num_pages;
	
	// Page table (hash table of pages)
	Page *pages[PageTableSize];

	// Return the page containing the address, or nullptr if not present
	Page *getPage(unsigned address);

	// Create a new page with the given address
	Page *newPage(unsigned address);

	// Access the memory at an aligned position
	void AccessAligned(unsigned address, int size, char *buffer,
			Memory::AccessType access);

	// Access the memory without alignment restrictions
	void Access(unsigned address, int size, char *buffer,
			Memory::AccessType access);
public:

	/// Create a speculative memory associated with a real memory object
	SpecMem(Memory *memory);

	/// Destructor
	~SpecMem()
	{
		Clear();
	}

	/// Read from the memory
	void Read(unsigned address, int size, char *buffer)
	{
		Access(address, size, buffer, Memory::AccessRead);
	}

	/// Write to memory
	void Write(unsigned address, int size, char *buffer)
	{
		Access(address, size, buffer, Memory::AccessWrite);
	}

	/// Clear content of the memory
	void Clear();
};


}  // namespace mem

#endif

