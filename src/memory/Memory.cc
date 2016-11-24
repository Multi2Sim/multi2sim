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

bool Memory::safe_mode = true;


Memory::Page *Memory::getPage(unsigned address)
{
	unsigned tag = address & ~(PageSize - 1);
	auto it = pages.find(tag);
	return it == pages.end() ? nullptr : it->second.get();
}


Memory::Page *Memory::getNextPage(unsigned address)
{
	// Get tag of the page just following address
	unsigned tag = (address + PageSize) & ~(PageSize - 1);
	if (!tag)
		return nullptr;

	// Look for a page exactly following the given address. If it is found,
	// return it.
	auto it = pages.find(tag);
	if (it != pages.end())
		return it->second.get();
	
	// Page following address is not found, so check all memory pages to
	// find the one with the lowest tag following address.
	unsigned min_tag = 0xffffffff;
	Page *min_page = nullptr;
	for (auto &pair : pages)
	{
		Page *page = pair.second.get();
		if (page->getTag() > tag && page->getTag() < min_tag)
		{
			min_tag = page->getTag();
			min_page = page;
		}
	}

	// Return the found page (or nullptr)
	return min_page;
}

Memory::Page *Memory::newPage(unsigned address, unsigned perm)
{
	// Allocate new page
	unsigned tag = address & ~(PageSize - 1);
	auto ret = pages.emplace(tag, misc::new_unique<Page>(tag, perm));

	// Check tag page was inserted successfully
	bool success = ret.second;
	if (!success)
		throw misc::Panic("Memory page already exists");

	// Obtain inserted page
	auto it = ret.first;
	Page *page = it->second.get();

	// Return it
	return page;
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
		
		// Different actions depending on whether source and
		// destination page data are allocated.
		if (page_src->getData())
		{
			page_dest->AllocateData();
			memcpy(page_dest->getData(), page_src->getData(),
					PageSize);
		}
		else
		{
			if (page_dest->getData())
				memset(page_dest->getData(), 0, PageSize);
		}

		// Advance pointers
		src += PageSize;
		dest += PageSize;
		size -= PageSize;
	}
}


char *Memory::getBuffer(unsigned address, unsigned size, AccessType access)
{
	// Get page offset and check page bounds
	unsigned offset = address & (PageSize - 1);
	if (offset + size > PageSize)
		return nullptr;
	
	// Look for page
	Page *page = getPage(address);
	if (!page)
		return nullptr;
	
	// Check page permissions
	if ((page->getPerm() & access) != access && safe)
		throw Error(misc::fmt("[0x%x] Permission denied", address));
	
	// Return pointer to page data
	page->AllocateData();
	return page->getData() + offset;
}


void Memory::AccessAtPageBoundary(unsigned address, unsigned size,
		char *buffer, AccessType access)
{
	// Find memory page and compute offset.
	Page *page = getPage(address);
	unsigned offset = address & (PageSize - 1);
	assert(offset + size <= PageSize);

	// On nonexistent page, raise segmentation fault in safe mode,
	// or create page with full privileges for writes in unsafe mode.
	if (!page)
	{
		if (safe)
			throw Error(misc::fmt("[0x%x] Segmentation fault in "
					"guest program", address));
		if (access == AccessRead || access == AccessExec)
		{
			memset(buffer, 0, size);
			return;
		}
		if (access == AccessWrite || access == AccessInit)
		{
			page = newPage(address, AccessRead |
				AccessWrite | AccessExec |
				AccessInit);
		}
	}
	assert(page);

	// If it is a write access, set the 'modified' flag in the page
	// attributes (perm). This is not done for 'initialize' access.
	if (access == AccessWrite)
		page->addPerm(AccessModified);

	// Check permissions in safe mode
	if (safe && (page->getPerm() & access) != access)
		throw Error(misc::fmt("[0x%x] Permission denied", address));

	// Read/execute access
	if (access == AccessRead || access == AccessExec)
	{
		if (page->getData())
			memcpy(buffer, page->getData() + offset, size);
		else
			memset(buffer, 0, size);
		return;
	}

	// Write/initialize access
	if (access == AccessWrite || access == AccessInit)
	{
		page->AllocateData();
		memcpy(page->getData() + offset, buffer, size);
		return;
	}

	// Shouldn't get here.
	abort();
}


void Memory::Access(unsigned address, unsigned size, char *buf,
			AccessType access)
{
	last_address = address;
	while (size)
	{
		unsigned offset = address & (PageSize - 1);
		int chunksize = std::min(size, PageSize - offset);
		AccessAtPageBoundary(address, chunksize, buf, access);

		size -= chunksize;
		buf += chunksize;
		address += chunksize;
	}
}


Memory::Memory()
{
	// Initialize
	safe = safe_mode;
}


Memory::Memory(const Memory &memory)
{
	// Copy pages
	safe = false;
	for (auto &it : memory.pages)
	{
		// Get source page
		Page *src_page = it.second.get();

		// Create destination page with same permissions
		newPage(src_page->getTag(), src_page->getPerm());

		// Copy data if any
		if (src_page->getData())
			Access(src_page->getTag(), PageSize,
					src_page->getData(),
					AccessInit);
	}

	// Copy other fields
	safe = memory.safe;
	heap_break = memory.heap_break;
}


unsigned Memory::MapSpace(unsigned address, unsigned size)
{
	assert(!(address & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag_start = address;
	unsigned tag_end = address;
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


unsigned Memory::MapSpaceDown(unsigned address, unsigned size)
{
	assert(!(address & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag_start = address;
	unsigned tag_end = address;
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


void Memory::Map(unsigned address, unsigned size, unsigned perm)
{
	// Calculate page boundaries
	unsigned tag1 = address & ~(PageSize-1);
	unsigned tag2 = (address + size - 1) & ~(PageSize-1);

	// Allocate pages
	for (unsigned tag = tag1; tag <= tag2; tag += PageSize)
	{
		Page *page = getPage(tag);
		if (!page)
			page = newPage(tag, perm);
		page->addPerm(perm);
	}
}


void Memory::Unmap(unsigned address, unsigned size)
{
	// Calculate page boundaries
	assert(!(address & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag1 = address & ~(PageSize-1);
	unsigned tag2 = (address + size - 1) & ~(PageSize-1);

	// Deallocate pages
	for (unsigned tag = tag1; tag <= tag2; tag += PageSize)
		pages.erase(tag);
}


void Memory::Protect(unsigned address, unsigned size, unsigned perm)
{
	// Calculate page boundaries
	assert(!(address & (PageSize - 1)));
	assert(!(size & (PageSize - 1)));
	unsigned tag1 = address & ~(PageSize-1);
	unsigned tag2 = (address + size - 1) & ~(PageSize-1);

	// Assign new permissions
	for (unsigned tag = tag1; tag <= tag2; tag += PageSize)
	{
		Page *page = getPage(tag);
		if (!page)
			continue;

		// Set page new protection flags
		page->setPerm(perm);
	}
}


void Memory::WriteString(unsigned address, const std::string &s)
{
	Write(address, s.length() + 1, const_cast<char *>(s.c_str()));
}


std::string Memory::ReadString(unsigned address, int max_length)
{
	std::string s;
	for (int i = 0; i < max_length; i++)
	{
		// Read character
		char c;
		Read(address + i, 1, &c);

		// Done if null terminator found
		if (!c)
			return s;

		// Add character
		s += c;
	}

	// If the maximum length was reached and the null character was not
	// found, throw exception.
	throw misc::Panic(misc::fmt("String at 0x%x exceeds %d characters",
			address, max_length));
}


void Memory::Zero(unsigned address, unsigned size)
{
	char zero = 0;
	while (size--)
		Write(address++, 1, &zero);
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
	for (auto &it : memory.pages)
	{
		// Get source page
		Page *src_page = it.second.get();

		// Create destination page with same permissions
		newPage(src_page->getTag(), src_page->getPerm());

		// Copy data if any
		if (src_page->getData())
			Access(src_page->getTag(), PageSize,
					src_page->getData(),
					AccessInit);
	}


	// Copy other fields
	safe = memory.safe;
	heap_break = memory.heap_break;
}


} // namespace mem

