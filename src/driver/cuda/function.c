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




struct cuda_function_t *cuda_function_create(struct cuda_module_t *module,
	char *function_name)
{
	struct cuda_function_t *function;
	char section_name[MAX_STRING_SIZE];
	struct elf_section_t *section;
	int i;

	/* Initialize */
	function = xcalloc(1, sizeof(struct cuda_function_t));
	function->id = cuda_object_new_id(CUDA_OBJ_FUNCTION);
	function->ref_count = 1;
	snprintf(function->name, MAX_STRING_SIZE, "%s", function_name);
	function->module_id = module->id;
	function->arg_list = list_create();

	/* Load function */
	snprintf(section_name, MAX_STRING_SIZE, ".text.%s", function_name);
	for (i = 0; i < list_count(module->elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *)list_get(module->elf_file->section_list, i);
		if (!strncmp(section->name, section_name, MAX_STRING_SIZE))
			break;
	}
	if (i == list_count(module->elf_file->section_list))
		fatal("%s section not found!\n", section_name);
	function->function_buffer.ptr = section->buffer.ptr;
	function->function_buffer.size = section->buffer.size;

	cuda_object_add(function);

	return function;
}


void cuda_function_free(struct cuda_function_t *function)
{
	int i;

	/* Free arguments */
	for (i = 0; i < list_count(function->arg_list); i++)
		cuda_function_arg_free((struct cuda_function_arg_t *)list_get(function->arg_list, i));
	list_free(function->arg_list);

	/* FIXME: free ELF file */

	cuda_object_remove(function);

	free(function);
}

