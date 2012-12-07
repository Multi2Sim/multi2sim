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

#include "m2s-clrt.h"
#include "debug.h"

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

struct clrt_queue_item_t *clrt_queue_item_create(
	struct _cl_command_queue *queue, 
	void *data, 
	queue_action_t action, 
	cl_event *done, 
	int num_wait, 
	cl_event *waits)
{
	struct clrt_queue_item_t *item;
	int i;

	item = (struct clrt_queue_item_t *) malloc(sizeof (struct clrt_queue_item_t));
	item->data = data;
	item->action = action;
	item->num_wait_events = num_wait;
	item->wait_events = waits;
	item->next = NULL;

	// the queue Item has a reference to all the prerequisite events
	for (i = 0; i < num_wait; i++)
		if (clRetainEvent(waits[i]) != CL_SUCCESS)
			fatal("%s: clRetainEvent failed on prerequisite event", __FUNCTION__);

	if (!done)
		item->done_event = NULL;
	else
	{
		item->done_event = clrt_event_create(queue);
		*done = item->done_event;
		// and to the completion event
		if (clRetainEvent(*done) != CL_SUCCESS)
			fatal("%s: clRetainEvent failed on done event", __FUNCTION__);
	}
	
	return item;
}


void clrt_queue_item_free(struct clrt_queue_item_t *item)
{
	int i;
	for (i = 0; i < item->num_wait_events; i++)
		if (clReleaseEvent(item->wait_events[i]) != CL_SUCCESS)
			fatal("%s: clReleaseEvent failed on prerequisite event", __FUNCTION__);

	if (item->done_event)
	{
		if (clReleaseEvent(item->done_event) != CL_SUCCESS)
			fatal("%s: clReleaseEvent failed on done event", __FUNCTION__);
	}

	free(item->data);
	free(item);
}


void clrt_command_queue_enqueue(struct _cl_command_queue *queue, 
	struct clrt_queue_item_t *item)
{
	pthread_mutex_lock(&queue->lock);
	if (!queue->head)
	{
		assert(!queue->tail);
		queue->head = queue->tail = item;
	}
	else
	{
		queue->tail->next = item;
		queue->tail = item;
	}
	pthread_mutex_unlock(&queue->lock);
}


void clrt_command_queue_flush(struct _cl_command_queue *command_queue)
{
	pthread_mutex_lock(&command_queue->lock);

	if (command_queue->head && !command_queue->process)
	{
		command_queue->process = 1;
		pthread_cond_signal(&command_queue->cond_process);
	}

	pthread_mutex_unlock(&command_queue->lock);

}

void clrt_command_queue_free(void *data)
{
	struct _cl_command_queue *queue;
	struct clrt_queue_item_t *item;

	queue = (struct _cl_command_queue *) data;
	item = clrt_queue_item_create(queue, NULL, NULL, NULL, 0, NULL);

	clrt_command_queue_enqueue(queue, item);
	clrt_command_queue_flush(queue);
	pthread_join(queue->queue_thread, NULL);
	assert(!queue->head);
	pthread_mutex_destroy(&queue->lock);
	pthread_cond_destroy(&queue->cond_process);
	free(queue);
}


struct clrt_queue_item_t *clrt_command_queue_dequeue(struct _cl_command_queue *queue)
{
	struct clrt_queue_item_t *item = NULL;

	pthread_mutex_lock(&queue->lock);
	
	/* In order to procede, the list must be processable
	 * and there must be at least one item present */
	while (!queue->process || !queue->head)
		pthread_cond_wait(&queue->cond_process, &queue->lock);
	
	/* Dequeue an Item */
	item = queue->head;
	queue->head = queue->head->next;
	if (item == queue->tail)
	{
		assert(!queue->head);
		queue->tail = NULL;
		queue->process = 0;
	}

	/* If we get the special termination item, return NULL */
	if (!item->data)
	{
		free(item);
		item = NULL;
	}

	pthread_mutex_unlock(&queue->lock);
	return item;
}


void clrt_command_queue_perform_item(struct clrt_queue_item_t *item)
{
	if (item->num_wait_events > 0)
		clWaitForEvents(item->num_wait_events, item->wait_events);
	item->action(item->data);
	if (item->done_event)
		clrt_event_set_status(item->done_event, CL_COMPLETE);
}


void *clrt_command_queue_thread_proc(void *data)
{
	struct _cl_command_queue *queue;
	struct clrt_queue_item_t *item;

	queue  = (struct _cl_command_queue *) data;

	while ((item = clrt_command_queue_dequeue(queue)))
	{
		clrt_command_queue_perform_item(item);
		clrt_queue_item_free(item);
	}
	return NULL;
}




/*
 * Public Functions
 */


cl_command_queue clCreateCommandQueue(
	cl_context context,
	cl_device_id device,
	cl_command_queue_properties properties,
	cl_int *errcode_ret)
{
	int i;
	int has_device = 0;
	struct _cl_command_queue *queue;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcontext = %p", context);
	m2s_clrt_debug("\tdevice = %p", device);
	m2s_clrt_debug("\tproperties = 0x%llx", (long long) properties);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	/* check to see that context is valid */
	if (!clrt_object_verify(context, CLRT_OBJECT_CONTEXT))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	/* check to make sure that the context has the passed-in device */
	for (i = 0; i < context->num_devices; i++)
		if (context->devices[i] == device)
		{
			has_device = 1;
			break;
		}
	if (!has_device)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_DEVICE;
		return NULL;
	}

	queue = (struct _cl_command_queue *) malloc(sizeof (struct _cl_command_queue));
	if(!queue)
		fatal("%s: out of memory", __FUNCTION__);
	queue->device = device;
	queue->head = NULL;
	queue->tail = NULL;
	queue->process = 0;
	queue->properties = properties;
	pthread_mutex_init(&queue->lock, NULL);
	pthread_cond_init(&queue->cond_process, NULL);
	pthread_create(&queue->queue_thread, NULL, clrt_command_queue_thread_proc, queue);
	clrt_object_create(queue, CLRT_OBJECT_COMMAND_QUEUE, clrt_command_queue_free);

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;

	return queue;
}


cl_int clRetainCommandQueue(
	cl_command_queue command_queue)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);

	return clrt_object_retain(command_queue, CLRT_OBJECT_COMMAND_QUEUE, CL_INVALID_COMMAND_QUEUE);
}


cl_int clReleaseCommandQueue(
	cl_command_queue command_queue)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);

	return clrt_object_release(command_queue, CLRT_OBJECT_COMMAND_QUEUE, CL_INVALID_COMMAND_QUEUE);
}


cl_int clGetCommandQueueInfo(
	cl_command_queue command_queue,
	cl_command_queue_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clSetCommandQueueProperty(
	cl_command_queue command_queue,
	cl_command_queue_properties properties,
	cl_bool enable,
	cl_command_queue_properties *old_properties)
{
	__M2S_CLRT_NOT_IMPL__
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
	struct clrt_queue_item_t *item;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcommand_queue = %p", command_queue);
	m2s_clrt_debug("\tbuffer = %p", buffer);
	m2s_clrt_debug("\tblocking_read = %u", blocking_read);
	m2s_clrt_debug("\toffset = %u", offset);
	m2s_clrt_debug("\tcopy bytes = %u", cb);
	m2s_clrt_debug("\tpointer = %p", ptr);
	m2s_clrt_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	m2s_clrt_debug("\tevent_wait_list = %p", event_wait_list);
	m2s_clrt_debug("\tevent = %p", event);

	if (!clrt_object_verify(command_queue, CLRT_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;
	if (!clrt_object_verify(buffer, CLRT_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;
	if (buffer->size < offset + cb)
		return CL_INVALID_VALUE;
	
	/* Check events before they are needed */
	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	transfer = (struct clrt_mem_transfer_t *) malloc(sizeof (struct clrt_mem_transfer_t));
	if (!transfer)
		fatal("%s: out of memory", __FUNCTION__);

	transfer->src = (char *) buffer->buffer + offset;
	transfer->dst = (void *) ptr;
	transfer->size = cb;

	
	item = clrt_queue_item_create(
		command_queue,
		transfer, 
		clrt_mem_transfer_action, 
		event, 
		num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	if (blocking_read)
	{
		clrt_command_queue_enqueue(command_queue, item);
		clFinish(command_queue);

	}
	else
		clrt_command_queue_enqueue(command_queue, item);

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
	__M2S_CLRT_NOT_IMPL__
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
	struct clrt_queue_item_t *item;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcommand_queue = %p", command_queue);
	m2s_clrt_debug("\tbuffer = %p", buffer);
	m2s_clrt_debug("\tblocking_write = %u", blocking_write);
	m2s_clrt_debug("\toffset = %u", offset);
	m2s_clrt_debug("\tcopy bytes = %u", cb);
	m2s_clrt_debug("\tpointer = %p", ptr);
	m2s_clrt_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	m2s_clrt_debug("\tevent_wait_list = %p", event_wait_list);
	m2s_clrt_debug("\tevent = %p", event);

	if (!clrt_object_verify(command_queue, CLRT_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;
	if (!clrt_object_verify(buffer, CLRT_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;
	if (buffer->size < offset + cb)
		return CL_INVALID_VALUE;
	
	/* Check events before they are needed */
	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	transfer = (struct clrt_mem_transfer_t *) malloc(sizeof (struct clrt_mem_transfer_t));
	if (!transfer)
		fatal("%s: out of memory", __FUNCTION__);

	transfer->dst = (char *) buffer->buffer + offset;
	transfer->src = (void *) ptr;
	transfer->size = cb;

	
	item = clrt_queue_item_create(
		command_queue,
		transfer, 
		clrt_mem_transfer_action, 
		event, 
		num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	if (blocking_write)
	{
		clrt_command_queue_enqueue(command_queue, item);
		clFinish(command_queue);
	}
	else
		clrt_command_queue_enqueue(command_queue, item);
		
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
	__M2S_CLRT_NOT_IMPL__
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
	struct clrt_queue_item_t *item;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcommand_queue = %p", command_queue);
	m2s_clrt_debug("\tsrc_buffer = %p", src_buffer);
	m2s_clrt_debug("\tdst_buffer = %p", dst_buffer);
	m2s_clrt_debug("\tsrc_offset = %u", src_offset);
	m2s_clrt_debug("\tdst_offset = %u", dst_offset);
	m2s_clrt_debug("\tcopy bytes = %u", cb);
	m2s_clrt_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	m2s_clrt_debug("\tevent_wait_list = %p", event_wait_list);
	m2s_clrt_debug("\tevent = %p", event);

	if (!clrt_object_verify(command_queue, CLRT_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;
	if (!clrt_object_verify(src_buffer, CLRT_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;
	if (!clrt_object_verify(dst_buffer, CLRT_OBJECT_MEM))
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

	transfer = (struct clrt_mem_transfer_t *) malloc(sizeof (struct clrt_mem_transfer_t));
	if (!transfer)
		fatal("%s: out of memory", __FUNCTION__);

	transfer->dst = (char *) dst_buffer->buffer + dst_offset;
	transfer->src = (void *) src_buffer->buffer + src_offset;
	transfer->size = cb;

	
	item = clrt_queue_item_create(
		command_queue,
		transfer, 
		clrt_mem_transfer_action, 
		event, 
		num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	clrt_command_queue_enqueue(command_queue, item);
		
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
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
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
	struct clrt_queue_item_t *item;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcommand_queue = %p", command_queue);
	m2s_clrt_debug("\tbuffer = %p", buffer);
	m2s_clrt_debug("\tblocking_map = %d", blocking_map);
	m2s_clrt_debug("\tmap_flags = %lld", map_flags);
	m2s_clrt_debug("\toffset = %u", offset);
	m2s_clrt_debug("\tcb = %u", cb);
	m2s_clrt_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	m2s_clrt_debug("\tevent_wait_list = %p", event_wait_list);
	m2s_clrt_debug("\tevent = %p", event);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	if (!clrt_object_verify(command_queue, CLRT_OBJECT_COMMAND_QUEUE))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_COMMAND_QUEUE;
		return NULL;
	}


	if (!clrt_object_verify(buffer, CLRT_OBJECT_MEM))
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

	char *data = (char *)malloc(sizeof (char));
	if (!data)
		fatal("%s: out of memory", __FUNCTION__);

	item = clrt_queue_item_create(
		command_queue,
		data, /* Just need to pass in some heap object */
		clrt_mem_map_action, 
		event, 
		num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	clrt_command_queue_enqueue(command_queue, item);
	if (blocking_map)
		clFinish(command_queue);

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;
	
	return (char *)buffer->buffer + offset;
}


void * clEnqueueMapImage(
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
	__M2S_CLRT_NOT_IMPL__
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
	struct clrt_queue_item_t *item;

	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcommand_queue = %p", command_queue);
	m2s_clrt_debug("\tmemobj = %p", memobj);
	m2s_clrt_debug("\tmapped_ptr = %p", mapped_ptr);
	m2s_clrt_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	m2s_clrt_debug("\tevent_wait_list = %p", event_wait_list);
	m2s_clrt_debug("\tevent = %p", event);

	if (!clrt_object_verify(command_queue, CLRT_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	if (!clrt_object_verify(memobj, CLRT_OBJECT_MEM))
		return CL_INVALID_MEM_OBJECT;

	if (memobj->buffer > mapped_ptr || memobj->buffer + memobj->size < mapped_ptr)
		return CL_INVALID_VALUE;

	/* Check events before they are needed */
	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	item = clrt_queue_item_create(
		command_queue,
		malloc(1), /* the queue will free this memory after clrt_mem_map_aciton completes */ 
		clrt_mem_map_action, 
		event, 
		num_events_in_wait_list, 
		(struct _cl_event **) event_wait_list);

	clrt_command_queue_enqueue(command_queue, item);
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
	int i;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcommand_queue = %p", command_queue);
	m2s_clrt_debug("\tkernel = %p", kernel);
	m2s_clrt_debug("\twork_dim = %u", work_dim);
	m2s_clrt_debug("\tglobal_work_offset = %p", global_work_offset);
	m2s_clrt_debug("\tglobal_work_size = %p", global_work_size);
	m2s_clrt_debug("\tlocal_work_size = %p", local_work_size);
	m2s_clrt_debug("\tnum_events_in_wait_list = %u", num_events_in_wait_list);
	m2s_clrt_debug("\tevent_wait_list = %p", event_wait_list);
	m2s_clrt_debug("\tevent = %p", event);

	if (!clrt_object_verify(command_queue, CLRT_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	if (!clrt_object_verify(kernel, CLRT_OBJECT_KERNEL))
		return CL_INVALID_KERNEL;

	status = clrt_event_wait_list_check(num_events_in_wait_list, event_wait_list);
	if (status != CL_SUCCESS)
		return status;

	if (work_dim > MAX_DIMS)
		return CL_INVALID_WORK_DIMENSION;

	struct clrt_kernel_run_t *run = malloc(sizeof *run);
	memset(run, 0, sizeof *run);	

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
	
	struct clrt_queue_item_t *item = clrt_queue_item_create(
		command_queue,
		run, 
		clrt_kernel_run_action, 
		event, 
		num_events_in_wait_list, 
		(cl_event *) event_wait_list);

	clrt_command_queue_enqueue(command_queue, item);

	return CL_SUCCESS;
}


cl_int clEnqueueTask(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueMarker(
	cl_command_queue command_queue,
	cl_event *event)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueWaitForEvents(
	cl_command_queue command_queue,
	cl_uint num_events,
	const cl_event *event_list)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueBarrier(
	cl_command_queue command_queue)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}

