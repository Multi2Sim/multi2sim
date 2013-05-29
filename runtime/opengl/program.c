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
#include "elf-format.h"
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
	if (program_obj->binary)
		free(program_obj->binary);
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


/* Program Objects [2.11.3] [2.14.3] */

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

static char *opengl_err_program_compile =
	"\tYour guest OpenGL application is trying to link an OpenGL program\n"
	"\tby calling function 'glLinkProgram'. Dynamic shader compilation\n"
	"\tand program linking is currently not supported. The following two\n"
	"\talternatives are suggested instead:\n"
	"\n"
	"\t1) Use 'glProgramBinary' in your program instead of glLinkProgram \n"
	"\n"
	"\t2) You can force the Multi2Sim runtime to load a specific kernel\n"
	"\t   binary every time the application performs a call to\n"
	"\t   'glLinkProgram' by setting environment varaible\n"
	"\t   'M2S_OPENGL_BINARY' to the path of a pre-compiled binary that\n"
	"\t   will be passed to the application.\n";

static char *opengl_err_program_not_found =
	"\tYour guest OpenGL application has executed function\n"
	"\t'glLinkProgram', while the Multi2Sim runtime found that\n"
	"\tenvironment variable 'M2S_OPENGL_BINARY' was set. However, the\n"
	"\tbinary file pointed to by this variable was not found.\n";

void glLinkProgram (GLuint program)
{
	struct opengl_program_obj_t *program_obj;
	struct opengl_shader_obj_t *shader_obj;
	int index;

	char *binary_name;
	void *binary;
	unsigned int size;
	FILE *f;

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

	/* Runtime compilation is not supported, but the user can have activated
	 * environment variable 'M2S_OPENGL_BINARY', which will make the runtime
	 * load that specific pre-compiled shader binary. */
	binary_name = getenv("M2S_OPENGL_BINARY");
	if (!binary_name || !*binary_name)
		fatal("%s: runtime shader compilation not supported.\n%s",
				__FUNCTION__, opengl_err_program_compile);

	/* Load binary */
	f = fopen(binary_name, "rb");
	if (!f)
		fatal("%s: %s: cannot open file.\n%s", __FUNCTION__,
			binary_name, opengl_err_program_not_found);
	
	/* Allocate buffer */
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	binary = xmalloc(size);

	/* Read binary */
	fseek(f, 0, SEEK_SET);
	fread(binary, size, 1, f);
	fclose(f);
	
	/* Add to program object */
	program_obj->binary = binary;
	program_obj->binary_size = size;


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

		/* Call the driver to setup the program binary */
		syscall(OPENGL_SYSCALL_CODE, opengl_abi_si_program_create);
		syscall(OPENGL_SYSCALL_CODE, opengl_abi_si_program_set_binary, 
			program_obj->id, program_obj->binary, program_obj->binary_size);
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

void glDeleteProgram (GLuint program)
{
	struct opengl_program_obj_t *program_obj;

	/* Debug */
	opengl_debug("API call %s(%d)\n", __FUNCTION__, program);

	program_obj = opengl_program_obj_repo_get_program(program_repo, program);

	opengl_program_obj_repo_remove(program_repo, program_obj);
	opengl_program_obj_delete(program_obj);
	
}

GLuint glCreateShaderProgramv (GLenum type, GLsizei count, const GLchar* *strings)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glProgramParameteri (GLuint program, GLenum pname, GLint value)
{
	__OPENGL_NOT_IMPL__
}

/* Program pipeline objects [2.11.4] [2.14.4] */

void glGenProgramPipelines (GLsizei n, GLuint *pipelines)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteProgramPipelines (GLsizei n, const GLuint *pipelines)
{
	__OPENGL_NOT_IMPL__
}

void glBindProgramPipeline (GLuint pipeline)
{
	__OPENGL_NOT_IMPL__
}

void glUseProgramStages (GLuint pipeline, GLbitfield stages, GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glActiveShaderProgram (GLuint pipeline, GLuint program)
{
	__OPENGL_NOT_IMPL__
}


/* Program binary [2.11.5] [2.14.5] */

void glGetProgramBinary (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary)
{
	__OPENGL_NOT_IMPL__
}

void glProgramBinary (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length)
{
	__OPENGL_NOT_IMPL__
}

/* Vertex Attributes [2.11.6] [2.14.6] */

void glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetAttribLocation (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindAttribLocation (GLuint program, GLuint index, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}


/* Uniform variables */

GLint glGetUniformLocation (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glGetUniformBlockIndex (GLuint program, const GLchar *uniformBlockName)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetActiveUniformBlockName (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniformBlockiv (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformIndices (GLuint program, GLsizei uniformCount, const GLchar* *uniformNames, GLuint *uniformIndices)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniformName (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniformsiv (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1f (GLint location, GLfloat v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2f (GLint location, GLfloat v0, GLfloat v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1i (GLint location, GLint v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2i (GLint location, GLint v0, GLint v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3i (GLint location, GLint v0, GLint v1, GLint v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1d (GLint location, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2d (GLint location, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3d (GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4d (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1ui (GLint location, GLuint v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2ui (GLint location, GLuint v0, GLuint v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3ui (GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4ui (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x3dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x4dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x2dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x4dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x2dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x3dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1i (GLuint program, GLint location, GLint v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1f (GLuint program, GLint location, GLfloat v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1d (GLuint program, GLint location, GLdouble v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1ui (GLuint program, GLint location, GLuint v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2i (GLuint program, GLint location, GLint v0, GLint v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2f (GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2d (GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2ui (GLuint program, GLint location, GLuint v0, GLuint v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3d (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4d (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x3dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x2dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x4dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x2dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x4dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x3dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	__OPENGL_NOT_IMPL__
}


/* Subroutine Uniform Variables */

GLint glGetSubroutineUniformLocation (GLuint program, GLenum shadertype, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glGetSubroutineIndex (GLuint program, GLenum shadertype, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetActiveSubroutineUniformiv (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveSubroutineUniformName (GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveSubroutineName (GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glUniformSubroutinesuiv (GLenum shadertype, GLsizei count, const GLuint *indices)
{
	__OPENGL_NOT_IMPL__
}


/* Varying variables [2.11.12] [2.14.12] */

void glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar* *varyings, GLenum bufferMode)
{
	int i;

	/* Debug */
	opengl_debug("API call %s(%d, %d, %p, %x)\n", 
		__FUNCTION__, program, count, varyings, bufferMode);

	for (i = 0; i < count; ++i)
		opengl_debug("\tVarying #%d: %s\n", i, varyings[i]);

}

void glGetTransformFeedbackVarying (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}


/* Program queries [6.1.12] [6.1.18] */
void glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribdv (GLuint index, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribIiv (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribIuiv (GLuint index, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribLdv (GLuint index, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformfv (GLuint program, GLint location, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformdv (GLuint program, GLint location, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformiv (GLuint program, GLint location, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformuiv (GLuint program, GLint location, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformSubroutineuiv (GLenum shadertype, GLint location, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsProgram (GLuint program)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetProgramiv (GLuint program, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsProgramPipeline (GLuint pipeline)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetProgramPipelineiv (GLuint pipeline, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramPipelineInfoLog (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
	__OPENGL_NOT_IMPL__
}
