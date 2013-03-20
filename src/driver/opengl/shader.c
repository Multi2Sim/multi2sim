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
#include "shader.h"
#include "opengl.h"

int shader_id = 1;

struct opengl_shader_t *opengl_shader_create(GLenum type)
{
	struct opengl_shader_t *shdr;

	/* Allocate */
	shdr = xcalloc(1, sizeof(struct opengl_shader_t));

	/* Initialize */
	shdr->type = type;
	shdr->id = shader_id;
	shdr->ref_count = 0;
	pthread_mutex_init(&shdr->ref_mutex, NULL);
	shdr->delete_pending = GL_FALSE;
	opengl_debug("\tShader #%d [%p] created\n", shdr->id, shdr);

	/* Update global shader id */
	shader_id += 1;

	/* Return */	
	return shdr;
}

/* Free checks no flags */
void opengl_shader_free(struct opengl_shader_t *shdr)
{
	/* Debug */
	opengl_debug("\t\tFree Shader #%d [%p]\n", shdr->id, shdr);

	/* Free mutex, buffer and the rests */
	pthread_mutex_destroy(&shdr->ref_mutex);
	if(shdr->isa_buffer)
		free(shdr->isa_buffer);
	free(shdr);
}

/* Delete checks flags */
void opengl_shader_detele(struct opengl_shader_t *shdr)
{
	shdr->delete_pending = GL_TRUE;
	if (shdr->ref_count == 0)
		opengl_shader_free(shdr);
}

/* Update reference count */
void opengl_shader_ref_update(struct opengl_shader_t *shdr, int change)
{
	int count;

	pthread_mutex_lock(&shdr->ref_mutex);
	shdr->ref_count += change;
	count = shdr->ref_count;
	pthread_mutex_unlock(&shdr->ref_mutex);

	if (count < 0)
		panic("%s: number of references is negative", __FUNCTION__);
}

/* Shader repository contains shaders created */
struct linked_list_t *opengl_shader_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create(); 

	opengl_debug("\tShader Repository [%p] created\n", lst);

	/* Return */	
	return lst;
}

/* Free shader repository */
void opengl_shader_repo_free(struct linked_list_t *shdr_repo)
{
	opengl_debug("\tFree Shader Repository [%p]\n", shdr_repo);

	struct opengl_shader_t *shdr;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(shdr_repo)
	{
		shdr = linked_list_get(shdr_repo);
		assert(shdr);
		opengl_shader_free(shdr);
	}

	/* Free shader repository */
	linked_list_free(shdr_repo);
}

/* Add a shader to repository */
void opengl_shader_repo_add(struct linked_list_t *shdr_repo, struct opengl_shader_t *shdr)
{
	linked_list_add(shdr_repo, shdr);
	opengl_debug("\tAdded Shader #%d [%p] to Shader Repository [%p]\n", shdr->id, shdr, shdr_repo);
}

/* Remove a shader from repository */
int opengl_shader_repo_remove(struct linked_list_t *shdr_repo, struct opengl_shader_t *shdr)
{
	if (shdr->ref_count != 0)
	{
		opengl_debug("\tShader %d [%p] cannot be removed immediately, as reference counter = %d\n", shdr->id, shdr, shdr->ref_count);
		return -1;
	}
	else 
	{
		/* Check if shader exists */
		linked_list_find(shdr_repo, shdr);
		if (shdr_repo->error_code)
			fatal("%s: Shader does not exist", __FUNCTION__);
		linked_list_remove(shdr_repo);
		opengl_debug("\tShader %d [%p] removed from Shader table\n", shdr->id, shdr);
		return 1;
	}
}

/* Get a shader from repository */
struct opengl_shader_t *opengl_shader_repo_get(struct linked_list_t *shdr_repo, int id)
{
	struct opengl_shader_t *shdr;

	/* Search shader */
	LINKED_LIST_FOR_EACH(shdr_repo)
	{
		shdr = linked_list_get(shdr_repo);
		assert(shdr);
		if (shdr->id == id)
			return shdr;
	}

	/* Not found */
	fatal("%s: requested Shader does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}

/* Get and reference a shader */
struct opengl_shader_t *opengl_shader_repo_reference(struct linked_list_t *shdr_repo, int id)
{
	struct opengl_shader_t *shdr;

	/* Get shader */
	shdr = opengl_shader_repo_get(shdr_repo, id);

	/* Update reference count */
	opengl_shader_ref_update(shdr, 1);

	/* Return */
	return shdr;
}
