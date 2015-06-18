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

#include <assert.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../include/cuda.h"
#include "debug.h"
#include "device.h"
#include "event.h"
#include "function.h"
#include "list.h"
#include "mhandle.h"
#include "stream.h"


void cuMemcpyAsyncImpl(struct cuda_stream_command_t *command)
{
	int i;
	CUdeviceptr mem_ptr, mem_tail_ptr;
	CUdeviceptr dst = command->m_args.dst_ptr;
	CUdeviceptr src = command->m_args.src_ptr;
	unsigned size = command->m_args.size;
	int dst_is_device = 0;
	int src_is_device = 0;

	cuda_debug("CUDA stream command 'cuMemcpyAsync' running now");
	cuda_debug("\tin: '%s' stream id = %d", __func__, command->id);
	cuda_debug("\tin: '%s' src_ptr = 0x%08x, stream id = %d",
					__func__, src, command->id);
	cuda_debug("\tin: '%s' dst_ptr = 0x%08x, stream id = %d",
					__func__, dst, command->id);
	cuda_debug("\tin: '%s' size = %d, stream id = %d",
					__func__, size, command->id);

	/* Determine if dst/src is host or device pointer */
	for (i = 0; i < list_count(device_memory_object_list); ++i)
	{
		mem_ptr = (CUdeviceptr) list_get(device_memory_object_list, i);
		mem_tail_ptr = (CUdeviceptr) list_get(device_memory_object_tail_list,
						i);
		cuda_debug("\tin: '%s' mem_ptr = 0x%08x, stream id = %d",
						__func__, mem_ptr, command->id);
		cuda_debug("\tin: '%s' mem_tail_ptr = 0x%08x, stream id = %d",
						__func__, mem_tail_ptr, command->id);
		//if ((! dst_is_device) && (mem_ptr == dst))
		if ((! dst_is_device) && (mem_ptr <= dst && dst < mem_tail_ptr) )
			dst_is_device = 1;
		else if ((! src_is_device) && (mem_ptr <= src && src < mem_tail_ptr))
			src_is_device = 1;
		if (dst_is_device && src_is_device)
			break;
	}

	unsigned args[3] = {(unsigned) dst, (unsigned) src,
				(unsigned) size};
	int ret;
	if ((!src_is_device) && dst_is_device)
		ret = ioctl(active_device->fd, cuda_call_MemWrite, args);
	else if (src_is_device && (! dst_is_device))
		ret = ioctl(active_device->fd, cuda_call_MemRead, args);
	else
	{
		warning("%s: host to host and device to device async memory copy \
				not implemented.\n", __func__);

		fatal("%s: not implemented\n", __FUNCTION__);
	}

	extern char *cuda_err_native;
	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	/* Syscall */
	/*
	if ((!src_is_device) && dst_is_device)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyHtoD, dst, src,
				size);
	else if (src_is_device && (! dst_is_device))
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyDtoH, dst, src,
				size);
	else
		warning("%s: host to host and device to device async memory copy \
				not implemented.\n", __func__);
	else
		fatal("device not supported.\n");*/
	//fatal("%s: not implemented", __FUNCTION__);
	
	/* Debug */
	cuda_debug("CUDA stream command 'cuMemcpyAsync' completed now");
}

void cudaConfigureCallImpl(struct cuda_stream_command_t *command)
{

}

void cudaSetupArgumentImpl(struct cuda_stream_command_t *command)
{

}

void cuLaunchKernelImpl(struct cuda_stream_command_t *command)
{
	CUfunction f = command->k_args.kernel;
	unsigned gridDimX = command->k_args.grid_dim_x;
	unsigned gridDimY = command->k_args.grid_dim_y;
	unsigned gridDimZ = command->k_args.grid_dim_z;
	unsigned blockDimX = command->k_args.block_dim_x;
	unsigned blockDimY = command->k_args.block_dim_y;
	unsigned blockDimZ = command->k_args.block_dim_z;
	unsigned sharedMemBytes = command->k_args.shared_mem_size;
	CUstream hStream = command->k_args.stream;
	int **kernelParams = (int **)command->k_args.kernel_params;
	void **extra = command->k_args.extra;
	//unsigned sys_args[11];
	int ret;
	extern char *cuda_err_native;

	cuda_debug("CUDA stream command 'cuLaunchKernel' running now");
	cuda_debug("\tin: function = [%p]", f);
	cuda_debug("\tin: gridDimX = %u", gridDimX);
	cuda_debug("\tin: gridDimY = %u", gridDimY);
	cuda_debug("\tin: gridDimZ = %u", gridDimZ);
	cuda_debug("\tin: blockDimX = %u", blockDimX);
	cuda_debug("\tin: blockDimY = %u", blockDimY);
	cuda_debug("\tin: blockDimZ = %u", blockDimZ);
	cuda_debug("\tin: sharedMemBytes = %u", sharedMemBytes);
	cuda_debug("\tin: hStream = [%p]", hStream);
	cuda_debug("\tin: kernelParams = [%p]", kernelParams);
	cuda_debug("\tin: extra = [%p]", extra);

	/* Check input */
	assert(gridDimX != 0 && gridDimY != 0 && gridDimZ != 0);
	assert(blockDimX != 0 && blockDimY != 0 && blockDimZ != 0);

	unsigned args[11];
	args[0] = f->id;
	args[1] = (unsigned) f->name;
	args[2] = gridDimX;
	args[3] = gridDimY;
	args[4] = gridDimZ;
	args[5] = blockDimX;
	args[6] = blockDimY;
	args[7] = blockDimZ;
	args[8] = sharedMemBytes;
	args[9] = (hStream ? hStream->id : 0);
	args[10] = (unsigned)kernelParams;
	args[11] = (unsigned)extra;
	ret = ioctl(active_device->fd, cuda_call_LaunchKernel, args);


	/* Syscall arguments */
	/*sys_args[0] = f->id;
	sys_args[1] = gridDimX;
	sys_args[2] = gridDimY;
	sys_args[3] = gridDimZ;
	sys_args[4] = blockDimX;
	sys_args[5] = blockDimY;
	sys_args[6] = blockDimZ;
	sys_args[7] = sharedMemBytes;
	sys_args[8] = (hStream ? hStream->id : 0);
	sys_args[9] = (unsigned)kernelParams;
	sys_args[10] = (unsigned)extra;*/

	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplLaunchKernel, sys_args);
	//ret = 0;
	//fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("CUDA stream command 'cuLaunchKernel' completed now");
}

void cuEventRecordImpl(struct cuda_stream_command_t *command)
{
	CUevent event = command->e_args.event;

	cuda_debug("CUDA stream command 'cuEventRecord' running now");
	cuda_debug("\tin: event = [%p]", event);

	cuda_event_record(event);
	event->recorded = 1;

	cuda_debug("CUDA stream command 'cuEventRecord' completed now");
}

void cuWaitEventImpl(struct cuda_stream_command_t *command)
{
	CUevent event = command->e_args.event;

	cuda_debug("CUDA stream command 'cuWaitEvent' running now");
	cuda_debug("\tin: event = [%p]", event);

	while (event->recorded)
		;

	cuda_debug("CUDA stream command 'cuWaitEvent' completed now");
}

void cuStreamCallbackImpl(struct cuda_stream_command_t *command)
{
	CUstreamCallback func = command->cb.func;
	CUstream stream = command->cb.stream;
	CUresult status = command->cb.status;
	void *userData = command->cb.userData;

	cuda_debug("CUDA stream command 'cuStreamCallback' running now");
	cuda_debug("\tin: func = [%p]", func);
	cuda_debug("\tin: stream = [%p]", stream);
	cuda_debug("\tin: status = %d", status);
	cuda_debug("\tin: userData = [%p]", userData);

	func(stream, status, userData);

	cuda_debug("CUDA stream command 'cuStreamCallback' completed now");
}

struct cuda_stream_command_t *cuda_stream_command_create(CUstream stream,
		cuda_stream_command_func_t func, struct memory_args_t *m_args,
		struct kernel_args_t *k_args, struct event_args_t *e_args,
		struct callback_t *cb)
{
	struct cuda_stream_command_t *command;

	/* Allocate command */
	command = xcalloc(1, sizeof(struct cuda_stream_command_t));

	/* Initialize */
	command->func = func;
	if (m_args != NULL)
	{
		command->m_args.src_ptr = m_args->src_ptr;
		command->m_args.dst_ptr = m_args->dst_ptr;
		command->m_args.size = m_args->size;
	}
	if (k_args != NULL)
	{
		command->k_args.grid_dim_x = k_args->grid_dim_x;
		command->k_args.grid_dim_y = k_args->grid_dim_y;
		command->k_args.grid_dim_z = k_args->grid_dim_z;
		command->k_args.block_dim_x = k_args->block_dim_x;
		command->k_args.block_dim_y = k_args->block_dim_y;
		command->k_args.block_dim_z = k_args->block_dim_z;
		command->k_args.shared_mem_size = k_args->shared_mem_size;
		command->k_args.kernel = k_args->kernel;
		command->k_args.stream = k_args->stream;
		command->k_args.kernel_params = k_args->kernel_params;
		command->k_args.extra = k_args->extra;
	}
	if (e_args != NULL)
		command->e_args.event = e_args->event;
	if (cb != NULL)
	{
		command->cb.func = cb->func;
		command->cb.stream = cb->stream;
		command->cb.status = cb->status;
		command->cb.userData = cb->userData;
	}
	command->completed = 0;
	command->id = stream->id;

	return command;
}

void cuda_stream_command_free(struct cuda_stream_command_t *command)
{
	free(command);
}

void cuda_stream_command_run(struct cuda_stream_command_t *command)
{
	command->func(command);
	command->completed = 1;
}

void *cuda_stream_thread_func(void *thread_data)
{
	CUstream stream = thread_data;
	struct cuda_stream_command_t *command;

	/* Execute commands sequentially */
	while (1)
	{
		if (stream->to_be_freed)
			break;

		/* If no command is in the stream, continue waiting */
		command = list_get(stream->command_list, 0);
		if (! command || ! command->ready_to_run)
			continue;

		/* Run command */
		cuda_stream_command_run(command);

		/* Dequeue and free command */
		cuda_stream_dequeue(stream);
		cuda_stream_command_free(command);
	}

	pthread_exit(NULL);
}

/* Create a stream */
CUstream cuda_stream_create(void)
{
	CUstream stream;

	/* Allocate stream */
	stream = xcalloc(1, sizeof(struct CUstream_st));

	/* Initialize */
	stream->id = list_count(active_device->stream_list);
	stream->user_thread = pthread_self();
	stream->command_list = list_create();
	stream->to_be_freed = 0;
	stream->configuring = 0;

	/* Create thread associated with stream */
	pthread_mutex_init(&stream->lock, NULL);
	pthread_create(&stream->thread, NULL, cuda_stream_thread_func, stream);

	return stream;
}

/* Free stream */
void cuda_stream_free(CUstream stream)
{
	stream->to_be_freed = 1;
	pthread_join(stream->thread, NULL);

	pthread_mutex_destroy(&stream->lock);

	free(stream);
}

/* Enqueue a command */
void cuda_stream_enqueue(CUstream stream, struct cuda_stream_command_t *command)
{
	pthread_mutex_lock(&stream->lock);

	list_enqueue(stream->command_list, command);

	pthread_mutex_unlock(&stream->lock);
}

/* Dequeue a command */
struct cuda_stream_command_t *cuda_stream_dequeue(CUstream stream)
{
	struct cuda_stream_command_t *command;

	pthread_mutex_lock(&stream->lock);

	command = list_dequeue(stream->command_list);

	pthread_mutex_unlock(&stream->lock);

	return command;
}

