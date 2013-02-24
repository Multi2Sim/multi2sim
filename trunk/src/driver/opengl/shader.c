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
	shdr->delete_pending = GL_FALSE;
	opengl_debug("\tShader #%d [%p] created\n", shdr->id, shdr);

	/* Update global shader id */
	shader_id += 1;

	/* Return */	
	return shdr;
}

/* Free checks the delete_pending flag and reference count while Delete doesn't */
void opengl_shader_free(struct opengl_shader_t *shdr)
{
	/* Free */
	opengl_debug("\tShader #%d [%p] freed\n", shdr->id, shdr);
	if(shdr->isa_buffer)
		free(shdr->isa_buffer);
	free(shdr);
}

void opengl_shader_detele(struct opengl_shader_t *shdr)
{
	shdr->delete_pending = GL_TRUE;
	if (shdr->ref_count == 0)
		opengl_shader_free(shdr);
}

struct linked_list_t *opengl_shader_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create(); 

	opengl_debug("\tCreated Shader Repository [%p]\n", lst);

	/* Return */	
	return lst;
}

void opengl_shader_repo_free(struct linked_list_t *shdr_repo)
{
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

	opengl_debug("\tFreed Shader Repository [%p]\n", shdr_repo);
}

void opengl_shader_repo_add(struct linked_list_t *shdr_repo, struct opengl_shader_t *shdr)
{
	linked_list_add(shdr_repo, shdr);
	opengl_debug("\tAdded Shader #%d [%p] to Shader Repository [%p]\n", shdr->id, shdr, shdr_repo);
}

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

int opengl_shader_repo_remove(struct linked_list_t *shdr_repo, struct opengl_shader_t *shdr)
{
	if (shdr->ref_count != 0)
	{
		opengl_debug("\tShader %d [%p] cannot be removed immediately, as reference counter = %d\n", shdr->id, shdr, shdr->ref_count);
		return -1;
	}
	else 
	{
		/* Check that shader exists */
		linked_list_find(shdr_repo, shdr);
		if (shdr_repo->error_code)
			fatal("%s: Shader does not exist", __FUNCTION__);
		linked_list_remove(shdr_repo);
		opengl_debug("\tShader %d [%p] removed from Shader table\n", shdr->id, shdr);
		return 1;
	}
}
