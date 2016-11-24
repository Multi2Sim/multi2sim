#include <assert.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "debug.h"
#include "event.h"
#include "kernel.h"
#include "list.h"
#include "mhandle.h"
#include "misc.h"
#include "program.h"
#include "union-device.h"
#include "union-kernel.h"

struct dispatch_info
{
	pthread_mutex_t *lock;
	pthread_barrier_t *barrier;

	int id;
	struct opencl_device_t *device;
	struct opencl_union_ndrange_t *ndrange;
	struct opencl_event_t *event;
	void *arch_kernel;
};

/* Some of the dynamic scheduling algorithms might try to schedule a zero-size NDRange.
   Better to just detect this in general */
int device_ndrange_has_work(unsigned int dims, const unsigned int *group_count)
{
	int i;
	for (i = 0; i < dims; i++)
		if (group_count[i] == 0)
			return 0;
	return 1;
}

void *device_ndrange_dispatch(void *ptr)
{
	struct dispatch_info *info = (struct dispatch_info *)ptr;
	struct opencl_union_ndrange_t *ndrange = info->ndrange;
	struct timespec start, end;
	struct sched_param sched_param_old;
	struct sched_param sched_param_new;

	void *arch_ndrange;

	cl_ulong cltime;

	int sched_policy_new;
	int sched_policy_old;
	int work_groups_executed = 0;

	unsigned int start_group;
	unsigned int num_groups_to_exec;
	unsigned int preferred_groups;

	assert(ptr);
	assert(info->lock);
	assert(info->device);
	assert(info->arch_kernel);
	assert(info->ndrange);

	/* Store old scheduling policy and priority */
	pthread_getschedparam(pthread_self(), &sched_policy_old, 
		&sched_param_old);

	/* Give dispatch threads the highest priority */
	sched_policy_new = SCHED_RR;
	/* Sometimes the SI scheduler thread will get scheduled with the
	 * x86 scheduler thread and starve while the x86 thread runs 
	 * work-groups.  This is an attempt to fix that */
	/* FIXME This is not scalable when new device types are added */
	if (info->id == 0) /* SI */
		sched_param_new.sched_priority = sched_get_priority_max(
			sched_policy_new);
	else /* x86 */ 
		sched_param_new.sched_priority = sched_get_priority_max(
			sched_policy_new)-1;
	pthread_setschedparam(pthread_self(), sched_policy_new, 
		&sched_param_new);

	opencl_debug("[%s] global size = (%d,%d,%d)", __FUNCTION__,
		ndrange->global_work_size[0],
		ndrange->global_work_size[1],
		ndrange->global_work_size[2]);

	/* Create architecture-specific ND-Range */
	arch_ndrange = info->device->arch_ndrange_create_func(ndrange->parent,
		info->arch_kernel, ndrange->work_dim, 
		ndrange->global_work_offset, ndrange->global_work_size, 
		ndrange->local_work_size, 1);
	
	/* Initialize architecture-specific ND-Range */
	info->device->arch_ndrange_init_func(arch_ndrange);

	opencl_debug("[%s] creating union nd-range", __FUNCTION__);
	preferred_groups = info->device->arch_device_preferred_workgroups_func(
		info->device->arch_device);
	opencl_debug("[%s] %s preferred groups = %u", __FUNCTION__,
		info->device->name, preferred_groups);

	pthread_barrier_wait(info->barrier);

	/* Tell the driver that the nd-range has started */
	if (!opencl_native_mode && info->id == 0)
	{
		// FIXME - What kind of communication do we need here?
		//ioctl(m2s_active_dev, SINDRangeStart);
	}

	/* Record the start time */
	if (info->event && info->id == 0)
	{
		clock_gettime(CLOCK_MONOTONIC, &start);
	}
	pthread_barrier_wait(info->barrier);

	/* Execute work groups until the ND-Range is complete */
	pthread_mutex_lock(info->lock);
	while (ndrange->next_group < ndrange->total_num_groups)
	{
		/* Determine the number of groups to give this device */
		start_group = ndrange->next_group;
		num_groups_to_exec = MIN(preferred_groups,
			ndrange->total_num_groups - ndrange->next_group);
		/*
		opencl_debug("[%s] start group = %d, num groups to exec = %d",
			__FUNCTION__, start_group, num_groups_to_exec);
		*/

		/* Update the group counter */
		ndrange->next_group += num_groups_to_exec;

		pthread_mutex_unlock(info->lock);

		opencl_debug("[%s] running work groups %d to %d"
			" on device %s", __FUNCTION__,
			start_group, start_group + num_groups_to_exec - 1,
			info->device->name);

		/*
		cltime = opencl_get_time();
		opencl_debug("[%s] %s ndrange partial start = %lld", 
			__FUNCTION__, info->device->name, cltime);
		*/

		info->device->arch_ndrange_run_partial_func(arch_ndrange,
			start_group, num_groups_to_exec);

		work_groups_executed += num_groups_to_exec;

		/*
		cltime = opencl_get_time();
		opencl_debug("[%s] %s ndrange partial end = %lld", 
			__FUNCTION__, info->device->name, cltime);
		*/

		pthread_mutex_lock(info->lock);
	}
	pthread_mutex_unlock(info->lock);

	/*
	cltime = opencl_get_time();
	opencl_debug("[%s] %s ndrange ready to call finish = %lld", 
		__FUNCTION__, info->device->name, cltime);
	*/
	/* All devices must sync before one tries to call flush.  
	 * Otherwise weird stuff may happen. */
	pthread_barrier_wait(info->barrier);

	/* opencl_debug("[%s] calling nd-range finish", __FUNCTION__); */
	info->device->arch_ndrange_finish_func(arch_ndrange);

	/* Record the end time */
	pthread_barrier_wait(info->barrier);
	if (info->event && info->id == 0)
	{
		clock_gettime(CLOCK_MONOTONIC, &end);

		cltime = (cl_ulong)start.tv_sec;
		cltime *= 1000000000;
		cltime += (cl_ulong)start.tv_nsec;
		info->event->time_start = cltime;

		cltime = (cl_ulong)end.tv_sec;
		cltime *= 1000000000;
		cltime += (cl_ulong)end.tv_nsec;
		info->event->time_end = cltime;
	}

	/* Tell the driver that the nd-range has completed */
	if (!opencl_native_mode && info->id == 0)
	{
		// FIXME - What kind of communication do we need here?
		//ioctl(m2s_active_dev, SINDRangeEnd);
	}

	pthread_barrier_wait(info->barrier);

	opencl_debug("[%s] Device %s ran %d work-groups",
		__FUNCTION__, info->device->name, work_groups_executed);

	/* opencl_debug("[%s] calling nd-range free", __FUNCTION__); */
	info->device->arch_ndrange_free_func(arch_ndrange);

	/* Reset old scheduling parameters */
	pthread_setschedparam(pthread_self(), sched_policy_old, 
		&sched_param_old);

	opencl_debug("[%s] device '%s' done", __FUNCTION__, info->device->name);

	return NULL;
}

void opencl_union_ndrange_run_partial(struct opencl_union_ndrange_t *ndrange, 
	unsigned int *work_group_start, unsigned int *work_group_count)
{
	fatal("%s: This function should never be called\n", __FUNCTION__);
}

void opencl_union_ndrange_run(struct opencl_union_ndrange_t *ndrange,
	struct opencl_event_t *event)
{
	struct dispatch_info *info;
	struct list_t *device_list;

	pthread_t *threads;
	pthread_mutex_t lock;
	pthread_barrier_t barrier;

	int i, j;
	int num_devices;

	opencl_debug("[%s] global work size = %d,%d,%d", __FUNCTION__, 
		ndrange->global_work_size[0], ndrange->global_work_size[1], 
		ndrange->global_work_size[2]);

	assert(ndrange->type == opencl_runtime_type_union);

	device_list = ndrange->kernel->program->device->devices;
	num_devices = list_count(device_list);

	assert(list_count(ndrange->arch_kernels) == num_devices);
	opencl_debug("[%s] num fused sub-devices = %d", __FUNCTION__, 
		num_devices);

	threads = xcalloc(num_devices - 1, sizeof (pthread_t));
	info = xcalloc(num_devices, sizeof (struct dispatch_info));
	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, num_devices);

	for (i = 0; i < num_devices; i++)
	{
		/* j is a hack to get the GPU to be scheduled before the CPU */
		j = num_devices - 1 - i;
		info[i].ndrange = ndrange;
		info[i].device = list_get(device_list, j);
		info[i].arch_kernel = list_get(ndrange->arch_kernels, j);
		opencl_debug("[%s] ndrange->kernel[%d] = %p", 
			__FUNCTION__, i, info[i].arch_kernel);
		info[i].lock = &lock;
		info[i].barrier = &barrier;
		info[i].id = i;
		info[i].event = event;
		
		if (i != num_devices - 1)
		{
			opencl_debug("[%s] dispatching %s as new thread",
				__FUNCTION__, info[i].device->name);
			pthread_create(threads + i, NULL, 
				device_ndrange_dispatch, info + i);
		}
		else
		{
			opencl_debug("[%s] dispatching %s as main thread",
				__FUNCTION__, info[i].device->name);
			device_ndrange_dispatch(info + i);
		}
	}

	for (i = 0; i < num_devices - 1; i++)
	{
		opencl_debug("[%s] thread %d complete", __FUNCTION__, i);
		pthread_join(threads[i], NULL);
	}
	
	pthread_mutex_destroy(&lock);
	pthread_barrier_destroy(&barrier);
	free(info);
	free(threads);
}

struct opencl_union_kernel_t *opencl_union_kernel_create(
		struct opencl_kernel_t *parent,
		struct opencl_union_program_t *program,
		char *func_name)
{
	int i;
	struct opencl_union_kernel_t *kernel;
	struct list_t *devices;

	opencl_debug("[%s] creating union kernel", __FUNCTION__);

	devices = program->device->devices;
	kernel = xcalloc(1, sizeof (struct opencl_union_kernel_t));
	kernel->type = opencl_runtime_type_union;
	kernel->parent = parent;
	kernel->program = program;
	kernel->arch_kernels = list_create();

	LIST_FOR_EACH(devices, i)
	{
		struct opencl_device_t *subdevice = list_get(devices, i);
		opencl_debug("[%s] creating kernel for '%s' ", __FUNCTION__,
			subdevice->name);
		list_add(kernel->arch_kernels, 
			subdevice->arch_kernel_create_func(NULL, 
				list_get(program->programs, i), func_name));
	}

	return kernel;
}

void opencl_union_kernel_free(struct opencl_union_kernel_t *kernel)
{
	assert(kernel->type == opencl_runtime_type_union);

	struct list_t *device_list;
	struct opencl_device_t *device;
	
	int i;

	assert(kernel);
	assert(kernel->arch_kernels);

	opencl_debug("[%s] releasing union kernel resources", __FUNCTION__);

	device_list = kernel->program->device->devices;

	assert(list_count(device_list) == list_count(kernel->arch_kernels));
	opencl_debug("[%s] there are %d sub-kernels", __FUNCTION__, 
		list_count(device_list));

	/* Free the kernel for each sub-device */
	LIST_FOR_EACH(device_list, i)
	{
		device = list_get(device_list, i);
		device->arch_kernel_free_func(
			list_get(kernel->arch_kernels, i));
	}

	/* Free the union kernel */
	list_free(kernel->arch_kernels);
	free(kernel);
}

int opencl_union_kernel_set_arg(struct opencl_union_kernel_t *kernel,
	int arg_index, unsigned int arg_size, void *arg_value)
{
	int i;
	struct list_t *device_list;

	assert(kernel->type == opencl_runtime_type_union);
	device_list = kernel->program->device->devices;
	LIST_FOR_EACH(device_list, i)
	{
		struct opencl_device_t *subdevice = list_get(device_list, i);
		if (subdevice->arch_kernel_set_arg_func(
			list_get(kernel->arch_kernels, i), arg_index, 
			arg_size, arg_value))
		{
			return 1;
		}
	}
	return 0;
}

struct opencl_union_ndrange_t *opencl_union_ndrange_create(
	struct opencl_ndrange_t *ndrange,
	struct opencl_union_kernel_t *union_kernel,
	unsigned int work_dim, unsigned int *global_work_offset,
	unsigned int *global_work_size, unsigned int *local_work_size,
	unsigned int fused)
{
	struct list_t *devices;
	//struct opencl_device_t *subdevice;
	struct opencl_union_ndrange_t *union_ndrange;
	
	void *subkernel;

	int i;

	opencl_debug("[%s] creating union nd-range", __FUNCTION__);

	assert(ndrange);
	assert(union_kernel);

	union_ndrange = (struct opencl_union_ndrange_t *)xcalloc(1, 
		sizeof(struct opencl_union_ndrange_t));
	union_ndrange->type = opencl_runtime_type_union;
	union_ndrange->parent = ndrange;
	union_ndrange->kernel = union_kernel;
	union_ndrange->work_dim = work_dim;
	union_ndrange->arch_kernels = list_create();

	/* Make sure the number of devices match the number of kernels */
	devices = union_kernel->program->device->devices;
	assert(list_count(devices) == list_count(union_kernel->arch_kernels));
	opencl_debug("[%s] union nd-range has %d kernels", __FUNCTION__,
		list_count(devices));

	LIST_FOR_EACH(devices, i)
	{
		subkernel = list_get(union_kernel->arch_kernels, i);
		list_add(union_ndrange->arch_kernels, subkernel);
		opencl_debug("[%s] union nd-range kernel %d: %p ", __FUNCTION__,
			i, subkernel);
	}

	/* Work sizes */
	for (i = 0; i < work_dim; i++)
	{
		union_ndrange->global_work_offset[i] = global_work_offset ?
			global_work_offset[i] : 0;
		union_ndrange->global_work_size[i] = global_work_size[i];
		union_ndrange->local_work_size[i] = local_work_size ?
			local_work_size[i] : 1;
		assert(!(global_work_size[i] % 
			union_ndrange->local_work_size[i]));
		union_ndrange->num_groups[i] = global_work_size[i] / 
			union_ndrange->local_work_size[i];
	}

	/* Unused dimensions */
	for (i = work_dim; i < 3; i++)
	{
		union_ndrange->global_work_offset[i] = 0;
		union_ndrange->global_work_size[i] = 1;
		union_ndrange->local_work_size[i] = 1;
		union_ndrange->num_groups[i] = 
			union_ndrange->global_work_size[i] / 
			union_ndrange->local_work_size[i];
	}

	/* Calculate the number of work groups in the ND-Range */
	union_ndrange->total_num_groups = 
		union_ndrange->num_groups[0] * 
		union_ndrange->num_groups[1] * 
		union_ndrange->num_groups[2];

	return union_ndrange;
}

void opencl_union_ndrange_init(struct opencl_union_ndrange_t *ndrange)
{
	/* Each architecture-specific nd-range is initialized
	 * within the run function */
	opencl_debug("[%s] empty", __FUNCTION__);
}

void opencl_union_ndrange_finish(struct opencl_union_ndrange_t *ndrange)
{
	/* Each architecture-specific nd-range is finished 
	 * within the run function */
	opencl_debug("[%s] empty", __FUNCTION__);
}

void opencl_union_ndrange_free(struct opencl_union_ndrange_t *ndrange)
{
	assert(ndrange);
	assert(ndrange->arch_kernels);

	opencl_debug("[%s] releasing union nd-range resources", __FUNCTION__);

	/* ND-Ranges of sub-devices are freed immediately 
	 * after they execute */

	/* Free the union ND-Range */
	list_free(ndrange->arch_kernels);
	free(ndrange);
}
