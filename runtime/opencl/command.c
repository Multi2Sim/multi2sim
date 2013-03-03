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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "command-queue.h"
#include "debug.h"
#include "device.h"
#include "event.h"
#include "mhandle.h"


/* Memory read */
static void opencl_command_run_mem_read(struct opencl_command_t *command)
{
	struct opencl_device_t *device = command->device;

	assert(device->arch_device_mem_read_func);
	device->arch_device_mem_read_func(
			device->arch_device,
			command->u.mem_read.host_ptr,
			command->u.mem_read.device_ptr,
			command->u.mem_read.size);
}


/* Memory write */
static void opencl_command_run_mem_write(struct opencl_command_t *command)
{
	struct opencl_device_t *device = command->device;

	assert(device->arch_device_mem_write_func);
	device->arch_device_mem_write_func(
			device->arch_device,
			command->u.mem_write.device_ptr,
			command->u.mem_write.host_ptr,
			command->u.mem_write.size);
}


/* Memory read */
static void opencl_command_run_mem_copy(struct opencl_command_t *command)
{
	struct opencl_device_t *device = command->device;

	assert(device->arch_device_mem_copy_func);
	device->arch_device_mem_copy_func(
			device->arch_device,
			command->u.mem_copy.device_dest_ptr,
			command->u.mem_copy.device_src_ptr,
			command->u.mem_copy.size);
}


/* Map a buffer */
static void opencl_command_run_map_buffer(struct opencl_command_t *command)
{
	warning("%s: not implemented", __FUNCTION__);
}


/* Unmap a buffer */
static void opencl_command_run_unmap_buffer(struct opencl_command_t *command)
{
	warning("%s: not implemented", __FUNCTION__);
}


/* Run a kernel */
static void opencl_command_run_launch_kernel(struct opencl_command_t *command)
{
	command->u.launch_kernel.device->arch_kernel_run_func(
			command->u.launch_kernel.arch_kernel,
			command->u.launch_kernel.work_dim,
			command->u.launch_kernel.global_work_offset,
			command->u.launch_kernel.global_work_size,
			command->u.launch_kernel.local_work_size);
}





/*
 * Public Functions
 */

struct opencl_command_t *opencl_command_create(
		enum opencl_command_type_t type,
		opencl_command_func_t func,
		struct opencl_command_queue_t *command_queue,
		cl_event *done_event_ptr,
		int num_wait_events,
		cl_event *wait_events)
{
	struct opencl_command_t *command;
	int i;

	/* Initialize */
	command = xcalloc(1, sizeof(struct opencl_command_t));
	command->type = type;
	command->func = func;
	command->command_queue = command_queue;
	command->device = command_queue->device;
	command->num_wait_events = num_wait_events;
	command->wait_events = wait_events;

	/* The command has a reference to all the prerequisite events */
	for (i = 0; i < num_wait_events; i++)
		if (clRetainEvent(wait_events[i]) != CL_SUCCESS)
			fatal("%s: clRetainEvent failed on prerequisite event", __FUNCTION__);

	/* Completion event */
	if (done_event_ptr)
	{
		command->done_event = opencl_event_create(command_queue);
		*done_event_ptr = command->done_event;
		if (clRetainEvent(*done_event_ptr) != CL_SUCCESS)
			fatal("%s: clRetainEvent failed on done event", __FUNCTION__);
	}

	/* Return */
	return command;
}


struct opencl_command_t *opencl_command_create_nop(
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	return opencl_command_create(opencl_command_nop, NULL, command_queue,
			done_event_ptr, num_wait_events, wait_events);
}


struct opencl_command_t *opencl_command_create_end(
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	return opencl_command_create(opencl_command_end, NULL, command_queue,
			done_event_ptr, num_wait_events, wait_events);
}


struct opencl_command_t *opencl_command_create_mem_read(
		void *host_ptr,
		void *device_ptr,
		unsigned int size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	struct opencl_command_t *command;

	/* Initialize */
	command = opencl_command_create(opencl_command_mem_read,
			opencl_command_run_mem_read,
			command_queue, done_event_ptr, num_wait_events,
			wait_events);
	command->u.mem_read.host_ptr = host_ptr;
	command->u.mem_read.device_ptr = device_ptr;
	command->u.mem_read.size = size;

	/* Return */
	return command;
}


struct opencl_command_t *opencl_command_create_mem_write(
		void *device_ptr,
		void *host_ptr,
		unsigned int size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	struct opencl_command_t *command;

	/* Initialize */
	command = opencl_command_create(opencl_command_mem_write,
			opencl_command_run_mem_write,
			command_queue, done_event_ptr, num_wait_events,
			wait_events);
	command->u.mem_write.device_ptr = device_ptr;
	command->u.mem_write.host_ptr = host_ptr;
	command->u.mem_write.size = size;

	/* Return */
	return command;
}


struct opencl_command_t *opencl_command_create_mem_copy(
		void *device_dest_ptr,
		void *device_src_ptr,
		unsigned int size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	struct opencl_command_t *command;

	/* Initialize */
	command = opencl_command_create(opencl_command_mem_copy,
			opencl_command_run_mem_copy,
			command_queue, done_event_ptr, num_wait_events,
			wait_events);
	command->u.mem_copy.device_dest_ptr = device_dest_ptr;
	command->u.mem_copy.device_src_ptr = device_src_ptr;
	command->u.mem_copy.size = size;

	/* Return */
	return command;
}


struct opencl_command_t *opencl_command_create_map_buffer(
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	return opencl_command_create(opencl_command_map_buffer,
			opencl_command_run_map_buffer, command_queue,
			done_event_ptr, num_wait_events, wait_events);
}


struct opencl_command_t *opencl_command_create_unmap_buffer(
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	return opencl_command_create(opencl_command_unmap_buffer,
			opencl_command_run_unmap_buffer, command_queue,
			done_event_ptr, num_wait_events, wait_events);
}


struct opencl_command_t *opencl_command_create_launch_kernel(
		struct opencl_device_t *device,
		void *arch_kernel,  /* of type 'opencl_xxx_kernel_t' */
		cl_uint work_dim,
		size_t *global_work_offset,
		size_t *global_work_size,
		size_t *local_work_size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events)
{
	struct opencl_command_t *command;
	int i;

	/* Initialize */
	command = opencl_command_create(opencl_command_launch_kernel,
			opencl_command_run_launch_kernel, command_queue, done_event_ptr,
			num_wait_events, wait_events);
	command->u.launch_kernel.device = device;
	command->u.launch_kernel.arch_kernel = arch_kernel;
	command->u.launch_kernel.work_dim = work_dim;

	/* Work sizes */
	assert(work_dim < 3);
	assert(global_work_size);
	for (i = 0; i < work_dim; i++)
	{
		command->u.launch_kernel.global_work_offset[i] = global_work_offset ?
				global_work_offset[i] : 0;
		command->u.launch_kernel.global_work_size[i] = global_work_size[i];
		command->u.launch_kernel.local_work_size[i] = local_work_size ?
				local_work_size[i] : 1;
	}

	/* Unused dimensions */
	for (i = work_dim; i < 3; i++)
	{
		command->u.launch_kernel.global_work_offset[i] = 0;
		command->u.launch_kernel.global_work_size[i] = 1;
		command->u.launch_kernel.local_work_size[i] = 1;
	}

	/* Return */
	return command;
}


void opencl_command_free(struct opencl_command_t *command)
{
	int i;

	/* Release events */
	for (i = 0; i < command->num_wait_events; i++)
		if (clReleaseEvent(command->wait_events[i]) != CL_SUCCESS)
			fatal("%s: clReleaseEvent failed on prerequisite event", __FUNCTION__);

	/* Completion event */
	if (command->done_event)
	{
		if (clReleaseEvent(command->done_event) != CL_SUCCESS)
			fatal("%s: clReleaseEvent failed on done event", __FUNCTION__);
	}

	/* Free */
	free(command);
}


void opencl_command_run(struct opencl_command_t *command)
{
	if (command->num_wait_events > 0)
		clWaitForEvents(command->num_wait_events, command->wait_events);
	if (command->func)
		command->func(command);
	if (command->done_event)
		opencl_event_set_status(command->done_event, CL_COMPLETE);
}

