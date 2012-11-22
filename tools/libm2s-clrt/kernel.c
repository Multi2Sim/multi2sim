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

#include "m2s-clrt.h"
#include "debug.h"


/*
 * Private Functions 
 */

void clrt_kernel_free(void *data)
{
	struct _cl_kernel *kernel = data;
	free(kernel->entries);
	free(kernel);
}


/*
 * Public Functions 
 */

cl_kernel clCreateKernel(
	cl_program program,
	const char *kernel_name,
	cl_int *errcode_ret)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tprogram = %p", program);
	m2s_clrt_debug("\tkernel_name = %s", kernel_name);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	if (!clrt_object_verify(program, CLRT_OBJECT_PROGRAM))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_PROGRAM;
		return NULL;
	}

	if (!kernel_name)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	cl_kernel kernel = (struct _cl_kernel *) malloc(sizeof (struct _cl_kernel));
	if (!kernel)
		fatal("%s: out of memory", __FUNCTION__);

	clrt_object_create(kernel, CLRT_OBJECT_KERNEL, clrt_kernel_free);

	kernel->num_entries = program->num_entries;
	kernel->entries = malloc(sizeof kernel->entries[0] * kernel->num_entries);
	int i;

	for (i = 0; i < kernel->num_entries; i++)
	{
		struct clrt_device_kernel_t *cur = kernel->entries + i;
		struct clrt_device_program_t *prog = program->entries + i;
		cur->device_type = prog->device_type;
		cur->kernel = cur->device_type->create_kernel(prog->handle, kernel_name, errcode_ret);
	}
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
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tkernel = %p", kernel);
	m2s_clrt_debug("\targ_index = %d", arg_index);
	m2s_clrt_debug("\ttarg_size = %u", arg_size);
	m2s_clrt_debug("\targ_value = %p", arg_value);

	int i;

	if (!clrt_object_verify(kernel, CLRT_OBJECT_KERNEL))
		return CL_INVALID_KERNEL;

	for (i = 0; i < kernel->num_entries; i++)
	{
		struct clrt_device_kernel_t *cur = kernel->entries + i;
		cl_int status = cur->device_type->set_kernel_arg(cur->kernel, arg_index, arg_size, arg_value);
		if (status != CL_SUCCESS)
			return status;
	}

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
	if (!clrt_object_verify(kernel, CLRT_OBJECT_KERNEL))
		return CL_INVALID_KERNEL;

	if (!verify_device(device))
		return CL_INVALID_DEVICE;

	switch (param_name)
	{
		case CL_KERNEL_WORK_GROUP_SIZE:
		{
			size_t size = 1024;
			return populateParameter(&size, sizeof size, param_value_size, param_value, param_value_size_ret);
		}

		case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
		{
			size_t size[] = {0, 0, 0};
			return populateParameter(&size, sizeof size, param_value_size, param_value, param_value_size_ret);
		}

		case CL_KERNEL_LOCAL_MEM_SIZE:
		{
			cl_ulong size = 0;
			return populateParameter(&size, sizeof size, param_value_size, param_value, param_value_size_ret);
		}

		case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
		{
			size_t mult = 1;
			return populateParameter(&mult, sizeof mult, param_value_size, param_value, param_value_size_ret);
		}

		case CL_KERNEL_PRIVATE_MEM_SIZE:
		{
			cl_ulong size = 0;
			return populateParameter(&size, sizeof size, param_value_size, param_value, param_value_size_ret);
		}

		default:
			return CL_INVALID_VALUE;
	}

	return 0;
}

