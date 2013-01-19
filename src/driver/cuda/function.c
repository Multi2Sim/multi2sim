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
#include "object.h"


struct frm_cuda_function_t *frm_cuda_function_create(void)
{
	struct frm_cuda_function_t *function;

	/* Initialize */
	function = xcalloc(1, sizeof(struct frm_cuda_function_t));
	function->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_FUNCTION);
	function->ref_count = 1;
	function->arg_list = list_create();

	/* Return */
	frm_cuda_object_add(function);
	return function;
}


void frm_cuda_function_free(struct frm_cuda_function_t *function)
{
	int i;

	/* Free arguments */
	for (i = 0; i < list_count(function->arg_list); i++)
		frm_cuda_function_arg_free((struct frm_cuda_function_arg_t *)list_get(function->arg_list, i));
	list_free(function->arg_list);

	/* FIXME: free ELF file */

	/* Free function */
	frm_cuda_object_remove(function);
	free(function);
}

