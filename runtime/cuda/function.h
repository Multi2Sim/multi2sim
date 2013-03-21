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

#include <stdlib.h>

#include "debug.h"
#include "elf-format.h"
#include "list.h"
#include "mhandle.h"
#include "string.h"

#include "function-arg.h"
#include "module.h"




extern struct list_t *function_list;

struct CUfunc_st
{
	int id;
	int ref_count;

	char *name;

	unsigned int module_id;
	struct list_t *arg_list;

	/* FIXME */
	struct elf_buffer_t function_buffer;
};

struct CUfunc_st *cuda_function_create(struct CUmod_st *module, 
	char *function_name);
void cuda_function_free(struct CUfunc_st *function);

#endif

