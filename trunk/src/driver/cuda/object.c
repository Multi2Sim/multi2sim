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

#include "device.h"
#include "function.h"
#include "memory.h"
#include "module.h"
#include "object.h"


/*
 * Global Variables
 */

struct linked_list_t *cuda_object_list;




/*
 * Functions
 */

/* Add a CUDA object to object list */
void cuda_object_add(void *object)
{
	linked_list_find(cuda_object_list, object);
	assert(cuda_object_list->error_code);
	linked_list_add(cuda_object_list, object);
}

/* Remove a CUDA object from object list */
void cuda_object_remove(void *object)
{
	linked_list_find(cuda_object_list, object);
	assert(!cuda_object_list->error_code);
	linked_list_remove(cuda_object_list);
}

/* Look for a CUDA object in the object list. The 'id' is the
 * first field for every object. */
void *cuda_object_get(enum cuda_obj_t type, unsigned int id)
{
	void *object;
	unsigned int object_id;

	if (id >> 16 != type)
		fatal("%s: requested CUDA object of incorrect type",
			__FUNCTION__);

	LINKED_LIST_FOR_EACH(cuda_object_list)
	{
		if (!(object = linked_list_get(cuda_object_list)))
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
void *cuda_object_get_type(enum cuda_obj_t type)
{
        void *object;
        unsigned int object_id;

	if (!cuda_object_list)
		return NULL;

        LINKED_LIST_FOR_EACH(cuda_object_list)
        {
                if (!(object = linked_list_get(cuda_object_list)))
                        panic("%s: empty object", __FUNCTION__);
                object_id = * (unsigned int *) object;
                if (object_id >> 16 == type)
                        return object;
        }

        return NULL;
}

/* Assignment of CUDA object identifiers
 * An identifier is a 32-bit value, whose 16 most significant bits represent the
 * object type, while the 16 least significant bits represent a unique object ID. */
unsigned int cuda_object_new_id(enum cuda_obj_t type)
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
void cuda_object_free_all()
{
	void *object;

	/* Devices */
	while ((object = cuda_object_get_type(CUDA_OBJ_DEVICE)))
		cuda_device_free((struct cuda_device_t *) object);

	/* Functions */
	while ((object = cuda_object_get_type(CUDA_OBJ_FUNCTION)))
		cuda_function_free((struct cuda_function_t *) object);

        /* Memories */
        while ((object = cuda_object_get_type(CUDA_OBJ_MEMORY)))
                cuda_memory_free((struct cuda_memory_t *) object);

	/* Modules */
	while ((object = cuda_object_get_type(CUDA_OBJ_MODULE)))
		cuda_module_free((struct cuda_module_t *) object);

	/* Any object left */
	if (cuda_object_list && linked_list_count(cuda_object_list))
		panic("cuda_object_free_all: objects remaining in the list");
}

