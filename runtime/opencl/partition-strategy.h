#ifndef __PARTITION_STRATEGY_H__
#define __PARTITION_STRATEGY_H__

typedef void *(*opencl_strategy_create_t)(int num_devices, unsigned int dims, unsigned int *groups);
typedef int (*opencl_strategy_get_partition_t)(void *inst, int desired_groups, unsigned int *group_offset, unsigned int *group_count);
typedef void (*opencl_strategy_destroy_t)(void *inst);

struct opencl_partition_strategy
{
	opencl_strategy_create_t create;
	opencl_strategy_get_partition_t get_partition;
	opencl_strategy_destroy_t destroy;
};

struct opencl_partition_strategy *get_strategy();

#endif
