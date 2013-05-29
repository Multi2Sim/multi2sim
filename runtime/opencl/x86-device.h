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

#ifndef RUNTIME_OPENCL_X86_DEVICE_H
#define RUNTIME_OPENCL_X86_DEVICE_H

#include <pthread.h>

#include "opencl.h"


typedef void (*opencl_x86_device_fiber_func_t)(void);
typedef void (*opencl_x86_device_fiber_return_func_t)(void);


struct opencl_x86_device_fiber_t
{
	void *esp;
	void *eip;
	void *stack_bottom;
	unsigned int stack_size;
};


struct opencl_x86_device_exec_t
{
	struct opencl_x86_kernel_t *kernel;

	int dims;
	size_t *global;
	size_t *local;

	pthread_mutex_t mutex;
	int num_groups;
	volatile int next_group;
	size_t *group_starts;
	size_t *group_ids;
};


struct opencl_x86_device_work_item_data_t
{
	int work_group_data;  /* 0x60 (Not actually part of AMD runtime, padding_0) */
	int barrier_func;  /* 0x5c (function *) */
	int local_reserved;  /* 0x58 (void *) */
	int work_dim;  /* 0x54 */
	int group_global[4];  /* [0x50, 0x44] */
	int global_size[4];  /* [0x40, 0x34] */
	int local_size[4];  /* [0x30, 0x24] */
	int group_id[4];  /* [0x20, 0x14] */
	int global_id[4];  /* [0x10, 0x04] */
};


struct opencl_x86_device_work_group_data_t
{
	int num_done;
	int num_items;
	int current_item;

	struct opencl_x86_device_fiber_t main_fiber;
	struct opencl_x86_device_fiber_t *current_fiber;
	struct opencl_x86_device_fiber_t *work_items;

	struct opencl_x86_device_work_item_data_t **work_item_data;

	unsigned int *stack_params;
	char *aligned_stacks;
};


struct opencl_x86_device_t
{
	enum opencl_runtime_type_t type;  /* First field */

	/* Parent generic device object */
	struct opencl_device_t *parent;

	volatile int num_kernels;
	volatile int num_done;

	volatile int num_cores;
	pthread_t *threads;

	pthread_mutex_t lock;
	pthread_cond_t ready;
	pthread_cond_t done;

	struct opencl_x86_device_exec_t *exec;
};



struct opencl_x86_device_t *opencl_x86_device_create(struct opencl_device_t *parent);
void opencl_x86_device_free(struct opencl_x86_device_t *device);
int opencl_x86_device_preferred_workgroups(struct opencl_x86_device_t *device);

void *opencl_x86_device_mem_alloc(struct opencl_x86_device_t *device,
		unsigned int size);
void opencl_x86_device_mem_free(struct opencl_x86_device_t *device,
		void *ptr);
void opencl_x86_device_mem_read(struct opencl_x86_device_t *device,
		void *host_ptr, void *device_ptr, unsigned int size);
void opencl_x86_device_mem_write(struct opencl_x86_device_t *device,
		void *device_ptr, void *host_ptr, unsigned int size);
void opencl_x86_device_mem_copy(struct opencl_x86_device_t *device,
		void *device_dest_ptr, void *device_src_ptr,
		unsigned int size);

void opencl_x86_device_make_fiber(
		struct opencl_x86_device_fiber_t *fiber,
		opencl_x86_device_fiber_func_t fiber_func,
		int num_args,
		...);
void opencl_x86_device_make_fiber_ex(
		struct opencl_x86_device_fiber_t *fiber,
		opencl_x86_device_fiber_func_t fiber_func,
		opencl_x86_device_fiber_return_func_t return_func,
		int arg_size,
		void *args);
void opencl_x86_device_switch_fiber(
		volatile struct opencl_x86_device_fiber_t *current,
		volatile struct opencl_x86_device_fiber_t *dest,
		volatile void *reg_values);
void opencl_x86_device_exit_fiber(void);
void opencl_x86_device_barrier(int data);

void *opencl_x86_device_core_func(struct opencl_x86_device_t *device);


#endif
