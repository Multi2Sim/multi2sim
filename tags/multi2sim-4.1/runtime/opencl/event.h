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

#ifndef RUNTIME_OPENCL_EVENT_H
#define RUNTIME_OPENCL_EVENT_H

#include "opencl.h"


/* Event object */
struct opencl_event_t
{
	cl_int status;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	struct opencl_command_queue_t *command_queue;
	struct opencl_context_t *context;

	/* Profiling Information */
	cl_ulong time_queued;
	cl_ulong time_submit;
	cl_ulong time_start;
	cl_ulong time_end;
};


/* Create/free */
struct opencl_event_t *opencl_event_create(struct opencl_command_queue_t *command_queue);
void opencl_event_free(struct opencl_event_t *event);

void opencl_event_set_status(struct opencl_event_t *event, cl_int status);
void opencl_event_wait(struct opencl_event_t *event);


#endif

