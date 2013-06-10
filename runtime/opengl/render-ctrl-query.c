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

#include "mhandle.h"
#include "debug.h"
#include "render-ctrl-query.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */

void glBeginQuery (GLenum target, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glEndQuery (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glBeginQueryIndexed (GLenum target, GLuint index, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glEndQueryIndexed (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glGenQueries (GLsizei n, GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteQueries (GLsizei n, const GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glBeginConditionalRender (GLuint id, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glEndConditionalRender (void)
{
	__OPENGL_NOT_IMPL__
}

void glGenTransformFeedbacks (GLsizei n, GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteTransformFeedbacks (GLsizei n, const GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glBindTransformFeedback (GLenum target, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glBeginTransformFeedback (GLenum primitiveMode)
{
	/* Debug */
	opengl_debug("API call %s(%x)\n", 
		__FUNCTION__, primitiveMode);

	/* FIXME */
}

void glEndTransformFeedback (void)
{
	/* Debug */
	opengl_debug("API call %s\n", __FUNCTION__);

	/* FIXME */
}

void glPauseTransformFeedback (void)
{
	__OPENGL_NOT_IMPL__
}

void glResumeTransformFeedback (void)
{
	__OPENGL_NOT_IMPL__
}

void glDrawTransformFeedback (GLenum mode, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glDrawTransformFeedbackStreamInstanced (GLenum mode, GLuint id, GLuint stream, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glDrawTransformFeedbackStream (GLenum mode, GLuint id, GLuint stream)
{
	__OPENGL_NOT_IMPL__
}

void glDrawTransformFeedbackInstanced (GLenum mode, GLuint id, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsTransformFeedback (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glRasterPos2d( GLdouble x, GLdouble y )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos2f( GLfloat x, GLfloat y )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos2i( GLint x, GLint y )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos2s( GLshort x, GLshort y )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3d( GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3f( GLfloat x, GLfloat y, GLfloat z )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3i( GLint x, GLint y, GLint z )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3s( GLshort x, GLshort y, GLshort z )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4i( GLint x, GLint y, GLint z, GLint w )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos2dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos2fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos2iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos2sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos3sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glRasterPos4sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2d (GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2dv (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2f (GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2fv (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2i (GLint x, GLint y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2iv (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2s (GLshort x, GLshort y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2sv (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3d (GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3dv (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3f (GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3fv (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3i (GLint x, GLint y, GLint z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3iv (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3s (GLshort x, GLshort y, GLshort z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3sv (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}


GLboolean glIsQuery (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetQueryiv (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryIndexediv (GLenum target, GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjectiv (GLuint id, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjecti64v (GLuint id, GLenum pname, GLint64 *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjectui64v (GLuint id, GLenum pname, GLuint64 *params)
{
	__OPENGL_NOT_IMPL__
}
