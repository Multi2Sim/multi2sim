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

#ifndef RUNTIME_OPENCL_OBJECT_H
#define RUNTIME_OPENCL_OBJECT_H

#include <stdio.h>


typedef void (*opencl_object_free_func_t)(void *);

enum opencl_object_type_t
{
	OPENCL_OBJECT_INVALID,
	OPENCL_OBJECT_CONTEXT,
	OPENCL_OBJECT_COMMAND_QUEUE,
	OPENCL_OBJECT_MEM,
	OPENCL_OBJECT_PROGRAM,
	OPENCL_OBJECT_KERNEL,
	OPENCL_OBJECT_EVENT,
	OPENCL_OBJECT_SAMPLER
};

struct opencl_object_t
{
	enum opencl_object_type_t type;

	/* Number of references to the data */
	int ref_count;
	pthread_mutex_t ref_mutex;

	/* Callback function to free object */
	opencl_object_free_func_t free_func;

	/* Object itself */
	void *data;
};


struct opencl_object_t *opencl_object_create(void *data, enum opencl_object_type_t type,
		opencl_object_free_func_t free_func);
void opencl_object_free(struct opencl_object_t *object);

struct opencl_object_t *opencl_object_enumerate(struct opencl_object_t *prev,
		enum opencl_object_type_t type);

struct opencl_object_t *opencl_object_find(void *object, enum opencl_object_type_t type);

int opencl_object_verify(void *object, enum opencl_object_type_t type);

int opencl_object_ref_update(void *object, enum opencl_object_type_t type, int change);
int opencl_object_retain(void *object, enum opencl_object_type_t type, int err_code);
int opencl_object_release(void *object, enum opencl_object_type_t type, int err_code);


#endif
