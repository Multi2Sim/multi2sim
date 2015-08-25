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

#include <cassert>

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/String.h>

#include "Memory.h"
#include "Mmu.h"


namespace mem
{


//
// Class 'Mmu::Space'
//

Mmu::Space::Space(const std::string &name, Mmu *mmu) :
		name(name),
		mmu(mmu)
{
	// Debug
	debug << misc::fmt("[MMU %s] Space %s created\n",
			mmu->getName().c_str(),
			name.c_str());
}


void Mmu::Space::addPage(Page *page)
{
	// Sanity
	unsigned virtual_address = page->getVirtualAddress();
	assert((virtual_address & ~PageMask) == 0);
	assert(virtual_pages.find(virtual_address) == virtual_pages.end());
	virtual_pages[virtual_address] = page;
}


Mmu::Page *Mmu::Space::getPage(unsigned virtual_address)
{
	assert((virtual_address & ~PageMask) == 0);
	auto it = virtual_pages.find(virtual_address);
	return it == virtual_pages.end() ? nullptr : it->second;
}




//
// Class 'Mmu'
//

std::string Mmu::debug_file;

misc::Debug Mmu::debug;


void Mmu::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Memory");

	// Option --mmu-debug <file>
	command_line->RegisterString("--mmu-debug <file>", debug_file,
			"Dump debug information related with the memory "
			"management unit, virtual/physical memory address "
			"spaces, and address translations.");
}


void Mmu::ProcessOptions()
{
	// Debug file
	if (!debug_file.empty())
		debug.setPath(debug_file);
}


Mmu::Mmu(const std::string &name) :
		name(name)
{
	// Debug
	debug << misc::fmt("[MMU %s] Memory management unit created\n",
			name.c_str());
}


Mmu::Space *Mmu::newSpace(const std::string &name)
{
	spaces.emplace_back(new Space(name, this));
	return spaces.back().get();
}


unsigned Mmu::TranslateVirtualAddress(Space *space,
		unsigned virtual_address)
{
	// Space must belong to current MMU
	assert(space->getMmu() == this);

	// Calculate tag and offset
	unsigned virtual_tag = virtual_address & PageMask;
	unsigned page_offset = virtual_address & ~PageMask;

	// Find page, and created if not found
	Page *page = space->getPage(virtual_tag);
	if (page == nullptr)
	{
		// Create new page
		pages.emplace_back(new Page(space, virtual_tag,
				top_physical_address));
		
		// Add page to virtual and physical maps
		page = pages.back().get();
		physical_pages[page->getPhysicalAddress()] = page;
		space->addPage(page);

		// Increment top of physical address space
		top_physical_address += PageSize;

		// Debug
		if (debug)
			debug << misc::fmt("[MMU %s] Page created. "
					"Space %s, Virtual 0x%x => "
					"Physical 0x%x\n", name.c_str(),
					space->getName().c_str(),
					virtual_tag,
					page->getPhysicalAddress());
	}

	// Calculate physical address
	unsigned physical_address = page->getPhysicalAddress() + page_offset;

	// Debug
	if (debug)
		debug << misc::fmt("[MMU %s] Space %s, Virtual 0x%x => "
				"Physical 0x%x\n", name.c_str(),
				space->getName().c_str(),
				virtual_address,
				physical_address);

	// Done
	return physical_address;
}


bool Mmu::TranslatePhysicalAddress(unsigned physical_address,
		Space *&space,
		unsigned &virtual_address)
{
	// Find page
	unsigned physical_tag = physical_address & PageMask;
	unsigned page_offset = physical_address & ~PageMask;
	auto it = physical_pages.find(physical_tag);

	// Page not found
	if (it == physical_pages.end())
	{
		// Debug
		if (debug)
			debug << misc::fmt("[MMU %s] Physical 0x%x => "
					"Invalid page\n", name.c_str(),
					physical_address);

		// No page found
		space = nullptr;
		virtual_address = 0;
		return false;
	}

	// Return page information
	Page *page = it->second;
	space = page->getSpace();
	virtual_address = page->getVirtualAddress() + page_offset;

	// Debug
	if (debug)
		debug << misc::fmt("[MMU %s] Physical 0x%x => "
				"Space %s, Virtual 0x%x\n",
				name.c_str(),
				physical_address,
				space->getName().c_str(),
				virtual_address);

	// Page found
	return true;
}
	

bool Mmu::isValidPhysicalAddress(unsigned physical_address)
{
	unsigned physical_tag = physical_address & PageMask;
	auto it = physical_pages.find(physical_tag);
	return it != physical_pages.end();
}


} // namespace mem

