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
#include <limits.h>

#include "debug.h"
#include "device.h"
#include "mhandle.h"
#include "string.h"
#include "x86-device.h"
#include "x86-kernel.h"
#include "x86-program.h"


/*
 * Private Functions
 */

#define XSTR(s) STR(s)
#define STR(s) #s

#define OPENCL_WORK_GROUP_STACK_ALIGN  16
#define OPENCL_WORK_GROUP_STACK_SIZE  0x00002000
#define OPENCL_WORK_GROUP_STACK_MASK  0xffffe000
#define OPENCL_WORK_GROUP_DATA_OFFSET  -0x60

static struct opencl_x86_device_work_group_data_t
		*opencl_x86_device_get_work_group_data(void)
{
	struct opencl_x86_device_work_group_data_t *data;

	asm volatile (
		"lea " XSTR(OPENCL_WORK_GROUP_STACK_SIZE) "(%%esp), %%eax\n\t"
		"and $" XSTR(OPENCL_WORK_GROUP_STACK_MASK) ", %%eax\n\t"
		"mov " XSTR(OPENCL_WORK_GROUP_DATA_OFFSET) "(%%eax), %0\n\t"
		: "=r" (data)
		:
		: "eax"
	);

	return data;
}


/* Check to see whether the device has been assigned work. Assume that the
 * calling thread owns 'device->lock'. */
static struct opencl_x86_device_exec_t *opencl_x86_device_has_work(
		struct opencl_x86_device_t *device, int *old_count)
{
	while (device->num_kernels == *old_count)
		pthread_cond_wait(&device->ready, &device->lock);

	(*old_count)++;
	return device->exec;
}

/* Get the next work-group in an NDRange */
static int opencl_x86_device_get_next_work_group(
		struct opencl_x86_device_exec_t *exec)
{
	int val;

	pthread_mutex_lock(&exec->mutex);
	val = exec->next_group++;
	pthread_mutex_unlock(&exec->mutex);
	return val;
}


/* Return the number of cores in the host CPU in order to decide the number of
 * threads to spawn running work-groups. */
static int opencl_x86_device_get_num_cores(void)
{
	char s[MAX_LONG_STRING_SIZE];
	int num_cores = 0;
	FILE *f;
	
	/* Get this information from /proc/cpuinfo */
	f = fopen("/proc/cpuinfo", "rt");
	if (!f)
		fatal("%s: cannot access /proc/cpuinfo", __FUNCTION__);

	/* Count entries starting with token 'processor' */
	while (fgets(s, sizeof s, f))
	{
		strtok(s, "\n\t :");
		if (!strcmp(s, "processor"))
			num_cores++;
	}

	/* Done */
	fclose(f);
	return num_cores;
}





/*
 * Public Functions
 */

void opencl_x86_device_make_fiber(
		struct opencl_x86_device_fiber_t *fiber,
		opencl_x86_device_fiber_func_t fiber_func,
		int num_args,
		...)
{
	int i;
	int *params;
	int stack_words;
	int *stack_bottom;

	params = &num_args + 1;

	assert(!((int) params % sizeof(int)));
	assert(!((int) fiber->stack_bottom % sizeof(int)));
	assert(!(fiber->stack_size % sizeof(int)));

	stack_words = fiber->stack_size / sizeof(int);
	stack_bottom = (int *) fiber->stack_bottom;

	for (i = 0; i < num_args; i++)
		stack_bottom[stack_words - num_args + i] = params[i];

	/* Return address of fake 'calling function' */
	stack_bottom[stack_words - num_args - 1] = 0;

	fiber->eip = fiber_func;
	fiber->esp = stack_bottom + stack_words - num_args - 1;
}


void opencl_x86_device_make_fiber_ex(
		struct opencl_x86_device_fiber_t *fiber,
		opencl_x86_device_fiber_func_t fiber_func,
		opencl_x86_device_fiber_return_func_t return_func,
		int arg_size,
		void *args)
{
	arg_size *= sizeof(size_t);
	assert(!(arg_size % sizeof(size_t)));
	assert(!((size_t) fiber->stack_bottom % sizeof(size_t)));
	assert(!(fiber->stack_size % sizeof(size_t)));

	memcpy((char *) fiber->stack_bottom + fiber->stack_size
			- arg_size, args, arg_size);

	fiber->eip = fiber_func;
	fiber->esp = (char *) fiber->stack_bottom + fiber->stack_size
			- arg_size - sizeof (size_t);
	*(size_t *) fiber->esp = (size_t) return_func;
}


void opencl_x86_device_switch_fiber(volatile struct opencl_x86_device_fiber_t *current,
		volatile struct opencl_x86_device_fiber_t *dest,
		volatile void *reg_values)
{
	/* The following code has been added to prevent the compiler from
	 * optimizing out the parts of the caller that set arguments 'current', 'dest',
	 * and 'reg_values'. If it is omitted, using '-O3' will make gcc think
	 * that these values are not used in the function, and the caller will
	 * skip pushing them to the stack. Maybe there is a better way of disabling
	 * this optimization?? For now, we just use some code that uses the variables,
	 * by checking that 'current' and 'dest' are not NULL, and that 'reg_values'
	 * is 16-byte aligned. */
	if ((long) reg_values % 16)
		panic("%s: 'reg_values' not aligned", __FUNCTION__);
	if (!current || !dest)
		panic("%s: 'current' or 'dest' is NULL", __FUNCTION__);

	/* The useful code next */
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
		"je 1f\n\t"				/* Jump to 'switch_fiber_no_regs' */

		"movaps 0x0(%%ecx), %%xmm0\n\t"		/* AMD uses xmm0-xmm3 to pass in parameters */
		"movaps 0x10(%%ecx), %%xmm1\n\t"
		"movaps 0x20(%%ecx), %%xmm2\n\t"
		"movaps 0x30(%%ecx), %%xmm3\n\t"

		"1:\n\t"			/* Former label 'switch_fiber_no_regs' */
		"mov %%esp, (%%eax)\n\t"	/* current->esp <= esp */
		"movl $2f, 0x4(%%eax)\n\t"	/* current->eip <= label 'switch_fiber_return' */

		"mov (%%edx), %%esp\n\t"	/* esp <= dest->esp */
		"jmp *0x4(%%edx)\n\t"		/* eip <= dest->eip */

		"2:\n\t"			/* Former label 'switch_fiber_return' */
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


void opencl_x86_device_exit_fiber(void)
{
	struct opencl_x86_device_work_group_data_t *workgroup_data;

	void *new_esp;
	void *new_eip;

	workgroup_data = opencl_x86_device_get_work_group_data();
	workgroup_data->num_done++;

	new_esp = workgroup_data->main_fiber.esp;
	new_eip = workgroup_data->main_fiber.eip;

	asm volatile (
		"mov %0, %%esp\n\t"
		"jmp *%1\n\t"
		:
		: "g" (new_esp), "g" (new_eip)
	);
}


void opencl_x86_device_barrier(int data)
{
	struct opencl_x86_device_work_group_data_t *workgroup_data;
	struct opencl_x86_device_fiber_t *sleep_fiber;
	struct opencl_x86_device_fiber_t *resume_fiber;

	workgroup_data = opencl_x86_device_get_work_group_data();

	sleep_fiber = workgroup_data->work_items + workgroup_data->current_item;
	workgroup_data->current_item = (workgroup_data->current_item + 1)
			% workgroup_data->num_items;

	resume_fiber = workgroup_data->work_items + workgroup_data->current_item;
	opencl_x86_device_switch_fiber(sleep_fiber, resume_fiber, NULL);
}


/* We need a variable holding the address of the barrier function. The address
 * of this variable is kept in the work-item data structure. */
typedef void (*opencl_x86_device_barrier_func_t)(int user_data);
static opencl_x86_device_barrier_func_t opencl_x86_device_barrier_func
		= opencl_x86_device_barrier;


void opencl_x86_device_init_work_item(
	struct opencl_x86_device_t *device,
	struct opencl_x86_device_work_item_data_t *work_item_data, 
	int dims, 
	const size_t *global, 
	const size_t *local, 
	struct opencl_x86_device_work_group_data_t *work_group_data,
	void *local_reserved)
{
	int i;

	memset(work_item_data, 0, sizeof(struct opencl_x86_device_work_item_data_t));
	for (i = 0; i < 4; i++)
	{
		work_item_data->global_size[i] = 1;
		work_item_data->local_size[i] = 1;
	}

	work_item_data->work_group_data = (int) work_group_data;
	work_item_data->barrier_func = (int) &opencl_x86_device_barrier_func;
	work_item_data->local_reserved = (int) local_reserved;
	work_item_data->work_dim = dims;
	
	assert(dims > 0);
	for (i = 0; i < dims; i++)
	{
		work_item_data->global_size[i] = global[i];
		work_item_data->local_size[i] = local[i];
	}
} 


void opencl_x86_device_work_group_init(
	struct opencl_x86_device_t *device,
	struct opencl_x86_device_work_group_data_t *work_group,
	struct opencl_x86_kernel_t *kernel,
	int dims,
	const size_t *global,
	const size_t *local)
{
	int i;
	void *local_reserved; /* TODO: this needs to be restructured */

	work_group->num_items = 1;
	for (i = 0; i < dims; i++)
		work_group->num_items *= local[i];

	work_group->num_done = 0;
	work_group->current_fiber = NULL;
	work_group->work_items = xmalloc(sizeof (struct opencl_x86_device_fiber_t) * work_group->num_items);
	work_group->work_item_data = xmalloc(sizeof (struct opencl_x86_device_work_item_data_t *) * work_group->num_items);

	if (posix_memalign((void **) &work_group->aligned_stacks,
			OPENCL_WORK_GROUP_STACK_SIZE,
			OPENCL_WORK_GROUP_STACK_SIZE * work_group->num_items))
		fatal("%s: aligned memory allocation failure", __FUNCTION__);
	mhandle_register_ptr(work_group->aligned_stacks,
		OPENCL_WORK_GROUP_STACK_SIZE * work_group->num_items);


	if (kernel->local_reserved_bytes)
		local_reserved = xmalloc(kernel->local_reserved_bytes);
	else
		local_reserved = NULL;

	for (i = 0; i < work_group->num_items; i++)
	{
		struct opencl_x86_device_fiber_t *fiber;

		/* properly initialize the stack and work_group */
		fiber = work_group->work_items + i;
		fiber->stack_bottom = work_group->aligned_stacks
			+ (i * OPENCL_WORK_GROUP_STACK_SIZE);
		fiber->stack_size = OPENCL_WORK_GROUP_STACK_SIZE
			- sizeof(struct opencl_x86_device_work_item_data_t);
		work_group->work_item_data[i] = (struct opencl_x86_device_work_item_data_t *)
			((char *) fiber->stack_bottom + fiber->stack_size);
		opencl_x86_device_init_work_item(device, work_group->work_item_data[i],
			dims, global, local, work_group, local_reserved);
	}

	/* set up params with local memory pointers sperate from those of other threads */
	work_group->stack_params = (size_t *) xmalloc(sizeof (size_t) * kernel->stack_param_words);
	memcpy(work_group->stack_params, kernel->stack_params, sizeof (size_t) * kernel->stack_param_words);
	for (i = 0; i < kernel->num_params; i++)
		if (kernel->param_info[i].mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_LOCAL)
		{
			int offset = kernel->param_info[i].stack_offset;
			if (posix_memalign((void **) (work_group->stack_params + offset),
					OPENCL_WORK_GROUP_STACK_ALIGN, kernel->stack_params[offset]))
				fatal("%s: out of memory", __FUNCTION__);
			mhandle_register_ptr(*(void **) (work_group->stack_params + offset),
					kernel->stack_params[offset]);
		}
}


/* Blocking call to execute a work-group.
 * This code is function is run from within a core-assigned runtime thread */
void opencl_x86_device_work_group_launch(
	struct opencl_x86_kernel_t *kernel,
	int dims,
	const size_t *group_start,
	const size_t *global,
	const size_t *local,
	const size_t *group_id,
	struct opencl_x86_device_work_group_data_t *workgroup_data)
{
	size_t i;
	size_t j;
	size_t k;
	size_t local_size[3];
	size_t group_global[3];
	size_t gid[3];

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
			gid[i] = group_id[i];
		}
		else
		{
			local_size[i] = 1;
			group_global[i] = 0;
			gid[i] = 0;
		}
	}

	/* Initialize stuff that changes per work group */
	for (i = 0; i < local_size[2]; i++)
	{
		for (j = 0; j < local_size[1]; j++)
		{
			for (k = 0; k < local_size[0]; k++)
			{
				size_t id;
				int x;
				struct opencl_x86_device_work_item_data_t *workitem_data;

				id = i * local_size[1] * local_size[0] + j * local_size[0] + k;
				workitem_data = workgroup_data->work_item_data[id];

				/* Set the global id */
				workitem_data->global_id[0] = (int32_t) (group_global[0] + k);
				workitem_data->global_id[1] = (int32_t) (group_global[1] + j);
				workitem_data->global_id[2] = (int32_t) (group_global[2] + i);

				/* Set group global start id and group id */
				for (x = 0; x < 3; x++)
				{
					workitem_data->group_global[x] = (int32_t) group_global[x];
					workitem_data->group_id[x] = (int32_t) gid[x];
				}

			}
		}
	}

	workgroup_data->num_done = 0;

	/* Make new contexts so that they start at the beginning of their functions again  */
	for (i = 0; i < workgroup_data->num_items; i++)
		opencl_x86_device_make_fiber_ex(workgroup_data->work_items + i, kernel->func,
			opencl_x86_device_exit_fiber, kernel->stack_param_words, workgroup_data->stack_params);

	/* Launch fibers */
	while (workgroup_data->num_items > workgroup_data->num_done)
	{
		for (workgroup_data->current_item = 0;
				workgroup_data->current_item < workgroup_data->num_items;
				workgroup_data->current_item++)
		{
			opencl_x86_device_switch_fiber(&workgroup_data->main_fiber,
					workgroup_data->work_items + workgroup_data->current_item,
					kernel->register_params);
		}
	}
}


void opencl_x86_device_work_group_done(
		struct opencl_x86_device_work_group_data_t *work_group_data,
		struct opencl_x86_kernel_t *kernel)
{
	int i;
	int offset;

	free(work_group_data->work_items);
	free(work_group_data->work_item_data);
	free(work_group_data->aligned_stacks);
	for (i = 0; i < kernel->num_params; i++)
	{
		if (kernel->param_info[i].mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_LOCAL)
		{
			offset = kernel->param_info[i].stack_offset;
			free((void *) work_group_data->stack_params[offset]);
		}
	}
	free(work_group_data->stack_params);
}


/* Each core on every device has a thread that runs this procedure
 * It polls for work-groups and launches them on its core */
void *opencl_x86_device_core_func(struct opencl_x86_device_t *device)
{
	struct opencl_x86_device_exec_t *exec;
	struct opencl_x86_device_work_group_data_t work_group_data;
	int count = 0;
	int num;

	/* Lock */
	pthread_mutex_lock(&device->lock);

	/* Get work-groups until done */
	for (;;)
	{
		/* Get one more work-group */
		exec = opencl_x86_device_has_work(device, &count);
		if (!exec)
			break;

		/* Unlock while processing work-group */
		pthread_mutex_unlock(&device->lock);

		/* Initialize work-group data */
		opencl_x86_device_work_group_init(device, &work_group_data,
				exec->kernel, exec->dims, exec->global, exec->local);

		/* Launch work-groups */
		for (;;)
		{
			/* Get next work-group */
			num = opencl_x86_device_get_next_work_group(exec);
			if (num >= exec->num_groups)
				break;

			/* Launch it */
			opencl_x86_device_work_group_launch(exec->kernel,
					exec->dims, exec->group_starts + 3 * num,
					exec->global, exec->local, exec->group_ids + 3 * num, &work_group_data);
		}

		/* Finalize work-group */
		opencl_x86_device_work_group_done(&work_group_data, exec->kernel);

		/* Lock again */
		pthread_mutex_lock(&device->lock);
		device->num_done++;
		if (device->num_done == device->num_cores);
			pthread_cond_signal(&device->done);
	}

	/* Unlock */
	pthread_mutex_unlock(&device->lock);
	return NULL;
}


struct opencl_x86_device_t *opencl_x86_device_create(
		struct opencl_device_t *parent)
{
	struct opencl_x86_device_t *device;
	int err;
	int i;

	/* Initialize */
	device = xcalloc(1, sizeof(struct opencl_x86_device_t));
	device->type = opencl_runtime_type_x86;
	device->parent = parent;
	device->num_cores = opencl_x86_device_get_num_cores();
	device->num_kernels = 0;
	device->num_done = 0;
	device->exec = NULL;

	/* Initialize parent device */
	parent->address_bits = 8 * sizeof (void *);
	parent->available = CL_TRUE;
	parent->compiler_available = CL_FALSE;
	parent->double_fp_config = CL_FP_DENORM |
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;
	parent->endian_little = CL_TRUE;
	parent->error_correction_support = CL_FALSE;
	parent->execution_capabilities = CL_EXEC_KERNEL;
	parent->extensions = "cl_khr_fp64 cl_khr_byte_addressable_store "
			"cl_khr_global_int32_base_atomics "
			"cl_khr_local_int32_base_atomics";
	parent->global_mem_cache_size = 0;
	parent->global_mem_cache_type = CL_READ_WRITE_CACHE;
	parent->global_mem_cacheline_size = 0;
	parent->global_mem_size = 0;
	parent->host_unified_memory = CL_TRUE;
	parent->image_support = CL_FALSE;
	parent->image2d_max_height = 0;
	parent->image2d_max_width = 0;
	parent->image3d_max_depth = 0;
	parent->image3d_max_height = 0;
	parent->image3d_max_width = 0;
	parent->local_mem_size = INT_MAX;
	parent->local_mem_type = CL_GLOBAL;
	parent->max_clock_frequency = 0;
	parent->max_compute_units = opencl_x86_device_get_num_cores();
	parent->max_constant_args = 0;
	parent->max_constant_buffer_size = 0;
	parent->max_mem_alloc_size = INT_MAX;
	parent->max_parameter_size = sizeof (cl_ulong16);
	parent->max_read_image_args = 0;
	parent->max_samplers = 0;
	parent->max_work_group_size = 1024;
	parent->max_work_item_dimensions = 3;
	parent->max_work_item_sizes[0] = 1024;
	parent->max_work_item_sizes[1] = 1024;
	parent->max_work_item_sizes[2] = 1024;
	parent->max_write_image_args = 0;
	parent->mem_base_addr_align = sizeof (cl_float4);
	parent->min_data_type_align_size = 1;
	parent->name = "Multi2Sim 32-bit x86 CPU Model";
	parent->opencl_c_version = VERSION;
	parent->version = "1";
	parent->driver_version = "1";
	parent->vector_width_char = 16;
	parent->vector_width_short = 16 / sizeof (cl_short);
	parent->vector_width_int = 16 / sizeof (cl_int);
	parent->vector_width_long = 16 / sizeof (cl_long);
	parent->vector_width_float = 16 / sizeof (cl_float);
	parent->vector_width_double = 16 / sizeof (cl_double);
	parent->vector_width_half = 0;
	parent->profile = "PROFILE";
	parent->profiling_timer_resolution = 0;
	parent->queue_properties = CL_QUEUE_PROFILING_ENABLE;
	parent->single_fp_config = CL_FP_DENORM | 
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;
	parent->type = CL_DEVICE_TYPE_CPU;
	parent->vendor_id = 0;

	/* Call-back functions for architecture-specific device */
	parent->arch_device_free_func =
			(opencl_arch_device_free_func_t)
			opencl_x86_device_free;
	parent->arch_device_mem_alloc_func =
			(opencl_arch_device_mem_alloc_func_t)
			opencl_x86_device_mem_alloc;
	parent->arch_device_mem_free_func =
			(opencl_arch_device_mem_free_func_t)
			opencl_x86_device_mem_free;
	parent->arch_device_mem_read_func =
			(opencl_arch_device_mem_read_func_t)
			opencl_x86_device_mem_read;
	parent->arch_device_mem_write_func =
			(opencl_arch_device_mem_write_func_t)
			opencl_x86_device_mem_write;
	parent->arch_device_mem_copy_func =
			(opencl_arch_device_mem_copy_func_t)
			opencl_x86_device_mem_copy;
	parent->arch_device_preferred_workgroups_func =
			(opencl_arch_device_preferred_workgroups_func_t)
			opencl_x86_device_preferred_workgroups;

	/* Call-back functions for architecture-specific program */
	parent->arch_program_create_func =
			(opencl_arch_program_create_func_t)
			opencl_x86_program_create;
	parent->arch_program_free_func =
			(opencl_arch_program_free_func_t)
			opencl_x86_program_free;
	parent->arch_program_valid_binary_func =
			opencl_x86_program_valid_binary;

	/* Call-back functions for architecture-specific kernel */
	parent->arch_kernel_create_func =
			(opencl_arch_kernel_create_func_t)
			opencl_x86_kernel_create;
	parent->arch_kernel_free_func =
			(opencl_arch_kernel_free_func_t)
			opencl_x86_kernel_free;
	parent->arch_kernel_set_arg_func =
			(opencl_arch_kernel_set_arg_func_t)
			opencl_x86_kernel_set_arg;

	/* Call-back functions for architecture-specific ND-Range */
	parent->arch_ndrange_create_func =
			(opencl_arch_ndrange_create_func_t)
			opencl_x86_ndrange_create;
	parent->arch_ndrange_free_func =
			(opencl_arch_ndrange_free_func_t)
			opencl_x86_ndrange_free;
	parent->arch_ndrange_init_func =
			(opencl_arch_ndrange_init_func_t)
			opencl_x86_ndrange_init;
	parent->arch_ndrange_run_func =
			(opencl_arch_ndrange_run_func_t)
			opencl_x86_ndrange_run;
	parent->arch_ndrange_run_partial_func =
			(opencl_arch_ndrange_run_partial_func_t)
			opencl_x86_ndrange_run_partial;

	/* Initialize mutex and condition variables */
	pthread_mutex_init(&device->lock, NULL);
	pthread_cond_init(&device->ready, NULL);
	pthread_cond_init(&device->done, NULL);

	/* Initialize threads */
	device->threads = xcalloc(device->num_cores, sizeof(pthread_t));
	for (i = 0; i < device->num_cores; i++)
	{
		cpu_set_t cpu_set;

		/* Create thread */
		err = pthread_create(device->threads + i, NULL,
				(opencl_callback_t) opencl_x86_device_core_func,
				device);
		if (err)
			fatal("%s: could not create thread", __FUNCTION__);

		/* Assign thread to CPU core */
		CPU_ZERO(&cpu_set);
		CPU_SET(i, &cpu_set);
		pthread_setaffinity_np(device->threads[i], sizeof cpu_set, &cpu_set);
	}

	opencl_debug("[%s] opencl_x86_device_t device = %p", __FUNCTION__, 
		device);

	/* Return */
	return device;
}


void opencl_x86_device_free(struct opencl_x86_device_t *device)
{
	free(device->threads);
	free(device);
}


void *opencl_x86_device_mem_alloc(struct opencl_x86_device_t *device,
		unsigned int size)
{
	void *ptr;

	if (posix_memalign(&ptr, 16, size))
		fatal("%s: out of memory", __FUNCTION__);
	mhandle_register_ptr(ptr, size);
	return ptr;
}


void opencl_x86_device_mem_free(struct opencl_x86_device_t *device,
		void *ptr)
{
	free(ptr);
}


void opencl_x86_device_mem_read(struct opencl_x86_device_t *device,
		void *host_ptr, void *device_ptr, unsigned int size)
{
	/* Host and device are the same for x86 CPU */
	memcpy(host_ptr, device_ptr, size);
}


void opencl_x86_device_mem_write(struct opencl_x86_device_t *device,
		void *device_ptr, void *host_ptr, unsigned int size)
{
	/* Host and device are the same for x86 CPU */
	memcpy(device_ptr, host_ptr, size);
}


void opencl_x86_device_mem_copy(struct opencl_x86_device_t *device,
		void *device_dest_ptr, void *device_src_ptr, unsigned int size)
{
	/* Host and device are the same for x86 CPU */
	memcpy(device_dest_ptr, device_src_ptr, size);
}

int opencl_x86_device_preferred_workgroups(struct opencl_x86_device_t *device)
{
	return device->num_cores;
}
