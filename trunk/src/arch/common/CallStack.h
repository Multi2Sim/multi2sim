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

#ifndef ARCH_COMMON_CALL_STACK_H
#define ARCH_COMMON_CALL_STACK_H

#include <deque>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <lib/cpp/Debug.h>
#include <lib/cpp/ELFReader.h>


namespace comm
{

class CallStackMap
{
	// Path of ELF file for the map
	std::string path;

	// Virtual address in the context's memory where the map was made
	unsigned address;

	// Size in bytes of the map
	unsigned size;

public:

	/// Constructor
	CallStackMap(const std::string &path,
			unsigned address,
			unsigned size)
			:
			path(path),
			address(address),
			size(size)
	{
	}

	/// Return the ELF path of this map
	const std::string &getPath() const { return path; }

	/// Return the virtual address where map was made
	unsigned getAddress() const { return address; }

	/// Return the size of the map
	unsigned getSize() const { return size; }
};


class CallStack
{
	// Debugger
	static misc::Debug debug;

	// Path of main executable
	std::string path;

	// Maximum call stack size
	size_t max_size;

	// Entries
	std::deque<unsigned> stack;

	// Maps
	std::vector<CallStackMap> maps;

	// Current call level
	int level;

	// List of ELF files created, holding ownership with smart pointer
	std::list<std::unique_ptr<ELFReader::File>> elf_file_list;

	// Map of ELF files, indexed by name
	std::unordered_map<std::string, ELFReader::File *> elf_file_map;

	// Parse ELF file and return it, or return a previously parsed one.
	ELFReader::File *getELFFile(const std::string &path);

	// Return a symbol name for a virtual address.
	std::string getSymbolName(unsigned address);

	// Dump the current level of the call stack
	void DumpCurrentLevel(std::ostream &os);

public:

	/// Constructor
	///
	/// \param path
	///	Main executable for which the call stack is created.
	///
	/// \param max_size (optional, default = 500)
	///	Maximum call stack size.
	CallStack(const std::string &path, size_t max_size = 500);

	/// Map a portion of an ELF binary into a region of the memory map. This
	/// information is used to search for ELF symbols when reconstructing
	/// the stack back trace.
	///
	/// \param binary
	///	Name of the ELF binary from which the region is taken.
	///
	/// \param offset
	///	Offset in ELF binary where region starts.
	///
	/// \param size
	///	Size of the region in bytes.
	///
	/// \param address
	///	Virtual memory address where the region is loaded.
	void Map(const std::string &binary,
			unsigned address,
			unsigned size);

	/// Record a function call
	void Call(unsigned address);

	/// Record a function return
	void Return();
	
	/// Dump stack back trace to output stream
	void BackTrace(std::ostream &os = std::cout);

	/// Activate debug information for the call stacks.
	///
	/// \param path
	///	File where debug information will be dumped. Special names \c
	///	stdout and \c stderr can be used to refer to the standard output
	///	and standard error output, respectively.
	static void setDebugPath(const std::string &path)
	{
		debug.setPath(path);
		debug.setPrefix("[CallStack]");
	}
};

}  // namespace comm

#endif

