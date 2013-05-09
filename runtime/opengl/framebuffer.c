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
#include "framebuffer.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */

/* Frame buffer objects */
/* Binding and managing [4.4.1] */

void glBindFramebuffer (GLenum target, GLuint framebuffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenFramebuffers (GLsizei n, GLuint *framebuffers)
{
	__OPENGL_NOT_IMPL__
}

/* Attach images [4.4.2] */

void glBindRenderbuffer (GLenum target, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenRenderbuffers (GLsizei n, GLuint *renderbuffers)
{
	__OPENGL_NOT_IMPL__
}

void glRenderbufferStorageMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTextureLayer (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__OPENGL_NOT_IMPL__
}


/* Framebuffer completeness [4.4.4] */

GLenum glCheckFramebufferStatus (GLenum target)
{
	__OPENGL_NOT_IMPL__
	return 0;
}


/* Framebuffer object queries [6.1.13] [6.1.19] */

GLboolean glIsFramebuffer (GLuint framebuffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}


/* Renderbuffer objects queries [6.1.14] [6.1.20] */

GLboolean glIsRenderbuffer (GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

/* Whole framebuffer */
/* Selecting a buffer for writing [4.2.1] */

 void glDrawBuffer( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}

void glDrawBuffers (GLsizei n, const GLenum *bufs)
{
	__OPENGL_NOT_IMPL__
}


/* Fine control of buffer updates [4.2.2] */

void glIndexMask( GLuint mask )
{
	__OPENGL_NOT_IMPL__
}

void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColorMaski (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	__OPENGL_NOT_IMPL__
}

void glStencilMask( GLuint mask )
{
	__OPENGL_NOT_IMPL__
}

void glStencilMaskSeparate (GLenum face, GLuint mask)
{
	__OPENGL_NOT_IMPL__
}

void glDepthMask( GLboolean flag )
{
	__OPENGL_NOT_IMPL__
}


/* Clearing the buffers [4.2.3] */

void glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	/* Debug */
	opengl_debug("API call %s(%f, %f, %f, %f)\n", 
		__FUNCTION__, red, green, blue, alpha);

	/* Specify a clear value for the color buffers */
	opengl_ctx->props->color[0] = red;
	opengl_ctx->props->color[0] = green;
	opengl_ctx->props->color[0] = blue;
	opengl_ctx->props->color[0] = alpha;
}

void glClearDepth( GLclampd depth )
{
	/* Debug */
	opengl_debug("API call %s(%f)\n", __FUNCTION__, depth);

	/* Specify a clear value for the depth buffer */
	opengl_ctx->props->depth = depth;
}

void glClearDepthf (GLclampf d)
{
	__OPENGL_NOT_IMPL__
}

void glClearIndex( GLfloat c )
{
	/* Debug */
	opengl_debug("API call %s(%f)\n", __FUNCTION__, c);

	/* FIXME: Specify a clear value for the color index buffers */

}

void glClearStencil( GLint s )
{
	/* Debug */
	opengl_debug("API call %s(%d)\n", __FUNCTION__, s);

	/* Specify a clear value for the stencil buffer */
	opengl_ctx->props->stencil = s;
}

void glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferiv (GLenum buffer, GLint drawbuffer, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferuiv (GLenum buffer, GLint drawbuffer, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferfv (GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferfi (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	__OPENGL_NOT_IMPL__
}


/* Acculumation buffer [4.2.4] */

void glAccum( GLenum op, GLfloat value )
{
	__OPENGL_NOT_IMPL__
}


