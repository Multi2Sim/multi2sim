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
