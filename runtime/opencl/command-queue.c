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

#include "clrt.h"
#include "command-queue.h"
#include "context.h"
#include "debug.h"
#include "kernel.h"
#include "list.h"
#include "mem.h"
#include "mhandle.h"


#define MAX_DIMS 3

struct clrt_mem_transfer_t
{
	void *dst;
	void *src;
	unsigned int size;
};


struct clrt_kernel_run_t
{
	cl_device_id device;
	void *kernel; /* device-dependent kernel */
	cl_uint work_dim; 
	size_t global_work_offset[MAX_DIMS];
	size_t global_work_size[MAX_DIMS];
	size_t local_work_size[MAX_DIMS];
};




/*
 * Private Functions
 */

void clrt_kernel_run_action(void *data)
{
	struct clrt_kernel_run_t *run = data;

	run->device->device_type->execute_ndrange(
		run->device->device,
		run->kernel,
		run->work_dim,
		run->global_work_offset,
		run->global_work_size,
		run->local_work_size);
}


void clrt_mem_transfer_action(void *data)
{
	struct clrt_mem_transfer_t *transfer;
	transfer = (struct clrt_mem_transfer_t *) data;
	memcpy(transfer->dst, transfer->src, transfer->size);
}

void clrt_mem_map_action(void *data)
{

}


struct opencl_command_queue_task_t *opencl_command_queue_task_create(
	struct opencl_command_queue_t *command_queue, 
	void *user_data, opencl_command_queue_action_func_t action_func, 
	cl_event *done, int num_wait, cl_event *waits)
{
	struct opencl_command_queue_task_t *task;
	int i;

	/* Initialize */
	task = xcalloc(1, sizeof(struct opencl_command_queue_task_t));
	task->user_data = user_data;
	task->action_func = action_func;
	task->num_wait_events = num_wait;
	task->wait_events = waits;

	/* The task has a reference to all the prerequisite events */
	for (i = 0; i < num_wait; i++)
		if (clRetainEvent(waits[i]) != CL_SUCCESS)
			fatal("%s: clRetainEvent failed on prerequisite event", __FUNCTION__);

	/* Completion event */
	if (done)
	{
		task->done_event = clrt_event_create(command_queue);
		*done = task->done_event;
		if (clRetainEvent(*done) != CL_SUCCESS)
			fatal("%s: clRetainEvent failed on done event", __FUNCTION__);
	}

	/* Return task */
	return task;
}


void opencl_command_queue_task_free(struct opencl_command_queue_task_t *task)
{
	int i;

	/* Release events */
	for (i = 0; i < task->num_wait_events; i++)
		if (clReleaseEvent(task->wait_events[i]) != CL_SUCCESS)
			fatal("%s: clReleaseEvent failed on prerequisite event", __FUNCTION__);

	/* Completion event */
	if (task->done_event)
	{
		if (clReleaseEvent(task->done_event) != CL_SUCCESS)
			fatal("%s: clReleaseEvent failed on done event", __FUNCTION__);
	}

	/* Free */
	free(task);
}


void opencl_command_queue_enqueue(struct opencl_command_queue_t *command_queue, 
	struct opencl_command_queue_task_t *task)
{
	pthread_mutex_lock(&command_queue->lock);
	list_add(command_queue->task_list, task);
	pthread_mutex_unlock(&command_queue->lock);
}


void opencl_command_queue_flush(struct opencl_command_queue_t *command_queue)
{
	pthread_mutex_lock(&command_queue->lock);
	if (command_queue->task_list->count && !command_queue->process)
	{
		command_queue->process = 1;
		pthread_cond_signal(&command_queue->cond_process);
	}
	pthread_mutex_unlock(&command_queue->lock);

}


struct opencl_command_queue_task_t *opencl_command_queue_dequeue(struct opencl_command_queue_t *command_queue)
{
	struct opencl_command_queue_task_t *task;

	pthread_mutex_lock(&command_queue->lock);
	
	/* In order to procede, the list must be processable
	 * and there must be at least one item present */
	while (!command_queue->process || !command_queue->task_list->count)
		pthread_cond_wait(&command_queue->cond_process, &command_queue->lock);
	
	/* Dequeue an Item */
	task = list_remove_at(command_queue->task_list, 0);
	if (!command_queue->task_list->count)
		command_queue->process = 0;

	/* If we get the special termination item, return NULL */
	if (!task->user_data)
	{
		opencl_command_queue_task_free(task);
		task = NULL;
	}

	pthread_mutex_unlock(&command_queue->lock);
	return task;
}


void opencl_command_queue_task_run(struct opencl_command_queue_task_t *task)
{
	if (task->num_wait_events > 0)
		clWaitForEvents(task->num_wait_events, task->wait_events);
	task->action_func(task->user_data);
	if (task->done_event)
		clrt_event_set_status(task->done_event, CL_COMPLETE);
}




/*
 * Command Queue
 */

static void *opencl_command_queue_thread_func(void *user_data)
{
	struct opencl_command_queue_t *command_queue = user_data;
	struct opencl_command_queue_task_t *task;

	/* Execute tasks sequentially */
	while ((task = opencl_command_queue_dequeue(command_queue)))
	{
		opencl_command_queue_task_run(task);
		opencl_command_queue_task_free(task);
	}

	/* End */
	return NULL;
}


struct opencl_command_queue_t *opencl_command_queue_create(void)
{
	struct opencl_command_queue_t *command_queue;

	/* Initialize */
	command_queue = xcalloc(1, sizeof(struct opencl_command_queue_t));
	command_queue->task_list = list_create();
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
	struct opencl_command_queue_task_t *task;

	task = opencl_command_queue_task_create(command_queue,
		NULL, NULL, NULL, 0, NULL);

	opencl_command_queue_enqueue(command_queue, task);
	opencl_command_queue_flush(command_queue);
	pthread_join(command_queue->queue_thread, NULL);
	assert(!command_queue->task_list->count);

	pthread_mutex_destroy(&command_queue->lock);
	pthread_cond_destroy(&command_queue->cond_process);

	free(command_queue);
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

	return opencl_object_retain(command_queue, OPENCL_OBJECT_COMMAND_QUEUE, CL_INVALID_COMMAND_QUEUE);
}


cl_int clReleaseCommandQueue(
	cl_command_queue command_queue)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);

	return opencl_object_release(command_queue, OPENCL_OBJECT_COMMAND_QUEUE, CL_INVALID_COMMAND_QUEUE);
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

	struct clrt_mem_transfer_t *transfer;
	struct opencl_command_queue_task_t *task;

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
	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	transfer = xmalloc(sizeof (struct clrt_mem_transfer_t));
	transfer->src = (char *) buffer->buffer + offset;
	transfer->dst = (void *) ptr;
	transfer->size = cb;

	task = opencl_command_queue_task_create(command_queue,
		transfer, clrt_mem_transfer_action, 
		event, num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	if (blocking_read)
	{
		opencl_command_queue_enqueue(command_queue, task);
		clFinish(command_queue);

	}
	else
	{
		opencl_command_queue_enqueue(command_queue, task);
	}

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
	int status;

	struct clrt_mem_transfer_t *transfer;
	struct opencl_command_queue_task_t *task;

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
	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	transfer = xmalloc(sizeof (struct clrt_mem_transfer_t));

	transfer->dst = (char *) buffer->buffer + offset;
	transfer->src = (void *) ptr;
	transfer->size = cb;

	task = opencl_command_queue_task_create(command_queue,
		transfer, clrt_mem_transfer_action, 
		event, num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	if (blocking_write)
	{
		opencl_command_queue_enqueue(command_queue, task);
		clFinish(command_queue);
	}
	else
	{
		opencl_command_queue_enqueue(command_queue, task);
	}
		
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
	int status;

	struct clrt_mem_transfer_t *transfer;
	struct opencl_command_queue_task_t *task;

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
	if(!cb)
		return CL_INVALID_VALUE;
	if((src_buffer == dst_buffer) && ((src_offset == dst_offset) || 
		((src_offset < dst_offset) && (src_offset + cb > dst_offset)) ||
		((src_offset > dst_offset) && (dst_offset + cb > src_offset))))
		return CL_MEM_COPY_OVERLAP;  
	
	/* Check events before they are needed */
	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	transfer = xmalloc(sizeof (struct clrt_mem_transfer_t));
	transfer->dst = (char *) dst_buffer->buffer + dst_offset;
	transfer->src = (void *) src_buffer->buffer + src_offset;
	transfer->size = cb;

	task = opencl_command_queue_task_create(command_queue,
		transfer, clrt_mem_transfer_action, 
		event, num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	opencl_command_queue_enqueue(command_queue, task);
		
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


void * clEnqueueMapBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_map,
	cl_map_flags map_flags,
	size_t offset,
	size_t cb,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event,
	cl_int *errcode_ret)
{
	cl_int status;
	struct opencl_command_queue_task_t *task;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tbuffer = %p", buffer);
	opencl_debug("\tblocking_map = %d", blocking_map);
	opencl_debug("\tmap_flags = %lld", map_flags);
	opencl_debug("\toffset = %u", offset);
	opencl_debug("\tcb = %u", cb);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_COMMAND_QUEUE;
		return NULL;
	}

	if (!opencl_object_verify(buffer, OPENCL_OBJECT_MEM))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_MEM_OBJECT;
		return NULL;
	}

	if ((status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list)) != CL_SUCCESS)
	{
		if (errcode_ret)
			*errcode_ret = status;
		return NULL;
	}

	if (offset + cb > buffer->size)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	char *data = xmalloc(sizeof (char));
	task = opencl_command_queue_task_create(
		command_queue, data, clrt_mem_map_action, event, 
		num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	opencl_command_queue_enqueue(command_queue, task);
	if (blocking_map)
		clFinish(command_queue);

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;
	
	return (char *) buffer->buffer + offset;
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
	cl_mem memobj,
	void *mapped_ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	cl_int status;
	struct opencl_command_queue_task_t *task;

	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);
	opencl_debug("\tmemobj = %p", memobj);
	opencl_debug("\tmapped_ptr = %p", mapped_ptr);
	opencl_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	opencl_debug("\tevent_wait_list = %p", event_wait_list);
	opencl_debug("\tevent = %p", event);

	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	if (!opencl_object_verify(memobj, OPENCL_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;

	if (memobj->buffer > mapped_ptr || memobj->buffer + memobj->size < mapped_ptr)
		return CL_INVALID_VALUE;

	/* Check events before they are needed */
	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	task = opencl_command_queue_task_create(
		command_queue,
		xmalloc(1), /* the queue will free this memory after clrt_mem_map_aciton completes */ 
		clrt_mem_map_action, 
		event, 
		num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	opencl_command_queue_enqueue(command_queue, task);
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
	struct opencl_command_queue_task_t *task;
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

	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	if (!opencl_object_verify(kernel, OPENCL_OBJECT_KERNEL))
		return CL_INVALID_KERNEL;

	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	if (work_dim > MAX_DIMS)
		return CL_INVALID_WORK_DIMENSION;

	struct clrt_kernel_run_t *run = xcalloc(1, sizeof *run);
	run->work_dim = work_dim;

	for (i = 0; i < kernel->num_entries; i++)
		if (kernel->entries[i].device_type == command_queue->device->device_type)
			run->kernel = kernel->entries[i].kernel;

	if (run->kernel == NULL)
		return CL_INVALID_VALUE;

	if (command_queue->device->device_type->check_kernel(run->kernel) != CL_SUCCESS)
		return CL_INVALID_VALUE;

	run->device = command_queue->device;

	if (global_work_offset)
		memcpy(run->global_work_offset, global_work_offset, sizeof (size_t) * work_dim);

	memcpy(run->global_work_size, global_work_size, sizeof (size_t) * work_dim);

	if (local_work_size)
		memcpy(run->local_work_size, local_work_size, sizeof (size_t) * work_dim);
	else
		for (i = 0; i < work_dim; i++)
			run->local_work_size[i] = 1;
	
	for (i = work_dim; i < MAX_DIMS; i++)
	{
		run->global_work_offset[i] = 0;
		run->global_work_size[i] = 1;
		run->local_work_size[i] = 1;
	}
	
	task = opencl_command_queue_task_create(
		command_queue,
		run, 
		clrt_kernel_run_action, 
		event, 
		num_events_in_wait_list, 
		(cl_event *) event_wait_list);

	opencl_command_queue_enqueue(command_queue, task);

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

