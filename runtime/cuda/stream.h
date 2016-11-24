/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef RUNTIME_CUDA_STREAM_H
#define RUNTIME_CUDA_STREAM_H


#include "api.h"

enum cuda_stream_command_type
{
	cuda_stream_command_invalid = 0,
	cuda_stream_command_mem_read
};

struct cuda_stream_command_t;

typedef void (*cuda_stream_command_func_t)(
		struct cuda_stream_command_t *command);

struct memory_args_t
{
	CUdeviceptr src_ptr;
	CUdeviceptr dst_ptr;
	unsigned size;
};

struct kernel_args_t
{
	CUfunction kernel;
	unsigned grid_dim_x;
	unsigned grid_dim_y;
	unsigned grid_dim_z;
	unsigned block_dim_x;
	unsigned block_dim_y;
	unsigned block_dim_z;
	unsigned shared_mem_size;
	CUstream stream;
	void **kernel_params;
	void **extra;

	struct list_t *args;
};

struct event_args_t
{
	CUevent event;
};

struct callback_t
{
	CUstreamCallback func;
	CUstream stream;
	CUresult status;
	void *userData;
};

struct cuda_stream_command_t
{
	cuda_stream_command_func_t func;

	union
	{
		/* Arguments for memory copy functions */
		struct memory_args_t m_args;

		/* Arguments for kernel launch functions */
		struct kernel_args_t k_args;

		/* Arguments for event functions */
		struct event_args_t e_args;

		/* Callback functions */
		struct callback_t cb;
	};

	/* Flags */
	unsigned completed;
	unsigned ready_to_run;
	unsigned id;
};

struct CUstream_st
{
	unsigned id;

	pthread_t user_thread;

	struct list_t *command_list;

	pthread_t thread;
	pthread_mutex_t lock;

	/* Flags */
	unsigned to_be_freed;
	unsigned configuring;
};

void cuMemcpyAsyncImpl(struct cuda_stream_command_t *command);
void cudaConfigureCallImpl(struct cuda_stream_command_t *command);
void cudaSetupArgumentImpl(struct cuda_stream_command_t *command);
void cuLaunchKernelImpl(struct cuda_stream_command_t *command);
void cuEventRecordImpl(struct cuda_stream_command_t *command);
void cuWaitEventImpl(struct cuda_stream_command_t *command);
void cuStreamCallbackImpl(struct cuda_stream_command_t *command);
struct cuda_stream_command_t *cuda_stream_command_create(CUstream stream,
		cuda_stream_command_func_t func, struct memory_args_t *mem_args,
		struct kernel_args_t *k_args, struct event_args_t *e_args,
		struct callback_t *cb);
void cuda_stream_command_free(struct cuda_stream_command_t *command);
void cuda_stream_command_run(struct cuda_stream_command_t *command);
CUstream cuda_stream_create(void);
void cuda_stream_free(CUstream stream);
void cuda_stream_enqueue(CUstream stream,
		struct cuda_stream_command_t *command);
struct cuda_stream_command_t *cuda_stream_dequeue(CUstream stream);


#endif

