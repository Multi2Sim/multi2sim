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

#include <algorithm>
#include <cassert>
#include <cstring>

#include <lib/cpp/Misc.h>

#include "SpecMem.h"


namespace mem
{

SpecMem::Page *SpecMem::getPage(unsigned address)
{
	// Get page position
	address &= PageMask;
	unsigned index = (address >> LogPageSize) % PageTableSize;

	// Look for page
	Page *page = pages[index];
	while (page && page->address != address)
		page = page->next;

	// Return it (could be nullptr)
	return page;
}


SpecMem::Page *SpecMem::newPage(unsigned address)
{
	// Get page position
	address &= PageMask;
	unsigned index = (address >> LogPageSize) % PageTableSize;

	// Create page
	Page *page = new Page();
	page->address = address;

	// Read initial contents of page. The read has to be done in unsafe mode,
	// since we might be speculatively accessing an invalid page.
	bool safe = memory->getSafe();
	memory->setSafe(false);
	memory->Read(address, PageSize, page->data);
	memory->setSafe(safe);

	// Insert page in hash table
	page->next = pages[index];
	pages[index] = page;
	num_pages++;

	// Return it
	return page;
}


void SpecMem::AccessAligned(unsigned address, int size, char *buffer,
		Memory::AccessType access)
{
	// Get the memory page
	Page *page = getPage(address);
	if (!page)
	{
		// On a read, an access to a non-existent location in
		// speculative memory just translates into the corresponding
		// access to non-speculative memory. This access must be made in
		// unsafe mode, in case the page is invalid.
		if (access == Memory::AccessRead)
		{
			bool safe = memory->getSafe();
			memory->setSafe(false);
			memory->Read(address, size, buffer);
			memory->setSafe(safe);
			return;
		}
		else if (access == Memory::AccessWrite)
		{
			// On a write, we need to create a new page. If the
			// maximum number of pages has been reached, the write
			// will be ignored.
			if (num_pages >= MaxPages)
				return;

			// Create page
			page = newPage(address);
		}
		else
		{
			misc::panic("%s: invalid access type (%d)",
					__FUNCTION__, access);
		}
	}

	// We have the page, now access it
	unsigned offset = address & ~PageMask;
	assert(offset + size <= PageSize);
	if (access == Memory::AccessRead)
		memcpy(buffer, page->data + offset, size);
	else if (access == Memory::AccessWrite)
		memcpy(page->data + offset, buffer, size);
	else
		misc::panic("%s: invalid access type (%d)",
				__FUNCTION__, access);
}


void SpecMem::Access(unsigned address, int size, char *buffer,
		Memory::AccessType access)
{
	while (size)
	{
		unsigned int offset = address & (PageSize - 1);
		int chunksize = std::min(size, (int) (PageSize - offset));
		AccessAligned(address, chunksize, buffer, access);

		size -= chunksize;
		buffer += chunksize;
		address += chunksize;
	}
}


SpecMem::SpecMem(Memory *memory)
{
	// Make sure that the speculative memory page is a divisor of the
	// non-speculative memory page size.
	assert(Memory::PageSize % PageSize == 0);

	// Initialize
	this->memory = memory;
	num_pages = 0;
	for (unsigned i = 0; i < PageTableSize; i++)
		pages[i] = nullptr;
}


void SpecMem::Clear()
{
	for (unsigned i = 0; i < PageTableSize; i++)
	{
		Page *page = pages[i];
		while (page)
		{
			Page *next = page->next;
			delete page;
			page = next;
		}
		pages[i] = nullptr;
	}
	num_pages = 0;
}

} // namespace mem

