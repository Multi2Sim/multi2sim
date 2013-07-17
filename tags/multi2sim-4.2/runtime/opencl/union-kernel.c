#include <assert.h>
#include <pthread.h>

#include "debug.h"
#include "kernel.h"
#include "list.h"
#include "mhandle.h"
#include "misc.h"
#include "program.h"
#include "union-device.h"
#include "union-kernel.h"

struct dispatch_info
{
	void *part;
	pthread_mutex_t *lock;

	int id;
	struct opencl_device_t *device;
	struct opencl_union_ndrange_t *ndrange;
	void *arch_kernel;
};

void *device_ndrange_dispatch(void *ptr)
{
	int i;

	struct dispatch_info *info = (struct dispatch_info *)ptr;
	struct opencl_union_ndrange_t *ndrange = info->ndrange;

	void *arch_ndrange;

	assert(ptr);
	assert(info->part);
	assert(info->lock);
	assert(info->device);
	assert(info->arch_kernel);
	assert(info->ndrange);

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
	}
	opencl_debug("[%s] global size = (%d,%d,%d)", __FUNCTION__,
		ndrange->global_work_size[0],
		ndrange->global_work_size[1],
		ndrange->global_work_size[2]);
	opencl_debug("[%s] group count = (%d,%d,%d)", __FUNCTION__,
		group_count[0], group_count[1], group_count[2]);

	/* Create architecture-specific ND-Range */
	arch_ndrange = info->device->arch_ndrange_create_func(ndrange->parent,
		info->arch_kernel, ndrange->work_dim, 
		ndrange->global_work_offset, ndrange->global_work_size, 
		ndrange->local_work_size, 1);
	
	/* Initialize architecture-specific ND-Range */
	info->device->arch_ndrange_init_func(arch_ndrange);

	/* Execute work groups until the ND-Range is complete */
	pthread_mutex_lock(info->lock);
	while (get_strategy()->get_partition(
		info->part, 
		info->id,
		info->device->arch_device_preferred_workgroups_func(
			info->device), group_offset, group_count))
	{
		opencl_debug("[%s] running work groups (%d,%d,%d) to (%d,%d,%d)"
			" on device %s", __FUNCTION__,
			group_offset[0], group_offset[1], group_offset[2],
			group_offset[0]+group_count[0]-1,
			group_offset[1]+group_count[1]-1,
			group_offset[2]+group_count[2]-1,
			info->device->name);
		pthread_mutex_unlock(info->lock);

		info->device->arch_ndrange_run_partial_func(arch_ndrange,
			group_offset, group_count);

		opencl_debug("[%s] id %d. offset: %d.  count: %d", 
			__FUNCTION__, info->id, group_offset[0], 
			group_count[0]);

		pthread_mutex_lock(info->lock);
	}
	pthread_mutex_unlock(info->lock);

	opencl_debug("[%s] calling nd-range finish", __FUNCTION__);
	/* Initialize architecture-specific ND-Range */
	info->device->arch_ndrange_free_func(arch_ndrange);

	free(group_offset);
	free(group_count);

	opencl_debug("[%s] device '%s' done", __FUNCTION__, info->device->name);

	return NULL;
}

void opencl_union_ndrange_run_partial(struct opencl_union_ndrange_t *ndrange, 
	unsigned int *work_group_start, unsigned int *work_group_count)
{
	fatal("%s: This function should never be called\n", __FUNCTION__);
}

void opencl_union_ndrange_run(struct opencl_union_ndrange_t *ndrange)
{
	struct dispatch_info *info;
	struct list_t *device_list;

	pthread_t *threads;
	pthread_mutex_t lock;

	int i;
	int num_devices;

	void *part;

	opencl_debug("[%s] global work size = %d,%d,%d", __FUNCTION__, 
		ndrange->global_work_size[0], ndrange->global_work_size[1], 
		ndrange->global_work_size[2]);

	assert(ndrange->type == opencl_runtime_type_union);

	device_list = ndrange->kernel->program->device->devices;
	num_devices = list_count(device_list);

	assert(list_count(ndrange->arch_kernels) == num_devices);
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
		info[i].device = list_get(device_list, i);
		info[i].arch_kernel = list_get(ndrange->arch_kernels, i);
		opencl_debug("[%s] ndrange->kernel[%d] = %p", 
			__FUNCTION__, i, info[i].arch_kernel);
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
	kernel->type = opencl_runtime_type_union;
	kernel->parent = parent;
	//kernel->device = program->device;
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
		union_ndrange->group_count[i] = global_work_size[i] / 
			union_ndrange->local_work_size[i];
	}

	/* Unused dimensions */
	for (i = work_dim; i < 3; i++)
	{
		union_ndrange->global_work_offset[i] = 0;
		union_ndrange->global_work_size[i] = 1;
		union_ndrange->local_work_size[i] = 1;
		union_ndrange->group_count[i] = 
			union_ndrange->global_work_size[i] / 
			union_ndrange->local_work_size[i];
	}

	/* Calculate the number of work groups in the ND-Range */
	union_ndrange->num_groups = union_ndrange->group_count[0] * 
		union_ndrange->group_count[1] * union_ndrange->group_count[2];

	return union_ndrange;
}

void opencl_union_ndrange_init(struct opencl_union_ndrange_t *ndrange)
{
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
