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
#include <unistd.h>
#include <stdio.h>

#include "buffer.h"
#include "context.h"
#include "debug.h"
#include "list.h"
#include "linked-list.h"
#include "mhandle.h"
#include "vertex-array.h"

/* Global variables */
struct linked_list_t *buffer_repo;
static unsigned int buffer_obj_id = 0; /* ID 0 is reserved */

/* Forward declaration */
static unsigned int opengl_buffer_obj_assign_id();

static struct opengl_buffer_obj_t *opengl_buffer_obj_create();
static void opengl_buffer_obj_free(struct opengl_buffer_obj_t *buffer_obj);
static void opengl_buffer_obj_delete(struct opengl_buffer_obj_t *buffer_obj);
static void opengl_buffer_obj_data(struct opengl_buffer_obj_t *buffer_obj, unsigned int size, const void *data, unsigned int usage);


static void opengl_buffer_obj_repo_add(struct linked_list_t *buffer_obj_repo, 
	struct opengl_buffer_obj_t *buffer_obj);
static int opengl_buffer_obj_repo_remove(struct linked_list_t *buffer_obj_repo, 
	struct opengl_buffer_obj_t *buffer_obj);


static void opengl_buffer_binding_target_bind_buffer(
	struct opengl_buffer_binding_target_t *binding_target, struct opengl_buffer_obj_t *buffer_obj);
static void opengl_buffer_binding_target_unbind_buffer(struct opengl_buffer_binding_target_t *binding_target);

static struct opengl_buffer_binding_target_t *opengl_indexed_buffer_binding_points_get_target(
	struct opengl_indexed_buffer_binding_points_t *idx_bbp, unsigned int target, unsigned int index);


/*
 * Private Functions
 */


static unsigned int opengl_buffer_obj_assign_id()
{
	buffer_obj_id += 1;
	return buffer_obj_id;
}

static struct opengl_buffer_obj_t *opengl_buffer_obj_create()
{
	struct opengl_buffer_obj_t *buffer_obj;

	/* Allocate */
	buffer_obj = xcalloc(1, sizeof(struct opengl_buffer_obj_t));

	/* Initialize */
	buffer_obj->id = opengl_buffer_obj_assign_id();
	buffer_obj->ref_count = 0;
	pthread_mutex_init(&buffer_obj->ref_mutex, NULL);
	buffer_obj->delete_pending = GL_FALSE;
	buffer_obj->bound_targets = list_create();
	buffer_obj->bound_vattribs = list_create();
	buffer_obj->data = NULL;
	buffer_obj->size = 0;
	buffer_obj->usage = GL_STATIC_DRAW;
	buffer_obj->access= GL_READ_WRITE;
	buffer_obj->map_access_flags = 0;
	buffer_obj->mapped = 0;
	buffer_obj->map_pointer = NULL;
	buffer_obj->map_offset = 0;
	buffer_obj->map_length = 0;

	/* Debug */
	opengl_debug("\t%s: Buffer Object #%d [%p] created\n", 
		__FUNCTION__, buffer_obj->id, buffer_obj);

	/* Return */
	return buffer_obj;
}

static void opengl_buffer_obj_free(struct opengl_buffer_obj_t *buffer_obj)
{
	/* Debug */
	opengl_debug("\t%s: Buffer Object #%d [%p] freed\n", 
		__FUNCTION__, buffer_obj->id, buffer_obj);

	/* Free mutex */
	pthread_mutex_destroy(&buffer_obj->ref_mutex);

	/* Free list */
	list_free(buffer_obj->bound_targets);
	list_free(buffer_obj->bound_vattribs);

	/* Free data */
	if (buffer_obj->data)
	{
		free(buffer_obj->data);
	}

	/* Free */
	free(buffer_obj);
}

static void opengl_buffer_obj_delete(struct opengl_buffer_obj_t *buffer_obj)
{
	struct opengl_buffer_binding_target_t *bbt;
	struct opengl_vertex_attrib_t *vattrib;
	int i;

	buffer_obj->delete_pending = 1;

	/* Delete checks conditions */
	if (!buffer_obj->ref_count && !buffer_obj->mapped)
	{
		/* Reset associated binding targets to 0 */
		LIST_FOR_EACH(buffer_obj->bound_targets, i)
		{
			bbt = list_get(buffer_obj->bound_targets, i);
			pthread_mutex_lock(&bbt->mutex);
			bbt->bound_buffer_id = 0;
			pthread_mutex_unlock(&bbt->mutex);
		}

		/* Detach from vertex attribute objects */
		LIST_FOR_EACH(buffer_obj->bound_vattribs, i)
		{
			vattrib = list_get(buffer_obj->bound_vattribs, i);
			opengl_vertex_attrib_detach_buffer_obj(vattrib, buffer_obj);
		}

		opengl_buffer_obj_free(buffer_obj);
	}
	else
	{
		/* Debug */
		opengl_debug("\t%s: Cannot delete Buffer Object #%d [%p], check if still in use\n", 
			__FUNCTION__, buffer_obj->id, buffer_obj);
	}
}

static void opengl_buffer_obj_data(struct opengl_buffer_obj_t *buffer_obj, unsigned int size, const void *data, unsigned int usage)
{
	/* Save a copy in buffer object */
	buffer_obj->data = xcalloc(1, size);
	if (data)
		memcpy(buffer_obj->data, data, size);
	buffer_obj->size = size;
	buffer_obj->usage = usage;
}

static void opengl_buffer_obj_repo_add(struct linked_list_t *buffer_obj_repo, 
	struct opengl_buffer_obj_t *buffer_obj)
{
	/* Add to repository */
	linked_list_add(buffer_obj_repo, buffer_obj);

	/* Debug */
	opengl_debug("\t%s: add Buffer Object #%d [%p] to Buffer Object repository [%p]\n", 
		__FUNCTION__,  buffer_obj->id, buffer_obj, buffer_obj_repo);
}

static int opengl_buffer_obj_repo_remove(struct linked_list_t *buffer_obj_repo, 
	struct opengl_buffer_obj_t *buffer_obj)
{
	if (buffer_obj->ref_count != 0)
	{
		opengl_debug("\t%s: Buffer Object #%d [%p] cannot be removed immediately, as reference counter = %d\n", 
			__FUNCTION__, buffer_obj->id, buffer_obj, buffer_obj->ref_count);
		return -1;
	}
	else 
	{
		/* Check if Buffer Object exists */
		linked_list_find(buffer_obj_repo, buffer_obj);
		if (buffer_obj_repo->error_code)
			fatal("\t%s: Buffer Object does not exist", __FUNCTION__);
		linked_list_remove(buffer_obj_repo);
		opengl_debug("\t%s: Remove Buffer Object #%d [%p] from Buffer Object Repository [%p]\n", 
			__FUNCTION__, buffer_obj->id, buffer_obj, buffer_obj_repo);
		return 1;
	}
}

static void opengl_buffer_binding_target_bind_buffer(
	struct opengl_buffer_binding_target_t *binding_target, struct opengl_buffer_obj_t *buffer_obj)
{
	/* Need to update current binding*/
	opengl_buffer_binding_target_unbind_buffer(binding_target);

	if (buffer_obj)
	{
		/* Record in the list and update reference count */
		pthread_mutex_lock(&binding_target->mutex);
		binding_target->bound_buffer_id = buffer_obj->id;
		pthread_mutex_unlock(&binding_target->mutex);	
		opengl_buffer_obj_ref_update(buffer_obj, 1);

		/* Debug */
		opengl_debug("\t%s: Buffer Object #%d [%p] bind to Binding Target [%p]\n", 
			__FUNCTION__, buffer_obj->id, buffer_obj, binding_target);

	}
	else
	{
		binding_target->bound_buffer_id = 0;
		/* Debug */
		opengl_debug("\t%s: Buffer Object #0 bind to Binding Target [%p]\n", 
			__FUNCTION__, binding_target);
	}
}

static void opengl_buffer_binding_target_unbind_buffer(
	struct opengl_buffer_binding_target_t *binding_target)
{
	struct opengl_buffer_obj_t *curr_bound_buffer_obj;

	/* Need to update current binding*/
	if (binding_target->bound_buffer_id != 0)
	{
		/* Get buffer object currently bound to binding target */
		curr_bound_buffer_obj = opengl_buffer_obj_repo_get(buffer_repo, 
			binding_target->bound_buffer_id);

		/* Debug */
		opengl_debug("\t%s: Buffer Object #%d [%p] unbind from Binding Target [%p]\n", 
			__FUNCTION__, curr_bound_buffer_obj->id, curr_bound_buffer_obj, binding_target);

		/* Buffer object removes record of binding target */
		list_remove(curr_bound_buffer_obj->bound_targets, binding_target);
		opengl_buffer_obj_ref_update(curr_bound_buffer_obj, -1);
		binding_target->bound_buffer_id = 0;
	}

}

static struct opengl_buffer_binding_target_t *opengl_indexed_buffer_binding_points_get_target(
	struct opengl_indexed_buffer_binding_points_t *idx_bbp, unsigned int target, unsigned int index)
{
	struct opengl_buffer_binding_target_t *bbt;

	switch(target)
	{

	case GL_ATOMIC_COUNTER_BUFFER:
	{
		bbt = idx_bbp->atomic_counter_buffer[index];
		break;
	}
	case GL_SHADER_STORAGE_BUFFER:
	{
		bbt = idx_bbp->shader_storage_buffer[index];
		break;
	}
	case GL_TRANSFORM_FEEDBACK_BUFFER:
	{
		bbt = idx_bbp->transform_feedback_buffer[index];
		break;
	}
	case GL_UNIFORM_BUFFER:
	{
		bbt = idx_bbp->uniform_buffer[index];
		break;
	}
	default:
		bbt = NULL;
		break;
	}

	/* Return */
	return bbt;	
}

/*
 * Public Functions
 */

/* Update buffer reference count */
void opengl_buffer_obj_ref_update(struct opengl_buffer_obj_t *buffer_obj, int change)
{
	int count;

	pthread_mutex_lock(&buffer_obj->ref_mutex);
	buffer_obj->ref_count += change;
	count = buffer_obj->ref_count;
	pthread_mutex_unlock(&buffer_obj->ref_mutex);

	if (count < 0)
		panic("\t%s: number of references is negative", __FUNCTION__);
}

/* Created in opengl_context_init() */
struct opengl_buffer_binding_points_t *opengl_buffer_binding_points_create()
{
	struct opengl_buffer_binding_points_t *bbp;

	/* Allocate */
	bbp = xcalloc(1, sizeof(struct opengl_buffer_binding_points_t));
	bbp->array_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->atomic_counter_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->copy_read_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->copy_write_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->dispatch_indirect_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->draw_indirect_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->element_array_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->pixel_pack_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->pixel_unpack_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->shader_storage_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->texture_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->transform_feedback_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	bbp->uniform_buffer = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));

	/* Initialize all mutex */
	pthread_mutex_init(&bbp->array_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->atomic_counter_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->copy_read_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->copy_write_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->dispatch_indirect_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->draw_indirect_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->element_array_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->pixel_pack_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->pixel_unpack_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->shader_storage_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->texture_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->transform_feedback_buffer->mutex, NULL);
	pthread_mutex_init(&bbp->uniform_buffer->mutex, NULL);

	/* Debug */
	opengl_debug("\t%s: OpenGL buffer binding points [%p] created\n", __FUNCTION__, bbp);

	/* Return */	
	return bbp;
}

/* Free in opengl_context_destroy() */
void opengl_buffer_binding_points_free(struct opengl_buffer_binding_points_t *bbp)
{
	/* Debug */
	opengl_debug("\t%s: OpenGL buffer binding points [%p] freed\n", __FUNCTION__, bbp);

	/* Destroy all mutex */
	pthread_mutex_destroy(&bbp->array_buffer->mutex);
	pthread_mutex_destroy(&bbp->atomic_counter_buffer->mutex);
	pthread_mutex_destroy(&bbp->copy_read_buffer->mutex);
	pthread_mutex_destroy(&bbp->copy_write_buffer->mutex);
	pthread_mutex_destroy(&bbp->dispatch_indirect_buffer->mutex);
	pthread_mutex_destroy(&bbp->draw_indirect_buffer->mutex);
	pthread_mutex_destroy(&bbp->element_array_buffer->mutex);
	pthread_mutex_destroy(&bbp->pixel_pack_buffer->mutex);
	pthread_mutex_destroy(&bbp->pixel_unpack_buffer->mutex);
	pthread_mutex_destroy(&bbp->shader_storage_buffer->mutex);
	pthread_mutex_destroy(&bbp->texture_buffer->mutex);
	pthread_mutex_destroy(&bbp->transform_feedback_buffer->mutex);
	pthread_mutex_destroy(&bbp->uniform_buffer->mutex);

	/* Free binding targets */
	free(bbp->array_buffer);
	free(bbp->atomic_counter_buffer);
	free(bbp->copy_read_buffer);
	free(bbp->dispatch_indirect_buffer);
	free(bbp->draw_indirect_buffer);
	free(bbp->element_array_buffer);
	free(bbp->pixel_pack_buffer);
	free(bbp->pixel_unpack_buffer);
	free(bbp->shader_storage_buffer);
	free(bbp->texture_buffer);
	free(bbp->transform_feedback_buffer);
	free(bbp->uniform_buffer);	

	free(bbp);

}

struct opengl_buffer_binding_target_t *opengl_buffer_binding_points_get_target(
	struct opengl_buffer_binding_points_t *bbp, unsigned int target_id)
{
	struct opengl_buffer_binding_target_t *bbt;

	switch(target_id)
	{

	case GL_ARRAY_BUFFER:
	{
		bbt = bbp->array_buffer;
		break;
	}
	case GL_ATOMIC_COUNTER_BUFFER:
	{
		bbt = bbp->atomic_counter_buffer;
		break;
	}
	case GL_COPY_READ_BUFFER:
	{
		bbt = bbp->copy_read_buffer;
		break;
	}
	case GL_COPY_WRITE_BUFFER:
	{
		bbt = bbp->copy_write_buffer;
		break;
	}
	case GL_DRAW_INDIRECT_BUFFER:
	{
		bbt = bbp->draw_indirect_buffer;
		break;
	}
	case GL_DISPATCH_INDIRECT_BUFFER:
	{
		bbt = bbp->dispatch_indirect_buffer;
		break;		
	}
	case GL_ELEMENT_ARRAY_BUFFER:
	{
		bbt = bbp->element_array_buffer;
		break;		
	}
	case GL_PIXEL_PACK_BUFFER:
	{
		bbt = bbp->pixel_pack_buffer;
		break;		
	}
	case GL_PIXEL_UNPACK_BUFFER:
	{
		bbt = bbp->pixel_unpack_buffer;
		break;
	}
	case GL_SHADER_STORAGE_BUFFER:
	{
		bbt = bbp->shader_storage_buffer;
		break;
	}
	case GL_TEXTURE_BUFFER:
	{
		bbt = bbp->texture_buffer;
		break;
	}
	case GL_TRANSFORM_FEEDBACK_BUFFER:
	{
		bbt = bbp->transform_feedback_buffer;
		break;
	}
	case GL_UNIFORM_BUFFER:
	{
		bbt = bbp->uniform_buffer;
		break;
	}
	default:
		bbt = NULL;
		break;
	}

	/* Return */
	return bbt;
}

struct opengl_indexed_buffer_binding_points_t *opengl_indexed_buffer_binding_points_create(unsigned int max_indexed_target)
{
	struct opengl_indexed_buffer_binding_points_t *idx_bbp;
	int i;

	/* Allocate */
	idx_bbp = xcalloc(1, sizeof(struct opengl_indexed_buffer_binding_points_t));
	idx_bbp->atomic_counter_buffer = xcalloc(max_indexed_target, sizeof(struct opengl_buffer_binding_target_t *));
	idx_bbp->shader_storage_buffer = xcalloc(max_indexed_target, sizeof(struct opengl_buffer_binding_target_t *));
	idx_bbp->transform_feedback_buffer = xcalloc(max_indexed_target, sizeof(struct opengl_buffer_binding_target_t *));
	idx_bbp->uniform_buffer = xcalloc(max_indexed_target, sizeof(struct opengl_buffer_binding_target_t *));

	for (i = 0; i < max_indexed_target; ++i)
	{
		idx_bbp->atomic_counter_buffer[i] = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
		idx_bbp->shader_storage_buffer[i] = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
		idx_bbp->transform_feedback_buffer[i] = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
		idx_bbp->uniform_buffer[i] = xcalloc(1, sizeof(struct opengl_buffer_binding_target_t));
	}

	/* Initialize */
	idx_bbp->max_indexed_targets = max_indexed_target;
	for (i = 0; i < max_indexed_target; ++i)
	{
		pthread_mutex_init(&idx_bbp->atomic_counter_buffer[i]->mutex, NULL);
		pthread_mutex_init(&idx_bbp->shader_storage_buffer[i]->mutex, NULL);
		pthread_mutex_init(&idx_bbp->transform_feedback_buffer[i]->mutex, NULL);
		pthread_mutex_init(&idx_bbp->uniform_buffer[i]->mutex, NULL);
	}

	/* Debug */
	opengl_debug("\t%s: OpenGL indexed buffer binding points [%p] created\n", __FUNCTION__, idx_bbp);
	
	/* Return */	
	return idx_bbp;
}

void opengl_indexed_buffer_binding_points_free(struct opengl_indexed_buffer_binding_points_t *idx_bbp)
{
	int i;

	/* Debug */
	opengl_debug("\t%s: OpenGL indexed buffer binding points [%p] freed\n", __FUNCTION__, idx_bbp);

	for (i = 0; i < idx_bbp->max_indexed_targets; ++i)
	{
		free(idx_bbp->atomic_counter_buffer[i]);
		free(idx_bbp->shader_storage_buffer[i]);
		free(idx_bbp->transform_feedback_buffer[i]);
		free(idx_bbp->uniform_buffer[i]);
	}

	for (i = 0; i < idx_bbp->max_indexed_targets; ++i)
	{
		pthread_mutex_destroy(&idx_bbp->atomic_counter_buffer[i]->mutex);
		pthread_mutex_destroy(&idx_bbp->shader_storage_buffer[i]->mutex);
		pthread_mutex_destroy(&idx_bbp->transform_feedback_buffer[i]->mutex);
		pthread_mutex_destroy(&idx_bbp->uniform_buffer[i]->mutex);
	}

	free(idx_bbp);

}

struct linked_list_t *opengl_buffer_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	/* Debug */
	opengl_debug("\t%s: OpenGL Buffer Repository [%p] created\n", __FUNCTION__, lst);

	/* Return */	
	return lst;
}

void opengl_buffer_obj_repo_free(struct linked_list_t *buffer_obj_repo)
{
	struct opengl_buffer_obj_t *buffer_obj;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(buffer_obj_repo)
	{
		buffer_obj = linked_list_get(buffer_obj_repo);
		assert(buffer_obj);
		opengl_buffer_obj_free(buffer_obj);
	}

	/* Free buffer repository */
	linked_list_free(buffer_obj_repo);

	/* Debug */
	opengl_debug("\t%s: OpenGL Buffer Repository [%p] freed\n", __FUNCTION__, buffer_obj_repo);	
}

struct opengl_buffer_obj_t *opengl_buffer_obj_repo_get(
	struct linked_list_t *buffer_obj_repo, unsigned int id)
{
	/* ID 0 is reserved */
	if (id == 0)
		return NULL;

	struct opengl_buffer_obj_t *buffer_obj;

	/* Search Buffer Object */
	LINKED_LIST_FOR_EACH(buffer_obj_repo)
	{
		buffer_obj = linked_list_get(buffer_obj_repo);
		assert(buffer_obj);
		if (buffer_obj->id == id && !buffer_obj->delete_pending)
			return buffer_obj;
	}

	/* Not found or being deleted */
	fatal("\t%s: requested Buffer Object is not available (id=%d)",
		__FUNCTION__, id);
	return NULL;
}


/* 
 * OpenGL API functions 
 */


/* Buffer Objects [2.9-10] */

void glGenBuffers (GLsizei n, GLuint *buffers)
{
	/* Debug */
	opengl_debug("API call %s(%d, %p) \n", __FUNCTION__, n, buffers);

	int i;
	struct opengl_buffer_obj_t *buffer_obj;

	if (n < 0)
	{
		opengl_context_set_error(opengl_ctx, GL_INVALID_VALUE);
		return;
	}

	for (i = 0; i < n; ++i)
	{
		buffer_obj = opengl_buffer_obj_create();
		opengl_buffer_obj_repo_add(buffer_repo, buffer_obj);
		buffers[i] = buffer_obj->id;
	}
}

void glDeleteBuffers (GLsizei n, const GLuint *buffers)
{
	/* Debug */
	opengl_debug("API call %s(%d, %p)\n", __FUNCTION__, n, buffers);

	int i;
	struct opengl_buffer_obj_t *buffer_obj;

	if (n < 0)
	{
		opengl_context_set_error(opengl_ctx, GL_INVALID_VALUE);
		return;
	}

	for (i = 0; i < n; ++i)
	{
		buffer_obj = opengl_buffer_obj_repo_get(buffer_repo, buffers[i]);
		opengl_buffer_obj_repo_remove(buffer_repo, buffer_obj);
		opengl_buffer_obj_delete(buffer_obj);
	}

}


/* Creating and binding Buffer objects [2.9.1] */

void glBindBuffer (GLenum target, GLuint buffer)
{
	/* Debug */
	opengl_debug("API call %s(%x, %d)\n", __FUNCTION__, target, buffer);

	struct opengl_buffer_binding_target_t *target_obj;
	struct opengl_buffer_obj_t *buffer_obj;

	target_obj = opengl_buffer_binding_points_get_target(opengl_ctx->buffer_binding_points, target);
	if (buffer)
	{
		buffer_obj = opengl_buffer_obj_repo_get(buffer_repo, buffer);
		opengl_buffer_binding_target_bind_buffer(target_obj, buffer_obj);		
	}
	else
		opengl_buffer_binding_target_bind_buffer(target_obj, NULL);		

}

void glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferBase (GLenum target, GLuint index, GLuint buffer)
{
	/* Debug */
	opengl_debug("API call %s(%x, %d)\n", __FUNCTION__, target, buffer);

	struct opengl_buffer_binding_target_t *target_obj;
	struct opengl_buffer_binding_target_t *idxed_target_obj;
	struct opengl_buffer_obj_t *buffer_obj;

	/* Need to bind to generic binding target as well as indexed binding target */
	target_obj = opengl_buffer_binding_points_get_target(opengl_ctx->buffer_binding_points, target);
	idxed_target_obj = opengl_indexed_buffer_binding_points_get_target(opengl_ctx->idxed_buffer_binding_points, target, index);
	buffer_obj = opengl_buffer_obj_repo_get(buffer_repo, buffer);
	opengl_buffer_binding_target_bind_buffer(target_obj, buffer_obj);
	opengl_buffer_binding_target_bind_buffer(idxed_target_obj, buffer_obj);
}


/* Creating Buffer Object Data Stores [2.9.2] */

void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
	/* Debug */
	opengl_debug("API call %s(%x, %d, %p, %x)\n", 
		__FUNCTION__, target, size, data, usage);

	struct opengl_buffer_binding_target_t *target_obj;
	struct opengl_buffer_obj_t *buffer_obj;

	target_obj = opengl_buffer_binding_points_get_target(opengl_ctx->buffer_binding_points, target);
	buffer_obj = opengl_buffer_obj_repo_get(buffer_repo, target_obj->bound_buffer_id);
	
	/* GL_INVALID_ENUM is generated if target is not one of the accepted buffer targets */
	if (!target_obj)
	{
		opengl_context_set_error(opengl_ctx, GL_INVALID_ENUM);
		return;
	}

	/* GL_INVALID_OPERATION is generated if the reserved buffer object name 0 is bound to target. */
	if (target_obj->bound_buffer_id == 0)
	{
		opengl_context_set_error(opengl_ctx, GL_INVALID_OPERATION);
		return;
	}
	
	/* GL_INVALID_VALUE is generated if size is negative. */
	if (size < 0)
	{
		opengl_context_set_error(opengl_ctx, GL_INVALID_VALUE);
		return;
	}

	/* Save a copy in buffer object */
	opengl_buffer_obj_data(buffer_obj, size, data, usage);
	if(!data)
		opengl_debug("\tNULL data pointer\n");

	/* GL_OUT_OF_MEMORY is generated if the GL is unable to create a data store with the specified size. */
	if (!buffer_obj->data)
		opengl_context_set_error(opengl_ctx, GL_OUT_OF_MEMORY);
}


/* Mapping/Unmapping Buffer Data [2.9.3] */

GLvoid* glMapBufferRange (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLvoid* glMapBuffer (GLenum target, GLenum access)
{
	struct opengl_buffer_binding_target_t *target_obj;
	struct opengl_buffer_obj_t *buffer_obj;

	/* Debug */
	opengl_debug("API call %s(%x, %x)\n", __FUNCTION__, target, access);

	target_obj = opengl_buffer_binding_points_get_target(opengl_ctx->buffer_binding_points, target);
	buffer_obj = opengl_buffer_obj_repo_get(buffer_repo, target_obj->bound_buffer_id);
	buffer_obj->map_access_flags = access;
	buffer_obj->mapped = 1;
	buffer_obj->map_pointer = buffer_obj->data;
	buffer_obj->map_offset = 0;
	buffer_obj->map_length = buffer_obj->size;

	return buffer_obj->map_pointer;
}


void glFlushMappedBufferRange (GLenum target, GLintptr offset, GLsizeiptr length)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glUnmapBuffer (GLenum target)
{
	struct opengl_buffer_binding_target_t *target_obj;
	struct opengl_buffer_obj_t *buffer_obj;

	/* Debug */
	opengl_debug("API call %s(%x)\n", __FUNCTION__, target);

	target_obj = opengl_buffer_binding_points_get_target(opengl_ctx->buffer_binding_points, target);
	buffer_obj = opengl_buffer_obj_repo_get(buffer_repo, target_obj->bound_buffer_id);
	if (buffer_obj->mapped)
	{
		buffer_obj->map_access_flags = 0;
		buffer_obj->mapped = 0;
		buffer_obj->map_pointer = NULL;
		buffer_obj->map_offset = 0;
		buffer_obj->map_length = 0;
		return GL_TRUE;
	}
	else
	{
		/* Debug */
		opengl_debug("\t%s: Buffer %d [%p] not mapped\n", 
			__FUNCTION__, buffer_obj->id, buffer_obj);
		return GL_FALSE;
	}

}

/* Copying between buffers */
void glCopyBufferSubData (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}


/* Buffer Object Queries [6.1.9] [6.1.15] */

GLboolean glIsBuffer (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferParameteri64v (GLenum target, GLenum pname, GLint64 *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params)
{
	__OPENGL_NOT_IMPL__
}


void glClear( GLbitfield mask )
{
	/* Debug */
	opengl_debug("API call %s(%x)\n", __FUNCTION__, mask);

	/* FIXME */
	if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT))
	{
		opengl_debug("\tInvalid mask!\n");
		opengl_context_set_error(opengl_ctx, GL_INVALID_VALUE);
	}

	if ((mask & GL_COLOR_BUFFER_BIT) == GL_COLOR_BUFFER_BIT) 
	{
		opengl_debug("\tColor buffer cleared\n");
		/* TODO: Clear color buffer */
	}

	if ((mask & GL_DEPTH_BUFFER_BIT) == GL_DEPTH_BUFFER_BIT) 
	{
		opengl_debug("\tDepth buffer cleared\n");
		/* TODO: Clear depth buffer */
	}

	if ((mask & GL_STENCIL_BUFFER_BIT) == GL_STENCIL_BUFFER_BIT)
	{
		opengl_debug("\tStencil buffer cleared\n");
		/* TODO: Clear stencil buffer */
	}
}
