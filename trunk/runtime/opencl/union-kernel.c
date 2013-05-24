#include <assert.h>
#include <pthread.h>

#include "kernel.h"
#include "list.h"
#include "mhandle.h"
#include "misc.h"
#include "si-kernel.h" ///////////////////////////////////
#include "union-device.h"
#include "union-kernel.h"

struct dispatch_info
{
	void *part;
	pthread_mutex_t *lock;

	int id;
	struct opencl_device_t *device;
	void *arch_kernel;
	struct opencl_ndrange_t *ndrange;
};

void *device_ndrange_dispatch(void *ptr)
{
	int i;

	struct dispatch_info *info = (struct dispatch_info *)ptr;
	struct opencl_ndrange_t *ndrange = info->ndrange;

	void *arch_ndrange;

	if (info->id == 1)
	{
		struct opencl_si_kernel_t *si_kernel = (struct opencl_si_kernel_t *)info->arch_kernel; ///////////
		opencl_debug("[%s] kernel id is %d",
			__FUNCTION__, si_kernel->id); ////////////
	}

	/* Allocate the maximum number of dimensions */
	unsigned int *group_offset = xcalloc(3, sizeof (unsigned int));
	unsigned int *group_count = xcalloc(3, sizeof (unsigned int));

	/* Initialize to reasonable values */
	for (i = 0; i < 3; i++)
	{
		group_offset[i] = 0;
		group_count[i] = (ndrange->global_work_size[i] /
			ndrange->local_work_size[i]) - 
			(ndrange->global_work_offset[i] /
			ndrange->local_work_size[i]);
		opencl_debug("[%s] global size %d = %d", __FUNCTION__, i, 
			ndrange->global_work_size[i]);
		opencl_debug("[%s] work group count %d = %d", __FUNCTION__, i, 
			group_count[i]);
	}

	arch_ndrange = info->device->arch_ndrange_create_func(ndrange, 
		info->arch_kernel);
	ndrange->arch_ndrange = arch_ndrange;

	info->device->arch_ndrange_init_func(ndrange);

	pthread_mutex_lock(info->lock);
	while (get_strategy()->get_partition(
		info->part, 
		info->id,
		info->device->arch_device_preferred_workgroups_func(
			info->device), group_offset, group_count))
	{
		opencl_debug("[%s] running work groups %d:%d on device %s",
			__FUNCTION__,
			group_offset[1]*group_count[0] + group_offset[0], 
			group_offset[1]*group_count[0] + group_offset[0] + 
			group_count[0] - 1, info->device->name);
		pthread_mutex_unlock(info->lock);

		info->device->arch_ndrange_run_partial_func(ndrange,
			group_offset, group_count);

		opencl_debug("[%s] id %d. offset: %d.  count: %d", 
			__FUNCTION__, info->id, group_offset[0], 
			group_count[0]);

		pthread_mutex_lock(info->lock);
	}
	pthread_mutex_unlock(info->lock);

	opencl_debug("[%s] calling nd-range finish", __FUNCTION__);
	info->device->arch_ndrange_free_func(ndrange);

	free(group_offset);
	free(group_count);

	opencl_debug("[%s] device '%s' done", __FUNCTION__, info->device->name);

	return NULL;
}

void opencl_union_ndrange_run(struct opencl_ndrange_t *ndrange)
{
	struct dispatch_info *info;
	struct opencl_union_ndrange_t *union_ndrange;

	pthread_t *threads;
	pthread_mutex_t lock;

	int i;
	int num_devices;

	void *part;

	ndrange->fused = 1;

	opencl_debug("[%s] global work size = %d,%d,%d", __FUNCTION__, 
		ndrange->global_work_size[0], ndrange->global_work_size[1], 
		ndrange->global_work_size[2]);

	union_ndrange = ndrange->arch_ndrange;
	assert(union_ndrange);

	num_devices = list_count(union_ndrange->devices);
	assert(list_count(union_ndrange->arch_kernels) == num_devices);
	opencl_debug("[%s] num fused sub-devices = %d", __FUNCTION__, 
		num_devices);

	part = get_strategy()->create(num_devices, ndrange->work_dim, 
		ndrange->group_count);
	threads = xcalloc(num_devices - 1, sizeof (pthread_t));
	info = xcalloc(num_devices, sizeof (struct dispatch_info));
	pthread_mutex_init(&lock, NULL);

	for (i = 0; i < num_devices; i++)
	{
		info[i].ndrange = ndrange;
		info[i].part = part;
		info[i].device = list_get(union_ndrange->devices, i);
		info[i].arch_kernel = list_get(union_ndrange->arch_kernels, i);
		info[i].lock = &lock;
		info[i].id = i;
		
		if (i != num_devices - 1)
		{
			opencl_debug("[%s] dispatching %s as new thread",
				__FUNCTION__, info[i].device->name);
			pthread_create(threads + i, NULL, 
				device_ndrange_dispatch, info + i);
		}
		else
		{
			opencl_debug("[%s] device address is %p",
				__FUNCTION__, info[i].device); ////////
			opencl_debug("[%s] dispatching %s as main thread",
				__FUNCTION__, info[i].device->name);
			struct opencl_si_kernel_t *si_kernel = (struct opencl_si_kernel_t *)info[i].arch_kernel; ///////////
			opencl_debug("[%s] kernel id is %d",
				__FUNCTION__, si_kernel->id); ////////////
			device_ndrange_dispatch(info + i);
		}
	}

	for (i = 0; i < num_devices - 1; i++)
	{
		opencl_debug("[%s] thread %d complete", __FUNCTION__, i);
		pthread_join(threads[i], NULL);
	}
	
	pthread_mutex_destroy(&lock);
	get_strategy()->destroy(part);
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
	kernel->parent = parent;
	kernel->device = program->device;
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
	/* FIXME need to free stuff */
}

int opencl_union_kernel_set_arg(
		struct opencl_union_kernel_t *kernel,
		int arg_index,
		unsigned int arg_size,
		void *arg_value)
{
	int i;
	LIST_FOR_EACH(kernel->device->devices, i)
	{
		struct opencl_device_t *subdevice = list_get(kernel->device->devices, i);
		if (subdevice->arch_kernel_set_arg_func(list_get(kernel->arch_kernels, i), arg_index, arg_size, arg_value))
			return 1;
	}
	return 0;
}

struct opencl_union_ndrange_t *opencl_union_ndrange_create(
	struct opencl_ndrange_t *ndrange,
	struct opencl_union_kernel_t *union_kernel)
{
	struct opencl_union_ndrange_t *union_ndrange;
	struct opencl_device_t *subdevice;
	void *arch_kernel;

	int i;
	int num_devices;
	int num_kernels;

	opencl_debug("[%s] creating union nd-range", __FUNCTION__);

	assert(ndrange);
	assert(union_kernel);

	union_ndrange = (struct opencl_union_ndrange_t *)xcalloc(1, 
		sizeof(struct opencl_union_ndrange_t));
	union_ndrange->kernel = union_kernel;
	union_ndrange->devices = list_create();
	union_ndrange->arch_kernels = list_create();

	union_ndrange->parent = ndrange;

	num_devices = list_count(union_kernel->device->devices);
	num_kernels = list_count(union_kernel->arch_kernels);
	assert(num_devices == num_kernels);

	for (i = 0; i < num_devices; i++)
	{
		subdevice = (struct opencl_device_t *)
			list_get(union_kernel->device->devices, i);
		list_add(union_ndrange->devices, subdevice);
		opencl_debug("[%s] union nd-range device %d: %p ", __FUNCTION__,
			i, subdevice);

		arch_kernel = list_get(union_kernel->arch_kernels, i);
		list_add(union_ndrange->arch_kernels, arch_kernel);
		opencl_debug("[%s] union nd-range kernel %d: %p ", __FUNCTION__,
			i, arch_kernel);
	}

	opencl_debug("[%s] union nd-range has %d kernels", __FUNCTION__,
		list_count(union_kernel->arch_kernels));

	return union_ndrange;

}

