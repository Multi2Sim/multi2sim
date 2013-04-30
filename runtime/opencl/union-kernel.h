#ifndef __UNION_KERNEL_H__
#define __UNION_KERNEL_H__

#include "list.h"
#include "opencl.h"
#include "union-device.h"
#include "union-program.h"
#include "partition-strategy.h"

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
		struct opencl_union_kernel_t *kernel,
		int work_dim,
		unsigned int *global_work_offset,
		unsigned int *global_work_size,
		unsigned int *local_work_size,
		unsigned int *group_id_offset,
		unsigned int *group_count);

void opencl_union_kernel_free(
		struct opencl_union_kernel_t *kernel);

int opencl_union_kernel_set_arg(
		struct opencl_union_kernel_t *kernel,
		int arg_index,
		unsigned int arg_size,
		void *arg_value);
#endif
