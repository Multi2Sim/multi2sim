#include <pthread.h>

#include "kernel.h"
#include "mhandle.h"
#include "list.h"
#include "union-kernel.h"

struct opencl_partition_strategy *strat = NULL;


struct dispatch_info
{
	int work_dim;
	unsigned int *global_work_offset;
	unsigned int *global_work_size;
	unsigned int *local_work_size;
	unsigned int *group_id_offset;
	void *part;
};

void *device_kernel_dispatch(void *ptr)
{
	struct dispatch_info *info = (struct dispatch_info *)ptr;

	unsigned int *group_offset = xcalloc(info->work_dim, sizeof (unsigned int));
	unsigned int *group_count = xcalloc(info->work_dim, sizeof (unsigned int));
	
	while (strat->get_partition(info->part, 1, group_offset, group_count))
	{
		//TODO: pass the dispatcher the actual device object and get it to launch kernels.
	}

	free(group_offset);
	free(group_count);
	return NULL;
}

void opencl_union_kernel_run(
		struct opencl_kernel_t *kernel,
		int work_dim,
		unsigned int *global_work_offset,
		unsigned int *global_work_size,
		unsigned int *local_work_size,
		unsigned int *group_id_offset)
{
	int i;
	struct dispatch_info info;
	unsigned int *num_groups = xcalloc(work_dim, sizeof (unsigned int));

	for (i = 0; i < (int)num_groups; i++)
		num_groups[i] = global_work_size[i] / local_work_size[i];

	int num_devices = list_count(kernel->entry_list);
	void *part = strat->create(num_devices, work_dim, num_groups);

	info.work_dim = work_dim;
	info.global_work_offset = global_work_offset;
	info.global_work_size = global_work_size;
	info.local_work_size = local_work_size;
	info.group_id_offset = group_id_offset;
	info.part = part;

	pthread_t *threads = xcalloc(num_devices - 1, sizeof (pthread_t));
	
	for (i = 0; i < num_devices - 1; i++)
		pthread_create(threads + i, NULL, device_kernel_dispatch, &info);
	device_kernel_dispatch(&info);

	for (i = 0; i < num_devices - 1; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(num_groups);
}

struct opencl_union_kernel_t *opencl_union_kernel_create(
		struct opencl_kernel_t *parent,
		struct opencl_union_program_t *program,
		char *func_name)
{
	int i;
	struct opencl_union_kernel_t *kernel;
	struct list_t *devices;

	devices = program->device->devices;
	kernel = xcalloc(1, sizeof (struct opencl_union_kernel_t));
	kernel->parent = parent;
	kernel->device = program->device;
	kernel->kernels = list_create_with_size(list_count(devices));

	LIST_FOR_EACH(devices, i)
	{
		struct opencl_device_t *subdevice = list_get(devices, i);
		list_set(kernel->kernels, i, subdevice->arch_kernel_create_func(NULL, list_get(program->programs, i), func_name));
	}

	return kernel;
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
		if (subdevice->arch_kernel_set_arg_func(list_get(kernel->kernels, i), arg_index, arg_size, arg_value))
			return 1;
	}
	return 0;
}
