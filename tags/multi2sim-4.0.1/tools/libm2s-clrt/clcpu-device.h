#ifndef __CLCPU_DEVICE_H__
#define __CLCPU_DEVICE_H__

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


struct clcpu_workgroup_data_t *get_workgroup_data(); 

#endif

