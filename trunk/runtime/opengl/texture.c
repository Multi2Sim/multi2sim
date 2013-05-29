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

#include <unistd.h>
#include <stdio.h>

#include "debug.h"
#include "linked-list.h"
#include "mhandle.h"
#include "texture.h"

struct linked_list_t *texture_repo;
struct linked_list_t *sampler_repo;

/*
 * Private Functions
 */



/*
 * Public Functions
 */

struct linked_list_t *opengl_texture_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = xcalloc(1, sizeof(struct linked_list_t));

	/* Return */
	return lst;
}
void opengl_texture_obj_repo_free(struct linked_list_t *lst)
{
	/* Free */
	free(lst);
}


/* 
 * OpenGL API functions 
 */


/* Texuring [3.9] [3.10] */

 void glActiveTexture( GLenum texture )
{
	__OPENGL_NOT_IMPL__
}

/* Texture objects [3.9.1] [3.10.1] */

void glBindTexture( GLenum target, GLuint texture )
{
	__OPENGL_NOT_IMPL__
}

void glDeleteTextures( GLsizei n, const GLuint *textures)
{
	__OPENGL_NOT_IMPL__
}

void glGenTextures( GLsizei n, GLuint *textures )
{
	__OPENGL_NOT_IMPL__
}

GLboolean glAreTexturesResident( GLsizei n,
	const GLuint *textures,
	GLboolean *residences )
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glPrioritizeTextures( GLsizei n,
	const GLuint *textures,
	const GLclampf *priorities )
{
	__OPENGL_NOT_IMPL__
}


/* Sampler objects [3.9.2] [3.10.2] */

void glGenSamplers (GLsizei count, GLuint *samplers)
{
	__OPENGL_NOT_IMPL__
}

void glBindSampler (GLuint unit, GLuint sampler)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameteri (GLuint sampler, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameteriv (GLuint sampler, GLenum pname, const GLint *param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterf (GLuint sampler, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterfv (GLuint sampler, GLenum pname, const GLfloat *param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterIiv (GLuint sampler, GLenum pname, const GLint *param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterIuiv (GLuint sampler, GLenum pname, const GLuint *param)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteSamplers (GLsizei count, const GLuint *samplers)
{
	__OPENGL_NOT_IMPL__
}


/* Texture image spec [3.9.3] [3.10.3] */

void glTexImage3D( GLenum target, GLint level,
	GLint internalFormat,
	GLsizei width, GLsizei height,
	GLsizei depth, GLint border,
	GLenum format, GLenum type,
	const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}

void glTexImage2D( GLenum target, GLint level,
	GLint internalFormat,
	GLsizei width, GLsizei height,
	GLint border, GLenum format, GLenum type,
	const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}

void glTexImage1D( GLenum target, GLint level,
	GLint internalFormat,
	GLsizei width, GLint border,
	GLenum format, GLenum type,
	const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


/* Alternate texute image spec [3.9.4] [3.10.4] */

void glCopyTexImage2D( GLenum target, GLint level,
	GLenum internalformat,
	GLint x, GLint y,
	GLsizei width, GLsizei height,
	GLint border )
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexImage1D( GLenum target, GLint level,
	GLenum internalformat,
	GLint x, GLint y,
	GLsizei width, GLint border )
{
	__OPENGL_NOT_IMPL__
}

void glTexSubImage3D( GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLint zoffset, GLsizei width,
	GLsizei height, GLsizei depth,
	GLenum format,
	GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTexSubImage2D( GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height,
	GLenum format, GLenum type,
	const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}

void glTexSubImage1D( GLenum target, GLint level,
	GLint xoffset,
	GLsizei width, GLenum format,
	GLenum type, const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexSubImage3D( GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLint zoffset, GLint x,
	GLint y, GLsizei width,
	GLsizei height )
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexSubImage2D( GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLint x, GLint y,
	GLsizei width, GLsizei height )
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexSubImage1D( GLenum target, GLint level,
	GLint xoffset, GLint x, GLint y,
	GLsizei width )
{
	__OPENGL_NOT_IMPL__
}


/* Compressed texture image [3.9.5] [3.10.5] */

void glCompressedTexImage1D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


/* Multisample textures [3.9.6] [3.10.6] */

void glTexImage3DMultisample (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	__OPENGL_NOT_IMPL__
}

void glTexImage2DMultisample (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	__OPENGL_NOT_IMPL__
}


/* Buffer textures [3.9.7] [3.10.7] */

void glTexBuffer (GLenum target, GLenum internalformat, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}


/* Texture parameters [3.9.8] [3.10.8] */

void glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glTexParameteri( GLenum target, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glTexParameterfv( GLenum target, GLenum pname,
	const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexParameteriv( GLenum target, GLenum pname,
	const GLint *params )
{
	__OPENGL_NOT_IMPL__
}

void glTexParameterIiv (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glTexParameterIuiv (GLenum target, GLenum pname, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}


/* Texture minification [3.9.11] [3.10.11] */

void glGenerateMipmap (GLenum target)
{
	__OPENGL_NOT_IMPL__
}


/* Immutable-format texture images */

void glTexStorage1D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glTexStorage3D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	__OPENGL_NOT_IMPL__
}


/* Texture environments & funcions [3.10.17] */

void glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glTexEnvi( GLenum target, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexEnviv( GLenum target, GLenum pname, const GLint *params )
{
	__OPENGL_NOT_IMPL__
}


/* Texture image loads/stores [3.9.20] [3.10.20] */

void glBindImageTexture (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
	__OPENGL_NOT_IMPL__
}


/* Enumerated queries [6.1.15] [6.1.21] */

void glGetInternalformativ (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}

/* Texture Coordinates [2.12.3] */

void glTexGend( GLenum coord, GLenum pname, GLdouble param )
{
	__OPENGL_NOT_IMPL__
}


void glTexGenf( GLenum coord, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glTexGeni( GLenum coord, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glTexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterfv( GLenum target,
	GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}


void glGetTexParameteriv( GLenum target,
	GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterIiv (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterIuiv (GLenum target, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexLevelParameterfv( GLenum target, GLint level,
	GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexLevelParameteriv( GLenum target, GLint level,
	GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}


/* Texture queries [6.1.4] */

void glGetTexImage( GLenum target, GLint level,
	GLenum format, GLenum type,
	GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}

void glGetCompressedTexImage( GLenum target, GLint lod, GLvoid *img )
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsTexture( GLuint texture )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


/* Sampler queries */

GLboolean glIsSampler (GLuint sampler)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetSamplerParameteriv (GLuint sampler, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSamplerParameterIiv (GLuint sampler, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSamplerParameterfv (GLuint sampler, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSamplerParameterIuiv (GLuint sampler, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}






