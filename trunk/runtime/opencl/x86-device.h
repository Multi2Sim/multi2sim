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
#define X86_MAX_WORK_GROUP_SIZE 1024

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
	struct opencl_x86_ndrange_t *ndrange;

	unsigned int work_group_start[3];
	unsigned int work_group_count[3];

	int num_groups;
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_t next_group_lock;
#endif
	volatile int next_group;
};


struct opencl_x86_device_work_item_data_t
{
	unsigned int work_group_data;  /* 0x60 (Not actually part of AMD runtime, padding_0) */
	unsigned int barrier_func;  /* 0x5c (function *) */
	unsigned int local_reserved;  /* 0x58 (void *) */
	unsigned int work_dim;  /* 0x54 */
	unsigned int group_global[4];  /* [0x50, 0x44] */
	unsigned int global_size[4];  /* [0x40, 0x34] */
	unsigned int local_size[4];  /* [0x30, 0x24] */
	unsigned int group_id[4];  /* [0x20, 0x14] */
	unsigned int global_id[4];  /* [0x10, 0x04] */
};

struct opencl_x86_ndrange_t;

struct opencl_x86_device_core_t
{
	const void *register_params; /* this is accessed in assembly - don't move it */
	const void *kernel_fn; /* so is this - don't move it either */
	
	struct opencl_x86_ndrange_t *nd; /* NDRange currently being executed */
	unsigned int group_global[3]; /* lowest global ID of current group */
	unsigned int group_id[3]; /* group ID of current group */

	volatile int num_started; /* number of work-items that have been started */
	volatile int num_done; /* number of work-items that are complete */
	int num_items; /* number of work-items in the work group */
	volatile int current_item; /* currently-executing work-item */
	volatile int hit_barrier; /* has this work-group hit a barrier? */

	unsigned int *stack_params; /* these parameters have the correct local buffers for this core */
	char *aligned_stacks; 
	void *local_reserved;

	struct opencl_x86_device_fiber_t main_fiber;
	struct opencl_x86_device_fiber_t work_fibers[X86_MAX_WORK_GROUP_SIZE];
	struct opencl_x86_device_work_item_data_t *work_item_data[X86_MAX_WORK_GROUP_SIZE];


};

struct opencl_x86_device_sync_t
{
#ifndef HAVE_SYNC_BUILTINS
	pthread_mutex_t lock;
	pthread_cond_t cond;
#endif 
	volatile int count;
};

void opencl_x86_device_sync_init(struct opencl_x86_device_sync_t *sync);
void opencl_x86_device_sync_destroy(struct opencl_x86_device_sync_t *sync);
void opencl_x86_device_sync_wait(struct opencl_x86_device_sync_t *sync, int value);
void opencl_x86_device_sync_post(struct opencl_x86_device_sync_t *sync);

struct opencl_x86_device_t
{
	enum opencl_runtime_type_t type;  /* First field */

	/* Parent generic device object */
	struct opencl_device_t *parent;

	struct opencl_x86_device_sync_t cores_done;
	struct opencl_x86_device_sync_t work_ready;
	int core_done_count;

	int set_queue_affinity;
	int num_cores;
	pthread_t *threads;

	struct opencl_x86_device_exec_t *exec;
	struct opencl_x86_device_core_t queue_core;
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
void opencl_x86_device_switch_fiber(
		volatile struct opencl_x86_device_fiber_t *current,
		volatile struct opencl_x86_device_fiber_t *dest);
void opencl_x86_device_exit_fiber(void);
void opencl_x86_device_barrier(int data);

void *opencl_x86_device_core_func(struct opencl_x86_device_t *device);

void opencl_x86_work_item_entry_point(void);

void opencl_x86_device_init_work_item(int i, struct opencl_x86_device_core_t *core);
void opencl_x86_device_run_exec(
	struct opencl_x86_device_core_t *core,
	struct opencl_x86_device_exec_t *exec);

#endif
