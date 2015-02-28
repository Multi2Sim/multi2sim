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

	// Arguments
	std::vector<std::unique_ptr<Argument>> arguments;

public:

	/// Constructor
	Function(int id, Module *module, const std::string &name);

	/// Get function id
	int getId() const { return id; }

	/// Get the size of the ISA section in the associated ELF binary
	int getTextSize() const { return text_size; }

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
