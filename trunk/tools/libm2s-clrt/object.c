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
#include <string.h>

#include "m2s-clrt.h"
#include "thread-list.h"
#include "debug.h"



static struct clrt_thread_list_t *list = NULL;

static struct clrt_thread_list_t *get_list(void)
{
	if (!list)
		list = clrt_thread_list_create();
	return list;
}

int print_object(void *context, void *data)
{	
	struct clrt_object_t *object = data;
	printf("{%p, %d} ", object->data, object->type);
	return 1;
}

void print_object_list(void)
{
	clrt_thread_list_visit(get_list(), print_object, NULL);
	printf("\n\n");
}



struct clrt_object_t *clrt_object_create(void *data, enum clrt_object_type_t type,
	clrt_object_destroy_func_t destroy)
{
	struct clrt_object_t *object;

	/* Allocate */
	object = (struct clrt_object_t *) malloc(sizeof (struct clrt_object_t));
	if (!object)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	object->data = data;
	object->destroy_func = destroy;
	object->type = type;
	object->ref_count = 1;
	pthread_mutex_init(&object->ref_mutex, NULL);

	/* Insert to object list */
	clrt_thread_list_insert(get_list(), object);
	/* Return object */
	return object;
}


void clrt_object_free(struct clrt_object_t *object)
{
	pthread_mutex_destroy(&object->ref_mutex);
	memset(object, 0, sizeof (struct clrt_object_t));
	free(object);
}


/* this is a helper function for clrt_object_find
   It visits each of the elements in the OpenCL object list */
int match_object(void *context, void *data)
{
	struct clrt_object_t *object = data; /* each list item is an OpenCL object wrapper */
	struct clrt_object_context_t *ctx = context; /* the context as passed into list_visit */

	if (object->data == ctx->target && object->type == ctx->type)
		ctx->match = object;

	return !ctx->match;
}


/* Return the object that matches data and type */
struct clrt_object_t *clrt_object_find(void *data, enum clrt_object_type_t type)
{
	struct clrt_object_context_t context;
	context.target = data;
	context.type = type;
	context.match = NULL;

	clrt_thread_list_visit(get_list(), match_object, &context);
	return context.match;
}

int clrt_object_ref_update(void *data, enum clrt_object_type_t type, int change)
{
	struct clrt_object_t *object = clrt_object_find(data, type);

	/* Does an object of the right type exist? */
	if (!object)
		return CL_INVALID_VALUE;

	/* Protect code that accesses reference count with a lock */
	pthread_mutex_lock(&object->ref_mutex);
	object->ref_count += change; /* update the count */
	int count = object->ref_count; /* capture value for use outside */
	pthread_mutex_unlock(&object->ref_mutex);

	/* There are no more references to this object.  
           No one has chnaged ref_count since unlock because 
           no one has a reference to it anymore. */
	if (!count)
	{
		/* remove it from the list */
		if (!clrt_thread_list_remove(get_list(), object))
			panic("%s: could not remove object that just existed", __FUNCTION__);

		object->destroy_func(object->data);
		clrt_object_free(object);
	}
	else if (count < 0)
		panic("%s: number of references is negative", __FUNCTION__);

	return CL_SUCCESS;
}


/* Check the type of an object. The function returns true if the object passed
 * in 'data' is a valid OpenCL object, and its type matches 'type'. */
int clrt_object_verify(void *data, enum clrt_object_type_t type)
{
	struct clrt_object_t *object;

	/* Invalid data */
	if (!data)
		return 0;

	/* Return whether object was found and type matches */
	object = clrt_object_find(data, type);
	return (int)object;
}


/* Increment the number of references of an object */
int clrt_object_retain(void *data, enum clrt_object_type_t type, int err_code)
{
	if (clrt_object_ref_update(data, type, 1) != CL_SUCCESS)
		return err_code;
	return CL_SUCCESS;
}


/* Decrement the number of references of an object */
int clrt_object_release(void *data, enum clrt_object_type_t type, int err_code)
{
	if (clrt_object_ref_update(data, type, -1) != CL_SUCCESS)
		return err_code;
	return CL_SUCCESS;
}
