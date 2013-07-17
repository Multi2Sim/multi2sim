#ifndef __UNION_KERNEL_H__
#define __UNION_KERNEL_H__

#include "list.h"
#include "opencl.h"
#include "union-device.h"
#include "union-program.h"
#include "partition-strategy.h"

struct opencl_union_kernel_t
{
	enum opencl_runtime_type_t type;  /* First field */

	struct opencl_kernel_t *parent;
	struct opencl_union_program_t *program;

	/* list of architecture-specific kernels */
	struct list_t *arch_kernels;
};

struct opencl_union_ndrange_t
{
	enum opencl_runtime_type_t type;  /* First field */

	struct opencl_ndrange_t *parent;	
	struct opencl_union_kernel_t *kernel;

	struct list_t *arch_kernels;

	int work_dim;

	unsigned int global_work_offset[3];
	unsigned int global_work_size[3];
	unsigned int local_work_size[3];

	unsigned int group_count[3];
	unsigned int num_groups;
};


/* Kernel functions */
struct opencl_union_kernel_t *opencl_union_kernel_create(
	struct opencl_kernel_t *parent,
	struct opencl_union_program_t *program,
	char *func_name);

void opencl_union_kernel_free(
	struct opencl_union_kernel_t *kernel);

int opencl_union_kernel_set_arg(
	struct opencl_union_kernel_t *kernel,
	int arg_index,
	unsigned int arg_size,
	void *arg_value);


/* ND-Range functions */
struct opencl_union_ndrange_t *opencl_union_ndrange_create(
	struct opencl_ndrange_t *ndrange, 
	struct opencl_union_kernel_t *union_kernel,
	unsigned int work_dim, unsigned int *global_work_offset,
	unsigned int *global_work_size, unsigned int *local_work_size, 
	unsigned int fused);

void opencl_union_ndrange_free(
	struct opencl_union_ndrange_t *ndrange);

void opencl_union_ndrange_init(
	struct opencl_union_ndrange_t *ndrange);

void opencl_union_ndrange_run(
	struct opencl_union_ndrange_t *ndrange);

void opencl_union_ndrange_run_partial(struct opencl_union_ndrange_t *ndrange, 
	unsigned int *work_group_start, unsigned int *work_group_count);

#endif
