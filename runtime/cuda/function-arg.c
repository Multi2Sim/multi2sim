/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../include/cuda.h"
#include "function-arg.h"
#include "function.h"
#include "list.h"
#include "mhandle.h"


struct cuda_function_arg_t *cuda_function_arg_create(CUfunction function,
		unsigned int value, int size)
{
	struct cuda_function_arg_t *arg;

	/* Initialize */
	arg = (struct cuda_function_arg_t *)xcalloc(1, 
			sizeof(struct cuda_function_arg_t));
	arg->id = list_count(function->arg_list);
	arg->value = value;
	arg->size = size;

	list_add(function->arg_list, arg);

	return arg;
}

void cuda_function_arg_free(CUfunction function, struct cuda_function_arg_t *arg)
{
	list_remove(function->arg_list, arg);

	free(arg);
}

