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
#include "vertex-buffer.h"




int vbo_id = 1;

struct opengl_vertex_buffer_obj_t *opengl_vertex_buffer_obj_create()
{
	struct opengl_vertex_buffer_obj_t *vbo;

	/* Allocate */
	vbo = xcalloc(1, sizeof(struct opengl_vertex_buffer_obj_t));

	/* Initialize */
	vbo->id = vbo_id;
	vbo->ref_count = 0;
	vbo->delete_pending = GL_FALSE;
	vbo->data = NULL;
	/* FIXME */
	vbo->usage = GL_DYNAMIC_DRAW;

	/* Update global shader id */
	vbo_id += 1;

	opengl_debug("\t\tVBO #%d [%p] Created\n", vbo->id, vbo);

	/* Return */	
	return vbo;
}

/* Free checks the delete_pending flag and reference count while Delete doesn't */
void opengl_vertex_buffer_obj_free(struct opengl_vertex_buffer_obj_t *vbo)
{
	/* Free */
	free(vbo->data);
	free(vbo);
	opengl_debug("\t\tVBO #%d [%p] Freed\n", vbo->id, vbo);
}

void opengl_vertex_buffer_obj_detele(struct opengl_vertex_buffer_obj_t *vbo)
{
	vbo->delete_pending = GL_TRUE;
	if (vbo->ref_count == 0)
		opengl_vertex_buffer_obj_free(vbo);
}

void opengl_vertex_buffer_obj_data(struct opengl_vertex_buffer_obj_t *vbo, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
	/* Clean previous saved data */
	if (vbo->data != NULL)
		free(vbo->data);
	
	/* Allocate */
	vbo->data = xcalloc(1, size);

	/* Record data size info */
	vbo->data_size = size;
	
	/* Save a local copy */
	memcpy(vbo->data, data, size);

	/* Update VBO state */
	vbo->usage = usage;

	opengl_debug("\tCopied %td byte from [%p] to VBO #%d [%p]\n", size, data, vbo->id, vbo);
}


struct linked_list_t *opengl_vertex_buffer_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	opengl_debug("\tCreated VBO repository [%p]\n", lst);

	/* Return */	
	return lst;
}

void opengl_vertex_buffer_obj_repo_free(struct linked_list_t *vbo_repo)
{
	struct opengl_vertex_buffer_obj_t *vbo;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(vbo_repo)
	{
		vbo = linked_list_get(vbo_repo);
		assert(vbo);
		opengl_vertex_buffer_obj_free(vbo);
	}

	/* Free shader repository */
	linked_list_free(vbo_repo);

	opengl_debug("\tFreed VBO repository [%p]\n", vbo_repo);
}

void opengl_vertex_buffer_obj_repo_add(struct linked_list_t *vbo_repo, struct opengl_vertex_buffer_obj_t *vbo)
{
	linked_list_add(vbo_repo, vbo);
	opengl_debug("\tInserting VBO #%d [%p] into VBO repository [%p]\n", vbo->id, vbo, vbo_repo);
}

struct opengl_vertex_buffer_obj_t *opengl_vertex_buffer_obj_repo_get(struct linked_list_t *vbo_repo, int id)
{
	struct opengl_vertex_buffer_obj_t *vbo;

	/* Search VBO */
	LINKED_LIST_FOR_EACH(vbo_repo)
	{
		vbo = linked_list_get(vbo_repo);
		assert(vbo);
		if (vbo->id == id)
			return vbo;
	}

	/* Not found */
	fatal("%s: requested VBO does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}

int opengl_vertex_buffer_obj_repo_remove(struct linked_list_t *vbo_repo, struct opengl_vertex_buffer_obj_t *vbo)
{
	if (vbo->ref_count != 0)
	{
		opengl_debug("\tVBO #%d [%p] cannot be removed immediately, as reference counter = %d\n", vbo->id, vbo, vbo->ref_count);
		return -1;
	}
	else 
	{
		/* Check that VBO exists */
		linked_list_find(vbo_repo, vbo);
		if (vbo_repo->error_code)
			fatal("%s: VBO does not exist", __FUNCTION__);
		linked_list_remove(vbo_repo);
		opengl_debug("\tVBO #%d [%p] removed from VBO table [%p]\n", vbo->id, vbo, vbo_repo);
		return 1;
	}
}
