#ifndef __UNION_KERNEL_H__
#define __UNION_KERNEL_H__

#include "opencl.h"

void opencl_union_kernel_run(
		struct opencl_kernel_t *kernel,
		int work_dim,
		unsigned int *global_work_offset,
		unsigned int *global_work_size,
		unsigned int *local_work_size,
		unsigned int *group_id_offset);

typedef void *(*opencl_strategy_create_t)(int num_devices, unsigned int dims, unsigned int *groups);
typedef int (*opencl_strategy_get_partition_t)(void *inst, int desired_groups, unsigned int *group_offset, unsigned int *group_count);

struct opencl_partition_strategy
{
	opencl_strategy_create_t create;
	opencl_strategy_get_partition_t get_partition;
};

#endif
