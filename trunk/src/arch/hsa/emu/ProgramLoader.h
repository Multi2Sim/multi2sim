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

#include <string>
#include <memory>

#include <arch/hsa/asm/BrigFile.h>
#include <arch/hsa/asm/BrigDirEntry.h>
#include <arch/common/FileTable.h>

#include "Emu.h"
#include "Function.h"

#ifndef ARCH_HSA_EMU_PROGRAMLOADER_H
#define ARCH_HSA_EMU_PROGRAMLOADER_H

namespace HSA
{

/// Abstract the program to emulate
class ProgramLoader
{

	// Unique instance of hsa program loader
	static std::unique_ptr<ProgramLoader> instance;

	// Private constructor
	ProgramLoader(){};

	// Brig file to be executed.
	// TODO extend this field to support multi-file program
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

	// Function table for the functions in the brig file
	std::map<std::string, std::unique_ptr<Function>>
			function_table;

	// File descriptor table
	std::shared_ptr<comm::FileTable> file_table;

	// Load and parse Brig ELF binary
	void LoadBinary();

	// Load functions in the brig table. Prepare the function table in
	// loader.
	// \return
	// 	Number of functions loaded
	unsigned int loadFunctions();

	// Parse and create a function object
	void parseFunction(BrigDirEntry *dir);

	// Load output arguments for a function
	//
	// \param num_out_arg
	// 	Number of output arguments
	//
	// \param next_dir
	// 	Pointer to the argument to start with
	//
	// \param isInput
	//	if true, add input arguments. Otherwise, add output arguments
	//
	// \param function
	// 	Pointer to the function to load arguments
	//
	// \return
	// 	Pointer to next directive to parse
	char *loadArguments(unsigned short num_arg, char *next_dir,
			bool isInput, Function* function);

	// Preprocess register allocation in a function
	//
	// \param entry_point
	// 	Pointer to first instruction to parse
	//
	// \param inst_count
	// 	Number of instructions in the function
	//
	// \param function
	// 	Pointer to the function to process
	void preprocessRegisters(char *entry_point,
			unsigned int inst_count, Function* function);

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

	/// Returns the Brig binary
	BrigFile *getBinary(){return binary.get();}

};

}  // namespace HSA

#endif

