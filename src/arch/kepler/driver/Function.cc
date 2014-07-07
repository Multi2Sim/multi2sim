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

#include <lib/cpp/ELFReader.h>
#include <lib/util/string.h>

#include "Function.h"
#include "Function-arg.h"


namespace Kepler
{

std::vector<Function*> Function::function_list;

Function::Function(Module *module, char *function_name)
{
	// Initialization
	this->id = function_list.size();
	this->name.reset(function_name);
	this->module_id = module->getID();

	// Get cubin
	ELFReader::File *cubin;
	cubin = module->getELF();

	// Get .text.kernel_name section
	ELFReader::Section *function_text_section;
	ELFReader::Section *sec;
	char func_text_sec_name[MAX_STRING_SIZE];
	snprintf(func_text_sec_name, sizeof func_text_sec_name, ".text.%s",
			function_name);
	std::string section_name (func_text_sec_name);
	int i = 0;
	for (i = 0; i < cubin->getNumSections(); ++i)
	{
		sec = cubin->getSection(i);
		std::string name = sec->getName();
		int flag = 0;
		for(unsigned j = 0; j < section_name.length(); ++j)
		{
			if( name.at(j) != section_name.at(j) )
			{
				flag = 1;
				break;
			}
		}

		if(!flag)
			function_text_section = cubin->getSection(i);
	}
	assert(i < cubin->getNumSections());

	// Get instruction binary
	this->inst_bin_size = function_text_section->getEntSize();
	this->inst_buffer= (unsigned long long *)new char[inst_bin_size];
	char *inst_bin_byte = new char [cubin->getSize()];
	strcpy(inst_bin_byte, cubin->getBuffer());
	for (i = 0; i < this->inst_bin_size; ++i)
	{
		if (i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3)
		{
			this->inst_buffer[i / 8] |=
						(unsigned long long int)(*(inst_bin_byte+i)) << (i * 8 + 32);
		}
		else
		{
			this->inst_buffer[i / 8] |=
						(unsigned long long int)(*(inst_bin_byte+i)) << (i * 8 - 32);
		}
	}

	// Get GPR usage         !!!!!!!!!!!!  need to be done!!!!
	//this->num_gpr = function_text_section->header->sh_info >> 24;
	//this->num_gpr = function_text_section

	// Get .nv.info.kernel_name section
	ELFReader::Section *function_info_section;
	char func_info_sec_name[MAX_STRING_SIZE];
	snprintf(func_info_sec_name, MAX_STRING_SIZE, ".nv.info.%s",
			name.get());
	for (i = 0; i < cubin->getNumSections(); ++i)
	{
		std::string name = sec->getName();
		int flag = 0;
		for(unsigned j = 0; j < section_name.length(); ++j)
		{
			if( name.at(j) != section_name.at(j) )
			{
				flag = 1;
				break;
			}
		}

		if(!flag)
			function_info_section = cubin->getSection(i);

	}
	assert(i < cubin->getNumSections());

	// Get the number of arguments
	this->setArgCount(((unsigned char *)
				function_info_section->getBuffer())[10] / 4);

	// Create arguments
	this->arg_array = new Argument* [this->arg_count];
	char arg_name[MAX_STRING_SIZE];
	for (i = 0; i < this->getArgCount(); ++i)
	{
		snprintf(arg_name, MAX_STRING_SIZE, "arg_%d", i);
		this->arg_array[i] = new Argument(arg_name);
	}

	// Add function to function list
	function_list.push_back(this);
}

Function::~Function()
{
	delete this->inst_buffer;
	delete this->arg_array;
}

} // namespace Kepler

