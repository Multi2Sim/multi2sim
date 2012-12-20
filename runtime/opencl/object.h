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


/* Container for all OpenCL objects */
typedef void (*clrt_object_destroy_func_t)(void *);

enum clrt_object_type_t
{
	CLRT_OBJECT_INVALID,
	CLRT_OBJECT_CONTEXT,
	CLRT_OBJECT_COMMAND_QUEUE,
	CLRT_OBJECT_MEM,
	CLRT_OBJECT_PROGRAM,
	CLRT_OBJECT_KERNEL,
	CLRT_OBJECT_EVENT,
	CLRT_OBJECT_SAMPLER
};

struct clrt_object_t
{
	enum clrt_object_type_t type;

	/* Callback function to destroy data */
	clrt_object_destroy_func_t destroy_func;

	/* Number of references to the data */
	int ref_count;
	pthread_mutex_t ref_mutex;

	/* Object itself */
	void *data;
};

struct clrt_object_context_t
{
	void *target;
	enum clrt_object_type_t type;
	struct clrt_object_t *match;
};

struct clrt_object_t *clrt_object_create(void *data, enum clrt_object_type_t type,
		clrt_object_destroy_func_t destroy);

void clrt_object_free(struct clrt_object_t *object);

struct clrt_object_t *clrt_object_enumerate(struct clrt_object_t *prev,
		enum clrt_object_type_t type);

struct clrt_object_t *clrt_object_find(void *object, enum clrt_object_type_t type);

int clrt_object_verify(void *object, enum clrt_object_type_t type);

int clrt_object_ref_update(void *object, enum clrt_object_type_t type, int change);
int clrt_object_retain(void *object, enum clrt_object_type_t type, int err_code);
int clrt_object_release(void *object, enum clrt_object_type_t type, int err_code);


#endif
