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

#include "../include/GL/glu.h"
#include "api.h"
#include "debug.h"
#include "string.h"




/*
 * Error Messages
 */

#define __GLU_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, glu_err_not_impl);


static char *glu_err_not_impl =
	"\tMulti2Sim provides partial support for the GLU library.\n"
	"\tTo request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

static char *glu_err_version =
	"\tYour guest application is using a version of the GLU runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tGLU runtime library ('libm2s-glu').\n";

static char *glu_err_native =
	"\tYou are trying to run natively an application using the Multi2Sim GLU\n"
	"\tlibrary implementation ('libm2s-glu'). Please run this program on top of\n"
	"\tMulti2Sim.\n";

/*static char *glu_err_version =
	"\tYour guest application is using a version of the GLU runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tOpengl runtime library ('libm2s-glu').\n";*/




/* Multi2Sim GLU Runtime required */
#define GLU_RUNTIME_VERSION_MAJOR	0
#define GLU_RUNTIME_VERSION_MINOR	669

struct glu_version_t
{
	int major;
	int minor;
};

void gluInit(int *argcp, char **argv)
{
	struct glu_version_t version;

	int ret;

	/* Runtime function 'init' */
	ret = syscall(GLU_SYSCALL_CODE, glu_call_init, &version);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call glu_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			glu_err_native);

	/* Check that exact major version matches */
	if (version.major != GLU_RUNTIME_VERSION_MAJOR
			|| version.minor < GLU_RUNTIME_VERSION_MINOR)
		fatal("incompatible GLU runtime versions.\n"
			"\tGuest library v. %d.%d / Host implementation v. %d.%d.\n"
			"%s", GLU_RUNTIME_VERSION_MAJOR, GLU_RUNTIME_VERSION_MINOR,
			version.major, version.minor, glu_err_version);

	/* Initialize */
	
}

void gluBeginCurve(GLUnurbs * nurb)
{
	__GLU_NOT_IMPL__
}

void gluBeginPolygon(GLUtesselator * tess)
{
	__GLU_NOT_IMPL__
}

void gluBeginSurface(GLUnurbs * nurb)
{
	__GLU_NOT_IMPL__
}

void gluBeginTrim(GLUnurbs * nurb)
{
	__GLU_NOT_IMPL__
}

GLint gluBuild1DMipmapLevels(GLenum target, GLint internalFormat,
	GLsizei width, GLenum format, GLenum type, GLint level, GLint base,
	GLint max, const void *data)
{
	__GLU_NOT_IMPL__
	return 0;
}

GLint gluBuild1DMipmaps(GLenum target, GLint internalFormat,
	GLsizei width, GLenum format, GLenum type, const void *data)
{
	__GLU_NOT_IMPL__
	return 0;
}

GLint gluBuild2DMipmapLevels(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLenum format, GLenum type,
	GLint level, GLint base, GLint max, const void *data)
{
	__GLU_NOT_IMPL__
	return 0;
}

GLint gluBuild2DMipmaps(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLenum format, GLenum type,
	const void *data)
{
	__GLU_NOT_IMPL__
	return 0;
}

GLint gluBuild3DMipmapLevels(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format,
	GLenum type, GLint level, GLint base, GLint max, const void *data)
{
	__GLU_NOT_IMPL__
	return 0;
}

GLint gluBuild3DMipmaps(GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format,
	GLenum type, const void *data)
{
	__GLU_NOT_IMPL__
	return 0;
}

GLboolean gluCheckExtension(const GLubyte * extName,
	const GLubyte * extString)
{
	__GLU_NOT_IMPL__
	return 0;
}

void gluCylinder(GLUquadric * quad, GLdouble base, GLdouble top,
	GLdouble height, GLint slices, GLint stacks)
{
	__GLU_NOT_IMPL__
}

void gluDeleteNurbsRenderer(GLUnurbs * nurb)
{
	__GLU_NOT_IMPL__
}

void gluDeleteQuadric(GLUquadric * quad)
{
	__GLU_NOT_IMPL__
}

void gluDeleteTess(GLUtesselator * tess)
{
	__GLU_NOT_IMPL__
}

void gluDisk(GLUquadric * quad, GLdouble inner, GLdouble outer,
	GLint slices, GLint loops)
{
	__GLU_NOT_IMPL__
}

void gluEndCurve(GLUnurbs * nurb)
{
	__GLU_NOT_IMPL__
}

void gluEndPolygon(GLUtesselator * tess)
{
	__GLU_NOT_IMPL__
}

void gluEndSurface(GLUnurbs * nurb)
{
	__GLU_NOT_IMPL__
}

void gluEndTrim(GLUnurbs * nurb)
{
	__GLU_NOT_IMPL__
}

static struct str_map_t opengl_error_map =
{
	7,
	{
		{ "INVALID_ENUM", GL_INVALID_ENUM },
		{ "INVALID_VALUE", GL_INVALID_VALUE },
		{ "INVALID_OPERATION", GL_INVALID_OPERATION },
		{ "INVALID_FRAMEBUFFER_OPERATION", GL_INVALID_FRAMEBUFFER_OPERATION },
		{ "OUT_OF_MEMORY ", GL_OUT_OF_MEMORY },
		{ "STACK_OVERFLOW", GL_STACK_OVERFLOW },
		{ "STACK_UNDERFLOW", GL_STACK_OVERFLOW }
	}
};

const GLubyte *gluErrorString(GLenum error)
{
	return (const GLubyte *)str_map_value(&opengl_error_map, error);
}

void gluGetNurbsProperty(GLUnurbs * nurb, GLenum property,
	GLfloat * data)
{
	__GLU_NOT_IMPL__
}

const GLubyte *gluGetString(GLenum name)
{
	__GLU_NOT_IMPL__
	return NULL;
}

void gluGetTessProperty(GLUtesselator * tess, GLenum which,
	GLdouble * data)
{
	__GLU_NOT_IMPL__
}

void gluLoadSamplingMatrices(GLUnurbs * nurb, const GLfloat * model,
	const GLfloat * perspective, const GLint * view)
{
	__GLU_NOT_IMPL__
}

void gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
	GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX,
	GLdouble upY, GLdouble upZ)
{
	__GLU_NOT_IMPL__
}

GLUnurbs *gluNewNurbsRenderer(void)
{
	__GLU_NOT_IMPL__
	return NULL;
}

GLUquadric *gluNewQuadric(void)
{
	__GLU_NOT_IMPL__
	return NULL;
}

GLUtesselator *gluNewTess(void)
{
	__GLU_NOT_IMPL__
	return NULL;
}

void gluNextContour(GLUtesselator * tess, GLenum type)
{
	__GLU_NOT_IMPL__
}

void gluNurbsCallback(GLUnurbs * nurb, GLenum which,
	_GLUfuncptr CallBackFunc)
{
	__GLU_NOT_IMPL__
}

void gluNurbsCallbackData(GLUnurbs * nurb, GLvoid * userData)
{
	__GLU_NOT_IMPL__
}

void gluNurbsCallbackDataEXT(GLUnurbs * nurb, GLvoid * userData)
{
	__GLU_NOT_IMPL__
}

void gluNurbsCurve(GLUnurbs * nurb, GLint knotCount, GLfloat * knots,
	GLint stride, GLfloat * control, GLint order, GLenum type)
{
	__GLU_NOT_IMPL__
}

void gluNurbsProperty(GLUnurbs * nurb, GLenum property, GLfloat value)
{
	__GLU_NOT_IMPL__
}

void gluNurbsSurface(GLUnurbs * nurb, GLint sKnotCount,
	GLfloat * sKnots, GLint tKnotCount, GLfloat * tKnots, GLint sStride,
	GLint tStride, GLfloat * control, GLint sOrder, GLint tOrder,
	GLenum type)
{
	__GLU_NOT_IMPL__
}

void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom,
	GLdouble top)
{
	__GLU_NOT_IMPL__
}

void gluPartialDisk(GLUquadric * quad, GLdouble inner, GLdouble outer,
	GLint slices, GLint loops, GLdouble start, GLdouble sweep)
{
	__GLU_NOT_IMPL__
}

void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear,
	GLdouble zFar)
{
	__GLU_NOT_IMPL__
}

void gluPickMatrix(GLdouble x, GLdouble y, GLdouble delX,
	GLdouble delY, GLint * viewport)
{
	__GLU_NOT_IMPL__
}

GLint gluProject(GLdouble objX, GLdouble objY, GLdouble objZ,
	const GLdouble * model, const GLdouble * proj, const GLint * view,
	GLdouble * winX, GLdouble * winY, GLdouble * winZ)
{
	__GLU_NOT_IMPL__
	return 0;
}

void gluPwlCurve(GLUnurbs * nurb, GLint count, GLfloat * data,
	GLint stride, GLenum type)
{
	__GLU_NOT_IMPL__
}

void gluQuadricCallback(GLUquadric * quad, GLenum which,
	_GLUfuncptr CallBackFunc)
{
	__GLU_NOT_IMPL__
}

void gluQuadricDrawStyle(GLUquadric * quad, GLenum draw)
{
	__GLU_NOT_IMPL__
}

void gluQuadricNormals(GLUquadric * quad, GLenum normal)
{
	__GLU_NOT_IMPL__
}

void gluQuadricOrientation(GLUquadric * quad, GLenum orientation)
{
	__GLU_NOT_IMPL__
}

void gluQuadricTexture(GLUquadric * quad, GLboolean texture)
{
	__GLU_NOT_IMPL__
}

GLint gluScaleImage(GLenum format, GLsizei wIn, GLsizei hIn,
	GLenum typeIn, const void *dataIn, GLsizei wOut, GLsizei hOut,
	GLenum typeOut, GLvoid * dataOut)
{
	__GLU_NOT_IMPL__
	return 0;
}

void gluSphere(GLUquadric * quad, GLdouble radius, GLint slices,
	GLint stacks)
{
	__GLU_NOT_IMPL__
}

void gluTessBeginContour(GLUtesselator * tess)
{
	__GLU_NOT_IMPL__
}

void gluTessBeginPolygon(GLUtesselator * tess, GLvoid * data)
{
	__GLU_NOT_IMPL__
}

void gluTessCallback(GLUtesselator * tess, GLenum which,
	_GLUfuncptr CallBackFunc)
{
	__GLU_NOT_IMPL__
}

void gluTessEndContour(GLUtesselator * tess)
{
	__GLU_NOT_IMPL__
}

void gluTessEndPolygon(GLUtesselator * tess)
{
	__GLU_NOT_IMPL__
}

void gluTessNormal(GLUtesselator * tess, GLdouble valueX,
	GLdouble valueY, GLdouble valueZ)
{
	__GLU_NOT_IMPL__
}

void gluTessProperty(GLUtesselator * tess, GLenum which,
	GLdouble data)
{
	__GLU_NOT_IMPL__
}

void gluTessVertex(GLUtesselator * tess, GLdouble * location,
	GLvoid * data)
{
	__GLU_NOT_IMPL__
}

GLint gluUnProject(GLdouble winX, GLdouble winY, GLdouble winZ,
	const GLdouble * model, const GLdouble * proj, const GLint * view,
	GLdouble * objX, GLdouble * objY, GLdouble * objZ)
{
	__GLU_NOT_IMPL__
	return 0;
}

GLint gluUnProject4(GLdouble winX, GLdouble winY, GLdouble winZ,
	GLdouble clipW, const GLdouble * model, const GLdouble * proj,
	const GLint * view, GLdouble nearVal, GLdouble farVal,
	GLdouble * objX, GLdouble * objY, GLdouble * objZ, GLdouble * objW)
{
	__GLU_NOT_IMPL__
	return 0;
}

