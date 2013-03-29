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

#include "../include/cuda.h"
#include "elf-format.h"
#include "list.h"
#include "mhandle.h"
#include "module.h"


struct list_t *module_list;

/* Create a module */
CUmodule cuda_module_create(void)
{
	CUmodule module;

	/* Initialize */
	module = (CUmodule)xcalloc(1, sizeof(struct CUmod_st));
	module->id = list_count(module_list);
	module->ref_count = 1;

	list_add(module_list, module);

	return module;
}

/* Free module */
void cuda_module_free(CUmodule module)
{
	list_remove(module_list, module);

	module->ref_count--;
	free(module);
}

