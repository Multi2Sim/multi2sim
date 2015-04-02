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

#ifndef ARCH_HSA_EMU_PROGRAMLOADER_H
#define ARCH_HSA_EMU_PROGRAMLOADER_H

#include <string>
#include <memory>

#include <arch/hsa/asm/BrigFile.h>
#include <arch/common/FileTable.h>

#include "Function.h"


namespace HSA
{

/// ProgramLoader is a singleton that loads the the program to be emulated
class ProgramLoader
{
	// Unique instance of hsa program loader
	static std::unique_ptr<ProgramLoader> instance;

	// Private constructor
	ProgramLoader()
	{};

	// Brig file to be executed.
	std::unique_ptr<BrigFile> binary;

	// Command-line arguments
	std::vector<std::string> args;

	// Executable file name
	std::string exe;

	// Current working directory
	std::string cwd;

	// File name for standard input and output
	std::string stdin_file_name;
	std::string stdout_file_name;

	// File descriptor table
	std::shared_ptr<comm::FileTable> file_table;



public:

	/// Load the program to be executed. Unlike a typical singleton, it can
	/// only be initialized with \c LoadProgram function. The meaning of
	/// each argument is identical to the prototype of comm::Emu::Load()
	static void LoadProgram(const std::vector<std::string> &args,
 			const std::vector<std::string> &env,
 			const std::string &cwd,
 			const std::string &stdin_file_name,
 			const std::string &stdout_file_name);


	/// Returns the pointer to the instance, or throw a \c Panic if no
	/// program has been loaded
	static ProgramLoader *getInstance();

	/// Determine if the HSA program loader has loaded a program.
	static bool isLoaded();

	/// Load binary
	void LoadBinary();

	/// Return pointer to file table
	comm::FileTable *getFileTable() const { return file_table.get(); };

};

}  // namespace HSA

#endif

