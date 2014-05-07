/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu (gu.sid@husky.neu.edu)
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

#include <iostream>
#include <memory>
#include <mem-system/Memory.h>
#include <lib/cpp/ELFReader.h>
#include "Regs.h"

namespace mips
{
class Emu;

// mips Context
class Context
{
	// Emulator it belongs to
	Emu *emu;

	// Virtual memory address space index
	int address_space_index;

	// Context memory. This object can be shared by multiple contexts, so it
	// is declared as a shared pointer. The last freed context pointing to
	// this memory object will be the one automatically freeing it.
	std::shared_ptr<mem::Memory> memory;

	// Register file. Each context has its own copy always.
	Regs regs;

	// Load program image from disk to main memory
	struct Loader
	{
		// Program executable
		std::unique_ptr<ELFReader::File> binary;

		// Command-line arguments
		std::vector<std::string> args;

		// File name for standard input and output
		std::string stdin_file_name;
		std::string stdout_file_name;

		// Current working directory
		std::string cwd;
	};

	// Loader information. This information can be shared among multiple
	// contexts. For this reason, it is declared as a shared pointer. The
	// last destructed context sharing this variable will automatically free
	// it.
	std::shared_ptr<Loader> loader;

public:
	Context();
	~Context();

	/// Run one instruction for the context at the position pointed to by
	/// register program counter.
	void Execute();

	/// Load a program from a command line into an existing context.
	/// \param args
	///	Command line to be used, where the first argument contains the
	///	path to the executable ELF file.
	void Load(const std::vector<std::string> &args);

	// Load ELF binary, as already decoded in 'loader.binary'
	void LoadBinary();

	/// Given a file name, return its full path based on the current working
	/// directory for the context.
	std::string getFullPath(const std::string &path);



};
}
