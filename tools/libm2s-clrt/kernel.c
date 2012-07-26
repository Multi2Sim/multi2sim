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

#include <unistd.h>
#include <stdio.h>
#include <elf.h>
#include <assert.h>
#include <string.h>

#include <m2s-clrt.h>

#define MAX_SSE_REG_PARAMS 4
#define SSE_REG_SIZE_IN_WORDS (16 / sizeof (size_t))




/*
 * Private Functions 
 */

void clrt_kernel_free(void *data)
{
	struct _cl_kernel *kernel;

	kernel = (struct _cl_kernel *) data;
	free(kernel->param_info);
	free(kernel->stack_params);
	clrt_buffer_free(kernel->register_params);	
}


int clrt_type_size(enum clrt_param_type_t param_type)
{
	switch (param_type)
	{
	case CLRT_TYPE_CHAR:
		return 1;

	case CLRT_TYPE_SHORT:
	case CLRT_TYPE_CHAR2:
		return 2;

	case CLRT_TYPE_CHAR3:
		return 3;
	
	case CLRT_TYPE_CHAR4:
	case CLRT_TYPE_SHORT2:
	case CLRT_TYPE_INT:
	case CLRT_TYPE_FLOAT:
	case CLRT_TYPE_POINTER:
		return 4;

	case CLRT_TYPE_SHORT3:
		return 6;

	case CLRT_TYPE_CHAR8:
	case CLRT_TYPE_SHORT4:
	case CLRT_TYPE_INT2:
	case CLRT_TYPE_FLOAT2:
	case CLRT_TYPE_LONG:
	case CLRT_TYPE_DOUBLE:
		return 8;

	case CLRT_TYPE_INT3:
	case CLRT_TYPE_FLOAT3:
		return 12;

	case CLRT_TYPE_CHAR16:
	case CLRT_TYPE_SHORT8:
	case CLRT_TYPE_INT4:
	case CLRT_TYPE_FLOAT4:
	case CLRT_TYPE_LONG2:
	case CLRT_TYPE_DOUBLE2:
		return 16;

	case CLRT_TYPE_LONG3:
	case CLRT_TYPE_DOUBLE3:
		return 24;

	case CLRT_TYPE_SHORT16:
	case CLRT_TYPE_INT8:
	case CLRT_TYPE_FLOAT8:
	case CLRT_TYPE_LONG4:
	case CLRT_TYPE_DOUBLE4:
		return 32;

	case CLRT_TYPE_INT16:
	case CLRT_TYPE_FLOAT16:
	case CLRT_TYPE_LONG8:
	case CLRT_TYPE_DOUBLE8:
		return 64;

	case CLRT_TYPE_LONG16:
	case CLRT_TYPE_DOUBLE16:
		return 128;

	default:
		return 0;
	}

}


int stack_words(enum clrt_param_type_t param_type)
{
	int size;
	int rem;

	/* round up size to the nearest sizeof (size_t)-bytes. */
	size = clrt_type_size(param_type);
	rem = size % sizeof (size_t);
	if (rem != 0)
		rem = sizeof (size_t) - rem;
	return (size + rem) / sizeof (size_t);
}


int is_vector_type(enum clrt_param_type_t param_type)
{
	switch (param_type)
	{
	case CLRT_TYPE_CHAR:
	case CLRT_TYPE_SHORT:
	case CLRT_TYPE_INT:
	case CLRT_TYPE_LONG:
	case CLRT_TYPE_POINTER:
	case CLRT_TYPE_FLOAT:
	case CLRT_TYPE_DOUBLE:
		return 0;
	default:
		return 1;
	}
}




/*
 * Public Functions 
 */

cl_kernel clCreateKernel(
	cl_program program,
	const char *kernel_name,
	cl_int *errcode_ret)
{
	void *inner_elf;
	int i;
	int meta_size;
	int stride;
	int num_reg;
	int stack_offset;
	int remainder;
	struct _cl_kernel *kernel;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tprogram = %p", program);
	m2s_clrt_debug("\tkernel_name = %s", kernel_name);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	if (!clrt_object_verify(program, CLRT_OBJECT_PROGRAM))
	{
		if (errcode_ret != NULL)
			*errcode_ret = CL_INVALID_PROGRAM;
		return NULL;
	}

	kernel = (struct _cl_kernel *) malloc(sizeof (struct _cl_kernel));
	if (kernel == NULL)
		fatal("%s: out of memory", __FUNCTION__);

	clrt_object_create(kernel, CLRT_OBJECT_KERNEL, clrt_kernel_free);

	inner_elf = get_inner_elf_addr(program->elf_data);

	kernel->function = (clrt_function_t) get_function_info(inner_elf, kernel_name, &kernel->metadata, &meta_size);
	kernel->num_params = (kernel->metadata[0] - 44) / 24;
	kernel->param_info = (struct clrt_parameter_t *) malloc(sizeof (struct clrt_parameter_t) * kernel->num_params);
	if (kernel->param_info == NULL)
		fatal("%s: out of memory", __FUNCTION__);

	memset(kernel->param_info, 0, sizeof (struct clrt_parameter_t) * kernel->num_params);

	kernel->register_params = (struct clrt_reg_param_t *) clrt_buffer_allocate(sizeof (struct clrt_reg_param_t) * MAX_SSE_REG_PARAMS);
	/* in AMD APP SDK 2.5, the stride is 6, in 2.6, 2.7 the stride is (sometimes?) 8 */
	stride = 0;
	if (meta_size == kernel->metadata[0])
		stride = 6;
	else
		stride = 8;

	assert(meta_size > 4 * (8 + stride * (kernel->num_params - 1) + 1));

	num_reg = 0;
	stack_offset = 0;
	
	for (i = 0; i < kernel->num_params; i++)
	{
		struct clrt_parameter_t *param_info;

		param_info = kernel->param_info + i;
		param_info->param_type = kernel->metadata[8 + stride * i];
		param_info->mem_type = kernel->metadata[8  + stride * i + 1];
		param_info->is_set = 0;
		param_info->size = stack_words(kernel->param_info[i].param_type);
		param_info->is_stack = 1;

		if (is_vector_type(param_info->param_type) && param_info->size <= (MAX_SSE_REG_PARAMS - num_reg) * SSE_REG_SIZE_IN_WORDS)
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
			if (remainder != 0)
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
	if (remainder == 0)
		kernel->stack_param_words = stack_offset;
	else
		kernel->stack_param_words = stack_offset + SSE_REG_SIZE_IN_WORDS - remainder;

	kernel->stack_params = (size_t *) malloc(sizeof (size_t) * kernel->stack_param_words);
	if (kernel->stack_params == NULL)
		fatal("%s: out of memory", __FUNCTION__);

	memset(kernel->stack_params, 0, sizeof (size_t) * kernel->stack_param_words);
	return kernel;
}


cl_int clCreateKernelsInProgram(
	cl_program program,
	cl_uint num_kernels,
	cl_kernel *kernels,
	cl_uint *num_kernels_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clRetainKernel(
	cl_kernel kernel)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tkernel = %p", kernel);

	return clrt_object_retain(kernel, CLRT_OBJECT_KERNEL, CL_INVALID_KERNEL);
}


cl_int clReleaseKernel(
	cl_kernel kernel)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tkernel = %p", kernel);

	return clrt_object_release(kernel, CLRT_OBJECT_KERNEL, CL_INVALID_KERNEL);
}


cl_int clSetKernelArg(
	cl_kernel kernel,
	cl_uint arg_index,
	size_t arg_size,
	const void *arg_value)
{
	struct clrt_parameter_t *param_info;
	
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tkernel = %p", kernel);
	m2s_clrt_debug("\targ_index = %d", arg_index);
	m2s_clrt_debug("\ttarg_size = %u", arg_size);
	m2s_clrt_debug("\targ_value = %p", arg_value);

	assert(arg_index >= 0 && arg_index < kernel->num_params);
	param_info = kernel->param_info + arg_index;
	assert(param_info->size * sizeof (size_t) >= arg_size || arg_value == NULL);

	assert((arg_value == NULL) == (param_info->mem_type == CLRT_MEM_LOCAL));

	/* local memory */
	if (arg_value == NULL)
		kernel->stack_params[param_info->stack_offset] = arg_size;
	else if (param_info->mem_type == CLRT_MEM_GLOBAL || param_info->mem_type == CLRT_MEM_CONSTANT)
	{
		struct _cl_mem *mem = *(cl_mem *) arg_value;
		if (!clrt_object_verify(mem, CLRT_OBJECT_MEM))
			return CL_INVALID_MEM_OBJECT;
		memcpy(kernel->stack_params + param_info->stack_offset, &mem->buffer, sizeof mem->buffer);
	}
	/* only works on Little-endian machines */
	else if (param_info->is_stack)
		memcpy(kernel->stack_params + param_info->stack_offset, arg_value, arg_size); 
	else
		memcpy(kernel->register_params + param_info->reg_offset, arg_value, arg_size);

	kernel->param_info[arg_index].is_set = 1;
	return 0;
}


cl_int clGetKernelInfo(
	cl_kernel kernel,
	cl_kernel_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetKernelWorkGroupInfo(
	cl_kernel kernel,
	cl_device_id device,
	cl_kernel_work_group_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}

