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
#include "context.h"
#include "vertex-array.h"

static int vao_id = 1;

void opengl_vertex_client_array_set_element_size(struct opengl_vertex_client_array_t *vca, GLint size, GLenum type)
{
	if (!vca->normalized)
	{
		switch(type)
		{
		case GL_FLOAT:
			vca->element_size = size*sizeof(GLfloat);
			break;
		case GL_DOUBLE:
			vca->element_size = size*sizeof(GLdouble);
			break;
		case GL_BYTE:
			vca->element_size = size*sizeof(GLbyte);
			break;
		case GL_UNSIGNED_BYTE:
			vca->element_size = size*sizeof(GLubyte);
			break;
		case GL_SHORT:
			vca->element_size = size*sizeof(GLshort);
			break;
		case GL_UNSIGNED_SHORT:
			vca->element_size = size*sizeof(GLushort);
			break;
		case GL_INT:
			vca->element_size = size*sizeof(GLint);
			break;
		case GL_UNSIGNED_INT:
			vca->element_size = size*sizeof(GLuint);
			break;
		case GL_INT_2_10_10_10_REV:
			if (size!=4)
				fatal("size must be 4 when type = GL_INT_2_10_10_10_REV\n");
			vca->element_size = size*sizeof(GLuint);
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			if (size!=4)
				fatal("size must be 4 when type GL_INT_2_10_10_10_REV\n");
			vca->element_size = size*sizeof(GLuint);
			break;
		default:
			vca->element_size = size*sizeof(GLfloat);
			break;
		}
	}
}

static void opengl_vertex_client_array_init(struct opengl_vertex_client_array_t* vca, GLint size, GLenum type)
{
	vca->size = size;
	vca->type = type;
	vca->format = GL_RGBA;
	vca->stride = 0;
	vca->stride_byte = 0;
	vca->ptr = NULL;
	vca->enabled = GL_FALSE;
	vca->normalized = GL_FALSE;
	vca->integer = GL_FALSE;
	opengl_vertex_client_array_set_element_size(vca, size, type);
	/* FIXME: point to a vbo with id = 0? */
	vca->vbo = NULL;
}

static void opengl_vertex_array_obj_init(struct opengl_vertex_array_obj_t *vao)
{
	int i;
	struct opengl_vertex_client_array_t *vca;

	for (i = 0; i < GL_MAX_VERTEX_ATTRIB_BINDINGS; ++i)
	{
		vca = &vao->vtx_attrib[i];
		opengl_vertex_client_array_init(vca, 4, GL_FLOAT);
	}
}

struct opengl_vertex_array_obj_t *opengl_vertex_array_obj_create()
{
	struct opengl_vertex_array_obj_t *vao;

	/* Allocate */
	vao = xcalloc(1, sizeof(struct opengl_vertex_array_obj_t));

	/* Initialize */
	vao->id = vao_id;
	vao->ref_count = 0;
	vao->delete_pending = GL_FALSE;
	opengl_vertex_array_obj_init(vao);

	/* Update global vao id */
	vao_id += 1;

	/* Return */	
	return vao;
}

/* Free doesn't check flags */
void opengl_vertex_array_obj_free(struct opengl_vertex_array_obj_t *vao)
{
	/* Free */
	free(vao);		
}

/* Delete checks flags */
void opengl_vertex_array_obj_detele(struct opengl_vertex_array_obj_t *vao)
{
	vao->delete_pending = GL_TRUE;
	opengl_debug("\tVAO #%d delete pending\n", vao->id);

	if (vao->ref_count == 0)
	{
		opengl_vertex_array_obj_free(vao);
		opengl_debug("\tVAO #%d deleted\n", vao->id);
	}
}

void opengl_vertex_array_obj_bind(struct opengl_vertex_array_obj_t *vao, struct opengl_context_t *ctx)
{
	if (vao && ctx)
	{
		if (ctx->array_attrib->curr_vao)
		{
			/* Remove and update current  program */
			ctx->array_attrib->curr_vao->ref_count -= 1;
			/* Bind program */
			vao->ref_count += 1;
			ctx->array_attrib->curr_vao = vao;
			opengl_debug("\tVAO #%d [%p] bind to OpenGL context [%p]\n", 
				vao->id, vao, ctx);
		}
	}
}

void opengl_vertex_array_obj_unbind(struct opengl_vertex_array_obj_t *vao, struct opengl_context_t *ctx)
{
	if (vao && ctx)
	{
		vao->ref_count -= 1;
		ctx->array_attrib->curr_vao = NULL;
		opengl_debug("\tVAO #%d [%p] bind to OpenGL context [%p]\n", 
			vao->id, vao, ctx);
	}
}

struct linked_list_t *opengl_vertex_array_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	opengl_debug("\tCreated VAO repository [%p]\n", lst);

	/* Return */	
	return lst;
}

void opengl_vertex_array_obj_repo_free(struct linked_list_t *vao_repo)
{
	struct opengl_vertex_array_obj_t *vao;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(vao_repo)
	{
		vao = linked_list_get(vao_repo);
		assert(vao);
		opengl_vertex_array_obj_free(vao);
	}

	/* Free VAO repository */
	linked_list_free(vao_repo);

	opengl_debug("\tFreed VAO repository [%p]\n", vao_repo);
}

void opengl_vertex_array_obj_repo_add(struct linked_list_t *vao_repo, struct opengl_vertex_array_obj_t *vao)
{
	linked_list_add(vao_repo, vao);
	opengl_debug("\tAdded VAO #%d [%p] into VAO repository [%p]\n", 
		vao->id, vao, vao_repo);
}

struct opengl_vertex_array_obj_t *opengl_vertex_array_obj_repo_get(struct linked_list_t *vao_repo, int id)
{
	struct opengl_vertex_array_obj_t *vao;

	/* Search VAO */
	LINKED_LIST_FOR_EACH(vao_repo)
	{
		vao = linked_list_get(vao_repo);
		assert(vao);
		if (vao->id == id)
			return vao;
	}

	/* Not found */
	fatal("%s: requested VAO does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}

int opengl_vertex_array_obj_repo_remove(struct linked_list_t *vao_repo, struct opengl_vertex_array_obj_t *vao)
{
	if (vao->ref_count != 0)
	{
		opengl_debug("\tVAO #%d [%p] cannot be removed immediately as reference counter = %d\n", 
			vao->id, vao, vao->ref_count);
		return -1;
	}
	else 
	{
		/* Check that VAO exists */
		linked_list_find(vao_repo, vao);
		if (vao_repo->error_code)
			fatal("%s: VAO does not exist", __FUNCTION__);
		linked_list_remove(vao_repo);
		opengl_debug("\tVAO #%d [%p] removed from VAO table [%p]\n", 
			vao->id, vao, vao_repo);
		return 1;
	}
}

struct opengl_vertex_array_attrib_t *opengl_vertex_array_attrib_create()
{
	struct opengl_vertex_array_obj_t *default_vao;
	struct opengl_vertex_array_attrib_t *va_attrib;

	/* Allocate */
	va_attrib = xcalloc(1, sizeof(struct opengl_vertex_array_attrib_t));
	default_vao = xcalloc(1, sizeof(struct opengl_vertex_array_obj_t));

	/* Initialize */
	default_vao->id = 0;
	default_vao->ref_count = 1;

	/* Initially VAO with ID 0 is bound to current VAO */
	va_attrib->curr_vao = default_vao;
	va_attrib->default_vao = default_vao;
	va_attrib->curr_vbo = NULL;

	/* Return */	
	return va_attrib;
}

void opengl_vertex_array_attrib_free(struct opengl_vertex_array_attrib_t *va_attrib)
{
	/* Free default VAO */
	free(va_attrib->default_vao);
	free(va_attrib);
}
