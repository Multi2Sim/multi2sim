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

#include <m2s-opengl.h>

void glClearIndex( GLfloat c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClearIndex, sys_args);
}


void glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClearColor, sys_args);
}


void glClear( GLbitfield mask )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClear, sys_args);
}


void glIndexMask( GLuint mask )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexMask, sys_args);
}


void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColorMask, sys_args);
}


void glAlphaFunc( GLenum func, GLclampf ref )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) func;
	sys_args[1] = (unsigned int) ref;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glAlphaFunc, sys_args);
}


void glBlendFunc( GLenum sfactor, GLenum dfactor )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) sfactor;
	sys_args[1] = (unsigned int) dfactor;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glBlendFunc, sys_args);
}


void glLogicOp( GLenum opcode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) opcode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLogicOp, sys_args);
}


void glCullFace( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCullFace, sys_args);
}


void glFrontFace( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFrontFace, sys_args);
}


void glPointSize( GLfloat size )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) size;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPointSize, sys_args);
}


void glLineWidth( GLfloat width )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) width;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLineWidth, sys_args);
}


void glLineStipple( GLint factor, GLushort pattern )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) factor;
	sys_args[1] = (unsigned int) pattern;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLineStipple, sys_args);
}


void glPolygonMode( GLenum face, GLenum mode )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPolygonMode, sys_args);
}


void glPolygonOffset( GLfloat factor, GLfloat units )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) factor;
	sys_args[1] = (unsigned int) units;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPolygonOffset, sys_args);
}


void glPolygonStipple( const GLubyte *mask )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPolygonStipple, sys_args);
}


void glGetPolygonStipple( GLubyte *mask )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetPolygonStipple, sys_args);
}


void glEdgeFlag( GLboolean flag )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) flag;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEdgeFlag, sys_args);
}


void glEdgeFlagv( const GLboolean *flag )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) flag;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEdgeFlagv, sys_args);
}


void glScissor( GLint x, GLint y, GLsizei width, GLsizei height)

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) height;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glScissor, sys_args);
}


void glClipPlane( GLenum plane, const GLdouble *equation )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) plane;
	sys_args[1] = (unsigned int) equation;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClipPlane, sys_args);
}


void glGetClipPlane( GLenum plane, GLdouble *equation )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) plane;
	sys_args[1] = (unsigned int) equation;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetClipPlane, sys_args);
}


void glDrawBuffer( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDrawBuffer, sys_args);
}


void glReadBuffer( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glReadBuffer, sys_args);
}


void glEnable( GLenum cap )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) cap;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEnable, sys_args);
}


void glDisable( GLenum cap )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) cap;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDisable, sys_args);
}


GLboolean glIsEnabled( GLenum cap )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) cap;
	return (GLboolean) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIsEnabled, sys_args);
}


void glEnableClientState( GLenum cap )  /* 1.1 */

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) cap;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEnableClientState, sys_args);
}


void glDisableClientState( GLenum cap )  /* 1.1 */

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) cap;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDisableClientState, sys_args);
}


void glGetBooleanv( GLenum pname, GLboolean *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetBooleanv, sys_args);
}


void glGetDoublev( GLenum pname, GLdouble *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetDoublev, sys_args);
}


void glGetFloatv( GLenum pname, GLfloat *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetFloatv, sys_args);
}


void glGetIntegerv( GLenum pname, GLint *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetIntegerv, sys_args);
}


void glPushAttrib( GLbitfield mask )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPushAttrib, sys_args);
}


void glPopAttrib( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPopAttrib, sys_args);
}


void glPushClientAttrib( GLbitfield mask )  /* 1.1 */

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPushClientAttrib, sys_args);
}


void glPopClientAttrib( void )  /* 1.1 */

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPopClientAttrib, sys_args);
}


GLint glRenderMode( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	return (GLint) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRenderMode, sys_args);
}


GLenum glGetError( void )

{
	unsigned int sys_args[1];
	return (GLenum) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetError, sys_args);
}


const GLubyte * glGetString( GLenum name )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) name;
	return (const GLubyte*) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetString, sys_args);
}


void glFinish( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFinish, sys_args);
}


void glFlush( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFlush, sys_args);
}


void glHint( GLenum target, GLenum mode )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glHint, sys_args);
}


void glClearDepth( GLclampd depth )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) depth;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClearDepth, sys_args);
}


void glDepthFunc( GLenum func )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) func;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDepthFunc, sys_args);
}


void glDepthMask( GLboolean flag )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) flag;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDepthMask, sys_args);
}


void glDepthRange( GLclampd near_val, GLclampd far_val )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) near_val;
	sys_args[1] = (unsigned int) far_val;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDepthRange, sys_args);
}


void glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClearAccum, sys_args);
}


void glAccum( GLenum op, GLfloat value )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) op;
	sys_args[1] = (unsigned int) value;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glAccum, sys_args);
}


void glMatrixMode( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMatrixMode, sys_args);
}


void glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) left;
	sys_args[1] = (unsigned int) right;
	sys_args[2] = (unsigned int) bottom;
	sys_args[3] = (unsigned int) top;
	sys_args[4] = (unsigned int) near_val;
	sys_args[5] = (unsigned int) far_val;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glOrtho, sys_args);
}


void glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_val, GLdouble far_val )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) left;
	sys_args[1] = (unsigned int) right;
	sys_args[2] = (unsigned int) bottom;
	sys_args[3] = (unsigned int) top;
	sys_args[4] = (unsigned int) near_val;
	sys_args[5] = (unsigned int) far_val;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFrustum, sys_args);
}


void glViewport( GLint x, GLint y,
                                    GLsizei width, GLsizei height )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) height;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glViewport, sys_args);
}


void glPushMatrix( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPushMatrix, sys_args);
}


void glPopMatrix( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPopMatrix, sys_args);
}


void glLoadIdentity( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLoadIdentity, sys_args);
}


void glLoadMatrixd( const GLdouble *m )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLoadMatrixd, sys_args);
}


void glLoadMatrixf( const GLfloat *m )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLoadMatrixf, sys_args);
}


void glMultMatrixd( const GLdouble *m )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultMatrixd, sys_args);
}


void glMultMatrixf( const GLfloat *m )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultMatrixf, sys_args);
}


void glRotated( GLdouble angle,
                                   GLdouble x, GLdouble y, GLdouble z )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) angle;
	sys_args[1] = (unsigned int) x;
	sys_args[2] = (unsigned int) y;
	sys_args[3] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRotated, sys_args);
}


void glRotatef( GLfloat angle,
                                   GLfloat x, GLfloat y, GLfloat z )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) angle;
	sys_args[1] = (unsigned int) x;
	sys_args[2] = (unsigned int) y;
	sys_args[3] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRotatef, sys_args);
}


void glScaled( GLdouble x, GLdouble y, GLdouble z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glScaled, sys_args);
}


void glScalef( GLfloat x, GLfloat y, GLfloat z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glScalef, sys_args);
}


void glTranslated( GLdouble x, GLdouble y, GLdouble z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTranslated, sys_args);
}


void glTranslatef( GLfloat x, GLfloat y, GLfloat z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTranslatef, sys_args);
}


GLboolean glIsList( GLuint list )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) list;
	return (GLboolean) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIsList, sys_args);
}


void glDeleteLists( GLuint list, GLsizei range )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) list;
	sys_args[1] = (unsigned int) range;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDeleteLists, sys_args);
}


GLuint glGenLists( GLsizei range )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) range;
	return (GLuint) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGenLists, sys_args);
}


void glNewList( GLuint list, GLenum mode )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) list;
	sys_args[1] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNewList, sys_args);
}


void glEndList( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEndList, sys_args);
}


void glCallList( GLuint list )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) list;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCallList, sys_args);
}


void glCallLists( GLsizei n, GLenum type,
                                     const GLvoid *lists )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) n;
	sys_args[1] = (unsigned int) type;
	sys_args[2] = (unsigned int) lists;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCallLists, sys_args);
}


void glListBase( GLuint base )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) base;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glListBase, sys_args);
}


void glBegin( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glBegin, sys_args);
}


void glEnd( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEnd, sys_args);
}


void glVertex2d( GLdouble x, GLdouble y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2d, sys_args);
}


void glVertex2f( GLfloat x, GLfloat y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2f, sys_args);
}


void glVertex2i( GLint x, GLint y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2i, sys_args);
}


void glVertex2s( GLshort x, GLshort y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2s, sys_args);
}


void glVertex3d( GLdouble x, GLdouble y, GLdouble z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3d, sys_args);
}


void glVertex3f( GLfloat x, GLfloat y, GLfloat z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3f, sys_args);
}


void glVertex3i( GLint x, GLint y, GLint z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3i, sys_args);
}


void glVertex3s( GLshort x, GLshort y, GLshort z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3s, sys_args);
}


void glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4d, sys_args);
}


void glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4f, sys_args);
}


void glVertex4i( GLint x, GLint y, GLint z, GLint w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4i, sys_args);
}


void glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4s, sys_args);
}


void glVertex2dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2dv, sys_args);
}


void glVertex2fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2fv, sys_args);
}


void glVertex2iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2iv, sys_args);
}


void glVertex2sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex2sv, sys_args);
}


void glVertex3dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3dv, sys_args);
}


void glVertex3fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3fv, sys_args);
}


void glVertex3iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3iv, sys_args);
}


void glVertex3sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex3sv, sys_args);
}


void glVertex4dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4dv, sys_args);
}


void glVertex4fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4fv, sys_args);
}


void glVertex4iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4iv, sys_args);
}


void glVertex4sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertex4sv, sys_args);
}


void glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) nx;
	sys_args[1] = (unsigned int) ny;
	sys_args[2] = (unsigned int) nz;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3b, sys_args);
}


void glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) nx;
	sys_args[1] = (unsigned int) ny;
	sys_args[2] = (unsigned int) nz;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3d, sys_args);
}


void glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) nx;
	sys_args[1] = (unsigned int) ny;
	sys_args[2] = (unsigned int) nz;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3f, sys_args);
}


void glNormal3i( GLint nx, GLint ny, GLint nz )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) nx;
	sys_args[1] = (unsigned int) ny;
	sys_args[2] = (unsigned int) nz;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3i, sys_args);
}


void glNormal3s( GLshort nx, GLshort ny, GLshort nz )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) nx;
	sys_args[1] = (unsigned int) ny;
	sys_args[2] = (unsigned int) nz;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3s, sys_args);
}


void glNormal3bv( const GLbyte *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3bv, sys_args);
}


void glNormal3dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3dv, sys_args);
}


void glNormal3fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3fv, sys_args);
}


void glNormal3iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3iv, sys_args);
}


void glNormal3sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormal3sv, sys_args);
}


void glIndexd( GLdouble c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexd, sys_args);
}


void glIndexf( GLfloat c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexf, sys_args);
}


void glIndexi( GLint c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexi, sys_args);
}


void glIndexs( GLshort c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexs, sys_args);
}


void glIndexub( GLubyte c )  /* 1.1 */

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexub, sys_args);
}


void glIndexdv( const GLdouble *c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexdv, sys_args);
}


void glIndexfv( const GLfloat *c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexfv, sys_args);
}


void glIndexiv( const GLint *c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexiv, sys_args);
}


void glIndexsv( const GLshort *c )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexsv, sys_args);
}


void glIndexubv( const GLubyte *c )  /* 1.1 */

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) c;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexubv, sys_args);
}


void glColor3b( GLbyte red, GLbyte green, GLbyte blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3b, sys_args);
}


void glColor3d( GLdouble red, GLdouble green, GLdouble blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3d, sys_args);
}


void glColor3f( GLfloat red, GLfloat green, GLfloat blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3f, sys_args);
}


void glColor3i( GLint red, GLint green, GLint blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3i, sys_args);
}


void glColor3s( GLshort red, GLshort green, GLshort blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3s, sys_args);
}


void glColor3ub( GLubyte red, GLubyte green, GLubyte blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3ub, sys_args);
}


void glColor3ui( GLuint red, GLuint green, GLuint blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3ui, sys_args);
}


void glColor3us( GLushort red, GLushort green, GLushort blue )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3us, sys_args);
}


void glColor4b( GLbyte red, GLbyte green,
                                   GLbyte blue, GLbyte alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4b, sys_args);
}


void glColor4d( GLdouble red, GLdouble green,
                                   GLdouble blue, GLdouble alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4d, sys_args);
}


void glColor4f( GLfloat red, GLfloat green,
                                   GLfloat blue, GLfloat alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4f, sys_args);
}


void glColor4i( GLint red, GLint green,
                                   GLint blue, GLint alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4i, sys_args);
}


void glColor4s( GLshort red, GLshort green,
                                   GLshort blue, GLshort alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4s, sys_args);
}


void glColor4ub( GLubyte red, GLubyte green,
                                    GLubyte blue, GLubyte alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4ub, sys_args);
}


void glColor4ui( GLuint red, GLuint green,
                                    GLuint blue, GLuint alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4ui, sys_args);
}


void glColor4us( GLushort red, GLushort green,
                                    GLushort blue, GLushort alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4us, sys_args);
}


void glColor3bv( const GLbyte *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3bv, sys_args);
}


void glColor3dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3dv, sys_args);
}


void glColor3fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3fv, sys_args);
}


void glColor3iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3iv, sys_args);
}


void glColor3sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3sv, sys_args);
}


void glColor3ubv( const GLubyte *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3ubv, sys_args);
}


void glColor3uiv( const GLuint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3uiv, sys_args);
}


void glColor3usv( const GLushort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor3usv, sys_args);
}


void glColor4bv( const GLbyte *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4bv, sys_args);
}


void glColor4dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4dv, sys_args);
}


void glColor4fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4fv, sys_args);
}


void glColor4iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4iv, sys_args);
}


void glColor4sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4sv, sys_args);
}


void glColor4ubv( const GLubyte *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4ubv, sys_args);
}


void glColor4uiv( const GLuint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4uiv, sys_args);
}


void glColor4usv( const GLushort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColor4usv, sys_args);
}


void glTexCoord1d( GLdouble s )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1d, sys_args);
}


void glTexCoord1f( GLfloat s )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1f, sys_args);
}


void glTexCoord1i( GLint s )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1i, sys_args);
}


void glTexCoord1s( GLshort s )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1s, sys_args);
}


void glTexCoord2d( GLdouble s, GLdouble t )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2d, sys_args);
}


void glTexCoord2f( GLfloat s, GLfloat t )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2f, sys_args);
}


void glTexCoord2i( GLint s, GLint t )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2i, sys_args);
}


void glTexCoord2s( GLshort s, GLshort t )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2s, sys_args);
}


void glTexCoord3d( GLdouble s, GLdouble t, GLdouble r )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3d, sys_args);
}


void glTexCoord3f( GLfloat s, GLfloat t, GLfloat r )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3f, sys_args);
}


void glTexCoord3i( GLint s, GLint t, GLint r )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3i, sys_args);
}


void glTexCoord3s( GLshort s, GLshort t, GLshort r )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3s, sys_args);
}


void glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	sys_args[3] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4d, sys_args);
}


void glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	sys_args[3] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4f, sys_args);
}


void glTexCoord4i( GLint s, GLint t, GLint r, GLint q )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	sys_args[3] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4i, sys_args);
}


void glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) s;
	sys_args[1] = (unsigned int) t;
	sys_args[2] = (unsigned int) r;
	sys_args[3] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4s, sys_args);
}


void glTexCoord1dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1dv, sys_args);
}


void glTexCoord1fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1fv, sys_args);
}


void glTexCoord1iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1iv, sys_args);
}


void glTexCoord1sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord1sv, sys_args);
}


void glTexCoord2dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2dv, sys_args);
}


void glTexCoord2fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2fv, sys_args);
}


void glTexCoord2iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2iv, sys_args);
}


void glTexCoord2sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord2sv, sys_args);
}


void glTexCoord3dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3dv, sys_args);
}


void glTexCoord3fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3fv, sys_args);
}


void glTexCoord3iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3iv, sys_args);
}


void glTexCoord3sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord3sv, sys_args);
}


void glTexCoord4dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4dv, sys_args);
}


void glTexCoord4fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4fv, sys_args);
}


void glTexCoord4iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4iv, sys_args);
}


void glTexCoord4sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoord4sv, sys_args);
}


void glRasterPos2d( GLdouble x, GLdouble y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2d, sys_args);
}


void glRasterPos2f( GLfloat x, GLfloat y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2f, sys_args);
}


void glRasterPos2i( GLint x, GLint y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2i, sys_args);
}


void glRasterPos2s( GLshort x, GLshort y )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2s, sys_args);
}


void glRasterPos3d( GLdouble x, GLdouble y, GLdouble z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3d, sys_args);
}


void glRasterPos3f( GLfloat x, GLfloat y, GLfloat z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3f, sys_args);
}


void glRasterPos3i( GLint x, GLint y, GLint z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3i, sys_args);
}


void glRasterPos3s( GLshort x, GLshort y, GLshort z )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3s, sys_args);
}


void glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4d, sys_args);
}


void glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4f, sys_args);
}


void glRasterPos4i( GLint x, GLint y, GLint z, GLint w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4i, sys_args);
}


void glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) z;
	sys_args[3] = (unsigned int) w;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4s, sys_args);
}


void glRasterPos2dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2dv, sys_args);
}


void glRasterPos2fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2fv, sys_args);
}


void glRasterPos2iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2iv, sys_args);
}


void glRasterPos2sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos2sv, sys_args);
}


void glRasterPos3dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3dv, sys_args);
}


void glRasterPos3fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3fv, sys_args);
}


void glRasterPos3iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3iv, sys_args);
}


void glRasterPos3sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos3sv, sys_args);
}


void glRasterPos4dv( const GLdouble *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4dv, sys_args);
}


void glRasterPos4fv( const GLfloat *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4fv, sys_args);
}


void glRasterPos4iv( const GLint *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4iv, sys_args);
}


void glRasterPos4sv( const GLshort *v )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRasterPos4sv, sys_args);
}


void glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x1;
	sys_args[1] = (unsigned int) y1;
	sys_args[2] = (unsigned int) x2;
	sys_args[3] = (unsigned int) y2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRectd, sys_args);
}


void glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x1;
	sys_args[1] = (unsigned int) y1;
	sys_args[2] = (unsigned int) x2;
	sys_args[3] = (unsigned int) y2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRectf, sys_args);
}


void glRecti( GLint x1, GLint y1, GLint x2, GLint y2 )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x1;
	sys_args[1] = (unsigned int) y1;
	sys_args[2] = (unsigned int) x2;
	sys_args[3] = (unsigned int) y2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRecti, sys_args);
}


void glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) x1;
	sys_args[1] = (unsigned int) y1;
	sys_args[2] = (unsigned int) x2;
	sys_args[3] = (unsigned int) y2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRects, sys_args);
}


void glRectdv( const GLdouble *v1, const GLdouble *v2 )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) v1;
	sys_args[1] = (unsigned int) v2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRectdv, sys_args);
}


void glRectfv( const GLfloat *v1, const GLfloat *v2 )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) v1;
	sys_args[1] = (unsigned int) v2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRectfv, sys_args);
}


void glRectiv( const GLint *v1, const GLint *v2 )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) v1;
	sys_args[1] = (unsigned int) v2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRectiv, sys_args);
}


void glRectsv( const GLshort *v1, const GLshort *v2 )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) v1;
	sys_args[1] = (unsigned int) v2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glRectsv, sys_args);
}


void glVertexPointer( GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) size;
	sys_args[1] = (unsigned int) type;
	sys_args[2] = (unsigned int) stride;
	sys_args[3] = (unsigned int) ptr;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glVertexPointer, sys_args);
}


void glNormalPointer( GLenum type, GLsizei stride,
                                       const GLvoid *ptr )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) type;
	sys_args[1] = (unsigned int) stride;
	sys_args[2] = (unsigned int) ptr;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glNormalPointer, sys_args);
}


void glColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) size;
	sys_args[1] = (unsigned int) type;
	sys_args[2] = (unsigned int) stride;
	sys_args[3] = (unsigned int) ptr;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColorPointer, sys_args);
}


void glIndexPointer( GLenum type, GLsizei stride,
                                      const GLvoid *ptr )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) type;
	sys_args[1] = (unsigned int) stride;
	sys_args[2] = (unsigned int) ptr;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIndexPointer, sys_args);
}


void glTexCoordPointer( GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) size;
	sys_args[1] = (unsigned int) type;
	sys_args[2] = (unsigned int) stride;
	sys_args[3] = (unsigned int) ptr;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexCoordPointer, sys_args);
}


void glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) stride;
	sys_args[1] = (unsigned int) ptr;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEdgeFlagPointer, sys_args);
}


void glGetPointerv( GLenum pname, GLvoid **params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetPointerv, sys_args);
}


void glArrayElement( GLint i )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) i;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glArrayElement, sys_args);
}


void glDrawArrays( GLenum mode, GLint first, GLsizei count )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) mode;
	sys_args[1] = (unsigned int) first;
	sys_args[2] = (unsigned int) count;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDrawArrays, sys_args);
}


void glDrawElements( GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) mode;
	sys_args[1] = (unsigned int) count;
	sys_args[2] = (unsigned int) type;
	sys_args[3] = (unsigned int) indices;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDrawElements, sys_args);
}


void glInterleavedArrays( GLenum format, GLsizei stride,
                                           const GLvoid *pointer )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) format;
	sys_args[1] = (unsigned int) stride;
	sys_args[2] = (unsigned int) pointer;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glInterleavedArrays, sys_args);
}


void glShadeModel( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glShadeModel, sys_args);
}


void glLightf( GLenum light, GLenum pname, GLfloat param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) light;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLightf, sys_args);
}


void glLighti( GLenum light, GLenum pname, GLint param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) light;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLighti, sys_args);
}


void glLightfv( GLenum light, GLenum pname,
                                 const GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) light;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLightfv, sys_args);
}


void glLightiv( GLenum light, GLenum pname,
                                 const GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) light;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLightiv, sys_args);
}


void glGetLightfv( GLenum light, GLenum pname,
                                    GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) light;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetLightfv, sys_args);
}


void glGetLightiv( GLenum light, GLenum pname,
                                    GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) light;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetLightiv, sys_args);
}


void glLightModelf( GLenum pname, GLfloat param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLightModelf, sys_args);
}


void glLightModeli( GLenum pname, GLint param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLightModeli, sys_args);
}


void glLightModelfv( GLenum pname, const GLfloat *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLightModelfv, sys_args);
}


void glLightModeliv( GLenum pname, const GLint *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLightModeliv, sys_args);
}


void glMaterialf( GLenum face, GLenum pname, GLfloat param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMaterialf, sys_args);
}


void glMateriali( GLenum face, GLenum pname, GLint param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMateriali, sys_args);
}


void glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMaterialfv, sys_args);
}


void glMaterialiv( GLenum face, GLenum pname, const GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMaterialiv, sys_args);
}


void glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMaterialfv, sys_args);
}


void glGetMaterialiv( GLenum face, GLenum pname, GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMaterialiv, sys_args);
}


void glColorMaterial( GLenum face, GLenum mode )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) face;
	sys_args[1] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColorMaterial, sys_args);
}


void glPixelZoom( GLfloat xfactor, GLfloat yfactor )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) xfactor;
	sys_args[1] = (unsigned int) yfactor;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelZoom, sys_args);
}


void glPixelStoref( GLenum pname, GLfloat param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelStoref, sys_args);
}


void glPixelStorei( GLenum pname, GLint param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelStorei, sys_args);
}


void glPixelTransferf( GLenum pname, GLfloat param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelTransferf, sys_args);
}


void glPixelTransferi( GLenum pname, GLint param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelTransferi, sys_args);
}


void glPixelMapfv( GLenum map, GLsizei mapsize,
                                    const GLfloat *values )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) map;
	sys_args[1] = (unsigned int) mapsize;
	sys_args[2] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelMapfv, sys_args);
}


void glPixelMapuiv( GLenum map, GLsizei mapsize,
                                     const GLuint *values )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) map;
	sys_args[1] = (unsigned int) mapsize;
	sys_args[2] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelMapuiv, sys_args);
}


void glPixelMapusv( GLenum map, GLsizei mapsize,
                                     const GLushort *values )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) map;
	sys_args[1] = (unsigned int) mapsize;
	sys_args[2] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPixelMapusv, sys_args);
}


void glGetPixelMapfv( GLenum map, GLfloat *values )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) map;
	sys_args[1] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetPixelMapfv, sys_args);
}


void glGetPixelMapuiv( GLenum map, GLuint *values )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) map;
	sys_args[1] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetPixelMapuiv, sys_args);
}


void glGetPixelMapusv( GLenum map, GLushort *values )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) map;
	sys_args[1] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetPixelMapusv, sys_args);
}


void glBitmap( GLsizei width, GLsizei height,
                                GLfloat xorig, GLfloat yorig,
                                GLfloat xmove, GLfloat ymove,
                                const GLubyte *bitmap )

{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) width;
	sys_args[1] = (unsigned int) height;
	sys_args[2] = (unsigned int) xorig;
	sys_args[3] = (unsigned int) yorig;
	sys_args[4] = (unsigned int) xmove;
	sys_args[5] = (unsigned int) ymove;
	sys_args[6] = (unsigned int) bitmap;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glBitmap, sys_args);
}


void glReadPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels )

{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) height;
	sys_args[4] = (unsigned int) format;
	sys_args[5] = (unsigned int) type;
	sys_args[6] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glReadPixels, sys_args);
}


void glDrawPixels( GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) width;
	sys_args[1] = (unsigned int) height;
	sys_args[2] = (unsigned int) format;
	sys_args[3] = (unsigned int) type;
	sys_args[4] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDrawPixels, sys_args);
}


void glCopyPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum type )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) x;
	sys_args[1] = (unsigned int) y;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) height;
	sys_args[4] = (unsigned int) type;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyPixels, sys_args);
}


void glStencilFunc( GLenum func, GLint ref, GLuint mask )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) func;
	sys_args[1] = (unsigned int) ref;
	sys_args[2] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glStencilFunc, sys_args);
}


void glStencilMask( GLuint mask )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mask;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glStencilMask, sys_args);
}


void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) fail;
	sys_args[1] = (unsigned int) zfail;
	sys_args[2] = (unsigned int) zpass;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glStencilOp, sys_args);
}


void glClearStencil( GLint s )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClearStencil, sys_args);
}


void glTexGend( GLenum coord, GLenum pname, GLdouble param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexGend, sys_args);
}


void glTexGenf( GLenum coord, GLenum pname, GLfloat param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexGenf, sys_args);
}


void glTexGeni( GLenum coord, GLenum pname, GLint param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexGeni, sys_args);
}


void glTexGendv( GLenum coord, GLenum pname, const GLdouble *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexGendv, sys_args);
}


void glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexGenfv, sys_args);
}


void glTexGeniv( GLenum coord, GLenum pname, const GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexGeniv, sys_args);
}


void glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexGendv, sys_args);
}


void glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexGenfv, sys_args);
}


void glGetTexGeniv( GLenum coord, GLenum pname, GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) coord;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexGeniv, sys_args);
}


void glTexEnvf( GLenum target, GLenum pname, GLfloat param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexEnvf, sys_args);
}


void glTexEnvi( GLenum target, GLenum pname, GLint param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexEnvi, sys_args);
}


void glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexEnvfv, sys_args);
}


void glTexEnviv( GLenum target, GLenum pname, const GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexEnviv, sys_args);
}


void glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexEnvfv, sys_args);
}


void glGetTexEnviv( GLenum target, GLenum pname, GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexEnviv, sys_args);
}


void glTexParameterf( GLenum target, GLenum pname, GLfloat param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexParameterf, sys_args);
}


void glTexParameteri( GLenum target, GLenum pname, GLint param )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexParameteri, sys_args);
}


void glTexParameterfv( GLenum target, GLenum pname,
                                          const GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexParameterfv, sys_args);
}


void glTexParameteriv( GLenum target, GLenum pname,
                                          const GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexParameteriv, sys_args);
}


void glGetTexParameterfv( GLenum target,
                                           GLenum pname, GLfloat *params)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexParameterfv, sys_args);
}


void glGetTexParameteriv( GLenum target,
                                           GLenum pname, GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexParameteriv, sys_args);
}


void glGetTexLevelParameterfv( GLenum target, GLint level,
                                                GLenum pname, GLfloat *params )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) pname;
	sys_args[3] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexLevelParameterfv, sys_args);
}


void glGetTexLevelParameteriv( GLenum target, GLint level,
                                                GLenum pname, GLint *params )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) pname;
	sys_args[3] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexLevelParameteriv, sys_args);
}


void glTexImage1D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLint border,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels )

{
	unsigned int sys_args[8];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalFormat;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) border;
	sys_args[5] = (unsigned int) format;
	sys_args[6] = (unsigned int) type;
	sys_args[7] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexImage1D, sys_args);
}


void glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels )

{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalFormat;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) height;
	sys_args[5] = (unsigned int) border;
	sys_args[6] = (unsigned int) format;
	sys_args[7] = (unsigned int) type;
	sys_args[8] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexImage2D, sys_args);
}


void glGetTexImage( GLenum target, GLint level,
                                     GLenum format, GLenum type,
                                     GLvoid *pixels )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) format;
	sys_args[3] = (unsigned int) type;
	sys_args[4] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetTexImage, sys_args);
}


void glGenTextures( GLsizei n, GLuint *textures )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) n;
	sys_args[1] = (unsigned int) textures;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGenTextures, sys_args);
}


void glDeleteTextures( GLsizei n, const GLuint *textures)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) n;
	sys_args[1] = (unsigned int) textures;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDeleteTextures, sys_args);
}


void glBindTexture( GLenum target, GLuint texture )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) texture;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glBindTexture, sys_args);
}


void glPrioritizeTextures( GLsizei n,
                                            const GLuint *textures,
                                            const GLclampf *priorities )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) n;
	sys_args[1] = (unsigned int) textures;
	sys_args[2] = (unsigned int) priorities;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPrioritizeTextures, sys_args);
}


GLboolean glAreTexturesResident( GLsizei n,
                                                  const GLuint *textures,
                                                  GLboolean *residences )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) n;
	sys_args[1] = (unsigned int) textures;
	sys_args[2] = (unsigned int) residences;
	return (GLboolean) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glAreTexturesResident, sys_args);
}


GLboolean glIsTexture( GLuint texture )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) texture;
	return (GLboolean) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glIsTexture, sys_args);
}


void glTexSubImage1D( GLenum target, GLint level,
                                       GLint xoffset,
                                       GLsizei width, GLenum format,
                                       GLenum type, const GLvoid *pixels )

{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) format;
	sys_args[5] = (unsigned int) type;
	sys_args[6] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexSubImage1D, sys_args);
}


void glTexSubImage2D( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels )

{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) yoffset;
	sys_args[4] = (unsigned int) width;
	sys_args[5] = (unsigned int) height;
	sys_args[6] = (unsigned int) format;
	sys_args[7] = (unsigned int) type;
	sys_args[8] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexSubImage2D, sys_args);
}


void glCopyTexImage1D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLint border )

{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalformat;
	sys_args[3] = (unsigned int) x;
	sys_args[4] = (unsigned int) y;
	sys_args[5] = (unsigned int) width;
	sys_args[6] = (unsigned int) border;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyTexImage1D, sys_args);
}


void glCopyTexImage2D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border )

{
	unsigned int sys_args[8];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalformat;
	sys_args[3] = (unsigned int) x;
	sys_args[4] = (unsigned int) y;
	sys_args[5] = (unsigned int) width;
	sys_args[6] = (unsigned int) height;
	sys_args[7] = (unsigned int) border;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyTexImage2D, sys_args);
}


void glCopyTexSubImage1D( GLenum target, GLint level,
                                           GLint xoffset, GLint x, GLint y,
                                           GLsizei width )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) x;
	sys_args[4] = (unsigned int) y;
	sys_args[5] = (unsigned int) width;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyTexSubImage1D, sys_args);
}


void glCopyTexSubImage2D( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height )

{
	unsigned int sys_args[8];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) yoffset;
	sys_args[4] = (unsigned int) x;
	sys_args[5] = (unsigned int) y;
	sys_args[6] = (unsigned int) width;
	sys_args[7] = (unsigned int) height;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyTexSubImage2D, sys_args);
}


void glMap1d( GLenum target, GLdouble u1, GLdouble u2,
                               GLint stride,
                               GLint order, const GLdouble *points )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	sys_args[3] = (unsigned int) stride;
	sys_args[4] = (unsigned int) order;
	sys_args[5] = (unsigned int) points;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMap1d, sys_args);
}


void glMap1f( GLenum target, GLfloat u1, GLfloat u2,
                               GLint stride,
                               GLint order, const GLfloat *points )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	sys_args[3] = (unsigned int) stride;
	sys_args[4] = (unsigned int) order;
	sys_args[5] = (unsigned int) points;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMap1f, sys_args);
}


void glMap2d( GLenum target,
		     GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
		     GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
		     const GLdouble *points )

{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	sys_args[3] = (unsigned int) ustride;
	sys_args[4] = (unsigned int) uorder;
	sys_args[5] = (unsigned int) v1;
	sys_args[6] = (unsigned int) v2;
	sys_args[7] = (unsigned int) vstride;
	sys_args[8] = (unsigned int) vorder;
	sys_args[9] = (unsigned int) points;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMap2d, sys_args);
}


void glMap2f( GLenum target,
		     GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		     GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
		     const GLfloat *points )

{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	sys_args[3] = (unsigned int) ustride;
	sys_args[4] = (unsigned int) uorder;
	sys_args[5] = (unsigned int) v1;
	sys_args[6] = (unsigned int) v2;
	sys_args[7] = (unsigned int) vstride;
	sys_args[8] = (unsigned int) vorder;
	sys_args[9] = (unsigned int) points;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMap2f, sys_args);
}


void glGetMapdv( GLenum target, GLenum query, GLdouble *v )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) query;
	sys_args[2] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMapdv, sys_args);
}


void glGetMapfv( GLenum target, GLenum query, GLfloat *v )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) query;
	sys_args[2] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMapfv, sys_args);
}


void glGetMapiv( GLenum target, GLenum query, GLint *v )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) query;
	sys_args[2] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMapiv, sys_args);
}


void glEvalCoord1d( GLdouble u )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) u;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord1d, sys_args);
}


void glEvalCoord1f( GLfloat u )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) u;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord1f, sys_args);
}


void glEvalCoord1dv( const GLdouble *u )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) u;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord1dv, sys_args);
}


void glEvalCoord1fv( const GLfloat *u )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) u;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord1fv, sys_args);
}


void glEvalCoord2d( GLdouble u, GLdouble v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) u;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord2d, sys_args);
}


void glEvalCoord2f( GLfloat u, GLfloat v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) u;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord2f, sys_args);
}


void glEvalCoord2dv( const GLdouble *u )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) u;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord2dv, sys_args);
}


void glEvalCoord2fv( const GLfloat *u )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) u;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalCoord2fv, sys_args);
}


void glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) un;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMapGrid1d, sys_args);
}


void glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) un;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMapGrid1f, sys_args);
}


void glMapGrid2d( GLint un, GLdouble u1, GLdouble u2,
                                   GLint vn, GLdouble v1, GLdouble v2 )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) un;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	sys_args[3] = (unsigned int) vn;
	sys_args[4] = (unsigned int) v1;
	sys_args[5] = (unsigned int) v2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMapGrid2d, sys_args);
}


void glMapGrid2f( GLint un, GLfloat u1, GLfloat u2,
                                   GLint vn, GLfloat v1, GLfloat v2 )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) un;
	sys_args[1] = (unsigned int) u1;
	sys_args[2] = (unsigned int) u2;
	sys_args[3] = (unsigned int) vn;
	sys_args[4] = (unsigned int) v1;
	sys_args[5] = (unsigned int) v2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMapGrid2f, sys_args);
}


void glEvalPoint1( GLint i )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) i;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalPoint1, sys_args);
}


void glEvalPoint2( GLint i, GLint j )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) i;
	sys_args[1] = (unsigned int) j;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalPoint2, sys_args);
}


void glEvalMesh1( GLenum mode, GLint i1, GLint i2 )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) mode;
	sys_args[1] = (unsigned int) i1;
	sys_args[2] = (unsigned int) i2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalMesh1, sys_args);
}


void glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) mode;
	sys_args[1] = (unsigned int) i1;
	sys_args[2] = (unsigned int) i2;
	sys_args[3] = (unsigned int) j1;
	sys_args[4] = (unsigned int) j2;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEvalMesh2, sys_args);
}


void glFogf( GLenum pname, GLfloat param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFogf, sys_args);
}


void glFogi( GLenum pname, GLint param )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) param;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFogi, sys_args);
}


void glFogfv( GLenum pname, const GLfloat *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFogfv, sys_args);
}


void glFogiv( GLenum pname, const GLint *params )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) pname;
	sys_args[1] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFogiv, sys_args);
}


void glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) size;
	sys_args[1] = (unsigned int) type;
	sys_args[2] = (unsigned int) buffer;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFeedbackBuffer, sys_args);
}


void glPassThrough( GLfloat token )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) token;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPassThrough, sys_args);
}


void glSelectBuffer( GLsizei size, GLuint *buffer )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) size;
	sys_args[1] = (unsigned int) buffer;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glSelectBuffer, sys_args);
}


void glInitNames( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glInitNames, sys_args);
}


void glLoadName( GLuint name )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) name;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLoadName, sys_args);
}


void glPushName( GLuint name )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) name;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPushName, sys_args);
}


void glPopName( void )

{
	unsigned int sys_args[1];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glPopName, sys_args);
}


void glDrawRangeElements( GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid *indices )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) mode;
	sys_args[1] = (unsigned int) start;
	sys_args[2] = (unsigned int) end;
	sys_args[3] = (unsigned int) count;
	sys_args[4] = (unsigned int) type;
	sys_args[5] = (unsigned int) indices;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glDrawRangeElements, sys_args);
}


void glTexImage3D( GLenum target, GLint level,
                                      GLint internalFormat,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLint border,
                                      GLenum format, GLenum type,
                                      const GLvoid *pixels )

{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalFormat;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) height;
	sys_args[5] = (unsigned int) depth;
	sys_args[6] = (unsigned int) border;
	sys_args[7] = (unsigned int) format;
	sys_args[8] = (unsigned int) type;
	sys_args[9] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexImage3D, sys_args);
}


void glTexSubImage3D( GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels)

{
	unsigned int sys_args[11];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) yoffset;
	sys_args[4] = (unsigned int) zoffset;
	sys_args[5] = (unsigned int) width;
	sys_args[6] = (unsigned int) height;
	sys_args[7] = (unsigned int) depth;
	sys_args[8] = (unsigned int) format;
	sys_args[9] = (unsigned int) type;
	sys_args[10] = (unsigned int) pixels;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glTexSubImage3D, sys_args);
}


void glCopyTexSubImage3D( GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLint x,
                                             GLint y, GLsizei width,
                                             GLsizei height )

{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) yoffset;
	sys_args[4] = (unsigned int) zoffset;
	sys_args[5] = (unsigned int) x;
	sys_args[6] = (unsigned int) y;
	sys_args[7] = (unsigned int) width;
	sys_args[8] = (unsigned int) height;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyTexSubImage3D, sys_args);
}


void glColorTable( GLenum target, GLenum internalformat,
                                    GLsizei width, GLenum format,
                                    GLenum type, const GLvoid *table )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) format;
	sys_args[4] = (unsigned int) type;
	sys_args[5] = (unsigned int) table;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColorTable, sys_args);
}


void glColorSubTable( GLenum target,
                                       GLsizei start, GLsizei count,
                                       GLenum format, GLenum type,
                                       const GLvoid *data )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) start;
	sys_args[2] = (unsigned int) count;
	sys_args[3] = (unsigned int) format;
	sys_args[4] = (unsigned int) type;
	sys_args[5] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColorSubTable, sys_args);
}


void glColorTableParameteriv(GLenum target, GLenum pname,
                                              const GLint *params)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColorTableParameteriv, sys_args);
}


void glColorTableParameterfv(GLenum target, GLenum pname,
                                              const GLfloat *params)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glColorTableParameterfv, sys_args);
}


void glCopyColorSubTable( GLenum target, GLsizei start,
                                           GLint x, GLint y, GLsizei width )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) start;
	sys_args[2] = (unsigned int) x;
	sys_args[3] = (unsigned int) y;
	sys_args[4] = (unsigned int) width;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyColorSubTable, sys_args);
}


void glCopyColorTable( GLenum target, GLenum internalformat,
                                        GLint x, GLint y, GLsizei width )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) x;
	sys_args[3] = (unsigned int) y;
	sys_args[4] = (unsigned int) width;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyColorTable, sys_args);
}


void glGetColorTable( GLenum target, GLenum format,
                                       GLenum type, GLvoid *table )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) format;
	sys_args[2] = (unsigned int) type;
	sys_args[3] = (unsigned int) table;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetColorTable, sys_args);
}


void glGetColorTableParameterfv( GLenum target, GLenum pname,
                                                  GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetColorTableParameterfv, sys_args);
}


void glGetColorTableParameteriv( GLenum target, GLenum pname,
                                                  GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetColorTableParameteriv, sys_args);
}


void glBlendEquation( GLenum mode )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) mode;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glBlendEquation, sys_args);
}


void glBlendColor( GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) red;
	sys_args[1] = (unsigned int) green;
	sys_args[2] = (unsigned int) blue;
	sys_args[3] = (unsigned int) alpha;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glBlendColor, sys_args);
}


void glHistogram( GLenum target, GLsizei width,
				   GLenum internalformat, GLboolean sink )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) width;
	sys_args[2] = (unsigned int) internalformat;
	sys_args[3] = (unsigned int) sink;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glHistogram, sys_args);
}


void glResetHistogram( GLenum target )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) target;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glResetHistogram, sys_args);
}


void glGetHistogram( GLenum target, GLboolean reset,
				      GLenum format, GLenum type,
				      GLvoid *values )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) reset;
	sys_args[2] = (unsigned int) format;
	sys_args[3] = (unsigned int) type;
	sys_args[4] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetHistogram, sys_args);
}


void glGetHistogramParameterfv( GLenum target, GLenum pname,
						 GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetHistogramParameterfv, sys_args);
}


void glGetHistogramParameteriv( GLenum target, GLenum pname,
						 GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetHistogramParameteriv, sys_args);
}


void glMinmax( GLenum target, GLenum internalformat,
				GLboolean sink )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) sink;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMinmax, sys_args);
}


void glResetMinmax( GLenum target )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) target;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glResetMinmax, sys_args);
}


void glGetMinmax( GLenum target, GLboolean reset,
                                   GLenum format, GLenum types,
                                   GLvoid *values )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) reset;
	sys_args[2] = (unsigned int) format;
	sys_args[3] = (unsigned int) types;
	sys_args[4] = (unsigned int) values;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMinmax, sys_args);
}


void glGetMinmaxParameterfv( GLenum target, GLenum pname,
					      GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMinmaxParameterfv, sys_args);
}


void glGetMinmaxParameteriv( GLenum target, GLenum pname,
					      GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetMinmaxParameteriv, sys_args);
}


void glConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) format;
	sys_args[4] = (unsigned int) type;
	sys_args[5] = (unsigned int) image;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glConvolutionFilter1D, sys_args);
}


void glConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image )

{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) height;
	sys_args[4] = (unsigned int) format;
	sys_args[5] = (unsigned int) type;
	sys_args[6] = (unsigned int) image;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glConvolutionFilter2D, sys_args);
}


void glConvolutionParameterf( GLenum target, GLenum pname,
	GLfloat params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glConvolutionParameterf, sys_args);
}


void glConvolutionParameterfv( GLenum target, GLenum pname,
	const GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glConvolutionParameterfv, sys_args);
}


void glConvolutionParameteri( GLenum target, GLenum pname,
	GLint params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glConvolutionParameteri, sys_args);
}


void glConvolutionParameteriv( GLenum target, GLenum pname,
	const GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glConvolutionParameteriv, sys_args);
}


void glCopyConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) x;
	sys_args[3] = (unsigned int) y;
	sys_args[4] = (unsigned int) width;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyConvolutionFilter1D, sys_args);
}


void glCopyConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height)

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) x;
	sys_args[3] = (unsigned int) y;
	sys_args[4] = (unsigned int) width;
	sys_args[5] = (unsigned int) height;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCopyConvolutionFilter2D, sys_args);
}


void glGetConvolutionFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *image )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) format;
	sys_args[2] = (unsigned int) type;
	sys_args[3] = (unsigned int) image;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetConvolutionFilter, sys_args);
}


void glGetConvolutionParameterfv( GLenum target, GLenum pname,
	GLfloat *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetConvolutionParameterfv, sys_args);
}


void glGetConvolutionParameteriv( GLenum target, GLenum pname,
	GLint *params )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) pname;
	sys_args[2] = (unsigned int) params;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetConvolutionParameteriv, sys_args);
}


void glSeparableFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column )

{
	unsigned int sys_args[8];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) internalformat;
	sys_args[2] = (unsigned int) width;
	sys_args[3] = (unsigned int) height;
	sys_args[4] = (unsigned int) format;
	sys_args[5] = (unsigned int) type;
	sys_args[6] = (unsigned int) row;
	sys_args[7] = (unsigned int) column;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glSeparableFilter2D, sys_args);
}


void glGetSeparableFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span )

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) format;
	sys_args[2] = (unsigned int) type;
	sys_args[3] = (unsigned int) row;
	sys_args[4] = (unsigned int) column;
	sys_args[5] = (unsigned int) span;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetSeparableFilter, sys_args);
}


void glActiveTexture( GLenum texture )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) texture;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glActiveTexture, sys_args);
}


void glClientActiveTexture( GLenum texture )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) texture;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClientActiveTexture, sys_args);
}


void glCompressedTexImage1D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data )

{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalformat;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) border;
	sys_args[5] = (unsigned int) imageSize;
	sys_args[6] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCompressedTexImage1D, sys_args);
}


void glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data )

{
	unsigned int sys_args[8];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalformat;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) height;
	sys_args[5] = (unsigned int) border;
	sys_args[6] = (unsigned int) imageSize;
	sys_args[7] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCompressedTexImage2D, sys_args);
}


void glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data )

{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) internalformat;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) height;
	sys_args[5] = (unsigned int) depth;
	sys_args[6] = (unsigned int) border;
	sys_args[7] = (unsigned int) imageSize;
	sys_args[8] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCompressedTexImage3D, sys_args);
}


void glCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data )

{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) width;
	sys_args[4] = (unsigned int) format;
	sys_args[5] = (unsigned int) imageSize;
	sys_args[6] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCompressedTexSubImage1D, sys_args);
}


void glCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data )

{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) yoffset;
	sys_args[4] = (unsigned int) width;
	sys_args[5] = (unsigned int) height;
	sys_args[6] = (unsigned int) format;
	sys_args[7] = (unsigned int) imageSize;
	sys_args[8] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCompressedTexSubImage2D, sys_args);
}


void glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data )

{
	unsigned int sys_args[11];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) level;
	sys_args[2] = (unsigned int) xoffset;
	sys_args[3] = (unsigned int) yoffset;
	sys_args[4] = (unsigned int) zoffset;
	sys_args[5] = (unsigned int) width;
	sys_args[6] = (unsigned int) height;
	sys_args[7] = (unsigned int) depth;
	sys_args[8] = (unsigned int) format;
	sys_args[9] = (unsigned int) imageSize;
	sys_args[10] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCompressedTexSubImage3D, sys_args);
}


void glGetCompressedTexImage( GLenum target, GLint lod, GLvoid *img )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) lod;
	sys_args[2] = (unsigned int) img;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetCompressedTexImage, sys_args);
}


void glMultiTexCoord1d( GLenum target, GLdouble s )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1d, sys_args);
}


void glMultiTexCoord1dv( GLenum target, const GLdouble *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1dv, sys_args);
}


void glMultiTexCoord1f( GLenum target, GLfloat s )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1f, sys_args);
}


void glMultiTexCoord1fv( GLenum target, const GLfloat *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1fv, sys_args);
}


void glMultiTexCoord1i( GLenum target, GLint s )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1i, sys_args);
}


void glMultiTexCoord1iv( GLenum target, const GLint *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1iv, sys_args);
}


void glMultiTexCoord1s( GLenum target, GLshort s )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1s, sys_args);
}


void glMultiTexCoord1sv( GLenum target, const GLshort *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1sv, sys_args);
}


void glMultiTexCoord2d( GLenum target, GLdouble s, GLdouble t )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2d, sys_args);
}


void glMultiTexCoord2dv( GLenum target, const GLdouble *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2dv, sys_args);
}


void glMultiTexCoord2f( GLenum target, GLfloat s, GLfloat t )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2f, sys_args);
}


void glMultiTexCoord2fv( GLenum target, const GLfloat *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2fv, sys_args);
}


void glMultiTexCoord2i( GLenum target, GLint s, GLint t )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2i, sys_args);
}


void glMultiTexCoord2iv( GLenum target, const GLint *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2iv, sys_args);
}


void glMultiTexCoord2s( GLenum target, GLshort s, GLshort t )

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2s, sys_args);
}


void glMultiTexCoord2sv( GLenum target, const GLshort *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2sv, sys_args);
}


void glMultiTexCoord3d( GLenum target, GLdouble s, GLdouble t, GLdouble r )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3d, sys_args);
}


void glMultiTexCoord3dv( GLenum target, const GLdouble *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3dv, sys_args);
}


void glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t, GLfloat r )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3f, sys_args);
}


void glMultiTexCoord3fv( GLenum target, const GLfloat *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3fv, sys_args);
}


void glMultiTexCoord3i( GLenum target, GLint s, GLint t, GLint r )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3i, sys_args);
}


void glMultiTexCoord3iv( GLenum target, const GLint *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3iv, sys_args);
}


void glMultiTexCoord3s( GLenum target, GLshort s, GLshort t, GLshort r )

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3s, sys_args);
}


void glMultiTexCoord3sv( GLenum target, const GLshort *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3sv, sys_args);
}


void glMultiTexCoord4d( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4d, sys_args);
}


void glMultiTexCoord4dv( GLenum target, const GLdouble *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4dv, sys_args);
}


void glMultiTexCoord4f( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4f, sys_args);
}


void glMultiTexCoord4fv( GLenum target, const GLfloat *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4fv, sys_args);
}


void glMultiTexCoord4i( GLenum target, GLint s, GLint t, GLint r, GLint q )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4i, sys_args);
}


void glMultiTexCoord4iv( GLenum target, const GLint *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4iv, sys_args);
}


void glMultiTexCoord4s( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q )

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4s, sys_args);
}


void glMultiTexCoord4sv( GLenum target, const GLshort *v )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4sv, sys_args);
}


void glLoadTransposeMatrixd( const GLdouble m[16] )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m[16];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLoadTransposeMatrixd, sys_args);
}


void glLoadTransposeMatrixf( const GLfloat m[16] )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m[16];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glLoadTransposeMatrixf, sys_args);
}


void glMultTransposeMatrixd( const GLdouble m[16] )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m[16];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultTransposeMatrixd, sys_args);
}


void glMultTransposeMatrixf( const GLfloat m[16] )

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) m[16];
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultTransposeMatrixf, sys_args);
}


void glSampleCoverage( GLclampf value, GLboolean invert )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) value;
	sys_args[1] = (unsigned int) invert;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glSampleCoverage, sys_args);
}


void glActiveTextureARB(GLenum texture)

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) texture;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glActiveTextureARB, sys_args);
}


void glClientActiveTextureARB(GLenum texture)

{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) texture;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClientActiveTextureARB, sys_args);
}


void glMultiTexCoord1dARB(GLenum target, GLdouble s)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1dARB, sys_args);
}


void glMultiTexCoord1dvARB(GLenum target, const GLdouble *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1dvARB, sys_args);
}


void glMultiTexCoord1fARB(GLenum target, GLfloat s)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1fARB, sys_args);
}


void glMultiTexCoord1fvARB(GLenum target, const GLfloat *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1fvARB, sys_args);
}


void glMultiTexCoord1iARB(GLenum target, GLint s)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1iARB, sys_args);
}


void glMultiTexCoord1ivARB(GLenum target, const GLint *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1ivARB, sys_args);
}


void glMultiTexCoord1sARB(GLenum target, GLshort s)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1sARB, sys_args);
}


void glMultiTexCoord1svARB(GLenum target, const GLshort *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord1svARB, sys_args);
}


void glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2dARB, sys_args);
}


void glMultiTexCoord2dvARB(GLenum target, const GLdouble *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2dvARB, sys_args);
}


void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2fARB, sys_args);
}


void glMultiTexCoord2fvARB(GLenum target, const GLfloat *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2fvARB, sys_args);
}


void glMultiTexCoord2iARB(GLenum target, GLint s, GLint t)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2iARB, sys_args);
}


void glMultiTexCoord2ivARB(GLenum target, const GLint *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2ivARB, sys_args);
}


void glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2sARB, sys_args);
}


void glMultiTexCoord2svARB(GLenum target, const GLshort *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord2svARB, sys_args);
}


void glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r)

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3dARB, sys_args);
}


void glMultiTexCoord3dvARB(GLenum target, const GLdouble *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3dvARB, sys_args);
}


void glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r)

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3fARB, sys_args);
}


void glMultiTexCoord3fvARB(GLenum target, const GLfloat *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3fvARB, sys_args);
}


void glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r)

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3iARB, sys_args);
}


void glMultiTexCoord3ivARB(GLenum target, const GLint *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3ivARB, sys_args);
}


void glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r)

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3sARB, sys_args);
}


void glMultiTexCoord3svARB(GLenum target, const GLshort *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord3svARB, sys_args);
}


void glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4dARB, sys_args);
}


void glMultiTexCoord4dvARB(GLenum target, const GLdouble *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4dvARB, sys_args);
}


void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4fARB, sys_args);
}


void glMultiTexCoord4fvARB(GLenum target, const GLfloat *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4fvARB, sys_args);
}


void glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q)

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4iARB, sys_args);
}


void glMultiTexCoord4ivARB(GLenum target, const GLint *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4ivARB, sys_args);
}


void glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) s;
	sys_args[2] = (unsigned int) t;
	sys_args[3] = (unsigned int) r;
	sys_args[4] = (unsigned int) q;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4sARB, sys_args);
}


void glMultiTexCoord4svARB(GLenum target, const GLshort *v)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glMultiTexCoord4svARB, sys_args);
}


GLhandleARB glCreateDebugObjectMESA (void)

{
	unsigned int sys_args[1];
	return (GLhandleARB) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glCreateDebugObjectMESA , sys_args);
}


void glClearDebugLogMESA (GLhandleARB obj, GLenum logType, GLenum shaderType)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) obj;
	sys_args[1] = (unsigned int) logType;
	sys_args[2] = (unsigned int) shaderType;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glClearDebugLogMESA , sys_args);
}


void glGetDebugLogMESA (GLhandleARB obj, GLenum logType, GLenum shaderType, GLsizei maxLength,
                                         GLsizei *length, GLcharARB *debugLog)

{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) obj;
	sys_args[1] = (unsigned int) logType;
	sys_args[2] = (unsigned int) shaderType;
	sys_args[3] = (unsigned int) maxLength;
	sys_args[4] = (unsigned int) length;
	sys_args[5] = (unsigned int) debugLog;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetDebugLogMESA , sys_args);
}


GLsizei glGetDebugLogLengthMESA (GLhandleARB obj, GLenum logType, GLenum shaderType)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) obj;
	sys_args[1] = (unsigned int) logType;
	sys_args[2] = (unsigned int) shaderType;
	return (GLsizei) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetDebugLogLengthMESA , sys_args);
}


void glProgramCallbackMESA(GLenum target, GLprogramcallbackMESA callback, GLvoid *data)

{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) callback;
	sys_args[2] = (unsigned int) data;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glProgramCallbackMESA, sys_args);
}


void glGetProgramRegisterfvMESA(GLenum target, GLsizei len, const GLubyte *name, GLfloat *v)

{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) len;
	sys_args[2] = (unsigned int) name;
	sys_args[3] = (unsigned int) v;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glGetProgramRegisterfvMESA, sys_args);
}


void *glFramebufferTextureLayerEXT(GLenum target,
    GLenum attachment, GLuint texture, GLint level, GLint layer)

{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) attachment;
	sys_args[2] = (unsigned int) texture;
	sys_args[3] = (unsigned int) level;
	sys_args[4] = (unsigned int) layer;
	return (void*) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glFramebufferTextureLayerEXT, sys_args);
}


void glBlendEquationSeparateATI( GLenum modeRGB, GLenum modeA )

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) modeRGB;
	sys_args[1] = (unsigned int) modeA;
	syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glBlendEquationSeparateATI, sys_args);
}


void *glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) image;
	return (void*) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEGLImageTargetTexture2DOES , sys_args);
}


void *glEGLImageTargetRenderbufferStorageOES (GLenum target, GLeglImageOES image)

{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) target;
	sys_args[1] = (unsigned int) image;
	return (void*) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_glEGLImageTargetRenderbufferStorageOES , sys_args);
}


