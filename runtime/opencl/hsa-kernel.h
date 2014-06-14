/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef HSA_KERNEL_H_
#define HSA_KERNEL_H_

#include "opencl.h"


/* Forward declarations */
struct opencl_hsa_device_t;
struct opencl_hsa_program_t;


enum opencl_hsa_arg_type_t
{
	opencl_hsa_arg_type_invalid = 0,
	opencl_hsa_arg_value,
	opencl_hsa_arg_pointer,
	opencl_hsa_arg_image,
	opencl_hsa_arg_sampler
};


struct opencl_hsa_arg_t
{
	enum opencl_hsa_arg_type_t type;
	char *name;
};


struct opencl_hsa_arg_t *opencl_hsa_arg_create(enum opencl_hsa_arg_type_t type,
		char *name);
void opencl_hsa_arg_free(struct opencl_hsa_arg_t *arg);




/*
 * Southern Islands Kernel
 */

/* Subclass of 'opencl_kernel_t' containing Southern Islands specific
 * information of the OpenCL kernel. */
struct opencl_hsa_kernel_t
{
	enum opencl_runtime_type_t type;  /* First field */

	/* Kernel object acting as parent object. */
	struct opencl_kernel_t *parent;

	/* Architecture-specific program and device associated */
	struct opencl_hsa_program_t *program;
	struct opencl_hsa_device_t *device;

	/* List of arguments */
	struct list_t *arg_list;

	/* ID in driver */
	int id;
};

struct opencl_hsa_ndrange_t
{
	int id;  /* ID in driver */

	enum opencl_runtime_type_t type;  /* First field */

	struct opencl_ndrange_t *parent;
	struct opencl_hsa_kernel_t *arch_kernel;

	// unsigned int fused;

	int work_dim;

	unsigned int global_work_offset[3];
	unsigned int global_work_size[3];
	unsigned int local_work_size[3];

	unsigned int num_groups[3];
	unsigned int total_num_groups;

	void *table_ptr;
	void *cb_ptr;
};

/* Kernel callbacks */
struct opencl_hsa_kernel_t *opencl_hsa_kernel_create(
	struct opencl_kernel_t *parent, struct opencl_hsa_program_t *program,
	char *func_name);

void opencl_hsa_kernel_debug(struct opencl_hsa_kernel_t *kernel);

int opencl_hsa_kernel_set_arg(struct opencl_hsa_kernel_t *kernel, int arg_index,
	unsigned int arg_size, void *arg_value);

void opencl_hsa_kernel_free(struct opencl_hsa_kernel_t *kernel);

/* ND-Range callbacks */
struct opencl_hsa_ndrange_t *opencl_hsa_ndrange_create(
	struct opencl_ndrange_t *ndrange, struct opencl_hsa_kernel_t * hsa_kernel,
	unsigned int work_dim, unsigned int *global_work_offset,
	unsigned int *global_work_size, unsigned int *local_work_size);

void opencl_hsa_ndrange_init(struct opencl_hsa_ndrange_t *ndrange);

void opencl_hsa_ndrange_run(struct opencl_hsa_ndrange_t *ndrange,
	struct opencl_event_t *event);

void opencl_hsa_ndrange_run_partial(struct opencl_hsa_ndrange_t *ndrange,
	unsigned int work_group_start, unsigned int work_group_count);

void opencl_hsa_ndrange_finish(struct opencl_hsa_ndrange_t *ndrange);

void opencl_hsa_ndrange_free(struct opencl_hsa_ndrange_t *ndrange);

#endif /* HSA_KERNEL_H_ */
