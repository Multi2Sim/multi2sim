#ifndef __UNION_KERNEL_H__
#define __UNION_KERNEL_H__

#include "list.h"
#include "opencl.h"
#include "union-device.h"
#include "union-program.h"

struct opencl_union_kernel_t
{
	struct opencl_kernel_t *parent;
	struct opencl_union_device_t *device;
	struct list_t *kernels;
};

struct opencl_union_kernel_t *opencl_union_kernel_create(
		struct opencl_kernel_t *parent,
		struct opencl_union_program_t *program,
		char *func_name);

void opencl_union_kernel_run(
		struct opencl_kernel_t *kernel,
		int work_dim,
		unsigned int *global_work_offset,
		unsigned int *global_work_size,
		unsigned int *local_work_size,
		unsigned int *group_id_offset);

void opencl_union_kernel_free(
		struct opencl_union_kernel_t *kernel);

int opencl_union_kernel_set_arg(
		struct opencl_union_kernel_t *kernel,
		int arg_index,
		unsigned int arg_size,
		void *arg_value);


typedef void *(*opencl_strategy_create_t)(int num_devices, unsigned int dims, unsigned int *groups);
typedef int (*opencl_strategy_get_partition_t)(void *inst, int desired_groups, unsigned int *group_offset, unsigned int *group_count);

struct opencl_partition_strategy
{
	opencl_strategy_create_t create;
	opencl_strategy_get_partition_t get_partition;
};

#endif
