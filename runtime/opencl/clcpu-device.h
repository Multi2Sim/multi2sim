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

#ifndef RUNTIME_OPENCL_CLCPU_DEVICE_H
#define RUNTIME_OPENCL_CLCPU_DEVICE_H

#include "fibers.h"

typedef void (*clrt_barrier_t)(int option);

struct clcpu_execution_t
{
	struct clcpu_kernel_t *kernel;
	int dims;
	const size_t *global;
	const size_t *local;

	pthread_mutex_t mutex;
	int num_groups;
	volatile int next_group;
	size_t *group_starts;
};


struct clcpu_device_t
{
	volatile int num_kernels;
	volatile int num_done;

	volatile int num_cores;
	pthread_t *threads;

	pthread_mutex_t lock;
	pthread_cond_t ready;
	pthread_cond_t done;

	struct clcpu_execution_t *exec;
};


struct clcpu_workitem_data_t
{
	int workgroup_data;  /* 0x60 (Not actually part of AMD runtime, padding_0) */
	int barrier_func;  /* 0x5c (function *) */
	int local_reserved;  /* 0x58 (void *) */
	int work_dim;  /* 0x54 */
	int group_global[4];  /* [0x50, 0x44] */
	int global_size[4];  /* [0x40, 0x34] */
	int local_size[4];  /* [0x30, 0x24] */
	int group_id[4];  /* [0x20, 0x14] */
	int global_id[4];  /* [0x10, 0x04] */
};


struct clcpu_workgroup_data_t
{
	int num_done;
	int num_items;
	int cur_item;
	struct fiber_t main_ctx;
	struct fiber_t *cur_ctx;
	struct fiber_t *workitems;
	struct clcpu_workitem_data_t **workitem_data;
	unsigned int *stack_params;
	char *aligned_stacks;
};


#endif

