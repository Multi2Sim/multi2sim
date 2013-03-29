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

#include <stdio.h>
#include <string.h>

#include "../include/cuda.h"
#include "api.h"
#include "list.h"
#include "function-arg.h"
#include "function.h"
#include "mhandle.h"
#include "module.h"


struct list_t *function_list;

CUfunction cuda_function_create(CUmodule module, const char *function_name)
{
	CUfunction function;

	/* Initialize */
	function = (CUfunction)xcalloc(1, sizeof(struct CUfunc_st));
	function->id = list_count(function_list);
	function->ref_count = 1;
	function->name = xstrdup(function_name);
	function->module_id = module->id;
	function->inst_buffer = (unsigned long long int *)xcalloc(1,
			(unsigned long int)inst_buffer);
	memcpy(function->inst_buffer, inst_buffer, inst_buffer_size);
	function->inst_buffer_size = inst_buffer_size;
	function->arg_list = list_create();

	list_add(function_list, function);

	return function;
}

void cuda_function_free(CUfunction function)
{
	int i;

	list_remove(function_list, function);

	for (i = 0; i < list_count(function->arg_list); i++)
		cuda_function_arg_free(function, (struct cuda_function_arg_t *)list_get(function->arg_list, i));
	list_free(function->arg_list);
	free(function->name);
	function->ref_count--;
	free(function);
}

