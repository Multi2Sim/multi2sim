/*
d *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_KEPLER_DRIVER_FUNCTION_H
#define ARCH_KEPLER_DRIVER_FUNCTION_H

#include <cassert>
#include <memory>
#include <vector>

#include <lib/cpp/Misc.h>

#include "Argument.h"
//#include "Module.h"


namespace Kepler
{

class Module;

class Function
{
	// Function unique identifier
	int id;

	// Module that the function belongs to
	Module *module;

	// Name
	std::string name;

	// Pointer to a region of the ELF file in the associated module
	// containing the ISA section for the function. FIXME
	const char *text_buffer;
	
	// Size of the ISA section in bytes FIXME
	int text_size;

	// Max threads allowed per block
	int max_threads_per_block;

	// Shared memory used by function in Bytes
	unsigned shared_memory_size;

	// Constant memory used by function in Bytes
	unsigned constant_memory_size;

	// Local memory used by function in Bytes
	unsigned local_memory_size;

	// Number of barriers in the function
	unsigned num_barriers;

	// Number of register per thread
	unsigned num_registers_per_thread;

	// Arguments
	std::vector<std::unique_ptr<Argument>> arguments;

public:

	/// Constructor
	Function(int id, Module *module, const std::string &name);

	/// Get function id
	int getId() const { return id; }

	/// Get the size of the ISA section in the associated ELF binary
	int getTextSize() const { return text_size; }

	/// Get the size of shared memory in bytes used in the function
	unsigned getSharedMemorySize() const { return shared_memory_size; }

	/// Get the size of local memory in bytes used in the function
	unsigned getLocalMemorySize() const { return local_memory_size; }

	/// Get the size of constant memory in bytes used in the function
	unsigned getConstantMemorySize() const { return constant_memory_size; }

	/// Get the number of registers used in the function
	unsigned getNumRegistersPerThread() const
	{
		return num_registers_per_thread;
	}

	/// Get the number of barriers in the function
	unsigned getNumBarriers() const { return num_barriers; }

	/// Get a buffer pointing to the ISA section in the associated ELF file
	const char *getTextBuffer() const { return text_buffer; }

	/// Get number of arguments
	int getNumArguments() const { return arguments.size(); }

	/// Get the arguments with the given index
	Argument *getArgument(int index)
	{
		assert(misc::inRange((unsigned) index, 0,
				arguments.size() - 1));
		return arguments[index].get();
	}

	/// Get function name
	std::string getName() const { return name; }
};


} // namespace Kepler

#endif
