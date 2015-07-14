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

#ifndef ARCH_HSA_EMU_HSAEXECUTABLE_H
#define ARCH_HSA_EMU_HSAEXECUTABLE_H

#include "HsaProgram.h"

#include <arch/hsa/disassembler/BrigFile.h>


namespace HSA
{
class HsaProgram;
class BrigFile;
class Function;
class BrigCodeEntry;
class HsaExecutableSymbol;

/**
 * An HsaExecutable is a finished executable
 */
class HsaExecutable
{
private:

	// Modules in the HSA executable
	std::vector<std::unique_ptr<BrigFile>> modules;

	// Function table for the functions in the brig file
	std::map<std::string, std::unique_ptr<Function>> function_table;

	// Load functions in the brig file.
	//
	// \return
	// 	Number of functions loaded
	unsigned int loadFunctions(BrigFile *file);

	// Parse and create a function object
	void parseFunction(BrigFile *file, std::unique_ptr<BrigCodeEntry> dir);

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
	void preprocessRegisters(BrigFile *binary,
			std::unique_ptr<BrigCodeEntry> first_entry,
			std::unique_ptr<BrigCodeEntry> next_module_entry,
			Function* function);

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
	//
	std::unique_ptr<BrigCodeEntry> loadArguments(
			BrigFile *file,
			unsigned short num_arg,
			std::unique_ptr<BrigCodeEntry> entry,
			bool isInput, Function* function);

public:

	/// Constructor
	HsaExecutable();

	/// Load code object
	void LoadCodeObject(HsaCodeObject *code_object);

	/// Add a module to the module list
	void AddModule(const char *module);

	/// Get the symbol name
	HsaExecutableSymbol *getSymbol(const char *symbol_name);

	/// Return the pointer to the function by the name
	Function *getFunction(const std::string &name) const;
};

}  // namespace HSA

#endif
