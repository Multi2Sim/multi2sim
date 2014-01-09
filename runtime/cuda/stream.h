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

#ifndef RUNTIME_CUDA_STREAM_H
#define RUNTIME_CUDA_STREAM_H


extern struct list_t *stream_list;

enum cuda_stream_command_type
{
	cuda_stream_command_invalid = 0,
	cuda_stream_command_mem_read
};

struct cuda_stream_command_t;

typedef void (*cuda_stream_command_func_t)(
		struct cuda_stream_command_t *command);

struct cuda_stream_command_t
{
	unsigned id;

	enum cuda_stream_command_type type;
	cuda_stream_command_func_t func;

	unsigned completed;
};

struct CUstream_st
{
	unsigned id;

	struct list_t *command_list;

	pthread_t thread;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

CUstream cuda_stream_create(void);
void cuda_stream_free(CUstream stream);
void cuda_stream_enqueue(CUstream stream,
		struct cuda_stream_command_t *command);
struct cuda_stream_command_t *cuda_stream_dequeue(CUstream stream);


#endif

