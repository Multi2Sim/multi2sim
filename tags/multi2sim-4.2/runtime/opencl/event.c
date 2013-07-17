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
#include <sys/time.h>

#include "command.h"
#include "command-queue.h"
#include "debug.h"
#include "event.h"
#include "list.h"
#include "mhandle.h"
#include "object.h"


/*
 * Public Functions 
 */

struct opencl_event_t *opencl_event_create(struct opencl_command_queue_t *command_queue)
{
	struct opencl_event_t *event;

	/* Initialize */
	event = xcalloc(1, sizeof(struct opencl_event_t));
	event->status = CL_QUEUED;
	event->command_queue = command_queue;
	pthread_mutex_init(&event->mutex, NULL);
	pthread_cond_init(&event->cond, NULL);

	/* Register OpenCL object */
	opencl_object_create(event, OPENCL_OBJECT_EVENT,
		(opencl_object_free_func_t) opencl_event_free);

	/* Return */
	return event;
}


void opencl_event_free(struct opencl_event_t *event)
{
	pthread_mutex_destroy(&event->mutex);
	pthread_cond_destroy(&event->cond);
	free(event);
}


void opencl_event_set_status(struct opencl_event_t *event, cl_int status)
{
	struct timeval t;

	/* Lock and set new status */
	pthread_mutex_lock(&event->mutex);
	event->status = status;

	/* Update times */
	if (event->command_queue && (event->command_queue->properties & CL_QUEUE_PROFILING_ENABLE))
	{
		gettimeofday(&t, NULL);
		cl_ulong cltime = t.tv_sec;
		cltime *= 1000000;
		cltime += t.tv_usec;
		cltime *= 1000;

		/* If the framework has set the end time, but hasn't set any of
		 * the earlier ones then it will fall through the cases, setting
		 * those times too */
		switch (status)
		{
		case CL_QUEUED:
			if (!event->time_end)
				event->time_end = cltime;

		case CL_SUBMITTED:
			if (!event->time_start)
				event->time_start = cltime;

		case CL_RUNNING:
			if (!event->time_submit)
				event->time_submit = cltime;

		case CL_COMPLETE:
			if (!event->time_queued)
				event->time_queued = cltime;
		}
	}

	/* If event completed, notify dependences */
	if (status == CL_COMPLETE)
		pthread_cond_broadcast(&event->cond);

	/* Unlock */
	pthread_mutex_unlock(&event->mutex);
}


void opencl_event_wait(struct opencl_event_t *event)
{
	struct opencl_command_queue_t *command_queue;

	command_queue = event->command_queue;
	if (command_queue)
	{
		pthread_mutex_lock(&command_queue->lock);
		if (command_queue->command_list->count && !command_queue->process)
		{
			command_queue->process = 1;
			pthread_cond_signal(&command_queue->cond_process);
		}
		pthread_mutex_unlock(&command_queue->lock);
	}

	/* Lock */
	pthread_mutex_lock(&event->mutex);

	/* Wait until event completes */
	while (event->status != CL_COMPLETE)
		pthread_cond_wait(&event->cond, &event->mutex);

	/* Unlock */
	pthread_mutex_unlock(&event->mutex);
}




/*
 * OpenCL API Functions
 */

cl_int clWaitForEvents(
	cl_uint num_events,
	const cl_event *event_list)
{
	cl_uint i;
	
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tnum_events = %d", num_events);
	opencl_debug("\tevent_list = %p", event_list);

	if (!num_events || !event_list)
		return CL_INVALID_VALUE;

	/* Verify that the parameter list is valid up-front */
	for (i = 0; i < num_events; i++)
	{
		if (!opencl_object_verify(event_list[i], OPENCL_OBJECT_EVENT))
			return CL_INVALID_EVENT;
	}

	for (i = 0; i < num_events; i++)
		opencl_event_wait(event_list[i]);

	return CL_SUCCESS;
}


cl_int clGetEventInfo(
	cl_event event,
	cl_event_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (!opencl_object_verify(event, OPENCL_OBJECT_EVENT))
		return CL_INVALID_EVENT;

	switch (param_name)
	{
		case CL_EVENT_COMMAND_QUEUE:
			return opencl_set_param(&event->command_queue, sizeof event->command_queue,
				param_value_size, param_value, param_value_size_ret);

		case CL_EVENT_CONTEXT:
			OPENCL_ARG_NOT_SUPPORTED(param_name);
			return CL_SUCCESS;

		case CL_EVENT_COMMAND_TYPE:
			OPENCL_ARG_NOT_SUPPORTED(param_name);
			return CL_SUCCESS;

		case CL_EVENT_COMMAND_EXECUTION_STATUS:
			return opencl_set_param(&event->status, sizeof event->status,
				param_value_size, param_value, param_value_size_ret);

		case CL_EVENT_REFERENCE_COUNT:
		{
			struct opencl_object_t *object;
			cl_uint count;
			
			object = opencl_object_find(event, OPENCL_OBJECT_EVENT);
			count = object->ref_count;
			return opencl_set_param(&count, sizeof count, param_value_size,
				param_value, param_value_size_ret);
		}		
	}

	return 0;
}


cl_event clCreateUserEvent(
	cl_context context,
	cl_int *errcode_ret)
{
	struct opencl_event_t *event;
	
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Check valid context */
	if (!opencl_object_verify(context, OPENCL_OBJECT_CONTEXT))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	/* Initialize event */
	event = opencl_event_create(NULL);

	/* Return event */
	return event;
}


cl_int clRetainEvent(
	cl_event event)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tevent = %p", event);

	return opencl_object_retain(event, OPENCL_OBJECT_EVENT, CL_INVALID_EVENT);
}


cl_int clReleaseEvent(
	cl_event event)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tevent = %p", event);
	return opencl_object_release(event, OPENCL_OBJECT_EVENT, CL_INVALID_EVENT);
}


cl_int clSetUserEventStatus(
	cl_event event,
	cl_int execution_status)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tevent = %p", event);
	opencl_debug("\texecution_status = %d", execution_status);

	/* Check event */
	if (!opencl_object_verify(event, OPENCL_OBJECT_EVENT) || event->command_queue)
		return CL_INVALID_EVENT;

	/* Check execution status */
	if (execution_status != CL_COMPLETE && execution_status >= 0)
		return CL_INVALID_VALUE;
	if (event->status == CL_COMPLETE || event->status < 0)
		return CL_INVALID_OPERATION;

	/* Update event status */
	opencl_event_set_status(event, execution_status);

	/* Success */
	return CL_SUCCESS;

}


cl_int clSetEventCallback(
	cl_event event,
	cl_int command_exec_callback_type,
	void (*pfn_notify)(cl_event , cl_int , void *),
	void *user_data)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clGetEventProfilingInfo(
	cl_event event,
	cl_profiling_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (!opencl_object_verify(event, OPENCL_OBJECT_EVENT))
		return CL_INVALID_EVENT;

	if (!event->command_queue || 
		!(event->command_queue->properties & CL_QUEUE_PROFILING_ENABLE))
	{
		return CL_PROFILING_INFO_NOT_AVAILABLE;
	}

	if (param_value_size_ret)
		*param_value_size_ret = sizeof (cl_ulong);
	
	if (param_value)
	{
		if (param_value_size < sizeof (cl_ulong))
			return CL_INVALID_VALUE;

		switch (param_name)
		{
		case CL_PROFILING_COMMAND_QUEUED:
			*(cl_ulong *)param_value = event->time_queued;
			return CL_SUCCESS;

		case CL_PROFILING_COMMAND_SUBMIT:
			*(cl_ulong *)param_value = event->time_submit;
			return CL_SUCCESS;

		case CL_PROFILING_COMMAND_START:
			*(cl_ulong *)param_value = event->time_start;
			return CL_SUCCESS;

		case CL_PROFILING_COMMAND_END:
			*(cl_ulong *)param_value = event->time_end;
			return CL_SUCCESS;

		default:
			return CL_INVALID_VALUE;
		}
	}	

	return CL_SUCCESS;
}


cl_int clFlush(
	cl_command_queue command_queue)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);

	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	pthread_mutex_lock(&command_queue->lock);

	if (command_queue->command_list->count && !command_queue->process)
	{
		command_queue->process = 1;
		pthread_cond_signal(&command_queue->cond_process);
	}

	pthread_mutex_unlock(&command_queue->lock);
	return CL_SUCCESS;
}


cl_int clFinish(
	cl_command_queue command_queue)
{
	struct opencl_command_t *command;
	struct opencl_event_t *event;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcommand_queue = %p", command_queue);

	/* Check command queue */
	if (!opencl_object_verify(command_queue, OPENCL_OBJECT_COMMAND_QUEUE))
		return CL_INVALID_COMMAND_QUEUE;

	/* Create NOP command and wait for it */
	command = opencl_command_create_nop(command_queue, &event, 0, NULL);
	opencl_command_queue_enqueue(command_queue, command);
	opencl_event_wait(event);

	/* Free event and return */
	clReleaseEvent(event);
	return CL_SUCCESS;	
}

