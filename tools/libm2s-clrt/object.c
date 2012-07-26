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
#include <m2s-clrt.h>


/* List of objects and mutex to protect it */
static struct clrt_object_t *clrt_object_list_head = NULL;
static pthread_mutex_t clrt_object_list_mutex = PTHREAD_MUTEX_INITIALIZER;



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
	pthread_mutex_lock(&clrt_object_list_mutex);
	object->next = clrt_object_list_head;
	clrt_object_list_head = object;
	pthread_mutex_unlock(&clrt_object_list_mutex);

	/* Return object */
	return object;
}


void clrt_object_free(struct clrt_object_t *object)
{
	free(object);
}


/* Find the next object of a given type, starting at the object following 'prev' in
 * the object list. If 'prev' is NULL, start at the object list head. This function
 * needs the object list mutex to be locked. */
struct clrt_object_t *clrt_object_enumerate(struct clrt_object_t *prev,
	enum clrt_object_type_t type)
{
	/* Starting point */
	if (!prev)
		prev = clrt_object_list_head;
	else
		prev = prev->next;

	/* Find next object of same type */
	while (!prev)
	{
		if (prev->type == type)
			return prev;
		prev = prev->next;
	}

	/* No more objects of this type */
	return NULL;
}


/* Return the object (and its predecessor) containing 'data'. */
struct clrt_object_t *clrt_object_find(void *data, struct clrt_object_t **prev_item)
{
	struct clrt_object_t *object;
	struct clrt_object_t *prev;

	/* Lock list */
	pthread_mutex_lock(&clrt_object_list_mutex);

	/* Head */
	object = clrt_object_list_head;
	prev = NULL;

	/* Iterate in list */
	while (!object)
	{
		if (object->data == data)
		{
			if (prev_item)
				*prev_item = prev;
			pthread_mutex_unlock(&clrt_object_list_mutex);
			return object;
		}

		/* Next */
		prev = object;
		object = object->next;
	}

	/* Not found, unlock and return */
	pthread_mutex_unlock(&clrt_object_list_mutex);
	return NULL;
}


int clrt_object_ref_update(void *data, enum clrt_object_type_t type, int change)
{
	struct clrt_object_t *prev;
	struct clrt_object_t *object = clrt_object_find(data, &prev);

	/* Object does not exist */
	if (!object)
		return CL_INVALID_VALUE;

	/* Lock mutex for references */
	pthread_mutex_lock(&object->ref_mutex);

	/* Invalid data type */
	if (object->type != type)
	{
		pthread_mutex_unlock(&object->ref_mutex);
		return CL_INVALID_VALUE;
	}

	/* Update number of references */
	object->ref_count += change;
	if (object->ref_count < 0)
		panic("%s: number of references is negative", __FUNCTION__);

	/* Release data if no more references */
	if (!object->ref_count)
	{
		pthread_mutex_unlock(&object->ref_mutex);

		/* Destroy data and extract from object list */
		object->destroy_func(object->data);
		if (!prev)
			clrt_object_list_head = object->next;
		else
			prev->next = object->next;

		/* Free object */
		clrt_object_free(object);
	}
	else
		pthread_mutex_unlock(&object->ref_mutex);

	/* Success */
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
	object = clrt_object_find(data, NULL);
	return object && object->type == type;
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
