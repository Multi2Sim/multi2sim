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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>

#include "command-queue.h"
#include "context.h"
#include "device.h"
#include "event.h"
#include "kernel.h"
#include "mem.h"
#include "platform.h"
#include "program.h"
#include "repo.h"
#include "sampler.h"


struct si_opencl_repo_t
{
	struct linked_list_t *object_list;
};


struct si_opencl_repo_t *si_opencl_repo_create(void)
{
	struct si_opencl_repo_t *repo;

	/* Initialize */
	repo = xcalloc(1, sizeof(struct si_opencl_repo_t));
	repo->object_list = linked_list_create();

	/* Return */
	return repo;
}


void si_opencl_repo_free(struct si_opencl_repo_t *repo)
{
	linked_list_free(repo->object_list);
	free(repo);
}


void si_opencl_repo_add_object(struct si_opencl_repo_t *repo,
	void *object)
{
	struct linked_list_t *object_list = repo->object_list;

	/* Check that object does not exist */
	linked_list_find(object_list, object);
	if (!object_list->error_code)
		fatal("%s: object already exists", __FUNCTION__);
	
	/* Insert */
	linked_list_add(object_list, object);
}


void si_opencl_repo_remove_object(struct si_opencl_repo_t *repo,
	void *object)
{
	struct linked_list_t *object_list = repo->object_list;

	/* Check that object exists */
	linked_list_find(object_list, object);
	if (object_list->error_code)
		fatal("%s: object does not exist", __FUNCTION__);
	
	/* Remove */
	linked_list_remove(object_list);
}


/* Look for an object in the repository. The first field of every OpenCL object
 * is its identifier. */
void *si_opencl_repo_get_object(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type, unsigned int object_id)
{
	struct linked_list_t *object_list = repo->object_list;
	void *object;
	unsigned int current_object_id;

	/* Upper 16-bits represent the type of the object */
	if (object_id >> 16 != type)
		fatal("%s: requested OpenCL object of incorrect type",
			__FUNCTION__);

	/* Search object */
	LINKED_LIST_FOR_EACH(object_list)
	{
		object = linked_list_get(object_list);
		assert(object);
		current_object_id = * (unsigned int *) object;
		if (current_object_id == object_id)
			return object;
	}

	/* Not found */
	fatal("%s: requested OpenCL does not exist (id=0x%x)",
		__FUNCTION__, object_id);
	return NULL;
}


/* Get the oldest created OpenCL object of the specified type */
void *si_opencl_repo_get_object_of_type(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type)
{
	struct linked_list_t *object_list = repo->object_list;
	void *object;
	unsigned int object_id;

	/* Find object. Upper 16-bits of identifier contain its type. */
	LINKED_LIST_FOR_EACH(object_list)
	{
		object = linked_list_get(object_list);
		assert(object);
		object_id = * (unsigned int *) object;
		if (object_id >> 16 == type)
			return object;

	}

	/* No object found */
	return NULL;
}


/* Assignment of OpenCL object identifiers
 * An identifier is a 32-bit value, whose 16 most significant bits represent the
 * object type, while the 16 least significant bits represent a unique object ID. */
unsigned int si_opencl_repo_new_object_id(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type)
{
	static unsigned int si_opencl_object_id_counter;
	unsigned int object_id;

	object_id = (type << 16) | si_opencl_object_id_counter;
	si_opencl_object_id_counter++;
	if (si_opencl_object_id_counter > 0xffff)
		fatal("%s: limit of OpenCL objects exceeded\n", __FUNCTION__);
	return object_id;
}


void si_opencl_repo_free_all_objects(struct si_opencl_repo_t *repo)
{
	void *object;

	/* Platforms */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_platform)))
		si_opencl_platform_free((struct si_opencl_platform_t *) object);
	
	/* Devices */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_device)))
		si_opencl_device_free((struct si_opencl_device_t *) object);
	
	/* Contexts */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_context)))
		si_opencl_context_free((struct si_opencl_context_t *) object);
	
	/* Command queues */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_command_queue)))
		si_opencl_command_queue_free((struct si_opencl_command_queue_t *) object);
	
	/* Programs */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_program)))
		si_opencl_program_free((struct si_opencl_program_t *) object);
	
	/* Kernels */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_kernel)))
		si_opencl_kernel_free((struct si_opencl_kernel_t *) object);
	
	/* Mems */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_mem)))
		si_opencl_mem_free((struct si_opencl_mem_t *) object);
	
	/* Events */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_event)))
		si_opencl_event_free((struct si_opencl_event_t *) object);

	/* Samplers */
	while ((object = si_opencl_repo_get_object_of_type(repo, si_opencl_object_sampler)))
		si_opencl_sampler_free((struct si_opencl_sampler_t *) object);
	
	/* Any object left */
	if (linked_list_count(repo->object_list))
		panic("%s: not all objects were freed", __FUNCTION__);
	
}

