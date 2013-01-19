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

enum frm_cuda_mem_scope_t
{
	FRM_CUDA_MEM_SCOPE_NONE = 0,
	FRM_CUDA_MEM_SCOPE_GLOBAL,
	FRM_CUDA_MEM_SCOPE_LOCAL,
	FRM_CUDA_MEM_SCOPE_PRIVATE,
	FRM_CUDA_MEM_SCOPE_CONSTANT
};

enum frm_cuda_function_arg_kind_t
{
	FRM_CUDA_FUNCTION_ARG_KIND_VALUE = 1,
	FRM_CUDA_FUNCTION_ARG_KIND_POINTER
};

enum frm_cuda_function_arg_access_type_t
{
	FRM_CUDA_FUNCTION_ARG_READ_ONLY = 1,
	FRM_CUDA_FUNCTION_ARG_WRITE_ONLY,
	FRM_CUDA_FUNCTION_ARG_READ_WRITE
};

struct frm_cuda_function_arg_t
{
	int id;
	char *name;

	enum frm_cuda_function_arg_kind_t kind;
	enum frm_cuda_mem_scope_t mem_scope;
	enum frm_cuda_function_arg_access_type_t access_type;

	unsigned int value;
};

struct frm_cuda_function_arg_t *frm_cuda_function_arg_create(char *name);
void frm_cuda_function_arg_free(struct frm_cuda_function_arg_t *arg);


#endif

