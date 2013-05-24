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

	/* list of architecture-specific kernels */
	struct list_t *arch_kernels;
};

struct opencl_union_ndrange_t
{
	struct opencl_ndrange_t *parent;	
	struct opencl_union_kernel_t *kernel;

	/* items in these two lists must correspond */
	struct list_t *arch_kernels;
	struct list_t *devices;  /* type 'opencl_device_t*' */
};

struct opencl_union_kernel_t *opencl_union_kernel_create(
	struct opencl_kernel_t *parent,
	struct opencl_union_program_t *program,
	char *func_name);

void opencl_x86_kernel_free(
	struct opencl_union_kernel_t *kernel);

struct opencl_union_ndrange_t *opencl_union_ndrange_create(
	struct opencl_ndrange_t *ndrange,
	struct opencl_union_kernel_t *union_kernel);

void opencl_union_ndrange_free(
	struct opencl_ndrange_t *ndrange);

void opencl_union_ndrange_run(
	struct opencl_ndrange_t *ndrange);

void opencl_union_kernel_free(
	struct opencl_union_kernel_t *kernel);

int opencl_union_kernel_set_arg(
	struct opencl_union_kernel_t *kernel,
	int arg_index,
	unsigned int arg_size,
	void *arg_value);

#endif
