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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "command.h"
#include "command-queue.h"
#include "context.h"
#include "debug.h"
#include "device.h"
#include "event.h"
#include "kernel.h"
#include "list.h"
#include "mem.h"
#include "mhandle.h"
#include "misc.h"
#include "object.h"
#include "opencl.h"


static void *opencl_command_queue_thread_func(void *user_data)
{
	struct opencl_command_queue_t *command_queue = user_data;
	struct opencl_command_t *command;

	/* Execute commands sequentially */
	for (;;)
	{
		/* Get command */
		command = opencl_command_queue_dequeue(command_queue);
		if (!command)
			break;

		/* Run it */
		opencl_command_run(command);
		opencl_command_free(command);
	}

	/* End */
	return NULL;
}


struct opencl_command_queue_t *opencl_command_queue_create(void)
{
	struct opencl_command_queue_t *command_queue;

	/* Initialize */
	command_queue = xcalloc(1, sizeof(struct opencl_command_queue_t));
	command_queue->command_list = list_create();
	pthread_mutex_init(&command_queue->lock, NULL);
	pthread_cond_init(&command_queue->cond_process, NULL);

	/* Create thread associated with command queue */
	pthread_create(&command_queue->queue_thread, NULL,
		opencl_command_queue_thread_func, command_queue);

	/* Register OpenCL object */
	opencl_object_create(command_queue, OPENCL_OBJECT_COMMAND_QUEUE,
		(opencl_object_free_func_t) opencl_command_queue_free);

	/* Return */
	return command_queue;
}


void opencl_command_queue_free(struct opencl_command_queue_t *command_queue)
{
	struct opencl_command_t *command;

	/* Create END command and flush queue */
	command = opencl_command_create_end(command_queue, NULL, 0, NULL);
	opencl_command_queue_enqueue(command_queue, command);
	opencl_command_queue_flush(command_queue);
	pthread_join(command_queue->queue_thread, NULL);
	assert(!command_queue->command_list->count);

	pthread_mutex_destroy(&command_queue->lock);
	pthread_cond_destroy(&command_queue->cond_process);

	free(command_queue);
}


void opencl_command_queue_enqueue(struct opencl_command_queue_t *command_queue, 
	struct opencl_command_t *command)
{
	struct timespec t;

	cl_ulong cltime;

	pthread_mutex_lock(&command_queue->lock);

	if (command->done_event)
	{
		clock_gettime(CLOCK_MONOTONIC, &t);
		cltime = t.tv_sec;
		cltime *= 1000000000;
		cltime += t.tv_nsec;
		command->done_event->time_queued = cltime;
	}

	list_add(command_queue->command_list, command);
	pthread_mutex_unlock(&command_queue->lock);
}


void opencl_command_queue_flush(struct opencl_command_queue_t *command_queue)
{
	pthread_mutex_lock(&command_queue->lock);
	if (command_queue->command_list->count && !command_queue->process)
	{
		command_queue->process = 1;
		pthread_cond_signal(&command_queue->cond_process);
	}
	pthread_mutex_unlock(&command_queue->lock);

}


struct opencl_command_t *opencl_command_queue_dequeue(struct opencl_command_queue_t *command_queue)
{
	struct opencl_command_t *command;
	struct timespec t;

	cl_ulong cltime;

	/* Lock */
	pthread_mutex_lock(&command_queue->lock);
	
	/* In order to proceed, the list must be processable
	 * and there must be at least one item present */
	while (!command_queue->process || !command_queue->command_list->count)
		pthread_cond_wait(&command_queue->cond_process, &command_queue->lock);
	
	/* Dequeue an item */
	command = list_remove_at(command_queue->command_list, 0);
	if (!command_queue->command_list->count)
		command_queue->process = 0;

	if (command->done_event)
	{
		clock_gettime(CLOCK_MONOTONIC, &t);
		cltime = t.tv_sec;
		cltime *= 1000000000;
		cltime += t.tv_nsec;
		command->done_event->time_submit = cltime;
	}

	/* If we get an 'end' command, return NULL */
	if (command->type == opencl_command_end)
	{
		opencl_command_free(command);
		command = NULL;
	}

	/* Unlock */
	pthread_mutex_unlock(&command_queue->lock);
	return command;
}






/*
 * OpenCL API Functions
 */


cl_command_queue clCreateCommandQueue(
	cl_context context,
	cl_device_id device,
	cl_command_queue_properties properties,
	cl_int *errcode_ret)
{
	struct opencl_command_queue_t *command_queue;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);
	opencl_debug("\tdevice = %p", device);
	opencl_debug("\tproperties = 0x%llx", (long long) properties);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Check if context is valid */
	if (!opencl_object_verify(context, OPENCL_OBJECT_CONTEXT))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	/* Check that context has passed device */
	if (!opencl_context_has_device(context, device))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_DEVICE;
		return NULL;
	}

	/* Create command queue */
	command_queue = opencl_command_queue_create();
	command_queue->device = device;
	command_queue->properties = properties;

	/* Success */
	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;

	/* Return command queue */
	return command_queue;
}


cl_int clRetainCommandQueue(
	cl_command_queue command_queue)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);

	return opencl_object_retain(command_queue,
		OPENCL_OBJECT_COMMAND_QUEUE, CL_INVALID_COMMAND_QUEUE);
}


cl_int clReleaseCommandQueue(
	cl_command_queue command_queue)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);

	return opencl_object_release(command_queue,
		OPENCL_OBJECT_COMMAND_QUEUE, CL_INVALID_COMMAND_QUEUE);
}


cl_int clGetCommandQueueInfo(
	cl_command_queue command_queue,
	cl_command_queue_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clSetCommandQueueProperty(
	cl_command_queue command_queue,
	cl_command_queue_properties properties,
	cl_bool enable,
	cl_command_queue_properties *old_properties)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueReadBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_read,
	size_t offset,
	size_t cb,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	int status;

	struct opencl_command_t *command;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tbuffer = %p", buffer);
	opencl_debug("\tblocking_read = %u", blocking_read);
	opencl_debug("\toffset = %u", offset);
	opencl_debug("\tcopy bytes = %u", cb);
	opencl_debug("\tpointer = %p", ptr);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);

	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;
	if (!opencl_object_verify(buffer, OPENCL_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;
	if (buffer->size < offset + cb)
		return CL_INVALID_VALUE;
	
	/* Check events before they are needed */
	status = opencl_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	/* Create command */
	command = opencl_command_create_mem_read(ptr, buffer->device_ptr + offset, cb,
			command_queue, event, num_events_in_wait_list, (cl_event *) event_wait_list);
	opencl_command_queue_enqueue(command_queue, command);

	/* Flush command queue if blocking */
	if (blocking_read)
		clFinish(command_queue);

	return CL_SUCCESS;

}


cl_int clEnqueueReadBufferRect(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_read,
	const size_t *buffer_origin,
	const size_t *host_origin,
	const size_t *region,
	size_t buffer_row_pitch,
	size_t buffer_slice_pitch,
	size_t host_row_pitch,
	size_t host_slice_pitch,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueWriteBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_write,
	size_t offset,
	size_t cb,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	struct opencl_command_t *command;
	int status;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tbuffer = %p", buffer);
	opencl_debug("\tblocking_write = %u", blocking_write);
	opencl_debug("\toffset = %u", offset);
	opencl_debug("\tcopy bytes = %u", cb);
	opencl_debug("\tpointer = %p", ptr);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);

	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;
	if (!opencl_object_verify(buffer, OPENCL_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;
	if (buffer->size < offset + cb)
		return CL_INVALID_VALUE;
	
	/* Check events before they are needed */
	status = opencl_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	/* Create command */
	command = opencl_command_create_mem_write(buffer->device_ptr + offset, (void *) ptr, cb,
			command_queue, event, num_events_in_wait_list, (cl_event *) event_wait_list);
	opencl_command_queue_enqueue(command_queue, command);

	/* If it is a blocking write, wait for command queue completion */
	if (blocking_write)
		clFinish(command_queue);

	/* Success */
	return CL_SUCCESS;
}


cl_int clEnqueueWriteBufferRect(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_write,
	const size_t *buffer_origin,
	const size_t *host_origin,
	const size_t *region,
	size_t buffer_row_pitch,
	size_t buffer_slice_pitch,
	size_t host_row_pitch,
	size_t host_slice_pitch,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueCopyBuffer(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_buffer,
	size_t src_offset,
	size_t dst_offset,
	size_t cb,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	struct opencl_command_t *command;
	int status;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tsrc_buffer = %p", src_buffer);
	opencl_debug("\tdst_buffer = %p", dst_buffer);
	opencl_debug("\tsrc_offset = %u", src_offset);
	opencl_debug("\tdst_offset = %u", dst_offset);
	opencl_debug("\tcopy bytes = %u", cb);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);

	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;
	if (!opencl_object_verify(src_buffer, OPENCL_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;
	if (!opencl_object_verify(dst_buffer, OPENCL_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;
	if ((src_buffer->size < src_offset + cb) || (dst_buffer->size < dst_offset + cb))
		return CL_INVALID_VALUE;
	if (!cb)
		return CL_INVALID_VALUE;
	if ((src_buffer == dst_buffer) && ((src_offset == dst_offset) || 
		((src_offset < dst_offset) && (src_offset + cb > dst_offset)) ||
		((src_offset > dst_offset) && (dst_offset + cb > src_offset))))
		return CL_MEM_COPY_OVERLAP;  
	
	/* Check events before they are needed */
	status = opencl_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	/* Create command */
	command = opencl_command_create_mem_copy(dst_buffer->device_ptr + dst_offset,
			src_buffer->device_ptr + src_offset, cb,
			command_queue, event, num_events_in_wait_list,
			(cl_event *) event_wait_list);
	opencl_command_queue_enqueue(command_queue, command);
		
	/* Success */
	return CL_SUCCESS;
}


cl_int clEnqueueCopyBufferRect(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_buffer,
	const size_t *src_origin,
	const size_t *dst_origin,
	const size_t *region,
	size_t src_row_pitch,
	size_t src_slice_pitch,
	size_t dst_row_pitch,
	size_t dst_slice_pitch,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueReadImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_read,
	const size_t *origin3,
	const size_t *region3,
	size_t row_pitch,
	size_t slice_pitch,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueWriteImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_write,
	const size_t *origin3,
	const size_t *region3,
	size_t input_row_pitch,
	size_t input_slice_pitch,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueCopyImage(
	cl_command_queue command_queue,
	cl_mem src_image,
	cl_mem dst_image,
	const size_t *src_origin3,
	const size_t *dst_origin3,
	const size_t *region3,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueCopyImageToBuffer(
	cl_command_queue command_queue,
	cl_mem src_image,
	cl_mem dst_buffer,
	const size_t *src_origin3,
	const size_t *region3,
	size_t dst_offset,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueCopyBufferToImage(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_image,
	size_t src_offset,
	const size_t *dst_origin3,
	const size_t *region3,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


void *clEnqueueMapBuffer(
	cl_command_queue command_queue,
	cl_mem mem,
	cl_bool blocking_map,
	cl_map_flags map_flags,
	size_t offset,
	size_t cb,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event,
	cl_int *errcode_ret)
{
	struct opencl_command_t *command;
	cl_int status;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tbuffer = %p", mem);
	opencl_debug("\tblocking_map = %d", blocking_map);
	opencl_debug("\tmap_flags = %lld", map_flags);
	opencl_debug("\toffset = %u", offset);
	opencl_debug("\tcb = %u", cb);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Check command queue */
	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_COMMAND_QUEUE;
		return NULL;
	}

	/* Check buffer object */
	if (!opencl_object_verify(mem, OPENCL_OBJECT_MEM))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_MEM_OBJECT;
		return NULL;
	}

	/* Check event */
	if ((status = opencl_event_wait_list_check(num_events_in_wait_list,
			event_wait_list)) != CL_SUCCESS)
	{
		if (errcode_ret)
			*errcode_ret = status;
		return NULL;
	}

	/* It is not specified what happens when a memory object is mapped
	 * twice without being unmapped. Detect this case and abort program. */
	if (mem->mapped)
		fatal("%s: cl_mem object already mapped",
				__FUNCTION__);

	/* Check buffer boundaries */
	if (offset + cb > mem->size)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	/* Save map information */
	mem->mapped = 1;
	mem->map_flags = map_flags;
	mem->map_offset = offset;
	mem->map_size = cb;

	/* If host pointer does not exist, allocate it now. It is necessary to
	 * do the host memory allocation now, and not when the command is
	 * processed by the command queue, since the function needs to return
	 * the host pointer right away. */
	if (!mem->host_ptr)
	{
		assert(!mem->use_host_ptr);
		mem->host_ptr = xcalloc(1, mem->size);
	}

	/* Create command */
	command = opencl_command_create_map_buffer(mem,
			command_queue, event, num_events_in_wait_list,
			(cl_event *) event_wait_list);
	opencl_command_queue_enqueue(command_queue, command);

	/* If mapping is blocking, flush command queue */
	if (blocking_map)
		clFinish(command_queue);

	/* Success */
	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;
	
	/* Return mapped buffer */
	return mem->host_ptr + offset;
}


void *clEnqueueMapImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_map,
	cl_map_flags map_flags,
	const size_t *origin3,
	const size_t *region3,
	size_t *image_row_pitch,
	size_t *image_slice_pitch,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event,
	cl_int *errcode_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueUnmapMemObject(
	cl_command_queue command_queue,
	cl_mem mem,
	void *mapped_ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	struct opencl_command_t *command;
	cl_int status;

	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tmemobj = %p", mem);
	opencl_debug("\tmapped_ptr = %p", mapped_ptr);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);

	/* Check command queue */
	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	/* Check memory object */
	if (!opencl_object_verify(mem, OPENCL_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;

	/* Object not mapped before */
	if (!mem->mapped)
		return CL_INVALID_VALUE;

	/* Check valid range of host pointer */
	if (!IN_RANGE(mapped_ptr, mem->host_ptr, mem->host_ptr + mem->size - 1))
		return CL_INVALID_VALUE;

	/* Check events before they are needed */
	status = opencl_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	/* Initialize data associated with the command */
	command = opencl_command_create_unmap_buffer(mem, command_queue, event,
			num_events_in_wait_list, (cl_event *) event_wait_list);
	opencl_command_queue_enqueue(command_queue, command);

	/* Success */
	return CL_SUCCESS;
}


cl_int clEnqueueNDRangeKernel(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint work_dim,
	const size_t *global_work_offset,
	const size_t *global_work_size,
	const size_t *local_work_size,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	cl_int status;

	struct opencl_device_t *device;
	struct opencl_command_t *command;

	int i;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tkernel = %p", kernel);
	opencl_debug("\twork_dim = %u", work_dim);
	opencl_debug("\tglobal_work_offset = %p", global_work_offset);
	opencl_debug("\tglobal_work_size = %p", global_work_size);
	opencl_debug("\tlocal_work_size = %p", local_work_size);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);

	/* Check valid command queue */
	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	/* Check valid kernel */
	if (!opencl_object_verify(kernel, OPENCL_OBJECT_KERNEL))
		return CL_INVALID_KERNEL;

	/* Check valid events */
	status = opencl_event_wait_list_check(num_events_in_wait_list, 
		event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	/* Check valid work dimensions */
	if (!IN_RANGE(work_dim, 1, 3))
		return CL_INVALID_WORK_DIMENSION;

	/* Global size is NULL */
	if (!global_work_size)
		return CL_INVALID_GLOBAL_WORK_SIZE;

	/* Invalid global size components */
	for (i = 0; i < work_dim; i++)
		if (!global_work_size[i])
			return CL_INVALID_GLOBAL_WORK_SIZE;

	/* Invalid local size components */
	if (local_work_size)
	{
		for (i = 0; i < work_dim; i++)
		{
			/* Component is 0 */
			if (!local_work_size[i])
				return CL_INVALID_WORK_GROUP_SIZE;

			/* Global size not evenly divisible */
			if (global_work_size[i] % local_work_size[i])
				return CL_INVALID_WORK_GROUP_SIZE;
		}
	}

	/* Kernel to run */
	device = command_queue->device;

	/* Create command */
	command = opencl_command_create_ndrange(device, kernel, work_dim,
		(unsigned int *) global_work_offset,
		(unsigned int *) global_work_size,
		(unsigned int *) local_work_size, command_queue, event,
		num_events_in_wait_list, (cl_event *) event_wait_list);

	/* Enqueue command */
	opencl_command_queue_enqueue(command_queue, command);

	/* Success */
	return CL_SUCCESS;
}


cl_int clEnqueueTask(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueNativeKernel(
	cl_command_queue command_queue,
	void (*user_func)(void *),
	void *args,
	size_t cb_args,
	cl_uint num_mem_objects,
	const cl_mem *mem_list,
	const void **args_mem_loc,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueMarker(
	cl_command_queue command_queue,
	cl_event *event)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueWaitForEvents(
	cl_command_queue command_queue,
	cl_uint num_events,
	const cl_event *event_list)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clEnqueueBarrier(
	cl_command_queue command_queue)
{
	__OPENCL_NOT_IMPL__
	return 0;
}

