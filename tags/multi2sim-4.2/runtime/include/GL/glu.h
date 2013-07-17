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

#ifndef RUNTIME_INCLUDE_GL_GLU_H
#define RUNTIME_INCLUDE_GL_GLU_H

#include "gl.h"


typedef struct GLUnurbs GLUnurbs;
typedef struct GLUquadric GLUquadric;
typedef struct GLUtesselator GLUtesselator;


typedef GLUnurbs GLUnurbsObj;
typedef GLUquadric GLUquadricObj;
typedef GLUtesselator GLUtesselatorObj;
typedef GLUtesselator GLUtriangulatorObj;




typedef void (*_GLUfuncptr) (void);

extern void gluBeginCurve(GLUnurbs * nurb);
extern void gluBeginPolygon(GLUtesselator * tess);
extern void gluBeginSurface(GLUnurbs * nurb);
extern void gluBeginTrim(GLUnurbs * nurb);
extern GLint gluBuild1DMipmapLevels(GLenum target, GLint internalFormat,
	GLsizei width, GLenum format, GLenum type, GLint level, GLint base,
	GLint max, const void *data);
extern GLint gluBuild1DMipmaps(GLenum target, GLint internalFormat,
	GLsizei width, GLenum format, GLenum type, const void *data);
extern GLint gluBuild2DMipmapLevels(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLenum format, GLenum type,
	GLint level, GLint base, GLint max, const void *data);
extern GLint gluBuild2DMipmaps(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLenum format, GLenum type,
	const void *data);
extern GLint gluBuild3DMipmapLevels(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format,
	GLenum type, GLint level, GLint base, GLint max, const void *data);
extern GLint gluBuild3DMipmaps(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format,
	GLenum type, const void *data);
extern GLboolean gluCheckExtension(const GLubyte * extName,
	const GLubyte * extString);
extern void gluCylinder(GLUquadric * quad, GLdouble base, GLdouble top,
	GLdouble height, GLint slices, GLint stacks);
extern void gluDeleteNurbsRenderer(GLUnurbs * nurb);
extern void gluDeleteQuadric(GLUquadric * quad);
extern void gluDeleteTess(GLUtesselator * tess);
extern void gluDisk(GLUquadric * quad, GLdouble inner, GLdouble outer,
	GLint slices, GLint loops);
extern void gluEndCurve(GLUnurbs * nurb);
extern void gluEndPolygon(GLUtesselator * tess);
extern void gluEndSurface(GLUnurbs * nurb);
extern void gluEndTrim(GLUnurbs * nurb);
extern const GLubyte *gluErrorString(GLenum error);
extern void gluGetNurbsProperty(GLUnurbs * nurb, GLenum property,
	GLfloat * data);
extern const GLubyte *gluGetString(GLenum name);
extern void gluGetTessProperty(GLUtesselator * tess, GLenum which,
	GLdouble * data);
extern void gluLoadSamplingMatrices(GLUnurbs * nurb, const GLfloat * model,
	const GLfloat * perspective, const GLint * view);
extern void gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
	GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX,
	GLdouble upY, GLdouble upZ);
extern GLUnurbs *gluNewNurbsRenderer(void);
extern GLUquadric *gluNewQuadric(void);
extern GLUtesselator *gluNewTess(void);
extern void gluNextContour(GLUtesselator * tess, GLenum type);
extern void gluNurbsCallback(GLUnurbs * nurb, GLenum which,
	_GLUfuncptr CallBackFunc);
extern void gluNurbsCallbackData(GLUnurbs * nurb, GLvoid * userData);
extern void gluNurbsCallbackDataEXT(GLUnurbs * nurb, GLvoid * userData);
extern void gluNurbsCurve(GLUnurbs * nurb, GLint knotCount, GLfloat * knots,
	GLint stride, GLfloat * control, GLint order, GLenum type);
extern void gluNurbsProperty(GLUnurbs * nurb, GLenum property, GLfloat value);
extern void gluNurbsSurface(GLUnurbs * nurb, GLint sKnotCount,
	GLfloat * sKnots, GLint tKnotCount, GLfloat * tKnots, GLint sStride,
	GLint tStride, GLfloat * control, GLint sOrder, GLint tOrder,
	GLenum type);
extern void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom,
	GLdouble top);
extern void gluPartialDisk(GLUquadric * quad, GLdouble inner, GLdouble outer,
	GLint slices, GLint loops, GLdouble start, GLdouble sweep);
extern void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear,
	GLdouble zFar);
extern void gluPickMatrix(GLdouble x, GLdouble y, GLdouble delX,
	GLdouble delY, GLint * viewport);
extern GLint gluProject(GLdouble objX, GLdouble objY, GLdouble objZ,
	const GLdouble * model, const GLdouble * proj, const GLint * view,
	GLdouble * winX, GLdouble * winY, GLdouble * winZ);
extern void gluPwlCurve(GLUnurbs * nurb, GLint count, GLfloat * data,
	GLint stride, GLenum type);
extern void gluQuadricCallback(GLUquadric * quad, GLenum which,
	_GLUfuncptr CallBackFunc);
extern void gluQuadricDrawStyle(GLUquadric * quad, GLenum draw);
extern void gluQuadricNormals(GLUquadric * quad, GLenum normal);
extern void gluQuadricOrientation(GLUquadric * quad, GLenum orientation);
extern void gluQuadricTexture(GLUquadric * quad, GLboolean texture);
extern GLint gluScaleImage(GLenum format, GLsizei wIn, GLsizei hIn,
	GLenum typeIn, const void *dataIn, GLsizei wOut, GLsizei hOut,
	GLenum typeOut, GLvoid * dataOut);
extern void gluSphere(GLUquadric * quad, GLdouble radius, GLint slices,
	GLint stacks);
extern void gluTessBeginContour(GLUtesselator * tess);
extern void gluTessBeginPolygon(GLUtesselator * tess, GLvoid * data);
extern void gluTessCallback(GLUtesselator * tess, GLenum which,
	_GLUfuncptr CallBackFunc);
extern void gluTessEndContour(GLUtesselator * tess);
extern void gluTessEndPolygon(GLUtesselator * tess);
extern void gluTessNormal(GLUtesselator * tess, GLdouble valueX,
	GLdouble valueY, GLdouble valueZ);
extern void gluTessProperty(GLUtesselator * tess, GLenum which,
	GLdouble data);
extern void gluTessVertex(GLUtesselator * tess, GLdouble * location,
	GLvoid * data);
extern GLint gluUnProject(GLdouble winX, GLdouble winY, GLdouble winZ,
	const GLdouble * model, const GLdouble * proj, const GLint * view,
	GLdouble * objX, GLdouble * objY, GLdouble * objZ);
extern GLint gluUnProject4(GLdouble winX, GLdouble winY, GLdouble winZ,
	GLdouble clipW, const GLdouble * model, const GLdouble * proj,
	const GLint * view, GLdouble nearVal, GLdouble farVal,
	GLdouble * objX, GLdouble * objY, GLdouble * objZ, GLdouble * objW);

#endif

