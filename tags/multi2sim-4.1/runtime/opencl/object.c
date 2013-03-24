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

#include "debug.h"
#include "mhandle.h"
#include "object.h"
#include "opencl.h"
#include "thread-list.h"


/* List of OpenCL objects */
static struct thread_list_t *opencl_object_list;

/* Flag set when this module has been initialized */
static int opencl_object_initialized;




/*
 * Private Functions
 */


static void opencl_object_init(void)
{
	/* Ignore if already initialized */
	if (opencl_object_initialized)
		return;

	/* Create object list */
	opencl_object_list = thread_list_create();

	/* Mark initialized */
	opencl_object_initialized = 1;
}


struct opencl_object_list_visit_match_data_t
{
	void *data;
	enum opencl_object_type_t type;
	struct opencl_object_t *object;
};


/* Thread-safe list visitor function stopping list walk when an object of a
 * given type and data has been found. */
int opencl_object_list_visit_match(void *elem, void *data)
{
	struct opencl_object_t *object = elem;
	struct opencl_object_list_visit_match_data_t *match_data = data;

	/* If object is found, stop walking list and return it. */
	if (object->data == match_data->data && object->type == match_data->type)
	{
		match_data->object = object;
		return 0;
	}

	/* Continue walking list */
	return 1;
}


#if 0
static int opencl_object_list_visit_dump(void *elem, void *data)
{
	struct opencl_object_t *object;
	FILE *f;

	/* Dump current element */
	object = elem;
	f = data;
	fprintf(f, "{%p, %d} ", object->data, object->type);
	
	/* Continue visiting elements */
	return 1;
}


static void opencl_object_list_dump(FILE *f)
{
	fprintf(f, "List of OpenCL objects:\n");
	thread_list_visit(opencl_object_list, opencl_object_list_visit_dump, f);
}
#endif





/*
 * Public Functions
 */

struct opencl_object_t *opencl_object_create(void *data, enum opencl_object_type_t type,
	opencl_object_free_func_t free_func)
{
	struct opencl_object_t *object;

	opencl_object_init();

	/* Initialize */
	object = xcalloc(1, sizeof(struct opencl_object_t));
	object->data = data;
	object->type = type;
	object->free_func = free_func;
	object->ref_count = 1;
	pthread_mutex_init(&object->ref_mutex, NULL);

	/* Insert to object list */
	thread_list_insert(opencl_object_list, object);

	/* Return object */
	return object;
}


void opencl_object_free(struct opencl_object_t *object)
{
	pthread_mutex_destroy(&object->ref_mutex);
	free(object);
}


/* Return the object that matches data and type */
struct opencl_object_t *opencl_object_find(void *data, enum opencl_object_type_t type)
{
	struct opencl_object_list_visit_match_data_t match_data;

	opencl_object_init();

	/* Walk list */
	match_data.data = data;
	match_data.type = type;
	match_data.object = NULL;
	thread_list_visit(opencl_object_list, opencl_object_list_visit_match, &match_data);

	/* Return found object */
	return match_data.object;
}

int opencl_object_ref_update(void *data, enum opencl_object_type_t type, int change)
{
	struct opencl_object_t *object;
	int count;

	opencl_object_init();
	
	/* Find object */
	object = opencl_object_find(data, type);
	if (!object)
		return CL_INVALID_VALUE;

	/* Protect code that accesses reference count with a lock */
	pthread_mutex_lock(&object->ref_mutex);
	object->ref_count += change; /* update the count */
	count = object->ref_count; /* capture value for use outside */
	pthread_mutex_unlock(&object->ref_mutex);

	/* There are no more references to this object. No one has changed
	 * ref_count since unlock because no one has a reference to it anymore. */
	if (!count)
	{
		/* Remove it from the list */
		if (!thread_list_remove(opencl_object_list, object))
			panic("%s: could not remove object that just existed", __FUNCTION__);

		assert(object->free_func);
		object->free_func(object->data);
		opencl_object_free(object);
	}
	else if (count < 0)
	{
		panic("%s: number of references is negative", __FUNCTION__);
	}

	return CL_SUCCESS;
}


/* Check the type of an object. The function returns true if the object passed
 * in 'data' is a valid OpenCL object, and its type matches 'type'. */
int opencl_object_verify(void *data, enum opencl_object_type_t type)
{
	struct opencl_object_t *object;

	/* Invalid data */
	if (!data)
		return 0;

	/* Return whether object was found and type matches */
	object = opencl_object_find(data, type);
	return !!object;
}


/* Increment the number of references of an object */
int opencl_object_retain(void *data, enum opencl_object_type_t type, int err_code)
{
	if (opencl_object_ref_update(data, type, 1) != CL_SUCCESS)
		return err_code;
	return CL_SUCCESS;
}


/* Decrement the number of references of an object */
int opencl_object_release(void *data, enum opencl_object_type_t type, int err_code)
{
	if (opencl_object_ref_update(data, type, -1) != CL_SUCCESS)
		return err_code;
	return CL_SUCCESS;
}

