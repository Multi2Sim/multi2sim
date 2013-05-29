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
#include <string.h>

#include "debug.h"
#include "list.h"
#include "linked-list.h"
#include "mhandle.h"
#include "program.h"
#include "shader.h"

struct linked_list_t *shader_repo;
static unsigned int shader_obj_id = 0; /* ID 0 is reserved */

/* Forward declarataion */
static unsigned int opengl_shader_assign_id();

static struct opengl_shader_obj_t *opengl_shader_obj_create();
static void opengl_shader_obj_free(struct opengl_shader_obj_t *shader_obj);
static void opengl_shader_obj_delete(struct opengl_shader_obj_t *shader_obj);

static void opengl_shader_obj_repo_add(struct linked_list_t *shader_obj_repo, 
	struct opengl_shader_obj_t *shader_obj);
static int opengl_shader_obj_repo_remove(struct linked_list_t *shader_obj_repo, 
	struct opengl_shader_obj_t *shader_obj);

/*
 * Private Functions
 */

static unsigned int opengl_shader_assign_id()
{
	shader_obj_id += 1;
	return shader_obj_id;
}

static struct opengl_shader_obj_t *opengl_shader_obj_create(unsigned int type)
{
	struct opengl_shader_obj_t *shader_obj;

	/* Allocate */
	shader_obj = xcalloc(1, sizeof(struct opengl_shader_obj_t));

	/* Initialize */
	shader_obj->id = opengl_shader_assign_id();
	shader_obj->type = type;
	shader_obj->ref_count = 0;
	pthread_mutex_init(&shader_obj->mutex, NULL);
	shader_obj->delete_pending = GL_FALSE;
	shader_obj->bound_programs = list_create();

	shader_obj->source = list_create();

	/* Return */	
	return shader_obj;
}

static void opengl_shader_obj_free(struct opengl_shader_obj_t *shader_obj)
{
	struct opengl_program_obj_t *program_obj;
	char *source;
	int index;

	pthread_mutex_destroy(&shader_obj->mutex);

	if (shader_obj->source)
	{
		LIST_FOR_EACH(shader_obj->source, index)
		{
			source = list_get(shader_obj->source, index);
			free(source);
		}
	}
	list_free(shader_obj->source);


	/* Remove bindings from program objects */
	LIST_FOR_EACH(shader_obj->bound_programs, index)
	{
		program_obj = list_get(shader_obj->bound_programs, index);
		list_remove(program_obj->shaders, shader_obj);
	}

	list_free(shader_obj->bound_programs);
	free(shader_obj);
}

static void opengl_shader_obj_delete(struct opengl_shader_obj_t *shader_obj)
{
	shader_obj->delete_pending = 1;
	if (!shader_obj->ref_count)
		opengl_shader_obj_free(shader_obj);
}

static void opengl_shader_obj_repo_add(struct linked_list_t *shader_obj_repo, struct opengl_shader_obj_t *shader_obj)
{
	/* Add */
	linked_list_add(shader_obj_repo, shader_obj);

	/* Debug */
	opengl_debug("\t%s: Shader #%d [%p] add to Shader Repository [%p]\n", 
		__FUNCTION__, shader_obj->id, shader_obj, shader_obj_repo);
}

static int opengl_shader_obj_repo_remove(struct linked_list_t *shader_obj_repo, struct opengl_shader_obj_t *shader_obj)
{
	if (shader_obj->ref_count != 0)
	{
		opengl_debug("\t%s: Shader Object #%d [%p] cannot be removed immediately, as reference counter = %d\n", 
			__FUNCTION__, shader_obj->id, shader_obj, shader_obj->ref_count);
		return -1;
	}
	else 
	{
		/* Check if Shader Object exists */
		linked_list_find(shader_obj_repo, shader_obj);
		if (shader_obj_repo->error_code)
			fatal("\t%s: Shader Object does not exist", __FUNCTION__);
		linked_list_remove(shader_obj_repo);
		opengl_debug("\t%s: Remove Shader Object #%d [%p] from Shader Object Repository [%p]\n", 
			__FUNCTION__, shader_obj->id, shader_obj, shader_obj_repo);
		return 1;
	}
}

/*
 * Public Functions
 */

/* Update shader reference count */
void opengl_shader_obj_ref_update(struct opengl_shader_obj_t *shader_obj, int change)
{
	int count;

	pthread_mutex_lock(&shader_obj->mutex);
	shader_obj->ref_count += change;
	count = shader_obj->ref_count;
	pthread_mutex_unlock(&shader_obj->mutex);

	if (count < 0)
		panic("\t%s: number of references is negative", __FUNCTION__);
}

struct linked_list_t *opengl_shader_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	/* Debug */
	opengl_debug("\t%s: OpenGL Shader Repository [%p] created\n", __FUNCTION__, lst);

	/* Return */	
	return lst;
}

void opengl_shader_obj_repo_free(struct linked_list_t *shader_obj_repo)
{
	struct opengl_shader_obj_t *shader_obj;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(shader_obj_repo)
	{
		shader_obj = linked_list_get(shader_obj_repo);
		assert(shader_obj);
		opengl_shader_obj_free(shader_obj);
	}

	/* Free shader repository */
	linked_list_free(shader_obj_repo);

	/* Debug */
	opengl_debug("\t%s: OpenGL Shader Repository [%p] freed\n", __FUNCTION__, shader_obj_repo);	
}

struct opengl_shader_obj_t *opengl_shader_obj_repo_get_shader(struct linked_list_t *shader_obj_repo, unsigned int id)
{
	/* ID 0 is reserved */
	if (id == 0)
		return NULL;

	struct opengl_shader_obj_t *shader_obj;

	/* Search Shader Object */
	LINKED_LIST_FOR_EACH(shader_obj_repo)
	{
		shader_obj = linked_list_get(shader_obj_repo);
		assert(shader_obj);
		if (shader_obj->id == id && !shader_obj->delete_pending)
			return shader_obj;
	}

	/* Not found or being deleted */
	fatal("\t%s: requested Shader Object is not available (id=0x%d)",
		__FUNCTION__, id);
	return NULL;

}


/* 
 * OpenGL API functions 
 */

/* Shader Objects [2.11.1-2] [2.14.1-2] */
GLuint glCreateShader (GLenum type)
{
	struct opengl_shader_obj_t *shader_obj;

	/* Debug */
	opengl_debug("API call %s(%x)\n", __FUNCTION__, type);

	/* Create and add to repository */
	shader_obj = opengl_shader_obj_create(type);
	opengl_shader_obj_repo_add(shader_repo, shader_obj);

	/* Return */
	return shader_obj->id;
}

void glShaderSource (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length)
{
	struct opengl_shader_obj_t *shader_obj;
	char *source;
	int i;

	/* Debug */
	opengl_debug("API call %s(%d, %d, %p, %p)\n", 
		__FUNCTION__, shader, count, string, length);

	/* Get shader and store source code */
	shader_obj = opengl_shader_obj_repo_get_shader(shader_repo, shader);
	if (count && length)
	{
		for (i = 0; i < count; ++i)
		{
			source = xcalloc(1, length[i]);
			strncpy(source, string[i], length[i]);
		}
		list_add(shader_obj->source, source);
	}
	else
	{
		source = xstrdup(string[0]);
		list_add(shader_obj->source, source);
	}
}

void glCompileShader (GLuint shader)
{
	struct opengl_shader_obj_t *shader_obj;
	char *source;
	int index;

	/* Debug */
	opengl_debug("API call %s(%d)\n", __FUNCTION__, shader);

	/* Get shader and print source code */
	shader_obj = opengl_shader_obj_repo_get_shader(shader_repo, shader);
	LIST_FOR_EACH(shader_obj->source, index)
	{
		source = list_get(shader_obj->source, index);
		/* Debug */
		opengl_debug("\tShader Source: \n%s\n", source);
	}

	/* Shader compilation is not supported currently */
}

void glReleaseShaderCompiler (void)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteShader (GLuint shader)
{
	struct opengl_shader_obj_t *shader_obj;

	/* Debug */
	opengl_debug("API call %s(%d)\n", __FUNCTION__, shader);

	/* Get shader and delete */
	shader_obj = opengl_shader_obj_repo_get_shader(shader_repo, shader);
	opengl_shader_obj_repo_remove(shader_repo, shader_obj);
	opengl_shader_obj_delete(shader_obj);
}

void glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length)
{
	__OPENGL_NOT_IMPL__
}

/* Shader Execution [2.11.13] [2.14.13] */

void glValidateProgram (GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glValidateProgramPipeline (GLuint pipeline)
{
	__OPENGL_NOT_IMPL__
}


/*Shader Memory Access [2.11.4] [2.14.14] */

void glMemoryBarrier (GLbitfield barriers)
{
	__OPENGL_NOT_IMPL__
}


/* Tessellation Control Shaders */

void glPatchParameterfv (GLenum pname, const GLfloat *values)
{
	__OPENGL_NOT_IMPL__
}


/* Fragment Shaders [2.10.2] [3.13.2] */

void glBindFragDataLocation (GLuint program, GLuint color, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glBindFragDataLocationIndexed (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetFragDataLocation (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLint glGetFragDataIndex (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}


/* Shader queries */

GLboolean glIsShader (GLuint shader)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetShaderiv (GLuint shader, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramStageiv (GLuint program, GLenum shadertype, GLenum pname, GLint *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
	__OPENGL_NOT_IMPL__
}
