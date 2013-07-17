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
#include "fragment.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */


void glScissorArrayv (GLuint first, GLsizei count, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glScissorIndexed (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glScissorIndexedv (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glSampleCoverage( GLclampf value, GLboolean invert )
{
	__OPENGL_NOT_IMPL__
}

void glSampleMaski (GLuint index, GLbitfield mask)
{
	__OPENGL_NOT_IMPL__
}

void glAlphaFunc( GLenum func, GLclampf ref )
{
	__OPENGL_NOT_IMPL__
}

void glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
	__OPENGL_NOT_IMPL__
}

void glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask)
{
	__OPENGL_NOT_IMPL__
}

void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
	__OPENGL_NOT_IMPL__
}

void glStencilOpSeparate (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	__OPENGL_NOT_IMPL__
}

void glDepthFunc( GLenum func )
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquation( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationi (GLuint buf, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationSeparatei (GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFunc( GLenum sfactor, GLenum dfactor )
{
	__OPENGL_NOT_IMPL__
}

void glBlendFunci (GLuint buf, GLenum src, GLenum dst)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncSeparatei (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendColor( GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha )
{
	__OPENGL_NOT_IMPL__
}

void glLogicOp( GLenum opcode )
{
	__OPENGL_NOT_IMPL__
}
