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
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "function.h"
#include "function-arg.h"


struct list_t *function_list;

struct cuda_function_t *cuda_function_create(struct cuda_module_t *module,
	char *func_name)
{
	struct cuda_function_t *function;

	struct elf_file_t *cubin;
	struct elf_section_t *sec;
	struct elf_section_t *func_text_sec;
	struct elf_section_t *func_info_sec;
	char func_text_sec_name[MAX_STRING_SIZE];
	char func_info_sec_name[MAX_STRING_SIZE];
	unsigned char inst_bin_byte;
	char arg_name[MAX_STRING_SIZE];
	int i;

	/* Allocate */
	function = xcalloc(1, sizeof(struct cuda_function_t));

	/* Initialization */
	function->id = list_count(function_list);
	function->name = xstrdup(func_name);
	function->module_id = module->id;

	/* Get cubin */
	cubin = module->elf_file;

	/* Get .text.kernel_name section */
	snprintf(func_text_sec_name, sizeof func_text_sec_name, ".text.%s",
			func_name);
	for (i = 0; i < list_count(cubin->section_list); ++i)
	{
		sec = list_get(cubin->section_list, i);
		if (!strncmp(sec->name, func_text_sec_name, strlen(func_text_sec_name)))
		{
			func_text_sec = list_get(cubin->section_list, i);
			break;
		}
	}
	assert(i < list_count(cubin->section_list));

	/* Get instruction binary */
	function->inst_bin_size = func_text_sec->header->sh_size;
	function->inst_bin = xcalloc(1, function->inst_bin_size);
	for (i = 0; i < function->inst_bin_size; ++i)
	{
		elf_buffer_seek(&(cubin->buffer), func_text_sec->header->sh_offset + i);
		elf_buffer_read(&(cubin->buffer), &inst_bin_byte, 1);
		if (i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3)
		{
			function->inst_bin[i / 8] |= (unsigned long long int)(inst_bin_byte)
					<< (i * 8 + 32);
		}
		else
		{
			function->inst_bin[i / 8] |= (unsigned long long int)(inst_bin_byte)
					<< (i * 8 - 32);
		}
	}

	/* Get GPR usage */
	function->num_gpr = func_text_sec->header->sh_info >> 24;

	/* Get .nv.info.kernel_name section */
	snprintf(func_info_sec_name, MAX_STRING_SIZE, ".nv.info.%s",
			function->name);
	for (i = 0; i < list_count(cubin->section_list); ++i)
	{
		sec = list_get(cubin->section_list, i);
		if (!strncmp(sec->name, func_info_sec_name, strlen(func_info_sec_name)))
		{
			func_info_sec = list_get(cubin->section_list, i);
			break;
		}
	}
	assert(i < list_count(cubin->section_list));

	/* Get the number of arguments */
	function->arg_count = ((unsigned char *)func_info_sec->buffer.ptr)[10] / 4;

	/* Create arguments */
	function->arg_array = xcalloc(function->arg_count, 
			sizeof(struct cuda_function_arg_t *));
	for (i = 0; i < function->arg_count; ++i)
	{
		snprintf(arg_name, MAX_STRING_SIZE, "arg_%d", i);
		function->arg_array[i] = cuda_function_arg_create(arg_name);
	}

	/* Add function to function list */
	list_add(function_list, function);

	return function;
}


void cuda_function_free(struct cuda_function_t *function)
{
	int i;

	for (i = 0; i < function->arg_count; ++i)
		cuda_function_arg_free(function->arg_array[i]);
	free(function->arg_array);
	free(function->inst_bin);
	free(function->name);

	free(function);
}

