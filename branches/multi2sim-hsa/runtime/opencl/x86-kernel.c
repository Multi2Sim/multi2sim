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

#include <sys/syscall.h>
#include <sys/ioctl.h>
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

const int OPENCL_WORK_GROUP_STACK_ALIGN=16;

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
		memcpy(kernel->cur_stack_params + param_info->stack_offset, 
			&addr, sizeof addr);
	}
	else if (param_info->is_stack)
	{
		memcpy(kernel->cur_stack_params + param_info->stack_offset, 
			arg_value, arg_size);
	}
	else
	{
		memcpy(kernel->cur_register_params + param_info->reg_offset, 
			arg_value, arg_size);
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
		arch_ndrange->num_groups[i] = global_work_size[i] / 
			arch_ndrange->local_work_size[i];
	}

	/* Unused dimensions */
	for (i = work_dim; i < 3; i++)
	{
		arch_ndrange->global_work_offset[i] = 0;
		arch_ndrange->global_work_size[i] = 1;
		arch_ndrange->local_work_size[i] = 1;
		arch_ndrange->num_groups[i] = 1;
	}

	/* Calculate the number of work groups in the ND-Range */
	arch_ndrange->total_num_groups = 
		arch_ndrange->num_groups[0] * 
		arch_ndrange->num_groups[1] * 
		arch_ndrange->num_groups[2];

	/* Check that all arguments are set */
	for (i = 0; i < x86_kernel->num_params; i++)
	{
		assert(x86_kernel->param_info);
		if (!x86_kernel->param_info[i].is_set)
			fatal("%s: argument %d not set", __FUNCTION__, i);
	}

	opencl_debug("[%s] dims = %d", __FUNCTION__, work_dim);
	opencl_debug("[%s] local size = (%d, %d, %d)", __FUNCTION__,
		arch_ndrange->local_work_size[0], 
		arch_ndrange->local_work_size[1], 
		arch_ndrange->local_work_size[2]);
	opencl_debug("[%s] global size = (%d, %d, %d)", __FUNCTION__,
		arch_ndrange->global_work_size[0], 
		arch_ndrange->global_work_size[1], 
		arch_ndrange->global_work_size[2]);

	/* copy over register arguments */
	size_t reg_size = sizeof x86_kernel->cur_register_params[0] * 
		MAX_SSE_REG_PARAMS;
	if (posix_memalign((void **) &arch_ndrange->register_params, 
		MEMORY_ALIGN, reg_size))
	{
		fatal("%s: could not allocate aligned memory", __FUNCTION__);
	}
	mhandle_register_ptr(arch_ndrange->register_params, reg_size);
	memcpy(arch_ndrange->register_params, x86_kernel->cur_register_params, 
		reg_size);

	/* copy over stack arguments */
	arch_ndrange->stack_params = xmalloc(x86_kernel->stack_param_words * 
		sizeof(size_t));
	if (!arch_ndrange->stack_params)
		fatal("%s: out of memory", __FUNCTION__);
	memcpy(arch_ndrange->stack_params, x86_kernel->cur_stack_params, 
		x86_kernel->stack_param_words * sizeof (size_t));
	
	return arch_ndrange;
}


/*
 * One-time initialization of work-group data for an nd-range
 */
void opencl_x86_kernel_work_group_init(
	struct opencl_x86_work_group_t *work_group,
	struct opencl_x86_device_exec_t *e)
{
	int i;
	struct opencl_x86_ndrange_t *nd = e->ndrange;


	work_group->num_items = 1;
	for (i = 0; i < nd->work_dim; i++)
		work_group->num_items *= e->ndrange->local_work_size[i];

	work_group->register_params = nd->register_params;
	work_group->kernel_fn = nd->arch_kernel->func;

	if (e->kernel->local_reserved_bytes)
	{
		work_group->local_reserved = xmalloc(
			e->kernel->local_reserved_bytes);
	}
	else
	{
		work_group->local_reserved = NULL;
	}

	/* Set up params with local memory pointers separate from those 
	 * of other threads */
	work_group->stack_params = (size_t *) xmalloc(
		sizeof (size_t) * e->kernel->stack_param_words);
	memcpy(work_group->stack_params, e->ndrange->stack_params, 
		sizeof (size_t) * e->kernel->stack_param_words);
	for (i = 0; i < e->kernel->num_params; i++)
	{
		if (e->kernel->param_info[i].mem_arg_type == 
			OPENCL_X86_KERNEL_MEM_ARG_LOCAL)
		{
			int offset = e->kernel->param_info[i].stack_offset;
			if (posix_memalign((void **) (work_group->stack_params +
				offset), OPENCL_WORK_GROUP_STACK_ALIGN, 
				e->ndrange->stack_params[offset]))
			{
				fatal("%s: out of memory", __FUNCTION__);
			}
			mhandle_register_ptr(*(void **) (
				work_group->stack_params + offset),
				e->ndrange->stack_params[offset]);
		}
	}
}


/* Initialize an ND-Range */
void opencl_x86_ndrange_init(struct opencl_x86_ndrange_t *ndrange)
{
	opencl_debug("[%s] initing x86 ndrange", __FUNCTION__);

	struct opencl_x86_device_t *device = ndrange->arch_kernel->device;

	/* Set the affinity of the dispatch thread to the last core.
	 * The worker threads will be scheduled to the N-1 cores. */
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(device->num_cores - 1, &cpu_set);
	pthread_setaffinity_np(pthread_self(), sizeof cpu_set, &cpu_set);

	struct opencl_x86_device_exec_t *exec;
	exec = xcalloc(1, sizeof(struct opencl_x86_device_exec_t));

#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_init(&exec->next_group_lock, NULL);
#endif
	exec->ndrange = ndrange;
	exec->kernel = ndrange->arch_kernel;
	ndrange->exec = exec;

	/* Initialize the static work-group data for this kernel
	 * for the main x86 thread (the worker threads initialize
	 * their structures in opencl_x86_device_run_func). */
	opencl_x86_kernel_work_group_init(&device->work_group, exec);
}


/* 
 * Release resources required by the work-group 
 */
void opencl_x86_kernel_work_group_done(
	struct opencl_x86_work_group_t *work_group,
	struct opencl_x86_kernel_t *kernel)
{
	int i;
	int offset;

	for (i = 0; i < kernel->num_params; i++)
	{
		if (kernel->param_info[i].mem_arg_type == 
			OPENCL_X86_KERNEL_MEM_ARG_LOCAL)
		{
			offset = kernel->param_info[i].stack_offset;
			free((void *) work_group->stack_params[offset]);
		}
	}
	free(work_group->stack_params);
	if (work_group->local_reserved)
		free(work_group->local_reserved);
}

/* 
 * Release resources required by the nd-range
 */
void opencl_x86_ndrange_finish(struct opencl_x86_ndrange_t *ndrange)
{
	struct opencl_x86_device_t *device = ndrange->arch_kernel->device;
	struct opencl_x86_device_exec_t *exec = ndrange->exec;

	ndrange->done = 1;

	device->exec = exec;
	opencl_x86_device_sync_post(&device->work_ready);
	opencl_x86_device_run_exec(&device->work_group, exec);
	device->work_group_done_count += device->num_cores - 1;
	opencl_x86_device_sync_wait(&device->work_groups_done, 
		device->work_group_done_count);

	/* Release the work group resources */
	opencl_x86_kernel_work_group_done(&device->work_group, 
		ndrange->arch_kernel);
}

/* 
 * Finalize an ND-Range 
 * */
void opencl_x86_ndrange_free(struct opencl_x86_ndrange_t *ndrange)
{
	opencl_debug("[%s] freeing x86 ndrange", __FUNCTION__);
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_destroy(&ndrange->exec->next_group_lock);
#endif
	free(ndrange->exec);
}

void opencl_x86_ndrange_run_partial(struct opencl_x86_ndrange_t *ndrange, 
	unsigned int work_group_start, unsigned int work_group_count)
{
	struct opencl_x86_device_exec_t *exec = ndrange->exec;
	struct opencl_x86_device_t *device = ndrange->arch_kernel->device;

	opencl_debug("[%s] running x86 partial ndrange", __FUNCTION__);
	/*
	opencl_debug("[%s] x86 scheduling thread has tid %ld", 
		__FUNCTION__, syscall(SYS_gettid));
	opencl_debug("[%s] group start = %u, group count = %u", __FUNCTION__,
		work_group_start, work_group_count);
		*/

	exec->work_group_start = work_group_start;
	exec->next_group = work_group_start;
	exec->work_group_count = work_group_count;

	device->exec = exec;
	opencl_x86_device_sync_post(&device->work_ready);
	opencl_x86_device_run_exec(&device->work_group, exec);
	device->work_group_done_count += device->num_cores - 1;
	opencl_x86_device_sync_wait(&device->work_groups_done, 
		device->work_group_done_count);

	ndrange->scheduling_pass++;
}

/* Run an ND-Range */
void opencl_x86_ndrange_run(struct opencl_x86_ndrange_t *ndrange,
		struct opencl_event_t *event)
{
	struct timespec start, end;

	cl_ulong cltime;

	int sched_policy_new;
	int sched_policy_old;
	struct sched_param sched_param_old;
	struct sched_param sched_param_new;

	/* One-time initialization */
	opencl_x86_ndrange_init(ndrange);

	/* Tell the driver that the nd-range has started.  Give 
	 * dispatch thread highest priority. */
	if (!opencl_native_mode)
	{
		/* FIXME - x86 execution should not communicate with driver */
		// ioctl(m2s_active_dev, SINDRangeStart);

		/* Store old scheduling policy and priority */
		pthread_getschedparam(pthread_self(), &sched_policy_old, 
			&sched_param_old);

		/* Give dispatch threads the highest priority */
		sched_policy_new = SCHED_RR;
		sched_param_new.sched_priority = sched_get_priority_max(
			sched_policy_new);
		pthread_setschedparam(pthread_self(), sched_policy_new, 
			&sched_param_new);
	}

	/* Record start time */
	if (event)
	{
		clock_gettime(CLOCK_MONOTONIC, &start);
	}

	/* Execute the nd-range */
	opencl_x86_ndrange_run_partial(ndrange, 0, ndrange->total_num_groups);

	/* Record end time */
	if (event)
	{
		clock_gettime(CLOCK_MONOTONIC, &end);

		cltime = (cl_ulong)start.tv_sec;
		cltime *= 1000000000;
		cltime += (cl_ulong)start.tv_nsec;
		event->time_start = cltime;

		cltime = (cl_ulong)end.tv_sec;
		cltime *= 1000000000;
		cltime += (cl_ulong)end.tv_nsec;
		event->time_end = cltime;
	}

	opencl_x86_ndrange_finish(ndrange);

	if (!opencl_native_mode)
	{
		/* Reset old scheduling parameters */
		pthread_setschedparam(pthread_self(), sched_policy_old, 
			&sched_param_old);

		/* Tell the driver that the nd-range has ended */
		// FIXME - x86 device should not communicate with driver */
		//ioctl(m2s_active_dev, SINDRangeEnd);
	}

	/* Tear-down */
	opencl_x86_ndrange_free(ndrange);
}

/* convert a linear address into an n-dimensional address */
void opencl_nd_address(int linear_id, const unsigned int *dims, 
	unsigned int *id)
{
	if (dims[2] == 1)
	{
		id[2] = 0;
		id[1] = linear_id/dims[0];
		id[0] = linear_id - (id[1]*dims[0]);
	}
	else
	{
		unsigned int items_per_slice = (dims[1]*dims[0]);
		id[2] = linear_id/items_per_slice;
		unsigned int tmp = linear_id - (id[2]*items_per_slice);
		id[1] = tmp/dims[0];
		id[0] = tmp - (id[1]*dims[0]);
	}

	return;
}

/* convert a linear address into an n-dimensional address */
void opencl_nd_address_old(int linear_id, const unsigned int *dims, 
	unsigned int *id)
{
	id[2] = linear_id / (dims[1] * dims[0]);
	id[1] = (linear_id / dims[0]) % dims[1];
	id[0] = (linear_id % dims[0]);
	return;
}
