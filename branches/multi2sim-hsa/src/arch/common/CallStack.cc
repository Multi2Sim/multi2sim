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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CallStack.h"

#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>


namespace comm
{

misc::Debug CallStack::debug;


ELFReader::File *CallStack::getELFFile(const std::string &path)
{
	// Check if file was already parsed
	auto it = elf_file_map.find(path);
	if (it != elf_file_map.end())
		return it->second;
	
	// Parse file
	ELFReader::File *elf_file;
	try
	{
		// Load file
		elf_file = new ELFReader::File(path);
	}
	catch (ELFReader::Error &e)
	{
		// Error loading file
		debug << e;
		elf_file_map[path] = nullptr;
		return nullptr;
	}

	// Save ELF file
	elf_file_list.emplace_back(elf_file);
	elf_file_map[path] = elf_file;

	// Return it
	return elf_file;
}


std::string CallStack::getSymbolName(unsigned address)
{
	// Identify map
	CallStackMap *map = nullptr;
	for (auto it = maps.rbegin(); it != maps.rend(); ++it)
	{
		if (address >= it->getAddress() &&
				address < it->getAddress()
				+ it->getSize())
		{
			map = &(*it);
			break;
		}
	}

	// No map found
	std::string address_str = misc::fmt("<0x%x>", address);
	if (!map)
		return address_str;

	// Get ELF file
	ELFReader::File *elf_file = getELFFile(map->getPath());
	address_str = misc::fmt("<0x%x> @%s", address, map->getPath().c_str());
	if (!elf_file)
		return address_str;

	// Get symbol
	unsigned offset;
	ELFReader::Symbol *elf_symbol = elf_file->getSymbolByAddress(
			address - map->getAddress() + map->getOffset(),
			offset);
	if (!elf_symbol)
		return address_str;

	// Symbol and offset found
	std::string offset_str = offset ? misc::fmt(" + 0x%x", offset) : "";
	std::string name = misc::fmt("%s%s @ %s",
			elf_symbol->getName().c_str(),
			offset_str.c_str(),
			map->getPath().c_str());
	return name;
}


void CallStack::DumpCurrentLevel(std::ostream &os)
{
	// Level prefix
	std::string levels;
	for (int i = 0; i < level; i++)
		levels += ".  ";
	
	// Get current address
	if (level == 0)
	{
		debug << misc::fmt("[%s] Top\n", path.c_str());
		return;
	}
	else if (stack.size() == 0)
	{
		debug << misc::fmt("[%s] %sLost\n", path.c_str(),
				levels.c_str());
		return;
	}

	// Dump
	unsigned ip = stack.back().getIp();
	std::string name = getSymbolName(ip);
	debug << misc::fmt("[%s] %s%s\n",
			path.c_str(),
			levels.c_str(),
			name.c_str());
}


CallStack::CallStack(const std::string &path, size_t max_size)
{
	// Initialize arguments
	this->path = path;
	this->max_size = max_size;

	// Initialize fields
	level = 0;

	// Debug
	debug << misc::fmt("[%s] Call stack object created\n", path.c_str());
}
	

void CallStack::Map(const std::string &binary_path,
		unsigned offset,
		unsigned address,
		unsigned size,
		bool dynamic)
{
	// Add map
	maps.emplace_back(binary_path,
			offset,
			address,
			size,
			dynamic);
	
	// Debug
	debug << misc::fmt("[%s] %s@0x%x mapped to %x-%x\n",
			path.c_str(),
			binary_path.c_str(),
			offset,
			address,
			address + size - 1);
}


void CallStack::Call(unsigned ip, unsigned sp)
{
	// Push new address to stack
	stack.emplace_back(ip, sp);

	// Remove bottom of stack if maximum size exceeded
	if (stack.size() > max_size)
		stack.pop_front();
	
	// Increase call level
	level++;
	
	// Debug
	if (debug)
		DumpCurrentLevel(debug);
}


void CallStack::Return(unsigned ip, unsigned sp)
{
	// Pop stack
	if (stack.size() > 0)
		stack.pop_back();
	
	// Decrease call level
	if (level > 0)
		level--;
	
	// Debug
	if (debug)
		DumpCurrentLevel(debug);
}


void CallStack::BackTrace(unsigned address, std::ostream &os)
{
	// Header
	os << misc::fmt("\nBacktrace for '%s'\n", path.c_str());

	// No stack
	if (stack.size() == 0)
	{
		os << "\t-No stack-\n\n";
		return;
	}

	// Traverse stack
	for (int i = 0; i < (int) stack.size(); i++)
	{
		unsigned ip = i == (int) stack.size() - 1 ?
				address : stack[i].getIp();
		std::string name = getSymbolName(ip);
		os << misc::fmt("\t%d. %s\n", i + 1, name.c_str());
	}
}


}  // namespace comm

