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
#include "special-func.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */

void glMap1d( GLenum target, GLdouble u1, GLdouble u2,
	GLint stride,
	GLint order, const GLdouble *points )
{
	__OPENGL_NOT_IMPL__
}


void glMap1f( GLenum target, GLfloat u1, GLfloat u2,
	GLint stride,
	GLint order, const GLfloat *points )
{
	__OPENGL_NOT_IMPL__
}


void glMap2d( GLenum target,
	GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
	GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
	const GLdouble *points )
{
	__OPENGL_NOT_IMPL__
}


void glMap2f( GLenum target,
	GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
	GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
	const GLfloat *points )
{
	__OPENGL_NOT_IMPL__
}

void glEvalCoord1d( GLdouble u )
{
	__OPENGL_NOT_IMPL__
}


void glEvalCoord1f( GLfloat u )
{
	__OPENGL_NOT_IMPL__
}


void glEvalCoord1dv( const GLdouble *u )
{
	__OPENGL_NOT_IMPL__
}


void glEvalCoord1fv( const GLfloat *u )
{
	__OPENGL_NOT_IMPL__
}


void glEvalCoord2d( GLdouble u, GLdouble v )
{
	__OPENGL_NOT_IMPL__
}


void glEvalCoord2f( GLfloat u, GLfloat v )
{
	__OPENGL_NOT_IMPL__
}


void glEvalCoord2dv( const GLdouble *u )
{
	__OPENGL_NOT_IMPL__
}


void glEvalCoord2fv( const GLfloat *u )
{
	__OPENGL_NOT_IMPL__
}

void glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 )
{
	__OPENGL_NOT_IMPL__
}


void glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 )
{
	__OPENGL_NOT_IMPL__
}


void glMapGrid2d( GLint un, GLdouble u1, GLdouble u2,
	GLint vn, GLdouble v1, GLdouble v2 )
{
	__OPENGL_NOT_IMPL__
}


void glMapGrid2f( GLint un, GLfloat u1, GLfloat u2,
	GLint vn, GLfloat v1, GLfloat v2 )
{
	__OPENGL_NOT_IMPL__
}

void glEvalPoint1( GLint i )
{
	__OPENGL_NOT_IMPL__
}


void glEvalPoint2( GLint i, GLint j )
{
	__OPENGL_NOT_IMPL__
}


void glEvalMesh1( GLenum mode, GLint i1, GLint i2 )
{
	__OPENGL_NOT_IMPL__
}


void glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
	__OPENGL_NOT_IMPL__
}

void glGetMapdv( GLenum target, GLenum query, GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glGetMapfv( GLenum target, GLenum query, GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glGetMapiv( GLenum target, GLenum query, GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glInitNames( void )
{
	__OPENGL_NOT_IMPL__
}


void glLoadName( GLuint name )
{
	__OPENGL_NOT_IMPL__
}


void glPushName( GLuint name )
{
	__OPENGL_NOT_IMPL__
}


void glPopName( void )
{
	__OPENGL_NOT_IMPL__
}

GLint glRenderMode( GLenum mode )
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glSelectBuffer( GLsizei size, GLuint *buffer )
{
	__OPENGL_NOT_IMPL__
}

void glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )
{
	__OPENGL_NOT_IMPL__
}

void glPassThrough( GLfloat token )
{
	__OPENGL_NOT_IMPL__
}

void glQueryCounter (GLuint id, GLenum target)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsList( GLuint list )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


void glDeleteLists( GLuint list, GLsizei range )
{
	__OPENGL_NOT_IMPL__
}


GLuint glGenLists( GLsizei range )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


void glNewList( GLuint list, GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glEndList( void )
{
	__OPENGL_NOT_IMPL__
}


void glCallList( GLuint list )
{
	__OPENGL_NOT_IMPL__
}


void glCallLists( GLsizei n, GLenum type,
	const GLvoid *lists )
{
	__OPENGL_NOT_IMPL__
}


void glListBase( GLuint base )
{
	__OPENGL_NOT_IMPL__
}

