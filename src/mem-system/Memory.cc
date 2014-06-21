/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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
#include <fstream>

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Memory.h"


namespace mem
{

const unsigned Memory::LogPageSize;
const unsigned Memory::PageSize;
const unsigned Memory::PageMask;
const unsigned Memory::PageCount;

bool Memory::safe_mode = true;


Memory::Page *Memory::getPage(unsigned addr)
{
	unsigned tag = addr & ~(PageSize - 1);
	unsigned index = (addr >> LogPageSize) % PageCount;
	Page *page = page_table[index];
	Page *prev = nullptr;
	
	// Look for page
	while (page && page->tag != tag)
	{
		prev = page;
		page = page->next;
	}
	
	// Place page into list head
	if (prev && page)
	{
		prev->next = page->next;
		page->next = page_table[index];
		page_table[index] = page;
	}
	
	// Return found page
	return page;
}


Memory::Page *Memory::getNextPage(unsigned addr)
{
	// Get tag of the page just following addr
	unsigned tag = (addr + PageSize) & ~(PageSize - 1);
	if (!tag)
		return nullptr;
	unsigned index = (tag >> LogPageSize) % PageCount;
	Page *page = page_table[index];

	// Look for a page exactly following addr. If it is found, return it.
	while (page && page->tag != tag)
		page = page->next;
	if (page)
		return page;
	
	// Page following addr is not found, so check all memory pages to find
	// the one with the lowest tag following addr.
	unsigned mintag = 0xffffffff;
	Page *minpage = nullptr;
	for (index = 0; index < PageCount; index++)
	{
		for (page = page_table[index]; page; page = page->next)
		{
			if (page->tag > tag && page->tag < mintag)
			{
				mintag = page->tag;
				minpage = page;
			}
		}
	}

	// Return the found page (or nullptr)
	return minpage;
}

Memory::Page *Memory::newPage(unsigned addr, unsigned perm)
{
	// Initialize
	Page *page = new Page;
	unsigned tag = addr & ~(PageSize - 1);
	unsigned index = (addr >> LogPageSize) % PageCount;
	page->tag = tag;
	page->perm = perm;
	page->data = nullptr;
	
	// Insert in pages hash table
	page->next = page_table[index];
	page_table[index] = page;

	// Return
	return page;
}


void Memory::freePage(unsigned addr)
{
	unsigned tag = addr & ~(PageSize - 1);
	unsigned index = (addr >> LogPageSize) % PageCount;
	Page *prev = nullptr;

	// Find page
	Page *page = page_table[index];
	while (page && page->tag != tag)
	{
		prev = page;
		page = page->next;
	}
	if (!page)
		return;
	
	// Free page
	if (prev)
		prev->next = page->next;
	else
		page_table[index] = page->next;
	if (page->data)
		delete page->data;
	delete page;
}


void Memory::Copy(unsigned dest, unsigned src, unsigned size)
{
	// Restrictions. No overlapping allowed.
	assert(!(dest & (PageSize-1)));
	assert(!(src & (PageSize-1)));
	assert(!(size & (PageSize-1)));
	if ((src < dest && src + size > dest) ||
			(dest < src && dest + size > src))
		misc::panic("%s: cannot copy overlapping regions", __FUNCTION__);
	
	// Copy
	while (size > 0)
	{
		// Get source and destination pages
		Page *page_dest = getPage(dest);
		Page *page_src = getPage(src);
		assert(page_src && page_dest);
		
		/* Different actions depending on whether source and
		 * destination page data are allocated. */
		if (page_src->data)
		{
			if (!page_dest->data)
				page_dest->data = new char[PageSize];
			memcpy(page_dest->data, page_src->data, PageSize);
		}
		else
		{
			if (page_dest->data)
				memset(page_dest->data, 0, PageSize);
		}

		// Advance pointers
		src += PageSize;
		dest += PageSize;
		size -= PageSize;
	}
}


char *Memory::getBuffer(unsigned addr, unsigned size, AccessType access)
{
	// Get page offset and check page bounds
	unsigned offset = addr & (PageSize - 1);
	if (offset + size > PageSize)
		return nullptr;
	
	// Look for page
	Page *page = getPage(addr);
	if (!page)
		return nullptr;
	
	// Check page permissions
	if ((page->perm & access) != access && safe)
		throw Error(misc::fmt("[0x%x] Permission denied", addr));
	
	// Allocate and initialize page data if it does not exist yet.
	if (!page->data)
		page->data = new char[PageSize]();
	
	// Return pointer to page data
	return page->data + offset;
}


void Memory::AccessAtPageBoundary(unsigned addr, unsigned size, char *buf,
		AccessType access)
{
	// Find memory page and compute offset.
	Page *page = getPage(addr);
	unsigned offset = addr & (PageSize - 1);
	assert(offset + size <= PageSize);

	// On nonexistent page, raise segmentation fault in safe mode,
	// or create page with full privileges for writes in unsafe mode.
	if (!page)
	{
		if (safe)
			throw Error(misc::fmt("[0x%x] Segmentation fault in "
					"guest program", addr));
		if (access == AccessRead || access == AccessExec)
		{
			memset(buf, 0, size);
			return;
		}
		if (access == AccessWrite || access == AccessInit)
		{
			page = newPage(addr, AccessRead |
				AccessWrite | AccessExec |
				AccessInit);
		}
	}
	assert(page);

	// If it is a write access, set the 'modified' flag in the page
	// attributes (perm). This is not done for 'initialize' access.
	if (access == AccessWrite)
		page->perm |= AccessModif;

	// Check permissions in safe mode
	if (safe && (page->perm & access) != access)
		throw Error(misc::fmt("[0x%x] Permission denied", addr));

	// Read/execute access
	if (access == AccessRead || access == AccessExec)
	{
		if (page->data)
			memcpy(buf, page->data + offset, size);
		else
			memset(buf, 0, size);
		return;
	}

	// Write/initialize access
	if (access == AccessWrite || access == AccessInit)
	{
		if (!page->data)
			page->data = new char[PageSize]();
		memcpy(page->data + offset, buf, size);
		return;
	}

	// Shouldn't get here.
	abort();
}


void Memory::Access(unsigned addr, unsigned size, char *buf,
			AccessType access)
{
	last_address = addr;
	while (size)
	{
		unsigned offset = addr & (PageSize - 1);
		int chunksize = std::min(size, PageSize - offset);
		AccessAtPageBoundary(addr, chunksize, buf, access);

		size -= chunksize;
		buf += chunksize;
		addr += chunksize;
	}
}


Memory::Memory()
{
	// Initialize
	safe = safe_mode;
	heap_break = 0;
	last_address = 0;

	// Reset page table
	for (unsigned i = 0; i < PageCount; ++i)
		page_table[i] = nullptr;
}


Memory::Memory(const Memory &memory)
{
	// Reset page table
	for (unsigned i = 0; i < PageCount; ++i)
		page_table[i] = nullptr;

	// Copy pages
	safe = false;
	for (unsigned i = 0; i < PageCount; i++)
	{
		for (Page *page = memory.page_table[i]; page;
				page = page->next)
		{
			newPage(page->tag, page->perm);
			if (page->data)
				Access(page->tag, PageSize,
					page->data, AccessInit);
		}
	}

	// Copy other fields
	safe = memory.safe;
	heap_break = memory.heap_break;
	last_address = 0;
}


Memory::~Memory()
{
	Clear();
}


void Memory::Clear()
{
	for (unsigned i = 0; i < PageCount; i++)
		while (page_table[i])
			freePage(page_table[i]->tag);
}


unsigned Memory::MapSpace(unsigned addr, unsigned size)
{
	assert(!(addr & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag_start = addr;
	unsigned tag_end = addr;
	for (;;)
	{
		// Address space overflow
		if (!tag_end)
			return -1;
		
		// Not enough free pages in current region
		if (getPage(tag_end))
		{
			tag_end += PageSize;
			tag_start = tag_end;
			continue;
		}
		
		// Enough free pages
		if (tag_end - tag_start + PageSize == size)
			break;
		assert(tag_end - tag_start + PageSize < size);
		
		// we have a new free page
		tag_end += PageSize;
	}

	// Return the start of the free space
	return tag_start;
}


unsigned Memory::MapSpaceDown(unsigned addr, unsigned size)
{
	assert(!(addr & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag_start = addr;
	unsigned tag_end = addr;
	for (;;)
	{
		// Address space overflow
		if (!tag_start)
			return (unsigned) -1;
		
		// Not enough free pages in current region
		if (getPage(tag_start))
		{
			tag_start -= PageSize;
			tag_end = tag_start;
			continue;
		}
		
		// Enough free pages
		if (tag_end - tag_start + PageSize == size)
			break;
		assert(tag_end - tag_start + PageSize < size);
		
		// we have a new free page
		tag_start -= PageSize;
	}

	// Return the start of the free space
	return tag_start;
}


void Memory::Map(unsigned addr, unsigned size, unsigned perm)
{
	// Calculate page boundaries
	unsigned tag1 = addr & ~(PageSize-1);
	unsigned tag2 = (addr + size - 1) & ~(PageSize-1);

	// Allocate pages
	for (unsigned tag = tag1; tag <= tag2; tag += PageSize)
	{
		Page *page = getPage(tag);
		if (!page)
			page = newPage(tag, perm);
		page->perm |= perm;
	}
}


void Memory::Unmap(unsigned addr, unsigned size)
{
	// Calculate page boundaries
	assert(!(addr & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag1 = addr & ~(PageSize-1);
	unsigned tag2 = (addr + size - 1) & ~(PageSize-1);

	// Deallocate pages
	for (unsigned tag = tag1; tag <= tag2; tag += PageSize)
		freePage(tag);
}


void Memory::Protect(unsigned addr, unsigned size, unsigned perm)
{
	// Calculate page boundaries
	assert(!(addr & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag1 = addr & ~(PageSize-1);
	unsigned tag2 = (addr + size - 1) & ~(PageSize-1);

	// Assign new permissions
	for (unsigned tag = tag1; tag <= tag2; tag += PageSize)
	{
		Page *page = getPage(tag);
		if (!page)
			continue;

		// Set page new protection flags
		page->perm = perm;
	}
}


void Memory::WriteString(unsigned addr, const std::string &s)
{
	Write(addr, s.length() + 1, const_cast<char *>(s.c_str()));
}


std::string Memory::ReadString(unsigned addr, int max_length)
{
	std::string s;
	for (int i = 0; i < max_length; i++)
	{
		// Read character
		char c;
		Read(addr + i, 1, &c);
		if (!c)
			break;

		// Add character
		s += c;
	}
	return s;
}


void Memory::Zero(unsigned addr, unsigned size)
{
	char zero = 0;
	while (size--)
		Write(addr++, 1, &zero);
}


void Memory::Save(const std::string &path, unsigned start, unsigned end)
{
	std::ofstream f(path);
	if (!f)
		throw Error(misc::fmt("%s: Cannot open file", path.c_str()));
	
	// Set unsafe mode and dump
	bool old_safe = safe;
	safe = false;
	while (start < end)
	{
		char buf[PageSize];
		unsigned size = std::min(PageSize, end - start);
		Read(start, size, buf);
		f.write(buf, size);
		start += size;
	}

	// Restore safe mode
	safe = old_safe;
}


void Memory::Load(const std::string &path, unsigned start)
{
	// Open file
	std::ifstream f(path);
	if (!f)
		throw Error(misc::fmt("%s: Cannot open file", path.c_str()));
	
	// Set unsafe mode and load
	bool old_safe = safe;
	safe = false;
	while (f)
	{
		char buf[PageSize];
		f.read(buf, PageSize);
		unsigned size = f.gcount();
		Write(start, size, buf);
		start += size;
	}

	// Restore safe mode
	safe = old_safe;
}


void Memory::Clone(const Memory &memory)
{
	// Clear destination memory
	Clear();

	// Copy pages
	safe = false;
	for (unsigned i = 0; i < PageCount; i++)
	{
		for (Page *page = memory.page_table[i]; page; page = page->next)
		{
			newPage(page->tag, page->perm);
			if (page->data)
				Access(page->tag, PageSize, page->data,
						AccessInit);
		}
	}

	// Copy other fields
	safe = memory.safe;
	heap_break = memory.heap_break;
}


} // namespace mem

