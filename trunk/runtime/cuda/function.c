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

#include "function.h"




struct list_t *function_list;

struct CUfunc_st *cuda_function_create(struct CUmod_st *module,
	char *function_name)
{
	struct CUfunc_st *function;
	int section_name_len;
	char *section_name;
	struct elf_section_t *section;
	int i;

	/* Initialize */
	function = (struct CUfunc_st *)xcalloc(1, sizeof(struct CUfunc_st));
	function->id = list_count(function_list) - 1;
	function->name = xstrdup(function_name);
	function->ref_count = 1;
	function->module_id = module->id;
	function->arg_list = list_create();

	/* Load function */
	section_name_len = strlen(function_name) + 7;
	section_name = (char *)xmalloc(section_name_len);
	snprintf(section_name, section_name_len, ".text.%s", function_name);
	for (i = 0; i < list_count(module->elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *)list_get(module->elf_file->section_list, i);
		if (!strncmp(section->name, section_name, section_name_len))
			break;
	}
	if (i == list_count(module->elf_file->section_list))
		fatal("%s section not found!\n", section_name);
	free(section_name);
	function->function_buffer.ptr = section->buffer.ptr;
	function->function_buffer.size = section->buffer.size;

	return function;
}


void cuda_function_free(struct CUfunc_st *function)
{
	int i;

	free(function->name);

	/* Free arguments */
	for (i = 0; i < list_count(function->arg_list); i++)
		cuda_function_arg_free((struct cuda_function_arg_t *)list_get(function->arg_list, i));
	list_free(function->arg_list);

	/* FIXME: free ELF file */

	free(function);
}

