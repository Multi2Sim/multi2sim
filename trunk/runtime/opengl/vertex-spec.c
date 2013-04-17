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

#include "debug.h"
#include "mhandle.h"
#include "context.h"
#include "vertex-spec.h"

/* For old OpenGL 1.x */
struct linked_list_t *vertex_repo;

/*
 * Private Functions
 */



/*
 * Public Functions
 */



/* 
 * OpenGL API functions 
 */


/* Begin and End [2.6] */

void glBegin( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}

void glEnd( void )
{
	__OPENGL_NOT_IMPL__
}

void glPatchParameteri (GLenum pname, GLint value)
{
	__OPENGL_NOT_IMPL__
}


/* Polygon Edges [2.6.2] */

void glEdgeFlag( GLboolean flag )
{
	__OPENGL_NOT_IMPL__
}

void glEdgeFlagv( const GLboolean *flag )
{
	__OPENGL_NOT_IMPL__
}


/* Vertex Specification [2.7] */

void glVertex2d( GLdouble x, GLdouble y )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2f( GLfloat x, GLfloat y )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2i( GLint x, GLint y )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2s( GLshort x, GLshort y )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3d( GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3i( GLint x, GLint y, GLint z )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3s( GLshort x, GLshort y, GLshort z )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4i( GLint x, GLint y, GLint z, GLint w )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex3sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glVertex4sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glVertexP2ui (GLenum type, GLuint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexP2uiv (GLenum type, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexP3ui (GLenum type, GLuint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexP3uiv (GLenum type, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexP4ui (GLenum type, GLuint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexP4uiv (GLenum type, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord1d( GLdouble s )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord1f( GLfloat s )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord1i( GLint s )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord1s( GLshort s )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2d( GLdouble s, GLdouble t )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2f( GLfloat s, GLfloat t )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2i( GLint s, GLint t )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2s( GLshort s, GLshort t )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3d( GLdouble s, GLdouble t, GLdouble r )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3f( GLfloat s, GLfloat t, GLfloat r )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3i( GLint s, GLint t, GLint r )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3s( GLshort s, GLshort t, GLshort r )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4i( GLint s, GLint t, GLint r, GLint q )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord1dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord1fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord1iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord1sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord2sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord3sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoord4sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP1ui (GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP1uiv (GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP2ui (GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP2uiv (GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP3ui (GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP3uiv (GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP4ui (GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordP4uiv (GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1d( GLenum target, GLdouble s )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1dv( GLenum target, const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1f( GLenum target, GLfloat s )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1fv( GLenum target, const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1i( GLenum target, GLint s )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1iv( GLenum target, const GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1s( GLenum target, GLshort s )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1sv( GLenum target, const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2d( GLenum target, GLdouble s, GLdouble t )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2dv( GLenum target, const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2f( GLenum target, GLfloat s, GLfloat t )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2fv( GLenum target, const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2i( GLenum target, GLint s, GLint t )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2iv( GLenum target, const GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2s( GLenum target, GLshort s, GLshort t )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2sv( GLenum target, const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3d( GLenum target, GLdouble s, GLdouble t, GLdouble r )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3dv( GLenum target, const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t, GLfloat r )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3fv( GLenum target, const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3i( GLenum target, GLint s, GLint t, GLint r )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3iv( GLenum target, const GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3s( GLenum target, GLshort s, GLshort t, GLshort r )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3sv( GLenum target, const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4d( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4dv( GLenum target, const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4f( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4fv( GLenum target, const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4i( GLenum target, GLint s, GLint t, GLint r, GLint q )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4iv( GLenum target, const GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4s( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4sv( GLenum target, const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP1ui (GLenum texture, GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP1uiv (GLenum texture, GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP2ui (GLenum texture, GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP2uiv (GLenum texture, GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP3ui (GLenum texture, GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP3uiv (GLenum texture, GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP4ui (GLenum texture, GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordP4uiv (GLenum texture, GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3i( GLint nx, GLint ny, GLint nz )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3s( GLshort nx, GLshort ny, GLshort nz )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3bv( const GLbyte *v )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glNormal3sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glNormalP3ui (GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormalP3uiv (GLenum type, const GLuint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordf (GLfloat coord)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordfv (const GLfloat *coord)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordd (GLdouble coord)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoorddv (const GLdouble *coord)
{
	__OPENGL_NOT_IMPL__
}
void glColor3b( GLbyte red, GLbyte green, GLbyte blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor3d( GLdouble red, GLdouble green, GLdouble blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor3f( GLfloat red, GLfloat green, GLfloat blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor3i( GLint red, GLint green, GLint blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor3s( GLshort red, GLshort green, GLshort blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor3ub( GLubyte red, GLubyte green, GLubyte blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor3ui( GLuint red, GLuint green, GLuint blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor3us( GLushort red, GLushort green, GLushort blue )
{
	__OPENGL_NOT_IMPL__
}

void glColor4b( GLbyte red, GLbyte green,
                                   GLbyte blue, GLbyte alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor4d( GLdouble red, GLdouble green,
                                   GLdouble blue, GLdouble alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor4f( GLfloat red, GLfloat green,
                                   GLfloat blue, GLfloat alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor4i( GLint red, GLint green,
                                   GLint blue, GLint alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor4s( GLshort red, GLshort green,
                                   GLshort blue, GLshort alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor4ub( GLubyte red, GLubyte green,
                                    GLubyte blue, GLubyte alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor4ui( GLuint red, GLuint green,
                                    GLuint blue, GLuint alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor4us( GLushort red, GLushort green,
                                    GLushort blue, GLushort alpha )
{
	__OPENGL_NOT_IMPL__
}

void glColor3bv( const GLbyte *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor3dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor3fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor3iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor3sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor3ubv( const GLubyte *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor3uiv( const GLuint *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor3usv( const GLushort *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4bv( const GLbyte *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4dv( const GLdouble *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4fv( const GLfloat *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4iv( const GLint *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4sv( const GLshort *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4ubv( const GLubyte *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4uiv( const GLuint *v )
{
	__OPENGL_NOT_IMPL__
}

void glColor4usv( const GLushort *v )
{
	__OPENGL_NOT_IMPL__
}

void glColorP3ui (GLenum type, GLuint color)
{
	__OPENGL_NOT_IMPL__
}

void glColorP3uiv (GLenum type, const GLuint *color)
{
	__OPENGL_NOT_IMPL__
}

void glColorP4ui (GLenum type, GLuint color)
{
	__OPENGL_NOT_IMPL__
}

void glColorP4uiv (GLenum type, const GLuint *color)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3b (GLbyte red, GLbyte green, GLbyte blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3bv (const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3d (GLdouble red, GLdouble green, GLdouble blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3dv (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3fv (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3i (GLint red, GLint green, GLint blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3iv (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3s (GLshort red, GLshort green, GLshort blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3sv (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3ub (GLubyte red, GLubyte green, GLubyte blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3ubv (const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3ui (GLuint red, GLuint green, GLuint blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3uiv (const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3us (GLushort red, GLushort green, GLushort blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3usv (const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColorP3ui (GLenum type, GLuint color)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColorP3uiv (GLenum type, const GLuint *color)
{
	__OPENGL_NOT_IMPL__
}

void glIndexd( GLdouble c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexf( GLfloat c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexi( GLint c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexs( GLshort c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexub( GLubyte c )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}

void glIndexdv( const GLdouble *c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexfv( const GLfloat *c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexiv( const GLint *c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexsv( const GLshort *c )
{
	__OPENGL_NOT_IMPL__
}

void glIndexubv( const GLubyte *c )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1d (GLuint index, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1f (GLuint index, GLfloat x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1fv (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1s (GLuint index, GLshort x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1sv (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2d (GLuint index, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2fv (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2s (GLuint index, GLshort x, GLshort y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2sv (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3d (GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3fv (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3s (GLuint index, GLshort x, GLshort y, GLshort z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3sv (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4Nbv (GLuint index, const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4Niv (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4Nsv (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4Nub (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4Nubv (GLuint index, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4Nuiv (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4Nusv (GLuint index, const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4bv (GLuint index, const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4d (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4fv (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4iv (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4s (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4sv (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4ubv (GLuint index, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4uiv (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4usv (GLuint index, const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1i (GLuint index, GLint x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2i (GLuint index, GLint x, GLint y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3i (GLuint index, GLint x, GLint y, GLint z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4i (GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1ui (GLuint index, GLuint x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2ui (GLuint index, GLuint x, GLuint y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3ui (GLuint index, GLuint x, GLuint y, GLuint z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4ui (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1iv (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2iv (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3iv (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4iv (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1uiv (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2uiv (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3uiv (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4uiv (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4bv (GLuint index, const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4sv (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4ubv (GLuint index, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4usv (GLuint index, const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1d (GLuint index, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2d (GLuint index, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3d (GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4d (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4dv (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP1ui (GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP1uiv (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP2ui (GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP2uiv (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP3ui (GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP3uiv (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP4ui (GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribP4uiv (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}
