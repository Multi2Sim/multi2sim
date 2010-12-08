/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel-local.h>
#include <assert.h>
#include <debug.h>
#include <stdlib.h>
#include <lnlist.h>


/* OpenCL Objects */

struct lnlist_t *opencl_object_list;


/* Add an OpenCL object to object list */
void opencl_object_add(void *object)
{
	lnlist_find(opencl_object_list, object);
	assert(lnlist_error(opencl_object_list));
	lnlist_add(opencl_object_list, object);
}


/* Remove an OpenCL object from object list */
void opencl_object_remove(void *object)
{
	lnlist_find(opencl_object_list, object);
	assert(!lnlist_error(opencl_object_list));
	lnlist_remove(opencl_object_list);
}


/* Look for an OpenCL object in the object list. The 'id' is the
 * first field for every object. */
void *opencl_object_get(enum opencl_obj_enum type, uint32_t id)
{
	void *object;
	uint32_t object_id;

	if (id >> 16 != type)
		fatal("opencl_object_get: requested OpenCL object of incorrect type");
	for (lnlist_head(opencl_object_list); !lnlist_eol(opencl_object_list); lnlist_next(opencl_object_list)) {
		if (!(object = lnlist_get(opencl_object_list)))
			panic("opencl_object_get: empty object");
		object_id = * (uint32_t *) object;
		if (object_id == id)
			return object;
	}
	fatal("opencl_object_get: requested OpenCL does not exist (id=0x%x)", id);
	return NULL;
}


/* Get the oldest created OpenCL object of the specified type */
void *opencl_object_get_type(enum opencl_obj_enum type)
{
	void *object;
	uint32_t object_id;

	/* Find object */
	for (lnlist_head(opencl_object_list); !lnlist_eol(opencl_object_list); lnlist_next(opencl_object_list)) {
		if (!(object = lnlist_get(opencl_object_list)))
			panic("opencl_object_get_type: empty object");
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
uint32_t opencl_assign_object_id(enum opencl_obj_enum type)
{
	static uint32_t opencl_current_object_id;
	uint32_t id;

	id = (type << 16) | opencl_current_object_id;
	opencl_current_object_id++;
	if (opencl_current_object_id > 0xffff)
		fatal("opencl_assign_object_id: too many OpenCL objects");
	return id;
}





/* OpenCL Platform */

struct opencl_platform_t *opencl_platform;


struct opencl_platform_t *opencl_platform_create()
{
	struct opencl_platform_t *platform;

	platform = calloc(1, sizeof(struct opencl_platform_t));
	platform->id = opencl_assign_object_id(OPENCL_OBJ_PLATFORM);
	opencl_object_add(platform);
	return platform;
}


void opencl_platform_free(struct opencl_platform_t *platform)
{
	opencl_object_remove(platform);
	free(platform);
}




/* OpenCL Device */


/* Create a device */
struct opencl_device_t *opencl_device_create()
{
	struct opencl_device_t *device;

	device = calloc(1, sizeof(struct opencl_device_t));
	device->id = opencl_assign_object_id(OPENCL_OBJ_DEVICE);
	opencl_object_add(device);
	return device;
}


/* Free device */
void opencl_device_free(struct opencl_device_t *device)
{
	opencl_object_remove(device);
	free(device);
}




/* OpenCL Context */


/* Create a context */
struct opencl_context_t *opencl_context_create()
{
	struct opencl_context_t *context;

	context = calloc(1, sizeof(struct opencl_context_t));
	context->id = opencl_assign_object_id(OPENCL_OBJ_CONTEXT);
	opencl_object_add(context);
	return context;
}


/* Free context */
void opencl_context_free(struct opencl_context_t *context)
{
	opencl_object_remove(context);
	free(context);
}



/* OpenCL Command Queue */

/* Create a command queue */
struct opencl_command_queue_t *opencl_command_queue_create()
{
	struct opencl_command_queue_t *command_queue;

	command_queue = calloc(1, sizeof(struct opencl_command_queue_t));
	command_queue->id = opencl_assign_object_id(OPENCL_OBJ_COMMAND_QUEUE);
	opencl_object_add(command_queue);
	return command_queue;
}


/* Free command queue */
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue)
{
	opencl_object_remove(command_queue);
	free(command_queue);
}

