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

#include <pthread.h>

#include "../include/cuda.h"
#include "list.h"
#include "mhandle.h"
#include "stream.h"


struct cuda_stream_command_t *cuda_stream_command_create(CUstream stream,
		enum cuda_stream_command_type type, cuda_stream_command_func_t func)
{
	struct cuda_stream_command_t *command;

	/* Allocate command */
	command = (struct cuda_stream_command_t *)xcalloc(1,
			sizeof(struct cuda_stream_command_t));

	/* Initialize */
	command->id = list_count(stream->command_list);
	command->type = type;
	command->func = func;
	command->completed = 0;

	list_enqueue(stream->command_list, command);

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

static void *cuda_stream_thread_func(void *thread_data)
{
	CUstream stream = thread_data;
	struct cuda_stream_command_t *command;

	/* Execute commands sequentially */
	while (1)
	{
		/* Get command */
		command = cuda_stream_dequeue(stream);
		if (! command)
			break;

		/* Run command */
		cuda_stream_command_run(command);
		cuda_stream_command_free(command);
	}

	pthread_exit(NULL);

	return NULL;
}

/* Create a stream */
CUstream cuda_stream_create(void)
{
	CUstream stream;

	/* Allocate stream */
	stream = (CUstream)xcalloc(1, sizeof(struct CUstream_st));

	/* Initialize */
	stream->id = list_count(stream_list);
	stream->command_list = list_create();

	/* Create thread associated with stream */
	pthread_mutex_init(&stream->lock, NULL);
	pthread_cond_init(&stream->cond, NULL);
	pthread_create(&stream->thread, NULL, cuda_stream_thread_func, stream);

	list_add(stream_list, stream);

	return stream;
}

/* Free stream */
void cuda_stream_free(CUstream stream)
{
	pthread_join(stream->thread, NULL);

	pthread_mutex_destroy(&stream->lock);
	pthread_cond_destroy(&stream->cond);
	pthread_exit(NULL);

	list_remove(stream_list, stream);

	free(stream);
}

/* Enqueue a command */
void cuda_stream_enqueue(CUstream stream, struct cuda_stream_command_t *command)
{
	pthread_mutex_lock(&stream->lock);

	list_add(stream->command_list, command);

	pthread_mutex_unlock(&stream->lock);
}

/* Dequeue a command */
struct cuda_stream_command_t *cuda_stream_dequeue(CUstream stream)
{
	struct cuda_stream_command_t *command;

	pthread_mutex_lock(&stream->lock);

	while (! list_count(stream->command_list))
		pthread_cond_wait(&stream->cond, &stream->lock);
	command = list_dequeue(stream->command_list);

	pthread_mutex_unlock(&stream->lock);

	return command;
}
