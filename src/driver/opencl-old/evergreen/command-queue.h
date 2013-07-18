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

#ifndef ARCH_EVERGREEN_EMU_OPENCL_COMMAND_QUEUE_H
#define ARCH_EVERGREEN_EMU_OPENCL_COMMAND_QUEUE_H

#include <lib/util/class.h>



enum evg_opencl_command_type_t
{
	evg_opencl_command_queue_task_invalid,
	evg_opencl_command_queue_task_read_buffer,
	evg_opencl_command_queue_task_write_buffer,
	evg_opencl_command_queue_task_ndrange_kernel
};

struct evg_opencl_command_t
{
	enum evg_opencl_command_type_t type;
	union
	{
		struct
		{
			struct evg_ndrange_t *ndrange;
		} ndrange_kernel;
	} u;
};

struct evg_opencl_command_queue_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;
	unsigned int context_id;
	unsigned int properties;

	struct linked_list_t *command_list;
};

struct evg_opencl_command_queue_t *evg_opencl_command_queue_create(void);
void evg_opencl_command_queue_free(struct evg_opencl_command_queue_t *command_queue);

struct evg_opencl_command_t *evg_opencl_command_create(enum
	evg_opencl_command_type_t type);
void evg_opencl_command_free(struct evg_opencl_command_t *command);

void evg_opencl_command_queue_submit(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_t *command);
void evg_opencl_command_queue_complete(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_t *command);

/* Callback function of type 'x86_ctx_wakeup_callback_func_t'.
 * Argument 'data' is type-casted to 'struct evg_opencl_command_queue_t' */
int evg_opencl_command_queue_can_wakeup(X86Context *ctx, void *data);


#endif

