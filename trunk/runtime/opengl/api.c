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

#include <stdio.h>
#include <unistd.h>

#include "../include/GL/gl.h"
#include "api.h"
#include "debug.h"



/*
 * Error Messages
 */

#define __OPENGL_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, opengl_err_not_impl);


static char *opengl_err_not_impl =
	"\tMulti2Sim provides partial support for the OpenGL library.\n"
	"\tTo request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

/*static char *opengl_err_version =
	"\tYour guest application is using a version of the OpenGL runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tOpengl runtime library ('libm2s-opengl').\n";*/




/* Multi2Sim OPENGL Runtime required */
#define OPENGL_RUNTIME_VERSION_MAJOR	0
#define OPENGL_RUNTIME_VERSION_MINOR	669

struct opengl_version_t
{
	int major;
	int minor;
};

void glClearIndex( GLfloat c )
{
	__OPENGL_NOT_IMPL__
}


void glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	printf("glClearColor\n");
	GLdouble sys_args[4];
	sys_args[0] = (GLdouble) red;
	sys_args[1] = (GLdouble) green;
	sys_args[2] = (GLdouble) blue;
	sys_args[3] = (GLdouble) alpha;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glClearColor, &sys_args);	
}


void glClear( GLbitfield mask )
{
	printf("glClear\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glClear, &mask);
}


void glIndexMask( GLuint mask )
{
	__OPENGL_NOT_IMPL__
}


void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
	__OPENGL_NOT_IMPL__
}


void glAlphaFunc( GLenum func, GLclampf ref )
{
	__OPENGL_NOT_IMPL__
}


void glBlendFunc( GLenum sfactor, GLenum dfactor )
{
	__OPENGL_NOT_IMPL__
}


void glLogicOp( GLenum opcode )
{
	__OPENGL_NOT_IMPL__
}


void glCullFace( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glFrontFace( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glPointSize( GLfloat size )
{
	__OPENGL_NOT_IMPL__
}


void glLineWidth( GLfloat width )
{
	__OPENGL_NOT_IMPL__
}


void glLineStipple( GLint factor, GLushort pattern )
{
	__OPENGL_NOT_IMPL__
}


void glPolygonMode( GLenum face, GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glPolygonOffset( GLfloat factor, GLfloat units )
{
	__OPENGL_NOT_IMPL__
}


void glPolygonStipple( const GLubyte *mask )
{
	__OPENGL_NOT_IMPL__
}


void glGetPolygonStipple( GLubyte *mask )
{
	__OPENGL_NOT_IMPL__
}


void glEdgeFlag( GLboolean flag )
{
	__OPENGL_NOT_IMPL__
}


void glEdgeFlagv( const GLboolean *flag )
{
	__OPENGL_NOT_IMPL__
}


void glScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}


void glClipPlane( GLenum plane, const GLdouble *equation )
{
	__OPENGL_NOT_IMPL__
}


void glGetClipPlane( GLenum plane, GLdouble *equation )
{
	__OPENGL_NOT_IMPL__
}


void glDrawBuffer( GLenum mode )
{
	printf("glDrawBuffer\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glDrawBuffer, &mode);
}


void glReadBuffer( GLenum mode )
{
	printf("glReadBuffer\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glReadBuffer, &mode);
}


void glEnable( GLenum cap )
{
	printf("glEnable\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glEnable, &cap);
}


void glDisable( GLenum cap )
{
	__OPENGL_NOT_IMPL__
}


GLboolean glIsEnabled( GLenum cap )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


void glEnableClientState( GLenum cap )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}


void glDisableClientState( GLenum cap )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}


void glGetBooleanv( GLenum pname, GLboolean *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetDoublev( GLenum pname, GLdouble *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetFloatv( GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetIntegerv( GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glPushAttrib( GLbitfield mask )
{
	__OPENGL_NOT_IMPL__
}


void glPopAttrib( void )
{
	__OPENGL_NOT_IMPL__
}


void glPushClientAttrib( GLbitfield mask )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}


void glPopClientAttrib( void )  /* 1.1 */
{
	__OPENGL_NOT_IMPL__
}


GLint glRenderMode( GLenum mode )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


GLenum glGetError( void )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


const GLubyte * glGetString( GLenum name )
{
	__OPENGL_NOT_IMPL__
	return NULL;
}


void glFinish( void )
{
	__OPENGL_NOT_IMPL__
}


void glFlush( void )
{
	printf("glFlush\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glFlush);
}


void glHint( GLenum target, GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glClearDepth( GLclampd depth )
{
	__OPENGL_NOT_IMPL__
}


void glDepthFunc( GLenum func )
{
	__OPENGL_NOT_IMPL__
}


void glDepthMask( GLboolean flag )
{
	__OPENGL_NOT_IMPL__
}


void glDepthRange( GLclampd near_val, GLclampd far_val )
{
	__OPENGL_NOT_IMPL__
}


void glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	__OPENGL_NOT_IMPL__
}


void glAccum( GLenum op, GLfloat value )
{
	__OPENGL_NOT_IMPL__
}


void glMatrixMode( GLenum mode )
{
	printf("glMatrixMode\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glMatrixMode, &mode);
}


void glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val )
{
	printf("glOrtho\n");
	GLdouble sys_args[6];
	sys_args[0] = (GLdouble) left;
	sys_args[1] = (GLdouble) right;
	sys_args[2] = (GLdouble) bottom;
	sys_args[3] = (GLdouble) top;
	sys_args[4] = (GLdouble) near_val;
	sys_args[5] = (GLdouble) far_val;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glOrtho, &sys_args);
}


void glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_val, GLdouble far_val )
{
	printf("glFrustum\n");
	GLdouble sys_args[6];
	sys_args[0] = (GLdouble) left;
	sys_args[1] = (GLdouble) right;
	sys_args[2] = (GLdouble) bottom;
	sys_args[3] = (GLdouble) top;
	sys_args[4] = (GLdouble) near_val;
	sys_args[5] = (GLdouble) far_val;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glFrustum, &sys_args);
}


void glViewport( GLint x, GLint y,
                                    GLsizei width, GLsizei height )
{
	printf("glViewport\n");
	GLint sys_args[4];
	sys_args[0] = (GLint) x;
	sys_args[1] = (GLint) y;
	sys_args[2] = (GLint) width;
	sys_args[3] = (GLint) height;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glViewport, &sys_args);
}


void glPushMatrix( void )
{
	printf("glPushMatrix\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glPushMatrix);
}


void glPopMatrix( void )
{
	printf("glPopMatrix\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glPopMatrix);

}


void glLoadIdentity( void )
{
	printf("glLoadIdentity\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glLoadIdentity);
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


void glRotated( GLdouble angle,
                                   GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glRotatef( GLfloat angle,
                                   GLfloat x, GLfloat y, GLfloat z )
{
	printf("glRotatef\n");
	GLfloat sys_args[4];
	sys_args[0] = (GLfloat) angle;
	sys_args[1] = (GLfloat) x;
	sys_args[2] = (GLfloat) y;
	sys_args[3] = (GLfloat) z;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glRotatef, &sys_args);

}


void glScaled( GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glScalef( GLfloat x, GLfloat y, GLfloat z )
{
	__OPENGL_NOT_IMPL__
}


void glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
	__OPENGL_NOT_IMPL__
}


void glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
	printf("glTranslatef\n");
	GLfloat sys_args[3];
	sys_args[0] = (GLfloat) x;
	sys_args[1] = (GLfloat) y;
	sys_args[2] = (GLfloat) z;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glTranslatef, &sys_args);	

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
	printf("glGenLists\n");
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) range;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glGenLists, &sys_args);	
	return 0;
}


void glNewList( GLuint list, GLenum mode )
{
	printf("glNewList\n");
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) list;
	sys_args[1] = (unsigned int) mode;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glNewList, &sys_args);	
}


void glEndList( void )
{
	printf("glEndList\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glEndList);		
}


void glCallList( GLuint list )
{
	printf("glCallList\n");
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) list;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glCallList, &sys_args);
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


void glBegin( GLenum mode )
{
	printf("glBegin\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glBegin, &mode);
}


void glEnd( void )
{
	printf("glEnd\n");
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glEnd);
}


void glVertex2d( GLdouble x, GLdouble y )
{
	__OPENGL_NOT_IMPL__
}


void glVertex2f( GLfloat x, GLfloat y )
{
	printf("glVertex2f\n");
	GLfloat sys_args[2];
	sys_args[0] = (GLfloat) x;
	sys_args[1] = (GLfloat) y;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glVertex2f, &sys_args);	
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
	printf("glVertex3f\n");
	GLfloat sys_args[3];
	sys_args[0] = (GLfloat) x;
	sys_args[1] = (GLfloat) y;
	sys_args[2] = (GLfloat) z;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glVertex3f, &sys_args);	
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
	printf("glNormal3f\n");
	GLfloat sys_args[3];
	sys_args[0] = (GLfloat) nx;
	sys_args[1] = (GLfloat) ny;
	sys_args[2] = (GLfloat) nz;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glNormal3f, &sys_args);	

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
	printf("glColor3f\n");
	GLfloat sys_args[3];
	sys_args[0] = (GLfloat) red;
	sys_args[1] = (GLfloat) green;
	sys_args[2] = (GLfloat) blue;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glColor3f, &sys_args);	
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


void glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
	__OPENGL_NOT_IMPL__
}


void glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
	__OPENGL_NOT_IMPL__
}


void glRecti( GLint x1, GLint y1, GLint x2, GLint y2 )
{
	__OPENGL_NOT_IMPL__
}


void glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
	__OPENGL_NOT_IMPL__
}


void glRectdv( const GLdouble *v1, const GLdouble *v2 )
{
	__OPENGL_NOT_IMPL__
}


void glRectfv( const GLfloat *v1, const GLfloat *v2 )
{
	__OPENGL_NOT_IMPL__
}


void glRectiv( const GLint *v1, const GLint *v2 )
{
	__OPENGL_NOT_IMPL__
}


void glRectsv( const GLshort *v1, const GLshort *v2 )
{
	__OPENGL_NOT_IMPL__
}


void glVertexPointer( GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr )
{
	__OPENGL_NOT_IMPL__
}


void glNormalPointer( GLenum type, GLsizei stride,
                                       const GLvoid *ptr )
{
	__OPENGL_NOT_IMPL__
}


void glColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr )
{
	__OPENGL_NOT_IMPL__
}


void glIndexPointer( GLenum type, GLsizei stride,
                                      const GLvoid *ptr )
{
	__OPENGL_NOT_IMPL__
}


void glTexCoordPointer( GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr )
{
	__OPENGL_NOT_IMPL__
}


void glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr )
{
	__OPENGL_NOT_IMPL__
}


void glGetPointerv( GLenum pname, GLvoid **params )
{
	__OPENGL_NOT_IMPL__
}


void glArrayElement( GLint i )
{
	__OPENGL_NOT_IMPL__
}


void glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
	__OPENGL_NOT_IMPL__
}


void glDrawElements( GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices )
{
	__OPENGL_NOT_IMPL__
}


void glInterleavedArrays( GLenum format, GLsizei stride,
                                           const GLvoid *pointer )
{
	__OPENGL_NOT_IMPL__
}


void glShadeModel( GLenum mode )
{
	printf("glShadeModel\n");
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glShadeModel, &sys_args);

}


void glLightf( GLenum light, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glLighti( GLenum light, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glLightfv( GLenum light, GLenum pname,
                                 const GLfloat *params )
{
	printf("glLightfv\n");
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) light;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glLightfv, &sys_args);
}


void glLightiv( GLenum light, GLenum pname,
                                 const GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetLightfv( GLenum light, GLenum pname,
                                    GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetLightiv( GLenum light, GLenum pname,
                                    GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glLightModelf( GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glLightModeli( GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glLightModelfv( GLenum pname, const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glLightModeliv( GLenum pname, const GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glMaterialf( GLenum face, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glMateriali( GLenum face, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
	printf("glMaterialfv\n");
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(OPENGL_SYSCALL_CODE, opengl_call_glMaterialfv, &sys_args);
}


void glMaterialiv( GLenum face, GLenum pname, const GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glColorMaterial( GLenum face, GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glPixelZoom( GLfloat xfactor, GLfloat yfactor )
{
	__OPENGL_NOT_IMPL__
}


void glPixelStoref( GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glPixelStorei( GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


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


void glBitmap( GLsizei width, GLsizei height,
                                GLfloat xorig, GLfloat yorig,
                                GLfloat xmove, GLfloat ymove,
                                const GLubyte *bitmap )
{
	__OPENGL_NOT_IMPL__
}


void glReadPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glDrawPixels( GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glCopyPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum type )
{
	__OPENGL_NOT_IMPL__
}


void glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
	__OPENGL_NOT_IMPL__
}


void glStencilMask( GLuint mask )
{
	__OPENGL_NOT_IMPL__
}


void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
	__OPENGL_NOT_IMPL__
}


void glClearStencil( GLint s )
{
	__OPENGL_NOT_IMPL__
}


void glTexGend( GLenum coord, GLenum pname, GLdouble param )
{
	__OPENGL_NOT_IMPL__
}


void glTexGenf( GLenum coord, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glTexGeni( GLenum coord, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glTexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glTexEnvi( GLenum target, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexEnviv( GLenum target, GLenum pname, const GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glTexParameteri( GLenum target, GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glTexParameterfv( GLenum target, GLenum pname,
                                          const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexParameteriv( GLenum target, GLenum pname,
                                          const GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexParameterfv( GLenum target,
                                           GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}


void glGetTexParameteriv( GLenum target,
                                           GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexLevelParameterfv( GLenum target, GLint level,
                                                GLenum pname, GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexLevelParameteriv( GLenum target, GLint level,
                                                GLenum pname, GLint *params )
{
	__OPENGL_NOT_IMPL__
}


void glTexImage1D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLint border,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glGetTexImage( GLenum target, GLint level,
                                     GLenum format, GLenum type,
                                     GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glGenTextures( GLsizei n, GLuint *textures )
{
	__OPENGL_NOT_IMPL__
}


void glDeleteTextures( GLsizei n, const GLuint *textures)
{
	__OPENGL_NOT_IMPL__
}


void glBindTexture( GLenum target, GLuint texture )
{
	__OPENGL_NOT_IMPL__
}


void glPrioritizeTextures( GLsizei n,
                                            const GLuint *textures,
                                            const GLclampf *priorities )
{
	__OPENGL_NOT_IMPL__
}


GLboolean glAreTexturesResident( GLsizei n,
                                                  const GLuint *textures,
                                                  GLboolean *residences )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


GLboolean glIsTexture( GLuint texture )
{
	__OPENGL_NOT_IMPL__
	return 0;
}


void glTexSubImage1D( GLenum target, GLint level,
                                       GLint xoffset,
                                       GLsizei width, GLenum format,
                                       GLenum type, const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glTexSubImage2D( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glCopyTexImage1D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLint border )
{
	__OPENGL_NOT_IMPL__
}


void glCopyTexImage2D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border )
{
	__OPENGL_NOT_IMPL__
}


void glCopyTexSubImage1D( GLenum target, GLint level,
                                           GLint xoffset, GLint x, GLint y,
                                           GLsizei width )
{
	__OPENGL_NOT_IMPL__
}


void glCopyTexSubImage2D( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height )
{
	__OPENGL_NOT_IMPL__
}


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


void glFogf( GLenum pname, GLfloat param )
{
	__OPENGL_NOT_IMPL__
}


void glFogi( GLenum pname, GLint param )
{
	__OPENGL_NOT_IMPL__
}


void glFogfv( GLenum pname, const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glFogiv( GLenum pname, const GLint *params )
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


void glSelectBuffer( GLsizei size, GLuint *buffer )
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


void glDrawRangeElements( GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
	__OPENGL_NOT_IMPL__
}


void glTexImage3D( GLenum target, GLint level,
                                      GLint internalFormat,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLint border,
                                      GLenum format, GLenum type,
                                      const GLvoid *pixels )
{
	__OPENGL_NOT_IMPL__
}


void glTexSubImage3D( GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}


void glCopyTexSubImage3D( GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLint x,
                                             GLint y, GLsizei width,
                                             GLsizei height )
{
	__OPENGL_NOT_IMPL__
}


void glColorTable( GLenum target, GLenum internalformat,
                                    GLsizei width, GLenum format,
                                    GLenum type, const GLvoid *table )
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


void glCopyColorSubTable( GLenum target, GLsizei start,
                                           GLint x, GLint y, GLsizei width )
{
	__OPENGL_NOT_IMPL__
}


void glCopyColorTable( GLenum target, GLenum internalformat,
                                        GLint x, GLint y, GLsizei width )
{
	__OPENGL_NOT_IMPL__
}


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


void glBlendEquation( GLenum mode )
{
	__OPENGL_NOT_IMPL__
}


void glBlendColor( GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha )
{
	__OPENGL_NOT_IMPL__
}


void glHistogram( GLenum target, GLsizei width,
				   GLenum internalformat, GLboolean sink )
{
	__OPENGL_NOT_IMPL__
}


void glResetHistogram( GLenum target )
{
	__OPENGL_NOT_IMPL__
}


void glGetHistogram( GLenum target, GLboolean reset,
				      GLenum format, GLenum type,
				      GLvoid *values )
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


void glConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image )
{
	__OPENGL_NOT_IMPL__
}


void glConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image )
{
	__OPENGL_NOT_IMPL__
}


void glConvolutionParameterf( GLenum target, GLenum pname,
	GLfloat params )
{
	__OPENGL_NOT_IMPL__
}


void glConvolutionParameterfv( GLenum target, GLenum pname,
	const GLfloat *params )
{
	__OPENGL_NOT_IMPL__
}


void glConvolutionParameteri( GLenum target, GLenum pname,
	GLint params )
{
	__OPENGL_NOT_IMPL__
}


void glConvolutionParameteriv( GLenum target, GLenum pname,
	const GLint *params )
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


void glSeparableFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column )
{
	__OPENGL_NOT_IMPL__
}


void glGetSeparableFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span )
{
	__OPENGL_NOT_IMPL__
}


void glActiveTexture( GLenum texture )
{
	__OPENGL_NOT_IMPL__
}


void glClientActiveTexture( GLenum texture )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexImage1D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data )
{
	__OPENGL_NOT_IMPL__
}


void glGetCompressedTexImage( GLenum target, GLint lod, GLvoid *img )
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


void glSampleCoverage( GLclampf value, GLboolean invert )
{
	__OPENGL_NOT_IMPL__
}


void glActiveTextureARB(GLenum texture)
{
	__OPENGL_NOT_IMPL__
}


void glClientActiveTextureARB(GLenum texture)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1dARB(GLenum target, GLdouble s)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1dvARB(GLenum target, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1fARB(GLenum target, GLfloat s)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1fvARB(GLenum target, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1iARB(GLenum target, GLint s)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1ivARB(GLenum target, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1sARB(GLenum target, GLshort s)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord1svARB(GLenum target, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2dvARB(GLenum target, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2fvARB(GLenum target, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2iARB(GLenum target, GLint s, GLint t)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2ivARB(GLenum target, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord2svARB(GLenum target, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3dvARB(GLenum target, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3fvARB(GLenum target, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3ivARB(GLenum target, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord3svARB(GLenum target, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4dvARB(GLenum target, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4fvARB(GLenum target, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4ivARB(GLenum target, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
	__OPENGL_NOT_IMPL__
}


void glMultiTexCoord4svARB(GLenum target, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}


GLhandleARB glCreateDebugObjectMESA (void)
{
	__OPENGL_NOT_IMPL__
	return 0;
}


void glClearDebugLogMESA (GLhandleARB obj, GLenum logType, GLenum shaderType)
{
	__OPENGL_NOT_IMPL__
}


void glGetDebugLogMESA (GLhandleARB obj, GLenum logType, GLenum shaderType, GLsizei maxLength,
                                         GLsizei *length, GLcharARB *debugLog)
{
	__OPENGL_NOT_IMPL__
}


GLsizei glGetDebugLogLengthMESA (GLhandleARB obj, GLenum logType, GLenum shaderType)
{
	__OPENGL_NOT_IMPL__
	return 0;
}


void glProgramCallbackMESA(GLenum target, GLprogramcallbackMESA callback, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}


void glGetProgramRegisterfvMESA(GLenum target, GLsizei len, const GLubyte *name, GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}


void *glFramebufferTextureLayerEXT(GLenum target,
    GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__OPENGL_NOT_IMPL__
	return NULL;
}


void glBlendEquationSeparateATI( GLenum modeRGB, GLenum modeA )
{
	__OPENGL_NOT_IMPL__
}


void *glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image)
{
	__OPENGL_NOT_IMPL__
	return NULL;
}


void *glEGLImageTargetRenderbufferStorageOES (GLenum target, GLeglImageOES image)
{
	__OPENGL_NOT_IMPL__
	return NULL;
}

/* GL/glext.h */

void glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawArrays (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawElements (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount)
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

void glFogCoordPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
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

void glSecondaryColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
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

void glGenQueries (GLsizei n, GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteQueries (GLsizei n, const GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsQuery (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBeginQuery (GLenum target, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glEndQuery (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryiv (GLenum target, GLenum pname, GLint *params)
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

void glBindBuffer (GLenum target, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteBuffers (GLsizei n, const GLuint *buffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenBuffers (GLsizei n, GLuint *buffers)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsBuffer (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
	__OPENGL_NOT_IMPL__
}

void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

GLvoid* glMapBuffer (GLenum target, GLenum access)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glUnmapBuffer (GLenum target)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glDrawBuffers (GLsizei n, const GLenum *bufs)
{
	__OPENGL_NOT_IMPL__
}

void glStencilOpSeparate (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	__OPENGL_NOT_IMPL__
}

void glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask)
{
	__OPENGL_NOT_IMPL__
}

void glStencilMaskSeparate (GLenum face, GLuint mask)
{
	__OPENGL_NOT_IMPL__
}

void glAttachShader (GLuint program, GLuint shader)
{
	__OPENGL_NOT_IMPL__
}

void glBindAttribLocation (GLuint program, GLuint index, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glCompileShader (GLuint shader)
{
	__OPENGL_NOT_IMPL__
}

GLuint glCreateProgram (void)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glCreateShader (GLenum type)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDeleteProgram (GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteShader (GLuint shader)
{
	__OPENGL_NOT_IMPL__
}

void glDetachShader (GLuint program, GLuint shader)
{
	__OPENGL_NOT_IMPL__
}

void glDisableVertexAttribArray (GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glEnableVertexAttribArray (GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetAttribLocation (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetProgramiv (GLuint program, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderiv (GLuint shader, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetUniformLocation (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetUniformfv (GLuint program, GLint location, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformiv (GLuint program, GLint location, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribdv (GLuint index, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsProgram (GLuint program)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glIsShader (GLuint shader)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glLinkProgram (GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glShaderSource (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length)
{
	__OPENGL_NOT_IMPL__
}

void glUseProgram (GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1f (GLint location, GLfloat v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2f (GLint location, GLfloat v0, GLfloat v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1i (GLint location, GLint v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2i (GLint location, GLint v0, GLint v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3i (GLint location, GLint v0, GLint v1, GLint v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4fv (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4iv (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glValidateProgram (GLuint program)
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

void glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glColorMaski (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	__OPENGL_NOT_IMPL__
}

void glGetBooleani_v (GLenum target, GLuint index, GLboolean *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetIntegeri_v (GLenum target, GLuint index, GLint *data)
{
	__OPENGL_NOT_IMPL__
}

void glEnablei (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glDisablei (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsEnabledi (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBeginTransformFeedback (GLenum primitiveMode)
{
	__OPENGL_NOT_IMPL__
}

void glEndTransformFeedback (void)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferBase (GLenum target, GLuint index, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar* *varyings, GLenum bufferMode)
{
	__OPENGL_NOT_IMPL__
}

void glGetTransformFeedbackVarying (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glClampColor (GLenum target, GLenum clamp)
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

void glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribIiv (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribIuiv (GLuint index, GLenum pname, GLuint *params)
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

void glGetUniformuiv (GLuint program, GLint location, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glBindFragDataLocation (GLuint program, GLuint color, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetFragDataLocation (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glUniform1ui (GLint location, GLuint v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2ui (GLint location, GLuint v0, GLuint v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3ui (GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4ui (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4uiv (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glTexParameterIiv (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glTexParameterIuiv (GLenum target, GLenum pname, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterIiv (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterIuiv (GLenum target, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferiv (GLenum buffer, GLint drawbuffer, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferuiv (GLenum buffer, GLint drawbuffer, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferfv (GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glClearBufferfi (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	__OPENGL_NOT_IMPL__
}

const GLubyte * glGetStringi (GLenum name, GLuint index)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glTexBuffer (GLenum target, GLenum internalformat, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glPrimitiveRestartIndex (GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glGetInteger64i_v (GLenum target, GLuint index, GLint64 *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferParameteri64v (GLenum target, GLenum pname, GLint64 *params)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribDivisor (GLuint index, GLuint divisor)
{
	__OPENGL_NOT_IMPL__
}

void glMinSampleShading (GLclampf value)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationi (GLuint buf, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationSeparatei (GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFunci (GLuint buf, GLenum src, GLenum dst)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncSeparatei (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTexSubImage3DARB (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTexSubImage2DARB (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTexSubImage1DARB (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetCompressedTexImageARB (GLenum target, GLint level, GLvoid *img)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterfARB (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterfvARB (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glWeightbvARB (GLint size, const GLbyte *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightsvARB (GLint size, const GLshort *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightivARB (GLint size, const GLint *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightfvARB (GLint size, const GLfloat *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightdvARB (GLint size, const GLdouble *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightubvARB (GLint size, const GLubyte *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightusvARB (GLint size, const GLushort *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightuivARB (GLint size, const GLuint *weights)
{
	__OPENGL_NOT_IMPL__
}

void glWeightPointerARB (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glVertexBlendARB (GLint count)
{
	__OPENGL_NOT_IMPL__
}

void glCurrentPaletteMatrixARB (GLint index)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixIndexubvARB (GLint size, const GLubyte *indices)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixIndexusvARB (GLint size, const GLushort *indices)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixIndexuivARB (GLint size, const GLuint *indices)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixIndexPointerARB (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2dARB (GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2dvARB (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2fARB (GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2fvARB (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2iARB (GLint x, GLint y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2ivARB (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2sARB (GLshort x, GLshort y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2svARB (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3dARB (GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3dvARB (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3fARB (GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3fvARB (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3iARB (GLint x, GLint y, GLint z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3ivARB (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3sARB (GLshort x, GLshort y, GLshort z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3svARB (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1dARB (GLuint index, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1dvARB (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1fARB (GLuint index, GLfloat x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1fvARB (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1sARB (GLuint index, GLshort x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1svARB (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2dARB (GLuint index, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2dvARB (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2fARB (GLuint index, GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2fvARB (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2sARB (GLuint index, GLshort x, GLshort y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2svARB (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3dARB (GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3dvARB (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3fARB (GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3fvARB (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3sARB (GLuint index, GLshort x, GLshort y, GLshort z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3svARB (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4NbvARB (GLuint index, const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4NivARB (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4NsvARB (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4NubARB (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4NubvARB (GLuint index, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4NuivARB (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4NusvARB (GLuint index, const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4bvARB (GLuint index, const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4dARB (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4dvARB (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4fARB (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4fvARB (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4ivARB (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4sARB (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4svARB (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4ubvARB (GLuint index, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4uivARB (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4usvARB (GLuint index, const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribPointerARB (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glEnableVertexAttribArrayARB (GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glDisableVertexAttribArrayARB (GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glProgramStringARB (GLenum target, GLenum format, GLsizei len, const GLvoid *string)
{
	__OPENGL_NOT_IMPL__
}

void glBindProgramARB (GLenum target, GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteProgramsARB (GLsizei n, const GLuint *programs)
{
	__OPENGL_NOT_IMPL__
}

void glGenProgramsARB (GLsizei n, GLuint *programs)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameter4dARB (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameter4dvARB (GLenum target, GLuint index, const GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameter4fARB (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameter4fvARB (GLenum target, GLuint index, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameter4dARB (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameter4dvARB (GLenum target, GLuint index, const GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameter4fARB (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameter4fvARB (GLenum target, GLuint index, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramEnvParameterdvARB (GLenum target, GLuint index, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramEnvParameterfvARB (GLenum target, GLuint index, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramLocalParameterdvARB (GLenum target, GLuint index, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramLocalParameterfvARB (GLenum target, GLuint index, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramivARB (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramStringARB (GLenum target, GLenum pname, GLvoid *string)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribdvARB (GLuint index, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribfvARB (GLuint index, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribivARB (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribPointervARB (GLuint index, GLenum pname, GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsProgramARB (GLuint program)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindBufferARB (GLenum target, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteBuffersARB (GLsizei n, const GLuint *buffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenBuffersARB (GLsizei n, GLuint *buffers)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsBufferARB (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBufferDataARB (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage)
{
	__OPENGL_NOT_IMPL__
}

void glBufferSubDataARB (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferSubDataARB (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

GLvoid* glMapBufferARB (GLenum target, GLenum access)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glUnmapBufferARB (GLenum target)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetBufferParameterivARB (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetBufferPointervARB (GLenum target, GLenum pname, GLvoid* *params)
{
	__OPENGL_NOT_IMPL__
}

void glGenQueriesARB (GLsizei n, GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteQueriesARB (GLsizei n, const GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsQueryARB (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBeginQueryARB (GLenum target, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glEndQueryARB (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryivARB (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjectivARB (GLuint id, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjectuivARB (GLuint id, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteObjectARB (GLhandleARB obj)
{
	__OPENGL_NOT_IMPL__
}

GLhandleARB glGetHandleARB (GLenum pname)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDetachObjectARB (GLhandleARB containerObj, GLhandleARB attachedObj)
{
	__OPENGL_NOT_IMPL__
}

GLhandleARB glCreateShaderObjectARB (GLenum shaderType)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glShaderSourceARB (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length)
{
	__OPENGL_NOT_IMPL__
}

void glCompileShaderARB (GLhandleARB shaderObj)
{
	__OPENGL_NOT_IMPL__
}

GLhandleARB glCreateProgramObjectARB (void)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glAttachObjectARB (GLhandleARB containerObj, GLhandleARB obj)
{
	__OPENGL_NOT_IMPL__
}

void glLinkProgramARB (GLhandleARB programObj)
{
	__OPENGL_NOT_IMPL__
}

void glUseProgramObjectARB (GLhandleARB programObj)
{
	__OPENGL_NOT_IMPL__
}

void glValidateProgramARB (GLhandleARB programObj)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1fARB (GLint location, GLfloat v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2fARB (GLint location, GLfloat v0, GLfloat v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3fARB (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4fARB (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1iARB (GLint location, GLint v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2iARB (GLint location, GLint v0, GLint v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3iARB (GLint location, GLint v0, GLint v1, GLint v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4iARB (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1fvARB (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2fvARB (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3fvARB (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4fvARB (GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1ivARB (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2ivARB (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3ivARB (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4ivARB (GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glGetObjectParameterfvARB (GLhandleARB obj, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetObjectParameterivARB (GLhandleARB obj, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetInfoLogARB (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)
{
	__OPENGL_NOT_IMPL__
}

void glGetAttachedObjectsARB (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetUniformLocationARB (GLhandleARB programObj, const GLcharARB *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetActiveUniformARB (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformfvARB (GLhandleARB programObj, GLint location, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformivARB (GLhandleARB programObj, GLint location, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderSourceARB (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source)
{
	__OPENGL_NOT_IMPL__
}

void glBindAttribLocationARB (GLhandleARB programObj, GLuint index, const GLcharARB *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveAttribARB (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetAttribLocationARB (GLhandleARB programObj, const GLcharARB *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDrawBuffersARB (GLsizei n, const GLenum *bufs)
{
	__OPENGL_NOT_IMPL__
}

void glClampColorARB (GLenum target, GLenum clamp)
{
	__OPENGL_NOT_IMPL__
}

void glDrawArraysInstancedARB (GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsInstancedARB (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsRenderbuffer (GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindRenderbuffer (GLenum target, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenRenderbuffers (GLsizei n, GLuint *renderbuffers)
{
	__OPENGL_NOT_IMPL__
}

void glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsFramebuffer (GLuint framebuffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindFramebuffer (GLenum target, GLuint framebuffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenFramebuffers (GLsizei n, GLuint *framebuffers)
{
	__OPENGL_NOT_IMPL__
}

GLenum glCheckFramebufferStatus (GLenum target)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGenerateMipmap (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	__OPENGL_NOT_IMPL__
}

void glRenderbufferStorageMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTextureLayer (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameteriARB (GLuint program, GLenum pname, GLint value)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTextureARB (GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTextureLayerARB (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTextureFaceARB (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribDivisorARB (GLuint index, GLuint divisor)
{
	__OPENGL_NOT_IMPL__
}

GLvoid* glMapBufferRange (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glFlushMappedBufferRange (GLenum target, GLintptr offset, GLsizeiptr length)
{
	__OPENGL_NOT_IMPL__
}

void glTexBufferARB (GLenum target, GLenum internalformat, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glBindVertexArray (GLuint array)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteVertexArrays (GLsizei n, const GLuint *arrays)
{
	__OPENGL_NOT_IMPL__
}

void glGenVertexArrays (GLsizei n, GLuint *arrays)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsVertexArray (GLuint array)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetUniformIndices (GLuint program, GLsizei uniformCount, const GLchar* *uniformNames, GLuint *uniformIndices)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniformsiv (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniformName (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
	__OPENGL_NOT_IMPL__
}

GLuint glGetUniformBlockIndex (GLuint program, const GLchar *uniformBlockName)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetActiveUniformBlockiv (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveUniformBlockName (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
	__OPENGL_NOT_IMPL__
}

void glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	__OPENGL_NOT_IMPL__
}

void glCopyBufferSubData (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)
{
	__OPENGL_NOT_IMPL__
}

void glDrawRangeElementsBaseVertex (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsInstancedBaseVertex (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount, GLint basevertex)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawElementsBaseVertex (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount, const GLint *basevertex)
{
	__OPENGL_NOT_IMPL__
}

void glProvokingVertex (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

GLsync glFenceSync (GLenum condition, GLbitfield flags)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glIsSync (GLsync sync)
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

void glGetInteger64v (GLenum pname, GLint64 *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSynciv (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
	__OPENGL_NOT_IMPL__
}

void glTexImage2DMultisample (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	__OPENGL_NOT_IMPL__
}

void glTexImage3DMultisample (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultisamplefv (GLenum pname, GLuint index, GLfloat *val)
{
	__OPENGL_NOT_IMPL__
}

void glSampleMaski (GLuint index, GLbitfield mask)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationiARB (GLuint buf, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationSeparateiARB (GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFunciARB (GLuint buf, GLenum src, GLenum dst)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncSeparateiARB (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glMinSampleShadingARB (GLclampf value)
{
	__OPENGL_NOT_IMPL__
}

void glNamedStringARB (GLenum type, GLint namelen, const GLchar *name, GLint stringlen, const GLchar *string)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteNamedStringARB (GLint namelen, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glCompileShaderIncludeARB (GLuint shader, GLsizei count, const GLchar* *path, const GLint *length)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsNamedStringARB (GLint namelen, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetNamedStringARB (GLint namelen, const GLchar *name, GLsizei bufSize, GLint *stringlen, GLchar *string)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedStringivARB (GLint namelen, const GLchar *name, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glBindFragDataLocationIndexed (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetFragDataIndex (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGenSamplers (GLsizei count, GLuint *samplers)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteSamplers (GLsizei count, const GLuint *samplers)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsSampler (GLuint sampler)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindSampler (GLuint unit, GLuint sampler)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameteri (GLuint sampler, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameteriv (GLuint sampler, GLenum pname, const GLint *param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterf (GLuint sampler, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterfv (GLuint sampler, GLenum pname, const GLfloat *param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterIiv (GLuint sampler, GLenum pname, const GLint *param)
{
	__OPENGL_NOT_IMPL__
}

void glSamplerParameterIuiv (GLuint sampler, GLenum pname, const GLuint *param)
{
	__OPENGL_NOT_IMPL__
}

void glGetSamplerParameteriv (GLuint sampler, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSamplerParameterIiv (GLuint sampler, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSamplerParameterfv (GLuint sampler, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSamplerParameterIuiv (GLuint sampler, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glQueryCounter (GLuint id, GLenum target)
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

void glNormalP3ui (GLenum type, GLuint coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormalP3uiv (GLenum type, const GLuint *coords)
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

void glSecondaryColorP3ui (GLenum type, GLuint color)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColorP3uiv (GLenum type, const GLuint *color)
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

void glDrawArraysIndirect (GLenum mode, const GLvoid *indirect)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsIndirect (GLenum mode, GLenum type, const GLvoid *indirect)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1d (GLint location, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2d (GLint location, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3d (GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4d (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4dv (GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x3dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix2x4dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x2dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix3x4dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x2dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformMatrix4x3dv (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformdv (GLuint program, GLint location, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetSubroutineUniformLocation (GLuint program, GLenum shadertype, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glGetSubroutineIndex (GLuint program, GLenum shadertype, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetActiveSubroutineUniformiv (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveSubroutineUniformName (GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveSubroutineName (GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glUniformSubroutinesuiv (GLenum shadertype, GLsizei count, const GLuint *indices)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformSubroutineuiv (GLenum shadertype, GLint location, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramStageiv (GLuint program, GLenum shadertype, GLenum pname, GLint *values)
{
	__OPENGL_NOT_IMPL__
}

void glPatchParameteri (GLenum pname, GLint value)
{
	__OPENGL_NOT_IMPL__
}

void glPatchParameterfv (GLenum pname, const GLfloat *values)
{
	__OPENGL_NOT_IMPL__
}

void glBindTransformFeedback (GLenum target, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteTransformFeedbacks (GLsizei n, const GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glGenTransformFeedbacks (GLsizei n, GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsTransformFeedback (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
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

void glDrawTransformFeedbackStream (GLenum mode, GLuint id, GLuint stream)
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

void glGetQueryIndexediv (GLenum target, GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glReleaseShaderCompiler (void)
{
	__OPENGL_NOT_IMPL__
}

void glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length)
{
	__OPENGL_NOT_IMPL__
}

void glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
	__OPENGL_NOT_IMPL__
}

void glDepthRangef (GLclampf n, GLclampf f)
{
	__OPENGL_NOT_IMPL__
}

void glClearDepthf (GLclampf d)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramBinary (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary)
{
	__OPENGL_NOT_IMPL__
}

void glProgramBinary (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameteri (GLuint program, GLenum pname, GLint value)
{
	__OPENGL_NOT_IMPL__
}

void glUseProgramStages (GLuint pipeline, GLbitfield stages, GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glActiveShaderProgram (GLuint pipeline, GLuint program)
{
	__OPENGL_NOT_IMPL__
}

GLuint glCreateShaderProgramv (GLenum type, GLsizei count, const GLchar* *strings)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindProgramPipeline (GLuint pipeline)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteProgramPipelines (GLsizei n, const GLuint *pipelines)
{
	__OPENGL_NOT_IMPL__
}

void glGenProgramPipelines (GLsizei n, GLuint *pipelines)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsProgramPipeline (GLuint pipeline)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetProgramPipelineiv (GLuint pipeline, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1i (GLuint program, GLint location, GLint v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1f (GLuint program, GLint location, GLfloat v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1d (GLuint program, GLint location, GLdouble v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1ui (GLuint program, GLint location, GLuint v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2i (GLuint program, GLint location, GLint v0, GLint v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2f (GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2d (GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2ui (GLuint program, GLint location, GLuint v0, GLuint v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3d (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4d (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4dv (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x3dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x2dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x4dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x2dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x4dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x3dv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glValidateProgramPipeline (GLuint pipeline)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramPipelineInfoLog (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
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

void glVertexAttribLPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribLdv (GLuint index, GLenum pname, GLdouble *params)
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

void glDepthRangeArrayv (GLuint first, GLsizei count, const GLclampd *v)
{
	__OPENGL_NOT_IMPL__
}

void glDepthRangeIndexed (GLuint index, GLclampd n, GLclampd f)
{
	__OPENGL_NOT_IMPL__
}

void glGetFloati_v (GLenum target, GLuint index, GLfloat *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetDoublei_v (GLenum target, GLuint index, GLdouble *data)
{
	__OPENGL_NOT_IMPL__
}

GLsync glCreateSyncFromCLeventARB (struct _cl_context * context, struct _cl_event * event, GLbitfield flags)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDebugMessageControlARB (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)
{
	__OPENGL_NOT_IMPL__
}

void glDebugMessageInsertARB (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)
{
	__OPENGL_NOT_IMPL__
}

void glDebugMessageCallbackARB (GLDEBUGPROCARB callback, const GLvoid *userParam)
{
	__OPENGL_NOT_IMPL__
}

GLuint glGetDebugMessageLogARB (GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLenum glGetGraphicsResetStatusARB (void)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetnMapdvARB (GLenum target, GLenum query, GLsizei bufSize, GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glGetnMapfvARB (GLenum target, GLenum query, GLsizei bufSize, GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glGetnMapivARB (GLenum target, GLenum query, GLsizei bufSize, GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glGetnPixelMapfvARB (GLenum map, GLsizei bufSize, GLfloat *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetnPixelMapuivARB (GLenum map, GLsizei bufSize, GLuint *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetnPixelMapusvARB (GLenum map, GLsizei bufSize, GLushort *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetnPolygonStippleARB (GLsizei bufSize, GLubyte *pattern)
{
	__OPENGL_NOT_IMPL__
}

void glGetnColorTableARB (GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid *table)
{
	__OPENGL_NOT_IMPL__
}

void glGetnConvolutionFilterARB (GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid *image)
{
	__OPENGL_NOT_IMPL__
}

void glGetnSeparableFilterARB (GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, GLvoid *row, GLsizei columnBufSize, GLvoid *column, GLvoid *span)
{
	__OPENGL_NOT_IMPL__
}

void glGetnHistogramARB (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetnMinmaxARB (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetnTexImageARB (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid *img)
{
	__OPENGL_NOT_IMPL__
}

void glReadnPixelsARB (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetnCompressedTexImageARB (GLenum target, GLint lod, GLsizei bufSize, GLvoid *img)
{
	__OPENGL_NOT_IMPL__
}

void glGetnUniformfvARB (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetnUniformivARB (GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetnUniformuivARB (GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetnUniformdvARB (GLuint program, GLint location, GLsizei bufSize, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glDrawArraysInstancedBaseInstance (GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsInstancedBaseInstance (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount, GLuint baseinstance)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsInstancedBaseVertexBaseInstance (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount, GLint basevertex, GLuint baseinstance)
{
	__OPENGL_NOT_IMPL__
}

void glDrawTransformFeedbackInstanced (GLenum mode, GLuint id, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glDrawTransformFeedbackStreamInstanced (GLenum mode, GLuint id, GLuint stream, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glGetInternalformativ (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetActiveAtomicCounterBufferiv (GLuint program, GLuint bufferIndex, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glBindImageTexture (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
	__OPENGL_NOT_IMPL__
}

void glMemoryBarrier (GLbitfield barriers)
{
	__OPENGL_NOT_IMPL__
}

void glTexStorage1D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glTexStorage3D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	__OPENGL_NOT_IMPL__
}

void glTextureStorage1DEXT (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glTextureStorage2DEXT (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glTextureStorage3DEXT (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	__OPENGL_NOT_IMPL__
}

void glBlendColorEXT (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	__OPENGL_NOT_IMPL__
}

void glPolygonOffsetEXT (GLfloat factor, GLfloat bias)
{
	__OPENGL_NOT_IMPL__
}

void glTexImage3DEXT (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTexSubImage3DEXT (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexFilterFuncSGIS (GLenum target, GLenum filter, GLfloat *weights)
{
	__OPENGL_NOT_IMPL__
}

void glTexFilterFuncSGIS (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights)
{
	__OPENGL_NOT_IMPL__
}

void glTexSubImage1DEXT (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTexSubImage2DEXT (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexImage1DEXT (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexImage2DEXT (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexSubImage1DEXT (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexSubImage2DEXT (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTexSubImage3DEXT (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetHistogramEXT (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetHistogramParameterfvEXT (GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetHistogramParameterivEXT (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMinmaxEXT (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
	__OPENGL_NOT_IMPL__
}

void glGetMinmaxParameterfvEXT (GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMinmaxParameterivEXT (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glHistogramEXT (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
	__OPENGL_NOT_IMPL__
}

void glMinmaxEXT (GLenum target, GLenum internalformat, GLboolean sink)
{
	__OPENGL_NOT_IMPL__
}

void glResetHistogramEXT (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glResetMinmaxEXT (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionFilter1DEXT (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionFilter2DEXT (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionParameterfEXT (GLenum target, GLenum pname, GLfloat params)
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionParameterfvEXT (GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionParameteriEXT (GLenum target, GLenum pname, GLint params)
{
	__OPENGL_NOT_IMPL__
}

void glConvolutionParameterivEXT (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glCopyConvolutionFilter1DEXT (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glCopyConvolutionFilter2DEXT (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetConvolutionFilterEXT (GLenum target, GLenum format, GLenum type, GLvoid *image)
{
	__OPENGL_NOT_IMPL__
}

void glGetConvolutionParameterfvEXT (GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetConvolutionParameterivEXT (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetSeparableFilterEXT (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span)
{
	__OPENGL_NOT_IMPL__
}

void glSeparableFilter2DEXT (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column)
{
	__OPENGL_NOT_IMPL__
}

void glColorTableSGI (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
	__OPENGL_NOT_IMPL__
}

void glColorTableParameterfvSGI (GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glColorTableParameterivSGI (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glCopyColorTableSGI (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glGetColorTableSGI (GLenum target, GLenum format, GLenum type, GLvoid *table)
{
	__OPENGL_NOT_IMPL__
}

void glGetColorTableParameterfvSGI (GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetColorTableParameterivSGI (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTexGenSGIX (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTexGenParameteriSGIS (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTexGenParameterivSGIS (GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTexGenParameterfSGIS (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTexGenParameterfvSGIS (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetPixelTexGenParameterivSGIS (GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetPixelTexGenParameterfvSGIS (GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glTexImage4DSGIS (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTexSubImage4DSGIS (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glAreTexturesResidentEXT (GLsizei n, const GLuint *textures, GLboolean *residences)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindTextureEXT (GLenum target, GLuint texture)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteTexturesEXT (GLsizei n, const GLuint *textures)
{
	__OPENGL_NOT_IMPL__
}

void glGenTexturesEXT (GLsizei n, GLuint *textures)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsTextureEXT (GLuint texture)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glPrioritizeTexturesEXT (GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
	__OPENGL_NOT_IMPL__
}

void glDetailTexFuncSGIS (GLenum target, GLsizei n, const GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glGetDetailTexFuncSGIS (GLenum target, GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glSharpenTexFuncSGIS (GLenum target, GLsizei n, const GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glGetSharpenTexFuncSGIS (GLenum target, GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glSampleMaskSGIS (GLclampf value, GLboolean invert)
{
	__OPENGL_NOT_IMPL__
}

void glSamplePatternSGIS (GLenum pattern)
{
	__OPENGL_NOT_IMPL__
}

void glArrayElementEXT (GLint i)
{
	__OPENGL_NOT_IMPL__
}

void glColorPointerEXT (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glDrawArraysEXT (GLenum mode, GLint first, GLsizei count)
{
	__OPENGL_NOT_IMPL__
}

void glEdgeFlagPointerEXT (GLsizei stride, GLsizei count, const GLboolean *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glGetPointervEXT (GLenum pname, GLvoid* *params)
{
	__OPENGL_NOT_IMPL__
}

void glIndexPointerEXT (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glNormalPointerEXT (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordPointerEXT (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glVertexPointerEXT (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationEXT (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glSpriteParameterfSGIX (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glSpriteParameterfvSGIX (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glSpriteParameteriSGIX (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glSpriteParameterivSGIX (GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterfEXT (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterfvEXT (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterfSGIS (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterfvSGIS (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetInstrumentsSGIX (void)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glInstrumentsBufferSGIX (GLsizei size, GLint *buffer)
{
	__OPENGL_NOT_IMPL__
}

GLint glPollInstrumentsSGIX (GLint *marker_p)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glReadInstrumentsSGIX (GLint marker)
{
	__OPENGL_NOT_IMPL__
}

void glStartInstrumentsSGIX (void)
{
	__OPENGL_NOT_IMPL__
}

void glStopInstrumentsSGIX (GLint marker)
{
	__OPENGL_NOT_IMPL__
}

void glFrameZoomSGIX (GLint factor)
{
	__OPENGL_NOT_IMPL__
}

void glTagSampleBufferSGIX (void)
{
	__OPENGL_NOT_IMPL__
}

void glDeformationMap3dSGIX (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble *points)
{
	__OPENGL_NOT_IMPL__
}

void glDeformationMap3fSGIX (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glDeformSGIX (GLbitfield mask)
{
	__OPENGL_NOT_IMPL__
}

void glLoadIdentityDeformationMapSGIX (GLbitfield mask)
{
	__OPENGL_NOT_IMPL__
}

void glReferencePlaneSGIX (const GLdouble *equation)
{
	__OPENGL_NOT_IMPL__
}

void glFlushRasterSGIX (void)
{
	__OPENGL_NOT_IMPL__
}

void glFogFuncSGIS (GLsizei n, const GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glGetFogFuncSGIS (GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glImageTransformParameteriHP (GLenum target, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glImageTransformParameterfHP (GLenum target, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glImageTransformParameterivHP (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glImageTransformParameterfvHP (GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetImageTransformParameterivHP (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetImageTransformParameterfvHP (GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glColorSubTableEXT (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glCopyColorSubTableEXT (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glHintPGI (GLenum target, GLint mode)
{
	__OPENGL_NOT_IMPL__
}

void glColorTableEXT (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
	__OPENGL_NOT_IMPL__
}

void glGetColorTableEXT (GLenum target, GLenum format, GLenum type, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetColorTableParameterivEXT (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetColorTableParameterfvEXT (GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetListParameterfvSGIX (GLuint list, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetListParameterivSGIX (GLuint list, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glListParameterfSGIX (GLuint list, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glListParameterfvSGIX (GLuint list, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glListParameteriSGIX (GLuint list, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glListParameterivSGIX (GLuint list, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glIndexMaterialEXT (GLenum face, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glIndexFuncEXT (GLenum func, GLclampf ref)
{
	__OPENGL_NOT_IMPL__
}

void glLockArraysEXT (GLint first, GLsizei count)
{
	__OPENGL_NOT_IMPL__
}

void glUnlockArraysEXT (void)
{
	__OPENGL_NOT_IMPL__
}

void glCullParameterdvEXT (GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glCullParameterfvEXT (GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentColorMaterialSGIX (GLenum face, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightfSGIX (GLenum light, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightfvSGIX (GLenum light, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightiSGIX (GLenum light, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightivSGIX (GLenum light, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightModelfSGIX (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightModelfvSGIX (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightModeliSGIX (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentLightModelivSGIX (GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentMaterialfSGIX (GLenum face, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentMaterialfvSGIX (GLenum face, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentMaterialiSGIX (GLenum face, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glFragmentMaterialivSGIX (GLenum face, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFragmentLightfvSGIX (GLenum light, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFragmentLightivSGIX (GLenum light, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFragmentMaterialfvSGIX (GLenum face, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFragmentMaterialivSGIX (GLenum face, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glLightEnviSGIX (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glDrawRangeElementsEXT (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	__OPENGL_NOT_IMPL__
}

void glApplyTextureEXT (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glTextureLightEXT (GLenum pname)
{
	__OPENGL_NOT_IMPL__
}

void glTextureMaterialEXT (GLenum face, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glAsyncMarkerSGIX (GLuint marker)
{
	__OPENGL_NOT_IMPL__
}

GLint glFinishAsyncSGIX (GLuint *markerp)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLint glPollAsyncSGIX (GLuint *markerp)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glGenAsyncMarkersSGIX (GLsizei range)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDeleteAsyncMarkersSGIX (GLuint marker, GLsizei range)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsAsyncMarkerSGIX (GLuint marker)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glVertexPointervINTEL (GLint size, GLenum type, const GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glNormalPointervINTEL (GLenum type, const GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glColorPointervINTEL (GLint size, GLenum type, const GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordPointervINTEL (GLint size, GLenum type, const GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTransformParameteriEXT (GLenum target, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTransformParameterfEXT (GLenum target, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTransformParameterivEXT (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glPixelTransformParameterfvEXT (GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3bEXT (GLbyte red, GLbyte green, GLbyte blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3bvEXT (const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3dEXT (GLdouble red, GLdouble green, GLdouble blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3dvEXT (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3fEXT (GLfloat red, GLfloat green, GLfloat blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3fvEXT (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3iEXT (GLint red, GLint green, GLint blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3ivEXT (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3sEXT (GLshort red, GLshort green, GLshort blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3svEXT (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3ubEXT (GLubyte red, GLubyte green, GLubyte blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3ubvEXT (const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3uiEXT (GLuint red, GLuint green, GLuint blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3uivEXT (const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3usEXT (GLushort red, GLushort green, GLushort blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3usvEXT (const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColorPointerEXT (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glTextureNormalEXT (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawArraysEXT (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawElementsEXT (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordfEXT (GLfloat coord)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordfvEXT (const GLfloat *coord)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoorddEXT (GLdouble coord)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoorddvEXT (const GLdouble *coord)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordPointerEXT (GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3bEXT (GLbyte tx, GLbyte ty, GLbyte tz)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3bvEXT (const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3dEXT (GLdouble tx, GLdouble ty, GLdouble tz)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3dvEXT (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3fEXT (GLfloat tx, GLfloat ty, GLfloat tz)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3fvEXT (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3iEXT (GLint tx, GLint ty, GLint tz)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3ivEXT (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3sEXT (GLshort tx, GLshort ty, GLshort tz)
{
	__OPENGL_NOT_IMPL__
}

void glTangent3svEXT (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3bEXT (GLbyte bx, GLbyte by, GLbyte bz)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3bvEXT (const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3dEXT (GLdouble bx, GLdouble by, GLdouble bz)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3dvEXT (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3fEXT (GLfloat bx, GLfloat by, GLfloat bz)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3fvEXT (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3iEXT (GLint bx, GLint by, GLint bz)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3ivEXT (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3sEXT (GLshort bx, GLshort by, GLshort bz)
{
	__OPENGL_NOT_IMPL__
}

void glBinormal3svEXT (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glTangentPointerEXT (GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glBinormalPointerEXT (GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glFinishTextureSUNX (void)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactorbSUN (GLbyte factor)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactorsSUN (GLshort factor)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactoriSUN (GLint factor)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactorfSUN (GLfloat factor)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactordSUN (GLdouble factor)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactorubSUN (GLubyte factor)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactorusSUN (GLushort factor)
{
	__OPENGL_NOT_IMPL__
}

void glGlobalAlphaFactoruiSUN (GLuint factor)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiSUN (GLuint code)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeusSUN (GLushort code)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeubSUN (GLubyte code)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuivSUN (const GLuint *code)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeusvSUN (const GLushort *code)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeubvSUN (const GLubyte *code)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodePointerSUN (GLenum type, GLsizei stride, const GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glColor4ubVertex2fSUN (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glColor4ubVertex2fvSUN (const GLubyte *c, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glColor4ubVertex3fSUN (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glColor4ubVertex3fvSUN (const GLubyte *c, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glColor3fVertex3fSUN (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glColor3fVertex3fvSUN (const GLfloat *c, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glNormal3fVertex3fSUN (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glNormal3fVertex3fvSUN (const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glColor4fNormal3fVertex3fSUN (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glColor4fNormal3fVertex3fvSUN (const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fVertex3fSUN (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fVertex3fvSUN (const GLfloat *tc, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord4fVertex4fSUN (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord4fVertex4fvSUN (const GLfloat *tc, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fColor4ubVertex3fSUN (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fColor4ubVertex3fvSUN (const GLfloat *tc, const GLubyte *c, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fColor3fVertex3fSUN (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fColor3fVertex3fvSUN (const GLfloat *tc, const GLfloat *c, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fNormal3fVertex3fSUN (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fNormal3fVertex3fvSUN (const GLfloat *tc, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fColor4fNormal3fVertex3fSUN (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2fColor4fNormal3fVertex3fvSUN (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord4fColor4fNormal3fVertex4fSUN (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord4fColor4fNormal3fVertex4fvSUN (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiVertex3fSUN (GLuint rc, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiVertex3fvSUN (const GLuint *rc, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiColor4ubVertex3fSUN (GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiColor4ubVertex3fvSUN (const GLuint *rc, const GLubyte *c, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiColor3fVertex3fSUN (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiColor3fVertex3fvSUN (const GLuint *rc, const GLfloat *c, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiNormal3fVertex3fSUN (GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiNormal3fVertex3fvSUN (const GLuint *rc, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiColor4fNormal3fVertex3fSUN (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiColor4fNormal3fVertex3fvSUN (const GLuint *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiTexCoord2fVertex3fSUN (GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiTexCoord2fVertex3fvSUN (const GLuint *rc, const GLfloat *tc, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN (GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN (const GLuint *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN (GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN (const GLuint *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncSeparateEXT (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncSeparateINGR (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glVertexWeightfEXT (GLfloat weight)
{
	__OPENGL_NOT_IMPL__
}

void glVertexWeightfvEXT (const GLfloat *weight)
{
	__OPENGL_NOT_IMPL__
}

void glVertexWeightPointerEXT (GLsizei size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glFlushVertexArrayRangeNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glVertexArrayRangeNV (GLsizei length, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glCombinerParameterfvNV (GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glCombinerParameterfNV (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glCombinerParameterivNV (GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glCombinerParameteriNV (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glCombinerInputNV (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)
{
	__OPENGL_NOT_IMPL__
}

void glCombinerOutputNV (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum)
{
	__OPENGL_NOT_IMPL__
}

void glFinalCombinerInputNV (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)
{
	__OPENGL_NOT_IMPL__
}

void glGetCombinerInputParameterfvNV (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetCombinerInputParameterivNV (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetCombinerOutputParameterfvNV (GLenum stage, GLenum portion, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetCombinerOutputParameterivNV (GLenum stage, GLenum portion, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFinalCombinerInputParameterfvNV (GLenum variable, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFinalCombinerInputParameterivNV (GLenum variable, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glResizeBuffersMESA (void)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2dMESA (GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2dvMESA (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2fMESA (GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2fvMESA (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2iMESA (GLint x, GLint y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2ivMESA (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2sMESA (GLshort x, GLshort y)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos2svMESA (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3dMESA (GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3dvMESA (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3fMESA (GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3fvMESA (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3iMESA (GLint x, GLint y, GLint z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3ivMESA (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3sMESA (GLshort x, GLshort y, GLshort z)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos3svMESA (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4dMESA (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4dvMESA (const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4fMESA (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4fvMESA (const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4iMESA (GLint x, GLint y, GLint z, GLint w)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4ivMESA (const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4sMESA (GLshort x, GLshort y, GLshort z, GLshort w)
{
	__OPENGL_NOT_IMPL__
}

void glWindowPos4svMESA (const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glMultiModeDrawArraysIBM (const GLenum *mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride)
{
	__OPENGL_NOT_IMPL__
}

void glMultiModeDrawElementsIBM (const GLenum *mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei primcount, GLint modestride)
{
	__OPENGL_NOT_IMPL__
}

void glColorPointerListIBM (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColorPointerListIBM (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glEdgeFlagPointerListIBM (GLint stride, const GLboolean* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordPointerListIBM (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glIndexPointerListIBM (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glNormalPointerListIBM (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordPointerListIBM (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glVertexPointerListIBM (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride)
{
	__OPENGL_NOT_IMPL__
}

void glTbufferMask3DFX (GLuint mask)
{
	__OPENGL_NOT_IMPL__
}

void glSampleMaskEXT (GLclampf value, GLboolean invert)
{
	__OPENGL_NOT_IMPL__
}

void glSamplePatternEXT (GLenum pattern)
{
	__OPENGL_NOT_IMPL__
}

void glTextureColorMaskSGIS (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	__OPENGL_NOT_IMPL__
}

void glIglooInterfaceSGIX (GLenum pname, const GLvoid *params)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteFencesNV (GLsizei n, const GLuint *fences)
{
	__OPENGL_NOT_IMPL__
}

void glGenFencesNV (GLsizei n, GLuint *fences)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsFenceNV (GLuint fence)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glTestFenceNV (GLuint fence)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetFenceivNV (GLuint fence, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glFinishFenceNV (GLuint fence)
{
	__OPENGL_NOT_IMPL__
}

void glSetFenceNV (GLuint fence, GLenum condition)
{
	__OPENGL_NOT_IMPL__
}

void glMapControlPointsNV (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const GLvoid *points)
{
	__OPENGL_NOT_IMPL__
}

void glMapParameterivNV (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glMapParameterfvNV (GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMapControlPointsNV (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, GLvoid *points)
{
	__OPENGL_NOT_IMPL__
}

void glGetMapParameterivNV (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMapParameterfvNV (GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMapAttribParameterivNV (GLenum target, GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMapAttribParameterfvNV (GLenum target, GLuint index, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glEvalMapsNV (GLenum target, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glCombinerStageParameterfvNV (GLenum stage, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetCombinerStageParameterfvNV (GLenum stage, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glAreProgramsResidentNV (GLsizei n, const GLuint *programs, GLboolean *residences)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindProgramNV (GLenum target, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteProgramsNV (GLsizei n, const GLuint *programs)
{
	__OPENGL_NOT_IMPL__
}

void glExecuteProgramNV (GLenum target, GLuint id, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGenProgramsNV (GLsizei n, GLuint *programs)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramParameterdvNV (GLenum target, GLuint index, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramParameterfvNV (GLenum target, GLuint index, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramivNV (GLuint id, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramStringNV (GLuint id, GLenum pname, GLubyte *program)
{
	__OPENGL_NOT_IMPL__
}

void glGetTrackMatrixivNV (GLenum target, GLuint address, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribdvNV (GLuint index, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribfvNV (GLuint index, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribivNV (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribPointervNV (GLuint index, GLenum pname, GLvoid* *pointer)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsProgramNV (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glLoadProgramNV (GLenum target, GLuint id, GLsizei len, const GLubyte *program)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameter4dNV (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameter4dvNV (GLenum target, GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameter4fNV (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameter4fvNV (GLenum target, GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameters4dvNV (GLenum target, GLuint index, GLsizei count, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameters4fvNV (GLenum target, GLuint index, GLsizei count, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glRequestResidentProgramsNV (GLsizei n, const GLuint *programs)
{
	__OPENGL_NOT_IMPL__
}

void glTrackMatrixNV (GLenum target, GLuint address, GLenum matrix, GLenum transform)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribPointerNV (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1dNV (GLuint index, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1dvNV (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1fNV (GLuint index, GLfloat x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1fvNV (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1sNV (GLuint index, GLshort x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1svNV (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2dNV (GLuint index, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2dvNV (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2fNV (GLuint index, GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2fvNV (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2sNV (GLuint index, GLshort x, GLshort y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2svNV (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3dNV (GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3dvNV (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3fNV (GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3fvNV (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3sNV (GLuint index, GLshort x, GLshort y, GLshort z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3svNV (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4dNV (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4dvNV (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4fNV (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4fvNV (GLuint index, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4sNV (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4svNV (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4ubNV (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4ubvNV (GLuint index, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs1dvNV (GLuint index, GLsizei count, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs1fvNV (GLuint index, GLsizei count, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs1svNV (GLuint index, GLsizei count, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs2dvNV (GLuint index, GLsizei count, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs2fvNV (GLuint index, GLsizei count, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs2svNV (GLuint index, GLsizei count, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs3dvNV (GLuint index, GLsizei count, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs3fvNV (GLuint index, GLsizei count, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs3svNV (GLuint index, GLsizei count, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs4dvNV (GLuint index, GLsizei count, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs4fvNV (GLuint index, GLsizei count, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs4svNV (GLuint index, GLsizei count, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs4ubvNV (GLuint index, GLsizei count, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexBumpParameterivATI (GLenum pname, const GLint *param)
{
	__OPENGL_NOT_IMPL__
}

void glTexBumpParameterfvATI (GLenum pname, const GLfloat *param)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexBumpParameterivATI (GLenum pname, GLint *param)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexBumpParameterfvATI (GLenum pname, GLfloat *param)
{
	__OPENGL_NOT_IMPL__
}

GLuint glGenFragmentShadersATI (GLuint range)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindFragmentShaderATI (GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteFragmentShaderATI (GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glBeginFragmentShaderATI (void)
{
	__OPENGL_NOT_IMPL__
}

void glEndFragmentShaderATI (void)
{
	__OPENGL_NOT_IMPL__
}

void glPassTexCoordATI (GLuint dst, GLuint coord, GLenum swizzle)
{
	__OPENGL_NOT_IMPL__
}

void glSampleMapATI (GLuint dst, GLuint interp, GLenum swizzle)
{
	__OPENGL_NOT_IMPL__
}

void glColorFragmentOp1ATI (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
	__OPENGL_NOT_IMPL__
}

void glColorFragmentOp2ATI (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
	__OPENGL_NOT_IMPL__
}

void glColorFragmentOp3ATI (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
	__OPENGL_NOT_IMPL__
}

void glAlphaFragmentOp1ATI (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
	__OPENGL_NOT_IMPL__
}

void glAlphaFragmentOp2ATI (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
	__OPENGL_NOT_IMPL__
}

void glAlphaFragmentOp3ATI (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
	__OPENGL_NOT_IMPL__
}

void glSetFragmentShaderConstantATI (GLuint dst, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glPNTrianglesiATI (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glPNTrianglesfATI (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

GLuint glNewObjectBufferATI (GLsizei size, const GLvoid *pointer, GLenum usage)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glIsObjectBufferATI (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glUpdateObjectBufferATI (GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve)
{
	__OPENGL_NOT_IMPL__
}

void glGetObjectBufferfvATI (GLuint buffer, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetObjectBufferivATI (GLuint buffer, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glFreeObjectBufferATI (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glArrayObjectATI (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
	__OPENGL_NOT_IMPL__
}

void glGetArrayObjectfvATI (GLenum array, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetArrayObjectivATI (GLenum array, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glVariantArrayObjectATI (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
	__OPENGL_NOT_IMPL__
}

void glGetVariantArrayObjectfvATI (GLuint id, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVariantArrayObjectivATI (GLuint id, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glBeginVertexShaderEXT (void)
{
	__OPENGL_NOT_IMPL__
}

void glEndVertexShaderEXT (void)
{
	__OPENGL_NOT_IMPL__
}

void glBindVertexShaderEXT (GLuint id)
{
	__OPENGL_NOT_IMPL__
}

GLuint glGenVertexShadersEXT (GLuint range)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDeleteVertexShaderEXT (GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glShaderOp1EXT (GLenum op, GLuint res, GLuint arg1)
{
	__OPENGL_NOT_IMPL__
}

void glShaderOp2EXT (GLenum op, GLuint res, GLuint arg1, GLuint arg2)
{
	__OPENGL_NOT_IMPL__
}

void glShaderOp3EXT (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3)
{
	__OPENGL_NOT_IMPL__
}

void glSwizzleEXT (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)
{
	__OPENGL_NOT_IMPL__
}

void glWriteMaskEXT (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)
{
	__OPENGL_NOT_IMPL__
}

void glInsertComponentEXT (GLuint res, GLuint src, GLuint num)
{
	__OPENGL_NOT_IMPL__
}

void glExtractComponentEXT (GLuint res, GLuint src, GLuint num)
{
	__OPENGL_NOT_IMPL__
}

GLuint glGenSymbolsEXT (GLenum datatype, GLenum storagetype, GLenum range, GLuint components)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glSetInvariantEXT (GLuint id, GLenum type, const GLvoid *addr)
{
	__OPENGL_NOT_IMPL__
}

void glSetLocalConstantEXT (GLuint id, GLenum type, const GLvoid *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantbvEXT (GLuint id, const GLbyte *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantsvEXT (GLuint id, const GLshort *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantivEXT (GLuint id, const GLint *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantfvEXT (GLuint id, const GLfloat *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantdvEXT (GLuint id, const GLdouble *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantubvEXT (GLuint id, const GLubyte *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantusvEXT (GLuint id, const GLushort *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantuivEXT (GLuint id, const GLuint *addr)
{
	__OPENGL_NOT_IMPL__
}

void glVariantPointerEXT (GLuint id, GLenum type, GLuint stride, const GLvoid *addr)
{
	__OPENGL_NOT_IMPL__
}

void glEnableVariantClientStateEXT (GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glDisableVariantClientStateEXT (GLuint id)
{
	__OPENGL_NOT_IMPL__
}

GLuint glBindLightParameterEXT (GLenum light, GLenum value)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glBindMaterialParameterEXT (GLenum face, GLenum value)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glBindTexGenParameterEXT (GLenum unit, GLenum coord, GLenum value)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glBindTextureUnitParameterEXT (GLenum unit, GLenum value)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLuint glBindParameterEXT (GLenum value)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glIsVariantEnabledEXT (GLuint id, GLenum cap)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetVariantBooleanvEXT (GLuint id, GLenum value, GLboolean *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetVariantIntegervEXT (GLuint id, GLenum value, GLint *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetVariantFloatvEXT (GLuint id, GLenum value, GLfloat *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetVariantPointervEXT (GLuint id, GLenum value, GLvoid* *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetInvariantBooleanvEXT (GLuint id, GLenum value, GLboolean *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetInvariantIntegervEXT (GLuint id, GLenum value, GLint *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetInvariantFloatvEXT (GLuint id, GLenum value, GLfloat *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetLocalConstantBooleanvEXT (GLuint id, GLenum value, GLboolean *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetLocalConstantIntegervEXT (GLuint id, GLenum value, GLint *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetLocalConstantFloatvEXT (GLuint id, GLenum value, GLfloat *data)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1sATI (GLenum stream, GLshort x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1svATI (GLenum stream, const GLshort *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1iATI (GLenum stream, GLint x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1ivATI (GLenum stream, const GLint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1fATI (GLenum stream, GLfloat x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1fvATI (GLenum stream, const GLfloat *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1dATI (GLenum stream, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream1dvATI (GLenum stream, const GLdouble *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2sATI (GLenum stream, GLshort x, GLshort y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2svATI (GLenum stream, const GLshort *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2iATI (GLenum stream, GLint x, GLint y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2ivATI (GLenum stream, const GLint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2fATI (GLenum stream, GLfloat x, GLfloat y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2fvATI (GLenum stream, const GLfloat *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2dATI (GLenum stream, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream2dvATI (GLenum stream, const GLdouble *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3sATI (GLenum stream, GLshort x, GLshort y, GLshort z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3svATI (GLenum stream, const GLshort *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3iATI (GLenum stream, GLint x, GLint y, GLint z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3ivATI (GLenum stream, const GLint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3fATI (GLenum stream, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3fvATI (GLenum stream, const GLfloat *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3dATI (GLenum stream, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream3dvATI (GLenum stream, const GLdouble *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4sATI (GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4svATI (GLenum stream, const GLshort *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4iATI (GLenum stream, GLint x, GLint y, GLint z, GLint w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4ivATI (GLenum stream, const GLint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4fATI (GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4fvATI (GLenum stream, const GLfloat *coords)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4dATI (GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexStream4dvATI (GLenum stream, const GLdouble *coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3bATI (GLenum stream, GLbyte nx, GLbyte ny, GLbyte nz)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3bvATI (GLenum stream, const GLbyte *coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3sATI (GLenum stream, GLshort nx, GLshort ny, GLshort nz)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3svATI (GLenum stream, const GLshort *coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3iATI (GLenum stream, GLint nx, GLint ny, GLint nz)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3ivATI (GLenum stream, const GLint *coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3fATI (GLenum stream, GLfloat nx, GLfloat ny, GLfloat nz)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3fvATI (GLenum stream, const GLfloat *coords)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3dATI (GLenum stream, GLdouble nx, GLdouble ny, GLdouble nz)
{
	__OPENGL_NOT_IMPL__
}

void glNormalStream3dvATI (GLenum stream, const GLdouble *coords)
{
	__OPENGL_NOT_IMPL__
}

void glClientActiveVertexStreamATI (GLenum stream)
{
	__OPENGL_NOT_IMPL__
}

void glVertexBlendEnviATI (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glVertexBlendEnvfATI (GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glElementPointerATI (GLenum type, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementArrayATI (GLenum mode, GLsizei count)
{
	__OPENGL_NOT_IMPL__
}

void glDrawRangeElementArrayATI (GLenum mode, GLuint start, GLuint end, GLsizei count)
{
	__OPENGL_NOT_IMPL__
}

void glDrawMeshArraysSUN (GLenum mode, GLint first, GLsizei count, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glGenOcclusionQueriesNV (GLsizei n, GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteOcclusionQueriesNV (GLsizei n, const GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsOcclusionQueryNV (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBeginOcclusionQueryNV (GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glEndOcclusionQueryNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glGetOcclusionQueryivNV (GLuint id, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetOcclusionQueryuivNV (GLuint id, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameteriNV (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glPointParameterivNV (GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glActiveStencilFaceEXT (GLenum face)
{
	__OPENGL_NOT_IMPL__
}

void glElementPointerAPPLE (GLenum type, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementArrayAPPLE (GLenum mode, GLint first, GLsizei count)
{
	__OPENGL_NOT_IMPL__
}

void glDrawRangeElementArrayAPPLE (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawElementArrayAPPLE (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawRangeElementArrayAPPLE (GLenum mode, GLuint start, GLuint end, const GLint *first, const GLsizei *count, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glGenFencesAPPLE (GLsizei n, GLuint *fences)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteFencesAPPLE (GLsizei n, const GLuint *fences)
{
	__OPENGL_NOT_IMPL__
}

void glSetFenceAPPLE (GLuint fence)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsFenceAPPLE (GLuint fence)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glTestFenceAPPLE (GLuint fence)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glFinishFenceAPPLE (GLuint fence)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glTestObjectAPPLE (GLenum object, GLuint name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glFinishObjectAPPLE (GLenum object, GLint name)
{
	__OPENGL_NOT_IMPL__
}

void glBindVertexArrayAPPLE (GLuint array)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteVertexArraysAPPLE (GLsizei n, const GLuint *arrays)
{
	__OPENGL_NOT_IMPL__
}

void glGenVertexArraysAPPLE (GLsizei n, GLuint *arrays)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsVertexArrayAPPLE (GLuint array)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glVertexArrayRangeAPPLE (GLsizei length, GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glFlushVertexArrayRangeAPPLE (GLsizei length, GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glVertexArrayParameteriAPPLE (GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glDrawBuffersATI (GLsizei n, const GLenum *bufs)
{
	__OPENGL_NOT_IMPL__
}

void glProgramNamedParameter4fNV (GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramNamedParameter4dNV (GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramNamedParameter4fvNV (GLuint id, GLsizei len, const GLubyte *name, const GLfloat *v)
{
	__OPENGL_NOT_IMPL__
}

void glProgramNamedParameter4dvNV (GLuint id, GLsizei len, const GLubyte *name, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramNamedParameterfvNV (GLuint id, GLsizei len, const GLubyte *name, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramNamedParameterdvNV (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glVertex2hNV (GLhalfNV x, GLhalfNV y)
{
	__OPENGL_NOT_IMPL__
}

void glVertex2hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertex3hNV (GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
	__OPENGL_NOT_IMPL__
}

void glVertex3hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertex4hNV (GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
	__OPENGL_NOT_IMPL__
}

void glVertex4hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glNormal3hNV (GLhalfNV nx, GLhalfNV ny, GLhalfNV nz)
{
	__OPENGL_NOT_IMPL__
}

void glNormal3hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glColor3hNV (GLhalfNV red, GLhalfNV green, GLhalfNV blue)
{
	__OPENGL_NOT_IMPL__
}

void glColor3hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glColor4hNV (GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha)
{
	__OPENGL_NOT_IMPL__
}

void glColor4hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord1hNV (GLhalfNV s)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord1hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2hNV (GLhalfNV s, GLhalfNV t)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord2hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord3hNV (GLhalfNV s, GLhalfNV t, GLhalfNV r)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord3hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord4hNV (GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoord4hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1hNV (GLenum target, GLhalfNV s)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord1hvNV (GLenum target, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2hNV (GLenum target, GLhalfNV s, GLhalfNV t)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord2hvNV (GLenum target, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3hNV (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord3hvNV (GLenum target, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4hNV (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoord4hvNV (GLenum target, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordhNV (GLhalfNV fog)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordhvNV (const GLhalfNV *fog)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3hNV (GLhalfNV red, GLhalfNV green, GLhalfNV blue)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColor3hvNV (const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexWeighthNV (GLhalfNV weight)
{
	__OPENGL_NOT_IMPL__
}

void glVertexWeighthvNV (const GLhalfNV *weight)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1hNV (GLuint index, GLhalfNV x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib1hvNV (GLuint index, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2hNV (GLuint index, GLhalfNV x, GLhalfNV y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib2hvNV (GLuint index, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3hNV (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib3hvNV (GLuint index, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4hNV (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttrib4hvNV (GLuint index, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs1hvNV (GLuint index, GLsizei n, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs2hvNV (GLuint index, GLsizei n, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs3hvNV (GLuint index, GLsizei n, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribs4hvNV (GLuint index, GLsizei n, const GLhalfNV *v)
{
	__OPENGL_NOT_IMPL__
}

void glPixelDataRangeNV (GLenum target, GLsizei length, GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glFlushPixelDataRangeNV (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glPrimitiveRestartNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glPrimitiveRestartIndexNV (GLuint index)
{
	__OPENGL_NOT_IMPL__
}

GLvoid* glMapObjectBufferATI (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glUnmapObjectBufferATI (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glStencilOpSeparateATI (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	__OPENGL_NOT_IMPL__
}

void glStencilFuncSeparateATI (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribArrayObjectATI (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribArrayObjectfvATI (GLuint index, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribArrayObjectivATI (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glDepthBoundsEXT (GLclampd zmin, GLclampd zmax)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationSeparateEXT (GLenum modeRGB, GLenum modeAlpha)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsRenderbufferEXT (GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindRenderbufferEXT (GLenum target, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteRenderbuffersEXT (GLsizei n, const GLuint *renderbuffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenRenderbuffersEXT (GLsizei n, GLuint *renderbuffers)
{
	__OPENGL_NOT_IMPL__
}

void glRenderbufferStorageEXT (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetRenderbufferParameterivEXT (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsFramebufferEXT (GLuint framebuffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBindFramebufferEXT (GLenum target, GLuint framebuffer)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteFramebuffersEXT (GLsizei n, const GLuint *framebuffers)
{
	__OPENGL_NOT_IMPL__
}

void glGenFramebuffersEXT (GLsizei n, GLuint *framebuffers)
{
	__OPENGL_NOT_IMPL__
}

GLenum glCheckFramebufferStatusEXT (GLenum target)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glFramebufferTexture1DEXT (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture2DEXT (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTexture3DEXT (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferRenderbufferEXT (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glGetFramebufferAttachmentParameterivEXT (GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGenerateMipmapEXT (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glStringMarkerGREMEDY (GLsizei len, const GLvoid *string)
{
	__OPENGL_NOT_IMPL__
}

void glStencilClearTagEXT (GLsizei stencilTagBits, GLuint stencilClearTag)
{
	__OPENGL_NOT_IMPL__
}

void glBlitFramebufferEXT (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	__OPENGL_NOT_IMPL__
}

void glRenderbufferStorageMultisampleEXT (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjecti64vEXT (GLuint id, GLenum pname, GLint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetQueryObjectui64vEXT (GLuint id, GLenum pname, GLuint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameters4fvEXT (GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameters4fvEXT (GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glBufferParameteriAPPLE (GLenum target, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glFlushMappedBufferRangeAPPLE (GLenum target, GLintptr offset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameterI4iNV (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameterI4ivNV (GLenum target, GLuint index, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParametersI4ivNV (GLenum target, GLuint index, GLsizei count, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameterI4uiNV (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParameterI4uivNV (GLenum target, GLuint index, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramLocalParametersI4uivNV (GLenum target, GLuint index, GLsizei count, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameterI4iNV (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameterI4ivNV (GLenum target, GLuint index, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParametersI4ivNV (GLenum target, GLuint index, GLsizei count, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameterI4uiNV (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParameterI4uivNV (GLenum target, GLuint index, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramEnvParametersI4uivNV (GLenum target, GLuint index, GLsizei count, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramLocalParameterIivNV (GLenum target, GLuint index, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramLocalParameterIuivNV (GLenum target, GLuint index, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramEnvParameterIivNV (GLenum target, GLuint index, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramEnvParameterIuivNV (GLenum target, GLuint index, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramVertexLimitNV (GLenum target, GLint limit)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTextureEXT (GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferTextureFaceEXT (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	__OPENGL_NOT_IMPL__
}

void glProgramParameteriEXT (GLuint program, GLenum pname, GLint value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1iEXT (GLuint index, GLint x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2iEXT (GLuint index, GLint x, GLint y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3iEXT (GLuint index, GLint x, GLint y, GLint z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4iEXT (GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1uiEXT (GLuint index, GLuint x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2uiEXT (GLuint index, GLuint x, GLuint y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3uiEXT (GLuint index, GLuint x, GLuint y, GLuint z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4uiEXT (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1ivEXT (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2ivEXT (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3ivEXT (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4ivEXT (GLuint index, const GLint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI1uivEXT (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI2uivEXT (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI3uivEXT (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4uivEXT (GLuint index, const GLuint *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4bvEXT (GLuint index, const GLbyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4svEXT (GLuint index, const GLshort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4ubvEXT (GLuint index, const GLubyte *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribI4usvEXT (GLuint index, const GLushort *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribIPointerEXT (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribIivEXT (GLuint index, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribIuivEXT (GLuint index, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformuivEXT (GLuint program, GLint location, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glBindFragDataLocationEXT (GLuint program, GLuint color, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetFragDataLocationEXT (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glUniform1uiEXT (GLint location, GLuint v0)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2uiEXT (GLint location, GLuint v0, GLuint v1)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3uiEXT (GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4uiEXT (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1uivEXT (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2uivEXT (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3uivEXT (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4uivEXT (GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glDrawArraysInstancedEXT (GLenum mode, GLint start, GLsizei count, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glDrawElementsInstancedEXT (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
	__OPENGL_NOT_IMPL__
}

void glTexBufferEXT (GLenum target, GLenum internalformat, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glDepthRangedNV (GLdouble zNear, GLdouble zFar)
{
	__OPENGL_NOT_IMPL__
}

void glClearDepthdNV (GLdouble depth)
{
	__OPENGL_NOT_IMPL__
}

void glDepthBoundsdNV (GLdouble zmin, GLdouble zmax)
{
	__OPENGL_NOT_IMPL__
}

void glRenderbufferStorageMultisampleCoverageNV (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glProgramBufferParametersfvNV (GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramBufferParametersIivNV (GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramBufferParametersIuivNV (GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glColorMaskIndexedEXT (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	__OPENGL_NOT_IMPL__
}

void glGetBooleanIndexedvEXT (GLenum target, GLuint index, GLboolean *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetIntegerIndexedvEXT (GLenum target, GLuint index, GLint *data)
{
	__OPENGL_NOT_IMPL__
}

void glEnableIndexedEXT (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glDisableIndexedEXT (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsEnabledIndexedEXT (GLenum target, GLuint index)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glBeginTransformFeedbackNV (GLenum primitiveMode)
{
	__OPENGL_NOT_IMPL__
}

void glEndTransformFeedbackNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glTransformFeedbackAttribsNV (GLuint count, const GLint *attribs, GLenum bufferMode)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferRangeNV (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferOffsetNV (GLenum target, GLuint index, GLuint buffer, GLintptr offset)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferBaseNV (GLenum target, GLuint index, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glTransformFeedbackVaryingsNV (GLuint program, GLsizei count, const GLint *locations, GLenum bufferMode)
{
	__OPENGL_NOT_IMPL__
}

void glActiveVaryingNV (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetVaryingLocationNV (GLuint program, const GLchar *name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetActiveVaryingNV (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glGetTransformFeedbackVaryingNV (GLuint program, GLuint index, GLint *location)
{
	__OPENGL_NOT_IMPL__
}

void glTransformFeedbackStreamAttribsNV (GLsizei count, const GLint *attribs, GLsizei nbuffers, const GLint *bufstreams, GLenum bufferMode)
{
	__OPENGL_NOT_IMPL__
}

void glUniformBufferEXT (GLuint program, GLint location, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

GLint glGetUniformBufferSizeEXT (GLuint program, GLint location)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLintptr glGetUniformOffsetEXT (GLuint program, GLint location)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glTexParameterIivEXT (GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glTexParameterIuivEXT (GLenum target, GLenum pname, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterIivEXT (GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterIuivEXT (GLenum target, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glClearColorIiEXT (GLint red, GLint green, GLint blue, GLint alpha)
{
	__OPENGL_NOT_IMPL__
}

void glClearColorIuiEXT (GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
	__OPENGL_NOT_IMPL__
}

void glFrameTerminatorGREMEDY (void)
{
	__OPENGL_NOT_IMPL__
}

void glBeginConditionalRenderNV (GLuint id, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glEndConditionalRenderNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glPresentFrameKeyedNV (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1)
{
	__OPENGL_NOT_IMPL__
}

void glPresentFrameDualFillNV (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideoivNV (GLuint video_slot, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideouivNV (GLuint video_slot, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideoi64vNV (GLuint video_slot, GLenum pname, GLint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideoui64vNV (GLuint video_slot, GLenum pname, GLuint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glBeginTransformFeedbackEXT (GLenum primitiveMode)
{
	__OPENGL_NOT_IMPL__
}

void glEndTransformFeedbackEXT (void)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferRangeEXT (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferOffsetEXT (GLenum target, GLuint index, GLuint buffer, GLintptr offset)
{
	__OPENGL_NOT_IMPL__
}

void glBindBufferBaseEXT (GLenum target, GLuint index, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glTransformFeedbackVaryingsEXT (GLuint program, GLsizei count, const GLchar* *varyings, GLenum bufferMode)
{
	__OPENGL_NOT_IMPL__
}

void glGetTransformFeedbackVaryingEXT (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
	__OPENGL_NOT_IMPL__
}

void glClientAttribDefaultEXT (GLbitfield mask)
{
	__OPENGL_NOT_IMPL__
}

void glPushClientAttribDefaultEXT (GLbitfield mask)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixLoadfEXT (GLenum mode, const GLfloat *m)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixLoaddEXT (GLenum mode, const GLdouble *m)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixMultfEXT (GLenum mode, const GLfloat *m)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixMultdEXT (GLenum mode, const GLdouble *m)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixLoadIdentityEXT (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixRotatefEXT (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixRotatedEXT (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixScalefEXT (GLenum mode, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixScaledEXT (GLenum mode, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixTranslatefEXT (GLenum mode, GLfloat x, GLfloat y, GLfloat z)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixTranslatedEXT (GLenum mode, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixFrustumEXT (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixOrthoEXT (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixPopEXT (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixPushEXT (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixLoadTransposefEXT (GLenum mode, const GLfloat *m)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixLoadTransposedEXT (GLenum mode, const GLdouble *m)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixMultTransposefEXT (GLenum mode, const GLfloat *m)
{
	__OPENGL_NOT_IMPL__
}

void glMatrixMultTransposedEXT (GLenum mode, const GLdouble *m)
{
	__OPENGL_NOT_IMPL__
}

void glTextureParameterfEXT (GLuint texture, GLenum target, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glTextureParameterfvEXT (GLuint texture, GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glTextureParameteriEXT (GLuint texture, GLenum target, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glTextureParameterivEXT (GLuint texture, GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glTextureImage1DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTextureImage2DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTextureSubImage1DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTextureSubImage2DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTextureImage1DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTextureImage2DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTextureSubImage1DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTextureSubImage2DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetTextureImageEXT (GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glGetTextureParameterfvEXT (GLuint texture, GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTextureParameterivEXT (GLuint texture, GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTextureLevelParameterfvEXT (GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTextureLevelParameterivEXT (GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glTextureImage3DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glTextureSubImage3DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glCopyTextureSubImage3DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexParameterfEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexParameterfvEXT (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexParameteriEXT (GLenum texunit, GLenum target, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexParameterivEXT (GLenum texunit, GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexImage1DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexImage2DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexSubImage1DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexSubImage2DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glCopyMultiTexImage1DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	__OPENGL_NOT_IMPL__
}

void glCopyMultiTexImage2DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	__OPENGL_NOT_IMPL__
}

void glCopyMultiTexSubImage1DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	__OPENGL_NOT_IMPL__
}

void glCopyMultiTexSubImage2DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexImageEXT (GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexParameterfvEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexParameterivEXT (GLenum texunit, GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexLevelParameterfvEXT (GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexLevelParameterivEXT (GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexImage3DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexSubImage3DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
	__OPENGL_NOT_IMPL__
}

void glCopyMultiTexSubImage3DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glBindMultiTextureEXT (GLenum texunit, GLenum target, GLuint texture)
{
	__OPENGL_NOT_IMPL__
}

void glEnableClientStateIndexedEXT (GLenum array, GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glDisableClientStateIndexedEXT (GLenum array, GLuint index)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexCoordPointerEXT (GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexEnvfEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexEnvfvEXT (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexEnviEXT (GLenum texunit, GLenum target, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexEnvivEXT (GLenum texunit, GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexGendEXT (GLenum texunit, GLenum coord, GLenum pname, GLdouble param)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexGendvEXT (GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexGenfEXT (GLenum texunit, GLenum coord, GLenum pname, GLfloat param)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexGenfvEXT (GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexGeniEXT (GLenum texunit, GLenum coord, GLenum pname, GLint param)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexGenivEXT (GLenum texunit, GLenum coord, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexEnvfvEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexEnvivEXT (GLenum texunit, GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexGendvEXT (GLenum texunit, GLenum coord, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexGenfvEXT (GLenum texunit, GLenum coord, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexGenivEXT (GLenum texunit, GLenum coord, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetFloatIndexedvEXT (GLenum target, GLuint index, GLfloat *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetDoubleIndexedvEXT (GLenum target, GLuint index, GLdouble *data)
{
	__OPENGL_NOT_IMPL__
}

void glGetPointerIndexedvEXT (GLenum target, GLuint index, GLvoid* *data)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTextureImage3DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTextureImage2DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTextureImage1DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTextureSubImage3DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTextureSubImage2DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedTextureSubImage1DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glGetCompressedTextureImageEXT (GLuint texture, GLenum target, GLint lod, GLvoid *img)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedMultiTexImage3DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedMultiTexImage2DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedMultiTexImage1DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedMultiTexSubImage3DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedMultiTexSubImage2DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glCompressedMultiTexSubImage1DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
	__OPENGL_NOT_IMPL__
}

void glGetCompressedMultiTexImageEXT (GLenum texunit, GLenum target, GLint lod, GLvoid *img)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramStringEXT (GLuint program, GLenum target, GLenum format, GLsizei len, const GLvoid *string)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameter4dEXT (GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameter4dvEXT (GLuint program, GLenum target, GLuint index, const GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameter4fEXT (GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameter4fvEXT (GLuint program, GLenum target, GLuint index, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedProgramLocalParameterdvEXT (GLuint program, GLenum target, GLuint index, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedProgramLocalParameterfvEXT (GLuint program, GLenum target, GLuint index, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedProgramivEXT (GLuint program, GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedProgramStringEXT (GLuint program, GLenum target, GLenum pname, GLvoid *string)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameters4fvEXT (GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameterI4iEXT (GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameterI4ivEXT (GLuint program, GLenum target, GLuint index, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParametersI4ivEXT (GLuint program, GLenum target, GLuint index, GLsizei count, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameterI4uiEXT (GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParameterI4uivEXT (GLuint program, GLenum target, GLuint index, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glNamedProgramLocalParametersI4uivEXT (GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedProgramLocalParameterIivEXT (GLuint program, GLenum target, GLuint index, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedProgramLocalParameterIuivEXT (GLuint program, GLenum target, GLuint index, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glTextureParameterIivEXT (GLuint texture, GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glTextureParameterIuivEXT (GLuint texture, GLenum target, GLenum pname, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTextureParameterIivEXT (GLuint texture, GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetTextureParameterIuivEXT (GLuint texture, GLenum target, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexParameterIivEXT (GLenum texunit, GLenum target, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexParameterIuivEXT (GLenum texunit, GLenum target, GLenum pname, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexParameterIivEXT (GLenum texunit, GLenum target, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultiTexParameterIuivEXT (GLenum texunit, GLenum target, GLenum pname, GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1fEXT (GLuint program, GLint location, GLfloat v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2fEXT (GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3fEXT (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4fEXT (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1iEXT (GLuint program, GLint location, GLint v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2iEXT (GLuint program, GLint location, GLint v0, GLint v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3iEXT (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4iEXT (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x3fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x2fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x4fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x2fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x4fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x3fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1uiEXT (GLuint program, GLint location, GLuint v0)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2uiEXT (GLuint program, GLint location, GLuint v0, GLuint v1)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3uiEXT (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4uiEXT (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	__OPENGL_NOT_IMPL__
}

void glNamedBufferDataEXT (GLuint buffer, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
	__OPENGL_NOT_IMPL__
}

void glNamedBufferSubDataEXT (GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

GLvoid* glMapNamedBufferEXT (GLuint buffer, GLenum access)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLboolean glUnmapNamedBufferEXT (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLvoid* glMapNamedBufferRangeEXT (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glFlushMappedNamedBufferRangeEXT (GLuint buffer, GLintptr offset, GLsizeiptr length)
{
	__OPENGL_NOT_IMPL__
}

void glNamedCopyBufferSubDataEXT (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedBufferParameterivEXT (GLuint buffer, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedBufferPointervEXT (GLuint buffer, GLenum pname, GLvoid* *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedBufferSubDataEXT (GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glTextureBufferEXT (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexBufferEXT (GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

void glNamedRenderbufferStorageEXT (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedRenderbufferParameterivEXT (GLuint renderbuffer, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

GLenum glCheckNamedFramebufferStatusEXT (GLuint framebuffer, GLenum target)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glNamedFramebufferTexture1DEXT (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glNamedFramebufferTexture2DEXT (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glNamedFramebufferTexture3DEXT (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	__OPENGL_NOT_IMPL__
}

void glNamedFramebufferRenderbufferEXT (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedFramebufferAttachmentParameterivEXT (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGenerateTextureMipmapEXT (GLuint texture, GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glGenerateMultiTexMipmapEXT (GLenum texunit, GLenum target)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferDrawBufferEXT (GLuint framebuffer, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferDrawBuffersEXT (GLuint framebuffer, GLsizei n, const GLenum *bufs)
{
	__OPENGL_NOT_IMPL__
}

void glFramebufferReadBufferEXT (GLuint framebuffer, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glGetFramebufferParameterivEXT (GLuint framebuffer, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glNamedRenderbufferStorageMultisampleEXT (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glNamedRenderbufferStorageMultisampleCoverageEXT (GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__OPENGL_NOT_IMPL__
}

void glNamedFramebufferTextureEXT (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
	__OPENGL_NOT_IMPL__
}

void glNamedFramebufferTextureLayerEXT (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__OPENGL_NOT_IMPL__
}

void glNamedFramebufferTextureFaceEXT (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	__OPENGL_NOT_IMPL__
}

void glTextureRenderbufferEXT (GLuint texture, GLenum target, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glMultiTexRenderbufferEXT (GLenum texunit, GLenum target, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1dEXT (GLuint program, GLint location, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2dEXT (GLuint program, GLint location, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3dEXT (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4dEXT (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x3dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix2x4dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x2dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix3x4dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x2dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformMatrix4x3dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	__OPENGL_NOT_IMPL__
}

void glGetMultisamplefvNV (GLenum pname, GLuint index, GLfloat *val)
{
	__OPENGL_NOT_IMPL__
}

void glSampleMaskIndexedNV (GLuint index, GLbitfield mask)
{
	__OPENGL_NOT_IMPL__
}

void glTexRenderbufferNV (GLenum target, GLuint renderbuffer)
{
	__OPENGL_NOT_IMPL__
}

void glBindTransformFeedbackNV (GLenum target, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteTransformFeedbacksNV (GLsizei n, const GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

void glGenTransformFeedbacksNV (GLsizei n, GLuint *ids)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsTransformFeedbackNV (GLuint id)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glPauseTransformFeedbackNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glResumeTransformFeedbackNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glDrawTransformFeedbackNV (GLenum mode, GLuint id)
{
	__OPENGL_NOT_IMPL__
}

void glGetPerfMonitorGroupsAMD (GLint *numGroups, GLsizei groupsSize, GLuint *groups)
{
	__OPENGL_NOT_IMPL__
}

void glGetPerfMonitorCountersAMD (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters)
{
	__OPENGL_NOT_IMPL__
}

void glGetPerfMonitorGroupStringAMD (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString)
{
	__OPENGL_NOT_IMPL__
}

void glGetPerfMonitorCounterStringAMD (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString)
{
	__OPENGL_NOT_IMPL__
}

void glGetPerfMonitorCounterInfoAMD (GLuint group, GLuint counter, GLenum pname, GLvoid *data)
{
	__OPENGL_NOT_IMPL__
}

void glGenPerfMonitorsAMD (GLsizei n, GLuint *monitors)
{
	__OPENGL_NOT_IMPL__
}

void glDeletePerfMonitorsAMD (GLsizei n, GLuint *monitors)
{
	__OPENGL_NOT_IMPL__
}

void glSelectPerfMonitorCountersAMD (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *counterList)
{
	__OPENGL_NOT_IMPL__
}

void glBeginPerfMonitorAMD (GLuint monitor)
{
	__OPENGL_NOT_IMPL__
}

void glEndPerfMonitorAMD (GLuint monitor)
{
	__OPENGL_NOT_IMPL__
}

void glGetPerfMonitorCounterDataAMD (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten)
{
	__OPENGL_NOT_IMPL__
}

void glTessellationFactorAMD (GLfloat factor)
{
	__OPENGL_NOT_IMPL__
}

void glTessellationModeAMD (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glProvokingVertexEXT (GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncIndexedAMD (GLuint buf, GLenum src, GLenum dst)
{
	__OPENGL_NOT_IMPL__
}

void glBlendFuncSeparateIndexedAMD (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationIndexedAMD (GLuint buf, GLenum mode)
{
	__OPENGL_NOT_IMPL__
}

void glBlendEquationSeparateIndexedAMD (GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	__OPENGL_NOT_IMPL__
}

void glTextureRangeAPPLE (GLenum target, GLsizei length, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glGetTexParameterPointervAPPLE (GLenum target, GLenum pname, GLvoid* *params)
{
	__OPENGL_NOT_IMPL__
}

void glEnableVertexAttribAPPLE (GLuint index, GLenum pname)
{
	__OPENGL_NOT_IMPL__
}

void glDisableVertexAttribAPPLE (GLuint index, GLenum pname)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsVertexAttribEnabledAPPLE (GLuint index, GLenum pname)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glMapVertexAttrib1dAPPLE (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
	__OPENGL_NOT_IMPL__
}

void glMapVertexAttrib1fAPPLE (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

void glMapVertexAttrib2dAPPLE (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
	__OPENGL_NOT_IMPL__
}

void glMapVertexAttrib2fAPPLE (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
	__OPENGL_NOT_IMPL__
}

GLenum glObjectPurgeableAPPLE (GLenum objectType, GLuint name, GLenum option)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLenum glObjectUnpurgeableAPPLE (GLenum objectType, GLuint name, GLenum option)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetObjectParameterivAPPLE (GLenum objectType, GLuint name, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glBeginVideoCaptureNV (GLuint video_capture_slot)
{
	__OPENGL_NOT_IMPL__
}

void glBindVideoCaptureStreamBufferNV (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset)
{
	__OPENGL_NOT_IMPL__
}

void glBindVideoCaptureStreamTextureNV (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture)
{
	__OPENGL_NOT_IMPL__
}

void glEndVideoCaptureNV (GLuint video_capture_slot)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideoCaptureivNV (GLuint video_capture_slot, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideoCaptureStreamivNV (GLuint video_capture_slot, GLuint stream, GLenum pname, GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideoCaptureStreamfvNV (GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVideoCaptureStreamdvNV (GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

GLenum glVideoCaptureNV (GLuint video_capture_slot, GLuint *sequence_num, GLuint64EXT *capture_time)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glVideoCaptureStreamParameterivNV (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint *params)
{
	__OPENGL_NOT_IMPL__
}

void glVideoCaptureStreamParameterfvNV (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat *params)
{
	__OPENGL_NOT_IMPL__
}

void glVideoCaptureStreamParameterdvNV (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glCopyImageSubDataNV (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth)
{
	__OPENGL_NOT_IMPL__
}

void glUseShaderProgramEXT (GLenum type, GLuint program)
{
	__OPENGL_NOT_IMPL__
}

void glActiveProgramEXT (GLuint program)
{
	__OPENGL_NOT_IMPL__
}

GLuint glCreateShaderProgramEXT (GLenum type, const GLchar *string)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glMakeBufferResidentNV (GLenum target, GLenum access)
{
	__OPENGL_NOT_IMPL__
}

void glMakeBufferNonResidentNV (GLenum target)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsBufferResidentNV (GLenum target)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glMakeNamedBufferResidentNV (GLuint buffer, GLenum access)
{
	__OPENGL_NOT_IMPL__
}

void glMakeNamedBufferNonResidentNV (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsNamedBufferResidentNV (GLuint buffer)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glGetBufferParameterui64vNV (GLenum target, GLenum pname, GLuint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetNamedBufferParameterui64vNV (GLuint buffer, GLenum pname, GLuint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetIntegerui64vNV (GLenum value, GLuint64EXT *result)
{
	__OPENGL_NOT_IMPL__
}

void glUniformui64NV (GLint location, GLuint64EXT value)
{
	__OPENGL_NOT_IMPL__
}

void glUniformui64vNV (GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformui64vNV (GLuint program, GLint location, GLuint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformui64NV (GLuint program, GLint location, GLuint64EXT value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniformui64vNV (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glBufferAddressRangeNV (GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length)
{
	__OPENGL_NOT_IMPL__
}

void glVertexFormatNV (GLint size, GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glNormalFormatNV (GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glColorFormatNV (GLint size, GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glIndexFormatNV (GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glTexCoordFormatNV (GLint size, GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glEdgeFlagFormatNV (GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glSecondaryColorFormatNV (GLint size, GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glFogCoordFormatNV (GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribFormatNV (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribIFormatNV (GLuint index, GLint size, GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glGetIntegerui64i_vNV (GLenum value, GLuint index, GLuint64EXT *result)
{
	__OPENGL_NOT_IMPL__
}

void glTextureBarrierNV (void)
{
	__OPENGL_NOT_IMPL__
}

void glBindImageTextureEXT (GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format)
{
	__OPENGL_NOT_IMPL__
}

void glMemoryBarrierEXT (GLbitfield barriers)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1dEXT (GLuint index, GLdouble x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2dEXT (GLuint index, GLdouble x, GLdouble y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3dEXT (GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4dEXT (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1dvEXT (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2dvEXT (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3dvEXT (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4dvEXT (GLuint index, const GLdouble *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribLPointerEXT (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribLdvEXT (GLuint index, GLenum pname, GLdouble *params)
{
	__OPENGL_NOT_IMPL__
}

void glVertexArrayVertexAttribLOffsetEXT (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__OPENGL_NOT_IMPL__
}

void glProgramSubroutineParametersuivNV (GLenum target, GLsizei count, const GLuint *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetProgramSubroutineParameteruivNV (GLenum target, GLuint index, GLuint *param)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1i64NV (GLint location, GLint64EXT x)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2i64NV (GLint location, GLint64EXT x, GLint64EXT y)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3i64NV (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4i64NV (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1i64vNV (GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2i64vNV (GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3i64vNV (GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4i64vNV (GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1ui64NV (GLint location, GLuint64EXT x)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2ui64NV (GLint location, GLuint64EXT x, GLuint64EXT y)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3ui64NV (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4ui64NV (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	__OPENGL_NOT_IMPL__
}

void glUniform1ui64vNV (GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform2ui64vNV (GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform3ui64vNV (GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glUniform4ui64vNV (GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glGetUniformi64vNV (GLuint program, GLint location, GLint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1i64NV (GLuint program, GLint location, GLint64EXT x)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2i64NV (GLuint program, GLint location, GLint64EXT x, GLint64EXT y)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3i64NV (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4i64NV (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1i64vNV (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2i64vNV (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3i64vNV (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4i64vNV (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1ui64NV (GLuint program, GLint location, GLuint64EXT x)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2ui64NV (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3ui64NV (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4ui64NV (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform1ui64vNV (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform2ui64vNV (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform3ui64vNV (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glProgramUniform4ui64vNV (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1i64NV (GLuint index, GLint64EXT x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2i64NV (GLuint index, GLint64EXT x, GLint64EXT y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3i64NV (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4i64NV (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1i64vNV (GLuint index, const GLint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2i64vNV (GLuint index, const GLint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3i64vNV (GLuint index, const GLint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4i64vNV (GLuint index, const GLint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1ui64NV (GLuint index, GLuint64EXT x)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2ui64NV (GLuint index, GLuint64EXT x, GLuint64EXT y)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3ui64NV (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4ui64NV (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL1ui64vNV (GLuint index, const GLuint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL2ui64vNV (GLuint index, const GLuint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL3ui64vNV (GLuint index, const GLuint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribL4ui64vNV (GLuint index, const GLuint64EXT *v)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribLi64vNV (GLuint index, GLenum pname, GLint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glGetVertexAttribLui64vNV (GLuint index, GLenum pname, GLuint64EXT *params)
{
	__OPENGL_NOT_IMPL__
}

void glVertexAttribLFormatNV (GLuint index, GLint size, GLenum type, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glGenNamesAMD (GLenum identifier, GLuint num, GLuint *names)
{
	__OPENGL_NOT_IMPL__
}

void glDeleteNamesAMD (GLenum identifier, GLuint num, const GLuint *names)
{
	__OPENGL_NOT_IMPL__
}

GLboolean glIsNameAMD (GLenum identifier, GLuint name)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glDebugMessageEnableAMD (GLenum category, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)
{
	__OPENGL_NOT_IMPL__
}

void glDebugMessageInsertAMD (GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar *buf)
{
	__OPENGL_NOT_IMPL__
}

void glDebugMessageCallbackAMD (GLDEBUGPROCAMD callback, GLvoid *userParam)
{
	__OPENGL_NOT_IMPL__
}

GLuint glGetDebugMessageLogAMD (GLuint count, GLsizei bufsize, GLenum *categories, GLuint *severities, GLuint *ids, GLsizei *lengths, GLchar *message)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glVDPAUInitNV (const GLvoid *vdpDevice, const GLvoid *getProcAddress)
{
	__OPENGL_NOT_IMPL__
}

void glVDPAUFiniNV (void)
{
	__OPENGL_NOT_IMPL__
}

GLvdpauSurfaceNV glVDPAURegisterVideoSurfaceNV (GLvoid *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

GLvdpauSurfaceNV glVDPAURegisterOutputSurfaceNV (GLvoid *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glVDPAUIsSurfaceNV (GLvdpauSurfaceNV surface)
{
	__OPENGL_NOT_IMPL__
}

void glVDPAUUnregisterSurfaceNV (GLvdpauSurfaceNV surface)
{
	__OPENGL_NOT_IMPL__
}

void glVDPAUGetSurfaceivNV (GLvdpauSurfaceNV surface, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
	__OPENGL_NOT_IMPL__
}

void glVDPAUSurfaceAccessNV (GLvdpauSurfaceNV surface, GLenum access)
{
	__OPENGL_NOT_IMPL__
}

void glVDPAUMapSurfacesNV (GLsizei numSurfaces, const GLvdpauSurfaceNV *surfaces)
{
	__OPENGL_NOT_IMPL__
}

void glVDPAUUnmapSurfacesNV (GLsizei numSurface, const GLvdpauSurfaceNV *surfaces)
{
	__OPENGL_NOT_IMPL__
}

void glTexImage2DMultisampleCoverageNV (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
	__OPENGL_NOT_IMPL__
}

void glTexImage3DMultisampleCoverageNV (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
	__OPENGL_NOT_IMPL__
}

void glTextureImage2DMultisampleNV (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
	__OPENGL_NOT_IMPL__
}

void glTextureImage3DMultisampleNV (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
	__OPENGL_NOT_IMPL__
}

void glTextureImage2DMultisampleCoverageNV (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
	__OPENGL_NOT_IMPL__
}

void glTextureImage3DMultisampleCoverageNV (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
	__OPENGL_NOT_IMPL__
}

void glSetMultisamplefvAMD (GLenum pname, GLuint index, const GLfloat *val)
{
	__OPENGL_NOT_IMPL__
}

GLsync glImportSyncEXT (GLenum external_sync_type, GLintptr external_sync, GLbitfield flags)
{
	__OPENGL_NOT_IMPL__
	return 0;
}

void glMultiDrawArraysIndirectAMD (GLenum mode, const GLvoid *indirect, GLsizei primcount, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}

void glMultiDrawElementsIndirectAMD (GLenum mode, GLenum type, const GLvoid *indirect, GLsizei primcount, GLsizei stride)
{
	__OPENGL_NOT_IMPL__
}




