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

#ifndef RUNTIME_OPENCL_COMMAND_QUEUE_H
#define RUNTIME_OPENCL_COMMAND_QUEUE_H

#include "opencl.h"


/*
 * Command Queue
 */
struct opencl_command_t;

struct opencl_command_queue_t
{
	struct opencl_device_t *device;

	/* List of commands - elements of type opencl_command_t */
	struct list_t *command_list;
	
	cl_command_queue_properties properties;

	pthread_t queue_thread;
	pthread_mutex_t lock;
	pthread_cond_t cond_process;

	volatile int process;
};


/* Create/free */
struct opencl_command_queue_t *opencl_command_queue_create(void);
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue);

void opencl_command_queue_enqueue(struct opencl_command_queue_t *command_queue,
		struct opencl_command_t *command);
struct opencl_command_t *opencl_command_queue_dequeue(
		struct opencl_command_queue_t *command_queue);
void opencl_command_queue_flush(struct opencl_command_queue_t *command_queue);


#endif
