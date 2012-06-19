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
#include <debug.h>
#include <stdlib.h>

#include <evergreen-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


/* OpenCL Objects */

struct linked_list_t *evg_opencl_object_list;


/* Add an OpenCL object to object list */
void evg_opencl_object_add(void *object)
{
	linked_list_find(evg_opencl_object_list, object);
	assert(evg_opencl_object_list->error_code);
	linked_list_add(evg_opencl_object_list, object);
}


/* Remove an OpenCL object from object list */
void evg_opencl_object_remove(void *object)
{
	linked_list_find(evg_opencl_object_list, object);
	assert(!evg_opencl_object_list->error_code);
	linked_list_remove(evg_opencl_object_list);
}


/* Look for an OpenCL object in the object list. The 'id' is the
 * first field for every object. */
void *evg_opencl_object_get(enum evg_opencl_obj_t type, uint32_t id)
{
	void *object;
	uint32_t object_id;

	if (id >> 16 != type)
		fatal("%s: requested OpenCL object of incorrect type",
			__FUNCTION__);
	LINKED_LIST_FOR_EACH(evg_opencl_object_list)
	{
		if (!(object = linked_list_get(evg_opencl_object_list)))
			panic("%s: empty object", __FUNCTION__);
		object_id = * (uint32_t *) object;
		if (object_id == id)
			return object;
	}
	fatal("%s: requested OpenCL does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}


/* Get the oldest created OpenCL object of the specified type */
void *evg_opencl_object_get_type(enum evg_opencl_obj_t type)
{
	void *object;
	uint32_t object_id;

	/* Find object */
	LINKED_LIST_FOR_EACH(evg_opencl_object_list)
	{
		if (!(object = linked_list_get(evg_opencl_object_list)))
			panic("%s: empty object", __FUNCTION__);
		object_id = * (uint32_t *) object;
		if (object_id >> 16 == type)
			return object;

	}

	/* No object found */
	return NULL;
}


/* Assignment of OpenCL object identifiers
 * An identifier is a 32-bit value, whose 16 most significant bits represent the
 * object type, while the 16 least significant bits represent a unique object ID. */
uint32_t evg_opencl_object_new_id(enum evg_opencl_obj_t type)
{
	static uint32_t opencl_current_object_id;
	uint32_t id;

	id = (type << 16) | opencl_current_object_id;
	opencl_current_object_id++;
	if (opencl_current_object_id > 0xffff)
		fatal("opencl_object_new_id: too many OpenCL objects");
	return id;
}


/* Free all OpenCL objects in the object list */

void evg_opencl_object_free_all()
{
	void *object;

	/* Platforms */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_PLATFORM)))
		evg_opencl_platform_free((struct evg_opencl_platform_t *) object);
	
	/* Devices */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_DEVICE)))
		evg_opencl_device_free((struct evg_opencl_device_t *) object);
	
	/* Contexts */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_CONTEXT)))
		evg_opencl_context_free((struct evg_opencl_context_t *) object);
	
	/* Command queues */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_COMMAND_QUEUE)))
		evg_opencl_command_queue_free((struct evg_opencl_command_queue_t *) object);
	
	/* Programs */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_PROGRAM)))
		evg_opencl_program_free((struct evg_opencl_program_t *) object);
	
	/* Kernels */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_KERNEL)))
		evg_opencl_kernel_free((struct evg_opencl_kernel_t *) object);
	
	/* Mems */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_MEM)))
		evg_opencl_mem_free((struct evg_opencl_mem_t *) object);
	
	/* Events */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_EVENT)))
		evg_opencl_event_free((struct evg_opencl_event_t *) object);

	/* Samplers */
	while ((object = evg_opencl_object_get_type(EVG_OPENCL_OBJ_SAMPLER)))
		evg_opencl_sampler_free((struct evg_opencl_sampler_t *) object);
	
	/* Any object left */
	if (linked_list_count(evg_opencl_object_list))
		panic("opencl_object_free_all: objects remaining in the list");
	
}

