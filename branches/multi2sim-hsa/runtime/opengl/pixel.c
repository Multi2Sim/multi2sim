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
#include "pixel.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */

void glReadPixels( GLint x, GLint y,
	GLsizei width, GLsizei height,
	GLenum format, GLenum type,
	GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}

void glReadBuffer( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}

void glCopyPixels( GLint x, GLint y,
	GLsizei width, GLsizei height,
	GLenum type )
{
	__OPENGL_NOT_IMPL__
}

void glBlitFramebuffer (GLint srcX0, GLint srcY0, 
	GLint srcX1, GLint srcY1, 
	GLint dstX0, GLint dstY0, 
	GLint dstX1, GLint dstY1, 
	GLbitfield mask, GLenum filter)
{
	__OPENGL_NOT_IMPL__
}
