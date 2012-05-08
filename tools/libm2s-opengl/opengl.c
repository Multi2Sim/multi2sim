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

#include <m2s-opengl.h>
#include <debug.h>
#include <list.h>
#include <misc.h>
#include <unistd.h>



/*
 * Error Messages
 */

#define __X86_OPENGL_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, err_x86_opengl_not_impl);


static char *err_x86_opengl_not_impl =
	"\tMulti2Sim provides partial support for the OpenGL library.\n"
	"\tTo request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

/*static char *err_x86_opengl_version =
	"\tYour guest application is using a version of the OpenGL runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tOpengl runtime library ('libm2s-opengl').\n";*/




/* Multi2Sim OPENGL Runtime required */
#define X86_OPENGL_RUNTIME_VERSION_MAJOR	0
#define X86_OPENGL_RUNTIME_VERSION_MINOR	669

struct x86_opengl_version_t
{
	int major;
	int minor;
};

void glClearIndex( GLfloat c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glClear( GLbitfield mask )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexMask( GLuint mask )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glAlphaFunc( GLenum func, GLclampf ref )
{
	__X86_OPENGL_NOT_IMPL__
}


void glBlendFunc( GLenum sfactor, GLenum dfactor )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLogicOp( GLenum opcode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCullFace( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFrontFace( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPointSize( GLfloat size )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLineWidth( GLfloat width )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLineStipple( GLint factor, GLushort pattern )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPolygonMode( GLenum face, GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPolygonOffset( GLfloat factor, GLfloat units )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPolygonStipple( const GLubyte *mask )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetPolygonStipple( GLubyte *mask )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEdgeFlag( GLboolean flag )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEdgeFlagv( const GLboolean *flag )
{
	__X86_OPENGL_NOT_IMPL__
}


void glScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{
	__X86_OPENGL_NOT_IMPL__
}


void glClipPlane( GLenum plane, const GLdouble *equation )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetClipPlane( GLenum plane, GLdouble *equation )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDrawBuffer( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glReadBuffer( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEnable( GLenum cap )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDisable( GLenum cap )
{
	__X86_OPENGL_NOT_IMPL__
}


GLboolean glIsEnabled( GLenum cap )
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


void glEnableClientState( GLenum cap )  /* 1.1 */
{
	__X86_OPENGL_NOT_IMPL__
}


void glDisableClientState( GLenum cap )  /* 1.1 */
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetBooleanv( GLenum pname, GLboolean *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetDoublev( GLenum pname, GLdouble *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetFloatv( GLenum pname, GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetIntegerv( GLenum pname, GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPushAttrib( GLbitfield mask )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPopAttrib( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPushClientAttrib( GLbitfield mask )  /* 1.1 */
{
	__X86_OPENGL_NOT_IMPL__
}


void glPopClientAttrib( void )  /* 1.1 */
{
	__X86_OPENGL_NOT_IMPL__
}


GLint glRenderMode( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


GLenum glGetError( void )
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


const GLubyte * glGetString( GLenum name )
{
	__X86_OPENGL_NOT_IMPL__
	return NULL;
}


void glFinish( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFlush( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glHint( GLenum target, GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glClearDepth( GLclampd depth )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDepthFunc( GLenum func )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDepthMask( GLboolean flag )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDepthRange( GLclampd near_val, GLclampd far_val )
{
	__X86_OPENGL_NOT_IMPL__
}


void glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glAccum( GLenum op, GLfloat value )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMatrixMode( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_val, GLdouble far_val )
{
	__X86_OPENGL_NOT_IMPL__
}


void glViewport( GLint x, GLint y,
                                    GLsizei width, GLsizei height )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPushMatrix( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPopMatrix( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLoadIdentity( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLoadMatrixd( const GLdouble *m )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLoadMatrixf( const GLfloat *m )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultMatrixd( const GLdouble *m )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultMatrixf( const GLfloat *m )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRotated( GLdouble angle,
                                   GLdouble x, GLdouble y, GLdouble z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRotatef( GLfloat angle,
                                   GLfloat x, GLfloat y, GLfloat z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glScaled( GLdouble x, GLdouble y, GLdouble z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glScalef( GLfloat x, GLfloat y, GLfloat z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
	__X86_OPENGL_NOT_IMPL__
}


GLboolean glIsList( GLuint list )
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


void glDeleteLists( GLuint list, GLsizei range )
{
	__X86_OPENGL_NOT_IMPL__
}


GLuint glGenLists( GLsizei range )
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


void glNewList( GLuint list, GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEndList( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCallList( GLuint list )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCallLists( GLsizei n, GLenum type,
                                     const GLvoid *lists )
{
	__X86_OPENGL_NOT_IMPL__
}


void glListBase( GLuint base )
{
	__X86_OPENGL_NOT_IMPL__
}


void glBegin( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEnd( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2d( GLdouble x, GLdouble y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2f( GLfloat x, GLfloat y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2i( GLint x, GLint y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2s( GLshort x, GLshort y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3d( GLdouble x, GLdouble y, GLdouble z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3i( GLint x, GLint y, GLint z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3s( GLshort x, GLshort y, GLshort z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4i( GLint x, GLint y, GLint z, GLint w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex2sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex3sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertex4sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3i( GLint nx, GLint ny, GLint nz )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3s( GLshort nx, GLshort ny, GLshort nz )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3bv( const GLbyte *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormal3sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexd( GLdouble c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexf( GLfloat c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexi( GLint c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexs( GLshort c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexub( GLubyte c )  /* 1.1 */
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexdv( const GLdouble *c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexfv( const GLfloat *c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexiv( const GLint *c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexsv( const GLshort *c )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexubv( const GLubyte *c )  /* 1.1 */
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3b( GLbyte red, GLbyte green, GLbyte blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3d( GLdouble red, GLdouble green, GLdouble blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3f( GLfloat red, GLfloat green, GLfloat blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3i( GLint red, GLint green, GLint blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3s( GLshort red, GLshort green, GLshort blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3ub( GLubyte red, GLubyte green, GLubyte blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3ui( GLuint red, GLuint green, GLuint blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3us( GLushort red, GLushort green, GLushort blue )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4b( GLbyte red, GLbyte green,
                                   GLbyte blue, GLbyte alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4d( GLdouble red, GLdouble green,
                                   GLdouble blue, GLdouble alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4f( GLfloat red, GLfloat green,
                                   GLfloat blue, GLfloat alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4i( GLint red, GLint green,
                                   GLint blue, GLint alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4s( GLshort red, GLshort green,
                                   GLshort blue, GLshort alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4ub( GLubyte red, GLubyte green,
                                    GLubyte blue, GLubyte alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4ui( GLuint red, GLuint green,
                                    GLuint blue, GLuint alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4us( GLushort red, GLushort green,
                                    GLushort blue, GLushort alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3bv( const GLbyte *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3ubv( const GLubyte *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3uiv( const GLuint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor3usv( const GLushort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4bv( const GLbyte *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4ubv( const GLubyte *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4uiv( const GLuint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColor4usv( const GLushort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1d( GLdouble s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1f( GLfloat s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1i( GLint s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1s( GLshort s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2d( GLdouble s, GLdouble t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2f( GLfloat s, GLfloat t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2i( GLint s, GLint t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2s( GLshort s, GLshort t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3d( GLdouble s, GLdouble t, GLdouble r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3f( GLfloat s, GLfloat t, GLfloat r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3i( GLint s, GLint t, GLint r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3s( GLshort s, GLshort t, GLshort r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4i( GLint s, GLint t, GLint r, GLint q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord1sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord2sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord3sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoord4sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2d( GLdouble x, GLdouble y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2f( GLfloat x, GLfloat y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2i( GLint x, GLint y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2s( GLshort x, GLshort y )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3d( GLdouble x, GLdouble y, GLdouble z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3f( GLfloat x, GLfloat y, GLfloat z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3i( GLint x, GLint y, GLint z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3s( GLshort x, GLshort y, GLshort z )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4i( GLint x, GLint y, GLint z, GLint w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos2sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos3sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4dv( const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4fv( const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4iv( const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRasterPos4sv( const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRecti( GLint x1, GLint y1, GLint x2, GLint y2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRectdv( const GLdouble *v1, const GLdouble *v2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRectfv( const GLfloat *v1, const GLfloat *v2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRectiv( const GLint *v1, const GLint *v2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glRectsv( const GLshort *v1, const GLshort *v2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glVertexPointer( GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr )
{
	__X86_OPENGL_NOT_IMPL__
}


void glNormalPointer( GLenum type, GLsizei stride,
                                       const GLvoid *ptr )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr )
{
	__X86_OPENGL_NOT_IMPL__
}


void glIndexPointer( GLenum type, GLsizei stride,
                                      const GLvoid *ptr )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexCoordPointer( GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetPointerv( GLenum pname, GLvoid **params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glArrayElement( GLint i )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDrawElements( GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices )
{
	__X86_OPENGL_NOT_IMPL__
}


void glInterleavedArrays( GLenum format, GLsizei stride,
                                           const GLvoid *pointer )
{
	__X86_OPENGL_NOT_IMPL__
}


void glShadeModel( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLightf( GLenum light, GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLighti( GLenum light, GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLightfv( GLenum light, GLenum pname,
                                 const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLightiv( GLenum light, GLenum pname,
                                 const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetLightfv( GLenum light, GLenum pname,
                                    GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetLightiv( GLenum light, GLenum pname,
                                    GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLightModelf( GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLightModeli( GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLightModelfv( GLenum pname, const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLightModeliv( GLenum pname, const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMaterialf( GLenum face, GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMateriali( GLenum face, GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMaterialiv( GLenum face, GLenum pname, const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColorMaterial( GLenum face, GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelZoom( GLfloat xfactor, GLfloat yfactor )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelStoref( GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelStorei( GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelTransferf( GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelTransferi( GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelMapfv( GLenum map, GLsizei mapsize,
                                    const GLfloat *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelMapuiv( GLenum map, GLsizei mapsize,
                                     const GLuint *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPixelMapusv( GLenum map, GLsizei mapsize,
                                     const GLushort *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetPixelMapfv( GLenum map, GLfloat *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetPixelMapuiv( GLenum map, GLuint *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetPixelMapusv( GLenum map, GLushort *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glBitmap( GLsizei width, GLsizei height,
                                GLfloat xorig, GLfloat yorig,
                                GLfloat xmove, GLfloat ymove,
                                const GLubyte *bitmap )
{
	__X86_OPENGL_NOT_IMPL__
}


void glReadPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDrawPixels( GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum type )
{
	__X86_OPENGL_NOT_IMPL__
}


void glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
	__X86_OPENGL_NOT_IMPL__
}


void glStencilMask( GLuint mask )
{
	__X86_OPENGL_NOT_IMPL__
}


void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
	__X86_OPENGL_NOT_IMPL__
}


void glClearStencil( GLint s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexGend( GLenum coord, GLenum pname, GLdouble param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexGenf( GLenum coord, GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexGeni( GLenum coord, GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexEnvi( GLenum target, GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexEnviv( GLenum target, GLenum pname, const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexParameteri( GLenum target, GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexParameterfv( GLenum target, GLenum pname,
                                          const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexParameteriv( GLenum target, GLenum pname,
                                          const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexParameterfv( GLenum target,
                                           GLenum pname, GLfloat *params)
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexParameteriv( GLenum target,
                                           GLenum pname, GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexLevelParameterfv( GLenum target, GLint level,
                                                GLenum pname, GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexLevelParameteriv( GLenum target, GLint level,
                                                GLenum pname, GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexImage1D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLint border,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetTexImage( GLenum target, GLint level,
                                     GLenum format, GLenum type,
                                     GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGenTextures( GLsizei n, GLuint *textures )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDeleteTextures( GLsizei n, const GLuint *textures)
{
	__X86_OPENGL_NOT_IMPL__
}


void glBindTexture( GLenum target, GLuint texture )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPrioritizeTextures( GLsizei n,
                                            const GLuint *textures,
                                            const GLclampf *priorities )
{
	__X86_OPENGL_NOT_IMPL__
}


GLboolean glAreTexturesResident( GLsizei n,
                                                  const GLuint *textures,
                                                  GLboolean *residences )
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


GLboolean glIsTexture( GLuint texture )
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


void glTexSubImage1D( GLenum target, GLint level,
                                       GLint xoffset,
                                       GLsizei width, GLenum format,
                                       GLenum type, const GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexSubImage2D( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyTexImage1D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLint border )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyTexImage2D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyTexSubImage1D( GLenum target, GLint level,
                                           GLint xoffset, GLint x, GLint y,
                                           GLsizei width )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyTexSubImage2D( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMap1d( GLenum target, GLdouble u1, GLdouble u2,
                               GLint stride,
                               GLint order, const GLdouble *points )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMap1f( GLenum target, GLfloat u1, GLfloat u2,
                               GLint stride,
                               GLint order, const GLfloat *points )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMap2d( GLenum target,
		     GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
		     GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
		     const GLdouble *points )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMap2f( GLenum target,
		     GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		     GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
		     const GLfloat *points )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMapdv( GLenum target, GLenum query, GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMapfv( GLenum target, GLenum query, GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMapiv( GLenum target, GLenum query, GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord1d( GLdouble u )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord1f( GLfloat u )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord1dv( const GLdouble *u )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord1fv( const GLfloat *u )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord2d( GLdouble u, GLdouble v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord2f( GLfloat u, GLfloat v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord2dv( const GLdouble *u )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalCoord2fv( const GLfloat *u )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMapGrid2d( GLint un, GLdouble u1, GLdouble u2,
                                   GLint vn, GLdouble v1, GLdouble v2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMapGrid2f( GLint un, GLfloat u1, GLfloat u2,
                                   GLint vn, GLfloat v1, GLfloat v2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalPoint1( GLint i )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalPoint2( GLint i, GLint j )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalMesh1( GLenum mode, GLint i1, GLint i2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFogf( GLenum pname, GLfloat param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFogi( GLenum pname, GLint param )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFogfv( GLenum pname, const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFogiv( GLenum pname, const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPassThrough( GLfloat token )
{
	__X86_OPENGL_NOT_IMPL__
}


void glSelectBuffer( GLsizei size, GLuint *buffer )
{
	__X86_OPENGL_NOT_IMPL__
}


void glInitNames( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLoadName( GLuint name )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPushName( GLuint name )
{
	__X86_OPENGL_NOT_IMPL__
}


void glPopName( void )
{
	__X86_OPENGL_NOT_IMPL__
}


void glDrawRangeElements( GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexImage3D( GLenum target, GLint level,
                                      GLint internalFormat,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLint border,
                                      GLenum format, GLenum type,
                                      const GLvoid *pixels )
{
	__X86_OPENGL_NOT_IMPL__
}


void glTexSubImage3D( GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels)
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyTexSubImage3D( GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLint x,
                                             GLint y, GLsizei width,
                                             GLsizei height )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColorTable( GLenum target, GLenum internalformat,
                                    GLsizei width, GLenum format,
                                    GLenum type, const GLvoid *table )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColorSubTable( GLenum target,
                                       GLsizei start, GLsizei count,
                                       GLenum format, GLenum type,
                                       const GLvoid *data )
{
	__X86_OPENGL_NOT_IMPL__
}


void glColorTableParameteriv(GLenum target, GLenum pname,
                                              const GLint *params)
{
	__X86_OPENGL_NOT_IMPL__
}


void glColorTableParameterfv(GLenum target, GLenum pname,
                                              const GLfloat *params)
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyColorSubTable( GLenum target, GLsizei start,
                                           GLint x, GLint y, GLsizei width )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyColorTable( GLenum target, GLenum internalformat,
                                        GLint x, GLint y, GLsizei width )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetColorTable( GLenum target, GLenum format,
                                       GLenum type, GLvoid *table )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetColorTableParameterfv( GLenum target, GLenum pname,
                                                  GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetColorTableParameteriv( GLenum target, GLenum pname,
                                                  GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glBlendEquation( GLenum mode )
{
	__X86_OPENGL_NOT_IMPL__
}


void glBlendColor( GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha )
{
	__X86_OPENGL_NOT_IMPL__
}


void glHistogram( GLenum target, GLsizei width,
				   GLenum internalformat, GLboolean sink )
{
	__X86_OPENGL_NOT_IMPL__
}


void glResetHistogram( GLenum target )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetHistogram( GLenum target, GLboolean reset,
				      GLenum format, GLenum type,
				      GLvoid *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetHistogramParameterfv( GLenum target, GLenum pname,
						 GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetHistogramParameteriv( GLenum target, GLenum pname,
						 GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMinmax( GLenum target, GLenum internalformat,
				GLboolean sink )
{
	__X86_OPENGL_NOT_IMPL__
}


void glResetMinmax( GLenum target )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMinmax( GLenum target, GLboolean reset,
                                   GLenum format, GLenum types,
                                   GLvoid *values )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMinmaxParameterfv( GLenum target, GLenum pname,
					      GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetMinmaxParameteriv( GLenum target, GLenum pname,
					      GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image )
{
	__X86_OPENGL_NOT_IMPL__
}


void glConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image )
{
	__X86_OPENGL_NOT_IMPL__
}


void glConvolutionParameterf( GLenum target, GLenum pname,
	GLfloat params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glConvolutionParameterfv( GLenum target, GLenum pname,
	const GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glConvolutionParameteri( GLenum target, GLenum pname,
	GLint params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glConvolutionParameteriv( GLenum target, GLenum pname,
	const GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCopyConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height)
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetConvolutionFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *image )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetConvolutionParameterfv( GLenum target, GLenum pname,
	GLfloat *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetConvolutionParameteriv( GLenum target, GLenum pname,
	GLint *params )
{
	__X86_OPENGL_NOT_IMPL__
}


void glSeparableFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetSeparableFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span )
{
	__X86_OPENGL_NOT_IMPL__
}


void glActiveTexture( GLenum texture )
{
	__X86_OPENGL_NOT_IMPL__
}


void glClientActiveTexture( GLenum texture )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCompressedTexImage1D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__X86_OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetCompressedTexImage( GLenum target, GLint lod, GLvoid *img )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1d( GLenum target, GLdouble s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1dv( GLenum target, const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1f( GLenum target, GLfloat s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1fv( GLenum target, const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1i( GLenum target, GLint s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1iv( GLenum target, const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1s( GLenum target, GLshort s )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1sv( GLenum target, const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2d( GLenum target, GLdouble s, GLdouble t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2dv( GLenum target, const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2f( GLenum target, GLfloat s, GLfloat t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2fv( GLenum target, const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2i( GLenum target, GLint s, GLint t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2iv( GLenum target, const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2s( GLenum target, GLshort s, GLshort t )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2sv( GLenum target, const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3d( GLenum target, GLdouble s, GLdouble t, GLdouble r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3dv( GLenum target, const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t, GLfloat r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3fv( GLenum target, const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3i( GLenum target, GLint s, GLint t, GLint r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3iv( GLenum target, const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3s( GLenum target, GLshort s, GLshort t, GLshort r )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3sv( GLenum target, const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4d( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4dv( GLenum target, const GLdouble *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4f( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4fv( GLenum target, const GLfloat *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4i( GLenum target, GLint s, GLint t, GLint r, GLint q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4iv( GLenum target, const GLint *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4s( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4sv( GLenum target, const GLshort *v )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLoadTransposeMatrixd( const GLdouble m[16] )
{
	__X86_OPENGL_NOT_IMPL__
}


void glLoadTransposeMatrixf( const GLfloat m[16] )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultTransposeMatrixd( const GLdouble m[16] )
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultTransposeMatrixf( const GLfloat m[16] )
{
	__X86_OPENGL_NOT_IMPL__
}


void glSampleCoverage( GLclampf value, GLboolean invert )
{
	__X86_OPENGL_NOT_IMPL__
}


void glActiveTextureARB(GLenum texture)
{
	__X86_OPENGL_NOT_IMPL__
}


void glClientActiveTextureARB(GLenum texture)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1dARB(GLenum target, GLdouble s)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1dvARB(GLenum target, const GLdouble *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1fARB(GLenum target, GLfloat s)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1fvARB(GLenum target, const GLfloat *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1iARB(GLenum target, GLint s)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1ivARB(GLenum target, const GLint *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1sARB(GLenum target, GLshort s)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord1svARB(GLenum target, const GLshort *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2dvARB(GLenum target, const GLdouble *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2fvARB(GLenum target, const GLfloat *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2iARB(GLenum target, GLint s, GLint t)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2ivARB(GLenum target, const GLint *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord2svARB(GLenum target, const GLshort *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3dvARB(GLenum target, const GLdouble *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3fvARB(GLenum target, const GLfloat *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3ivARB(GLenum target, const GLint *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord3svARB(GLenum target, const GLshort *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4dvARB(GLenum target, const GLdouble *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4fvARB(GLenum target, const GLfloat *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4ivARB(GLenum target, const GLint *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
	__X86_OPENGL_NOT_IMPL__
}


void glMultiTexCoord4svARB(GLenum target, const GLshort *v)
{
	__X86_OPENGL_NOT_IMPL__
}


GLhandleARB glCreateDebugObjectMESA (void)
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


void glClearDebugLogMESA (GLhandleARB obj, GLenum logType, GLenum shaderType)
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetDebugLogMESA (GLhandleARB obj, GLenum logType, GLenum shaderType, GLsizei maxLength,
                                         GLsizei *length, GLcharARB *debugLog)
{
	__X86_OPENGL_NOT_IMPL__
}


GLsizei glGetDebugLogLengthMESA (GLhandleARB obj, GLenum logType, GLenum shaderType)
{
	__X86_OPENGL_NOT_IMPL__
	return 0;
}


void glProgramCallbackMESA(GLenum target, GLprogramcallbackMESA callback, GLvoid *data)
{
	__X86_OPENGL_NOT_IMPL__
}


void glGetProgramRegisterfvMESA(GLenum target, GLsizei len, const GLubyte *name, GLfloat *v)
{
	__X86_OPENGL_NOT_IMPL__
}


void *glFramebufferTextureLayerEXT(GLenum target,
    GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__X86_OPENGL_NOT_IMPL__
	return NULL;
}


void glBlendEquationSeparateATI( GLenum modeRGB, GLenum modeA )
{
	__X86_OPENGL_NOT_IMPL__
}


void *glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image)
{
	__X86_OPENGL_NOT_IMPL__
	return NULL;
}


void *glEGLImageTargetRenderbufferStorageOES (GLenum target, GLeglImageOES image)
{
	__X86_OPENGL_NOT_IMPL__
	return NULL;
}






