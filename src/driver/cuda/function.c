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

#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>

#include "function.h"
#include "function-arg.h"
#include "module.h"


struct list_t *function_list;

struct cuda_function_t *cuda_function_create(struct cuda_module_t *module,
	char *function_name, unsigned long long int *inst_buffer, 
	unsigned int inst_buffer_size, unsigned int num_gpr_used)
{
	struct cuda_function_t *function;

	/* Initialize */
	function = xcalloc(1, sizeof(struct cuda_function_t));
	function->id = list_count(function_list);
	function->name = xstrdup(function_name);
	function->ref_count = 1;
	function->module_id = module->id;
	function->inst_buffer = (unsigned long long int *)xcalloc(1,
			(unsigned long int)inst_buffer_size);
	memcpy(function->inst_buffer, inst_buffer, inst_buffer_size);
	function->inst_buffer_size = inst_buffer_size;
	function->num_gpr_used = num_gpr_used;
	function->arg_list = list_create();

	list_add(function_list, function);

	return function;
}


void cuda_function_free(struct cuda_function_t *function)
{
	int i;

	for (i = 0; i < list_count(function->arg_list); i++)
		cuda_function_arg_free((struct cuda_function_arg_t *)list_get(function->arg_list, i));
	list_free(function->arg_list);
	free(function->inst_buffer);
	free(function->name);

	free(function);
}

