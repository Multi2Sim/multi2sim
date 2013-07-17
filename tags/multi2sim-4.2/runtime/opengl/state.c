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

#include "context.h"
#include "debug.h"
#include "mhandle.h"
#include "state.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */

void glGetBooleanv( GLenum pname, GLboolean *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetIntegerv( GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetInteger64v (GLenum pname, GLint64 *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFloatv( GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetDoublev( GLenum pname, GLdouble *params )
{
	__OPENGL_NOT_IMPL__
}

void glGetBooleani_v (GLenum target, GLuint index, GLboolean *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetIntegeri_v (GLenum target, GLuint index, GLint *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetFloati_v (GLenum target, GLuint index, GLfloat *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetDoublei_v (GLenum target, GLuint index, GLdouble *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetInteger64i_v (GLenum target, GLuint index, GLint64 *data)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsEnabled( GLenum cap )
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glIsEnabledi (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetPointerv( GLenum pname, GLvoid **params )
{
	__OPENGL_NOT_IMPL__
}

const GLubyte *glGetString( GLenum name )
{
	opengl_debug("API call %s \n", __FUNCTION__);

	const GLubyte *str;

	switch(name)
	{

	case GL_VENDOR:
	{
		sprintf(opengl_ctx->props->info, "%s\n", opengl_ctx->props->vendor);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	case GL_RENDERER:
	{
		sprintf(opengl_ctx->props->info, "%s\n", opengl_ctx->props->renderer);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	case GL_VERSION:
	{
		sprintf(opengl_ctx->props->info, "Multi2Sim OpenGL %u.%u\n", 
			opengl_ctx->props->gl_major, opengl_ctx->props->gl_minor);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	case GL_SHADING_LANGUAGE_VERSION:
	{
		sprintf(opengl_ctx->props->info, "Multi2Sim OpenGL GLSL %u.%u\n", 
			opengl_ctx->props->gl_major, opengl_ctx->props->gl_minor);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	default:
		str = NULL;
		break;
	}

	/* Return */
	return str;
}


const GLubyte * glGetStringi (GLenum name, GLuint index)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glPushAttrib( GLbitfield mask )
{
	__OPENGL_NOT_IMPL__
}


void glPopAttrib( void )
{
	__OPENGL_NOT_IMPL__
}


void glPushClientAttrib( GLbitfield mask )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}


void glPopClientAttrib( void )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}


