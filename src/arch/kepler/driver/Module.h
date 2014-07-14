/*  Multi2Sim
 *  Copyright (C) 2014  Xun Gong (gong.xun@husky.neu.edu)
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

#ifndef ARCH_KEPLER_MODULE_H
#define ARCH_KEPLER_MODULE_H

#include <memory>
#include <vector>

#include <lib/cpp/ELFReader.h>

#include "Function.h"


namespace Kepler
{

class Function;

class Module
{
	// Unique identifier for the module
	int id;

	// Kernel binary
	ELFReader::File elf_file;

	// List of CUDA functions created by the guest application
	std::vector<std::unique_ptr<Function>> functions;



public:

	/// Constructor
	Module(int id, const std::string &cubin_path);

	/// Get the module's unique identifier
	unsigned getId() const { return id; }

	/// Get kernel binary
	ELFReader::File *getELFFile() { return &elf_file; }

	/// Return the number of available functions
	int getNumFunctions() { return functions.size(); }

	/// Return the function with the given identifier, or `nullptr` if the
	/// identifier does not correspond to a valid function.
	Function *getFunction(int index)
	{
		return misc::inRange((unsigned) index, 0, functions.size()) ?
				functions[index].get() :
				nullptr;
	}

	/// Create a new CUDA function and return a pointer to it.
	Function *addFunction(Module *module, const std::string &name);

	/// Get function name
	std::string getFunctionName(int index)
	{
		return functions[index]->getName();
	}
};

} // namespace Kepler

#endif
