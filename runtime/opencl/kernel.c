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

#include "debug.h"
#include "device.h"
#include "kernel.h"
#include "list.h"
#include "mhandle.h"
#include "misc.h"
#include "object.h"
#include "program.h"


/*
 * Public Functions 
 */

struct opencl_kernel_t *opencl_kernel_create(void)
{
	struct opencl_kernel_t *kernel;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_kernel_t));
	kernel->entry_list = list_create();

	/* Register OpenCL object */
	opencl_object_create(kernel, OPENCL_OBJECT_KERNEL,
		(opencl_object_free_func_t) opencl_kernel_free);

	/* Return */
	return kernel;
}


void opencl_kernel_free(struct opencl_kernel_t *kernel)
{
	struct opencl_kernel_entry_t *entry;
	int index;

	/* Free kernel entries */
	LIST_FOR_EACH(kernel->entry_list, index)
	{
		entry = list_get(kernel->entry_list, index);
		assert(entry->device->arch_kernel_free_func);
		entry->device->arch_kernel_free_func(entry->arch_kernel);
	}

	/* Free kernel */
	list_free(kernel->entry_list);
	opencl_object_release(kernel->program, OPENCL_OBJECT_PROGRAM, CL_INVALID_PROGRAM);
	free(kernel);
}


struct opencl_kernel_entry_t *opencl_kernel_add(struct opencl_kernel_t *kernel,
		struct opencl_device_t *device, void *arch_kernel,
		void *arch_program)
{
	struct opencl_kernel_entry_t *entry;

	/* Initialize new entry */
	entry = xcalloc(1, sizeof(struct opencl_kernel_entry_t));
	entry->device = device;
	entry->arch_kernel = arch_kernel;
	entry->arch_program = arch_program;

	/* Add entry and return */
	list_add(kernel->entry_list, entry);
	return entry;
}




/*
 * OpenCL API Functions
 */

cl_kernel clCreateKernel(
	cl_program program,
	const char *kernel_name,
	cl_int *errcode_ret)
{
	struct opencl_kernel_t *kernel;
	int index;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tprogram = %p", program);
	opencl_debug("\tkernel_name = %s", kernel_name);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Check valid kernel name */
	if (!kernel_name)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	/* Check valid program */
	if (opencl_object_retain(program, OPENCL_OBJECT_PROGRAM, CL_INVALID_VALUE) != CL_SUCCESS)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_PROGRAM;
		return NULL;
	}



	/* Create kernel */
	kernel = opencl_kernel_create();
	kernel->program = program;

	/* For each device listed in the generic program object, create an
	 * architecture-specific kernel as well. */
	LIST_FOR_EACH(program->entry_list, index)
	{
		struct opencl_program_entry_t *entry;
		struct opencl_device_t *device;

		void *arch_program;
		void *arch_kernel;

		/* Get device and architecture-specific program */
		entry = list_get(program->entry_list, index);
		device = entry->device;
		arch_program = entry->arch_program;

		/* Create architecture-specific kernel */
		assert(device->arch_kernel_create_func);
		arch_kernel = device->arch_kernel_create_func(kernel,
			arch_program, kernel_name);

		/* Add new entry to the generic kernel object */
		opencl_kernel_add(kernel, device, arch_kernel, arch_program);
	}

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;

	/* Return kernel */
	return kernel;
}


cl_int clCreateKernelsInProgram(
	cl_program program,
	cl_uint num_kernels,
	cl_kernel *kernels,
	cl_uint *num_kernels_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clRetainKernel(
	cl_kernel kernel)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tkernel = %p", kernel);

	return opencl_object_retain(kernel, OPENCL_OBJECT_KERNEL, CL_INVALID_KERNEL);
}


cl_int clReleaseKernel(
	cl_kernel kernel)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tkernel = %p", kernel);

	return opencl_object_release(kernel, OPENCL_OBJECT_KERNEL, CL_INVALID_KERNEL);
}


cl_int clSetKernelArg(
	cl_kernel kernel,
	cl_uint arg_index,
	size_t arg_size,
	const void *arg_value)
{
	struct opencl_kernel_entry_t *entry;
	cl_int status;
	int i;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tkernel = %p", kernel);
	opencl_debug("\targ_index = %d", arg_index);
	opencl_debug("\ttarg_size = %u", arg_size);
	opencl_debug("\targ_value = %p", arg_value);

	/* Check valid kernel */
	if (!opencl_object_verify(kernel, OPENCL_OBJECT_KERNEL))
		return CL_INVALID_KERNEL;

	/* Set argument for all devices */
	LIST_FOR_EACH(kernel->entry_list, i)
	{
		entry = list_get(kernel->entry_list, i);
		assert(entry->device->arch_kernel_set_arg_func);
		status = entry->device->arch_kernel_set_arg_func(entry->arch_kernel,
			arg_index, arg_size, (void *) arg_value);
		if (status != CL_SUCCESS)
			return status;
	}

	/* Success */
	return 0;
}


cl_int clGetKernelInfo(
	cl_kernel kernel,
	cl_kernel_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__OPENCL_NOT_IMPL__
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
	/* Check valid kernel */
	if (!opencl_object_verify(kernel, OPENCL_OBJECT_KERNEL))
		return CL_INVALID_KERNEL;

	/* Check valid device */
	if (!opencl_device_verify(device))
		return CL_INVALID_DEVICE;

	switch (param_name)
	{
		case CL_KERNEL_WORK_GROUP_SIZE:
		{
			size_t size = device->max_work_group_size;
			return opencl_set_param(&size, sizeof size, param_value_size,
				param_value, param_value_size_ret);
		}

		case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
		{
			size_t size[] = {0, 0, 0};
			return opencl_set_param(&size, sizeof size, param_value_size,
				param_value, param_value_size_ret);
		}

		case CL_KERNEL_LOCAL_MEM_SIZE:
		{
			cl_ulong size = 0;
			return opencl_set_param(&size, sizeof size, param_value_size,
				param_value, param_value_size_ret);
		}

		case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
		{
			size_t mult = 1;
			return opencl_set_param(&mult, sizeof mult, param_value_size,
				param_value, param_value_size_ret);
		}

		case CL_KERNEL_PRIVATE_MEM_SIZE:
		{
			cl_ulong size = 0;
			return opencl_set_param(&size, sizeof size, param_value_size,
				param_value, param_value_size_ret);
		}

		default:
			return CL_INVALID_VALUE;
	}

	return 0;
}

struct opencl_ndrange_t *opencl_ndrange_create(
	struct opencl_device_t *device,
	struct opencl_kernel_t *kernel,
	unsigned int work_dim,
	unsigned int *global_work_offset,
	unsigned int *global_work_size,
	unsigned int *local_work_size)
{
	struct opencl_ndrange_t *ndrange;
	struct opencl_kernel_entry_t *kernel_entry;

	void *arch_kernel = NULL;

	int i;

	assert(kernel);
	assert(device);
	assert(global_work_size);
	assert(IN_RANGE(work_dim, 1, 3));

	ndrange = xcalloc(1, sizeof(struct opencl_ndrange_t));
	ndrange->device = device;
	ndrange->kernel = kernel;

	opencl_debug("[%s] creating an nd-range for %s", __FUNCTION__, 
		device->name);
	LIST_FOR_EACH(kernel->entry_list, i)
	{
		kernel_entry = list_get(kernel->entry_list, i);
		if (kernel_entry->device == device)
			arch_kernel = kernel_entry->arch_kernel;
	}
	assert(arch_kernel);

	ndrange->arch_ndrange = device->arch_ndrange_create_func(ndrange,
		arch_kernel, work_dim, global_work_offset, global_work_size,
		local_work_size, 0);

	return ndrange;
}
