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
#include "kernel.h"
#include "list.h"
#include "mem.h"
#include "mhandle.h"
#include "misc.h"
#include "opencl.h"
#include "x86-device.h"
#include "x86-kernel.h"
#include "x86-program.h"

#define MEMORY_ALIGN 16

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

struct opencl_x86_kernel_t *opencl_x86_kernel_create(
		struct opencl_kernel_t *parent,
		struct opencl_x86_program_t *program,
		char *func_name)
{
	int i;
	int stride;
	int num_reg;
	int stack_offset;
	int remainder;

	struct opencl_x86_kernel_t *kernel;

	opencl_debug("[%s] creating x86 kernel", __FUNCTION__);

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_x86_kernel_t));
	kernel->type = opencl_runtime_type_x86;
	kernel->parent = parent;
	kernel->program = program;
	kernel->device = program->device;
	kernel->func = opencl_x86_kernel_get_func_info(program->dlhandle,
			func_name, &kernel->metadata);

	/* Check valid kernel function name */
	if (!kernel->func)
		fatal("%s: %s: invalid kernel name", __FUNCTION__,
				func_name);

	kernel->local_reserved_bytes = kernel->metadata[1];
	kernel->num_params = (kernel->metadata[0] - 44) / 24;
	opencl_debug("[%s] num params = %d", __FUNCTION__, kernel->num_params);
	kernel->param_info = xcalloc(1, sizeof kernel->param_info[0] * kernel->num_params);
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

	/* Reserve space for stack arguments */
	kernel->cur_stack_params = xcalloc(kernel->stack_param_words, sizeof(size_t));
	opencl_debug("[%s] kernel = %p", __FUNCTION__, (void*)kernel);

	/* Return */
	return kernel;
}


void opencl_x86_kernel_free(struct opencl_x86_kernel_t *kernel)
{
	opencl_debug("[%s] freeing x86 kernel", __FUNCTION__);
	free(kernel->param_info);
	free(kernel->cur_stack_params);
	free(kernel);
}


int opencl_x86_kernel_set_arg(struct opencl_x86_kernel_t *kernel,
		int arg_index, unsigned int arg_size, void *arg_value)
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
		kernel->cur_stack_params[param_info->stack_offset] = arg_size;
	}
	else if (param_info->mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_GLOBAL
			|| param_info->mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_CONSTANT)
	{
		void *addr = opencl_mem_get_buffer(*(cl_mem *) arg_value);
		if (!addr)
			return CL_INVALID_MEM_OBJECT;
		memcpy(kernel->cur_stack_params + param_info->stack_offset, &addr, sizeof addr);
	}
	else if (param_info->is_stack)
	{
		memcpy(kernel->cur_stack_params + param_info->stack_offset, arg_value, arg_size);
	}
	else
	{
		memcpy(kernel->cur_register_params + param_info->reg_offset, arg_value, arg_size);
	}

	/* Label argument as set */
	kernel->param_info[arg_index].is_set = 1;
	return 0;
}

struct opencl_x86_ndrange_t *opencl_x86_ndrange_create(
	struct opencl_ndrange_t *ndrange,
	struct opencl_x86_kernel_t *x86_kernel,
	unsigned int work_dim, unsigned int *global_work_offset,
	unsigned int *global_work_size, unsigned int *local_work_size,
	unsigned int fused)
{
	int i;

	struct opencl_x86_ndrange_t *arch_ndrange;

	opencl_debug("[%s] creating x86 ndrange", __FUNCTION__);
	assert(x86_kernel->type == opencl_runtime_type_x86);

	arch_ndrange = (struct opencl_x86_ndrange_t *)xcalloc(1, 
		sizeof(struct opencl_x86_ndrange_t));
	arch_ndrange->type = opencl_runtime_type_x86;
	arch_ndrange->parent = ndrange;
	arch_ndrange->work_dim = work_dim;
	arch_ndrange->arch_kernel = x86_kernel;

	/* Work sizes */
	for (i = 0; i < work_dim; i++)
	{
		arch_ndrange->global_work_offset[i] = global_work_offset ?
			global_work_offset[i] : 0;
		arch_ndrange->global_work_size[i] = global_work_size[i];
		arch_ndrange->local_work_size[i] = local_work_size ?
			local_work_size[i] : 1;
		assert(!(global_work_size[i] % 
			arch_ndrange->local_work_size[i]));
		arch_ndrange->group_count[i] = global_work_size[i] / 
			arch_ndrange->local_work_size[i];
	}

	/* Unused dimensions */
	for (i = work_dim; i < 3; i++)
	{
		arch_ndrange->global_work_offset[i] = 0;
		arch_ndrange->global_work_size[i] = 1;
		arch_ndrange->local_work_size[i] = 1;
		arch_ndrange->group_count[i] = 1;
	}

	/* Calculate the number of work groups in the ND-Range */
	arch_ndrange->num_groups = arch_ndrange->group_count[0] * 
		arch_ndrange->group_count[1] * arch_ndrange->group_count[2];


	/* Check that all arguments are set */
	for (i = 0; i < x86_kernel->num_params; i++)
	{
		assert(x86_kernel->param_info);
		if (!x86_kernel->param_info[i].is_set)
			fatal("%s: argument %d not set", __FUNCTION__, i);
	}


	opencl_debug("[%s] dims = %d", __FUNCTION__, work_dim);
	opencl_debug("[%s] local size = (%d, %d, %d)", __FUNCTION__,
		arch_ndrange->local_work_size[0], arch_ndrange->local_work_size[1], arch_ndrange->local_work_size[2]);
	opencl_debug("[%s] global size = (%d, %d, %d)", __FUNCTION__,
		global_work_size[0], global_work_size[1], global_work_size[2]);

	/* copy over register arguments */
	size_t reg_size = sizeof x86_kernel->cur_register_params[0] * MAX_SSE_REG_PARAMS;
	if (posix_memalign((void **) &arch_ndrange->register_params, MEMORY_ALIGN, reg_size))
		fatal("%s: could not allocate aligned memory", __FUNCTION__);
	mhandle_register_ptr(arch_ndrange->register_params, reg_size);
	memcpy(arch_ndrange->register_params, x86_kernel->cur_register_params, reg_size);

	/* copy over stack arguments */
	arch_ndrange->stack_params = xmalloc(x86_kernel->stack_param_words * sizeof(size_t));
	if (!arch_ndrange->stack_params)
		fatal("%s: out of memory", __FUNCTION__);
	memcpy(arch_ndrange->stack_params, x86_kernel->cur_stack_params, x86_kernel->stack_param_words * sizeof (size_t));
	
	return arch_ndrange;
}
/* Initialize an ND-Range */
void opencl_x86_ndrange_init(struct opencl_x86_ndrange_t *ndrange)
{
	opencl_debug("[%s] initing x86 ndrange", __FUNCTION__);

	struct opencl_x86_device_exec_t *exec;
	exec = xcalloc(1, sizeof(struct opencl_x86_device_exec_t));

#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_init(&exec->next_group_lock, NULL);
#endif
	exec->ndrange = ndrange;
	exec->kernel = ndrange->arch_kernel;
	ndrange->exec = exec;
}

/* Finalize an ND-Range */
void opencl_x86_ndrange_free(struct opencl_x86_ndrange_t *ndrange)
{
	opencl_debug("[%s] freeing x86 ndrange", __FUNCTION__);
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_destroy(&ndrange->exec->next_group_lock);
#endif
	free(ndrange->exec);
}

void opencl_x86_ndrange_run_partial(struct opencl_x86_ndrange_t *ndrange, 
	unsigned int *work_group_start, unsigned int *work_group_count)
{
	struct opencl_x86_device_exec_t *exec = ndrange->exec;
	struct opencl_x86_device_t *device = ndrange->arch_kernel->device;

	opencl_debug("[%s] running x86 partial ndrange", __FUNCTION__);

	opencl_debug("[%s] group count = (%d, %d, %d)", __FUNCTION__,
		work_group_count[0], work_group_count[1], work_group_count[2]);

	memcpy(exec->work_group_start, work_group_start, sizeof (unsigned int) * 3);
	memcpy(exec->work_group_count, work_group_count, sizeof (unsigned int) * 3);

	exec->num_groups = 1;
	for (int i = 0; i < 3; i++)
		exec->num_groups *= work_group_count[i];

	/* we can use the queue thread a a worker thread, but we should set it's affinity */
	if (!device->set_queue_affinity)
	{
		cpu_set_t cpu_set;
		CPU_ZERO(&cpu_set);
		CPU_SET(device->num_cores - 1, &cpu_set);
		pthread_setaffinity_np(pthread_self(), sizeof cpu_set, &cpu_set);
		device->set_queue_affinity = 1;
	}

	device->exec = exec;
	opencl_x86_device_sync_post(&device->work_ready);
	opencl_x86_device_run_exec(&device->queue_core, exec);
	device->core_done_count += device->num_cores - 1;
	opencl_x86_device_sync_wait(&device->cores_done, device->core_done_count);
}

/* Run an ND-Range */
void opencl_x86_ndrange_run(struct opencl_x86_ndrange_t *ndrange)
{
	unsigned int group_start[3] = {0, 0, 0};

	opencl_x86_ndrange_init(ndrange);

	opencl_x86_ndrange_run_partial(ndrange, group_start, 
		ndrange->group_count);

	opencl_x86_ndrange_free(ndrange);
}

/* convert a linear address into an n-dimensional address */
void opencl_nd_address(int dim, int addr, const unsigned int *size, unsigned int *pos)
{
	switch (dim)
	{
	case 1:
		pos[0] = addr;
		return;
	case 2:
		pos[1] = addr / size[0];
		pos[0] = addr - (pos[1] * size[0]);
		return;
	case 3:
		pos[2] = addr / size[1];
		pos[1] = (addr - (pos[2] * size[1])) / size[0];
		pos[0] = addr - ((pos[2] * size[1]) + pos[1]) * size[0];
		return;
	default:
		fatal("%s: dim is greater than 3\n", __FUNCTION__);
		return;
	}
}
