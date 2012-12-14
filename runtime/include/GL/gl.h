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

#ifndef RUNTIME_INCLUDE_GL_GL_H
#define RUNTIME_INCLUDE_GL_GL_H

#include <stddef.h>
#include <stdint.h>


typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;

__attribute__ ((visibility("default")))
     void glClearIndex(GLfloat c);

__attribute__ ((visibility("default")))
     void glClearColor(GLclampf red, GLclampf green, GLclampf blue,
	GLclampf alpha);
__attribute__ ((visibility("default")))
     void glClear(GLbitfield mask);

__attribute__ ((visibility("default")))
     void glIndexMask(GLuint mask);

__attribute__ ((visibility("default")))
     void glColorMask(GLboolean red, GLboolean green, GLboolean blue,
	GLboolean alpha);
__attribute__ ((visibility("default")))
     void glAlphaFunc(GLenum func, GLclampf ref);

__attribute__ ((visibility("default")))
     void glBlendFunc(GLenum sfactor, GLenum dfactor);

__attribute__ ((visibility("default")))
     void glLogicOp(GLenum opcode);

__attribute__ ((visibility("default")))
     void glCullFace(GLenum mode);

__attribute__ ((visibility("default")))
     void glFrontFace(GLenum mode);

__attribute__ ((visibility("default")))
     void glPointSize(GLfloat size);

__attribute__ ((visibility("default")))
     void glLineWidth(GLfloat width);

__attribute__ ((visibility("default")))
     void glLineStipple(GLint factor, GLushort pattern);

__attribute__ ((visibility("default")))
     void glPolygonMode(GLenum face, GLenum mode);

__attribute__ ((visibility("default")))
     void glPolygonOffset(GLfloat factor, GLfloat units);

__attribute__ ((visibility("default")))
     void glPolygonStipple(const GLubyte * mask);

__attribute__ ((visibility("default")))
     void glGetPolygonStipple(GLubyte * mask);

__attribute__ ((visibility("default")))
     void glEdgeFlag(GLboolean flag);

__attribute__ ((visibility("default")))
     void glEdgeFlagv(const GLboolean * flag);

__attribute__ ((visibility("default")))
     void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);

__attribute__ ((visibility("default")))
     void glClipPlane(GLenum plane, const GLdouble * equation);

__attribute__ ((visibility("default")))
     void glGetClipPlane(GLenum plane, GLdouble * equation);

__attribute__ ((visibility("default")))
     void glDrawBuffer(GLenum mode);

__attribute__ ((visibility("default")))
     void glReadBuffer(GLenum mode);

__attribute__ ((visibility("default")))
     void glEnable(GLenum cap);

__attribute__ ((visibility("default")))
     void glDisable(GLenum cap);

__attribute__ ((visibility("default")))
     GLboolean glIsEnabled(GLenum cap);

__attribute__ ((visibility("default")))
     void glEnableClientState(GLenum cap);

__attribute__ ((visibility("default")))
     void glDisableClientState(GLenum cap);

__attribute__ ((visibility("default")))
     void glGetBooleanv(GLenum pname, GLboolean * params);

__attribute__ ((visibility("default")))
     void glGetDoublev(GLenum pname, GLdouble * params);

__attribute__ ((visibility("default")))
     void glGetFloatv(GLenum pname, GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetIntegerv(GLenum pname, GLint * params);

__attribute__ ((visibility("default")))
     void glPushAttrib(GLbitfield mask);

__attribute__ ((visibility("default")))
     void glPopAttrib(void);

__attribute__ ((visibility("default")))
     void glPushClientAttrib(GLbitfield mask);

__attribute__ ((visibility("default")))
     void glPopClientAttrib(void);

__attribute__ ((visibility("default")))
     GLint glRenderMode(GLenum mode);

__attribute__ ((visibility("default")))
     GLenum glGetError(void);

__attribute__ ((visibility("default")))
     const GLubyte *glGetString(GLenum name);

__attribute__ ((visibility("default")))
     void glFinish(void);

__attribute__ ((visibility("default")))
     void glFlush(void);

__attribute__ ((visibility("default")))
     void glHint(GLenum target, GLenum mode);






__attribute__ ((visibility("default")))
     void glClearDepth(GLclampd depth);

__attribute__ ((visibility("default")))
     void glDepthFunc(GLenum func);

__attribute__ ((visibility("default")))
     void glDepthMask(GLboolean flag);

__attribute__ ((visibility("default")))
     void glDepthRange(GLclampd near_val, GLclampd far_val);






__attribute__ ((visibility("default")))
     void glClearAccum(GLfloat red, GLfloat green, GLfloat blue,
	GLfloat alpha);

__attribute__ ((visibility("default")))
     void glAccum(GLenum op, GLfloat value);






__attribute__ ((visibility("default")))
     void glMatrixMode(GLenum mode);

__attribute__ ((visibility("default")))
     void glOrtho(GLdouble left, GLdouble right,
	GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);

__attribute__ ((visibility("default")))
     void glFrustum(GLdouble left, GLdouble right,
	GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);

__attribute__ ((visibility("default")))
     void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

__attribute__ ((visibility("default")))
     void glPushMatrix(void);

__attribute__ ((visibility("default")))
     void glPopMatrix(void);

__attribute__ ((visibility("default")))
     void glLoadIdentity(void);

__attribute__ ((visibility("default")))
     void glLoadMatrixd(const GLdouble * m);

__attribute__ ((visibility("default")))
     void glLoadMatrixf(const GLfloat * m);

__attribute__ ((visibility("default")))
     void glMultMatrixd(const GLdouble * m);

__attribute__ ((visibility("default")))
     void glMultMatrixf(const GLfloat * m);

__attribute__ ((visibility("default")))
     void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);

__attribute__ ((visibility("default")))
     void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

__attribute__ ((visibility("default")))
     void glScaled(GLdouble x, GLdouble y, GLdouble z);

__attribute__ ((visibility("default")))
     void glScalef(GLfloat x, GLfloat y, GLfloat z);

__attribute__ ((visibility("default")))
     void glTranslated(GLdouble x, GLdouble y, GLdouble z);

__attribute__ ((visibility("default")))
     void glTranslatef(GLfloat x, GLfloat y, GLfloat z);






__attribute__ ((visibility("default")))
     GLboolean glIsList(GLuint list);

__attribute__ ((visibility("default")))
     void glDeleteLists(GLuint list, GLsizei range);

__attribute__ ((visibility("default")))
     GLuint glGenLists(GLsizei range);

__attribute__ ((visibility("default")))
     void glNewList(GLuint list, GLenum mode);

__attribute__ ((visibility("default")))
     void glEndList(void);

__attribute__ ((visibility("default")))
     void glCallList(GLuint list);

__attribute__ ((visibility("default")))
     void glCallLists(GLsizei n, GLenum type, const GLvoid * lists);

__attribute__ ((visibility("default")))
     void glListBase(GLuint base);






__attribute__ ((visibility("default")))
     void glBegin(GLenum mode);

__attribute__ ((visibility("default")))
     void glEnd(void);


__attribute__ ((visibility("default")))
     void glVertex2d(GLdouble x, GLdouble y);

__attribute__ ((visibility("default")))
     void glVertex2f(GLfloat x, GLfloat y);

__attribute__ ((visibility("default")))
     void glVertex2i(GLint x, GLint y);

__attribute__ ((visibility("default")))
     void glVertex2s(GLshort x, GLshort y);

__attribute__ ((visibility("default")))
     void glVertex3d(GLdouble x, GLdouble y, GLdouble z);

__attribute__ ((visibility("default")))
     void glVertex3f(GLfloat x, GLfloat y, GLfloat z);

__attribute__ ((visibility("default")))
     void glVertex3i(GLint x, GLint y, GLint z);

__attribute__ ((visibility("default")))
     void glVertex3s(GLshort x, GLshort y, GLshort z);

__attribute__ ((visibility("default")))
     void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

__attribute__ ((visibility("default")))
     void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

__attribute__ ((visibility("default")))
     void glVertex4i(GLint x, GLint y, GLint z, GLint w);

__attribute__ ((visibility("default")))
     void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);

__attribute__ ((visibility("default")))
     void glVertex2dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glVertex2fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glVertex2iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glVertex2sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glVertex3dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glVertex3fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glVertex3iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glVertex3sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glVertex4dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glVertex4fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glVertex4iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glVertex4sv(const GLshort * v);


__attribute__ ((visibility("default")))
     void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);

__attribute__ ((visibility("default")))
     void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);

__attribute__ ((visibility("default")))
     void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);

__attribute__ ((visibility("default")))
     void glNormal3i(GLint nx, GLint ny, GLint nz);

__attribute__ ((visibility("default")))
     void glNormal3s(GLshort nx, GLshort ny, GLshort nz);

__attribute__ ((visibility("default")))
     void glNormal3bv(const GLbyte * v);

__attribute__ ((visibility("default")))
     void glNormal3dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glNormal3fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glNormal3iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glNormal3sv(const GLshort * v);


__attribute__ ((visibility("default")))
     void glIndexd(GLdouble c);

__attribute__ ((visibility("default")))
     void glIndexf(GLfloat c);

__attribute__ ((visibility("default")))
     void glIndexi(GLint c);

__attribute__ ((visibility("default")))
     void glIndexs(GLshort c);

__attribute__ ((visibility("default")))
     void glIndexub(GLubyte c);

__attribute__ ((visibility("default")))
     void glIndexdv(const GLdouble * c);

__attribute__ ((visibility("default")))
     void glIndexfv(const GLfloat * c);

__attribute__ ((visibility("default")))
     void glIndexiv(const GLint * c);

__attribute__ ((visibility("default")))
     void glIndexsv(const GLshort * c);

__attribute__ ((visibility("default")))
     void glIndexubv(const GLubyte * c);

__attribute__ ((visibility("default")))
     void glColor3b(GLbyte red, GLbyte green, GLbyte blue);

__attribute__ ((visibility("default")))
     void glColor3d(GLdouble red, GLdouble green, GLdouble blue);

__attribute__ ((visibility("default")))
     void glColor3f(GLfloat red, GLfloat green, GLfloat blue);

__attribute__ ((visibility("default")))
     void glColor3i(GLint red, GLint green, GLint blue);

__attribute__ ((visibility("default")))
     void glColor3s(GLshort red, GLshort green, GLshort blue);

__attribute__ ((visibility("default")))
     void glColor3ub(GLubyte red, GLubyte green, GLubyte blue);

__attribute__ ((visibility("default")))
     void glColor3ui(GLuint red, GLuint green, GLuint blue);

__attribute__ ((visibility("default")))
     void glColor3us(GLushort red, GLushort green, GLushort blue);

__attribute__ ((visibility("default")))
     void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);

__attribute__ ((visibility("default")))
     void glColor4d(GLdouble red, GLdouble green,
	GLdouble blue, GLdouble alpha);
__attribute__ ((visibility("default")))
     void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

__attribute__ ((visibility("default")))
     void glColor4i(GLint red, GLint green, GLint blue, GLint alpha);

__attribute__ ((visibility("default")))
     void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);

__attribute__ ((visibility("default")))
     void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);

__attribute__ ((visibility("default")))
     void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);

__attribute__ ((visibility("default")))
     void glColor4us(GLushort red, GLushort green,
	GLushort blue, GLushort alpha);


__attribute__ ((visibility("default")))
     void glColor3bv(const GLbyte * v);

__attribute__ ((visibility("default")))
     void glColor3dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glColor3fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glColor3iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glColor3sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glColor3ubv(const GLubyte * v);

__attribute__ ((visibility("default")))
     void glColor3uiv(const GLuint * v);

__attribute__ ((visibility("default")))
     void glColor3usv(const GLushort * v);

__attribute__ ((visibility("default")))
     void glColor4bv(const GLbyte * v);

__attribute__ ((visibility("default")))
     void glColor4dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glColor4fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glColor4iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glColor4sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glColor4ubv(const GLubyte * v);

__attribute__ ((visibility("default")))
     void glColor4uiv(const GLuint * v);

__attribute__ ((visibility("default")))
     void glColor4usv(const GLushort * v);


__attribute__ ((visibility("default")))
     void glTexCoord1d(GLdouble s);

__attribute__ ((visibility("default")))
     void glTexCoord1f(GLfloat s);

__attribute__ ((visibility("default")))
     void glTexCoord1i(GLint s);

__attribute__ ((visibility("default")))
     void glTexCoord1s(GLshort s);

__attribute__ ((visibility("default")))
     void glTexCoord2d(GLdouble s, GLdouble t);

__attribute__ ((visibility("default")))
     void glTexCoord2f(GLfloat s, GLfloat t);

__attribute__ ((visibility("default")))
     void glTexCoord2i(GLint s, GLint t);

__attribute__ ((visibility("default")))
     void glTexCoord2s(GLshort s, GLshort t);

__attribute__ ((visibility("default")))
     void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);

__attribute__ ((visibility("default")))
     void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);

__attribute__ ((visibility("default")))
     void glTexCoord3i(GLint s, GLint t, GLint r);

__attribute__ ((visibility("default")))
     void glTexCoord3s(GLshort s, GLshort t, GLshort r);

__attribute__ ((visibility("default")))
     void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);

__attribute__ ((visibility("default")))
     void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);

__attribute__ ((visibility("default")))
     void glTexCoord4i(GLint s, GLint t, GLint r, GLint q);

__attribute__ ((visibility("default")))
     void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);

__attribute__ ((visibility("default")))
     void glTexCoord1dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glTexCoord1fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glTexCoord1iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glTexCoord1sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glTexCoord2dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glTexCoord2fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glTexCoord2iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glTexCoord2sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glTexCoord3dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glTexCoord3fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glTexCoord3iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glTexCoord3sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glTexCoord4dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glTexCoord4fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glTexCoord4iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glTexCoord4sv(const GLshort * v);


__attribute__ ((visibility("default")))
     void glRasterPos2d(GLdouble x, GLdouble y);

__attribute__ ((visibility("default")))
     void glRasterPos2f(GLfloat x, GLfloat y);

__attribute__ ((visibility("default")))
     void glRasterPos2i(GLint x, GLint y);

__attribute__ ((visibility("default")))
     void glRasterPos2s(GLshort x, GLshort y);

__attribute__ ((visibility("default")))
     void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);

__attribute__ ((visibility("default")))
     void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);

__attribute__ ((visibility("default")))
     void glRasterPos3i(GLint x, GLint y, GLint z);

__attribute__ ((visibility("default")))
     void glRasterPos3s(GLshort x, GLshort y, GLshort z);

__attribute__ ((visibility("default")))
     void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

__attribute__ ((visibility("default")))
     void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

__attribute__ ((visibility("default")))
     void glRasterPos4i(GLint x, GLint y, GLint z, GLint w);

__attribute__ ((visibility("default")))
     void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);

__attribute__ ((visibility("default")))
     void glRasterPos2dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glRasterPos2fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glRasterPos2iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glRasterPos2sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glRasterPos3dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glRasterPos3fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glRasterPos3iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glRasterPos3sv(const GLshort * v);

__attribute__ ((visibility("default")))
     void glRasterPos4dv(const GLdouble * v);

__attribute__ ((visibility("default")))
     void glRasterPos4fv(const GLfloat * v);

__attribute__ ((visibility("default")))
     void glRasterPos4iv(const GLint * v);

__attribute__ ((visibility("default")))
     void glRasterPos4sv(const GLshort * v);


__attribute__ ((visibility("default")))
     void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);

__attribute__ ((visibility("default")))
     void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);

__attribute__ ((visibility("default")))
     void glRecti(GLint x1, GLint y1, GLint x2, GLint y2);

__attribute__ ((visibility("default")))
     void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);


__attribute__ ((visibility("default")))
     void glRectdv(const GLdouble * v1, const GLdouble * v2);

__attribute__ ((visibility("default")))
     void glRectfv(const GLfloat * v1, const GLfloat * v2);

__attribute__ ((visibility("default")))
     void glRectiv(const GLint * v1, const GLint * v2);

__attribute__ ((visibility("default")))
     void glRectsv(const GLshort * v1, const GLshort * v2);






__attribute__ ((visibility("default")))
     void glVertexPointer(GLint size, GLenum type,
	GLsizei stride, const GLvoid * ptr);

__attribute__ ((visibility("default")))
     void glNormalPointer(GLenum type, GLsizei stride, const GLvoid * ptr);

__attribute__ ((visibility("default")))
     void glColorPointer(GLint size, GLenum type,
	GLsizei stride, const GLvoid * ptr);

__attribute__ ((visibility("default")))
     void glIndexPointer(GLenum type, GLsizei stride, const GLvoid * ptr);

__attribute__ ((visibility("default")))
     void glTexCoordPointer(GLint size, GLenum type,
	GLsizei stride, const GLvoid * ptr);

__attribute__ ((visibility("default")))
     void glEdgeFlagPointer(GLsizei stride, const GLvoid * ptr);

__attribute__ ((visibility("default")))
     void glGetPointerv(GLenum pname, GLvoid ** params);

__attribute__ ((visibility("default")))
     void glArrayElement(GLint i);

__attribute__ ((visibility("default")))
     void glDrawArrays(GLenum mode, GLint first, GLsizei count);

__attribute__ ((visibility("default")))
     void glDrawElements(GLenum mode, GLsizei count,
	GLenum type, const GLvoid * indices);

__attribute__ ((visibility("default")))
     void glInterleavedArrays(GLenum format, GLsizei stride,
	const GLvoid * pointer);





__attribute__ ((visibility("default")))
     void glShadeModel(GLenum mode);

__attribute__ ((visibility("default")))
     void glLightf(GLenum light, GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glLighti(GLenum light, GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glLightfv(GLenum light, GLenum pname, const GLfloat * params);

__attribute__ ((visibility("default")))
     void glLightiv(GLenum light, GLenum pname, const GLint * params);

__attribute__ ((visibility("default")))
     void glGetLightfv(GLenum light, GLenum pname, GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetLightiv(GLenum light, GLenum pname, GLint * params);

__attribute__ ((visibility("default")))
     void glLightModelf(GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glLightModeli(GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glLightModelfv(GLenum pname, const GLfloat * params);

__attribute__ ((visibility("default")))
     void glLightModeliv(GLenum pname, const GLint * params);

__attribute__ ((visibility("default")))
     void glMaterialf(GLenum face, GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glMateriali(GLenum face, GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glMaterialfv(GLenum face, GLenum pname, const GLfloat * params);

__attribute__ ((visibility("default")))
     void glMaterialiv(GLenum face, GLenum pname, const GLint * params);

__attribute__ ((visibility("default")))
     void glGetMaterialfv(GLenum face, GLenum pname, GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetMaterialiv(GLenum face, GLenum pname, GLint * params);

__attribute__ ((visibility("default")))
     void glColorMaterial(GLenum face, GLenum mode);






__attribute__ ((visibility("default")))
     void glPixelZoom(GLfloat xfactor, GLfloat yfactor);

__attribute__ ((visibility("default")))
     void glPixelStoref(GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glPixelStorei(GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glPixelTransferf(GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glPixelTransferi(GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat * values);

__attribute__ ((visibility("default")))
     void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint * values);

__attribute__ ((visibility("default")))
     void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort * values);

__attribute__ ((visibility("default")))
     void glGetPixelMapfv(GLenum map, GLfloat * values);

__attribute__ ((visibility("default")))
     void glGetPixelMapuiv(GLenum map, GLuint * values);

__attribute__ ((visibility("default")))
     void glGetPixelMapusv(GLenum map, GLushort * values);

__attribute__ ((visibility("default")))
     void glBitmap(GLsizei width, GLsizei height,
	GLfloat xorig, GLfloat yorig,
	GLfloat xmove, GLfloat ymove, const GLubyte * bitmap);

__attribute__ ((visibility("default")))
     void glReadPixels(GLint x, GLint y,
	GLsizei width, GLsizei height,
	GLenum format, GLenum type, GLvoid * pixels);

__attribute__ ((visibility("default")))
     void glDrawPixels(GLsizei width, GLsizei height,
	GLenum format, GLenum type, const GLvoid * pixels);

__attribute__ ((visibility("default")))
     void glCopyPixels(GLint x, GLint y,
	GLsizei width, GLsizei height, GLenum type);



__attribute__ ((visibility("default")))
     void glStencilFunc(GLenum func, GLint ref, GLuint mask);

__attribute__ ((visibility("default")))
     void glStencilMask(GLuint mask);

__attribute__ ((visibility("default")))
     void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);

__attribute__ ((visibility("default")))
     void glClearStencil(GLint s);




__attribute__ ((visibility("default")))
     void glTexGend(GLenum coord, GLenum pname, GLdouble param);

__attribute__ ((visibility("default")))
     void glTexGenf(GLenum coord, GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glTexGeni(GLenum coord, GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glTexGendv(GLenum coord, GLenum pname, const GLdouble * params);

__attribute__ ((visibility("default")))
     void glTexGenfv(GLenum coord, GLenum pname, const GLfloat * params);

__attribute__ ((visibility("default")))
     void glTexGeniv(GLenum coord, GLenum pname, const GLint * params);

__attribute__ ((visibility("default")))
     void glGetTexGendv(GLenum coord, GLenum pname, GLdouble * params);

__attribute__ ((visibility("default")))
     void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetTexGeniv(GLenum coord, GLenum pname, GLint * params);


__attribute__ ((visibility("default")))
     void glTexEnvf(GLenum target, GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glTexEnvi(GLenum target, GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glTexEnvfv(GLenum target, GLenum pname, const GLfloat * params);

__attribute__ ((visibility("default")))
     void glTexEnviv(GLenum target, GLenum pname, const GLint * params);

__attribute__ ((visibility("default")))
     void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetTexEnviv(GLenum target, GLenum pname, GLint * params);


__attribute__ ((visibility("default")))
     void glTexParameterf(GLenum target, GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glTexParameteri(GLenum target, GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glTexParameterfv(GLenum target, GLenum pname,
	const GLfloat * params);
__attribute__ ((visibility("default")))
     void glTexParameteriv(GLenum target, GLenum pname, const GLint * params);

__attribute__ ((visibility("default")))
     void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetTexParameteriv(GLenum target, GLenum pname, GLint * params);

__attribute__ ((visibility("default")))
     void glGetTexLevelParameterfv(GLenum target, GLint level,
	GLenum pname, GLfloat * params);
__attribute__ ((visibility("default")))
     void glGetTexLevelParameteriv(GLenum target, GLint level,
	GLenum pname, GLint * params);


__attribute__ ((visibility("default")))
     void glTexImage1D(GLenum target, GLint level,
	GLint internalFormat,
	GLsizei width, GLint border,
	GLenum format, GLenum type, const GLvoid * pixels);

__attribute__ ((visibility("default")))
     void glTexImage2D(GLenum target, GLint level,
	GLint internalFormat,
	GLsizei width, GLsizei height,
	GLint border, GLenum format, GLenum type, const GLvoid * pixels);

__attribute__ ((visibility("default")))
     void glGetTexImage(GLenum target, GLint level,
	GLenum format, GLenum type, GLvoid * pixels);




__attribute__ ((visibility("default")))
     void glGenTextures(GLsizei n, GLuint * textures);

__attribute__ ((visibility("default")))
     void glDeleteTextures(GLsizei n, const GLuint * textures);

__attribute__ ((visibility("default")))
     void glBindTexture(GLenum target, GLuint texture);

__attribute__ ((visibility("default")))
     void glPrioritizeTextures(GLsizei n,
	const GLuint * textures, const GLclampf * priorities);

__attribute__ ((visibility("default")))
     GLboolean glAreTexturesResident(GLsizei n,
	const GLuint * textures, GLboolean * residences);

__attribute__ ((visibility("default")))
     GLboolean glIsTexture(GLuint texture);


__attribute__ ((visibility("default")))
     void glTexSubImage1D(GLenum target, GLint level,
	GLint xoffset,
	GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);


__attribute__ ((visibility("default")))
     void glTexSubImage2D(GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height,
	GLenum format, GLenum type, const GLvoid * pixels);


__attribute__ ((visibility("default")))
     void glCopyTexImage1D(GLenum target, GLint level,
	GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);


__attribute__ ((visibility("default")))
     void glCopyTexImage2D(GLenum target, GLint level,
	GLenum internalformat,
	GLint x, GLint y, GLsizei width, GLsizei height, GLint border);


__attribute__ ((visibility("default")))
     void glCopyTexSubImage1D(GLenum target, GLint level,
	GLint xoffset, GLint x, GLint y, GLsizei width);


__attribute__ ((visibility("default")))
     void glCopyTexSubImage2D(GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLint x, GLint y, GLsizei width, GLsizei height);


__attribute__ ((visibility("default")))
     void glMap1d(GLenum target, GLdouble u1, GLdouble u2,
	GLint stride, GLint order, const GLdouble * points);
__attribute__ ((visibility("default")))
     void glMap1f(GLenum target, GLfloat u1, GLfloat u2,
	GLint stride, GLint order, const GLfloat * points);

__attribute__ ((visibility("default")))
     void glMap2d(GLenum target,
	GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
	GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
	const GLdouble * points);
__attribute__ ((visibility("default")))
     void glMap2f(GLenum target,
	GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
	GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
	const GLfloat * points);

__attribute__ ((visibility("default")))
     void glGetMapdv(GLenum target, GLenum query, GLdouble * v);

__attribute__ ((visibility("default")))
     void glGetMapfv(GLenum target, GLenum query, GLfloat * v);

__attribute__ ((visibility("default")))
     void glGetMapiv(GLenum target, GLenum query, GLint * v);

__attribute__ ((visibility("default")))
     void glEvalCoord1d(GLdouble u);

__attribute__ ((visibility("default")))
     void glEvalCoord1f(GLfloat u);

__attribute__ ((visibility("default")))
     void glEvalCoord1dv(const GLdouble * u);

__attribute__ ((visibility("default")))
     void glEvalCoord1fv(const GLfloat * u);

__attribute__ ((visibility("default")))
     void glEvalCoord2d(GLdouble u, GLdouble v);

__attribute__ ((visibility("default")))
     void glEvalCoord2f(GLfloat u, GLfloat v);

__attribute__ ((visibility("default")))
     void glEvalCoord2dv(const GLdouble * u);

__attribute__ ((visibility("default")))
     void glEvalCoord2fv(const GLfloat * u);

__attribute__ ((visibility("default")))
     void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);

__attribute__ ((visibility("default")))
     void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);

__attribute__ ((visibility("default")))
     void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2,
	GLint vn, GLdouble v1, GLdouble v2);
__attribute__ ((visibility("default")))
     void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2,
	GLint vn, GLfloat v1, GLfloat v2);

__attribute__ ((visibility("default")))
     void glEvalPoint1(GLint i);

__attribute__ ((visibility("default")))
     void glEvalPoint2(GLint i, GLint j);

__attribute__ ((visibility("default")))
     void glEvalMesh1(GLenum mode, GLint i1, GLint i2);

__attribute__ ((visibility("default")))
     void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);



__attribute__ ((visibility("default")))
     void glFogf(GLenum pname, GLfloat param);

__attribute__ ((visibility("default")))
     void glFogi(GLenum pname, GLint param);

__attribute__ ((visibility("default")))
     void glFogfv(GLenum pname, const GLfloat * params);

__attribute__ ((visibility("default")))
     void glFogiv(GLenum pname, const GLint * params);



__attribute__ ((visibility("default")))
     void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat * buffer);

__attribute__ ((visibility("default")))
     void glPassThrough(GLfloat token);

__attribute__ ((visibility("default")))
     void glSelectBuffer(GLsizei size, GLuint * buffer);

__attribute__ ((visibility("default")))
     void glInitNames(void);

__attribute__ ((visibility("default")))
     void glLoadName(GLuint name);

__attribute__ ((visibility("default")))
     void glPushName(GLuint name);

__attribute__ ((visibility("default")))
     void glPopName(void);

__attribute__ ((visibility("default")))
     void glDrawRangeElements(GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid * indices);

__attribute__ ((visibility("default")))
     void glTexImage3D(GLenum target, GLint level,
	GLint internalFormat,
	GLsizei width, GLsizei height,
	GLsizei depth, GLint border,
	GLenum format, GLenum type, const GLvoid * pixels);

__attribute__ ((visibility("default")))
     void glTexSubImage3D(GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLint zoffset, GLsizei width,
	GLsizei height, GLsizei depth,
	GLenum format, GLenum type, const GLvoid * pixels);

__attribute__ ((visibility("default")))
     void glCopyTexSubImage3D(GLenum target, GLint level,
	GLint xoffset, GLint yoffset,
	GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

     typedef void (*PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
     typedef void (*PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level,
	GLint internalformat, GLsizei width, GLsizei height, GLsizei depth,
	GLint border, GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level,
	GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width,
	GLsizei height, GLsizei depth, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level,
	GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y,
	GLsizei width, GLsizei height);
__attribute__ ((visibility("default")))
     void glColorTable(GLenum target, GLenum internalformat,
	GLsizei width, GLenum format, GLenum type, const GLvoid * table);

__attribute__ ((visibility("default")))
     void glColorSubTable(GLenum target,
	GLsizei start, GLsizei count,
	GLenum format, GLenum type, const GLvoid * data);

__attribute__ ((visibility("default")))
     void glColorTableParameteriv(GLenum target, GLenum pname,
	const GLint * params);

__attribute__ ((visibility("default")))
     void glColorTableParameterfv(GLenum target, GLenum pname,
	const GLfloat * params);

__attribute__ ((visibility("default")))
     void glCopyColorSubTable(GLenum target, GLsizei start,
	GLint x, GLint y, GLsizei width);

__attribute__ ((visibility("default")))
     void glCopyColorTable(GLenum target, GLenum internalformat,
	GLint x, GLint y, GLsizei width);

__attribute__ ((visibility("default")))
     void glGetColorTable(GLenum target, GLenum format,
	GLenum type, GLvoid * table);

__attribute__ ((visibility("default")))
     void glGetColorTableParameterfv(GLenum target, GLenum pname,
	GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetColorTableParameteriv(GLenum target, GLenum pname,
	GLint * params);

__attribute__ ((visibility("default")))
     void glBlendEquation(GLenum mode);

__attribute__ ((visibility("default")))
     void glBlendColor(GLclampf red, GLclampf green,
	GLclampf blue, GLclampf alpha);

__attribute__ ((visibility("default")))
     void glHistogram(GLenum target, GLsizei width,
	GLenum internalformat, GLboolean sink);

__attribute__ ((visibility("default")))
     void glResetHistogram(GLenum target);

__attribute__ ((visibility("default")))
     void glGetHistogram(GLenum target, GLboolean reset,
	GLenum format, GLenum type, GLvoid * values);

__attribute__ ((visibility("default")))
     void glGetHistogramParameterfv(GLenum target, GLenum pname,
	GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetHistogramParameteriv(GLenum target, GLenum pname,
	GLint * params);

__attribute__ ((visibility("default")))
     void glMinmax(GLenum target, GLenum internalformat, GLboolean sink);

__attribute__ ((visibility("default")))
     void glResetMinmax(GLenum target);

__attribute__ ((visibility("default")))
     void glGetMinmax(GLenum target, GLboolean reset,
	GLenum format, GLenum types, GLvoid * values);

__attribute__ ((visibility("default")))
     void glGetMinmaxParameterfv(GLenum target, GLenum pname,
	GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint * params);

__attribute__ ((visibility("default")))
     void glConvolutionFilter1D(GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid * image);

__attribute__ ((visibility("default")))
     void glConvolutionFilter2D(GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid * image);

__attribute__ ((visibility("default")))
     void glConvolutionParameterf(GLenum target, GLenum pname,
	GLfloat params);

__attribute__ ((visibility("default")))
     void glConvolutionParameterfv(GLenum target, GLenum pname,
	const GLfloat * params);

__attribute__ ((visibility("default")))
     void glConvolutionParameteri(GLenum target, GLenum pname, GLint params);

__attribute__ ((visibility("default")))
     void glConvolutionParameteriv(GLenum target, GLenum pname,
	const GLint * params);

__attribute__ ((visibility("default")))
     void glCopyConvolutionFilter1D(GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width);

__attribute__ ((visibility("default")))
     void glCopyConvolutionFilter2D(GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height);

__attribute__ ((visibility("default")))
     void glGetConvolutionFilter(GLenum target, GLenum format,
	GLenum type, GLvoid * image);

__attribute__ ((visibility("default")))
     void glGetConvolutionParameterfv(GLenum target, GLenum pname,
	GLfloat * params);

__attribute__ ((visibility("default")))
     void glGetConvolutionParameteriv(GLenum target, GLenum pname,
	GLint * params);

__attribute__ ((visibility("default")))
     void glSeparableFilter2D(GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid * row, const GLvoid * column);

__attribute__ ((visibility("default")))
     void glGetSeparableFilter(GLenum target, GLenum format,
	GLenum type, GLvoid * row, GLvoid * column, GLvoid * span);

     typedef void (*PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green,
	GLclampf blue, GLclampf alpha);
     typedef void (*PFNGLBLENDEQUATIONPROC) (GLenum mode);

__attribute__ ((visibility("default")))
     void glActiveTexture(GLenum texture);

__attribute__ ((visibility("default")))
     void glClientActiveTexture(GLenum texture);

__attribute__ ((visibility("default")))
     void glCompressedTexImage1D(GLenum target, GLint level,
	GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize,
	const GLvoid * data);

__attribute__ ((visibility("default")))
     void glCompressedTexImage2D(GLenum target, GLint level,
	GLenum internalformat, GLsizei width, GLsizei height, GLint border,
	GLsizei imageSize, const GLvoid * data);

__attribute__ ((visibility("default")))
     void glCompressedTexImage3D(GLenum target, GLint level,
	GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
	GLint border, GLsizei imageSize, const GLvoid * data);

__attribute__ ((visibility("default")))
     void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset,
	GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data);

__attribute__ ((visibility("default")))
     void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset,
	GLint yoffset, GLsizei width, GLsizei height, GLenum format,
	GLsizei imageSize, const GLvoid * data);

__attribute__ ((visibility("default")))
     void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset,
	GLint yoffset, GLint zoffset, GLsizei width, GLsizei height,
	GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);

__attribute__ ((visibility("default")))
     void glGetCompressedTexImage(GLenum target, GLint lod, GLvoid * img);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1d(GLenum target, GLdouble s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1dv(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1f(GLenum target, GLfloat s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1fv(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1i(GLenum target, GLint s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1iv(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1s(GLenum target, GLshort s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1sv(GLenum target, const GLshort * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2dv(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2fv(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2i(GLenum target, GLint s, GLint t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2iv(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2sv(GLenum target, const GLshort * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t,
	GLdouble r);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3dv(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3fv(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3iv(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3sv(GLenum target, const GLshort * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r,
	GLdouble q);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4dv(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r,
	GLfloat q);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4fv(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r,
	GLint q);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4iv(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r,
	GLshort q);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4sv(GLenum target, const GLshort * v);


__attribute__ ((visibility("default")))
     void glLoadTransposeMatrixd(const GLdouble m[16]);

__attribute__ ((visibility("default")))
     void glLoadTransposeMatrixf(const GLfloat m[16]);

__attribute__ ((visibility("default")))
     void glMultTransposeMatrixd(const GLdouble m[16]);

__attribute__ ((visibility("default")))
     void glMultTransposeMatrixf(const GLfloat m[16]);

__attribute__ ((visibility("default")))
     void glSampleCoverage(GLclampf value, GLboolean invert);


     typedef void (*PFNGLACTIVETEXTUREPROC) (GLenum texture);
     typedef void (*PFNGLSAMPLECOVERAGEPROC) (GLclampf value,
	GLboolean invert);
     typedef void (*PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target,
	GLint level, GLenum internalformat, GLsizei width, GLsizei height,
	GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target,
	GLint level, GLenum internalformat, GLsizei width, GLsizei height,
	GLint border, GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target,
	GLint level, GLenum internalformat, GLsizei width, GLint border,
	GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target,
	GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format,
	GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target,
	GLint level, GLint xoffset, GLint yoffset, GLsizei width,
	GLsizei height, GLenum format, GLsizei imageSize,
	const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target,
	GLint level, GLint xoffset, GLsizei width, GLenum format,
	GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target,
	GLint level, GLvoid * img);
__attribute__ ((visibility("default")))
     void glActiveTextureARB(GLenum texture);

__attribute__ ((visibility("default")))
     void glClientActiveTextureARB(GLenum texture);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1dARB(GLenum target, GLdouble s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1dvARB(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1fARB(GLenum target, GLfloat s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1fvARB(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1iARB(GLenum target, GLint s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1ivARB(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1sARB(GLenum target, GLshort s);

__attribute__ ((visibility("default")))
     void glMultiTexCoord1svARB(GLenum target, const GLshort * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2dvARB(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2fvARB(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2iARB(GLenum target, GLint s, GLint t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2ivARB(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t);

__attribute__ ((visibility("default")))
     void glMultiTexCoord2svARB(GLenum target, const GLshort * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t,
	GLdouble r);
__attribute__ ((visibility("default")))
     void glMultiTexCoord3dvARB(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t,
	GLfloat r);
__attribute__ ((visibility("default")))
     void glMultiTexCoord3fvARB(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3ivARB(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t,
	GLshort r);
__attribute__ ((visibility("default")))
     void glMultiTexCoord3svARB(GLenum target, const GLshort * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t,
	GLdouble r, GLdouble q);
__attribute__ ((visibility("default")))
     void glMultiTexCoord4dvARB(GLenum target, const GLdouble * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r,
	GLfloat q);
__attribute__ ((visibility("default")))
     void glMultiTexCoord4fvARB(GLenum target, const GLfloat * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r,
	GLint q);
__attribute__ ((visibility("default")))
     void glMultiTexCoord4ivARB(GLenum target, const GLint * v);

__attribute__ ((visibility("default")))
     void glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r,
	GLshort q);
__attribute__ ((visibility("default")))
     void glMultiTexCoord4svARB(GLenum target, const GLshort * v);

     typedef void (*PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
     typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
     typedef void (*PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
     typedef void (*PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
     typedef void (*PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
     typedef void (*PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
     typedef void (*PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target,
	const GLshort * v);
     typedef void (*PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s,
	GLdouble t);
     typedef void (*PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s,
	GLfloat t);
     typedef void (*PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s,
	GLint t);
     typedef void (*PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s,
	GLshort t);
     typedef void (*PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target,
	const GLshort * v);
     typedef void (*PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s,
	GLdouble t, GLdouble r);
     typedef void (*PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s,
	GLfloat t, GLfloat r);
     typedef void (*PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s,
	GLint t, GLint r);
     typedef void (*PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s,
	GLshort t, GLshort r);
     typedef void (*PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target,
	const GLshort * v);
     typedef void (*PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s,
	GLdouble t, GLdouble r, GLdouble q);
     typedef void (*PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s,
	GLfloat t, GLfloat r, GLfloat q);
     typedef void (*PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s,
	GLint t, GLint r, GLint q);
     typedef void (*PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s,
	GLshort t, GLshort r, GLshort q);
     typedef void (*PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target,
	const GLshort * v);
     typedef char GLchar;




     typedef ptrdiff_t GLintptr;
     typedef ptrdiff_t GLsizeiptr;




     typedef ptrdiff_t GLintptrARB;
     typedef ptrdiff_t GLsizeiptrARB;




     typedef char GLcharARB;
     typedef unsigned int GLhandleARB;




     typedef unsigned short GLhalfARB;



     typedef unsigned short GLhalfNV;
     typedef long long GLint64EXT;
     typedef unsigned long long GLuint64EXT;



     typedef long long GLint64;
     typedef unsigned long long GLuint64;
     typedef struct __GLsync *GLsync;




     struct _cl_context;
     struct _cl_event;



     typedef void (*GLDEBUGPROCARB) (GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar * message,
	GLvoid * userParam);



     typedef void (*GLDEBUGPROCAMD) (GLuint id, GLenum category,
	GLenum severity, GLsizei length, const GLchar * message,
	GLvoid * userParam);



     typedef GLintptr GLvdpauSurfaceNV;
     typedef void (*PFNGLCOLORTABLEPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid * table);
     typedef void (*PFNGLCOLORTABLEPARAMETERFVPROC) (GLenum target,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLCOLORTABLEPARAMETERIVPROC) (GLenum target,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLCOPYCOLORTABLEPROC) (GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width);
     typedef void (*PFNGLGETCOLORTABLEPROC) (GLenum target, GLenum format,
	GLenum type, GLvoid * table);
     typedef void (*PFNGLGETCOLORTABLEPARAMETERFVPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETCOLORTABLEPARAMETERIVPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLCOLORSUBTABLEPROC) (GLenum target, GLsizei start,
	GLsizei count, GLenum format, GLenum type, const GLvoid * data);
     typedef void (*PFNGLCOPYCOLORSUBTABLEPROC) (GLenum target, GLsizei start,
	GLint x, GLint y, GLsizei width);
     typedef void (*PFNGLCONVOLUTIONFILTER1DPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid * image);
     typedef void (*PFNGLCONVOLUTIONFILTER2DPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid * image);
     typedef void (*PFNGLCONVOLUTIONPARAMETERFPROC) (GLenum target,
	GLenum pname, GLfloat params);
     typedef void (*PFNGLCONVOLUTIONPARAMETERFVPROC) (GLenum target,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLCONVOLUTIONPARAMETERIPROC) (GLenum target,
	GLenum pname, GLint params);
     typedef void (*PFNGLCONVOLUTIONPARAMETERIVPROC) (GLenum target,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLCOPYCONVOLUTIONFILTER1DPROC) (GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width);
     typedef void (*PFNGLCOPYCONVOLUTIONFILTER2DPROC) (GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height);
     typedef void (*PFNGLGETCONVOLUTIONFILTERPROC) (GLenum target,
	GLenum format, GLenum type, GLvoid * image);
     typedef void (*PFNGLGETCONVOLUTIONPARAMETERFVPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETCONVOLUTIONPARAMETERIVPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETSEPARABLEFILTERPROC) (GLenum target,
	GLenum format, GLenum type, GLvoid * row, GLvoid * column,
	GLvoid * span);
     typedef void (*PFNGLSEPARABLEFILTER2DPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid * row, const GLvoid * column);
     typedef void (*PFNGLGETHISTOGRAMPROC) (GLenum target, GLboolean reset,
	GLenum format, GLenum type, GLvoid * values);
     typedef void (*PFNGLGETHISTOGRAMPARAMETERFVPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETHISTOGRAMPARAMETERIVPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETMINMAXPROC) (GLenum target, GLboolean reset,
	GLenum format, GLenum type, GLvoid * values);
     typedef void (*PFNGLGETMINMAXPARAMETERFVPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETMINMAXPARAMETERIVPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLHISTOGRAMPROC) (GLenum target, GLsizei width,
	GLenum internalformat, GLboolean sink);
     typedef void (*PFNGLMINMAXPROC) (GLenum target, GLenum internalformat,
	GLboolean sink);
     typedef void (*PFNGLRESETHISTOGRAMPROC) (GLenum target);
     typedef void (*PFNGLRESETMINMAXPROC) (GLenum target);
     typedef void (*PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
     typedef void (*PFNGLMULTITEXCOORD1DPROC) (GLenum target, GLdouble s);
     typedef void (*PFNGLMULTITEXCOORD1DVPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD1FPROC) (GLenum target, GLfloat s);
     typedef void (*PFNGLMULTITEXCOORD1FVPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD1IPROC) (GLenum target, GLint s);
     typedef void (*PFNGLMULTITEXCOORD1IVPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD1SPROC) (GLenum target, GLshort s);
     typedef void (*PFNGLMULTITEXCOORD1SVPROC) (GLenum target,
	const GLshort * v);
     typedef void (*PFNGLMULTITEXCOORD2DPROC) (GLenum target, GLdouble s,
	GLdouble t);
     typedef void (*PFNGLMULTITEXCOORD2DVPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD2FPROC) (GLenum target, GLfloat s,
	GLfloat t);
     typedef void (*PFNGLMULTITEXCOORD2FVPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD2IPROC) (GLenum target, GLint s,
	GLint t);
     typedef void (*PFNGLMULTITEXCOORD2IVPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD2SPROC) (GLenum target, GLshort s,
	GLshort t);
     typedef void (*PFNGLMULTITEXCOORD2SVPROC) (GLenum target,
	const GLshort * v);
     typedef void (*PFNGLMULTITEXCOORD3DPROC) (GLenum target, GLdouble s,
	GLdouble t, GLdouble r);
     typedef void (*PFNGLMULTITEXCOORD3DVPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD3FPROC) (GLenum target, GLfloat s,
	GLfloat t, GLfloat r);
     typedef void (*PFNGLMULTITEXCOORD3FVPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD3IPROC) (GLenum target, GLint s,
	GLint t, GLint r);
     typedef void (*PFNGLMULTITEXCOORD3IVPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD3SPROC) (GLenum target, GLshort s,
	GLshort t, GLshort r);
     typedef void (*PFNGLMULTITEXCOORD3SVPROC) (GLenum target,
	const GLshort * v);
     typedef void (*PFNGLMULTITEXCOORD4DPROC) (GLenum target, GLdouble s,
	GLdouble t, GLdouble r, GLdouble q);
     typedef void (*PFNGLMULTITEXCOORD4DVPROC) (GLenum target,
	const GLdouble * v);
     typedef void (*PFNGLMULTITEXCOORD4FPROC) (GLenum target, GLfloat s,
	GLfloat t, GLfloat r, GLfloat q);
     typedef void (*PFNGLMULTITEXCOORD4FVPROC) (GLenum target,
	const GLfloat * v);
     typedef void (*PFNGLMULTITEXCOORD4IPROC) (GLenum target, GLint s,
	GLint t, GLint r, GLint q);
     typedef void (*PFNGLMULTITEXCOORD4IVPROC) (GLenum target,
	const GLint * v);
     typedef void (*PFNGLMULTITEXCOORD4SPROC) (GLenum target, GLshort s,
	GLshort t, GLshort r, GLshort q);
     typedef void (*PFNGLMULTITEXCOORD4SVPROC) (GLenum target,
	const GLshort * v);
     typedef void (*PFNGLLOADTRANSPOSEMATRIXFPROC) (const GLfloat * m);
     typedef void (*PFNGLLOADTRANSPOSEMATRIXDPROC) (const GLdouble * m);
     typedef void (*PFNGLMULTTRANSPOSEMATRIXFPROC) (const GLfloat * m);
     typedef void (*PFNGLMULTTRANSPOSEMATRIXDPROC) (const GLdouble * m);
     typedef void (*PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB,
	GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
     typedef void (*PFNGLMULTIDRAWARRAYSPROC) (GLenum mode,
	const GLint * first, const GLsizei * count, GLsizei primcount);
     typedef void (*PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode,
	const GLsizei * count, GLenum type, const GLvoid * *indices,
	GLsizei primcount);
     typedef void (*PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
     typedef void (*PFNGLPOINTPARAMETERFVPROC) (GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
     typedef void (*PFNGLPOINTPARAMETERIVPROC) (GLenum pname,
	const GLint * params);
     typedef void (*PFNGLFOGCOORDFPROC) (GLfloat coord);
     typedef void (*PFNGLFOGCOORDFVPROC) (const GLfloat * coord);
     typedef void (*PFNGLFOGCOORDDPROC) (GLdouble coord);
     typedef void (*PFNGLFOGCOORDDVPROC) (const GLdouble * coord);
     typedef void (*PFNGLFOGCOORDPOINTERPROC) (GLenum type, GLsizei stride,
	const GLvoid * pointer);
     typedef void (*PFNGLSECONDARYCOLOR3BPROC) (GLbyte red, GLbyte green,
	GLbyte blue);
     typedef void (*PFNGLSECONDARYCOLOR3BVPROC) (const GLbyte * v);
     typedef void (*PFNGLSECONDARYCOLOR3DPROC) (GLdouble red, GLdouble green,
	GLdouble blue);
     typedef void (*PFNGLSECONDARYCOLOR3DVPROC) (const GLdouble * v);
     typedef void (*PFNGLSECONDARYCOLOR3FPROC) (GLfloat red, GLfloat green,
	GLfloat blue);
     typedef void (*PFNGLSECONDARYCOLOR3FVPROC) (const GLfloat * v);
     typedef void (*PFNGLSECONDARYCOLOR3IPROC) (GLint red, GLint green,
	GLint blue);
     typedef void (*PFNGLSECONDARYCOLOR3IVPROC) (const GLint * v);
     typedef void (*PFNGLSECONDARYCOLOR3SPROC) (GLshort red, GLshort green,
	GLshort blue);
     typedef void (*PFNGLSECONDARYCOLOR3SVPROC) (const GLshort * v);
     typedef void (*PFNGLSECONDARYCOLOR3UBPROC) (GLubyte red, GLubyte green,
	GLubyte blue);
     typedef void (*PFNGLSECONDARYCOLOR3UBVPROC) (const GLubyte * v);
     typedef void (*PFNGLSECONDARYCOLOR3UIPROC) (GLuint red, GLuint green,
	GLuint blue);
     typedef void (*PFNGLSECONDARYCOLOR3UIVPROC) (const GLuint * v);
     typedef void (*PFNGLSECONDARYCOLOR3USPROC) (GLushort red, GLushort green,
	GLushort blue);
     typedef void (*PFNGLSECONDARYCOLOR3USVPROC) (const GLushort * v);
     typedef void (*PFNGLSECONDARYCOLORPOINTERPROC) (GLint size, GLenum type,
	GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLWINDOWPOS2DPROC) (GLdouble x, GLdouble y);
     typedef void (*PFNGLWINDOWPOS2DVPROC) (const GLdouble * v);
     typedef void (*PFNGLWINDOWPOS2FPROC) (GLfloat x, GLfloat y);
     typedef void (*PFNGLWINDOWPOS2FVPROC) (const GLfloat * v);
     typedef void (*PFNGLWINDOWPOS2IPROC) (GLint x, GLint y);
     typedef void (*PFNGLWINDOWPOS2IVPROC) (const GLint * v);
     typedef void (*PFNGLWINDOWPOS2SPROC) (GLshort x, GLshort y);
     typedef void (*PFNGLWINDOWPOS2SVPROC) (const GLshort * v);
     typedef void (*PFNGLWINDOWPOS3DPROC) (GLdouble x, GLdouble y,
	GLdouble z);
     typedef void (*PFNGLWINDOWPOS3DVPROC) (const GLdouble * v);
     typedef void (*PFNGLWINDOWPOS3FPROC) (GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLWINDOWPOS3FVPROC) (const GLfloat * v);
     typedef void (*PFNGLWINDOWPOS3IPROC) (GLint x, GLint y, GLint z);
     typedef void (*PFNGLWINDOWPOS3IVPROC) (const GLint * v);
     typedef void (*PFNGLWINDOWPOS3SPROC) (GLshort x, GLshort y, GLshort z);
     typedef void (*PFNGLWINDOWPOS3SVPROC) (const GLshort * v);
     typedef void (*PFNGLGENQUERIESPROC) (GLsizei n, GLuint * ids);
     typedef void (*PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint * ids);
     typedef GLboolean(*PFNGLISQUERYPROC) (GLuint id);
     typedef void (*PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
     typedef void (*PFNGLENDQUERYPROC) (GLenum target);
     typedef void (*PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname,
	GLuint * params);
     typedef void (*PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
     typedef void (*PFNGLDELETEBUFFERSPROC) (GLsizei n,
	const GLuint * buffers);
     typedef void (*PFNGLGENBUFFERSPROC) (GLsizei n, GLuint * buffers);
     typedef GLboolean(*PFNGLISBUFFERPROC) (GLuint buffer);
     typedef void (*PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size,
	const GLvoid * data, GLenum usage);
     typedef void (*PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset,
	GLsizeiptr size, const GLvoid * data);
     typedef void (*PFNGLGETBUFFERSUBDATAPROC) (GLenum target,
	GLintptr offset, GLsizeiptr size, GLvoid * data);
     typedef GLvoid *(*PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
     typedef GLboolean(*PFNGLUNMAPBUFFERPROC) (GLenum target);
     typedef void (*PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname,
	GLvoid * *params);
     typedef void (*PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB,
	GLenum modeAlpha);
     typedef void (*PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum * bufs);
     typedef void (*PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail,
	GLenum dpfail, GLenum dppass);
     typedef void (*PFNGLSTENCILFUNCSEPARATEPROC) (GLenum face, GLenum func,
	GLint ref, GLuint mask);
     typedef void (*PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
     typedef void (*PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
     typedef void (*PFNGLBINDATTRIBLOCATIONPROC) (GLuint program,
	GLuint index, const GLchar * name);
     typedef void (*PFNGLCOMPILESHADERPROC) (GLuint shader);
     typedef GLuint(*PFNGLCREATEPROGRAMPROC) (void);
     typedef GLuint(*PFNGLCREATESHADERPROC) (GLenum type);
     typedef void (*PFNGLDELETEPROGRAMPROC) (GLuint program);
     typedef void (*PFNGLDELETESHADERPROC) (GLuint shader);
     typedef void (*PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
     typedef void (*PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
     typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
     typedef void (*PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index,
	GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type,
	GLchar * name);
     typedef void (*PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index,
	GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type,
	GLchar * name);
     typedef void (*PFNGLGETATTACHEDSHADERSPROC) (GLuint program,
	GLsizei maxCount, GLsizei * count, GLuint * obj);
     typedef GLint(*PFNGLGETATTRIBLOCATIONPROC) (GLuint program,
	const GLchar * name);
     typedef void (*PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETPROGRAMINFOLOGPROC) (GLuint program,
	GLsizei bufSize, GLsizei * length, GLchar * infoLog);
     typedef void (*PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETSHADERINFOLOGPROC) (GLuint shader,
	GLsizei bufSize, GLsizei * length, GLchar * infoLog);
     typedef void (*PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize,
	GLsizei * length, GLchar * source);
     typedef GLint(*PFNGLGETUNIFORMLOCATIONPROC) (GLuint program,
	const GLchar * name);
     typedef void (*PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location,
	GLfloat * params);
     typedef void (*PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location,
	GLint * params);
     typedef void (*PFNGLGETVERTEXATTRIBDVPROC) (GLuint index, GLenum pname,
	GLdouble * params);
     typedef void (*PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname,
	GLfloat * params);
     typedef void (*PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index,
	GLenum pname, GLvoid * *pointer);
     typedef GLboolean(*PFNGLISPROGRAMPROC) (GLuint program);
     typedef GLboolean(*PFNGLISSHADERPROC) (GLuint shader);
     typedef void (*PFNGLLINKPROGRAMPROC) (GLuint program);
     typedef void (*PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count,
	const GLchar * *string, const GLint * length);
     typedef void (*PFNGLUSEPROGRAMPROC) (GLuint program);
     typedef void (*PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
     typedef void (*PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0,
	GLfloat v1);
     typedef void (*PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0,
	GLfloat v1, GLfloat v2);
     typedef void (*PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0,
	GLfloat v1, GLfloat v2, GLfloat v3);
     typedef void (*PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
     typedef void (*PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
     typedef void (*PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1,
	GLint v2);
     typedef void (*PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1,
	GLint v2, GLint v3);
     typedef void (*PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count,
	GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count,
	GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count,
	GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLVALIDATEPROGRAMPROC) (GLuint program);
     typedef void (*PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
     typedef void (*PFNGLVERTEXATTRIB1DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
     typedef void (*PFNGLVERTEXATTRIB1FVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
     typedef void (*PFNGLVERTEXATTRIB1SVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x,
	GLdouble y);
     typedef void (*PFNGLVERTEXATTRIB2DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x,
	GLfloat y);
     typedef void (*PFNGLVERTEXATTRIB2FVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x,
	GLshort y);
     typedef void (*PFNGLVERTEXATTRIB2SVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLVERTEXATTRIB3DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x,
	GLfloat y, GLfloat z);
     typedef void (*PFNGLVERTEXATTRIB3FVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x,
	GLshort y, GLshort z);
     typedef void (*PFNGLVERTEXATTRIB3SVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4NBVPROC) (GLuint index,
	const GLbyte * v);
     typedef void (*PFNGLVERTEXATTRIB4NIVPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIB4NSVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x,
	GLubyte y, GLubyte z, GLubyte w);
     typedef void (*PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index,
	const GLubyte * v);
     typedef void (*PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index,
	const GLushort * v);
     typedef void (*PFNGLVERTEXATTRIB4BVPROC) (GLuint index,
	const GLbyte * v);
     typedef void (*PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLVERTEXATTRIB4DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x,
	GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLVERTEXATTRIB4FVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint * v);
     typedef void (*PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x,
	GLshort y, GLshort z, GLshort w);
     typedef void (*PFNGLVERTEXATTRIB4SVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4UBVPROC) (GLuint index,
	const GLubyte * v);
     typedef void (*PFNGLVERTEXATTRIB4UIVPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIB4USVPROC) (GLuint index,
	const GLushort * v);
     typedef void (*PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size,
	GLenum type, GLboolean normalized, GLsizei stride,
	const GLvoid * pointer);
     typedef void (*PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLCOLORMASKIPROC) (GLuint index, GLboolean r,
	GLboolean g, GLboolean b, GLboolean a);
     typedef void (*PFNGLGETBOOLEANI_VPROC) (GLenum target, GLuint index,
	GLboolean * data);
     typedef void (*PFNGLGETINTEGERI_VPROC) (GLenum target, GLuint index,
	GLint * data);
     typedef void (*PFNGLENABLEIPROC) (GLenum target, GLuint index);
     typedef void (*PFNGLDISABLEIPROC) (GLenum target, GLuint index);
     typedef GLboolean(*PFNGLISENABLEDIPROC) (GLenum target, GLuint index);
     typedef void (*PFNGLBEGINTRANSFORMFEEDBACKPROC) (GLenum primitiveMode);
     typedef void (*PFNGLENDTRANSFORMFEEDBACKPROC) (void);
     typedef void (*PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index,
	GLuint buffer, GLintptr offset, GLsizeiptr size);
     typedef void (*PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index,
	GLuint buffer);
     typedef void (*PFNGLTRANSFORMFEEDBACKVARYINGSPROC) (GLuint program,
	GLsizei count, const GLchar * *varyings, GLenum bufferMode);
     typedef void (*PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) (GLuint program,
	GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size,
	GLenum * type, GLchar * name);
     typedef void (*PFNGLCLAMPCOLORPROC) (GLenum target, GLenum clamp);
     typedef void (*PFNGLBEGINCONDITIONALRENDERPROC) (GLuint id, GLenum mode);
     typedef void (*PFNGLENDCONDITIONALRENDERPROC) (void);
     typedef void (*PFNGLVERTEXATTRIBIPOINTERPROC) (GLuint index, GLint size,
	GLenum type, GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLGETVERTEXATTRIBIIVPROC) (GLuint index, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETVERTEXATTRIBIUIVPROC) (GLuint index, GLenum pname,
	GLuint * params);
     typedef void (*PFNGLVERTEXATTRIBI1IPROC) (GLuint index, GLint x);
     typedef void (*PFNGLVERTEXATTRIBI2IPROC) (GLuint index, GLint x,
	GLint y);
     typedef void (*PFNGLVERTEXATTRIBI3IPROC) (GLuint index, GLint x, GLint y,
	GLint z);
     typedef void (*PFNGLVERTEXATTRIBI4IPROC) (GLuint index, GLint x, GLint y,
	GLint z, GLint w);
     typedef void (*PFNGLVERTEXATTRIBI1UIPROC) (GLuint index, GLuint x);
     typedef void (*PFNGLVERTEXATTRIBI2UIPROC) (GLuint index, GLuint x,
	GLuint y);
     typedef void (*PFNGLVERTEXATTRIBI3UIPROC) (GLuint index, GLuint x,
	GLuint y, GLuint z);
     typedef void (*PFNGLVERTEXATTRIBI4UIPROC) (GLuint index, GLuint x,
	GLuint y, GLuint z, GLuint w);
     typedef void (*PFNGLVERTEXATTRIBI1IVPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI2IVPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI3IVPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI4IVPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI1UIVPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI2UIVPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI3UIVPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI4UIVPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI4BVPROC) (GLuint index,
	const GLbyte * v);
     typedef void (*PFNGLVERTEXATTRIBI4SVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIBI4UBVPROC) (GLuint index,
	const GLubyte * v);
     typedef void (*PFNGLVERTEXATTRIBI4USVPROC) (GLuint index,
	const GLushort * v);
     typedef void (*PFNGLGETUNIFORMUIVPROC) (GLuint program, GLint location,
	GLuint * params);
     typedef void (*PFNGLBINDFRAGDATALOCATIONPROC) (GLuint program,
	GLuint color, const GLchar * name);
     typedef GLint(*PFNGLGETFRAGDATALOCATIONPROC) (GLuint program,
	const GLchar * name);
     typedef void (*PFNGLUNIFORM1UIPROC) (GLint location, GLuint v0);
     typedef void (*PFNGLUNIFORM2UIPROC) (GLint location, GLuint v0,
	GLuint v1);
     typedef void (*PFNGLUNIFORM3UIPROC) (GLint location, GLuint v0,
	GLuint v1, GLuint v2);
     typedef void (*PFNGLUNIFORM4UIPROC) (GLint location, GLuint v0,
	GLuint v1, GLuint v2, GLuint v3);
     typedef void (*PFNGLUNIFORM1UIVPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLUNIFORM2UIVPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLUNIFORM3UIVPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLUNIFORM4UIVPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLTEXPARAMETERIIVPROC) (GLenum target, GLenum pname,
	const GLint * params);
     typedef void (*PFNGLTEXPARAMETERIUIVPROC) (GLenum target, GLenum pname,
	const GLuint * params);
     typedef void (*PFNGLGETTEXPARAMETERIIVPROC) (GLenum target, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETTEXPARAMETERIUIVPROC) (GLenum target,
	GLenum pname, GLuint * params);
     typedef void (*PFNGLCLEARBUFFERIVPROC) (GLenum buffer, GLint drawbuffer,
	const GLint * value);
     typedef void (*PFNGLCLEARBUFFERUIVPROC) (GLenum buffer, GLint drawbuffer,
	const GLuint * value);
     typedef void (*PFNGLCLEARBUFFERFVPROC) (GLenum buffer, GLint drawbuffer,
	const GLfloat * value);
     typedef void (*PFNGLCLEARBUFFERFIPROC) (GLenum buffer, GLint drawbuffer,
	GLfloat depth, GLint stencil);
     typedef const GLubyte *(*PFNGLGETSTRINGIPROC) (GLenum name,
	GLuint index);
     typedef void (*PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first,
	GLsizei count, GLsizei primcount);
     typedef void (*PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum mode,
	GLsizei count, GLenum type, const GLvoid * indices,
	GLsizei primcount);
     typedef void (*PFNGLTEXBUFFERPROC) (GLenum target, GLenum internalformat,
	GLuint buffer);
     typedef void (*PFNGLPRIMITIVERESTARTINDEXPROC) (GLuint index);
     typedef void (*PFNGLGETINTEGER64I_VPROC) (GLenum target, GLuint index,
	GLint64 * data);
     typedef void (*PFNGLGETBUFFERPARAMETERI64VPROC) (GLenum target,
	GLenum pname, GLint64 * params);
     typedef void (*PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level);
     typedef void (*PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index,
	GLuint divisor);
     typedef void (*PFNGLMINSAMPLESHADINGPROC) (GLclampf value);
     typedef void (*PFNGLBLENDEQUATIONIPROC) (GLuint buf, GLenum mode);
     typedef void (*PFNGLBLENDEQUATIONSEPARATEIPROC) (GLuint buf,
	GLenum modeRGB, GLenum modeAlpha);
     typedef void (*PFNGLBLENDFUNCIPROC) (GLuint buf, GLenum src, GLenum dst);
     typedef void (*PFNGLBLENDFUNCSEPARATEIPROC) (GLuint buf, GLenum srcRGB,
	GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
     typedef void (*PFNGLLOADTRANSPOSEMATRIXFARBPROC) (const GLfloat * m);
     typedef void (*PFNGLLOADTRANSPOSEMATRIXDARBPROC) (const GLdouble * m);
     typedef void (*PFNGLMULTTRANSPOSEMATRIXFARBPROC) (const GLfloat * m);
     typedef void (*PFNGLMULTTRANSPOSEMATRIXDARBPROC) (const GLdouble * m);







     typedef void (*PFNGLSAMPLECOVERAGEARBPROC) (GLclampf value,
	GLboolean invert);
     typedef void (*PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target,
	GLint level, GLenum internalformat, GLsizei width, GLsizei height,
	GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target,
	GLint level, GLenum internalformat, GLsizei width, GLsizei height,
	GLint border, GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target,
	GLint level, GLenum internalformat, GLsizei width, GLint border,
	GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target,
	GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format,
	GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target,
	GLint level, GLint xoffset, GLint yoffset, GLsizei width,
	GLsizei height, GLenum format, GLsizei imageSize,
	const GLvoid * data);
     typedef void (*PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target,
	GLint level, GLint xoffset, GLsizei width, GLenum format,
	GLsizei imageSize, const GLvoid * data);
     typedef void (*PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target,
	GLint level, GLvoid * img);
     typedef void (*PFNGLPOINTPARAMETERFARBPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLPOINTPARAMETERFVARBPROC) (GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLWEIGHTBVARBPROC) (GLint size,
	const GLbyte * weights);
     typedef void (*PFNGLWEIGHTSVARBPROC) (GLint size,
	const GLshort * weights);
     typedef void (*PFNGLWEIGHTIVARBPROC) (GLint size, const GLint * weights);
     typedef void (*PFNGLWEIGHTFVARBPROC) (GLint size,
	const GLfloat * weights);
     typedef void (*PFNGLWEIGHTDVARBPROC) (GLint size,
	const GLdouble * weights);
     typedef void (*PFNGLWEIGHTUBVARBPROC) (GLint size,
	const GLubyte * weights);
     typedef void (*PFNGLWEIGHTUSVARBPROC) (GLint size,
	const GLushort * weights);
     typedef void (*PFNGLWEIGHTUIVARBPROC) (GLint size,
	const GLuint * weights);
     typedef void (*PFNGLWEIGHTPOINTERARBPROC) (GLint size, GLenum type,
	GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLVERTEXBLENDARBPROC) (GLint count);
     typedef void (*PFNGLCURRENTPALETTEMATRIXARBPROC) (GLint index);
     typedef void (*PFNGLMATRIXINDEXUBVARBPROC) (GLint size,
	const GLubyte * indices);
     typedef void (*PFNGLMATRIXINDEXUSVARBPROC) (GLint size,
	const GLushort * indices);
     typedef void (*PFNGLMATRIXINDEXUIVARBPROC) (GLint size,
	const GLuint * indices);
     typedef void (*PFNGLMATRIXINDEXPOINTERARBPROC) (GLint size, GLenum type,
	GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLWINDOWPOS2DARBPROC) (GLdouble x, GLdouble y);
     typedef void (*PFNGLWINDOWPOS2DVARBPROC) (const GLdouble * v);
     typedef void (*PFNGLWINDOWPOS2FARBPROC) (GLfloat x, GLfloat y);
     typedef void (*PFNGLWINDOWPOS2FVARBPROC) (const GLfloat * v);
     typedef void (*PFNGLWINDOWPOS2IARBPROC) (GLint x, GLint y);
     typedef void (*PFNGLWINDOWPOS2IVARBPROC) (const GLint * v);
     typedef void (*PFNGLWINDOWPOS2SARBPROC) (GLshort x, GLshort y);
     typedef void (*PFNGLWINDOWPOS2SVARBPROC) (const GLshort * v);
     typedef void (*PFNGLWINDOWPOS3DARBPROC) (GLdouble x, GLdouble y,
	GLdouble z);
     typedef void (*PFNGLWINDOWPOS3DVARBPROC) (const GLdouble * v);
     typedef void (*PFNGLWINDOWPOS3FARBPROC) (GLfloat x, GLfloat y,
	GLfloat z);
     typedef void (*PFNGLWINDOWPOS3FVARBPROC) (const GLfloat * v);
     typedef void (*PFNGLWINDOWPOS3IARBPROC) (GLint x, GLint y, GLint z);
     typedef void (*PFNGLWINDOWPOS3IVARBPROC) (const GLint * v);
     typedef void (*PFNGLWINDOWPOS3SARBPROC) (GLshort x, GLshort y,
	GLshort z);
     typedef void (*PFNGLWINDOWPOS3SVARBPROC) (const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB1DARBPROC) (GLuint index, GLdouble x);
     typedef void (*PFNGLVERTEXATTRIB1DVARBPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB1FARBPROC) (GLuint index, GLfloat x);
     typedef void (*PFNGLVERTEXATTRIB1FVARBPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB1SARBPROC) (GLuint index, GLshort x);
     typedef void (*PFNGLVERTEXATTRIB1SVARBPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB2DARBPROC) (GLuint index, GLdouble x,
	GLdouble y);
     typedef void (*PFNGLVERTEXATTRIB2DVARBPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x,
	GLfloat y);
     typedef void (*PFNGLVERTEXATTRIB2FVARBPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB2SARBPROC) (GLuint index, GLshort x,
	GLshort y);
     typedef void (*PFNGLVERTEXATTRIB2SVARBPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB3DARBPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLVERTEXATTRIB3DVARBPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB3FARBPROC) (GLuint index, GLfloat x,
	GLfloat y, GLfloat z);
     typedef void (*PFNGLVERTEXATTRIB3FVARBPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB3SARBPROC) (GLuint index, GLshort x,
	GLshort y, GLshort z);
     typedef void (*PFNGLVERTEXATTRIB3SVARBPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4NBVARBPROC) (GLuint index,
	const GLbyte * v);
     typedef void (*PFNGLVERTEXATTRIB4NIVARBPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIB4NSVARBPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4NUBARBPROC) (GLuint index, GLubyte x,
	GLubyte y, GLubyte z, GLubyte w);
     typedef void (*PFNGLVERTEXATTRIB4NUBVARBPROC) (GLuint index,
	const GLubyte * v);
     typedef void (*PFNGLVERTEXATTRIB4NUIVARBPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIB4NUSVARBPROC) (GLuint index,
	const GLushort * v);
     typedef void (*PFNGLVERTEXATTRIB4BVARBPROC) (GLuint index,
	const GLbyte * v);
     typedef void (*PFNGLVERTEXATTRIB4DARBPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLVERTEXATTRIB4DVARBPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x,
	GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLVERTEXATTRIB4FVARBPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB4IVARBPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIB4SARBPROC) (GLuint index, GLshort x,
	GLshort y, GLshort z, GLshort w);
     typedef void (*PFNGLVERTEXATTRIB4SVARBPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4UBVARBPROC) (GLuint index,
	const GLubyte * v);
     typedef void (*PFNGLVERTEXATTRIB4UIVARBPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIB4USVARBPROC) (GLuint index,
	const GLushort * v);
     typedef void (*PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index,
	GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	const GLvoid * pointer);
     typedef void (*PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
     typedef void (*PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
     typedef void (*PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format,
	GLsizei len, const GLvoid * string);
     typedef void (*PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
     typedef void (*PFNGLDELETEPROGRAMSARBPROC) (GLsizei n,
	const GLuint * programs);
     typedef void (*PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint * programs);
     typedef void (*PFNGLPROGRAMENVPARAMETER4DARBPROC) (GLenum target,
	GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLPROGRAMENVPARAMETER4DVARBPROC) (GLenum target,
	GLuint index, const GLdouble * params);
     typedef void (*PFNGLPROGRAMENVPARAMETER4FARBPROC) (GLenum target,
	GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target,
	GLuint index, const GLfloat * params);
     typedef void (*PFNGLPROGRAMLOCALPARAMETER4DARBPROC) (GLenum target,
	GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLPROGRAMLOCALPARAMETER4DVARBPROC) (GLenum target,
	GLuint index, const GLdouble * params);
     typedef void (*PFNGLPROGRAMLOCALPARAMETER4FARBPROC) (GLenum target,
	GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target,
	GLuint index, const GLfloat * params);
     typedef void (*PFNGLGETPROGRAMENVPARAMETERDVARBPROC) (GLenum target,
	GLuint index, GLdouble * params);
     typedef void (*PFNGLGETPROGRAMENVPARAMETERFVARBPROC) (GLenum target,
	GLuint index, GLfloat * params);
     typedef void (*PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) (GLenum target,
	GLuint index, GLdouble * params);
     typedef void (*PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC) (GLenum target,
	GLuint index, GLfloat * params);
     typedef void (*PFNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETPROGRAMSTRINGARBPROC) (GLenum target,
	GLenum pname, GLvoid * string);
     typedef void (*PFNGLGETVERTEXATTRIBDVARBPROC) (GLuint index,
	GLenum pname, GLdouble * params);
     typedef void (*PFNGLGETVERTEXATTRIBFVARBPROC) (GLuint index,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETVERTEXATTRIBIVARBPROC) (GLuint index,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETVERTEXATTRIBPOINTERVARBPROC) (GLuint index,
	GLenum pname, GLvoid * *pointer);
     typedef GLboolean(*PFNGLISPROGRAMARBPROC) (GLuint program);
     typedef void (*PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
     typedef void (*PFNGLDELETEBUFFERSARBPROC) (GLsizei n,
	const GLuint * buffers);
     typedef void (*PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint * buffers);
     typedef GLboolean(*PFNGLISBUFFERARBPROC) (GLuint buffer);
     typedef void (*PFNGLBUFFERDATAARBPROC) (GLenum target,
	GLsizeiptrARB size, const GLvoid * data, GLenum usage);
     typedef void (*PFNGLBUFFERSUBDATAARBPROC) (GLenum target,
	GLintptrARB offset, GLsizeiptrARB size, const GLvoid * data);
     typedef void (*PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target,
	GLintptrARB offset, GLsizeiptrARB size, GLvoid * data);
     typedef GLvoid *(*PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
     typedef GLboolean(*PFNGLUNMAPBUFFERARBPROC) (GLenum target);
     typedef void (*PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target,
	GLenum pname, GLvoid * *params);
     typedef void (*PFNGLGENQUERIESARBPROC) (GLsizei n, GLuint * ids);
     typedef void (*PFNGLDELETEQUERIESARBPROC) (GLsizei n,
	const GLuint * ids);
     typedef GLboolean(*PFNGLISQUERYARBPROC) (GLuint id);
     typedef void (*PFNGLBEGINQUERYARBPROC) (GLenum target, GLuint id);
     typedef void (*PFNGLENDQUERYARBPROC) (GLenum target);
     typedef void (*PFNGLGETQUERYIVARBPROC) (GLenum target, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETQUERYOBJECTIVARBPROC) (GLuint id, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETQUERYOBJECTUIVARBPROC) (GLuint id, GLenum pname,
	GLuint * params);
     typedef void (*PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
     typedef GLhandleARB(*PFNGLGETHANDLEARBPROC) (GLenum pname);
     typedef void (*PFNGLDETACHOBJECTARBPROC) (GLhandleARB containerObj,
	GLhandleARB attachedObj);
     typedef GLhandleARB(*PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
     typedef void (*PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj,
	GLsizei count, const GLcharARB * *string, const GLint * length);
     typedef void (*PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
     typedef GLhandleARB(*PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
     typedef void (*PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj,
	GLhandleARB obj);
     typedef void (*PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
     typedef void (*PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
     typedef void (*PFNGLVALIDATEPROGRAMARBPROC) (GLhandleARB programObj);
     typedef void (*PFNGLUNIFORM1FARBPROC) (GLint location, GLfloat v0);
     typedef void (*PFNGLUNIFORM2FARBPROC) (GLint location, GLfloat v0,
	GLfloat v1);
     typedef void (*PFNGLUNIFORM3FARBPROC) (GLint location, GLfloat v0,
	GLfloat v1, GLfloat v2);
     typedef void (*PFNGLUNIFORM4FARBPROC) (GLint location, GLfloat v0,
	GLfloat v1, GLfloat v2, GLfloat v3);
     typedef void (*PFNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
     typedef void (*PFNGLUNIFORM2IARBPROC) (GLint location, GLint v0,
	GLint v1);
     typedef void (*PFNGLUNIFORM3IARBPROC) (GLint location, GLint v0,
	GLint v1, GLint v2);
     typedef void (*PFNGLUNIFORM4IARBPROC) (GLint location, GLint v0,
	GLint v1, GLint v2, GLint v3);
     typedef void (*PFNGLUNIFORM1FVARBPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM2FVARBPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM3FVARBPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM4FVARBPROC) (GLint location, GLsizei count,
	const GLfloat * value);
     typedef void (*PFNGLUNIFORM1IVARBPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORM2IVARBPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORM3IVARBPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORM4IVARBPROC) (GLint location, GLsizei count,
	const GLint * value);
     typedef void (*PFNGLUNIFORMMATRIX2FVARBPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX3FVARBPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLUNIFORMMATRIX4FVARBPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLfloat * value);
     typedef void (*PFNGLGETOBJECTPARAMETERFVARBPROC) (GLhandleARB obj,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETINFOLOGARBPROC) (GLhandleARB obj,
	GLsizei maxLength, GLsizei * length, GLcharARB * infoLog);
     typedef void (*PFNGLGETATTACHEDOBJECTSARBPROC) (GLhandleARB containerObj,
	GLsizei maxCount, GLsizei * count, GLhandleARB * obj);
     typedef GLint(*PFNGLGETUNIFORMLOCATIONARBPROC) (GLhandleARB programObj,
	const GLcharARB * name);
     typedef void (*PFNGLGETACTIVEUNIFORMARBPROC) (GLhandleARB programObj,
	GLuint index, GLsizei maxLength, GLsizei * length, GLint * size,
	GLenum * type, GLcharARB * name);
     typedef void (*PFNGLGETUNIFORMFVARBPROC) (GLhandleARB programObj,
	GLint location, GLfloat * params);
     typedef void (*PFNGLGETUNIFORMIVARBPROC) (GLhandleARB programObj,
	GLint location, GLint * params);
     typedef void (*PFNGLGETSHADERSOURCEARBPROC) (GLhandleARB obj,
	GLsizei maxLength, GLsizei * length, GLcharARB * source);
     typedef void (*PFNGLBINDATTRIBLOCATIONARBPROC) (GLhandleARB programObj,
	GLuint index, const GLcharARB * name);
     typedef void (*PFNGLGETACTIVEATTRIBARBPROC) (GLhandleARB programObj,
	GLuint index, GLsizei maxLength, GLsizei * length, GLint * size,
	GLenum * type, GLcharARB * name);
     typedef GLint(*PFNGLGETATTRIBLOCATIONARBPROC) (GLhandleARB programObj,
	const GLcharARB * name);
     typedef void (*PFNGLDRAWBUFFERSARBPROC) (GLsizei n, const GLenum * bufs);
     typedef void (*PFNGLCLAMPCOLORARBPROC) (GLenum target, GLenum clamp);
     typedef void (*PFNGLDRAWARRAYSINSTANCEDARBPROC) (GLenum mode,
	GLint first, GLsizei count, GLsizei primcount);
     typedef void (*PFNGLDRAWELEMENTSINSTANCEDARBPROC) (GLenum mode,
	GLsizei count, GLenum type, const GLvoid * indices,
	GLsizei primcount);
     typedef GLboolean(*PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
     typedef void (*PFNGLBINDRENDERBUFFERPROC) (GLenum target,
	GLuint renderbuffer);
     typedef void (*PFNGLDELETERENDERBUFFERSPROC) (GLsizei n,
	const GLuint * renderbuffers);
     typedef void (*PFNGLGENRENDERBUFFERSPROC) (GLsizei n,
	GLuint * renderbuffers);
     typedef void (*PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height);
     typedef void (*PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef GLboolean(*PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
     typedef void (*PFNGLBINDFRAMEBUFFERPROC) (GLenum target,
	GLuint framebuffer);
     typedef void (*PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n,
	const GLuint * framebuffers);
     typedef void (*PFNGLGENFRAMEBUFFERSPROC) (GLsizei n,
	GLuint * framebuffers);
     typedef GLenum(*PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
     typedef void (*PFNGLFRAMEBUFFERTEXTURE1DPROC) (GLenum target,
	GLenum attachment, GLenum textarget, GLuint texture, GLint level);
     typedef void (*PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target,
	GLenum attachment, GLenum textarget, GLuint texture, GLint level);
     typedef void (*PFNGLFRAMEBUFFERTEXTURE3DPROC) (GLenum target,
	GLenum attachment, GLenum textarget, GLuint texture, GLint level,
	GLint zoffset);
     typedef void (*PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target,
	GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
     typedef void (*PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum
	target, GLenum attachment, GLenum pname, GLint * params);
     typedef void (*PFNGLGENERATEMIPMAPPROC) (GLenum target);
     typedef void (*PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0,
	GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
	GLint dstY1, GLbitfield mask, GLenum filter);
     typedef void (*PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target,
	GLsizei samples, GLenum internalformat, GLsizei width,
	GLsizei height);
     typedef void (*PFNGLFRAMEBUFFERTEXTURELAYERPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level, GLint layer);
     typedef void (*PFNGLPROGRAMPARAMETERIARBPROC) (GLuint program,
	GLenum pname, GLint value);
     typedef void (*PFNGLFRAMEBUFFERTEXTUREARBPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level);
     typedef void (*PFNGLFRAMEBUFFERTEXTURELAYERARBPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level, GLint layer);
     typedef void (*PFNGLFRAMEBUFFERTEXTUREFACEARBPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level, GLenum face);
     typedef void (*PFNGLVERTEXATTRIBDIVISORARBPROC) (GLuint index,
	GLuint divisor);
     typedef GLvoid *(*PFNGLMAPBUFFERRANGEPROC) (GLenum target,
	GLintptr offset, GLsizeiptr length, GLbitfield access);
     typedef void (*PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target,
	GLintptr offset, GLsizeiptr length);







     typedef void (*PFNGLTEXBUFFERARBPROC) (GLenum target,
	GLenum internalformat, GLuint buffer);
     typedef void (*PFNGLBINDVERTEXARRAYPROC) (GLuint array);
     typedef void (*PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n,
	const GLuint * arrays);
     typedef void (*PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint * arrays);
     typedef GLboolean(*PFNGLISVERTEXARRAYPROC) (GLuint array);
     typedef void (*PFNGLGETUNIFORMINDICESPROC) (GLuint program,
	GLsizei uniformCount, const GLchar * *uniformNames,
	GLuint * uniformIndices);
     typedef void (*PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program,
	GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETACTIVEUNIFORMNAMEPROC) (GLuint program,
	GLuint uniformIndex, GLsizei bufSize, GLsizei * length,
	GLchar * uniformName);
     typedef GLuint(*PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program,
	const GLchar * uniformBlockName);
     typedef void (*PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program,
	GLuint uniformBlockIndex, GLenum pname, GLint * params);
     typedef void (*PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) (GLuint program,
	GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length,
	GLchar * uniformBlockName);
     typedef void (*PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program,
	GLuint uniformBlockIndex, GLuint uniformBlockBinding);
     typedef void (*PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readTarget,
	GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset,
	GLsizeiptr size);
     typedef void (*PFNGLDRAWELEMENTSBASEVERTEXPROC) (GLenum mode,
	GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex);
     typedef void (*PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) (GLenum mode,
	GLuint start, GLuint end, GLsizei count, GLenum type,
	const GLvoid * indices, GLint basevertex);
     typedef void (*PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) (GLenum mode,
	GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount,
	GLint basevertex);
     typedef void (*PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC) (GLenum mode,
	const GLsizei * count, GLenum type, const GLvoid * *indices,
	GLsizei primcount, const GLint * basevertex);
     typedef void (*PFNGLPROVOKINGVERTEXPROC) (GLenum mode);
     typedef GLsync(*PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
     typedef GLboolean(*PFNGLISSYNCPROC) (GLsync sync);
     typedef void (*PFNGLDELETESYNCPROC) (GLsync sync);
     typedef GLenum(*PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags,
	GLuint64 timeout);
     typedef void (*PFNGLWAITSYNCPROC) (GLsync sync, GLbitfield flags,
	GLuint64 timeout);
     typedef void (*PFNGLGETINTEGER64VPROC) (GLenum pname, GLint64 * params);
     typedef void (*PFNGLGETSYNCIVPROC) (GLsync sync, GLenum pname,
	GLsizei bufSize, GLsizei * length, GLint * values);
     typedef void (*PFNGLTEXIMAGE2DMULTISAMPLEPROC) (GLenum target,
	GLsizei samples, GLint internalformat, GLsizei width, GLsizei height,
	GLboolean fixedsamplelocations);
     typedef void (*PFNGLTEXIMAGE3DMULTISAMPLEPROC) (GLenum target,
	GLsizei samples, GLint internalformat, GLsizei width, GLsizei height,
	GLsizei depth, GLboolean fixedsamplelocations);
     typedef void (*PFNGLGETMULTISAMPLEFVPROC) (GLenum pname, GLuint index,
	GLfloat * val);
     typedef void (*PFNGLSAMPLEMASKIPROC) (GLuint index, GLbitfield mask);
     typedef void (*PFNGLBLENDEQUATIONIARBPROC) (GLuint buf, GLenum mode);
     typedef void (*PFNGLBLENDEQUATIONSEPARATEIARBPROC) (GLuint buf,
	GLenum modeRGB, GLenum modeAlpha);
     typedef void (*PFNGLBLENDFUNCIARBPROC) (GLuint buf, GLenum src,
	GLenum dst);
     typedef void (*PFNGLBLENDFUNCSEPARATEIARBPROC) (GLuint buf,
	GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);







     typedef void (*PFNGLMINSAMPLESHADINGARBPROC) (GLclampf value);
     typedef void (*PFNGLNAMEDSTRINGARBPROC) (GLenum type, GLint namelen,
	const GLchar * name, GLint stringlen, const GLchar * string);
     typedef void (*PFNGLDELETENAMEDSTRINGARBPROC) (GLint namelen,
	const GLchar * name);
     typedef void (*PFNGLCOMPILESHADERINCLUDEARBPROC) (GLuint shader,
	GLsizei count, const GLchar * *path, const GLint * length);
     typedef GLboolean(*PFNGLISNAMEDSTRINGARBPROC) (GLint namelen,
	const GLchar * name);
     typedef void (*PFNGLGETNAMEDSTRINGARBPROC) (GLint namelen,
	const GLchar * name, GLsizei bufSize, GLint * stringlen,
	GLchar * string);
     typedef void (*PFNGLGETNAMEDSTRINGIVARBPROC) (GLint namelen,
	const GLchar * name, GLenum pname, GLint * params);
     typedef void (*PFNGLBINDFRAGDATALOCATIONINDEXEDPROC) (GLuint program,
	GLuint colorNumber, GLuint index, const GLchar * name);
     typedef GLint(*PFNGLGETFRAGDATAINDEXPROC) (GLuint program,
	const GLchar * name);
     typedef void (*PFNGLGENSAMPLERSPROC) (GLsizei count, GLuint * samplers);
     typedef void (*PFNGLDELETESAMPLERSPROC) (GLsizei count,
	const GLuint * samplers);
     typedef GLboolean(*PFNGLISSAMPLERPROC) (GLuint sampler);
     typedef void (*PFNGLBINDSAMPLERPROC) (GLuint unit, GLuint sampler);
     typedef void (*PFNGLSAMPLERPARAMETERIPROC) (GLuint sampler, GLenum pname,
	GLint param);
     typedef void (*PFNGLSAMPLERPARAMETERIVPROC) (GLuint sampler,
	GLenum pname, const GLint * param);
     typedef void (*PFNGLSAMPLERPARAMETERFPROC) (GLuint sampler, GLenum pname,
	GLfloat param);
     typedef void (*PFNGLSAMPLERPARAMETERFVPROC) (GLuint sampler,
	GLenum pname, const GLfloat * param);
     typedef void (*PFNGLSAMPLERPARAMETERIIVPROC) (GLuint sampler,
	GLenum pname, const GLint * param);
     typedef void (*PFNGLSAMPLERPARAMETERIUIVPROC) (GLuint sampler,
	GLenum pname, const GLuint * param);
     typedef void (*PFNGLGETSAMPLERPARAMETERIVPROC) (GLuint sampler,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETSAMPLERPARAMETERIIVPROC) (GLuint sampler,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETSAMPLERPARAMETERFVPROC) (GLuint sampler,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETSAMPLERPARAMETERIUIVPROC) (GLuint sampler,
	GLenum pname, GLuint * params);
     typedef void (*PFNGLQUERYCOUNTERPROC) (GLuint id, GLenum target);
     typedef void (*PFNGLGETQUERYOBJECTI64VPROC) (GLuint id, GLenum pname,
	GLint64 * params);
     typedef void (*PFNGLGETQUERYOBJECTUI64VPROC) (GLuint id, GLenum pname,
	GLuint64 * params);
     typedef void (*PFNGLVERTEXP2UIPROC) (GLenum type, GLuint value);
     typedef void (*PFNGLVERTEXP2UIVPROC) (GLenum type, const GLuint * value);
     typedef void (*PFNGLVERTEXP3UIPROC) (GLenum type, GLuint value);
     typedef void (*PFNGLVERTEXP3UIVPROC) (GLenum type, const GLuint * value);
     typedef void (*PFNGLVERTEXP4UIPROC) (GLenum type, GLuint value);
     typedef void (*PFNGLVERTEXP4UIVPROC) (GLenum type, const GLuint * value);
     typedef void (*PFNGLTEXCOORDP1UIPROC) (GLenum type, GLuint coords);
     typedef void (*PFNGLTEXCOORDP1UIVPROC) (GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLTEXCOORDP2UIPROC) (GLenum type, GLuint coords);
     typedef void (*PFNGLTEXCOORDP2UIVPROC) (GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLTEXCOORDP3UIPROC) (GLenum type, GLuint coords);
     typedef void (*PFNGLTEXCOORDP3UIVPROC) (GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLTEXCOORDP4UIPROC) (GLenum type, GLuint coords);
     typedef void (*PFNGLTEXCOORDP4UIVPROC) (GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLMULTITEXCOORDP1UIPROC) (GLenum texture, GLenum type,
	GLuint coords);
     typedef void (*PFNGLMULTITEXCOORDP1UIVPROC) (GLenum texture, GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLMULTITEXCOORDP2UIPROC) (GLenum texture, GLenum type,
	GLuint coords);
     typedef void (*PFNGLMULTITEXCOORDP2UIVPROC) (GLenum texture, GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLMULTITEXCOORDP3UIPROC) (GLenum texture, GLenum type,
	GLuint coords);
     typedef void (*PFNGLMULTITEXCOORDP3UIVPROC) (GLenum texture, GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLMULTITEXCOORDP4UIPROC) (GLenum texture, GLenum type,
	GLuint coords);
     typedef void (*PFNGLMULTITEXCOORDP4UIVPROC) (GLenum texture, GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLNORMALP3UIPROC) (GLenum type, GLuint coords);
     typedef void (*PFNGLNORMALP3UIVPROC) (GLenum type,
	const GLuint * coords);
     typedef void (*PFNGLCOLORP3UIPROC) (GLenum type, GLuint color);
     typedef void (*PFNGLCOLORP3UIVPROC) (GLenum type, const GLuint * color);
     typedef void (*PFNGLCOLORP4UIPROC) (GLenum type, GLuint color);
     typedef void (*PFNGLCOLORP4UIVPROC) (GLenum type, const GLuint * color);
     typedef void (*PFNGLSECONDARYCOLORP3UIPROC) (GLenum type, GLuint color);
     typedef void (*PFNGLSECONDARYCOLORP3UIVPROC) (GLenum type,
	const GLuint * color);
     typedef void (*PFNGLVERTEXATTRIBP1UIPROC) (GLuint index, GLenum type,
	GLboolean normalized, GLuint value);
     typedef void (*PFNGLVERTEXATTRIBP1UIVPROC) (GLuint index, GLenum type,
	GLboolean normalized, const GLuint * value);
     typedef void (*PFNGLVERTEXATTRIBP2UIPROC) (GLuint index, GLenum type,
	GLboolean normalized, GLuint value);
     typedef void (*PFNGLVERTEXATTRIBP2UIVPROC) (GLuint index, GLenum type,
	GLboolean normalized, const GLuint * value);
     typedef void (*PFNGLVERTEXATTRIBP3UIPROC) (GLuint index, GLenum type,
	GLboolean normalized, GLuint value);
     typedef void (*PFNGLVERTEXATTRIBP3UIVPROC) (GLuint index, GLenum type,
	GLboolean normalized, const GLuint * value);
     typedef void (*PFNGLVERTEXATTRIBP4UIPROC) (GLuint index, GLenum type,
	GLboolean normalized, GLuint value);
     typedef void (*PFNGLVERTEXATTRIBP4UIVPROC) (GLuint index, GLenum type,
	GLboolean normalized, const GLuint * value);
     typedef void (*PFNGLDRAWARRAYSINDIRECTPROC) (GLenum mode,
	const GLvoid * indirect);
     typedef void (*PFNGLDRAWELEMENTSINDIRECTPROC) (GLenum mode, GLenum type,
	const GLvoid * indirect);
     typedef void (*PFNGLUNIFORM1DPROC) (GLint location, GLdouble x);
     typedef void (*PFNGLUNIFORM2DPROC) (GLint location, GLdouble x,
	GLdouble y);
     typedef void (*PFNGLUNIFORM3DPROC) (GLint location, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLUNIFORM4DPROC) (GLint location, GLdouble x,
	GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLUNIFORM1DVPROC) (GLint location, GLsizei count,
	const GLdouble * value);
     typedef void (*PFNGLUNIFORM2DVPROC) (GLint location, GLsizei count,
	const GLdouble * value);
     typedef void (*PFNGLUNIFORM3DVPROC) (GLint location, GLsizei count,
	const GLdouble * value);
     typedef void (*PFNGLUNIFORM4DVPROC) (GLint location, GLsizei count,
	const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX2DVPROC) (GLint location, GLsizei count,
	GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX3DVPROC) (GLint location, GLsizei count,
	GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX4DVPROC) (GLint location, GLsizei count,
	GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX2X3DVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX2X4DVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX3X2DVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX3X4DVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX4X2DVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLUNIFORMMATRIX4X3DVPROC) (GLint location,
	GLsizei count, GLboolean transpose, const GLdouble * value);
     typedef void (*PFNGLGETUNIFORMDVPROC) (GLuint program, GLint location,
	GLdouble * params);
     typedef GLint(*PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC) (GLuint program,
	GLenum shadertype, const GLchar * name);
     typedef GLuint(*PFNGLGETSUBROUTINEINDEXPROC) (GLuint program,
	GLenum shadertype, const GLchar * name);
     typedef void (*PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC) (GLuint program,
	GLenum shadertype, GLuint index, GLenum pname, GLint * values);
     typedef void (*PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC) (GLuint program,
	GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length,
	GLchar * name);
     typedef void (*PFNGLGETACTIVESUBROUTINENAMEPROC) (GLuint program,
	GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length,
	GLchar * name);
     typedef void (*PFNGLUNIFORMSUBROUTINESUIVPROC) (GLenum shadertype,
	GLsizei count, const GLuint * indices);
     typedef void (*PFNGLGETUNIFORMSUBROUTINEUIVPROC) (GLenum shadertype,
	GLint location, GLuint * params);
     typedef void (*PFNGLGETPROGRAMSTAGEIVPROC) (GLuint program,
	GLenum shadertype, GLenum pname, GLint * values);
     typedef void (*PFNGLPATCHPARAMETERIPROC) (GLenum pname, GLint value);
     typedef void (*PFNGLPATCHPARAMETERFVPROC) (GLenum pname,
	const GLfloat * values);
     typedef void (*PFNGLBINDTRANSFORMFEEDBACKPROC) (GLenum target,
	GLuint id);
     typedef void (*PFNGLDELETETRANSFORMFEEDBACKSPROC) (GLsizei n,
	const GLuint * ids);
     typedef void (*PFNGLGENTRANSFORMFEEDBACKSPROC) (GLsizei n, GLuint * ids);
     typedef GLboolean(*PFNGLISTRANSFORMFEEDBACKPROC) (GLuint id);
     typedef void (*PFNGLPAUSETRANSFORMFEEDBACKPROC) (void);
     typedef void (*PFNGLRESUMETRANSFORMFEEDBACKPROC) (void);
     typedef void (*PFNGLDRAWTRANSFORMFEEDBACKPROC) (GLenum mode, GLuint id);
     typedef void (*PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC) (GLenum mode,
	GLuint id, GLuint stream);
     typedef void (*PFNGLBEGINQUERYINDEXEDPROC) (GLenum target, GLuint index,
	GLuint id);
     typedef void (*PFNGLENDQUERYINDEXEDPROC) (GLenum target, GLuint index);
     typedef void (*PFNGLGETQUERYINDEXEDIVPROC) (GLenum target, GLuint index,
	GLenum pname, GLint * params);
     typedef void (*PFNGLRELEASESHADERCOMPILERPROC) (void);
     typedef void (*PFNGLSHADERBINARYPROC) (GLsizei count,
	const GLuint * shaders, GLenum binaryformat, const GLvoid * binary,
	GLsizei length);
     typedef void (*PFNGLGETSHADERPRECISIONFORMATPROC) (GLenum shadertype,
	GLenum precisiontype, GLint * range, GLint * precision);
     typedef void (*PFNGLDEPTHRANGEFPROC) (GLclampf n, GLclampf f);
     typedef void (*PFNGLCLEARDEPTHFPROC) (GLclampf d);
     typedef void (*PFNGLGETPROGRAMBINARYPROC) (GLuint program,
	GLsizei bufSize, GLsizei * length, GLenum * binaryFormat,
	GLvoid * binary);
     typedef void (*PFNGLPROGRAMBINARYPROC) (GLuint program,
	GLenum binaryFormat, const GLvoid * binary, GLsizei length);
     typedef void (*PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname,
	GLint value);
     typedef void (*PFNGLUSEPROGRAMSTAGESPROC) (GLuint pipeline,
	GLbitfield stages, GLuint program);
     typedef void (*PFNGLACTIVESHADERPROGRAMPROC) (GLuint pipeline,
	GLuint program);
     typedef GLuint(*PFNGLCREATESHADERPROGRAMVPROC) (GLenum type,
	GLsizei count, const GLchar * *strings);
     typedef void (*PFNGLBINDPROGRAMPIPELINEPROC) (GLuint pipeline);
     typedef void (*PFNGLDELETEPROGRAMPIPELINESPROC) (GLsizei n,
	const GLuint * pipelines);
     typedef void (*PFNGLGENPROGRAMPIPELINESPROC) (GLsizei n,
	GLuint * pipelines);
     typedef GLboolean(*PFNGLISPROGRAMPIPELINEPROC) (GLuint pipeline);
     typedef void (*PFNGLGETPROGRAMPIPELINEIVPROC) (GLuint pipeline,
	GLenum pname, GLint * params);
     typedef void (*PFNGLPROGRAMUNIFORM1IPROC) (GLuint program,
	GLint location, GLint v0);
     typedef void (*PFNGLPROGRAMUNIFORM1IVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORM1FPROC) (GLuint program,
	GLint location, GLfloat v0);
     typedef void (*PFNGLPROGRAMUNIFORM1FVPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM1DPROC) (GLuint program,
	GLint location, GLdouble v0);
     typedef void (*PFNGLPROGRAMUNIFORM1DVPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORM1UIPROC) (GLuint program,
	GLint location, GLuint v0);
     typedef void (*PFNGLPROGRAMUNIFORM1UIVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLPROGRAMUNIFORM2IPROC) (GLuint program,
	GLint location, GLint v0, GLint v1);
     typedef void (*PFNGLPROGRAMUNIFORM2IVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORM2FPROC) (GLuint program,
	GLint location, GLfloat v0, GLfloat v1);
     typedef void (*PFNGLPROGRAMUNIFORM2FVPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM2DPROC) (GLuint program,
	GLint location, GLdouble v0, GLdouble v1);
     typedef void (*PFNGLPROGRAMUNIFORM2DVPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORM2UIPROC) (GLuint program,
	GLint location, GLuint v0, GLuint v1);
     typedef void (*PFNGLPROGRAMUNIFORM2UIVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLPROGRAMUNIFORM3IPROC) (GLuint program,
	GLint location, GLint v0, GLint v1, GLint v2);
     typedef void (*PFNGLPROGRAMUNIFORM3IVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORM3FPROC) (GLuint program,
	GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
     typedef void (*PFNGLPROGRAMUNIFORM3FVPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM3DPROC) (GLuint program,
	GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
     typedef void (*PFNGLPROGRAMUNIFORM3DVPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORM3UIPROC) (GLuint program,
	GLint location, GLuint v0, GLuint v1, GLuint v2);
     typedef void (*PFNGLPROGRAMUNIFORM3UIVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLPROGRAMUNIFORM4IPROC) (GLuint program,
	GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
     typedef void (*PFNGLPROGRAMUNIFORM4IVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORM4FPROC) (GLuint program,
	GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
     typedef void (*PFNGLPROGRAMUNIFORM4FVPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM4DPROC) (GLuint program,
	GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
     typedef void (*PFNGLPROGRAMUNIFORM4DVPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORM4UIPROC) (GLuint program,
	GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
     typedef void (*PFNGLPROGRAMUNIFORM4UIVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLVALIDATEPROGRAMPIPELINEPROC) (GLuint pipeline);
     typedef void (*PFNGLGETPROGRAMPIPELINEINFOLOGPROC) (GLuint pipeline,
	GLsizei bufSize, GLsizei * length, GLchar * infoLog);
     typedef void (*PFNGLVERTEXATTRIBL1DPROC) (GLuint index, GLdouble x);
     typedef void (*PFNGLVERTEXATTRIBL2DPROC) (GLuint index, GLdouble x,
	GLdouble y);
     typedef void (*PFNGLVERTEXATTRIBL3DPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLVERTEXATTRIBL4DPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLVERTEXATTRIBL1DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBL2DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBL3DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBL4DVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBLPOINTERPROC) (GLuint index, GLint size,
	GLenum type, GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLGETVERTEXATTRIBLDVPROC) (GLuint index, GLenum pname,
	GLdouble * params);
     typedef void (*PFNGLVIEWPORTARRAYVPROC) (GLuint first, GLsizei count,
	const GLfloat * v);
     typedef void (*PFNGLVIEWPORTINDEXEDFPROC) (GLuint index, GLfloat x,
	GLfloat y, GLfloat w, GLfloat h);
     typedef void (*PFNGLVIEWPORTINDEXEDFVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLSCISSORARRAYVPROC) (GLuint first, GLsizei count,
	const GLint * v);
     typedef void (*PFNGLSCISSORINDEXEDPROC) (GLuint index, GLint left,
	GLint bottom, GLsizei width, GLsizei height);
     typedef void (*PFNGLSCISSORINDEXEDVPROC) (GLuint index, const GLint * v);
     typedef void (*PFNGLDEPTHRANGEARRAYVPROC) (GLuint first, GLsizei count,
	const GLclampd * v);
     typedef void (*PFNGLDEPTHRANGEINDEXEDPROC) (GLuint index, GLclampd n,
	GLclampd f);
     typedef void (*PFNGLGETFLOATI_VPROC) (GLenum target, GLuint index,
	GLfloat * data);
     typedef void (*PFNGLGETDOUBLEI_VPROC) (GLenum target, GLuint index,
	GLdouble * data);







     typedef GLsync(*PFNGLCREATESYNCFROMCLEVENTARBPROC) (struct _cl_context *
	context, struct _cl_event * event, GLbitfield flags);
     typedef void (*PFNGLDEBUGMESSAGECONTROLARBPROC) (GLenum source,
	GLenum type, GLenum severity, GLsizei count, const GLuint * ids,
	GLboolean enabled);
     typedef void (*PFNGLDEBUGMESSAGEINSERTARBPROC) (GLenum source,
	GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar * buf);
     typedef void (*PFNGLDEBUGMESSAGECALLBACKARBPROC) (GLDEBUGPROCARB
	callback, const GLvoid * userParam);
     typedef GLuint(*PFNGLGETDEBUGMESSAGELOGARBPROC) (GLuint count,
	GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids,
	GLenum * severities, GLsizei * lengths, GLchar * messageLog);
     typedef GLenum(*PFNGLGETGRAPHICSRESETSTATUSARBPROC) (void);
     typedef void (*PFNGLGETNMAPDVARBPROC) (GLenum target, GLenum query,
	GLsizei bufSize, GLdouble * v);
     typedef void (*PFNGLGETNMAPFVARBPROC) (GLenum target, GLenum query,
	GLsizei bufSize, GLfloat * v);
     typedef void (*PFNGLGETNMAPIVARBPROC) (GLenum target, GLenum query,
	GLsizei bufSize, GLint * v);
     typedef void (*PFNGLGETNPIXELMAPFVARBPROC) (GLenum map, GLsizei bufSize,
	GLfloat * values);
     typedef void (*PFNGLGETNPIXELMAPUIVARBPROC) (GLenum map, GLsizei bufSize,
	GLuint * values);
     typedef void (*PFNGLGETNPIXELMAPUSVARBPROC) (GLenum map, GLsizei bufSize,
	GLushort * values);
     typedef void (*PFNGLGETNPOLYGONSTIPPLEARBPROC) (GLsizei bufSize,
	GLubyte * pattern);
     typedef void (*PFNGLGETNCOLORTABLEARBPROC) (GLenum target, GLenum format,
	GLenum type, GLsizei bufSize, GLvoid * table);
     typedef void (*PFNGLGETNCONVOLUTIONFILTERARBPROC) (GLenum target,
	GLenum format, GLenum type, GLsizei bufSize, GLvoid * image);
     typedef void (*PFNGLGETNSEPARABLEFILTERARBPROC) (GLenum target,
	GLenum format, GLenum type, GLsizei rowBufSize, GLvoid * row,
	GLsizei columnBufSize, GLvoid * column, GLvoid * span);
     typedef void (*PFNGLGETNHISTOGRAMARBPROC) (GLenum target,
	GLboolean reset, GLenum format, GLenum type, GLsizei bufSize,
	GLvoid * values);
     typedef void (*PFNGLGETNMINMAXARBPROC) (GLenum target, GLboolean reset,
	GLenum format, GLenum type, GLsizei bufSize, GLvoid * values);
     typedef void (*PFNGLGETNTEXIMAGEARBPROC) (GLenum target, GLint level,
	GLenum format, GLenum type, GLsizei bufSize, GLvoid * img);
     typedef void (*PFNGLREADNPIXELSARBPROC) (GLint x, GLint y, GLsizei width,
	GLsizei height, GLenum format, GLenum type, GLsizei bufSize,
	GLvoid * data);
     typedef void (*PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC) (GLenum target,
	GLint lod, GLsizei bufSize, GLvoid * img);
     typedef void (*PFNGLGETNUNIFORMFVARBPROC) (GLuint program,
	GLint location, GLsizei bufSize, GLfloat * params);
     typedef void (*PFNGLGETNUNIFORMIVARBPROC) (GLuint program,
	GLint location, GLsizei bufSize, GLint * params);
     typedef void (*PFNGLGETNUNIFORMUIVARBPROC) (GLuint program,
	GLint location, GLsizei bufSize, GLuint * params);
     typedef void (*PFNGLGETNUNIFORMDVARBPROC) (GLuint program,
	GLint location, GLsizei bufSize, GLdouble * params);
     typedef void (*PFNGLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green,
	GLclampf blue, GLclampf alpha);







     typedef void (*PFNGLPOLYGONOFFSETEXTPROC) (GLfloat factor, GLfloat bias);
     typedef void (*PFNGLTEXIMAGE3DEXTPROC) (GLenum target, GLint level,
	GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
	GLint border, GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level,
	GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width,
	GLsizei height, GLsizei depth, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLGETTEXFILTERFUNCSGISPROC) (GLenum target,
	GLenum filter, GLfloat * weights);
     typedef void (*PFNGLTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter,
	GLsizei n, const GLfloat * weights);
     typedef void (*PFNGLTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level,
	GLint xoffset, GLsizei width, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level,
	GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
	GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLCOPYTEXIMAGE1DEXTPROC) (GLenum target, GLint level,
	GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
     typedef void (*PFNGLCOPYTEXIMAGE2DEXTPROC) (GLenum target, GLint level,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height, GLint border);
     typedef void (*PFNGLCOPYTEXSUBIMAGE1DEXTPROC) (GLenum target,
	GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
     typedef void (*PFNGLCOPYTEXSUBIMAGE2DEXTPROC) (GLenum target,
	GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
	GLsizei width, GLsizei height);
     typedef void (*PFNGLCOPYTEXSUBIMAGE3DEXTPROC) (GLenum target,
	GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x,
	GLint y, GLsizei width, GLsizei height);
     typedef void (*PFNGLGETHISTOGRAMEXTPROC) (GLenum target, GLboolean reset,
	GLenum format, GLenum type, GLvoid * values);
     typedef void (*PFNGLGETHISTOGRAMPARAMETERFVEXTPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETHISTOGRAMPARAMETERIVEXTPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETMINMAXEXTPROC) (GLenum target, GLboolean reset,
	GLenum format, GLenum type, GLvoid * values);
     typedef void (*PFNGLGETMINMAXPARAMETERFVEXTPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETMINMAXPARAMETERIVEXTPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLHISTOGRAMEXTPROC) (GLenum target, GLsizei width,
	GLenum internalformat, GLboolean sink);
     typedef void (*PFNGLMINMAXEXTPROC) (GLenum target, GLenum internalformat,
	GLboolean sink);
     typedef void (*PFNGLRESETHISTOGRAMEXTPROC) (GLenum target);
     typedef void (*PFNGLRESETMINMAXEXTPROC) (GLenum target);
     typedef void (*PFNGLCONVOLUTIONFILTER1DEXTPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid * image);
     typedef void (*PFNGLCONVOLUTIONFILTER2DEXTPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid * image);
     typedef void (*PFNGLCONVOLUTIONPARAMETERFEXTPROC) (GLenum target,
	GLenum pname, GLfloat params);
     typedef void (*PFNGLCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLCONVOLUTIONPARAMETERIEXTPROC) (GLenum target,
	GLenum pname, GLint params);
     typedef void (*PFNGLCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC) (GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width);
     typedef void (*PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC) (GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height);
     typedef void (*PFNGLGETCONVOLUTIONFILTEREXTPROC) (GLenum target,
	GLenum format, GLenum type, GLvoid * image);
     typedef void (*PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETSEPARABLEFILTEREXTPROC) (GLenum target,
	GLenum format, GLenum type, GLvoid * row, GLvoid * column,
	GLvoid * span);
     typedef void (*PFNGLSEPARABLEFILTER2DEXTPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid * row, const GLvoid * column);
     typedef void (*PFNGLCOLORTABLESGIPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid * table);
     typedef void (*PFNGLCOLORTABLEPARAMETERFVSGIPROC) (GLenum target,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLCOLORTABLEPARAMETERIVSGIPROC) (GLenum target,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLCOPYCOLORTABLESGIPROC) (GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width);
     typedef void (*PFNGLGETCOLORTABLESGIPROC) (GLenum target, GLenum format,
	GLenum type, GLvoid * table);
     typedef void (*PFNGLGETCOLORTABLEPARAMETERFVSGIPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETCOLORTABLEPARAMETERIVSGIPROC) (GLenum target,
	GLenum pname, GLint * params);







     typedef void (*PFNGLPIXELTEXGENSGIXPROC) (GLenum mode);
     typedef void (*PFNGLPIXELTEXGENPARAMETERISGISPROC) (GLenum pname,
	GLint param);
     typedef void (*PFNGLPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname,
	const GLint * params);
     typedef void (*PFNGLPIXELTEXGENPARAMETERFSGISPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLGETPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname,
	GLfloat * params);
     typedef void (*PFNGLTEXIMAGE4DSGISPROC) (GLenum target, GLint level,
	GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
	GLsizei size4d, GLint border, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLTEXSUBIMAGE4DSGISPROC) (GLenum target, GLint level,
	GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset,
	GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d,
	GLenum format, GLenum type, const GLvoid * pixels);
     typedef GLboolean(*PFNGLARETEXTURESRESIDENTEXTPROC) (GLsizei n,
	const GLuint * textures, GLboolean * residences);
     typedef void (*PFNGLBINDTEXTUREEXTPROC) (GLenum target, GLuint texture);
     typedef void (*PFNGLDELETETEXTURESEXTPROC) (GLsizei n,
	const GLuint * textures);
     typedef void (*PFNGLGENTEXTURESEXTPROC) (GLsizei n, GLuint * textures);
     typedef GLboolean(*PFNGLISTEXTUREEXTPROC) (GLuint texture);
     typedef void (*PFNGLPRIORITIZETEXTURESEXTPROC) (GLsizei n,
	const GLuint * textures, const GLclampf * priorities);
     typedef void (*PFNGLDETAILTEXFUNCSGISPROC) (GLenum target, GLsizei n,
	const GLfloat * points);
     typedef void (*PFNGLGETDETAILTEXFUNCSGISPROC) (GLenum target,
	GLfloat * points);
     typedef void (*PFNGLSHARPENTEXFUNCSGISPROC) (GLenum target, GLsizei n,
	const GLfloat * points);
     typedef void (*PFNGLGETSHARPENTEXFUNCSGISPROC) (GLenum target,
	GLfloat * points);
     typedef void (*PFNGLSAMPLEMASKSGISPROC) (GLclampf value,
	GLboolean invert);
     typedef void (*PFNGLSAMPLEPATTERNSGISPROC) (GLenum pattern);
     typedef void (*PFNGLARRAYELEMENTEXTPROC) (GLint i);
     typedef void (*PFNGLCOLORPOINTEREXTPROC) (GLint size, GLenum type,
	GLsizei stride, GLsizei count, const GLvoid * pointer);
     typedef void (*PFNGLDRAWARRAYSEXTPROC) (GLenum mode, GLint first,
	GLsizei count);
     typedef void (*PFNGLEDGEFLAGPOINTEREXTPROC) (GLsizei stride,
	GLsizei count, const GLboolean * pointer);
     typedef void (*PFNGLGETPOINTERVEXTPROC) (GLenum pname, GLvoid * *params);
     typedef void (*PFNGLINDEXPOINTEREXTPROC) (GLenum type, GLsizei stride,
	GLsizei count, const GLvoid * pointer);
     typedef void (*PFNGLNORMALPOINTEREXTPROC) (GLenum type, GLsizei stride,
	GLsizei count, const GLvoid * pointer);
     typedef void (*PFNGLTEXCOORDPOINTEREXTPROC) (GLint size, GLenum type,
	GLsizei stride, GLsizei count, const GLvoid * pointer);
     typedef void (*PFNGLVERTEXPOINTEREXTPROC) (GLint size, GLenum type,
	GLsizei stride, GLsizei count, const GLvoid * pointer);
     typedef void (*PFNGLBLENDEQUATIONEXTPROC) (GLenum mode);
     typedef void (*PFNGLSPRITEPARAMETERFSGIXPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLSPRITEPARAMETERFVSGIXPROC) (GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLSPRITEPARAMETERISGIXPROC) (GLenum pname,
	GLint param);
     typedef void (*PFNGLSPRITEPARAMETERIVSGIXPROC) (GLenum pname,
	const GLint * params);
     typedef void (*PFNGLPOINTPARAMETERFEXTPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLPOINTPARAMETERFVEXTPROC) (GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLPOINTPARAMETERFSGISPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLPOINTPARAMETERFVSGISPROC) (GLenum pname,
	const GLfloat * params);
     typedef GLint(*PFNGLGETINSTRUMENTSSGIXPROC) (void);
     typedef void (*PFNGLINSTRUMENTSBUFFERSGIXPROC) (GLsizei size,
	GLint * buffer);
     typedef GLint(*PFNGLPOLLINSTRUMENTSSGIXPROC) (GLint * marker_p);
     typedef void (*PFNGLREADINSTRUMENTSSGIXPROC) (GLint marker);
     typedef void (*PFNGLSTARTINSTRUMENTSSGIXPROC) (void);
     typedef void (*PFNGLSTOPINSTRUMENTSSGIXPROC) (GLint marker);
     typedef void (*PFNGLFRAMEZOOMSGIXPROC) (GLint factor);







     typedef void (*PFNGLTAGSAMPLEBUFFERSGIXPROC) (void);
     typedef void (*PFNGLDEFORMATIONMAP3DSGIXPROC) (GLenum target,
	GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1,
	GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2,
	GLint wstride, GLint worder, const GLdouble * points);
     typedef void (*PFNGLDEFORMATIONMAP3FSGIXPROC) (GLenum target, GLfloat u1,
	GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2,
	GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride,
	GLint worder, const GLfloat * points);
     typedef void (*PFNGLDEFORMSGIXPROC) (GLbitfield mask);
     typedef void (*PFNGLLOADIDENTITYDEFORMATIONMAPSGIXPROC) (GLbitfield
	mask);







     typedef void (*PFNGLREFERENCEPLANESGIXPROC) (const GLdouble * equation);







     typedef void (*PFNGLFLUSHRASTERSGIXPROC) (void);
     typedef void (*PFNGLFOGFUNCSGISPROC) (GLsizei n, const GLfloat * points);
     typedef void (*PFNGLGETFOGFUNCSGISPROC) (GLfloat * points);
     typedef void (*PFNGLIMAGETRANSFORMPARAMETERIHPPROC) (GLenum target,
	GLenum pname, GLint param);
     typedef void (*PFNGLIMAGETRANSFORMPARAMETERFHPPROC) (GLenum target,
	GLenum pname, GLfloat param);
     typedef void (*PFNGLIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start,
	GLsizei count, GLenum format, GLenum type, const GLvoid * data);
     typedef void (*PFNGLCOPYCOLORSUBTABLEEXTPROC) (GLenum target,
	GLsizei start, GLint x, GLint y, GLsizei width);
     typedef void (*PFNGLHINTPGIPROC) (GLenum target, GLint mode);
     typedef void (*PFNGLCOLORTABLEEXTPROC) (GLenum target,
	GLenum internalFormat, GLsizei width, GLenum format, GLenum type,
	const GLvoid * table);
     typedef void (*PFNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format,
	GLenum type, GLvoid * data);
     typedef void (*PFNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETLISTPARAMETERFVSGIXPROC) (GLuint list,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETLISTPARAMETERIVSGIXPROC) (GLuint list,
	GLenum pname, GLint * params);
     typedef void (*PFNGLLISTPARAMETERFSGIXPROC) (GLuint list, GLenum pname,
	GLfloat param);
     typedef void (*PFNGLLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLLISTPARAMETERISGIXPROC) (GLuint list, GLenum pname,
	GLint param);
     typedef void (*PFNGLLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum pname,
	const GLint * params);
     typedef void (*PFNGLINDEXMATERIALEXTPROC) (GLenum face, GLenum mode);







     typedef void (*PFNGLINDEXFUNCEXTPROC) (GLenum func, GLclampf ref);
     typedef void (*PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
     typedef void (*PFNGLUNLOCKARRAYSEXTPROC) (void);
     typedef void (*PFNGLCULLPARAMETERDVEXTPROC) (GLenum pname,
	GLdouble * params);
     typedef void (*PFNGLCULLPARAMETERFVEXTPROC) (GLenum pname,
	GLfloat * params);
     typedef void (*PFNGLFRAGMENTCOLORMATERIALSGIXPROC) (GLenum face,
	GLenum mode);
     typedef void (*PFNGLFRAGMENTLIGHTFSGIXPROC) (GLenum light, GLenum pname,
	GLfloat param);
     typedef void (*PFNGLFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLFRAGMENTLIGHTISGIXPROC) (GLenum light, GLenum pname,
	GLint param);
     typedef void (*PFNGLFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname,
	const GLint * params);
     typedef void (*PFNGLFRAGMENTLIGHTMODELFSGIXPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLFRAGMENTLIGHTMODELFVSGIXPROC) (GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLFRAGMENTLIGHTMODELISGIXPROC) (GLenum pname,
	GLint param);
     typedef void (*PFNGLFRAGMENTLIGHTMODELIVSGIXPROC) (GLenum pname,
	const GLint * params);
     typedef void (*PFNGLFRAGMENTMATERIALFSGIXPROC) (GLenum face,
	GLenum pname, GLfloat param);
     typedef void (*PFNGLFRAGMENTMATERIALFVSGIXPROC) (GLenum face,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLFRAGMENTMATERIALISGIXPROC) (GLenum face,
	GLenum pname, GLint param);
     typedef void (*PFNGLFRAGMENTMATERIALIVSGIXPROC) (GLenum face,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLGETFRAGMENTLIGHTFVSGIXPROC) (GLenum light,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETFRAGMENTLIGHTIVSGIXPROC) (GLenum light,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETFRAGMENTMATERIALFVSGIXPROC) (GLenum face,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETFRAGMENTMATERIALIVSGIXPROC) (GLenum face,
	GLenum pname, GLint * params);
     typedef void (*PFNGLLIGHTENVISGIXPROC) (GLenum pname, GLint param);
     typedef void (*PFNGLDRAWRANGEELEMENTSEXTPROC) (GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
     typedef void (*PFNGLAPPLYTEXTUREEXTPROC) (GLenum mode);
     typedef void (*PFNGLTEXTURELIGHTEXTPROC) (GLenum pname);
     typedef void (*PFNGLTEXTUREMATERIALEXTPROC) (GLenum face, GLenum mode);
     typedef void (*PFNGLASYNCMARKERSGIXPROC) (GLuint marker);
     typedef GLint(*PFNGLFINISHASYNCSGIXPROC) (GLuint * markerp);
     typedef GLint(*PFNGLPOLLASYNCSGIXPROC) (GLuint * markerp);
     typedef GLuint(*PFNGLGENASYNCMARKERSSGIXPROC) (GLsizei range);
     typedef void (*PFNGLDELETEASYNCMARKERSSGIXPROC) (GLuint marker,
	GLsizei range);
     typedef GLboolean(*PFNGLISASYNCMARKERSGIXPROC) (GLuint marker);
     typedef void (*PFNGLVERTEXPOINTERVINTELPROC) (GLint size, GLenum type,
	const GLvoid * *pointer);
     typedef void (*PFNGLNORMALPOINTERVINTELPROC) (GLenum type,
	const GLvoid * *pointer);
     typedef void (*PFNGLCOLORPOINTERVINTELPROC) (GLint size, GLenum type,
	const GLvoid * *pointer);
     typedef void (*PFNGLTEXCOORDPOINTERVINTELPROC) (GLint size, GLenum type,
	const GLvoid * *pointer);
     typedef void (*PFNGLPIXELTRANSFORMPARAMETERIEXTPROC) (GLenum target,
	GLenum pname, GLint param);
     typedef void (*PFNGLPIXELTRANSFORMPARAMETERFEXTPROC) (GLenum target,
	GLenum pname, GLfloat param);
     typedef void (*PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC) (GLenum target,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC) (GLenum target,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLSECONDARYCOLOR3BEXTPROC) (GLbyte red, GLbyte green,
	GLbyte blue);
     typedef void (*PFNGLSECONDARYCOLOR3BVEXTPROC) (const GLbyte * v);
     typedef void (*PFNGLSECONDARYCOLOR3DEXTPROC) (GLdouble red,
	GLdouble green, GLdouble blue);
     typedef void (*PFNGLSECONDARYCOLOR3DVEXTPROC) (const GLdouble * v);
     typedef void (*PFNGLSECONDARYCOLOR3FEXTPROC) (GLfloat red, GLfloat green,
	GLfloat blue);
     typedef void (*PFNGLSECONDARYCOLOR3FVEXTPROC) (const GLfloat * v);
     typedef void (*PFNGLSECONDARYCOLOR3IEXTPROC) (GLint red, GLint green,
	GLint blue);
     typedef void (*PFNGLSECONDARYCOLOR3IVEXTPROC) (const GLint * v);
     typedef void (*PFNGLSECONDARYCOLOR3SEXTPROC) (GLshort red, GLshort green,
	GLshort blue);
     typedef void (*PFNGLSECONDARYCOLOR3SVEXTPROC) (const GLshort * v);
     typedef void (*PFNGLSECONDARYCOLOR3UBEXTPROC) (GLubyte red,
	GLubyte green, GLubyte blue);
     typedef void (*PFNGLSECONDARYCOLOR3UBVEXTPROC) (const GLubyte * v);
     typedef void (*PFNGLSECONDARYCOLOR3UIEXTPROC) (GLuint red, GLuint green,
	GLuint blue);
     typedef void (*PFNGLSECONDARYCOLOR3UIVEXTPROC) (const GLuint * v);
     typedef void (*PFNGLSECONDARYCOLOR3USEXTPROC) (GLushort red,
	GLushort green, GLushort blue);
     typedef void (*PFNGLSECONDARYCOLOR3USVEXTPROC) (const GLushort * v);
     typedef void (*PFNGLSECONDARYCOLORPOINTEREXTPROC) (GLint size,
	GLenum type, GLsizei stride, const GLvoid * pointer);







     typedef void (*PFNGLTEXTURENORMALEXTPROC) (GLenum mode);
     typedef void (*PFNGLMULTIDRAWARRAYSEXTPROC) (GLenum mode,
	const GLint * first, const GLsizei * count, GLsizei primcount);
     typedef void (*PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode,
	const GLsizei * count, GLenum type, const GLvoid * *indices,
	GLsizei primcount);
     typedef void (*PFNGLFOGCOORDFEXTPROC) (GLfloat coord);
     typedef void (*PFNGLFOGCOORDFVEXTPROC) (const GLfloat * coord);
     typedef void (*PFNGLFOGCOORDDEXTPROC) (GLdouble coord);
     typedef void (*PFNGLFOGCOORDDVEXTPROC) (const GLdouble * coord);
     typedef void (*PFNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride,
	const GLvoid * pointer);
     typedef void (*PFNGLTANGENT3BEXTPROC) (GLbyte tx, GLbyte ty, GLbyte tz);
     typedef void (*PFNGLTANGENT3BVEXTPROC) (const GLbyte * v);
     typedef void (*PFNGLTANGENT3DEXTPROC) (GLdouble tx, GLdouble ty,
	GLdouble tz);
     typedef void (*PFNGLTANGENT3DVEXTPROC) (const GLdouble * v);
     typedef void (*PFNGLTANGENT3FEXTPROC) (GLfloat tx, GLfloat ty,
	GLfloat tz);
     typedef void (*PFNGLTANGENT3FVEXTPROC) (const GLfloat * v);
     typedef void (*PFNGLTANGENT3IEXTPROC) (GLint tx, GLint ty, GLint tz);
     typedef void (*PFNGLTANGENT3IVEXTPROC) (const GLint * v);
     typedef void (*PFNGLTANGENT3SEXTPROC) (GLshort tx, GLshort ty,
	GLshort tz);
     typedef void (*PFNGLTANGENT3SVEXTPROC) (const GLshort * v);
     typedef void (*PFNGLBINORMAL3BEXTPROC) (GLbyte bx, GLbyte by, GLbyte bz);
     typedef void (*PFNGLBINORMAL3BVEXTPROC) (const GLbyte * v);
     typedef void (*PFNGLBINORMAL3DEXTPROC) (GLdouble bx, GLdouble by,
	GLdouble bz);
     typedef void (*PFNGLBINORMAL3DVEXTPROC) (const GLdouble * v);
     typedef void (*PFNGLBINORMAL3FEXTPROC) (GLfloat bx, GLfloat by,
	GLfloat bz);
     typedef void (*PFNGLBINORMAL3FVEXTPROC) (const GLfloat * v);
     typedef void (*PFNGLBINORMAL3IEXTPROC) (GLint bx, GLint by, GLint bz);
     typedef void (*PFNGLBINORMAL3IVEXTPROC) (const GLint * v);
     typedef void (*PFNGLBINORMAL3SEXTPROC) (GLshort bx, GLshort by,
	GLshort bz);
     typedef void (*PFNGLBINORMAL3SVEXTPROC) (const GLshort * v);
     typedef void (*PFNGLTANGENTPOINTEREXTPROC) (GLenum type, GLsizei stride,
	const GLvoid * pointer);
     typedef void (*PFNGLBINORMALPOINTEREXTPROC) (GLenum type, GLsizei stride,
	const GLvoid * pointer);
     typedef void (*PFNGLFINISHTEXTURESUNXPROC) (void);
     typedef void (*PFNGLGLOBALALPHAFACTORBSUNPROC) (GLbyte factor);
     typedef void (*PFNGLGLOBALALPHAFACTORSSUNPROC) (GLshort factor);
     typedef void (*PFNGLGLOBALALPHAFACTORISUNPROC) (GLint factor);
     typedef void (*PFNGLGLOBALALPHAFACTORFSUNPROC) (GLfloat factor);
     typedef void (*PFNGLGLOBALALPHAFACTORDSUNPROC) (GLdouble factor);
     typedef void (*PFNGLGLOBALALPHAFACTORUBSUNPROC) (GLubyte factor);
     typedef void (*PFNGLGLOBALALPHAFACTORUSSUNPROC) (GLushort factor);
     typedef void (*PFNGLGLOBALALPHAFACTORUISUNPROC) (GLuint factor);
     typedef void (*PFNGLREPLACEMENTCODEUISUNPROC) (GLuint code);
     typedef void (*PFNGLREPLACEMENTCODEUSSUNPROC) (GLushort code);
     typedef void (*PFNGLREPLACEMENTCODEUBSUNPROC) (GLubyte code);
     typedef void (*PFNGLREPLACEMENTCODEUIVSUNPROC) (const GLuint * code);
     typedef void (*PFNGLREPLACEMENTCODEUSVSUNPROC) (const GLushort * code);
     typedef void (*PFNGLREPLACEMENTCODEUBVSUNPROC) (const GLubyte * code);
     typedef void (*PFNGLREPLACEMENTCODEPOINTERSUNPROC) (GLenum type,
	GLsizei stride, const GLvoid * *pointer);
     typedef void (*PFNGLCOLOR4UBVERTEX2FSUNPROC) (GLubyte r, GLubyte g,
	GLubyte b, GLubyte a, GLfloat x, GLfloat y);
     typedef void (*PFNGLCOLOR4UBVERTEX2FVSUNPROC) (const GLubyte * c,
	const GLfloat * v);
     typedef void (*PFNGLCOLOR4UBVERTEX3FSUNPROC) (GLubyte r, GLubyte g,
	GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLCOLOR4UBVERTEX3FVSUNPROC) (const GLubyte * c,
	const GLfloat * v);
     typedef void (*PFNGLCOLOR3FVERTEX3FSUNPROC) (GLfloat r, GLfloat g,
	GLfloat b, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLCOLOR3FVERTEX3FVSUNPROC) (const GLfloat * c,
	const GLfloat * v);
     typedef void (*PFNGLNORMAL3FVERTEX3FSUNPROC) (GLfloat nx, GLfloat ny,
	GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLNORMAL3FVERTEX3FVSUNPROC) (const GLfloat * n,
	const GLfloat * v);
     typedef void (*PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat r,
	GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz,
	GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat * c,
	const GLfloat * n, const GLfloat * v);
     typedef void (*PFNGLTEXCOORD2FVERTEX3FSUNPROC) (GLfloat s, GLfloat t,
	GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLTEXCOORD2FVERTEX3FVSUNPROC) (const GLfloat * tc,
	const GLfloat * v);
     typedef void (*PFNGLTEXCOORD4FVERTEX4FSUNPROC) (GLfloat s, GLfloat t,
	GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLTEXCOORD4FVERTEX4FVSUNPROC) (const GLfloat * tc,
	const GLfloat * v);
     typedef void (*PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC) (GLfloat s,
	GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x,
	GLfloat y, GLfloat z);
     typedef void (*PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC) (const GLfloat *
	tc, const GLubyte * c, const GLfloat * v);
     typedef void (*PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC) (GLfloat s,
	GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y,
	GLfloat z);
     typedef void (*PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC) (const GLfloat *
	tc, const GLfloat * c, const GLfloat * v);
     typedef void (*PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC) (GLfloat s,
	GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y,
	GLfloat z);
     typedef void (*PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *
	tc, const GLfloat * n, const GLfloat * v);
     typedef void (*PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat s,
	GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx,
	GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const
	GLfloat * tc, const GLfloat * c, const GLfloat * n,
	const GLfloat * v);
     typedef void (*PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC) (GLfloat s,
	GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b,
	GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y,
	GLfloat z, GLfloat w);
     typedef void (*PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC) (const
	GLfloat * tc, const GLfloat * c, const GLfloat * n,
	const GLfloat * v);
     typedef void (*PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC) (GLuint rc,
	GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC) (const GLuint *
	rc, const GLfloat * v);
     typedef void (*PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC) (GLuint rc,
	GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y,
	GLfloat z);
     typedef void (*PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC) (const
	GLuint * rc, const GLubyte * c, const GLfloat * v);
     typedef void (*PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC) (GLuint rc,
	GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC) (const
	GLuint * rc, const GLfloat * c, const GLfloat * v);
     typedef void (*PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC) (GLuint rc,
	GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC) (const
	GLuint * rc, const GLfloat * n, const GLfloat * v);
     typedef void (*PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC)
	(GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx,
	GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC)
	(const GLuint * rc, const GLfloat * c, const GLfloat * n,
	const GLfloat * v);
     typedef void (*PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC) (GLuint
	rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC) (const
	GLuint * rc, const GLfloat * tc, const GLfloat * v);
     typedef void (*PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC)
	(GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz,
	GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC)
	(const GLuint * rc, const GLfloat * tc, const GLfloat * n,
	const GLfloat * v);
     typedef void
	     (*PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC)
	(GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b,
	GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y,
	GLfloat z);
     typedef void
	     (*PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC)
	(const GLuint * rc, const GLfloat * tc, const GLfloat * c,
	const GLfloat * n, const GLfloat * v);







     typedef void (*PFNGLBLENDFUNCSEPARATEEXTPROC) (GLenum sfactorRGB,
	GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);







     typedef void (*PFNGLBLENDFUNCSEPARATEINGRPROC) (GLenum sfactorRGB,
	GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
     typedef void (*PFNGLVERTEXWEIGHTFEXTPROC) (GLfloat weight);
     typedef void (*PFNGLVERTEXWEIGHTFVEXTPROC) (const GLfloat * weight);
     typedef void (*PFNGLVERTEXWEIGHTPOINTEREXTPROC) (GLsizei size,
	GLenum type, GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
     typedef void (*PFNGLVERTEXARRAYRANGENVPROC) (GLsizei length,
	const GLvoid * pointer);
     typedef void (*PFNGLCOMBINERPARAMETERFVNVPROC) (GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLCOMBINERPARAMETERFNVPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLCOMBINERPARAMETERIVNVPROC) (GLenum pname,
	const GLint * params);
     typedef void (*PFNGLCOMBINERPARAMETERINVPROC) (GLenum pname,
	GLint param);
     typedef void (*PFNGLCOMBINERINPUTNVPROC) (GLenum stage, GLenum portion,
	GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
     typedef void (*PFNGLCOMBINEROUTPUTNVPROC) (GLenum stage, GLenum portion,
	GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale,
	GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct,
	GLboolean muxSum);
     typedef void (*PFNGLFINALCOMBINERINPUTNVPROC) (GLenum variable,
	GLenum input, GLenum mapping, GLenum componentUsage);
     typedef void (*PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC) (GLenum stage,
	GLenum portion, GLenum variable, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC) (GLenum stage,
	GLenum portion, GLenum variable, GLenum pname, GLint * params);
     typedef void (*PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC) (GLenum stage,
	GLenum portion, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC) (GLenum stage,
	GLenum portion, GLenum pname, GLint * params);
     typedef void (*PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC) (GLenum
	variable, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC) (GLenum
	variable, GLenum pname, GLint * params);
     typedef void (*PFNGLRESIZEBUFFERSMESAPROC) (void);
     typedef void (*PFNGLWINDOWPOS2DMESAPROC) (GLdouble x, GLdouble y);
     typedef void (*PFNGLWINDOWPOS2DVMESAPROC) (const GLdouble * v);
     typedef void (*PFNGLWINDOWPOS2FMESAPROC) (GLfloat x, GLfloat y);
     typedef void (*PFNGLWINDOWPOS2FVMESAPROC) (const GLfloat * v);
     typedef void (*PFNGLWINDOWPOS2IMESAPROC) (GLint x, GLint y);
     typedef void (*PFNGLWINDOWPOS2IVMESAPROC) (const GLint * v);
     typedef void (*PFNGLWINDOWPOS2SMESAPROC) (GLshort x, GLshort y);
     typedef void (*PFNGLWINDOWPOS2SVMESAPROC) (const GLshort * v);
     typedef void (*PFNGLWINDOWPOS3DMESAPROC) (GLdouble x, GLdouble y,
	GLdouble z);
     typedef void (*PFNGLWINDOWPOS3DVMESAPROC) (const GLdouble * v);
     typedef void (*PFNGLWINDOWPOS3FMESAPROC) (GLfloat x, GLfloat y,
	GLfloat z);
     typedef void (*PFNGLWINDOWPOS3FVMESAPROC) (const GLfloat * v);
     typedef void (*PFNGLWINDOWPOS3IMESAPROC) (GLint x, GLint y, GLint z);
     typedef void (*PFNGLWINDOWPOS3IVMESAPROC) (const GLint * v);
     typedef void (*PFNGLWINDOWPOS3SMESAPROC) (GLshort x, GLshort y,
	GLshort z);
     typedef void (*PFNGLWINDOWPOS3SVMESAPROC) (const GLshort * v);
     typedef void (*PFNGLWINDOWPOS4DMESAPROC) (GLdouble x, GLdouble y,
	GLdouble z, GLdouble w);
     typedef void (*PFNGLWINDOWPOS4DVMESAPROC) (const GLdouble * v);
     typedef void (*PFNGLWINDOWPOS4FMESAPROC) (GLfloat x, GLfloat y,
	GLfloat z, GLfloat w);
     typedef void (*PFNGLWINDOWPOS4FVMESAPROC) (const GLfloat * v);
     typedef void (*PFNGLWINDOWPOS4IMESAPROC) (GLint x, GLint y, GLint z,
	GLint w);
     typedef void (*PFNGLWINDOWPOS4IVMESAPROC) (const GLint * v);
     typedef void (*PFNGLWINDOWPOS4SMESAPROC) (GLshort x, GLshort y,
	GLshort z, GLshort w);
     typedef void (*PFNGLWINDOWPOS4SVMESAPROC) (const GLshort * v);
     typedef void (*PFNGLMULTIMODEDRAWARRAYSIBMPROC) (const GLenum * mode,
	const GLint * first, const GLsizei * count, GLsizei primcount,
	GLint modestride);
     typedef void (*PFNGLMULTIMODEDRAWELEMENTSIBMPROC) (const GLenum * mode,
	const GLsizei * count, GLenum type, const GLvoid * const *indices,
	GLsizei primcount, GLint modestride);
     typedef void (*PFNGLCOLORPOINTERLISTIBMPROC) (GLint size, GLenum type,
	GLint stride, const GLvoid * *pointer, GLint ptrstride);
     typedef void (*PFNGLSECONDARYCOLORPOINTERLISTIBMPROC) (GLint size,
	GLenum type, GLint stride, const GLvoid * *pointer, GLint ptrstride);
     typedef void (*PFNGLEDGEFLAGPOINTERLISTIBMPROC) (GLint stride,
	const GLboolean * *pointer, GLint ptrstride);
     typedef void (*PFNGLFOGCOORDPOINTERLISTIBMPROC) (GLenum type,
	GLint stride, const GLvoid * *pointer, GLint ptrstride);
     typedef void (*PFNGLINDEXPOINTERLISTIBMPROC) (GLenum type, GLint stride,
	const GLvoid * *pointer, GLint ptrstride);
     typedef void (*PFNGLNORMALPOINTERLISTIBMPROC) (GLenum type, GLint stride,
	const GLvoid * *pointer, GLint ptrstride);
     typedef void (*PFNGLTEXCOORDPOINTERLISTIBMPROC) (GLint size, GLenum type,
	GLint stride, const GLvoid * *pointer, GLint ptrstride);
     typedef void (*PFNGLVERTEXPOINTERLISTIBMPROC) (GLint size, GLenum type,
	GLint stride, const GLvoid * *pointer, GLint ptrstride);
     typedef void (*PFNGLTBUFFERMASK3DFXPROC) (GLuint mask);
     typedef void (*PFNGLSAMPLEMASKEXTPROC) (GLclampf value,
	GLboolean invert);
     typedef void (*PFNGLSAMPLEPATTERNEXTPROC) (GLenum pattern);
     typedef void (*PFNGLTEXTURECOLORMASKSGISPROC) (GLboolean red,
	GLboolean green, GLboolean blue, GLboolean alpha);







     typedef void (*PFNGLIGLOOINTERFACESGIXPROC) (GLenum pname,
	const GLvoid * params);
     typedef void (*PFNGLDELETEFENCESNVPROC) (GLsizei n,
	const GLuint * fences);
     typedef void (*PFNGLGENFENCESNVPROC) (GLsizei n, GLuint * fences);
     typedef GLboolean(*PFNGLISFENCENVPROC) (GLuint fence);
     typedef GLboolean(*PFNGLTESTFENCENVPROC) (GLuint fence);
     typedef void (*PFNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname,
	GLint * params);
     typedef void (*PFNGLFINISHFENCENVPROC) (GLuint fence);
     typedef void (*PFNGLSETFENCENVPROC) (GLuint fence, GLenum condition);
     typedef void (*PFNGLMAPCONTROLPOINTSNVPROC) (GLenum target, GLuint index,
	GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder,
	GLint vorder, GLboolean packed, const GLvoid * points);
     typedef void (*PFNGLMAPPARAMETERIVNVPROC) (GLenum target, GLenum pname,
	const GLint * params);
     typedef void (*PFNGLMAPPARAMETERFVNVPROC) (GLenum target, GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLGETMAPCONTROLPOINTSNVPROC) (GLenum target,
	GLuint index, GLenum type, GLsizei ustride, GLsizei vstride,
	GLboolean packed, GLvoid * points);
     typedef void (*PFNGLGETMAPPARAMETERIVNVPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETMAPPARAMETERFVNVPROC) (GLenum target,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETMAPATTRIBPARAMETERIVNVPROC) (GLenum target,
	GLuint index, GLenum pname, GLint * params);
     typedef void (*PFNGLGETMAPATTRIBPARAMETERFVNVPROC) (GLenum target,
	GLuint index, GLenum pname, GLfloat * params);
     typedef void (*PFNGLEVALMAPSNVPROC) (GLenum target, GLenum mode);
     typedef void (*PFNGLCOMBINERSTAGEPARAMETERFVNVPROC) (GLenum stage,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC) (GLenum stage,
	GLenum pname, GLfloat * params);
     typedef GLboolean(*PFNGLAREPROGRAMSRESIDENTNVPROC) (GLsizei n,
	const GLuint * programs, GLboolean * residences);
     typedef void (*PFNGLBINDPROGRAMNVPROC) (GLenum target, GLuint id);
     typedef void (*PFNGLDELETEPROGRAMSNVPROC) (GLsizei n,
	const GLuint * programs);
     typedef void (*PFNGLEXECUTEPROGRAMNVPROC) (GLenum target, GLuint id,
	const GLfloat * params);
     typedef void (*PFNGLGENPROGRAMSNVPROC) (GLsizei n, GLuint * programs);
     typedef void (*PFNGLGETPROGRAMPARAMETERDVNVPROC) (GLenum target,
	GLuint index, GLenum pname, GLdouble * params);
     typedef void (*PFNGLGETPROGRAMPARAMETERFVNVPROC) (GLenum target,
	GLuint index, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETPROGRAMIVNVPROC) (GLuint id, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETPROGRAMSTRINGNVPROC) (GLuint id, GLenum pname,
	GLubyte * program);
     typedef void (*PFNGLGETTRACKMATRIXIVNVPROC) (GLenum target,
	GLuint address, GLenum pname, GLint * params);
     typedef void (*PFNGLGETVERTEXATTRIBDVNVPROC) (GLuint index, GLenum pname,
	GLdouble * params);
     typedef void (*PFNGLGETVERTEXATTRIBFVNVPROC) (GLuint index, GLenum pname,
	GLfloat * params);
     typedef void (*PFNGLGETVERTEXATTRIBIVNVPROC) (GLuint index, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETVERTEXATTRIBPOINTERVNVPROC) (GLuint index,
	GLenum pname, GLvoid * *pointer);
     typedef GLboolean(*PFNGLISPROGRAMNVPROC) (GLuint id);
     typedef void (*PFNGLLOADPROGRAMNVPROC) (GLenum target, GLuint id,
	GLsizei len, const GLubyte * program);
     typedef void (*PFNGLPROGRAMPARAMETER4DNVPROC) (GLenum target,
	GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLPROGRAMPARAMETER4DVNVPROC) (GLenum target,
	GLuint index, const GLdouble * v);
     typedef void (*PFNGLPROGRAMPARAMETER4FNVPROC) (GLenum target,
	GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLPROGRAMPARAMETER4FVNVPROC) (GLenum target,
	GLuint index, const GLfloat * v);
     typedef void (*PFNGLPROGRAMPARAMETERS4DVNVPROC) (GLenum target,
	GLuint index, GLsizei count, const GLdouble * v);
     typedef void (*PFNGLPROGRAMPARAMETERS4FVNVPROC) (GLenum target,
	GLuint index, GLsizei count, const GLfloat * v);
     typedef void (*PFNGLREQUESTRESIDENTPROGRAMSNVPROC) (GLsizei n,
	const GLuint * programs);
     typedef void (*PFNGLTRACKMATRIXNVPROC) (GLenum target, GLuint address,
	GLenum matrix, GLenum transform);
     typedef void (*PFNGLVERTEXATTRIBPOINTERNVPROC) (GLuint index,
	GLint fsize, GLenum type, GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLVERTEXATTRIB1DNVPROC) (GLuint index, GLdouble x);
     typedef void (*PFNGLVERTEXATTRIB1DVNVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB1FNVPROC) (GLuint index, GLfloat x);
     typedef void (*PFNGLVERTEXATTRIB1FVNVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB1SNVPROC) (GLuint index, GLshort x);
     typedef void (*PFNGLVERTEXATTRIB1SVNVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB2DNVPROC) (GLuint index, GLdouble x,
	GLdouble y);
     typedef void (*PFNGLVERTEXATTRIB2DVNVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB2FNVPROC) (GLuint index, GLfloat x,
	GLfloat y);
     typedef void (*PFNGLVERTEXATTRIB2FVNVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB2SNVPROC) (GLuint index, GLshort x,
	GLshort y);
     typedef void (*PFNGLVERTEXATTRIB2SVNVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB3DNVPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLVERTEXATTRIB3DVNVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB3FNVPROC) (GLuint index, GLfloat x,
	GLfloat y, GLfloat z);
     typedef void (*PFNGLVERTEXATTRIB3FVNVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB3SNVPROC) (GLuint index, GLshort x,
	GLshort y, GLshort z);
     typedef void (*PFNGLVERTEXATTRIB3SVNVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4DNVPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLVERTEXATTRIB4DVNVPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIB4FNVPROC) (GLuint index, GLfloat x,
	GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLVERTEXATTRIB4FVNVPROC) (GLuint index,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIB4SNVPROC) (GLuint index, GLshort x,
	GLshort y, GLshort z, GLshort w);
     typedef void (*PFNGLVERTEXATTRIB4SVNVPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIB4UBNVPROC) (GLuint index, GLubyte x,
	GLubyte y, GLubyte z, GLubyte w);
     typedef void (*PFNGLVERTEXATTRIB4UBVNVPROC) (GLuint index,
	const GLubyte * v);
     typedef void (*PFNGLVERTEXATTRIBS1DVNVPROC) (GLuint index, GLsizei count,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBS1FVNVPROC) (GLuint index, GLsizei count,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIBS1SVNVPROC) (GLuint index, GLsizei count,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIBS2DVNVPROC) (GLuint index, GLsizei count,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBS2FVNVPROC) (GLuint index, GLsizei count,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIBS2SVNVPROC) (GLuint index, GLsizei count,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIBS3DVNVPROC) (GLuint index, GLsizei count,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBS3FVNVPROC) (GLuint index, GLsizei count,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIBS3SVNVPROC) (GLuint index, GLsizei count,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIBS4DVNVPROC) (GLuint index, GLsizei count,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBS4FVNVPROC) (GLuint index, GLsizei count,
	const GLfloat * v);
     typedef void (*PFNGLVERTEXATTRIBS4SVNVPROC) (GLuint index, GLsizei count,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIBS4UBVNVPROC) (GLuint index,
	GLsizei count, const GLubyte * v);
     typedef void (*PFNGLTEXBUMPPARAMETERIVATIPROC) (GLenum pname,
	const GLint * param);
     typedef void (*PFNGLTEXBUMPPARAMETERFVATIPROC) (GLenum pname,
	const GLfloat * param);
     typedef void (*PFNGLGETTEXBUMPPARAMETERIVATIPROC) (GLenum pname,
	GLint * param);
     typedef void (*PFNGLGETTEXBUMPPARAMETERFVATIPROC) (GLenum pname,
	GLfloat * param);
     typedef GLuint(*PFNGLGENFRAGMENTSHADERSATIPROC) (GLuint range);
     typedef void (*PFNGLBINDFRAGMENTSHADERATIPROC) (GLuint id);
     typedef void (*PFNGLDELETEFRAGMENTSHADERATIPROC) (GLuint id);
     typedef void (*PFNGLBEGINFRAGMENTSHADERATIPROC) (void);
     typedef void (*PFNGLENDFRAGMENTSHADERATIPROC) (void);
     typedef void (*PFNGLPASSTEXCOORDATIPROC) (GLuint dst, GLuint coord,
	GLenum swizzle);
     typedef void (*PFNGLSAMPLEMAPATIPROC) (GLuint dst, GLuint interp,
	GLenum swizzle);
     typedef void (*PFNGLCOLORFRAGMENTOP1ATIPROC) (GLenum op, GLuint dst,
	GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep,
	GLuint arg1Mod);
     typedef void (*PFNGLCOLORFRAGMENTOP2ATIPROC) (GLenum op, GLuint dst,
	GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep,
	GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
     typedef void (*PFNGLCOLORFRAGMENTOP3ATIPROC) (GLenum op, GLuint dst,
	GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep,
	GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod,
	GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
     typedef void (*PFNGLALPHAFRAGMENTOP1ATIPROC) (GLenum op, GLuint dst,
	GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
     typedef void (*PFNGLALPHAFRAGMENTOP2ATIPROC) (GLenum op, GLuint dst,
	GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod,
	GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
     typedef void (*PFNGLALPHAFRAGMENTOP3ATIPROC) (GLenum op, GLuint dst,
	GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod,
	GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3,
	GLuint arg3Rep, GLuint arg3Mod);
     typedef void (*PFNGLSETFRAGMENTSHADERCONSTANTATIPROC) (GLuint dst,
	const GLfloat * value);
     typedef void (*PFNGLPNTRIANGLESIATIPROC) (GLenum pname, GLint param);
     typedef void (*PFNGLPNTRIANGLESFATIPROC) (GLenum pname, GLfloat param);
     typedef GLuint(*PFNGLNEWOBJECTBUFFERATIPROC) (GLsizei size,
	const GLvoid * pointer, GLenum usage);
     typedef GLboolean(*PFNGLISOBJECTBUFFERATIPROC) (GLuint buffer);
     typedef void (*PFNGLUPDATEOBJECTBUFFERATIPROC) (GLuint buffer,
	GLuint offset, GLsizei size, const GLvoid * pointer, GLenum preserve);
     typedef void (*PFNGLGETOBJECTBUFFERFVATIPROC) (GLuint buffer,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETOBJECTBUFFERIVATIPROC) (GLuint buffer,
	GLenum pname, GLint * params);
     typedef void (*PFNGLFREEOBJECTBUFFERATIPROC) (GLuint buffer);
     typedef void (*PFNGLARRAYOBJECTATIPROC) (GLenum array, GLint size,
	GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
     typedef void (*PFNGLGETARRAYOBJECTFVATIPROC) (GLenum array, GLenum pname,
	GLfloat * params);
     typedef void (*PFNGLGETARRAYOBJECTIVATIPROC) (GLenum array, GLenum pname,
	GLint * params);
     typedef void (*PFNGLVARIANTARRAYOBJECTATIPROC) (GLuint id, GLenum type,
	GLsizei stride, GLuint buffer, GLuint offset);
     typedef void (*PFNGLGETVARIANTARRAYOBJECTFVATIPROC) (GLuint id,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETVARIANTARRAYOBJECTIVATIPROC) (GLuint id,
	GLenum pname, GLint * params);
     typedef void (*PFNGLBEGINVERTEXSHADEREXTPROC) (void);
     typedef void (*PFNGLENDVERTEXSHADEREXTPROC) (void);
     typedef void (*PFNGLBINDVERTEXSHADEREXTPROC) (GLuint id);
     typedef GLuint(*PFNGLGENVERTEXSHADERSEXTPROC) (GLuint range);
     typedef void (*PFNGLDELETEVERTEXSHADEREXTPROC) (GLuint id);
     typedef void (*PFNGLSHADEROP1EXTPROC) (GLenum op, GLuint res,
	GLuint arg1);
     typedef void (*PFNGLSHADEROP2EXTPROC) (GLenum op, GLuint res,
	GLuint arg1, GLuint arg2);
     typedef void (*PFNGLSHADEROP3EXTPROC) (GLenum op, GLuint res,
	GLuint arg1, GLuint arg2, GLuint arg3);
     typedef void (*PFNGLSWIZZLEEXTPROC) (GLuint res, GLuint in, GLenum outX,
	GLenum outY, GLenum outZ, GLenum outW);
     typedef void (*PFNGLWRITEMASKEXTPROC) (GLuint res, GLuint in,
	GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
     typedef void (*PFNGLINSERTCOMPONENTEXTPROC) (GLuint res, GLuint src,
	GLuint num);
     typedef void (*PFNGLEXTRACTCOMPONENTEXTPROC) (GLuint res, GLuint src,
	GLuint num);
     typedef GLuint(*PFNGLGENSYMBOLSEXTPROC) (GLenum datatype,
	GLenum storagetype, GLenum range, GLuint components);
     typedef void (*PFNGLSETINVARIANTEXTPROC) (GLuint id, GLenum type,
	const GLvoid * addr);
     typedef void (*PFNGLSETLOCALCONSTANTEXTPROC) (GLuint id, GLenum type,
	const GLvoid * addr);
     typedef void (*PFNGLVARIANTBVEXTPROC) (GLuint id, const GLbyte * addr);
     typedef void (*PFNGLVARIANTSVEXTPROC) (GLuint id, const GLshort * addr);
     typedef void (*PFNGLVARIANTIVEXTPROC) (GLuint id, const GLint * addr);
     typedef void (*PFNGLVARIANTFVEXTPROC) (GLuint id, const GLfloat * addr);
     typedef void (*PFNGLVARIANTDVEXTPROC) (GLuint id, const GLdouble * addr);
     typedef void (*PFNGLVARIANTUBVEXTPROC) (GLuint id, const GLubyte * addr);
     typedef void (*PFNGLVARIANTUSVEXTPROC) (GLuint id,
	const GLushort * addr);
     typedef void (*PFNGLVARIANTUIVEXTPROC) (GLuint id, const GLuint * addr);
     typedef void (*PFNGLVARIANTPOINTEREXTPROC) (GLuint id, GLenum type,
	GLuint stride, const GLvoid * addr);
     typedef void (*PFNGLENABLEVARIANTCLIENTSTATEEXTPROC) (GLuint id);
     typedef void (*PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC) (GLuint id);
     typedef GLuint(*PFNGLBINDLIGHTPARAMETEREXTPROC) (GLenum light,
	GLenum value);
     typedef GLuint(*PFNGLBINDMATERIALPARAMETEREXTPROC) (GLenum face,
	GLenum value);
     typedef GLuint(*PFNGLBINDTEXGENPARAMETEREXTPROC) (GLenum unit,
	GLenum coord, GLenum value);
     typedef GLuint(*PFNGLBINDTEXTUREUNITPARAMETEREXTPROC) (GLenum unit,
	GLenum value);
     typedef GLuint(*PFNGLBINDPARAMETEREXTPROC) (GLenum value);
     typedef GLboolean(*PFNGLISVARIANTENABLEDEXTPROC) (GLuint id, GLenum cap);
     typedef void (*PFNGLGETVARIANTBOOLEANVEXTPROC) (GLuint id, GLenum value,
	GLboolean * data);
     typedef void (*PFNGLGETVARIANTINTEGERVEXTPROC) (GLuint id, GLenum value,
	GLint * data);
     typedef void (*PFNGLGETVARIANTFLOATVEXTPROC) (GLuint id, GLenum value,
	GLfloat * data);
     typedef void (*PFNGLGETVARIANTPOINTERVEXTPROC) (GLuint id, GLenum value,
	GLvoid * *data);
     typedef void (*PFNGLGETINVARIANTBOOLEANVEXTPROC) (GLuint id,
	GLenum value, GLboolean * data);
     typedef void (*PFNGLGETINVARIANTINTEGERVEXTPROC) (GLuint id,
	GLenum value, GLint * data);
     typedef void (*PFNGLGETINVARIANTFLOATVEXTPROC) (GLuint id, GLenum value,
	GLfloat * data);
     typedef void (*PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC) (GLuint id,
	GLenum value, GLboolean * data);
     typedef void (*PFNGLGETLOCALCONSTANTINTEGERVEXTPROC) (GLuint id,
	GLenum value, GLint * data);
     typedef void (*PFNGLGETLOCALCONSTANTFLOATVEXTPROC) (GLuint id,
	GLenum value, GLfloat * data);
     typedef void (*PFNGLVERTEXSTREAM1SATIPROC) (GLenum stream, GLshort x);
     typedef void (*PFNGLVERTEXSTREAM1SVATIPROC) (GLenum stream,
	const GLshort * coords);
     typedef void (*PFNGLVERTEXSTREAM1IATIPROC) (GLenum stream, GLint x);
     typedef void (*PFNGLVERTEXSTREAM1IVATIPROC) (GLenum stream,
	const GLint * coords);
     typedef void (*PFNGLVERTEXSTREAM1FATIPROC) (GLenum stream, GLfloat x);
     typedef void (*PFNGLVERTEXSTREAM1FVATIPROC) (GLenum stream,
	const GLfloat * coords);
     typedef void (*PFNGLVERTEXSTREAM1DATIPROC) (GLenum stream, GLdouble x);
     typedef void (*PFNGLVERTEXSTREAM1DVATIPROC) (GLenum stream,
	const GLdouble * coords);
     typedef void (*PFNGLVERTEXSTREAM2SATIPROC) (GLenum stream, GLshort x,
	GLshort y);
     typedef void (*PFNGLVERTEXSTREAM2SVATIPROC) (GLenum stream,
	const GLshort * coords);
     typedef void (*PFNGLVERTEXSTREAM2IATIPROC) (GLenum stream, GLint x,
	GLint y);
     typedef void (*PFNGLVERTEXSTREAM2IVATIPROC) (GLenum stream,
	const GLint * coords);
     typedef void (*PFNGLVERTEXSTREAM2FATIPROC) (GLenum stream, GLfloat x,
	GLfloat y);
     typedef void (*PFNGLVERTEXSTREAM2FVATIPROC) (GLenum stream,
	const GLfloat * coords);
     typedef void (*PFNGLVERTEXSTREAM2DATIPROC) (GLenum stream, GLdouble x,
	GLdouble y);
     typedef void (*PFNGLVERTEXSTREAM2DVATIPROC) (GLenum stream,
	const GLdouble * coords);
     typedef void (*PFNGLVERTEXSTREAM3SATIPROC) (GLenum stream, GLshort x,
	GLshort y, GLshort z);
     typedef void (*PFNGLVERTEXSTREAM3SVATIPROC) (GLenum stream,
	const GLshort * coords);
     typedef void (*PFNGLVERTEXSTREAM3IATIPROC) (GLenum stream, GLint x,
	GLint y, GLint z);
     typedef void (*PFNGLVERTEXSTREAM3IVATIPROC) (GLenum stream,
	const GLint * coords);
     typedef void (*PFNGLVERTEXSTREAM3FATIPROC) (GLenum stream, GLfloat x,
	GLfloat y, GLfloat z);
     typedef void (*PFNGLVERTEXSTREAM3FVATIPROC) (GLenum stream,
	const GLfloat * coords);
     typedef void (*PFNGLVERTEXSTREAM3DATIPROC) (GLenum stream, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLVERTEXSTREAM3DVATIPROC) (GLenum stream,
	const GLdouble * coords);
     typedef void (*PFNGLVERTEXSTREAM4SATIPROC) (GLenum stream, GLshort x,
	GLshort y, GLshort z, GLshort w);
     typedef void (*PFNGLVERTEXSTREAM4SVATIPROC) (GLenum stream,
	const GLshort * coords);
     typedef void (*PFNGLVERTEXSTREAM4IATIPROC) (GLenum stream, GLint x,
	GLint y, GLint z, GLint w);
     typedef void (*PFNGLVERTEXSTREAM4IVATIPROC) (GLenum stream,
	const GLint * coords);
     typedef void (*PFNGLVERTEXSTREAM4FATIPROC) (GLenum stream, GLfloat x,
	GLfloat y, GLfloat z, GLfloat w);
     typedef void (*PFNGLVERTEXSTREAM4FVATIPROC) (GLenum stream,
	const GLfloat * coords);
     typedef void (*PFNGLVERTEXSTREAM4DATIPROC) (GLenum stream, GLdouble x,
	GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLVERTEXSTREAM4DVATIPROC) (GLenum stream,
	const GLdouble * coords);
     typedef void (*PFNGLNORMALSTREAM3BATIPROC) (GLenum stream, GLbyte nx,
	GLbyte ny, GLbyte nz);
     typedef void (*PFNGLNORMALSTREAM3BVATIPROC) (GLenum stream,
	const GLbyte * coords);
     typedef void (*PFNGLNORMALSTREAM3SATIPROC) (GLenum stream, GLshort nx,
	GLshort ny, GLshort nz);
     typedef void (*PFNGLNORMALSTREAM3SVATIPROC) (GLenum stream,
	const GLshort * coords);
     typedef void (*PFNGLNORMALSTREAM3IATIPROC) (GLenum stream, GLint nx,
	GLint ny, GLint nz);
     typedef void (*PFNGLNORMALSTREAM3IVATIPROC) (GLenum stream,
	const GLint * coords);
     typedef void (*PFNGLNORMALSTREAM3FATIPROC) (GLenum stream, GLfloat nx,
	GLfloat ny, GLfloat nz);
     typedef void (*PFNGLNORMALSTREAM3FVATIPROC) (GLenum stream,
	const GLfloat * coords);
     typedef void (*PFNGLNORMALSTREAM3DATIPROC) (GLenum stream, GLdouble nx,
	GLdouble ny, GLdouble nz);
     typedef void (*PFNGLNORMALSTREAM3DVATIPROC) (GLenum stream,
	const GLdouble * coords);
     typedef void (*PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC) (GLenum stream);
     typedef void (*PFNGLVERTEXBLENDENVIATIPROC) (GLenum pname, GLint param);
     typedef void (*PFNGLVERTEXBLENDENVFATIPROC) (GLenum pname,
	GLfloat param);
     typedef void (*PFNGLELEMENTPOINTERATIPROC) (GLenum type,
	const GLvoid * pointer);
     typedef void (*PFNGLDRAWELEMENTARRAYATIPROC) (GLenum mode,
	GLsizei count);
     typedef void (*PFNGLDRAWRANGEELEMENTARRAYATIPROC) (GLenum mode,
	GLuint start, GLuint end, GLsizei count);







     typedef void (*PFNGLDRAWMESHARRAYSSUNPROC) (GLenum mode, GLint first,
	GLsizei count, GLsizei width);
     typedef void (*PFNGLGENOCCLUSIONQUERIESNVPROC) (GLsizei n, GLuint * ids);
     typedef void (*PFNGLDELETEOCCLUSIONQUERIESNVPROC) (GLsizei n,
	const GLuint * ids);
     typedef GLboolean(*PFNGLISOCCLUSIONQUERYNVPROC) (GLuint id);
     typedef void (*PFNGLBEGINOCCLUSIONQUERYNVPROC) (GLuint id);
     typedef void (*PFNGLENDOCCLUSIONQUERYNVPROC) (void);
     typedef void (*PFNGLGETOCCLUSIONQUERYIVNVPROC) (GLuint id, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETOCCLUSIONQUERYUIVNVPROC) (GLuint id, GLenum pname,
	GLuint * params);
     typedef void (*PFNGLPOINTPARAMETERINVPROC) (GLenum pname, GLint param);
     typedef void (*PFNGLPOINTPARAMETERIVNVPROC) (GLenum pname,
	const GLint * params);
     typedef void (*PFNGLACTIVESTENCILFACEEXTPROC) (GLenum face);
     typedef void (*PFNGLELEMENTPOINTERAPPLEPROC) (GLenum type,
	const GLvoid * pointer);
     typedef void (*PFNGLDRAWELEMENTARRAYAPPLEPROC) (GLenum mode, GLint first,
	GLsizei count);
     typedef void (*PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC) (GLenum mode,
	GLuint start, GLuint end, GLint first, GLsizei count);
     typedef void (*PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC) (GLenum mode,
	const GLint * first, const GLsizei * count, GLsizei primcount);
     typedef void (*PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC) (GLenum mode,
	GLuint start, GLuint end, const GLint * first, const GLsizei * count,
	GLsizei primcount);
     typedef void (*PFNGLGENFENCESAPPLEPROC) (GLsizei n, GLuint * fences);
     typedef void (*PFNGLDELETEFENCESAPPLEPROC) (GLsizei n,
	const GLuint * fences);
     typedef void (*PFNGLSETFENCEAPPLEPROC) (GLuint fence);
     typedef GLboolean(*PFNGLISFENCEAPPLEPROC) (GLuint fence);
     typedef GLboolean(*PFNGLTESTFENCEAPPLEPROC) (GLuint fence);
     typedef void (*PFNGLFINISHFENCEAPPLEPROC) (GLuint fence);
     typedef GLboolean(*PFNGLTESTOBJECTAPPLEPROC) (GLenum object,
	GLuint name);
     typedef void (*PFNGLFINISHOBJECTAPPLEPROC) (GLenum object, GLint name);
     typedef void (*PFNGLBINDVERTEXARRAYAPPLEPROC) (GLuint array);
     typedef void (*PFNGLDELETEVERTEXARRAYSAPPLEPROC) (GLsizei n,
	const GLuint * arrays);
     typedef void (*PFNGLGENVERTEXARRAYSAPPLEPROC) (GLsizei n,
	GLuint * arrays);
     typedef GLboolean(*PFNGLISVERTEXARRAYAPPLEPROC) (GLuint array);
     typedef void (*PFNGLVERTEXARRAYRANGEAPPLEPROC) (GLsizei length,
	GLvoid * pointer);
     typedef void (*PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC) (GLsizei length,
	GLvoid * pointer);
     typedef void (*PFNGLVERTEXARRAYPARAMETERIAPPLEPROC) (GLenum pname,
	GLint param);
     typedef void (*PFNGLDRAWBUFFERSATIPROC) (GLsizei n, const GLenum * bufs);
     typedef void (*PFNGLPROGRAMNAMEDPARAMETER4FNVPROC) (GLuint id,
	GLsizei len, const GLubyte * name, GLfloat x, GLfloat y, GLfloat z,
	GLfloat w);
     typedef void (*PFNGLPROGRAMNAMEDPARAMETER4DNVPROC) (GLuint id,
	GLsizei len, const GLubyte * name, GLdouble x, GLdouble y, GLdouble z,
	GLdouble w);
     typedef void (*PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC) (GLuint id,
	GLsizei len, const GLubyte * name, const GLfloat * v);
     typedef void (*PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC) (GLuint id,
	GLsizei len, const GLubyte * name, const GLdouble * v);
     typedef void (*PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC) (GLuint id,
	GLsizei len, const GLubyte * name, GLfloat * params);
     typedef void (*PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC) (GLuint id,
	GLsizei len, const GLubyte * name, GLdouble * params);
     typedef void (*PFNGLVERTEX2HNVPROC) (GLhalfNV x, GLhalfNV y);
     typedef void (*PFNGLVERTEX2HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLVERTEX3HNVPROC) (GLhalfNV x, GLhalfNV y, GLhalfNV z);
     typedef void (*PFNGLVERTEX3HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLVERTEX4HNVPROC) (GLhalfNV x, GLhalfNV y, GLhalfNV z,
	GLhalfNV w);
     typedef void (*PFNGLVERTEX4HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLNORMAL3HNVPROC) (GLhalfNV nx, GLhalfNV ny,
	GLhalfNV nz);
     typedef void (*PFNGLNORMAL3HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLCOLOR3HNVPROC) (GLhalfNV red, GLhalfNV green,
	GLhalfNV blue);
     typedef void (*PFNGLCOLOR3HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLCOLOR4HNVPROC) (GLhalfNV red, GLhalfNV green,
	GLhalfNV blue, GLhalfNV alpha);
     typedef void (*PFNGLCOLOR4HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLTEXCOORD1HNVPROC) (GLhalfNV s);
     typedef void (*PFNGLTEXCOORD1HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLTEXCOORD2HNVPROC) (GLhalfNV s, GLhalfNV t);
     typedef void (*PFNGLTEXCOORD2HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLTEXCOORD3HNVPROC) (GLhalfNV s, GLhalfNV t,
	GLhalfNV r);
     typedef void (*PFNGLTEXCOORD3HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLTEXCOORD4HNVPROC) (GLhalfNV s, GLhalfNV t,
	GLhalfNV r, GLhalfNV q);
     typedef void (*PFNGLTEXCOORD4HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLMULTITEXCOORD1HNVPROC) (GLenum target, GLhalfNV s);
     typedef void (*PFNGLMULTITEXCOORD1HVNVPROC) (GLenum target,
	const GLhalfNV * v);
     typedef void (*PFNGLMULTITEXCOORD2HNVPROC) (GLenum target, GLhalfNV s,
	GLhalfNV t);
     typedef void (*PFNGLMULTITEXCOORD2HVNVPROC) (GLenum target,
	const GLhalfNV * v);
     typedef void (*PFNGLMULTITEXCOORD3HNVPROC) (GLenum target, GLhalfNV s,
	GLhalfNV t, GLhalfNV r);
     typedef void (*PFNGLMULTITEXCOORD3HVNVPROC) (GLenum target,
	const GLhalfNV * v);
     typedef void (*PFNGLMULTITEXCOORD4HNVPROC) (GLenum target, GLhalfNV s,
	GLhalfNV t, GLhalfNV r, GLhalfNV q);
     typedef void (*PFNGLMULTITEXCOORD4HVNVPROC) (GLenum target,
	const GLhalfNV * v);
     typedef void (*PFNGLFOGCOORDHNVPROC) (GLhalfNV fog);
     typedef void (*PFNGLFOGCOORDHVNVPROC) (const GLhalfNV * fog);
     typedef void (*PFNGLSECONDARYCOLOR3HNVPROC) (GLhalfNV red,
	GLhalfNV green, GLhalfNV blue);
     typedef void (*PFNGLSECONDARYCOLOR3HVNVPROC) (const GLhalfNV * v);
     typedef void (*PFNGLVERTEXWEIGHTHNVPROC) (GLhalfNV weight);
     typedef void (*PFNGLVERTEXWEIGHTHVNVPROC) (const GLhalfNV * weight);
     typedef void (*PFNGLVERTEXATTRIB1HNVPROC) (GLuint index, GLhalfNV x);
     typedef void (*PFNGLVERTEXATTRIB1HVNVPROC) (GLuint index,
	const GLhalfNV * v);
     typedef void (*PFNGLVERTEXATTRIB2HNVPROC) (GLuint index, GLhalfNV x,
	GLhalfNV y);
     typedef void (*PFNGLVERTEXATTRIB2HVNVPROC) (GLuint index,
	const GLhalfNV * v);
     typedef void (*PFNGLVERTEXATTRIB3HNVPROC) (GLuint index, GLhalfNV x,
	GLhalfNV y, GLhalfNV z);
     typedef void (*PFNGLVERTEXATTRIB3HVNVPROC) (GLuint index,
	const GLhalfNV * v);
     typedef void (*PFNGLVERTEXATTRIB4HNVPROC) (GLuint index, GLhalfNV x,
	GLhalfNV y, GLhalfNV z, GLhalfNV w);
     typedef void (*PFNGLVERTEXATTRIB4HVNVPROC) (GLuint index,
	const GLhalfNV * v);
     typedef void (*PFNGLVERTEXATTRIBS1HVNVPROC) (GLuint index, GLsizei n,
	const GLhalfNV * v);
     typedef void (*PFNGLVERTEXATTRIBS2HVNVPROC) (GLuint index, GLsizei n,
	const GLhalfNV * v);
     typedef void (*PFNGLVERTEXATTRIBS3HVNVPROC) (GLuint index, GLsizei n,
	const GLhalfNV * v);
     typedef void (*PFNGLVERTEXATTRIBS4HVNVPROC) (GLuint index, GLsizei n,
	const GLhalfNV * v);
     typedef void (*PFNGLPIXELDATARANGENVPROC) (GLenum target, GLsizei length,
	GLvoid * pointer);
     typedef void (*PFNGLFLUSHPIXELDATARANGENVPROC) (GLenum target);
     typedef void (*PFNGLPRIMITIVERESTARTNVPROC) (void);
     typedef void (*PFNGLPRIMITIVERESTARTINDEXNVPROC) (GLuint index);
     typedef GLvoid *(*PFNGLMAPOBJECTBUFFERATIPROC) (GLuint buffer);
     typedef void (*PFNGLUNMAPOBJECTBUFFERATIPROC) (GLuint buffer);
     typedef void (*PFNGLSTENCILOPSEPARATEATIPROC) (GLenum face, GLenum sfail,
	GLenum dpfail, GLenum dppass);
     typedef void (*PFNGLSTENCILFUNCSEPARATEATIPROC) (GLenum frontfunc,
	GLenum backfunc, GLint ref, GLuint mask);
     typedef void (*PFNGLVERTEXATTRIBARRAYOBJECTATIPROC) (GLuint index,
	GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	GLuint buffer, GLuint offset);
     typedef void (*PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC) (GLuint index,
	GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC) (GLuint index,
	GLenum pname, GLint * params);
     typedef void (*PFNGLDEPTHBOUNDSEXTPROC) (GLclampd zmin, GLclampd zmax);
     typedef void (*PFNGLBLENDEQUATIONSEPARATEEXTPROC) (GLenum modeRGB,
	GLenum modeAlpha);
     typedef GLboolean(*PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
     typedef void (*PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target,
	GLuint renderbuffer);
     typedef void (*PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n,
	const GLuint * renderbuffers);
     typedef void (*PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n,
	GLuint * renderbuffers);
     typedef void (*PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height);
     typedef void (*PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef GLboolean(*PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
     typedef void (*PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target,
	GLuint framebuffer);
     typedef void (*PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n,
	const GLuint * framebuffers);
     typedef void (*PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n,
	GLuint * framebuffers);
     typedef GLenum(*PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
     typedef void (*PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target,
	GLenum attachment, GLenum textarget, GLuint texture, GLint level);
     typedef void (*PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target,
	GLenum attachment, GLenum textarget, GLuint texture, GLint level);
     typedef void (*PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target,
	GLenum attachment, GLenum textarget, GLuint texture, GLint level,
	GLint zoffset);
     typedef void (*PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target,
	GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
     typedef void (*PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum
	target, GLenum attachment, GLenum pname, GLint * params);
     typedef void (*PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);







     typedef void (*PFNGLSTRINGMARKERGREMEDYPROC) (GLsizei len,
	const GLvoid * string);
     typedef void (*PFNGLSTENCILCLEARTAGEXTPROC) (GLsizei stencilTagBits,
	GLuint stencilClearTag);
     typedef void (*PFNGLBLITFRAMEBUFFEREXTPROC) (GLint srcX0, GLint srcY0,
	GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
	GLint dstY1, GLbitfield mask, GLenum filter);







     typedef void (*PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum
	target, GLsizei samples, GLenum internalformat, GLsizei width,
	GLsizei height);
     typedef void (*PFNGLGETQUERYOBJECTI64VEXTPROC) (GLuint id, GLenum pname,
	GLint64EXT * params);
     typedef void (*PFNGLGETQUERYOBJECTUI64VEXTPROC) (GLuint id, GLenum pname,
	GLuint64EXT * params);
     typedef void (*PFNGLPROGRAMENVPARAMETERS4FVEXTPROC) (GLenum target,
	GLuint index, GLsizei count, const GLfloat * params);
     typedef void (*PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC) (GLenum target,
	GLuint index, GLsizei count, const GLfloat * params);
     typedef void (*PFNGLBUFFERPARAMETERIAPPLEPROC) (GLenum target,
	GLenum pname, GLint param);
     typedef void (*PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC) (GLenum target,
	GLintptr offset, GLsizeiptr size);
     typedef void (*PFNGLPROGRAMLOCALPARAMETERI4INVPROC) (GLenum target,
	GLuint index, GLint x, GLint y, GLint z, GLint w);
     typedef void (*PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC) (GLenum target,
	GLuint index, const GLint * params);
     typedef void (*PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC) (GLenum target,
	GLuint index, GLsizei count, const GLint * params);
     typedef void (*PFNGLPROGRAMLOCALPARAMETERI4UINVPROC) (GLenum target,
	GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
     typedef void (*PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC) (GLenum target,
	GLuint index, const GLuint * params);
     typedef void (*PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC) (GLenum target,
	GLuint index, GLsizei count, const GLuint * params);
     typedef void (*PFNGLPROGRAMENVPARAMETERI4INVPROC) (GLenum target,
	GLuint index, GLint x, GLint y, GLint z, GLint w);
     typedef void (*PFNGLPROGRAMENVPARAMETERI4IVNVPROC) (GLenum target,
	GLuint index, const GLint * params);
     typedef void (*PFNGLPROGRAMENVPARAMETERSI4IVNVPROC) (GLenum target,
	GLuint index, GLsizei count, const GLint * params);
     typedef void (*PFNGLPROGRAMENVPARAMETERI4UINVPROC) (GLenum target,
	GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
     typedef void (*PFNGLPROGRAMENVPARAMETERI4UIVNVPROC) (GLenum target,
	GLuint index, const GLuint * params);
     typedef void (*PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC) (GLenum target,
	GLuint index, GLsizei count, const GLuint * params);
     typedef void (*PFNGLGETPROGRAMLOCALPARAMETERIIVNVPROC) (GLenum target,
	GLuint index, GLint * params);
     typedef void (*PFNGLGETPROGRAMLOCALPARAMETERIUIVNVPROC) (GLenum target,
	GLuint index, GLuint * params);
     typedef void (*PFNGLGETPROGRAMENVPARAMETERIIVNVPROC) (GLenum target,
	GLuint index, GLint * params);
     typedef void (*PFNGLGETPROGRAMENVPARAMETERIUIVNVPROC) (GLenum target,
	GLuint index, GLuint * params);
     typedef void (*PFNGLPROGRAMVERTEXLIMITNVPROC) (GLenum target,
	GLint limit);
     typedef void (*PFNGLFRAMEBUFFERTEXTUREEXTPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level);
     typedef void (*PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level, GLint layer);
     typedef void (*PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC) (GLenum target,
	GLenum attachment, GLuint texture, GLint level, GLenum face);







     typedef void (*PFNGLPROGRAMPARAMETERIEXTPROC) (GLuint program,
	GLenum pname, GLint value);
     typedef void (*PFNGLVERTEXATTRIBI1IEXTPROC) (GLuint index, GLint x);
     typedef void (*PFNGLVERTEXATTRIBI2IEXTPROC) (GLuint index, GLint x,
	GLint y);
     typedef void (*PFNGLVERTEXATTRIBI3IEXTPROC) (GLuint index, GLint x,
	GLint y, GLint z);
     typedef void (*PFNGLVERTEXATTRIBI4IEXTPROC) (GLuint index, GLint x,
	GLint y, GLint z, GLint w);
     typedef void (*PFNGLVERTEXATTRIBI1UIEXTPROC) (GLuint index, GLuint x);
     typedef void (*PFNGLVERTEXATTRIBI2UIEXTPROC) (GLuint index, GLuint x,
	GLuint y);
     typedef void (*PFNGLVERTEXATTRIBI3UIEXTPROC) (GLuint index, GLuint x,
	GLuint y, GLuint z);
     typedef void (*PFNGLVERTEXATTRIBI4UIEXTPROC) (GLuint index, GLuint x,
	GLuint y, GLuint z, GLuint w);
     typedef void (*PFNGLVERTEXATTRIBI1IVEXTPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI2IVEXTPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI3IVEXTPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI4IVEXTPROC) (GLuint index,
	const GLint * v);
     typedef void (*PFNGLVERTEXATTRIBI1UIVEXTPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI2UIVEXTPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI3UIVEXTPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI4UIVEXTPROC) (GLuint index,
	const GLuint * v);
     typedef void (*PFNGLVERTEXATTRIBI4BVEXTPROC) (GLuint index,
	const GLbyte * v);
     typedef void (*PFNGLVERTEXATTRIBI4SVEXTPROC) (GLuint index,
	const GLshort * v);
     typedef void (*PFNGLVERTEXATTRIBI4UBVEXTPROC) (GLuint index,
	const GLubyte * v);
     typedef void (*PFNGLVERTEXATTRIBI4USVEXTPROC) (GLuint index,
	const GLushort * v);
     typedef void (*PFNGLVERTEXATTRIBIPOINTEREXTPROC) (GLuint index,
	GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLGETVERTEXATTRIBIIVEXTPROC) (GLuint index,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETVERTEXATTRIBIUIVEXTPROC) (GLuint index,
	GLenum pname, GLuint * params);
     typedef void (*PFNGLGETUNIFORMUIVEXTPROC) (GLuint program,
	GLint location, GLuint * params);
     typedef void (*PFNGLBINDFRAGDATALOCATIONEXTPROC) (GLuint program,
	GLuint color, const GLchar * name);
     typedef GLint(*PFNGLGETFRAGDATALOCATIONEXTPROC) (GLuint program,
	const GLchar * name);
     typedef void (*PFNGLUNIFORM1UIEXTPROC) (GLint location, GLuint v0);
     typedef void (*PFNGLUNIFORM2UIEXTPROC) (GLint location, GLuint v0,
	GLuint v1);
     typedef void (*PFNGLUNIFORM3UIEXTPROC) (GLint location, GLuint v0,
	GLuint v1, GLuint v2);
     typedef void (*PFNGLUNIFORM4UIEXTPROC) (GLint location, GLuint v0,
	GLuint v1, GLuint v2, GLuint v3);
     typedef void (*PFNGLUNIFORM1UIVEXTPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLUNIFORM2UIVEXTPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLUNIFORM3UIVEXTPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLUNIFORM4UIVEXTPROC) (GLint location, GLsizei count,
	const GLuint * value);
     typedef void (*PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode,
	GLint start, GLsizei count, GLsizei primcount);
     typedef void (*PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode,
	GLsizei count, GLenum type, const GLvoid * indices,
	GLsizei primcount);
     typedef void (*PFNGLTEXBUFFEREXTPROC) (GLenum target,
	GLenum internalformat, GLuint buffer);
     typedef void (*PFNGLDEPTHRANGEDNVPROC) (GLdouble zNear, GLdouble zFar);
     typedef void (*PFNGLCLEARDEPTHDNVPROC) (GLdouble depth);
     typedef void (*PFNGLDEPTHBOUNDSDNVPROC) (GLdouble zmin, GLdouble zmax);
     typedef void (*PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC) (GLenum
	target, GLsizei coverageSamples, GLsizei colorSamples,
	GLenum internalformat, GLsizei width, GLsizei height);
     typedef void (*PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC) (GLenum target,
	GLuint buffer, GLuint index, GLsizei count, const GLfloat * params);
     typedef void (*PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC) (GLenum target,
	GLuint buffer, GLuint index, GLsizei count, const GLint * params);
     typedef void (*PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC) (GLenum target,
	GLuint buffer, GLuint index, GLsizei count, const GLuint * params);
     typedef void (*PFNGLCOLORMASKINDEXEDEXTPROC) (GLuint index, GLboolean r,
	GLboolean g, GLboolean b, GLboolean a);
     typedef void (*PFNGLGETBOOLEANINDEXEDVEXTPROC) (GLenum target,
	GLuint index, GLboolean * data);
     typedef void (*PFNGLGETINTEGERINDEXEDVEXTPROC) (GLenum target,
	GLuint index, GLint * data);
     typedef void (*PFNGLENABLEINDEXEDEXTPROC) (GLenum target, GLuint index);
     typedef void (*PFNGLDISABLEINDEXEDEXTPROC) (GLenum target, GLuint index);
     typedef GLboolean(*PFNGLISENABLEDINDEXEDEXTPROC) (GLenum target,
	GLuint index);
     typedef void (*PFNGLBEGINTRANSFORMFEEDBACKNVPROC) (GLenum primitiveMode);
     typedef void (*PFNGLENDTRANSFORMFEEDBACKNVPROC) (void);
     typedef void (*PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC) (GLuint count,
	const GLint * attribs, GLenum bufferMode);
     typedef void (*PFNGLBINDBUFFERRANGENVPROC) (GLenum target, GLuint index,
	GLuint buffer, GLintptr offset, GLsizeiptr size);
     typedef void (*PFNGLBINDBUFFEROFFSETNVPROC) (GLenum target, GLuint index,
	GLuint buffer, GLintptr offset);
     typedef void (*PFNGLBINDBUFFERBASENVPROC) (GLenum target, GLuint index,
	GLuint buffer);
     typedef void (*PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC) (GLuint program,
	GLsizei count, const GLint * locations, GLenum bufferMode);
     typedef void (*PFNGLACTIVEVARYINGNVPROC) (GLuint program,
	const GLchar * name);
     typedef GLint(*PFNGLGETVARYINGLOCATIONNVPROC) (GLuint program,
	const GLchar * name);
     typedef void (*PFNGLGETACTIVEVARYINGNVPROC) (GLuint program,
	GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size,
	GLenum * type, GLchar * name);
     typedef void (*PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC) (GLuint program,
	GLuint index, GLint * location);
     typedef void (*PFNGLTRANSFORMFEEDBACKSTREAMATTRIBSNVPROC) (GLsizei count,
	const GLint * attribs, GLsizei nbuffers, const GLint * bufstreams,
	GLenum bufferMode);
     typedef void (*PFNGLUNIFORMBUFFEREXTPROC) (GLuint program,
	GLint location, GLuint buffer);
     typedef GLint(*PFNGLGETUNIFORMBUFFERSIZEEXTPROC) (GLuint program,
	GLint location);
     typedef GLintptr(*PFNGLGETUNIFORMOFFSETEXTPROC) (GLuint program,
	GLint location);
     typedef void (*PFNGLTEXPARAMETERIIVEXTPROC) (GLenum target, GLenum pname,
	const GLint * params);
     typedef void (*PFNGLTEXPARAMETERIUIVEXTPROC) (GLenum target,
	GLenum pname, const GLuint * params);
     typedef void (*PFNGLGETTEXPARAMETERIIVEXTPROC) (GLenum target,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETTEXPARAMETERIUIVEXTPROC) (GLenum target,
	GLenum pname, GLuint * params);
     typedef void (*PFNGLCLEARCOLORIIEXTPROC) (GLint red, GLint green,
	GLint blue, GLint alpha);
     typedef void (*PFNGLCLEARCOLORIUIEXTPROC) (GLuint red, GLuint green,
	GLuint blue, GLuint alpha);







     typedef void (*PFNGLFRAMETERMINATORGREMEDYPROC) (void);
     typedef void (*PFNGLBEGINCONDITIONALRENDERNVPROC) (GLuint id,
	GLenum mode);
     typedef void (*PFNGLENDCONDITIONALRENDERNVPROC) (void);
     typedef void (*PFNGLPRESENTFRAMEKEYEDNVPROC) (GLuint video_slot,
	GLuint64EXT minPresentTime, GLuint beginPresentTimeId,
	GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0,
	GLuint key0, GLenum target1, GLuint fill1, GLuint key1);
     typedef void (*PFNGLPRESENTFRAMEDUALFILLNVPROC) (GLuint video_slot,
	GLuint64EXT minPresentTime, GLuint beginPresentTimeId,
	GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0,
	GLenum target1, GLuint fill1, GLenum target2, GLuint fill2,
	GLenum target3, GLuint fill3);
     typedef void (*PFNGLGETVIDEOIVNVPROC) (GLuint video_slot, GLenum pname,
	GLint * params);
     typedef void (*PFNGLGETVIDEOUIVNVPROC) (GLuint video_slot, GLenum pname,
	GLuint * params);
     typedef void (*PFNGLGETVIDEOI64VNVPROC) (GLuint video_slot, GLenum pname,
	GLint64EXT * params);
     typedef void (*PFNGLGETVIDEOUI64VNVPROC) (GLuint video_slot,
	GLenum pname, GLuint64EXT * params);
     typedef void (*PFNGLBEGINTRANSFORMFEEDBACKEXTPROC) (GLenum
	primitiveMode);
     typedef void (*PFNGLENDTRANSFORMFEEDBACKEXTPROC) (void);
     typedef void (*PFNGLBINDBUFFERRANGEEXTPROC) (GLenum target, GLuint index,
	GLuint buffer, GLintptr offset, GLsizeiptr size);
     typedef void (*PFNGLBINDBUFFEROFFSETEXTPROC) (GLenum target,
	GLuint index, GLuint buffer, GLintptr offset);
     typedef void (*PFNGLBINDBUFFERBASEEXTPROC) (GLenum target, GLuint index,
	GLuint buffer);
     typedef void (*PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC) (GLuint program,
	GLsizei count, const GLchar * *varyings, GLenum bufferMode);
     typedef void (*PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC) (GLuint program,
	GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size,
	GLenum * type, GLchar * name);
     typedef void (*PFNGLCLIENTATTRIBDEFAULTEXTPROC) (GLbitfield mask);
     typedef void (*PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC) (GLbitfield mask);
     typedef void (*PFNGLMATRIXLOADFEXTPROC) (GLenum mode, const GLfloat * m);
     typedef void (*PFNGLMATRIXLOADDEXTPROC) (GLenum mode,
	const GLdouble * m);
     typedef void (*PFNGLMATRIXMULTFEXTPROC) (GLenum mode, const GLfloat * m);
     typedef void (*PFNGLMATRIXMULTDEXTPROC) (GLenum mode,
	const GLdouble * m);
     typedef void (*PFNGLMATRIXLOADIDENTITYEXTPROC) (GLenum mode);
     typedef void (*PFNGLMATRIXROTATEFEXTPROC) (GLenum mode, GLfloat angle,
	GLfloat x, GLfloat y, GLfloat z);
     typedef void (*PFNGLMATRIXROTATEDEXTPROC) (GLenum mode, GLdouble angle,
	GLdouble x, GLdouble y, GLdouble z);
     typedef void (*PFNGLMATRIXSCALEFEXTPROC) (GLenum mode, GLfloat x,
	GLfloat y, GLfloat z);
     typedef void (*PFNGLMATRIXSCALEDEXTPROC) (GLenum mode, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLMATRIXTRANSLATEFEXTPROC) (GLenum mode, GLfloat x,
	GLfloat y, GLfloat z);
     typedef void (*PFNGLMATRIXTRANSLATEDEXTPROC) (GLenum mode, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLMATRIXFRUSTUMEXTPROC) (GLenum mode, GLdouble left,
	GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear,
	GLdouble zFar);
     typedef void (*PFNGLMATRIXORTHOEXTPROC) (GLenum mode, GLdouble left,
	GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear,
	GLdouble zFar);
     typedef void (*PFNGLMATRIXPOPEXTPROC) (GLenum mode);
     typedef void (*PFNGLMATRIXPUSHEXTPROC) (GLenum mode);
     typedef void (*PFNGLMATRIXLOADTRANSPOSEFEXTPROC) (GLenum mode,
	const GLfloat * m);
     typedef void (*PFNGLMATRIXLOADTRANSPOSEDEXTPROC) (GLenum mode,
	const GLdouble * m);
     typedef void (*PFNGLMATRIXMULTTRANSPOSEFEXTPROC) (GLenum mode,
	const GLfloat * m);
     typedef void (*PFNGLMATRIXMULTTRANSPOSEDEXTPROC) (GLenum mode,
	const GLdouble * m);
     typedef void (*PFNGLTEXTUREPARAMETERFEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, GLfloat param);
     typedef void (*PFNGLTEXTUREPARAMETERFVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, const GLfloat * params);
     typedef void (*PFNGLTEXTUREPARAMETERIEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, GLint param);
     typedef void (*PFNGLTEXTUREPARAMETERIVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, const GLint * params);
     typedef void (*PFNGLTEXTUREIMAGE1DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLint border, GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLTEXTUREIMAGE2DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLint border, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLTEXTURESUBIMAGE1DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLsizei width,
	GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLTEXTURESUBIMAGE2DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLCOPYTEXTUREIMAGE1DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
	GLsizei width, GLint border);
     typedef void (*PFNGLCOPYTEXTUREIMAGE2DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
	GLsizei width, GLsizei height, GLint border);
     typedef void (*PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLint x, GLint y,
	GLsizei width);
     typedef void (*PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x,
	GLint y, GLsizei width, GLsizei height);
     typedef void (*PFNGLGETTEXTUREIMAGEEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum format, GLenum type,
	GLvoid * pixels);
     typedef void (*PFNGLGETTEXTUREPARAMETERFVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETTEXTUREPARAMETERIVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, GLint * params);
     typedef void (*PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum pname, GLint * params);
     typedef void (*PFNGLTEXTUREIMAGE3DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLsizei depth, GLint border, GLenum format,
	GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLTEXTURESUBIMAGE3DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
	GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
     typedef void (*PFNGLMULTITEXPARAMETERFEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLfloat param);
     typedef void (*PFNGLMULTITEXPARAMETERFVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, const GLfloat * params);
     typedef void (*PFNGLMULTITEXPARAMETERIEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLint param);
     typedef void (*PFNGLMULTITEXPARAMETERIVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, const GLint * params);
     typedef void (*PFNGLMULTITEXIMAGE1DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLint border, GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLMULTITEXIMAGE2DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLint border, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLMULTITEXSUBIMAGE1DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLsizei width,
	GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLMULTITEXSUBIMAGE2DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type,
	const GLvoid * pixels);
     typedef void (*PFNGLCOPYMULTITEXIMAGE1DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
	GLsizei width, GLint border);
     typedef void (*PFNGLCOPYMULTITEXIMAGE2DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
	GLsizei width, GLsizei height, GLint border);
     typedef void (*PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLint x, GLint y,
	GLsizei width);
     typedef void (*PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x,
	GLint y, GLsizei width, GLsizei height);
     typedef void (*PFNGLGETMULTITEXIMAGEEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum format, GLenum type,
	GLvoid * pixels);
     typedef void (*PFNGLGETMULTITEXPARAMETERFVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETMULTITEXPARAMETERIVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLint * params);
     typedef void (*PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum pname, GLint * params);
     typedef void (*PFNGLMULTITEXIMAGE3DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLsizei depth, GLint border, GLenum format,
	GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLMULTITEXSUBIMAGE3DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
	GLenum format, GLenum type, const GLvoid * pixels);
     typedef void (*PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
     typedef void (*PFNGLBINDMULTITEXTUREEXTPROC) (GLenum texunit,
	GLenum target, GLuint texture);
     typedef void (*PFNGLENABLECLIENTSTATEINDEXEDEXTPROC) (GLenum array,
	GLuint index);
     typedef void (*PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC) (GLenum array,
	GLuint index);
     typedef void (*PFNGLMULTITEXCOORDPOINTEREXTPROC) (GLenum texunit,
	GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLMULTITEXENVFEXTPROC) (GLenum texunit, GLenum target,
	GLenum pname, GLfloat param);
     typedef void (*PFNGLMULTITEXENVFVEXTPROC) (GLenum texunit, GLenum target,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLMULTITEXENVIEXTPROC) (GLenum texunit, GLenum target,
	GLenum pname, GLint param);
     typedef void (*PFNGLMULTITEXENVIVEXTPROC) (GLenum texunit, GLenum target,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLMULTITEXGENDEXTPROC) (GLenum texunit, GLenum coord,
	GLenum pname, GLdouble param);
     typedef void (*PFNGLMULTITEXGENDVEXTPROC) (GLenum texunit, GLenum coord,
	GLenum pname, const GLdouble * params);
     typedef void (*PFNGLMULTITEXGENFEXTPROC) (GLenum texunit, GLenum coord,
	GLenum pname, GLfloat param);
     typedef void (*PFNGLMULTITEXGENFVEXTPROC) (GLenum texunit, GLenum coord,
	GLenum pname, const GLfloat * params);
     typedef void (*PFNGLMULTITEXGENIEXTPROC) (GLenum texunit, GLenum coord,
	GLenum pname, GLint param);
     typedef void (*PFNGLMULTITEXGENIVEXTPROC) (GLenum texunit, GLenum coord,
	GLenum pname, const GLint * params);
     typedef void (*PFNGLGETMULTITEXENVFVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETMULTITEXENVIVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLint * params);
     typedef void (*PFNGLGETMULTITEXGENDVEXTPROC) (GLenum texunit,
	GLenum coord, GLenum pname, GLdouble * params);
     typedef void (*PFNGLGETMULTITEXGENFVEXTPROC) (GLenum texunit,
	GLenum coord, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETMULTITEXGENIVEXTPROC) (GLenum texunit,
	GLenum coord, GLenum pname, GLint * params);
     typedef void (*PFNGLGETFLOATINDEXEDVEXTPROC) (GLenum target,
	GLuint index, GLfloat * data);
     typedef void (*PFNGLGETDOUBLEINDEXEDVEXTPROC) (GLenum target,
	GLuint index, GLdouble * data);
     typedef void (*PFNGLGETPOINTERINDEXEDVEXTPROC) (GLenum target,
	GLuint index, GLvoid * *data);
     typedef void (*PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLsizei depth, GLint border, GLsizei imageSize,
	const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLint border, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLint border, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
	GLenum format, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLsizei imageSize,
	const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC) (GLuint texture,
	GLenum target, GLint level, GLint xoffset, GLsizei width,
	GLenum format, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC) (GLuint texture,
	GLenum target, GLint lod, GLvoid * img);
     typedef void (*PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLsizei depth, GLint border, GLsizei imageSize,
	const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLsizei height, GLint border, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLenum internalformat, GLsizei width,
	GLint border, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
	GLenum format, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLsizei imageSize,
	const GLvoid * bits);
     typedef void (*PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC) (GLenum texunit,
	GLenum target, GLint level, GLint xoffset, GLsizei width,
	GLenum format, GLsizei imageSize, const GLvoid * bits);
     typedef void (*PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC) (GLenum texunit,
	GLenum target, GLint lod, GLvoid * img);
     typedef void (*PFNGLNAMEDPROGRAMSTRINGEXTPROC) (GLuint program,
	GLenum target, GLenum format, GLsizei len, const GLvoid * string);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC) (GLuint program,
	GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z,
	GLdouble w);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC) (GLuint
	program, GLenum target, GLuint index, const GLdouble * params);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC) (GLuint program,
	GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z,
	GLfloat w);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC) (GLuint
	program, GLenum target, GLuint index, const GLfloat * params);
     typedef void (*PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLdouble * params);
     typedef void (*PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLfloat * params);
     typedef void (*PFNGLGETNAMEDPROGRAMIVEXTPROC) (GLuint program,
	GLenum target, GLenum pname, GLint * params);
     typedef void (*PFNGLGETNAMEDPROGRAMSTRINGEXTPROC) (GLuint program,
	GLenum target, GLenum pname, GLvoid * string);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLsizei count,
	const GLfloat * params);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLint x, GLint y, GLint z,
	GLint w);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC) (GLuint
	program, GLenum target, GLuint index, const GLint * params);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLsizei count,
	const GLint * params);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z,
	GLuint w);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC) (GLuint
	program, GLenum target, GLuint index, const GLuint * params);
     typedef void (*PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLsizei count,
	const GLuint * params);
     typedef void (*PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLint * params);
     typedef void (*PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC) (GLuint
	program, GLenum target, GLuint index, GLuint * params);
     typedef void (*PFNGLTEXTUREPARAMETERIIVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, const GLint * params);
     typedef void (*PFNGLTEXTUREPARAMETERIUIVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, const GLuint * params);
     typedef void (*PFNGLGETTEXTUREPARAMETERIIVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, GLint * params);
     typedef void (*PFNGLGETTEXTUREPARAMETERIUIVEXTPROC) (GLuint texture,
	GLenum target, GLenum pname, GLuint * params);
     typedef void (*PFNGLMULTITEXPARAMETERIIVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, const GLint * params);
     typedef void (*PFNGLMULTITEXPARAMETERIUIVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, const GLuint * params);
     typedef void (*PFNGLGETMULTITEXPARAMETERIIVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLint * params);
     typedef void (*PFNGLGETMULTITEXPARAMETERIUIVEXTPROC) (GLenum texunit,
	GLenum target, GLenum pname, GLuint * params);
     typedef void (*PFNGLPROGRAMUNIFORM1FEXTPROC) (GLuint program,
	GLint location, GLfloat v0);
     typedef void (*PFNGLPROGRAMUNIFORM2FEXTPROC) (GLuint program,
	GLint location, GLfloat v0, GLfloat v1);
     typedef void (*PFNGLPROGRAMUNIFORM3FEXTPROC) (GLuint program,
	GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
     typedef void (*PFNGLPROGRAMUNIFORM4FEXTPROC) (GLuint program,
	GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
     typedef void (*PFNGLPROGRAMUNIFORM1IEXTPROC) (GLuint program,
	GLint location, GLint v0);
     typedef void (*PFNGLPROGRAMUNIFORM2IEXTPROC) (GLuint program,
	GLint location, GLint v0, GLint v1);
     typedef void (*PFNGLPROGRAMUNIFORM3IEXTPROC) (GLuint program,
	GLint location, GLint v0, GLint v1, GLint v2);
     typedef void (*PFNGLPROGRAMUNIFORM4IEXTPROC) (GLuint program,
	GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
     typedef void (*PFNGLPROGRAMUNIFORM1FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM2FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM3FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM4FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM1IVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORM2IVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORM3IVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORM4IVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLint * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLfloat * value);
     typedef void (*PFNGLPROGRAMUNIFORM1UIEXTPROC) (GLuint program,
	GLint location, GLuint v0);
     typedef void (*PFNGLPROGRAMUNIFORM2UIEXTPROC) (GLuint program,
	GLint location, GLuint v0, GLuint v1);
     typedef void (*PFNGLPROGRAMUNIFORM3UIEXTPROC) (GLuint program,
	GLint location, GLuint v0, GLuint v1, GLuint v2);
     typedef void (*PFNGLPROGRAMUNIFORM4UIEXTPROC) (GLuint program,
	GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
     typedef void (*PFNGLPROGRAMUNIFORM1UIVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLPROGRAMUNIFORM2UIVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLPROGRAMUNIFORM3UIVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLPROGRAMUNIFORM4UIVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint * value);
     typedef void (*PFNGLNAMEDBUFFERDATAEXTPROC) (GLuint buffer,
	GLsizeiptr size, const GLvoid * data, GLenum usage);
     typedef void (*PFNGLNAMEDBUFFERSUBDATAEXTPROC) (GLuint buffer,
	GLintptr offset, GLsizeiptr size, const GLvoid * data);
     typedef GLvoid *(*PFNGLMAPNAMEDBUFFEREXTPROC) (GLuint buffer,
	GLenum access);
     typedef GLboolean(*PFNGLUNMAPNAMEDBUFFEREXTPROC) (GLuint buffer);
     typedef GLvoid *(*PFNGLMAPNAMEDBUFFERRANGEEXTPROC) (GLuint buffer,
	GLintptr offset, GLsizeiptr length, GLbitfield access);
     typedef void (*PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC) (GLuint buffer,
	GLintptr offset, GLsizeiptr length);
     typedef void (*PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC) (GLuint readBuffer,
	GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset,
	GLsizeiptr size);
     typedef void (*PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC) (GLuint buffer,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETNAMEDBUFFERPOINTERVEXTPROC) (GLuint buffer,
	GLenum pname, GLvoid * *params);
     typedef void (*PFNGLGETNAMEDBUFFERSUBDATAEXTPROC) (GLuint buffer,
	GLintptr offset, GLsizeiptr size, GLvoid * data);
     typedef void (*PFNGLTEXTUREBUFFEREXTPROC) (GLuint texture, GLenum target,
	GLenum internalformat, GLuint buffer);
     typedef void (*PFNGLMULTITEXBUFFEREXTPROC) (GLenum texunit,
	GLenum target, GLenum internalformat, GLuint buffer);
     typedef void (*PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC) (GLuint
	renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
     typedef void (*PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC) (GLuint
	renderbuffer, GLenum pname, GLint * params);
     typedef GLenum(*PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC) (GLuint
	framebuffer, GLenum target);
     typedef void (*PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC) (GLuint
	framebuffer, GLenum attachment, GLenum textarget, GLuint texture,
	GLint level);
     typedef void (*PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC) (GLuint
	framebuffer, GLenum attachment, GLenum textarget, GLuint texture,
	GLint level);
     typedef void (*PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC) (GLuint
	framebuffer, GLenum attachment, GLenum textarget, GLuint texture,
	GLint level, GLint zoffset);
     typedef void (*PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC) (GLuint
	framebuffer, GLenum attachment, GLenum renderbuffertarget,
	GLuint renderbuffer);
     typedef void (*PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)
	(GLuint framebuffer, GLenum attachment, GLenum pname, GLint * params);
     typedef void (*PFNGLGENERATETEXTUREMIPMAPEXTPROC) (GLuint texture,
	GLenum target);
     typedef void (*PFNGLGENERATEMULTITEXMIPMAPEXTPROC) (GLenum texunit,
	GLenum target);
     typedef void (*PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC) (GLuint framebuffer,
	GLenum mode);
     typedef void (*PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC) (GLuint framebuffer,
	GLsizei n, const GLenum * bufs);
     typedef void (*PFNGLFRAMEBUFFERREADBUFFEREXTPROC) (GLuint framebuffer,
	GLenum mode);
     typedef void (*PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC) (GLuint
	framebuffer, GLenum pname, GLint * params);
     typedef void (*PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLuint
	renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width,
	GLsizei height);
     typedef void (*PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC)
	(GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples,
	GLenum internalformat, GLsizei width, GLsizei height);
     typedef void (*PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC) (GLuint framebuffer,
	GLenum attachment, GLuint texture, GLint level);
     typedef void (*PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC) (GLuint
	framebuffer, GLenum attachment, GLuint texture, GLint level,
	GLint layer);
     typedef void (*PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC) (GLuint
	framebuffer, GLenum attachment, GLuint texture, GLint level,
	GLenum face);
     typedef void (*PFNGLTEXTURERENDERBUFFEREXTPROC) (GLuint texture,
	GLenum target, GLuint renderbuffer);
     typedef void (*PFNGLMULTITEXRENDERBUFFEREXTPROC) (GLenum texunit,
	GLenum target, GLuint renderbuffer);
     typedef void (*PFNGLPROGRAMUNIFORM1DEXTPROC) (GLuint program,
	GLint location, GLdouble x);
     typedef void (*PFNGLPROGRAMUNIFORM2DEXTPROC) (GLuint program,
	GLint location, GLdouble x, GLdouble y);
     typedef void (*PFNGLPROGRAMUNIFORM3DEXTPROC) (GLuint program,
	GLint location, GLdouble x, GLdouble y, GLdouble z);
     typedef void (*PFNGLPROGRAMUNIFORM4DEXTPROC) (GLuint program,
	GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLPROGRAMUNIFORM1DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORM2DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORM3DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORM4DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC) (GLuint program,
	GLint location, GLsizei count, GLboolean transpose,
	const GLdouble * value);
     typedef void (*PFNGLGETMULTISAMPLEFVNVPROC) (GLenum pname, GLuint index,
	GLfloat * val);
     typedef void (*PFNGLSAMPLEMASKINDEXEDNVPROC) (GLuint index,
	GLbitfield mask);
     typedef void (*PFNGLTEXRENDERBUFFERNVPROC) (GLenum target,
	GLuint renderbuffer);
     typedef void (*PFNGLBINDTRANSFORMFEEDBACKNVPROC) (GLenum target,
	GLuint id);
     typedef void (*PFNGLDELETETRANSFORMFEEDBACKSNVPROC) (GLsizei n,
	const GLuint * ids);
     typedef void (*PFNGLGENTRANSFORMFEEDBACKSNVPROC) (GLsizei n,
	GLuint * ids);
     typedef GLboolean(*PFNGLISTRANSFORMFEEDBACKNVPROC) (GLuint id);
     typedef void (*PFNGLPAUSETRANSFORMFEEDBACKNVPROC) (void);
     typedef void (*PFNGLRESUMETRANSFORMFEEDBACKNVPROC) (void);
     typedef void (*PFNGLDRAWTRANSFORMFEEDBACKNVPROC) (GLenum mode,
	GLuint id);
     typedef void (*PFNGLGETPERFMONITORGROUPSAMDPROC) (GLint * numGroups,
	GLsizei groupsSize, GLuint * groups);
     typedef void (*PFNGLGETPERFMONITORCOUNTERSAMDPROC) (GLuint group,
	GLint * numCounters, GLint * maxActiveCounters, GLsizei counterSize,
	GLuint * counters);
     typedef void (*PFNGLGETPERFMONITORGROUPSTRINGAMDPROC) (GLuint group,
	GLsizei bufSize, GLsizei * length, GLchar * groupString);
     typedef void (*PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC) (GLuint group,
	GLuint counter, GLsizei bufSize, GLsizei * length,
	GLchar * counterString);
     typedef void (*PFNGLGETPERFMONITORCOUNTERINFOAMDPROC) (GLuint group,
	GLuint counter, GLenum pname, GLvoid * data);
     typedef void (*PFNGLGENPERFMONITORSAMDPROC) (GLsizei n,
	GLuint * monitors);
     typedef void (*PFNGLDELETEPERFMONITORSAMDPROC) (GLsizei n,
	GLuint * monitors);
     typedef void (*PFNGLSELECTPERFMONITORCOUNTERSAMDPROC) (GLuint monitor,
	GLboolean enable, GLuint group, GLint numCounters,
	GLuint * counterList);
     typedef void (*PFNGLBEGINPERFMONITORAMDPROC) (GLuint monitor);
     typedef void (*PFNGLENDPERFMONITORAMDPROC) (GLuint monitor);
     typedef void (*PFNGLGETPERFMONITORCOUNTERDATAAMDPROC) (GLuint monitor,
	GLenum pname, GLsizei dataSize, GLuint * data, GLint * bytesWritten);
     typedef void (*PFNGLTESSELLATIONFACTORAMDPROC) (GLfloat factor);
     typedef void (*PFNGLTESSELLATIONMODEAMDPROC) (GLenum mode);







     typedef void (*PFNGLPROVOKINGVERTEXEXTPROC) (GLenum mode);
     typedef void (*PFNGLBLENDFUNCINDEXEDAMDPROC) (GLuint buf, GLenum src,
	GLenum dst);
     typedef void (*PFNGLBLENDFUNCSEPARATEINDEXEDAMDPROC) (GLuint buf,
	GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
     typedef void (*PFNGLBLENDEQUATIONINDEXEDAMDPROC) (GLuint buf,
	GLenum mode);
     typedef void (*PFNGLBLENDEQUATIONSEPARATEINDEXEDAMDPROC) (GLuint buf,
	GLenum modeRGB, GLenum modeAlpha);
     typedef void (*PFNGLTEXTURERANGEAPPLEPROC) (GLenum target,
	GLsizei length, const GLvoid * pointer);
     typedef void (*PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC) (GLenum target,
	GLenum pname, GLvoid * *params);
     typedef void (*PFNGLENABLEVERTEXATTRIBAPPLEPROC) (GLuint index,
	GLenum pname);
     typedef void (*PFNGLDISABLEVERTEXATTRIBAPPLEPROC) (GLuint index,
	GLenum pname);
     typedef GLboolean(*PFNGLISVERTEXATTRIBENABLEDAPPLEPROC) (GLuint index,
	GLenum pname);
     typedef void (*PFNGLMAPVERTEXATTRIB1DAPPLEPROC) (GLuint index,
	GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order,
	const GLdouble * points);
     typedef void (*PFNGLMAPVERTEXATTRIB1FAPPLEPROC) (GLuint index,
	GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order,
	const GLfloat * points);
     typedef void (*PFNGLMAPVERTEXATTRIB2DAPPLEPROC) (GLuint index,
	GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
	GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
	const GLdouble * points);
     typedef void (*PFNGLMAPVERTEXATTRIB2FAPPLEPROC) (GLuint index,
	GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
	GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
	const GLfloat * points);
     typedef GLenum(*PFNGLOBJECTPURGEABLEAPPLEPROC) (GLenum objectType,
	GLuint name, GLenum option);
     typedef GLenum(*PFNGLOBJECTUNPURGEABLEAPPLEPROC) (GLenum objectType,
	GLuint name, GLenum option);
     typedef void (*PFNGLGETOBJECTPARAMETERIVAPPLEPROC) (GLenum objectType,
	GLuint name, GLenum pname, GLint * params);
     typedef void (*PFNGLBEGINVIDEOCAPTURENVPROC) (GLuint video_capture_slot);
     typedef void (*PFNGLBINDVIDEOCAPTURESTREAMBUFFERNVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum frame_region,
	GLintptrARB offset);
     typedef void (*PFNGLBINDVIDEOCAPTURESTREAMTEXTURENVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum frame_region, GLenum target,
	GLuint texture);
     typedef void (*PFNGLENDVIDEOCAPTURENVPROC) (GLuint video_capture_slot);
     typedef void (*PFNGLGETVIDEOCAPTUREIVNVPROC) (GLuint video_capture_slot,
	GLenum pname, GLint * params);
     typedef void (*PFNGLGETVIDEOCAPTURESTREAMIVNVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum pname, GLint * params);
     typedef void (*PFNGLGETVIDEOCAPTURESTREAMFVNVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum pname, GLfloat * params);
     typedef void (*PFNGLGETVIDEOCAPTURESTREAMDVNVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum pname, GLdouble * params);
     typedef GLenum(*PFNGLVIDEOCAPTURENVPROC) (GLuint video_capture_slot,
	GLuint * sequence_num, GLuint64EXT * capture_time);
     typedef void (*PFNGLVIDEOCAPTURESTREAMPARAMETERIVNVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum pname,
	const GLint * params);
     typedef void (*PFNGLVIDEOCAPTURESTREAMPARAMETERFVNVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum pname,
	const GLfloat * params);
     typedef void (*PFNGLVIDEOCAPTURESTREAMPARAMETERDVNVPROC) (GLuint
	video_capture_slot, GLuint stream, GLenum pname,
	const GLdouble * params);







     typedef void (*PFNGLCOPYIMAGESUBDATANVPROC) (GLuint srcName,
	GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
	GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX,
	GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth);
     typedef void (*PFNGLUSESHADERPROGRAMEXTPROC) (GLenum type,
	GLuint program);
     typedef void (*PFNGLACTIVEPROGRAMEXTPROC) (GLuint program);
     typedef GLuint(*PFNGLCREATESHADERPROGRAMEXTPROC) (GLenum type,
	const GLchar * string);
     typedef void (*PFNGLMAKEBUFFERRESIDENTNVPROC) (GLenum target,
	GLenum access);
     typedef void (*PFNGLMAKEBUFFERNONRESIDENTNVPROC) (GLenum target);
     typedef GLboolean(*PFNGLISBUFFERRESIDENTNVPROC) (GLenum target);
     typedef void (*PFNGLMAKENAMEDBUFFERRESIDENTNVPROC) (GLuint buffer,
	GLenum access);
     typedef void (*PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC) (GLuint buffer);
     typedef GLboolean(*PFNGLISNAMEDBUFFERRESIDENTNVPROC) (GLuint buffer);
     typedef void (*PFNGLGETBUFFERPARAMETERUI64VNVPROC) (GLenum target,
	GLenum pname, GLuint64EXT * params);
     typedef void (*PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC) (GLuint buffer,
	GLenum pname, GLuint64EXT * params);
     typedef void (*PFNGLGETINTEGERUI64VNVPROC) (GLenum value,
	GLuint64EXT * result);
     typedef void (*PFNGLUNIFORMUI64NVPROC) (GLint location,
	GLuint64EXT value);
     typedef void (*PFNGLUNIFORMUI64VNVPROC) (GLint location, GLsizei count,
	const GLuint64EXT * value);
     typedef void (*PFNGLGETUNIFORMUI64VNVPROC) (GLuint program,
	GLint location, GLuint64EXT * params);
     typedef void (*PFNGLPROGRAMUNIFORMUI64NVPROC) (GLuint program,
	GLint location, GLuint64EXT value);
     typedef void (*PFNGLPROGRAMUNIFORMUI64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint64EXT * value);
     typedef void (*PFNGLBUFFERADDRESSRANGENVPROC) (GLenum pname,
	GLuint index, GLuint64EXT address, GLsizeiptr length);
     typedef void (*PFNGLVERTEXFORMATNVPROC) (GLint size, GLenum type,
	GLsizei stride);
     typedef void (*PFNGLNORMALFORMATNVPROC) (GLenum type, GLsizei stride);
     typedef void (*PFNGLCOLORFORMATNVPROC) (GLint size, GLenum type,
	GLsizei stride);
     typedef void (*PFNGLINDEXFORMATNVPROC) (GLenum type, GLsizei stride);
     typedef void (*PFNGLTEXCOORDFORMATNVPROC) (GLint size, GLenum type,
	GLsizei stride);
     typedef void (*PFNGLEDGEFLAGFORMATNVPROC) (GLsizei stride);
     typedef void (*PFNGLSECONDARYCOLORFORMATNVPROC) (GLint size, GLenum type,
	GLsizei stride);
     typedef void (*PFNGLFOGCOORDFORMATNVPROC) (GLenum type, GLsizei stride);
     typedef void (*PFNGLVERTEXATTRIBFORMATNVPROC) (GLuint index, GLint size,
	GLenum type, GLboolean normalized, GLsizei stride);
     typedef void (*PFNGLVERTEXATTRIBIFORMATNVPROC) (GLuint index, GLint size,
	GLenum type, GLsizei stride);
     typedef void (*PFNGLGETINTEGERUI64I_VNVPROC) (GLenum value, GLuint index,
	GLuint64EXT * result);







     typedef void (*PFNGLTEXTUREBARRIERNVPROC) (void);
     typedef void (*PFNGLBINDIMAGETEXTUREEXTPROC) (GLuint index,
	GLuint texture, GLint level, GLboolean layered, GLint layer,
	GLenum access, GLint format);
     typedef void (*PFNGLMEMORYBARRIEREXTPROC) (GLbitfield barriers);
     typedef void (*PFNGLVERTEXATTRIBL1DEXTPROC) (GLuint index, GLdouble x);
     typedef void (*PFNGLVERTEXATTRIBL2DEXTPROC) (GLuint index, GLdouble x,
	GLdouble y);
     typedef void (*PFNGLVERTEXATTRIBL3DEXTPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z);
     typedef void (*PFNGLVERTEXATTRIBL4DEXTPROC) (GLuint index, GLdouble x,
	GLdouble y, GLdouble z, GLdouble w);
     typedef void (*PFNGLVERTEXATTRIBL1DVEXTPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBL2DVEXTPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBL3DVEXTPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBL4DVEXTPROC) (GLuint index,
	const GLdouble * v);
     typedef void (*PFNGLVERTEXATTRIBLPOINTEREXTPROC) (GLuint index,
	GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
     typedef void (*PFNGLGETVERTEXATTRIBLDVEXTPROC) (GLuint index,
	GLenum pname, GLdouble * params);
     typedef void (*PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC) (GLuint vaobj,
	GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride,
	GLintptr offset);
     typedef void (*PFNGLPROGRAMSUBROUTINEPARAMETERSUIVNVPROC) (GLenum target,
	GLsizei count, const GLuint * params);
     typedef void (*PFNGLGETPROGRAMSUBROUTINEPARAMETERUIVNVPROC) (GLenum
	target, GLuint index, GLuint * param);
     typedef void (*PFNGLUNIFORM1I64NVPROC) (GLint location, GLint64EXT x);
     typedef void (*PFNGLUNIFORM2I64NVPROC) (GLint location, GLint64EXT x,
	GLint64EXT y);
     typedef void (*PFNGLUNIFORM3I64NVPROC) (GLint location, GLint64EXT x,
	GLint64EXT y, GLint64EXT z);
     typedef void (*PFNGLUNIFORM4I64NVPROC) (GLint location, GLint64EXT x,
	GLint64EXT y, GLint64EXT z, GLint64EXT w);
     typedef void (*PFNGLUNIFORM1I64VNVPROC) (GLint location, GLsizei count,
	const GLint64EXT * value);
     typedef void (*PFNGLUNIFORM2I64VNVPROC) (GLint location, GLsizei count,
	const GLint64EXT * value);
     typedef void (*PFNGLUNIFORM3I64VNVPROC) (GLint location, GLsizei count,
	const GLint64EXT * value);
     typedef void (*PFNGLUNIFORM4I64VNVPROC) (GLint location, GLsizei count,
	const GLint64EXT * value);
     typedef void (*PFNGLUNIFORM1UI64NVPROC) (GLint location, GLuint64EXT x);
     typedef void (*PFNGLUNIFORM2UI64NVPROC) (GLint location, GLuint64EXT x,
	GLuint64EXT y);
     typedef void (*PFNGLUNIFORM3UI64NVPROC) (GLint location, GLuint64EXT x,
	GLuint64EXT y, GLuint64EXT z);
     typedef void (*PFNGLUNIFORM4UI64NVPROC) (GLint location, GLuint64EXT x,
	GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
     typedef void (*PFNGLUNIFORM1UI64VNVPROC) (GLint location, GLsizei count,
	const GLuint64EXT * value);
     typedef void (*PFNGLUNIFORM2UI64VNVPROC) (GLint location, GLsizei count,
	const GLuint64EXT * value);
     typedef void (*PFNGLUNIFORM3UI64VNVPROC) (GLint location, GLsizei count,
	const GLuint64EXT * value);
     typedef void (*PFNGLUNIFORM4UI64VNVPROC) (GLint location, GLsizei count,
	const GLuint64EXT * value);
     typedef void (*PFNGLGETUNIFORMI64VNVPROC) (GLuint program,
	GLint location, GLint64EXT * params);
     typedef void (*PFNGLPROGRAMUNIFORM1I64NVPROC) (GLuint program,
	GLint location, GLint64EXT x);
     typedef void (*PFNGLPROGRAMUNIFORM2I64NVPROC) (GLuint program,
	GLint location, GLint64EXT x, GLint64EXT y);
     typedef void (*PFNGLPROGRAMUNIFORM3I64NVPROC) (GLuint program,
	GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z);
     typedef void (*PFNGLPROGRAMUNIFORM4I64NVPROC) (GLuint program,
	GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z,
	GLint64EXT w);
     typedef void (*PFNGLPROGRAMUNIFORM1I64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint64EXT * value);
     typedef void (*PFNGLPROGRAMUNIFORM2I64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint64EXT * value);
     typedef void (*PFNGLPROGRAMUNIFORM3I64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint64EXT * value);
     typedef void (*PFNGLPROGRAMUNIFORM4I64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLint64EXT * value);
     typedef void (*PFNGLPROGRAMUNIFORM1UI64NVPROC) (GLuint program,
	GLint location, GLuint64EXT x);
     typedef void (*PFNGLPROGRAMUNIFORM2UI64NVPROC) (GLuint program,
	GLint location, GLuint64EXT x, GLuint64EXT y);
     typedef void (*PFNGLPROGRAMUNIFORM3UI64NVPROC) (GLuint program,
	GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
     typedef void (*PFNGLPROGRAMUNIFORM4UI64NVPROC) (GLuint program,
	GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z,
	GLuint64EXT w);
     typedef void (*PFNGLPROGRAMUNIFORM1UI64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint64EXT * value);
     typedef void (*PFNGLPROGRAMUNIFORM2UI64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint64EXT * value);
     typedef void (*PFNGLPROGRAMUNIFORM3UI64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint64EXT * value);
     typedef void (*PFNGLPROGRAMUNIFORM4UI64VNVPROC) (GLuint program,
	GLint location, GLsizei count, const GLuint64EXT * value);
     typedef void (*PFNGLVERTEXATTRIBL1I64NVPROC) (GLuint index,
	GLint64EXT x);
     typedef void (*PFNGLVERTEXATTRIBL2I64NVPROC) (GLuint index, GLint64EXT x,
	GLint64EXT y);
     typedef void (*PFNGLVERTEXATTRIBL3I64NVPROC) (GLuint index, GLint64EXT x,
	GLint64EXT y, GLint64EXT z);
     typedef void (*PFNGLVERTEXATTRIBL4I64NVPROC) (GLuint index, GLint64EXT x,
	GLint64EXT y, GLint64EXT z, GLint64EXT w);
     typedef void (*PFNGLVERTEXATTRIBL1I64VNVPROC) (GLuint index,
	const GLint64EXT * v);
     typedef void (*PFNGLVERTEXATTRIBL2I64VNVPROC) (GLuint index,
	const GLint64EXT * v);
     typedef void (*PFNGLVERTEXATTRIBL3I64VNVPROC) (GLuint index,
	const GLint64EXT * v);
     typedef void (*PFNGLVERTEXATTRIBL4I64VNVPROC) (GLuint index,
	const GLint64EXT * v);
     typedef void (*PFNGLVERTEXATTRIBL1UI64NVPROC) (GLuint index,
	GLuint64EXT x);
     typedef void (*PFNGLVERTEXATTRIBL2UI64NVPROC) (GLuint index,
	GLuint64EXT x, GLuint64EXT y);
     typedef void (*PFNGLVERTEXATTRIBL3UI64NVPROC) (GLuint index,
	GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
     typedef void (*PFNGLVERTEXATTRIBL4UI64NVPROC) (GLuint index,
	GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
     typedef void (*PFNGLVERTEXATTRIBL1UI64VNVPROC) (GLuint index,
	const GLuint64EXT * v);
     typedef void (*PFNGLVERTEXATTRIBL2UI64VNVPROC) (GLuint index,
	const GLuint64EXT * v);
     typedef void (*PFNGLVERTEXATTRIBL3UI64VNVPROC) (GLuint index,
	const GLuint64EXT * v);
     typedef void (*PFNGLVERTEXATTRIBL4UI64VNVPROC) (GLuint index,
	const GLuint64EXT * v);
     typedef void (*PFNGLGETVERTEXATTRIBLI64VNVPROC) (GLuint index,
	GLenum pname, GLint64EXT * params);
     typedef void (*PFNGLGETVERTEXATTRIBLUI64VNVPROC) (GLuint index,
	GLenum pname, GLuint64EXT * params);
     typedef void (*PFNGLVERTEXATTRIBLFORMATNVPROC) (GLuint index, GLint size,
	GLenum type, GLsizei stride);
     typedef void (*PFNGLGENNAMESAMDPROC) (GLenum identifier, GLuint num,
	GLuint * names);
     typedef void (*PFNGLDELETENAMESAMDPROC) (GLenum identifier, GLuint num,
	const GLuint * names);
     typedef GLboolean(*PFNGLISNAMEAMDPROC) (GLenum identifier, GLuint name);
     typedef void (*PFNGLDEBUGMESSAGEENABLEAMDPROC) (GLenum category,
	GLenum severity, GLsizei count, const GLuint * ids,
	GLboolean enabled);
     typedef void (*PFNGLDEBUGMESSAGEINSERTAMDPROC) (GLenum category,
	GLenum severity, GLuint id, GLsizei length, const GLchar * buf);
     typedef void (*PFNGLDEBUGMESSAGECALLBACKAMDPROC) (GLDEBUGPROCAMD
	callback, GLvoid * userParam);
     typedef GLuint(*PFNGLGETDEBUGMESSAGELOGAMDPROC) (GLuint count,
	GLsizei bufsize, GLenum * categories, GLuint * severities,
	GLuint * ids, GLsizei * lengths, GLchar * message);
     typedef void (*PFNGLVDPAUINITNVPROC) (const GLvoid * vdpDevice,
	const GLvoid * getProcAddress);
     typedef void (*PFNGLVDPAUFININVPROC) (void);
     typedef GLvdpauSurfaceNV(*PFNGLVDPAUREGISTERVIDEOSURFACENVPROC) (GLvoid *
	vdpSurface, GLenum target, GLsizei numTextureNames,
	const GLuint * textureNames);
     typedef GLvdpauSurfaceNV(*PFNGLVDPAUREGISTEROUTPUTSURFACENVPROC) (GLvoid
	* vdpSurface, GLenum target, GLsizei numTextureNames,
	const GLuint * textureNames);
     typedef void (*PFNGLVDPAUISSURFACENVPROC) (GLvdpauSurfaceNV surface);
     typedef void (*PFNGLVDPAUUNREGISTERSURFACENVPROC) (GLvdpauSurfaceNV
	surface);
     typedef void (*PFNGLVDPAUGETSURFACEIVNVPROC) (GLvdpauSurfaceNV surface,
	GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values);
     typedef void (*PFNGLVDPAUSURFACEACCESSNVPROC) (GLvdpauSurfaceNV surface,
	GLenum access);
     typedef void (*PFNGLVDPAUMAPSURFACESNVPROC) (GLsizei numSurfaces,
	const GLvdpauSurfaceNV * surfaces);
     typedef void (*PFNGLVDPAUUNMAPSURFACESNVPROC) (GLsizei numSurface,
	const GLvdpauSurfaceNV * surfaces);
     typedef void (*PFNGLTEXIMAGE2DMULTISAMPLECOVERAGENVPROC) (GLenum target,
	GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat,
	GLsizei width, GLsizei height, GLboolean fixedSampleLocations);
     typedef void (*PFNGLTEXIMAGE3DMULTISAMPLECOVERAGENVPROC) (GLenum target,
	GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat,
	GLsizei width, GLsizei height, GLsizei depth,
	GLboolean fixedSampleLocations);
     typedef void (*PFNGLTEXTUREIMAGE2DMULTISAMPLENVPROC) (GLuint texture,
	GLenum target, GLsizei samples, GLint internalFormat, GLsizei width,
	GLsizei height, GLboolean fixedSampleLocations);
     typedef void (*PFNGLTEXTUREIMAGE3DMULTISAMPLENVPROC) (GLuint texture,
	GLenum target, GLsizei samples, GLint internalFormat, GLsizei width,
	GLsizei height, GLsizei depth, GLboolean fixedSampleLocations);
     typedef void (*PFNGLTEXTUREIMAGE2DMULTISAMPLECOVERAGENVPROC) (GLuint
	texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples,
	GLint internalFormat, GLsizei width, GLsizei height,
	GLboolean fixedSampleLocations);
     typedef void (*PFNGLTEXTUREIMAGE3DMULTISAMPLECOVERAGENVPROC) (GLuint
	texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples,
	GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth,
	GLboolean fixedSampleLocations);
     typedef void (*PFNGLSETMULTISAMPLEFVAMDPROC) (GLenum pname, GLuint index,
	const GLfloat * val);







     typedef GLsync(*PFNGLIMPORTSYNCEXTPROC) (GLenum external_sync_type,
	GLintptr external_sync, GLbitfield flags);
     typedef void (*PFNGLMULTIDRAWARRAYSINDIRECTAMDPROC) (GLenum mode,
	const GLvoid * indirect, GLsizei primcount, GLsizei stride);
     typedef void (*PFNGLMULTIDRAWELEMENTSINDIRECTAMDPROC) (GLenum mode,
	GLenum type, const GLvoid * indirect, GLsizei primcount,
	GLsizei stride);
__attribute__ ((visibility("default")))
     GLhandleARB glCreateDebugObjectMESA(void);

__attribute__ ((visibility("default")))
     void glClearDebugLogMESA(GLhandleARB obj, GLenum logType,
	GLenum shaderType);
__attribute__ ((visibility("default")))
     void glGetDebugLogMESA(GLhandleARB obj, GLenum logType,
	GLenum shaderType, GLsizei maxLength, GLsizei * length,
	GLcharARB * debugLog);
__attribute__ ((visibility("default")))
     GLsizei glGetDebugLogLengthMESA(GLhandleARB obj, GLenum logType,
	GLenum shaderType);
     typedef void (*GLprogramcallbackMESA) (GLenum target, GLvoid * data);

__attribute__ ((visibility("default")))
     void glProgramCallbackMESA(GLenum target, GLprogramcallbackMESA callback,
	GLvoid * data);

__attribute__ ((visibility("default")))
     void glGetProgramRegisterfvMESA(GLenum target, GLsizei len,
	const GLubyte * name, GLfloat * v);
__attribute__ ((visibility("default")))
     void glBlendEquationSeparateATI(GLenum modeRGB, GLenum modeA);
     typedef void (*PFNGLBLENDEQUATIONSEPARATEATIPROC) (GLenum modeRGB,
	GLenum modeA);






     typedef void *GLeglImageOES;
     typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target,
	GLeglImageOES image);
     typedef void (*PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) (GLenum
	target, GLeglImageOES image);
	
#endif

