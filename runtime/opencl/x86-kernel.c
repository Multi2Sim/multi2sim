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

#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "debug.h"
#include "mem.h"
#include "mhandle.h"
#include "x86-kernel.h"


#define MEMORY_ALIGN 16
#define MAX_SSE_REG_PARAMS 4
#define SSE_REG_SIZE_IN_WORDS (16 / sizeof (size_t))


/*
 * Private Functions
 */


static int opencl_x86_kernel_get_arg_type_size(enum opencl_x86_kernel_arg_type_t arg_type)
{
	switch (arg_type)
	{
	case OPENCL_X86_KERNEL_ARG_CHAR:
		return 1;

	case OPENCL_X86_KERNEL_ARG_SHORT:
	case OPENCL_X86_KERNEL_ARG_CHAR2:
		return 2;

	case OPENCL_X86_KERNEL_ARG_CHAR3:
		return 3;

	case OPENCL_X86_KERNEL_ARG_CHAR4:
	case OPENCL_X86_KERNEL_ARG_SHORT2:
	case OPENCL_X86_KERNEL_ARG_INT:
	case OPENCL_X86_KERNEL_ARG_FLOAT:
	case OPENCL_X86_KERNEL_ARG_POINTER:
		return 4;

	case OPENCL_X86_KERNEL_ARG_SHORT3:
		return 6;

	case OPENCL_X86_KERNEL_ARG_CHAR8:
	case OPENCL_X86_KERNEL_ARG_SHORT4:
	case OPENCL_X86_KERNEL_ARG_INT2:
	case OPENCL_X86_KERNEL_ARG_FLOAT2:
	case OPENCL_X86_KERNEL_ARG_LONG:
	case OPENCL_X86_KERNEL_ARG_DOUBLE:
		return 8;

	case OPENCL_X86_KERNEL_ARG_INT3:
	case OPENCL_X86_KERNEL_ARG_FLOAT3:
		return 12;

	case OPENCL_X86_KERNEL_ARG_CHAR16:
	case OPENCL_X86_KERNEL_ARG_SHORT8:
	case OPENCL_X86_KERNEL_ARG_INT4:
	case OPENCL_X86_KERNEL_ARG_FLOAT4:
	case OPENCL_X86_KERNEL_ARG_LONG2:
	case OPENCL_X86_KERNEL_ARG_DOUBLE2:
		return 16;

	case OPENCL_X86_KERNEL_ARG_LONG3:
	case OPENCL_X86_KERNEL_ARG_DOUBLE3:
		return 24;

	case OPENCL_X86_KERNEL_ARG_SHORT16:
	case OPENCL_X86_KERNEL_ARG_INT8:
	case OPENCL_X86_KERNEL_ARG_FLOAT8:
	case OPENCL_X86_KERNEL_ARG_LONG4:
	case OPENCL_X86_KERNEL_ARG_DOUBLE4:
		return 32;

	case OPENCL_X86_KERNEL_ARG_INT16:
	case OPENCL_X86_KERNEL_ARG_FLOAT16:
	case OPENCL_X86_KERNEL_ARG_LONG8:
	case OPENCL_X86_KERNEL_ARG_DOUBLE8:
		return 64;

	case OPENCL_X86_KERNEL_ARG_LONG16:
	case OPENCL_X86_KERNEL_ARG_DOUBLE16:
		return 128;

	default:
		panic("%s: invalid argument type", __FUNCTION__);
		return 0;
	}

}


static int opencl_x86_kernel_get_arg_words(enum opencl_x86_kernel_arg_type_t arg_type)
{
	int size;
	int rem;

	/* Round up size to the nearest sizeof(size_t) bytes. */
	size = opencl_x86_kernel_get_arg_type_size(arg_type);
	rem = size % sizeof (size_t);
	if (rem)
		rem = sizeof (size_t) - rem;
	return (size + rem) / sizeof (size_t);
}


static int opencl_x86_kernel_arg_is_vector(enum opencl_x86_kernel_arg_type_t arg_type)
{
	switch (arg_type)
	{
	case OPENCL_X86_KERNEL_ARG_CHAR:
	case OPENCL_X86_KERNEL_ARG_SHORT:
	case OPENCL_X86_KERNEL_ARG_INT:
	case OPENCL_X86_KERNEL_ARG_LONG:
	case OPENCL_X86_KERNEL_ARG_POINTER:
	case OPENCL_X86_KERNEL_ARG_FLOAT:
	case OPENCL_X86_KERNEL_ARG_DOUBLE:
		return 0;
	default:
		return 1;
	}
}


static void *opencl_x86_kernel_get_func_info(void *dlhandle,
		const char *func_name, size_t **metadata)
{
	void *addr;
	char *full_name;
	char *meta_name;

	full_name = xmalloc(strlen(func_name) + 100);
	meta_name = xmalloc(strlen(func_name) + 100);
	sprintf(full_name, "__OpenCL_%s_kernel", func_name);
	sprintf(meta_name, "__OpenCL_%s_metadata", func_name);

	*metadata = (size_t *) dlsym(dlhandle, meta_name);
	addr = dlsym(dlhandle, full_name);

	free(full_name);
	free(meta_name);
	return addr;
}




/*
 * Public Functions
 */


struct opencl_x86_kernel_t *opencl_x86_kernel_create(void *dlhandle,
		char *func_name, cl_int *err_ptr)
{
	int i;
	int stride;
	int num_reg;
	int stack_offset;
	int remainder;

	struct opencl_x86_kernel_t *kernel;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_x86_kernel_t));
	kernel->func = opencl_x86_kernel_get_func_info(dlhandle, func_name, &kernel->metadata);

	/* Check valid kernel function name */
	if (!kernel->func)
	{
		if (err_ptr)
			*err_ptr = CL_INVALID_KERNEL_NAME;
		free(kernel);
		return NULL;
	}

	kernel->local_reserved_bytes = kernel->metadata[1];
	kernel->num_params = (kernel->metadata[0] - 44) / 24;
	kernel->param_info = xcalloc(1, sizeof kernel->param_info[0] * kernel->num_params);
	if (posix_memalign((void **) &kernel->register_params, MEMORY_ALIGN,
			sizeof kernel->register_params[0] * MAX_SSE_REG_PARAMS))
		fatal("%s: could not allocate aligned memory", __FUNCTION__);

	stride = 8;
	num_reg = 0;
	stack_offset = 0;

	for (i = 0; i < kernel->num_params; i++)
	{
		struct opencl_x86_kernel_arg_t *param_info;

		param_info = kernel->param_info + i;
		param_info->arg_type = kernel->metadata[8 + stride * i];
		param_info->mem_arg_type = kernel->metadata[8  + stride * i + 1];
		param_info->is_set = 0;
		param_info->size = opencl_x86_kernel_get_arg_words(kernel->param_info[i].arg_type);
		param_info->is_stack = 1;

		if (opencl_x86_kernel_arg_is_vector(param_info->arg_type) &&
				param_info->size <= (MAX_SSE_REG_PARAMS - num_reg)
				* SSE_REG_SIZE_IN_WORDS)
		{
			param_info->is_stack = 0;
			param_info->reg_offset = num_reg;
			num_reg += param_info->size / SSE_REG_SIZE_IN_WORDS;

			/* for double3, long3.	*/
			if (param_info->size % SSE_REG_SIZE_IN_WORDS != 0)
				num_reg++;
		}
		else
		{
			int align_size;
			int remainder;

			align_size = param_info->size;
			if (align_size > SSE_REG_SIZE_IN_WORDS)
				align_size = SSE_REG_SIZE_IN_WORDS;

			remainder = stack_offset % align_size;
			if (remainder)
				stack_offset += align_size - remainder;

			param_info->stack_offset = stack_offset;
			stack_offset += param_info->size;
		}

		/**
		printf("Param %d.  Type: %d.  Mem: %d. Size: %d. Offset: %d.\n",
                       i,
                       kernel->param_info[i].param_type,
                       kernel->param_info[i].mem_type,
		       kernel->param_info[i].size,
		       kernel->param_info[i].offset);
		*/
	}

	remainder = stack_offset % SSE_REG_SIZE_IN_WORDS;
	if (!remainder)
		kernel->stack_param_words = stack_offset;
	else
		kernel->stack_param_words = stack_offset + SSE_REG_SIZE_IN_WORDS - remainder;

	kernel->stack_params = xmalloc(sizeof (size_t) * kernel->stack_param_words);
	memset(kernel->stack_params, 0, sizeof (size_t) * kernel->stack_param_words);
	return kernel;
}


void opencl_x86_kernel_free(struct opencl_x86_kernel_t *kernel)
{
	free(kernel->param_info);
	free(kernel->stack_params);
	free(kernel->register_params);
	free(kernel);
}


cl_int opencl_x86_kernel_check(struct opencl_x86_kernel_t *kernel)
{
	int i;

	/* Check that all arguments are set */
	for (i = 0; i < kernel->num_params; i++)
		if (!kernel->param_info[i].is_set)
			return CL_INVALID_VALUE;

	/* Success */
	return CL_SUCCESS;
}


cl_int opencl_x86_kernel_set_arg(struct opencl_x86_kernel_t *kernel,
		cl_uint arg_index, size_t arg_size, const void *arg_value)
{
	struct opencl_x86_kernel_arg_t *param_info;

	assert(arg_index >= 0 && arg_index < kernel->num_params);
	param_info = kernel->param_info + arg_index;
	assert(param_info->size * sizeof(size_t) >= arg_size || !arg_value);

	/* Empty value only allowed for local memory */
	assert((!arg_value) == (param_info->mem_arg_type
			== OPENCL_X86_KERNEL_MEM_ARG_LOCAL));

	/* Different actions for each argument type */
	if (!arg_value)
	{
		/* Local memory */
		kernel->stack_params[param_info->stack_offset] = arg_size;
	}
	else if (param_info->mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_GLOBAL
			|| param_info->mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_CONSTANT)
	{
		void *addr = opencl_mem_get_buffer(*(cl_mem *) arg_value);
		if (!addr)
			return CL_INVALID_MEM_OBJECT;
		memcpy(kernel->stack_params + param_info->stack_offset, &addr, sizeof addr);
	}
	else if (param_info->is_stack)
	{
		memcpy(kernel->stack_params + param_info->stack_offset, arg_value, arg_size);
	}
	else
	{
		memcpy(kernel->register_params + param_info->reg_offset, arg_value, arg_size);
	}

	/* Label argument as set */
	kernel->param_info[arg_index].is_set = 1;
	return 0;
}
