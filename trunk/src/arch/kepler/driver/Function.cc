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

#include <lib/mhandle/mhandle.h>
extern "C"
{
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
}
#include <lib/util/string.h>

#include "Function.h"
#include "Function-arg.h"


namespace Kepler
{

std::vector<CUfunction*> CUfunction::function_list;

CUfunction::CUfunction(CUmodule *module, char *function_name)
{
	// Initialization
	this->id = function_list.size();
	this->name.reset(function_name);
	this->module_id = module->getID();

	// Get cubin
	struct elf_file_t *cubin;
	cubin = module->getELF();

	// Get .text.kernel_name section
	struct elf_section_t *func_info_sec;
	struct elf_section_t *func_text_sec;
	struct elf_section_t *sec;
	char func_text_sec_name[MAX_STRING_SIZE];
	snprintf(func_text_sec_name, sizeof func_text_sec_name, ".text.%s",
			function_name);
	int i = 0;
	for (i = 0; i < list_count(cubin->section_list); ++i)
	{
		sec = (elf_section_t*)list_get(cubin->section_list, i);
		if (!strncmp(sec->name, func_text_sec_name, strlen(func_text_sec_name)))
		{
			func_text_sec = (elf_section_t*)list_get(cubin->section_list, i);
			break;
		}
	}
	assert(i < list_count(cubin->section_list));

	// Get instruction binary
	this->inst_bin_size = func_text_sec->header->sh_size;
	//this->inst_buffer = (unsigned long long *)xcalloc(1, this->inst_bin_size);  complier error!!!!
	this->inst_buffer= (unsigned long long *)new char[inst_bin_size];
	unsigned char inst_bin_byte;
	for (i = 0; i < this->inst_bin_size; ++i)
	{
		elf_buffer_seek(&(cubin->buffer), func_text_sec->header->sh_offset + i);
		elf_buffer_read(&(cubin->buffer), &inst_bin_byte, 1);
		if (i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3)
		{
			this->inst_buffer[i / 8] |= (unsigned long long int)(inst_bin_byte)
					<< (i * 8 + 32);
		}
		else
		{
			this->inst_buffer[i / 8] |= (unsigned long long int)(inst_bin_byte)
					<< (i * 8 - 32);
		}
	}

	// Get GPR usage
	this->num_gpr = func_text_sec->header->sh_info >> 24;

	// Get .nv.info.kernel_name section
	//char *temp_name;
	char func_info_sec_name[MAX_STRING_SIZE];
	snprintf(func_info_sec_name, MAX_STRING_SIZE, ".nv.info.%s",
			name.get());
	for (i = 0; i < list_count(cubin->section_list); ++i)
	{
		sec = (elf_section_t*)list_get(cubin->section_list, i);
		if (!strncmp(sec->name, func_info_sec_name, strlen(func_info_sec_name)))
		{
			func_info_sec = (elf_section_t*)list_get(cubin->section_list, i);
			break;
		}
	}
	assert(i < list_count(cubin->section_list));

	// Get the number of arguments
	this->setArgCount(((unsigned char *)func_info_sec->buffer.ptr)[10] / 4);

	// Create arguments
	//this->arg_array = (CUfunction**)xcalloc(this->arg_count, sizeof(CUfunctionarg*)); //???  Error::  invalid conversion from ‘void*’ to ‘Kepler::CUfunctionarg**’
	this->arg_array = new CUfunctionarg* [this->arg_count];
	char arg_name[MAX_STRING_SIZE];
	for (i = 0; i < this->getArgCount(); ++i)
	{
		snprintf(arg_name, MAX_STRING_SIZE, "arg_%d", i);
		this->arg_array[i] = new CUfunctionarg(arg_name);
	}

	// Add function to function list
	function_list.push_back(this);
}

CUfunction::~CUfunction()
{
	delete this->inst_buffer;
	delete this->arg_array;
}

} // namespace Kepler

