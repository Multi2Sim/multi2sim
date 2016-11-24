/*
 *  Multi2Sim
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

#include <assert.h>
#include <cstring>
#include <sstream>

#include <lib/cpp/ELFReader.h>
#include <lib/cpp/String.h>

#include "Driver.h"
#include "Function.h"
#include "Module.h"


namespace Kepler
{

Function::Function(int id, Module *module, const std::string &name) :
		id(id),
		module(module),
		name(name)
{
	// Obtain cubin binary from associated module
	ELFReader::File *elf_file = module->getELFFile();

	// Get section named ".text.<name>" from the ELF file
	std::string text_section_name = ".text." + name;
	ELFReader::Section *text_section = elf_file->getSection(
			text_section_name);
	if (text_section == nullptr)
		throw Driver::Error(misc::fmt("Cannot find section '%s' in "
				"kernel binary", text_section_name.c_str()));

	// Get instruction binary
	text_buffer = text_section->getBuffer();
	text_size = text_section->getSize();

	// Get section named ".nv.info.<name>" from the ELF file
	std::string info_section_name = ".nv.info." + name;
	ELFReader::Section *info_section = elf_file->getSection(
			info_section_name);
	if (info_section == nullptr)
		throw Driver::Error(misc::fmt("Cannot find section '%s' in "
				"kernel binary", info_section_name.c_str()));

	// Get the number of arguments
	const char *info_buffer = info_section->getBuffer();
	int num_arguments = ((unsigned char *) (info_buffer))[10] / 4;

	// Create arguments
	for (int i = 0; i < num_arguments; i++)
	{
		std::string argument_name = misc::fmt("arg_%d", i);
		arguments.emplace_back(new Argument(argument_name));
	}
}
} // namespace Kepler

