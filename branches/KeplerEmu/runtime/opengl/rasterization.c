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
#include "rasterization.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */


/* Multisampleing [3.3.1] */

void glGetMultisamplefv (GLenum pname, GLuint index, GLfloat *val)
{
	__OPENGL_NOT_IMPL__
}

void glMinSampleShading (GLclampf value)
{
	__OPENGL_NOT_IMPL__
}


/* Points */

void glPointSize( GLfloat size )
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterf (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterfv (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameteri (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameteriv (GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}


/* Line segment [3.5] */

void glLineWidth( GLfloat width )
{
	__OPENGL_NOT_IMPL__
}


void glLineStipple( GLint factor, GLushort pattern )
{
	__OPENGL_NOT_IMPL__
}


/* Polygons [3.6] */

void glCullFace( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glFrontFace( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


/* Stippling [3.6.2] [6.1.6] */

void glPolygonStipple( const GLubyte *mask )
{
	__OPENGL_NOT_IMPL__
}


void glGetPolygonStipple( GLubyte *mask )
{
	__OPENGL_NOT_IMPL__
}


/* Polygon rasterization & depth offset */

void glPolygonMode( GLenum face, GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glPolygonOffset( GLfloat factor, GLfloat units )
{
	__OPENGL_NOT_IMPL__
}


/* Pixel storage modes [3.7.1] */

void glPixelStoref( GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glPixelStorei( GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


/* Pixel Transfer Modes [3.7.3] [6.1.3] */

void glPixelTransferf( GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glPixelTransferi( GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glPixelMapfv( GLenum map, GLsizei mapsize,
	const GLfloat *values )
{
	__OPENGL_NOT_IMPL__
}


void glPixelMapuiv( GLenum map, GLsizei mapsize,
	const GLuint *values )
{
	__OPENGL_NOT_IMPL__
}


void glPixelMapusv( GLenum map, GLsizei mapsize,
	const GLushort *values )
{
	__OPENGL_NOT_IMPL__
}


void glGetPixelMapfv( GLenum map, GLfloat *values )
{
	__OPENGL_NOT_IMPL__
}


void glGetPixelMapuiv( GLenum map, GLuint *values )
{
	__OPENGL_NOT_IMPL__
}


void glGetPixelMapusv( GLenum map, GLushort *values )
{
	__OPENGL_NOT_IMPL__
}


/* Color table specification */

void glColorTable( GLenum target, GLenum internalformat,
	GLsizei width, GLenum format,
	GLenum type, const GLvoid *table )
{
	__OPENGL_NOT_IMPL__
}

void glColorTableParameteriv(GLenum target, GLenum pname,
	const GLint *params)
{
	__OPENGL_NOT_IMPL__
}


void glColorTableParameterfv(GLenum target, GLenum pname,
	const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glCopyColorTable( GLenum target, GLenum internalformat,
	GLint x, GLint y, GLsizei width )
{
	__OPENGL_NOT_IMPL__
}

void glColorSubTable( GLenum target,
	GLsizei start, GLsizei count,
	GLenum format, GLenum type,
	const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}

void glCopyColorSubTable( GLenum target, GLsizei start,
	GLint x, GLint y, GLsizei width )
{
	__OPENGL_NOT_IMPL__
}


/* Color table query [6.1.8] */

void glGetColorTable( GLenum target, GLenum format,
	GLenum type, GLvoid *table )
{
	__OPENGL_NOT_IMPL__
}


void glGetColorTableParameterfv( GLenum target, GLenum pname,
	GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetColorTableParameteriv( GLenum target, GLenum pname,
	GLint *params )
{
	__OPENGL_NOT_IMPL__
}


/* Convolution filter specification [3.7.3] */

void glConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image )
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image )
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionParameterfv( GLenum target, GLenum pname,
	const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionParameteriv( GLenum target, GLenum pname,
	const GLint *params )
{
	__OPENGL_NOT_IMPL__
}

void glSeparableFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column )
{
	__OPENGL_NOT_IMPL__
}

void glCopyConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width )
{
	__OPENGL_NOT_IMPL__
}


void glCopyConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height)
{
	__OPENGL_NOT_IMPL__
}


void glGetConvolutionFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *image )
{
	__OPENGL_NOT_IMPL__
}

void glGetSeparableFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span )
{
	__OPENGL_NOT_IMPL__
}

void glGetConvolutionParameterfv( GLenum target, GLenum pname,
	GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetConvolutionParameteriv( GLenum target, GLenum pname,
	GLint *params )
{
	__OPENGL_NOT_IMPL__
}


/* Histogram table specification [3.7.3] */

void glHistogram( GLenum target, GLsizei width,
	GLenum internalformat, GLboolean sink )
{
	__OPENGL_NOT_IMPL__
}

void glGetHistogram( GLenum target, GLboolean reset,
	GLenum format, GLenum type,
	GLvoid *values )
{
	__OPENGL_NOT_IMPL__
}

void glResetHistogram( GLenum target )
{
	__OPENGL_NOT_IMPL__
}

void glGetHistogramParameterfv( GLenum target, GLenum pname,
	GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetHistogramParameteriv( GLenum target, GLenum pname,
	GLint *params )
{
	__OPENGL_NOT_IMPL__
}


/* Min/Max table specification */

void glMinmax( GLenum target, GLenum internalformat,
	GLboolean sink )
{
	__OPENGL_NOT_IMPL__
}

void glResetMinmax( GLenum target )
{
	__OPENGL_NOT_IMPL__
}


void glGetMinmax( GLenum target, GLboolean reset,
	GLenum format, GLenum types,
	GLvoid *values )
{
	__OPENGL_NOT_IMPL__
}


void glGetMinmaxParameterfv( GLenum target, GLenum pname,
	GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetMinmaxParameteriv( GLenum target, GLenum pname,
	GLint *params )
{
	__OPENGL_NOT_IMPL__
}


/* Rasterization of pixel rectangles [4.3.1] [3.7.5] */

void glDrawPixels( GLsizei width, GLsizei height,
	GLenum format, GLenum type,
	const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}

void glClampColor (GLenum target, GLenum clamp)
{
	__OPENGL_NOT_IMPL__
}

void glPixelZoom( GLfloat xfactor, GLfloat yfactor )
{
	__OPENGL_NOT_IMPL__
}


/* Pixel transfer operations [3.7.6] */


void glConvolutionParameteri( GLenum target, GLenum pname,
	GLint params )
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionParameterf( GLenum target, GLenum pname,
	GLfloat params )
{
	__OPENGL_NOT_IMPL__
}


/* Bitmap [3.8] */

void glBitmap( GLsizei width, GLsizei height,
	GLfloat xorig, GLfloat yorig,
	GLfloat xmove, GLfloat ymove,
	const GLubyte *bitmap )
{
	__OPENGL_NOT_IMPL__
}

