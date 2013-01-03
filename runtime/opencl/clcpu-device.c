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

#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <syscall.h>
#include <limits.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "../include/CL/cl.h"
#include "clcpu.h"
#include "clcpu-device.h"
#include "clcpu-program.h"
#include "clrt.h"
#include "debug.h"
#include "device.h"
#include "mhandle.h"


const char *DEVICE_EXTENSIONS = "cl_khr_fp64 cl_khr_byte_addressable_store cl_khr_global_int32_base_atomics cl_khr_local_int32_base_atomics";
const char *DEVICE_NAME = "x86 CPU";
const char *DRIVER_VERSION = "0.0";
const char *DEVICE_VERSION = "OpenCL 1.1 Multi2Sim";


#define XSTR(s) STR(s)
#define STR(s) #s

#define OPENCL_WORK_GROUP_STACK_ALIGN  16
#define OPENCL_WORK_GROUP_STACK_SIZE  0x00002000
#define OPENCL_WORK_GROUP_STACK_MASK  0xffffe000
#define OPENCL_WORK_GROUP_DATA_OFFSET  -0x60

struct clcpu_workgroup_data_t *get_workgroup_data(void)
{
	struct clcpu_workgroup_data_t *data;

	asm volatile (
		"lea " XSTR(OPENCL_WORK_GROUP_STACK_SIZE) "(%%esp), %%eax\n\t"
		"and $" XSTR(OPENCL_WORK_GROUP_STACK_MASK) ", %%eax\n\t"
		"mov " XSTR(OPENCL_WORK_GROUP_DATA_OFFSET) "(%%eax), %0\n\t"
		: "=g" (data)
		:
		: "eax"
	);

	return data;
}


void switch_fiber(struct fiber_t *current, struct fiber_t *dest, void *reg_values)
{
	asm volatile (
		"push %%eax\n\t"		/* Push registers onto sp + 24 */
		"push %%ebx\n\t"		/* sp + 20 */
		"push %%ecx\n\t"		/* sp + 16 */
		"push %%edx\n\t"		/* sp + 12 */
		"push %%esi\n\t"		/* sp + 8 */
		"push %%edi\n\t"		/* sp + 4 */
		"push %%ebp\n\t"		/* sp */

		"mov %0, %%eax\n\t"		/* eax <= current */
		"mov %1, %%edx\n\t"		/* edx <= dest */
		"mov %2, %%ecx\n\t"		/* ecx <= reg_values */

		"sub $0x80, %%esp\n\t"		/* Make room for SSE registers */
		"movups %%xmm0, 0x0(%%esp)\n\t"
		"movups %%xmm1, 0x10(%%esp)\n\t"
		"movups %%xmm2, 0x20(%%esp)\n\t"
		"movups %%xmm3, 0x30(%%esp)\n\t"
		"movups %%xmm4, 0x40(%%esp)\n\t"
		"movups %%xmm5, 0x50(%%esp)\n\t"
		"movups %%xmm6, 0x60(%%esp)\n\t"
		"movups %%xmm7, 0x70(%%esp)\n\t"
	
		"test %%ecx, %%ecx\n\t"			/* Skip if 'reg_values' is NULL */
		"je opencl_switch_fiber_no_regs\n\t"

		"movaps 0x0(%%ecx), %%xmm0\n\t"		/* AMD uses xmm0-xmm3 to pass in parameters */
		"movaps 0x10(%%ecx), %%xmm1\n\t"
		"movaps 0x20(%%ecx), %%xmm2\n\t"
		"movaps 0x30(%%ecx), %%xmm3\n\t"

		"\nopencl_switch_fiber_no_regs:\n\t"
		"mov %%esp, (%%eax)\n\t"	/* current->esp <= esp */

		"\nopencl_switch_fiber_done_backup:\n\t"
		"call opencl_switch_fiber_get_pc\n\t"

		"\nopencl_switch_fiber_get_pc:\n\t"
		"pop %%ecx\n\t"			/* ecx <= eip at 'opencl_switch_fiber_get_pic' */
		"add $opencl_switch_fiber_return - opencl_switch_fiber_get_pc, %%ecx\n\t"
		"mov %%ecx, 0x4(%%eax)\n\t"	/* current->eip <== opencl_switch_fiber_return */

		"mov (%%edx), %%esp\n\t"	/* esp <= dest->esp */
		"jmp *0x4(%%edx)\n\t"		/* eip <= dest->eip */

		"\nopencl_switch_fiber_return:\n\t"

		"movups 0x0(%%esp), %%xmm0\n\t"
		"movups 0x10(%%esp), %%xmm1\n\t"
		"movups 0x20(%%esp), %%xmm2\n\t"
		"movups 0x30(%%esp), %%xmm3\n\t"
		"movups 0x40(%%esp), %%xmm4\n\t"
		"movups 0x50(%%esp), %%xmm5\n\t"
		"movups 0x60(%%esp), %%xmm6\n\t"
		"movups 0x70(%%esp), %%xmm7\n\t"
		"add $0x80, %%esp\n\t"
	
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		"pop %%esi\n\t"
		"pop %%edx\n\t"
		"pop %%ecx\n\t"
		"pop %%ebx\n\t"
		"pop %%eax\n\t"
		:
		: "g" (current), "g" (dest), "g" (reg_values)
	);
}


void barrier(int data)
{
	struct clcpu_workgroup_data_t *workgroup_data;
	struct fiber_t *sleep;
	struct fiber_t *resume;

	workgroup_data = get_workgroup_data();

	sleep = workgroup_data->workitems + workgroup_data->cur_item;
	workgroup_data->cur_item = (workgroup_data->cur_item + 1) % workgroup_data->num_items;

	resume = workgroup_data->workitems + workgroup_data->cur_item;
	switch_fiber(sleep, resume, NULL);
}

static clrt_barrier_t barrier_addr = barrier;

struct opencl_device_type_t *clcpu_create_device_type(void)
{
	struct opencl_device_type_t *device_type = xmalloc(sizeof *device_type);
	device_type->init_devices = clcpu_device_type_init_devices;
	device_type->valid_binary = clcpu_device_type_is_valid_binary;
	device_type->create_kernel = clcpu_device_type_create_kernel;
	device_type->set_kernel_arg = clcpu_device_type_set_kernel_arg;
	device_type->execute_ndrange = clcpu_device_exceute_ndrange;
	device_type->check_kernel = clcpu_device_check_kernel;
	device_type->kernel_destroy = clcpu_device_kernel_destroy;

	return device_type;
}


int get_cpu_thread_count(void)
{
	if (opencl_native_mode)
		return sysconf(_SC_NPROCESSORS_ONLN); /* Won't work in Multi2Sim */
	else
		return 4; /* TODO: Fix this */
}


/* this function simply initializes all the fields common to all devices for the CPU device in particular */
void clcpu_device_info_init(cl_device_id cpu)
{
	cpu->address_bits = 8 * sizeof (void *);
	cpu->available = CL_TRUE;
	cpu->compiler_available = CL_FALSE;
	cpu->double_fp_config =	CL_FP_DENORM | 
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;
	cpu->endian_little = CL_TRUE;
	cpu->error_correction_support = CL_FALSE;
	cpu->execution_capabilities = CL_EXEC_KERNEL;
	cpu->extensions = DEVICE_EXTENSIONS;
	cpu->global_mem_cache_size = 0;
	cpu->global_mem_cache_type = CL_READ_WRITE_CACHE;
	cpu->global_mem_cacheline_size = 0;
	cpu->global_mem_size = 0;
	cpu->host_unified_memory = CL_TRUE;
	cpu->image_support = CL_FALSE;
	cpu->image2d_max_height = 0;
	cpu->image2d_max_width = 0;
	cpu->image3d_max_depth = 0;
	cpu->image3d_max_height = 0;
	cpu->image3d_max_width = 0;
	cpu->local_mem_size = INT_MAX;
	cpu->local_mem_type = CL_GLOBAL;
	cpu->max_clock_frequency = 0;
	cpu->max_compute_units = get_cpu_thread_count();
	cpu->max_constant_args = 0;
	cpu->max_constant_buffer_size = 0;
	cpu->max_mem_alloc_size = INT_MAX;
	cpu->max_parameter_size = sizeof (cl_ulong16);
	cpu->max_read_image_args = 0;
	cpu->max_samplers = 0;
	cpu->max_work_group_size = 1024;
	cpu->max_work_item_dimensions = 3;
	cpu->max_work_item_sizes[0] = 1024;
	cpu->max_work_item_sizes[1] = 1024;
	cpu->max_work_item_sizes[2] = 1024;
	cpu->max_write_image_args = 0;
	cpu->mem_base_addr_align = sizeof (cl_float4);
	cpu->min_data_type_align_size = 1;
	cpu->name = DEVICE_NAME;
	cpu->opencl_c_version = VERSION;
	cpu->vector_width_char = 16;
	cpu->vector_width_short = 16 / sizeof (cl_short);
	cpu->vector_width_int = 16 / sizeof (cl_int);
	cpu->vector_width_long = 16 / sizeof (cl_long);
	cpu->vector_width_float = 16 / sizeof (cl_float);
	cpu->vector_width_double = 16 / sizeof (cl_double);
	cpu->vector_width_half = 0;
	cpu->profile = "PROFILE";
	cpu->profiling_timer_resolution = 0;
	cpu->queue_properties = CL_QUEUE_PROFILING_ENABLE;
	cpu->single_fp_config = CL_FP_DENORM | 
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;
	cpu->type = CL_DEVICE_TYPE_CPU;
	cpu->vendor_id = 0;
} 


/* Check to see whether the device has been assigned work
 * Assume that the calling thread owns device->lock */
struct clcpu_execution_t *has_work(struct clcpu_device_t *device, int *old_count)
{
	while (device->num_kernels == *old_count)
		pthread_cond_wait(&device->ready, &device->lock);

	(*old_count)++;
	return device->exec;
}

void init_workitem(
	struct clcpu_workitem_data_t *workitem_data, 
	int dims, 
	const size_t *global, 
	const size_t *local, 
	struct clcpu_workgroup_data_t *workgroup_data,
	void *local_reserved)
{
	int i;

	memset(workitem_data, 0, sizeof (struct clcpu_workitem_data_t));
	for (i = 0; i < 4; i++)
	{
		workitem_data->global_size[i] = 1;
		workitem_data->local_size[i] = 1;
	}

	workitem_data->workgroup_data = (int32_t) workgroup_data;
	/* this probably won't work with 64-bit. */
	workitem_data->barrier_func = (int32_t) &barrier_addr;
	workitem_data->local_reserved = (int32_t) local_reserved;

	workitem_data->work_dim = dims;
	assert(dims > 0);
	
	for (i = 0; i < dims; i++)
	{
		workitem_data->global_size[i] = global[i];
		workitem_data->local_size[i] = local[i];
	}


} 

/* set the return address of a work item to point to this funciton. */
void exit_work_item(void)
{
	struct clcpu_workgroup_data_t *workgroup_data;

	void *new_esp;
	void *new_eip;

	workgroup_data = get_workgroup_data();
	workgroup_data->num_done++;

	new_esp = workgroup_data->main_ctx.esp;
	new_eip = workgroup_data->main_ctx.eip;

	asm volatile (
		"mov %0, %%esp\n\t"
		"jmp *%1\n\t"
		:
		: "g" (new_esp), "g" (new_eip)
	);
}  



/* Get the next workgroup in an NDRange */
int get_next(struct clcpu_execution_t *exec)
{
	int val;

	pthread_mutex_lock(&exec->mutex);
	val = exec->next_group++;
	pthread_mutex_unlock(&exec->mutex);
	return val;
}

void init_workgroup(
	struct clcpu_workgroup_data_t *workgroup, 
	struct clcpu_kernel_t *kernel, 
	int dims, 
	const size_t *global, 
	const size_t *local)
{
	int i;
	void *local_reserved; /* TODO: this needs to be restructured */

	workgroup->num_items = 1;
	for (i = 0; i < dims; i++)
		workgroup->num_items *= local[i];

	workgroup->num_done = 0;
	workgroup->cur_ctx = NULL;
	workgroup->workitems = xmalloc(sizeof (struct fiber_t) * workgroup->num_items);
	workgroup->workitem_data = xmalloc(sizeof (struct clcpu_workitem_data_t *) * workgroup->num_items);

	if (posix_memalign((void **)&workgroup->aligned_stacks, OPENCL_WORK_GROUP_STACK_SIZE, OPENCL_WORK_GROUP_STACK_SIZE * workgroup->num_items))
		fatal("%s: aligned memory allocation failure", __FUNCTION__);
	mhandle_register_ptr(workgroup->aligned_stacks, OPENCL_WORK_GROUP_STACK_SIZE * workgroup->num_items);


	if (kernel->local_reserved_bytes)
		local_reserved = xmalloc(kernel->local_reserved_bytes);
	else
		local_reserved = NULL;

	for (i = 0; i < workgroup->num_items; i++)
	{
		struct fiber_t *ctx;

		/* properly initialize the stack and workgroup */
		ctx = workgroup->workitems + i;
		ctx->stack_bottom = workgroup->aligned_stacks + (i * OPENCL_WORK_GROUP_STACK_SIZE);
		ctx->stack_size = OPENCL_WORK_GROUP_STACK_SIZE - sizeof (struct clcpu_workitem_data_t);
		workgroup->workitem_data[i] = (struct clcpu_workitem_data_t *) ((char *) ctx->stack_bottom + ctx->stack_size);
		init_workitem(workgroup->workitem_data[i], dims, global, local, workgroup, local_reserved);
	}

	/* set up params with local memory pointers sperate from those of other threads */
	workgroup->stack_params = (size_t *) xmalloc(sizeof (size_t) * kernel->stack_param_words);
	memcpy(workgroup->stack_params, kernel->stack_params, sizeof (size_t) * kernel->stack_param_words);
	for (i = 0; i < kernel->num_params; i++)
		if (kernel->param_info[i].mem_type == CLCPU_MEM_LOCAL)
		{
			int offset = kernel->param_info[i].stack_offset;
			if (posix_memalign((void **)(workgroup->stack_params + offset), OPENCL_WORK_GROUP_STACK_ALIGN, kernel->stack_params[offset]))
				fatal("%s: out of memory", __FUNCTION__);
			mhandle_register_ptr(*(void **) (workgroup->stack_params + offset), kernel->stack_params[offset]);
		}
} 


/* Blocking call to exceute a work group.
 * This code is function is run from within a core-assigned runtime thread */
void launch_work_group(
	struct clcpu_kernel_t *kernel, 
	int dims, 
	const size_t *group_start, 
	const size_t *global, 
	const size_t *local, 
	struct clcpu_workgroup_data_t *workgroup_data)
{
	size_t i;
	size_t j;
	size_t k;
	size_t local_size[3];
	size_t group_global[3];

	assert(workgroup_data->num_items > 0);
	
	/* we want to safely assume that we have all three dimensions.
	 * but the arrays passed in may only have dims elements,
	 * so we copy them and fill in the rest of the dimensions as having size 1. */
	for (i = 0; i < 3; i++)
	{
		if (i < dims)
		{
			local_size[i] = local[i];
			group_global[i] = group_start[i];
		}
		else
		{
			local_size[i] = 1;
			group_global[i] = 0;
		}
	}

	/* initialize stuff that changes per work group */
	for (i = 0; i < local_size[2]; i++)
		for (j = 0; j < local_size[1]; j++)
			for (k = 0; k < local_size[0]; k++)
			{
				size_t id;
				int x;
				struct clcpu_workitem_data_t *workitem_data;

				id = i * local_size[1] * local_size[0] + j * local_size[0] + k;	
				workitem_data = workgroup_data->workitem_data[id];
				/* set the global id */
				workitem_data->global_id[0] = (int32_t) (group_global[0] + k);
				workitem_data->global_id[1] = (int32_t) (group_global[1] + j);
				workitem_data->global_id[2] = (int32_t) (group_global[2] + i);

				/* set group global start id and group id */
				for (x = 0; x < 3; x++)
				{
					workitem_data->group_global[x] = (int32_t) group_global[x];
					workitem_data->group_id[x] = (int32_t) (group_global[x] / local_size[x]);
				}
				
			}

	workgroup_data->num_done = 0;
	/* make new contexts so that they start at the beginning of their functions again  */
	for (i = 0; i < workgroup_data->num_items; i++)
		make_fiber_ex(workgroup_data->workitems + i, kernel->function, exit_work_item, kernel->stack_param_words, workgroup_data->stack_params);

	while (workgroup_data->num_items > workgroup_data->num_done)
		for (workgroup_data->cur_item = 0; workgroup_data->cur_item < workgroup_data->num_items; workgroup_data->cur_item++)
			switch_fiber(&workgroup_data->main_ctx, workgroup_data->workitems + workgroup_data->cur_item, kernel->register_params);
}

void destroy_workgroup(struct clcpu_workgroup_data_t *workgroup, struct clcpu_kernel_t *kernel)
{
	int i;

	free(workgroup->workitems);
	free(workgroup->workitem_data);
	free(workgroup->aligned_stacks);
	for (i = 0; i < kernel->num_params; i++)
	{
		if (kernel->param_info[i].mem_type == CLCPU_MEM_LOCAL)
		{
			int offset;

			offset = kernel->param_info[i].stack_offset;
			free((void *) workgroup->stack_params[offset]);
		}
	}
	free(workgroup->stack_params);
}


/* Each core on every device has a thread that runs this procedure
 * It polls for workgroups and launches them on its core */
void *clcpu_device_core_proc(void *ptr)
{
	int count = 0;
	struct clcpu_device_t *device = ptr;
	struct clcpu_execution_t *exec = NULL;

	pthread_mutex_lock(&device->lock);
	while ((exec = has_work(device, &count)))
	{
		int num;
		struct clcpu_workgroup_data_t workgroup_data;

		pthread_mutex_unlock(&device->lock);

		init_workgroup(&workgroup_data, exec->kernel, exec->dims, exec->global, exec->local);

		while ((num = get_next(exec)) < exec->num_groups)
			launch_work_group(exec->kernel, exec->dims, exec->group_starts + 3 * num, exec->global, exec->local, &workgroup_data);

		destroy_workgroup(&workgroup_data, exec->kernel);
		pthread_mutex_lock(&device->lock);

		device->num_done++;
		if (device->num_done == device->num_cores);
			pthread_cond_signal(&device->done);
	}

	pthread_mutex_unlock(&device->lock);
	return NULL;
}



struct clcpu_device_t *clcpu_device_create(void)
{
	struct clcpu_device_t *cpu_device = xmalloc(sizeof *cpu_device);

	cpu_device->num_cores = get_cpu_thread_count();
	cpu_device->num_kernels = 0;
	cpu_device->num_done = 0;
	cpu_device->exec = NULL;

	pthread_mutex_init(&cpu_device->lock, NULL);
	pthread_cond_init(&cpu_device->ready, NULL);
	pthread_cond_init(&cpu_device->done, NULL);

	cpu_device->threads = xmalloc(sizeof (pthread_t) * cpu_device->num_cores);
	if(!cpu_device->threads)
		fatal("%s: out of memory", __FUNCTION__);

	int i;
	for (i = 0; i < cpu_device->num_cores; i++)
	{
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(i, &cpuset);
		assert(pthread_create(cpu_device->threads + i, NULL, clcpu_device_core_proc, cpu_device) == 0);
		pthread_setaffinity_np(cpu_device->threads[i], sizeof cpuset, &cpuset);
	}

	return cpu_device;
}

cl_int clcpu_device_type_init_devices(
	cl_uint num_entries, 
	cl_device_id *devices, 
	cl_uint *num_devices)
{
	if (num_devices)
		*num_devices = 1; /* there will always be one CPU */

	if (num_entries && devices)
	{
		cl_device_id cpu = xmalloc(sizeof *cpu);
		clcpu_device_info_init(cpu);
		cpu->device = clcpu_device_create();
		devices[0] = cpu;
	}

	return CL_SUCCESS;
}




