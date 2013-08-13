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

#include "module.h"


struct list_t *module_list;

/* Create a module */
struct cuda_module_t *cuda_module_create(char *cubin_path)
{
	struct cuda_module_t *module;

	/* Allocate */
	module = xcalloc(1, sizeof(struct cuda_module_t));

	/* ID */
	module->id = list_count(module_list);

	/* ELF */
	module->elf_file = elf_file_create_from_path(cubin_path);

	/* Add module to list */
	list_add(module_list, module);

	return module;
}

/* Free module */
void cuda_module_free(struct cuda_module_t *module)
{
	/* Free ELF */
	assert(module->elf_file);
	elf_file_free(module->elf_file);

	/* Free module */
	free(module);
}

