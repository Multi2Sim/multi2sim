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

#include "opengl.h"
#include "buffer-obj.h"

int buf_id = 1;

struct opengl_buffer_obj_t *opengl_buffer_obj_create()
{
	struct opengl_buffer_obj_t *buf_obj;

	/* Allocate */
	buf_obj = xcalloc(1, sizeof(struct opengl_buffer_obj_t));

	/* Initialize */
	buf_obj->id = buf_id;
	buf_obj->ref_count = 0;
	pthread_mutex_init(&buf_obj->ref_mutex, NULL);
	buf_obj->delete_pending = GL_FALSE;

	buf_obj->data = NULL;
	buf_obj->data_size = 0;
	buf_obj->usage = GL_DYNAMIC_DRAW;

	buf_obj->map_access_flags = 0x0;
	buf_obj->map_pointer = 0x0;
	buf_obj->map_length = 0x0;

	/* Update global buffer id */
	buf_id += 1;

	/* Debug */
	opengl_debug("\t\tBuffer Object #%d [%p] created\n", buf_obj->id, buf_obj);

	/* Return */	
	return buf_obj;
}

/* Free checks no flags */
void opengl_buffer_obj_free(struct opengl_buffer_obj_t *buf_obj)
{
	/* Check mapping */
	if (buf_obj->map_pointer)
		opengl_debug("\t\tBuffer Object #%d [%p] already mapped to [0x%x], use glUnmapBuffer to free it\n", 
			buf_obj->id, buf_obj, buf_obj->map_pointer);

	/* Debug */
	opengl_debug("\t\tFree Buffer Object #%d [%p]\n", buf_obj->id, buf_obj);

	/* Free */
	pthread_mutex_destroy(&buf_obj->ref_mutex);
	free(buf_obj->data);
	free(buf_obj);
}

/* Delete checks flags */
void opengl_buffer_obj_detele(struct opengl_buffer_obj_t *buf_obj)
{
	buf_obj->delete_pending = GL_TRUE;
	if (buf_obj->ref_count == 0)
		opengl_buffer_obj_free(buf_obj);
}

/* Update buffer reference count */
void opengl_buffer_obj_ref_update(struct opengl_buffer_obj_t *buf_obj, int change)
{
	int count;

	pthread_mutex_lock(&buf_obj->ref_mutex);
	buf_obj->ref_count += change;
	count = buf_obj->ref_count;
	pthread_mutex_unlock(&buf_obj->ref_mutex);

	if (count < 0)
		panic("%s: number of references is negative", __FUNCTION__);
}

/* Save a copy of data to buffer object */
void opengl_buffer_obj_data(struct opengl_buffer_obj_t *buf_obj, unsigned int size, const void *data, unsigned int usage)
{
	/* Clean previous saved data */
	if (buf_obj->data != NULL)
		free(buf_obj->data);
	
	/* Allocate */
	buf_obj->data = xcalloc(1, size);

	/* Record data size info */
	buf_obj->data_size = size;
	
	/* Save a local copy */
	if (data)
		memcpy(buf_obj->data, data, size);

	/* Update Buffer Object state */
	buf_obj->usage = usage;

	/* Debug */
	opengl_debug("\tCopy %u bytes from [%p] to Buffer Object #%d [%p]\n", 
		size, data, buf_obj->id, buf_obj);
}


struct linked_list_t *opengl_buffer_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	/* Debug */
	opengl_debug("\tBuffer Object repository [%p] is created\n", lst);

	/* Return */	
	return lst;
}

void opengl_buffer_obj_repo_free(struct linked_list_t *buf_obj_repo)
{
	/* Debug */
	opengl_debug("\tFree Buffer Object repository [%p]\n", buf_obj_repo);

	struct opengl_buffer_obj_t *buf_obj;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(buf_obj_repo)
	{
		buf_obj = linked_list_get(buf_obj_repo);
		assert(buf_obj);
		opengl_buffer_obj_free(buf_obj);
	}

	/* Free buffer repository */
	linked_list_free(buf_obj_repo);
}

void opengl_buffer_obj_repo_add(struct linked_list_t *buf_obj_repo, struct opengl_buffer_obj_t *buf_obj)
{
	/* Add to repository */
	linked_list_add(buf_obj_repo, buf_obj);

	/* Debug */
	opengl_debug("\tAdd Buffer Object #%d [%p] to Buffer Object repository [%p]\n", 
		buf_obj->id, buf_obj, buf_obj_repo);
}

int opengl_buffer_obj_repo_remove(struct linked_list_t *buf_obj_repo, struct opengl_buffer_obj_t *buf_obj)
{
	if (buf_obj->ref_count != 0)
	{
		opengl_debug("\tBuffer Object #%d [%p] cannot be removed immediately, as reference counter = %d\n", 
			buf_obj->id, buf_obj, buf_obj->ref_count);
		return -1;
	}
	else 
	{
		/* Check if Buffer Object exists */
		linked_list_find(buf_obj_repo, buf_obj);
		if (buf_obj_repo->error_code)
			fatal("%s: Buffer Object does not exist", __FUNCTION__);
		linked_list_remove(buf_obj_repo);
		opengl_debug("\tRemove Buffer Object #%d [%p] from Buffer Object table [%p]\n", 
			buf_obj->id, buf_obj, buf_obj_repo);
		return 1;
	}
}

struct opengl_buffer_obj_t *opengl_buffer_obj_repo_get(struct linked_list_t *buf_obj_repo, int id)
{
	struct opengl_buffer_obj_t *buf_obj;

	/* Search Buffer Object */
	LINKED_LIST_FOR_EACH(buf_obj_repo)
	{
		buf_obj = linked_list_get(buf_obj_repo);
		assert(buf_obj);
		if (buf_obj->id == id && !buf_obj->delete_pending)
			return buf_obj;
	}

	/* Not found or being deleted */
	fatal("%s: requested Buffer Object is not available (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}

struct opengl_buffer_obj_t *opengl_buffer_obj_repo_reference(struct linked_list_t *buf_obj_repo, int id)
{
	struct opengl_buffer_obj_t *buf_obj;

	/* Get buffer object and update the reference count */
	buf_obj = opengl_buffer_obj_repo_get(buf_obj_repo, id);
	if (!buf_obj->delete_pending)
	{
		opengl_buffer_obj_ref_update(buf_obj, 1);
		return buf_obj;
	}
	else
	{
		opengl_debug("\tCannot reference Buffer Object #%d [%p]: delete pending\n", 
			buf_obj->id, buf_obj);
		return NULL;	
	}
}
