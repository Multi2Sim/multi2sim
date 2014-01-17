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
#include "matrix.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */

/* Matrices [2.12.1] */
void glMatrixMode( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}

void glLoadMatrixd( const GLdouble *m )
{
	__OPENGL_NOT_IMPL__
}


void glLoadMatrixf( const GLfloat *m )
{
	__OPENGL_NOT_IMPL__
}

void glMultMatrixd( const GLdouble *m )
{
	__OPENGL_NOT_IMPL__
}


void glMultMatrixf( const GLfloat *m )
{
	__OPENGL_NOT_IMPL__
}

void glLoadTransposeMatrixd( const GLdouble m[16] )
{
	__OPENGL_NOT_IMPL__
}


void glLoadTransposeMatrixf( const GLfloat m[16] )
{
	__OPENGL_NOT_IMPL__
}

void glMultTransposeMatrixd( const GLdouble m[16] )
{
	__OPENGL_NOT_IMPL__
}


void glMultTransposeMatrixf( const GLfloat m[16] )
{
	__OPENGL_NOT_IMPL__
}

void glLoadIdentity( void )
{
	__OPENGL_NOT_IMPL__
}

void glRotated( GLdouble angle,
                                   GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glRotatef( GLfloat angle,
                                   GLfloat x, GLfloat y, GLfloat z )
{
	__OPENGL_NOT_IMPL__
}

void glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
	__OPENGL_NOT_IMPL__
}

void glScaled( GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glScalef( GLfloat x, GLfloat y, GLfloat z )
{
	__OPENGL_NOT_IMPL__
}

void glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_val, GLdouble far_val )
{
	__OPENGL_NOT_IMPL__
}

void glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val )
{
	__OPENGL_NOT_IMPL__
}

void glPushMatrix( void )
{
	__OPENGL_NOT_IMPL__
}


void glPopMatrix( void )
{
	__OPENGL_NOT_IMPL__
}

