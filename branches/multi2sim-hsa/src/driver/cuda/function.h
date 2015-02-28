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

#ifndef RUNTIME_CUDA_FUNCTION_H
#define RUNTIME_CUDA_FUNCTION_H


#include "module.h"


extern struct list_t *function_list;

struct cuda_function_t
{
	/* ID */
	int id;

	/* Name */
	char *name;

	/* Module ID */
	unsigned module_id;

	/* Instruction binary */
	int inst_bin_size;
	unsigned long long *inst_bin;

	/* Number of GPRs used by one thread */
	int num_gpr;

	/* Arguments */
	int arg_count;
	struct cuda_function_arg_t **arg_array;
};

struct cuda_function_t *cuda_function_create(struct cuda_module_t *module, 
		char *func_name);
void cuda_function_free(struct cuda_function_t *function);


#endif

