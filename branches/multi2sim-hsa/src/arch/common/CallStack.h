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

	// Position in the source file where the map starts
	unsigned offset;

	// Virtual address in the context's memory where the map was made
	unsigned address;

	// Size in bytes of the map
	unsigned size;

	// Static/dynamic map
	bool dynamic;

public:

	/// Constructor
	CallStackMap(const std::string &path,
			unsigned offset,
			unsigned address,
			unsigned size,
			bool dynamic)
			:
			path(path),
			offset(offset),
			address(address),
			size(size),
			dynamic(dynamic)
	{
	}

	/// Return the ELF path of this map
	const std::string &getPath() const { return path; }

	/// Return the offset in the source file where map was taken from
	unsigned getOffset() const { return offset; }

	/// Return the virtual address where map was made
	unsigned getAddress() const { return address; }

	/// Return the size of the map
	unsigned getSize() const { return size; }
};


class CallStackFrame
{
	// Instruction pointer;
	unsigned ip;

	// Stack pointer;
	unsigned sp;

public:

	CallStackFrame(unsigned ip, unsigned sp)
			:
			ip(ip),
			sp(sp)
	{
	}

	/// Return the instruction pointer
	unsigned getIp() const { return ip; }

	/// Return the stack pointer
	unsigned getSp() const { return sp; }
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
	std::deque<CallStackFrame> stack;

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
	/// \param path
	///	Name of the ELF binary from which the region is taken.
	///
	/// \param offset
	///	Position in the source file where the mapped region starts.
	///
	/// \param address
	///	Virtual memory address where the region is loaded.
	///
	/// \param size
	///	Size of the region in bytes.
	///
	/// \param dynamic
	///	Flag indicating whether this map is made from the initial
	///	static program loader, or from an `mmap` system call at runtime.
	void Map(const std::string &path,
			unsigned offset,
			unsigned address,
			unsigned size,
			bool dynamic);

	/// Record a function call
	void Call(unsigned ip, unsigned sp);

	/// Record a function return
	void Return(unsigned ip, unsigned sp);
	
	/// Dump stack back trace
	///
	/// \param address
	///	Current executing instruction
	///
	/// \param os
	///	Output stream to dump the back trace to
	void BackTrace(unsigned address, std::ostream &os = std::cout);

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

