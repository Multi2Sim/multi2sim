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

#ifndef DRIVER_CUDA_FUNCTION_ARG_H
#define DRIVER_CUDA_FUNCTION_ARG_H

enum cuda_mem_scope_t
{
	CUDA_MEM_SCOPE_NONE = 0,
	CUDA_MEM_SCOPE_GLOBAL,
	CUDA_MEM_SCOPE_LOCAL,
	CUDA_MEM_SCOPE_PRIVATE,
	CUDA_MEM_SCOPE_CONSTANT
};

enum cuda_function_arg_kind_t
{
	CUDA_FUNCTION_ARG_KIND_VALUE = 1,
	CUDA_FUNCTION_ARG_KIND_POINTER
};

enum cuda_function_arg_access_type_t
{
	CUDA_FUNCTION_ARG_READ_ONLY = 1,
	CUDA_FUNCTION_ARG_WRITE_ONLY,
	CUDA_FUNCTION_ARG_READ_WRITE
};

struct cuda_function_arg_t
{
	int id;
	char *name;

	enum cuda_function_arg_kind_t kind;
	enum cuda_mem_scope_t mem_scope;
	enum cuda_function_arg_access_type_t access_type;

	unsigned int value;
};

struct cuda_function_arg_t *cuda_function_arg_create(char *name);
void cuda_function_arg_free(struct cuda_function_arg_t *arg);

#endif

