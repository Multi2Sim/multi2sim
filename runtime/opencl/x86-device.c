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

static struct opencl_x86_device_core_t
		*opencl_x86_device_get_work_group_data(void)
{
	struct opencl_x86_device_core_t *data;

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
	(*old_count)++;

	opencl_x86_device_sync_wait(&device->work_ready, *old_count);
	return device->exec;
}


/* Get the next work-group in an NDRange */
static int opencl_x86_device_get_next_work_group(
		struct opencl_x86_device_exec_t *exec)
{
#ifndef HAVE_SYNC_BUILTINS
	int next;
	pthread_mutex_lock(&exec->next_group_lock);
	next = exec->next_group++;
	pthread_mutex_unlock(&exec->next_group_lock);
	return next;
#else
	return __sync_fetch_and_add(&exec->next_group, 1);
#endif
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

void opencl_x86_device_reinit_work_item(struct opencl_x86_device_core_t *core)
{
	struct opencl_x86_device_work_item_data_t *work_item_data = core->work_item_data[0];
	struct opencl_x86_device_fiber_t *fiber = core->work_fibers;
	size_t size = core->nd->arch_kernel->stack_param_words * sizeof (size_t);
	char *stack_top = (char *)fiber->stack_bottom + fiber->stack_size - size;

	opencl_nd_address(core->nd->work_dim, core->current_item, core->nd->local_work_size, work_item_data->global_id);

	work_item_data->global_id[0] += core->group_global[0];
	work_item_data->global_id[1] += core->group_global[1];
	work_item_data->global_id[2] += core->group_global[2];

	memcpy(stack_top, core->stack_params, size);
	fiber->eip = opencl_x86_work_item_entry_point;
	fiber->esp = stack_top - sizeof (size_t);
	*(size_t *)fiber->esp = (size_t)opencl_x86_device_exit_fiber;
}



void opencl_x86_device_exit_fiber(void)
{
	struct opencl_x86_device_core_t *workgroup_data;

	void *new_esp;
	void *new_eip;

	workgroup_data = opencl_x86_device_get_work_group_data();
	workgroup_data->num_done++;

	/* exit to the main fiber.  This work-group is done */
	if (workgroup_data->num_done == workgroup_data->num_items)
	{
		new_esp = workgroup_data->main_fiber.esp;
		new_eip = workgroup_data->main_fiber.eip;
	}
	/* this work-group has barriers and therefore, one stack per work-item.
	   switch to the next work-item's stack and initialize it if it is new */
	else if (workgroup_data->hit_barrier)
	{
		int i = workgroup_data->current_item + 1;
		struct opencl_x86_device_fiber_t *resume_fiber;

		workgroup_data->current_item = i;
		resume_fiber = workgroup_data->work_fibers + i;

		if (workgroup_data->num_started++ == i)
			opencl_x86_device_init_work_item(i, workgroup_data);

		new_esp = resume_fiber->esp;
		new_eip = resume_fiber->eip;
	}
	/* this work-group doesn't have barriers.  re-use the current work-item */
	else
	{
		workgroup_data->current_item++;
		opencl_x86_device_reinit_work_item(workgroup_data);
		new_esp = workgroup_data->work_fibers->esp;
		new_eip = workgroup_data->work_fibers->eip;
	}

	asm volatile (
		"mov %0, %%esp\n\t"
		"jmp *%1\n\t"
		:
		: "g" (new_esp), "g" (new_eip)
	);
}


void opencl_x86_device_barrier(int data)
{
	int i;
	struct opencl_x86_device_core_t *workgroup_data;
	struct opencl_x86_device_fiber_t *sleep_fiber;
	struct opencl_x86_device_fiber_t *resume_fiber;

	workgroup_data = opencl_x86_device_get_work_group_data();
	workgroup_data->hit_barrier = 1;
	i = workgroup_data->current_item;

	sleep_fiber = workgroup_data->work_fibers + i;
	i = (i + 1) % workgroup_data->num_items;

	workgroup_data->current_item = i;
	resume_fiber = workgroup_data->work_fibers + i;
	
	if (workgroup_data->num_started++ == i)
		opencl_x86_device_init_work_item(i, workgroup_data);

	opencl_x86_device_switch_fiber(sleep_fiber, resume_fiber);
}


/* We need a variable holding the address of the barrier function. The address
 * of this variable is kept in the work-item data structure. */
typedef void (*opencl_x86_device_barrier_func_t)(int user_data);
static opencl_x86_device_barrier_func_t opencl_x86_device_barrier_func
		= opencl_x86_device_barrier;


void opencl_x86_device_init_work_item(int i, struct opencl_x86_device_core_t *core)
{
	struct opencl_x86_device_work_item_data_t *work_item_data = core->work_item_data[i];
	struct opencl_x86_ndrange_t *nd = core->nd;
	size_t arg_size =  nd->arch_kernel->stack_param_words * sizeof (size_t);
	struct opencl_x86_device_fiber_t *fiber = core->work_fibers + i;
	char *stack_top = (char *)fiber->stack_bottom + fiber->stack_size - arg_size;

	work_item_data->local_reserved = (int) core->local_reserved;
	work_item_data->work_dim = nd->work_dim;
	
	work_item_data->group_global[0] = core->group_global[0];
	work_item_data->group_global[1] = core->group_global[1];
	work_item_data->group_global[2] = core->group_global[2];

	work_item_data->global_size[0] = nd->global_work_size[0];
	work_item_data->global_size[1] = nd->global_work_size[1];
	work_item_data->global_size[2] = nd->global_work_size[2];

	work_item_data->local_size[0] = nd->local_work_size[0];
	work_item_data->local_size[1] = nd->local_work_size[1];
	work_item_data->local_size[2] = nd->local_work_size[2];

	work_item_data->group_id[0] = core->group_id[0];
	work_item_data->group_id[1] = core->group_id[1];
	work_item_data->group_id[2] = core->group_id[2];

	opencl_nd_address(nd->work_dim, i, nd->local_work_size, work_item_data->global_id);

	work_item_data->global_id[0] += core->group_global[0];
	work_item_data->global_id[1] += core->group_global[1];
	work_item_data->global_id[2] += core->group_global[2];

	memcpy(stack_top, core->stack_params, arg_size);
	fiber->eip = opencl_x86_work_item_entry_point;
	fiber->esp = stack_top - sizeof (size_t);
	*(size_t *) fiber->esp = (size_t) opencl_x86_device_exit_fiber;
} 


void opencl_x86_device_work_group_init(
	struct opencl_x86_device_core_t *work_group,
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
		work_group->local_reserved = xmalloc(e->kernel->local_reserved_bytes);
	else
		work_group->local_reserved = NULL;

	/* set up params with local memory pointers sperate from those of other threads */
	work_group->stack_params = (size_t *) xmalloc(sizeof (size_t) * e->kernel->stack_param_words);
	memcpy(work_group->stack_params, e->ndrange->stack_params, sizeof (size_t) * e->kernel->stack_param_words);
	for (i = 0; i < e->kernel->num_params; i++)
		if (e->kernel->param_info[i].mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_LOCAL)
		{
			int offset = e->kernel->param_info[i].stack_offset;
			if (posix_memalign((void **) (work_group->stack_params + offset),
					OPENCL_WORK_GROUP_STACK_ALIGN, e->ndrange->stack_params[offset]))
				fatal("%s: out of memory", __FUNCTION__);
			mhandle_register_ptr(*(void **) (work_group->stack_params + offset),
					e->ndrange->stack_params[offset]);
		}
}


/* Blocking call to execute a work-group.
 * This code is function is run from within a core-assigned runtime thread */
void opencl_x86_device_work_group_launch(
	int num,
	struct opencl_x86_device_exec_t *exec,
	struct opencl_x86_device_core_t *core)
{
	const unsigned int *local_size = exec->ndrange->local_work_size;
	struct opencl_x86_ndrange_t *nd = exec->ndrange;

	opencl_nd_address(nd->work_dim, num, exec->work_group_count, core->group_id);
	for (int i = 0; i < 3; i++)
		core->group_global[i] = (core->group_id[i] + exec->work_group_start[i]) * local_size[i] + nd->global_work_offset[i];
	
	assert(core->num_items > 0);

	core->num_started = 1;
	core->num_done = 0;
	core->current_item = 0;
	core->nd = nd;
	core->hit_barrier = 0;

	opencl_x86_device_init_work_item(0, core);
	opencl_x86_device_switch_fiber(&core->main_fiber, core->work_fibers);
}


void opencl_x86_device_work_group_done(
		struct opencl_x86_device_core_t *core,
		struct opencl_x86_kernel_t *kernel)
{
	int i;
	int offset;

	for (i = 0; i < kernel->num_params; i++)
	{
		if (kernel->param_info[i].mem_arg_type == OPENCL_X86_KERNEL_MEM_ARG_LOCAL)
		{
			offset = kernel->param_info[i].stack_offset;
			free((void *) core->stack_params[offset]);
		}
	}
	free(core->stack_params);
	if (core->local_reserved)
		free(core->local_reserved);
}

void opencl_x86_device_core_init(struct opencl_x86_device_core_t *work_group)
{
	if (posix_memalign((void **) &work_group->aligned_stacks,
			OPENCL_WORK_GROUP_STACK_SIZE,
			OPENCL_WORK_GROUP_STACK_SIZE * X86_MAX_WORK_GROUP_SIZE))
		fatal("%s: aligned memory allocation failure", __FUNCTION__);
	mhandle_register_ptr(work_group->aligned_stacks,
		OPENCL_WORK_GROUP_STACK_SIZE * X86_MAX_WORK_GROUP_SIZE);


	for (int i = 0; i < X86_MAX_WORK_GROUP_SIZE; i++)
	{
		struct opencl_x86_device_fiber_t *fiber;
		struct opencl_x86_device_work_item_data_t *work_item_data;

		/* properly initialize the stack and work_group */
		fiber = work_group->work_fibers + i;
		fiber->stack_bottom = work_group->aligned_stacks
			+ (i * OPENCL_WORK_GROUP_STACK_SIZE);
		fiber->stack_size = OPENCL_WORK_GROUP_STACK_SIZE
			- sizeof(struct opencl_x86_device_work_item_data_t);
		
		work_item_data = (struct opencl_x86_device_work_item_data_t *)
			((char *) fiber->stack_bottom + fiber->stack_size);

		work_item_data->work_group_data = (int) work_group;
		work_item_data->barrier_func = (int) &opencl_x86_device_barrier_func;

		work_group->work_item_data[i] = work_item_data;
	}

}

void opencl_x86_device_core_treardown(struct opencl_x86_device_core_t *work_group_data)
{
	free(work_group_data->aligned_stacks);
}


void opencl_x86_device_run_exec(
	struct opencl_x86_device_core_t *core,
	struct opencl_x86_device_exec_t *exec)
{
	/* Initialize kernel data */
	opencl_x86_device_work_group_init(core, exec);

	/* Launch work-groups */
	for (;;)
	{
		/* Get next work-group */
		int num = opencl_x86_device_get_next_work_group(exec);
		if (num >= exec->num_groups)
			break;

		/* Launch it */
		opencl_x86_device_work_group_launch(num, exec, core);

	}

	/* Finalize kernel */
	opencl_x86_device_work_group_done(core, exec->kernel);
}

/* Each core on every device has a thread that runs this procedure
 * It polls for work-groups and launches them on its core */
void *opencl_x86_device_core_func(struct opencl_x86_device_t *device)
{
	struct opencl_x86_device_exec_t *exec;
	struct opencl_x86_device_core_t core;
	int count = 0;

	opencl_x86_device_core_init(&core);

	/* Get kernels until done */
	for (;;)
	{
		/* Get one more kernel */
		exec = opencl_x86_device_has_work(device, &count);
		if (!exec)
			break;

		opencl_x86_device_run_exec(&core, exec);

		opencl_x86_device_sync_post(&device->cores_done);
	}
	opencl_x86_device_core_treardown(&core);
	return NULL;
}

void opencl_x86_device_sync_init(struct opencl_x86_device_sync_t *sync)
{
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_init(&sync->lock, NULL);
	pthread_cond_init(&sync->cond, NULL);
#endif
	sync->count = 0;
}

void opencl_x86_device_sync_destroy(struct opencl_x86_device_sync_t *sync)
{
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_destroy(&sync->lock);
	pthread_cond_destroy(&sync->cond);
#endif
}

void opencl_x86_device_sync_wait(struct opencl_x86_device_sync_t *sync, int value)
{
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_lock(&sync->lock);
	while (sync->count != value)
		pthread_cond_wait(&sync->cond, &sync->lock);
	pthread_mutex_unlock(&sync->lock);
#else
	while (sync->count != value);
#endif

}

void opencl_x86_device_sync_post(struct opencl_x86_device_sync_t *sync)
{
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_lock(&sync->lock);
	sync->count++;
	pthread_mutex_unlock(&sync->lock);
	pthread_cond_broadcast(&sync->cond);
#else
	__sync_fetch_and_add(&sync->count, 1);
#endif
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
	opencl_x86_device_sync_init(&device->work_ready);
	opencl_x86_device_sync_init(&device->cores_done);
	device->exec = NULL;
	device->core_done_count = 0;
	device->set_queue_affinity = 0;

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
	parent->linker_available = CL_FALSE;
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
	parent->max_work_group_size = X86_MAX_WORK_GROUP_SIZE;
	parent->max_work_item_dimensions = 3;
	parent->max_work_item_sizes[0] = X86_MAX_WORK_GROUP_SIZE;
	parent->max_work_item_sizes[1] = X86_MAX_WORK_GROUP_SIZE;
	parent->max_work_item_sizes[2] = X86_MAX_WORK_GROUP_SIZE;
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

	/* Initialize threads */
	device->threads = xcalloc(device->num_cores, sizeof(pthread_t));
	for (i = 0; i < device->num_cores - 1; i++)
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
	opencl_x86_device_core_init(&device->queue_core);

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
