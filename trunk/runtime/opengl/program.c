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

#include "context.h"
#include "debug.h"
#include "list.h"
#include "linked-list.h"
#include "mhandle.h"
#include "program.h"
#include "shader.h"

/* Global variables*/
struct linked_list_t *program_repo;
static unsigned int program_obj_id = 0;

/* Forward declaration */
static unsigned int opengl_program_assign_id();

static struct opengl_program_obj_t *opengl_program_obj_create();
static void opengl_program_obj_free(struct opengl_program_obj_t *program_obj);
static void opengl_program_obj_delete(struct opengl_program_obj_t *program_obj);
static void opengl_program_obj_ref_update(struct opengl_program_obj_t *program_obj, int change);

static void opengl_program_obj_repo_add(struct linked_list_t *program_obj_repo, struct opengl_program_obj_t *program_obj);
static int opengl_program_obj_repo_remove(struct linked_list_t *program_obj_repo, struct opengl_program_obj_t *program_obj);

/*
 * Private Functions
 */

static unsigned int opengl_program_assign_id()
{
	program_obj_id += 1;
	return program_obj_id;
}

static struct opengl_program_obj_t *opengl_program_obj_create()
{
	struct opengl_program_obj_t *program_obj;

	/* Allocate */
	program_obj = xcalloc(1, sizeof(struct opengl_program_obj_t));

	/* Initialize */
	program_obj->id = opengl_program_assign_id();
	program_obj->ref_count = 0;
	pthread_mutex_init(&program_obj->mutex, NULL);
	program_obj->delete_pending = GL_FALSE;
	program_obj->shaders = list_create();

	/* Return */	
	return program_obj;
}

static void opengl_program_obj_free(struct opengl_program_obj_t *program_obj)
{
	struct opengl_shader_obj_t *shader_obj;
	int index;

	pthread_mutex_destroy(&program_obj->mutex);

	/* Remove bindings from shader objects */
	LIST_FOR_EACH(program_obj->shaders, index)
	{
		shader_obj = list_get(program_obj->shaders, index);
		list_remove(shader_obj->bound_programs, program_obj);
	}

	list_free(program_obj->shaders);
	free(program_obj);
}

static void opengl_program_obj_delete(struct opengl_program_obj_t *program_obj)
{
	program_obj->delete_pending = 1;
	if (!program_obj->ref_count)
		opengl_program_obj_free(program_obj);
}

/* Update program reference count */
static void opengl_program_obj_ref_update(struct opengl_program_obj_t *program_obj, int change)
{
	int count;

	pthread_mutex_lock(&program_obj->mutex);
	program_obj->ref_count += change;
	count = program_obj->ref_count;
	pthread_mutex_unlock(&program_obj->mutex);

	if (count < 0)
		panic("\t%s: number of references is negative", __FUNCTION__);
}

static void opengl_program_obj_repo_add(struct linked_list_t *program_obj_repo, struct opengl_program_obj_t *program_obj)
{
	/* Add */
	linked_list_add(program_obj_repo, program_obj);

	/* Debug */
	opengl_debug("\t%s: Program #%d [%p] add to Program Repository [%p]\n", 
		__FUNCTION__, program_obj->id, program_obj, program_obj_repo);
}

static int opengl_program_obj_repo_remove(struct linked_list_t *program_obj_repo, struct opengl_program_obj_t *program_obj)
{
	if (program_obj->ref_count != 0)
	{
		opengl_debug("\t%s: Program Object #%d [%p] cannot be removed immediately, as reference counter = %d\n", 
			__FUNCTION__, program_obj->id, program_obj, program_obj->ref_count);
		return -1;
	}
	else 
	{
		/* Check if Program Object exists */
		linked_list_find(program_obj_repo, program_obj);
		if (program_obj_repo->error_code)
			fatal("\t%s: Program Object does not exist", __FUNCTION__);
		linked_list_remove(program_obj_repo);
		opengl_debug("\t%s: Remove Program Object #%d [%p] from Program Object Repository [%p]\n", 
			__FUNCTION__, program_obj->id, program_obj, program_obj_repo);
		return 1;
	}
}


/*
 * Public Functions
 */

struct opengl_program_obj_t *opengl_program_obj_repo_get_program(struct linked_list_t *program_obj_repo, unsigned int id)
{
	/* ID 0 is reserved */
	if (id == 0)
		return NULL;

	struct opengl_program_obj_t *program_obj;

	/* Search Program Object */
	LINKED_LIST_FOR_EACH(program_obj_repo)
	{
		program_obj = linked_list_get(program_obj_repo);
		assert(program_obj);
		if (program_obj->id == id && !program_obj->delete_pending)
			return program_obj;
	}

	/* Not found or being deleted */
	fatal("\t%s: requested Program Object is not available (id=0x%d)",
		__FUNCTION__, id);
	return NULL;

}

struct linked_list_t *opengl_program_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	/* Return */
	return lst;
}

void opengl_program_obj_repo_free(struct linked_list_t *program_obj_repo)
{
	struct opengl_program_obj_t *program_obj;

	LINKED_LIST_FOR_EACH(program_obj_repo)
	{
		program_obj = linked_list_get(program_obj_repo);
		opengl_program_obj_free(program_obj);
	}
}


/* 
 * OpenGL API functions 
 */

GLuint glCreateProgram (void)
{
	struct opengl_program_obj_t *program_obj;

	/* Debug */
	opengl_debug("API call %s()\n", __FUNCTION__);

	/* Create and add to reposotory */
	program_obj = opengl_program_obj_create();
	opengl_program_obj_repo_add(program_repo, program_obj);

	/* Return */
	return program_obj->id;
}

void glDeleteProgram (GLuint program)
{
	struct opengl_program_obj_t *program_obj;

	/* Debug */
	opengl_debug("API call %s(%d)\n", __FUNCTION__, program);

	program_obj = opengl_program_obj_repo_get_program(program_repo, program);

	opengl_program_obj_repo_remove(program_repo, program_obj);
	opengl_program_obj_delete(program_obj);
	
}

void glAttachShader (GLuint program, GLuint shader)
{
	struct opengl_program_obj_t *program_obj;
	struct opengl_shader_obj_t *shader_obj;

	/* Debug */
	opengl_debug("API call %s(%d, %d)\n", __FUNCTION__, program, shader);

	program_obj = opengl_program_obj_repo_get_program(program_repo, program);
	shader_obj = opengl_shader_obj_repo_get_shader(shader_repo, shader);

	/* Update Shader info */
	opengl_shader_obj_ref_update(shader_obj, 1);
	list_add(shader_obj->bound_programs, program_obj);

	/* Update Program info */
	list_add(program_obj->shaders, shader_obj);

}

void glDetachShader (GLuint program, GLuint shader)
{
	struct opengl_program_obj_t *program_obj;
	struct opengl_shader_obj_t *shader_obj;

	/* Debug */
	opengl_debug("API call %s(%d, %d)\n", __FUNCTION__, program, shader);

	program_obj = opengl_program_obj_repo_get_program(program_repo, program);
	shader_obj = opengl_shader_obj_repo_get_shader(shader_repo, shader);

	/* Update Shader info */
	opengl_shader_obj_ref_update(shader_obj, -1);
	list_remove(shader_obj->bound_programs, program_obj);

	/* Update Program info */
	list_remove(program_obj->shaders, shader_obj);
}


void glLinkProgram (GLuint program)
{
	struct opengl_program_obj_t *program_obj;
	struct opengl_shader_obj_t *shader_obj;
	int index;

	/* Debug */
	opengl_debug("API call %s(%d)\n", __FUNCTION__, program);

	/* Get program and check shader information */
	program_obj = opengl_program_obj_repo_get_program(program_repo, program);
	LIST_FOR_EACH(program_obj->shaders, index)
	{
		shader_obj = list_get(program_obj->shaders, index);
		
		/* Debug */
		opengl_debug("\t%s: Link Shader Object #%d [%p] to Program #%d [%p] \n", 
			__FUNCTION__, shader_obj->id, shader_obj, program_obj->id, program_obj);
	}
}

void glUseProgram (GLuint program)
{
	struct opengl_program_obj_t *program_obj;

	/* Debug */
	opengl_debug("API call %s(%d)\n", __FUNCTION__, program);

	if (program != 0)
	{
		if (opengl_ctx->program_binding_point != NULL)
		{
			/* Clear binding point */
			program_obj = opengl_ctx->program_binding_point;
			opengl_program_obj_ref_update(program_obj, -1);
		}

		/* Get new program and attach to binding point */
		program_obj = opengl_program_obj_repo_get_program(program_repo, program);
		opengl_ctx->program_binding_point = program_obj;
		opengl_program_obj_ref_update(program_obj, 1);
	}
	else
	{
		if (opengl_ctx->program_binding_point != NULL)
		{
			/* Clear binding point */
			program_obj = opengl_ctx->program_binding_point;
			opengl_program_obj_ref_update(program_obj, -1);
		}

		opengl_ctx->program_binding_point = NULL;
	}
}
