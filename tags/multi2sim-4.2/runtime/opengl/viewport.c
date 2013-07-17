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
#include "viewport.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */

void glDepthRangeArrayv (GLuint first, GLsizei count, const GLclampd *v)
{
	__OPENGL_NOT_IMPL__
}

void glDepthRangeIndexed (GLuint index, GLclampd n, GLclampd f)
{
	__OPENGL_NOT_IMPL__
}

void glDepthRange( GLclampd near_val, GLclampd far_val )
{
	__OPENGL_NOT_IMPL__
}

void glDepthRangef (GLclampf n, GLclampf f)
{
	__OPENGL_NOT_IMPL__
}

void glViewportArrayv (GLuint first, GLsizei count, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glViewportIndexedf (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	__OPENGL_NOT_IMPL__
}

void glViewportIndexedfv (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	/* Debug */
	opengl_debug("API call %s(%d, %d, %d, %d)\n", 
		__FUNCTION__, x, y, width, height);

	/* Set the viewport */
	opengl_ctx->props->vp_x = x;
	opengl_ctx->props->vp_y = y;
	opengl_ctx->props->vp_w = width;
	opengl_ctx->props->vp_h = height;
}

void glClipPlane( GLenum plane, const GLdouble *equation )
{
	__OPENGL_NOT_IMPL__
}


void glGetClipPlane( GLenum plane, GLdouble *equation )
{
	__OPENGL_NOT_IMPL__
}


