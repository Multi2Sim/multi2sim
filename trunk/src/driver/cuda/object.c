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

#include <lib/util/debug.h>
#include <lib/util/linked-list.h>

#include "context.h"
#include "device.h"
#include "function.h"
#include "memory.h"
#include "module.h"
#include "object.h"
#include "stream.h"


struct linked_list_t *frm_cuda_object_list;


/* Add an CUDA object to object list */
void frm_cuda_object_add(void *object)
{
	linked_list_find(frm_cuda_object_list, object);
	assert(frm_cuda_object_list->error_code);
	linked_list_add(frm_cuda_object_list, object);
}


/* Remove an CUDA object from object list */
void frm_cuda_object_remove(void *object)
{
	linked_list_find(frm_cuda_object_list, object);
	assert(!frm_cuda_object_list->error_code);
	linked_list_remove(frm_cuda_object_list);
}


/* Look for an CUDA object in the object list. The 'id' is the
 * first field for every object. */
void *frm_cuda_object_get(enum frm_cuda_obj_t type, unsigned int id)
{
	void *object;
	unsigned int object_id;

	if (id >> 16 != type)
		fatal("%s: requested CUDA object of incorrect type",
			__FUNCTION__);
	LINKED_LIST_FOR_EACH(frm_cuda_object_list)
	{
		if (!(object = linked_list_get(frm_cuda_object_list)))
			panic("%s: empty object", __FUNCTION__);
		object_id = * (unsigned int *) object;
		if (object_id == id)
			return object;
	}
	fatal("%s: requested CUDA object does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}


/* Get the oldest created CUDA object of the specified type */
void *frm_cuda_object_get_type(enum frm_cuda_obj_t type)
{
        void *object;
        unsigned int object_id;

        /* Find object */
        LINKED_LIST_FOR_EACH(frm_cuda_object_list)
        {
                if (!(object = linked_list_get(frm_cuda_object_list)))
                        panic("%s: empty object", __FUNCTION__);
                object_id = * (unsigned int *) object;
                if (object_id >> 16 == type)
                        return object;
        }

        /* No object found */
        return NULL;
}


/* Assignment of CUDA object identifiers
 * An identifier is a 32-bit value, whose 16 most significant bits represent the
 * object type, while the 16 least significant bits represent a unique object ID. */
unsigned int frm_cuda_object_new_id(enum frm_cuda_obj_t type)
{
	static unsigned int cuda_current_object_id;
	unsigned int id;

	id = (type << 16) | cuda_current_object_id;
	cuda_current_object_id++;
	if (cuda_current_object_id > 0xffff)
		fatal("cuda_object_new_id: too many CUDA objects");
	return id;
}


/* Free all CUDA objects in the object list */
void frm_cuda_object_free_all()
{
	void *object;

	/* Devices */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_DEVICE)))
		frm_cuda_device_free((struct frm_cuda_device_t *) object);

	/* Contexts */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_CONTEXT)))
		frm_cuda_context_free((struct frm_cuda_context_t *) object);

	/* Modules */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_MODULE)))
		frm_cuda_module_free((struct frm_cuda_module_t *) object);

	/* Functions */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_FUNCTION)))
		frm_cuda_function_free((struct frm_cuda_function_t *) object);

        /* Mems */
        while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_MEMORY)))
                frm_cuda_memory_free((struct frm_cuda_memory_t *) object);

	/* Streams */
	while ((object = frm_cuda_object_get_type(FRM_CUDA_OBJ_STREAM)))
		frm_cuda_stream_free((struct frm_cuda_stream_t *) object);

	/* Any object left */
	if (linked_list_count(frm_cuda_object_list))
		panic("cuda_object_free_all: objects remaining in the list");
}

