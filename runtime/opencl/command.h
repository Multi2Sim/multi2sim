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

#ifndef RUNTIME_OPENCL_COMMAND_H
#define RUNTIME_OPENCL_COMMAND_H

#include "kernel.h"
#include "opencl.h"

struct opencl_command_t;
typedef void (*opencl_command_func_t)(struct opencl_command_t *command);

enum opencl_command_type_t
{
	opencl_command_invalid = 0,
	opencl_command_nop,
	opencl_command_end,
	opencl_command_mem_read,
	opencl_command_mem_write,
	opencl_command_mem_copy,
	opencl_command_map_buffer,
	opencl_command_unmap_buffer,
	opencl_command_launch_ndrange
};


struct opencl_command_t
{
	enum opencl_command_type_t type;
	opencl_command_func_t func;

	int num_wait_events;
	struct opencl_event_t *done_event;
	struct opencl_event_t **wait_events;

	struct opencl_command_queue_t *command_queue;
	struct opencl_device_t *device;
	void *ndrange;  /* Architecture-specific ND-Range */

	union
	{
		struct {
			void *host_ptr;  /* Destination */
			void *device_ptr;  /* Source */
			unsigned int size;
		} mem_read;

		struct {
			void *device_ptr;  /* Destination */
			void *host_ptr;  /* Source */
			unsigned int size;
		} mem_write;

		struct {
			void *device_dest_ptr;
			void *device_src_ptr;
			unsigned int size;
		} mem_copy;

		struct {
			struct opencl_mem_t *mem;
		} map_buffer;

		struct {
			struct opencl_mem_t *mem;
		} unmap_buffer;
	};
};


struct opencl_command_t *opencl_command_create(
		enum opencl_command_type_t type,
		opencl_command_func_t func,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_nop(
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_end(
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_mem_read(
		void *host_ptr,
		void *device_ptr,
		unsigned int size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_mem_write(
		void *device_ptr,
		void *host_ptr,
		unsigned int size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_mem_copy(
		void *device_dest_ptr,
		void *device_src_ptr,
		unsigned int size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_map_buffer(
		struct opencl_mem_t *mem,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_unmap_buffer(
		struct opencl_mem_t *mem,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

struct opencl_command_t *opencl_command_create_ndrange(
		struct opencl_device_t *device,
		struct opencl_kernel_t *kernel,
		int work_dim,
		unsigned int *global_work_offset,
		unsigned int *global_work_size,
		unsigned int *local_work_size,
		struct opencl_command_queue_t *command_queue,
		struct opencl_event_t **done_event_ptr,
		int num_wait_events,
		struct opencl_event_t **wait_events);

void opencl_command_free(struct opencl_command_t *command);

void opencl_command_run(struct opencl_command_t *command);


#endif

