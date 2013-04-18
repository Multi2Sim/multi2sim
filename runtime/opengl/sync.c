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
#include "sync.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */


void glFinish( void )
{
	__OPENGL_NOT_IMPL__
}


void glFlush( void )
{
	__OPENGL_NOT_IMPL__
}

GLsync glFenceSync (GLenum condition, GLbitfield flags)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDeleteSync (GLsync sync)
{
	__OPENGL_NOT_IMPL__
}

GLenum glClientWaitSync (GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glWaitSync (GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	__OPENGL_NOT_IMPL__
}

void glGetSynciv (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsSync (GLsync sync)
{
	__OPENGL_NOT_IMPL__
	return 0;
}


