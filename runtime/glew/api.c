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

#include "../include/GL/glew.h"
#include "api.h"
#include "debug.h"



/*
 * Error Messages
 */

#define __GLEW_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, glew_err_not_impl);


static char *glew_err_not_impl =
	"\tMulti2Sim provides partial support for the GLEW library.\n"
	"\tTo request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

static char *glew_err_version =
	"\tYour guest application is using a version of the GLEW runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tGLEW runtime library ('libm2s-glew').\n";

static char *glew_err_native =
	"\tYou are trying to run natively an application using the Multi2Sim GLEW\n"
	"\tlibrary implementation ('libm2s-glew'). Please run this program on top of\n"
	"\tMulti2Sim.\n";

/*static char *glew_err_version =
	"\tYour guest application is using a version of the GLEW runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tOpengl runtime library ('libm2s-glew').\n";*/




/* Multi2Sim GLEW Runtime required */
#define GLEW_RUNTIME_VERSION_MAJOR	0
#define GLEW_RUNTIME_VERSION_MINOR	669

struct glew_version_t
{
	int major;
	int minor;
};


/* GLEW global variable */
GLboolean glewExperimental;

GLenum glewInit(int *argcp, char **argv)
{
	struct glew_version_t version;

	int ret;

	/* Runtime function 'init' */
	ret = syscall(GLEW_SYSCALL_CODE, glew_call_init, &version);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call glew_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			glew_err_native);

	/* Check that exact major version matches */
	if (version.major != GLEW_RUNTIME_VERSION_MAJOR
			|| version.minor < GLEW_RUNTIME_VERSION_MINOR)
		fatal("incompatible GLEW runtime versions.\n"
			"\tGuest library v. %d.%d / Host implementation v. %d.%d.\n"
			"%s", GLEW_RUNTIME_VERSION_MAJOR, GLEW_RUNTIME_VERSION_MINOR,
			version.major, version.minor, glew_err_version);

	/* Initialize */
	
	/* Return */
	return 0;
}

 GLboolean glewIsSupported (const char* name)
 {
 	__GLEW_NOT_IMPL__
 	return 0;
 }

 GLboolean glewGetExtension (const char* name)
 {
 	__GLEW_NOT_IMPL__
 	return 0;
 }

const GLubyte* glewGetErrorString (GLenum error)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

const GLubyte* glewGetString (GLenum name)
{
	__GLEW_NOT_IMPL__
	return NULL;
}


 void __glewCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	__GLEW_NOT_IMPL__
}

void __glewTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewActiveTexture(GLenum texture)
{
	__GLEW_NOT_IMPL__
}

void __glewClientActiveTexture(GLenum texture)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCompressedTexImage(GLenum target, GLint lod, GLvoid *img)
{
	__GLEW_NOT_IMPL__
}

void __glewLoadTransposeMatrixd(const GLdouble m[16])
{
	__GLEW_NOT_IMPL__
}

void __glewLoadTransposeMatrixf(const GLfloat m[16])
{
	__GLEW_NOT_IMPL__
}

void __glewMultTransposeMatrixd(const GLdouble m[16])
{
	__GLEW_NOT_IMPL__
}

void __glewMultTransposeMatrixf(const GLfloat m[16])
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1d(GLenum target, GLdouble s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1dv(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1f(GLenum target, GLfloat s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1fv(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1i(GLenum target, GLint s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1iv(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1s(GLenum target, GLshort s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1sv(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2dv(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2fv(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2i(GLenum target, GLint s, GLint t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2iv(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2sv(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3dv(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3fv(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3iv(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3sv(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4dv(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4fv(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4iv(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4sv(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSampleCoverage(GLclampf value, GLboolean invert)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquation(GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordd(GLdouble coord)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoorddv(const GLdouble *coord)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordf(GLfloat coord)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordfv(const GLfloat *coord)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameterf(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameterfv(GLenum pname, const GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameteri(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameteriv(GLenum pname, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3bv(const GLbyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3dv(const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3fv(const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3i(GLint red, GLint green, GLint blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3iv(const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3s(GLshort red, GLshort green, GLshort blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3sv(const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3ubv(const GLubyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3ui(GLuint red, GLuint green, GLuint blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3uiv(const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3us(GLushort red, GLushort green, GLushort blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3usv(const GLushort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2d(GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2dv(const GLdouble *p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2f(GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2fv(const GLfloat *p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2i(GLint x, GLint y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2iv(const GLint *p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2s(GLshort x, GLshort y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2sv(const GLshort *p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3dv(const GLdouble *p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3fv(const GLfloat *p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3i(GLint x, GLint y, GLint z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3iv(const GLint *p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3s(GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3sv(const GLshort *p)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginQuery(GLenum target, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBuffer(GLenum target, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
	__GLEW_NOT_IMPL__
}

void __glewBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteBuffers(GLsizei n, const GLuint* buffers)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteQueries(GLsizei n, const GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewEndQuery(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewGenBuffers(GLsizei n, GLuint* buffers)
{
	__GLEW_NOT_IMPL__
}

void __glewGenQueries(GLsizei n, GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBufferPointerv(GLenum target, GLenum pname, GLvoid** params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjectiv(GLuint id, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryiv(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsBuffer(GLuint buffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLboolean __glewIsQuery(GLuint id)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLvoid* __glewMapBuffer(GLenum target, GLenum access)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

GLboolean __glewUnmapBuffer(GLenum target)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewAttachShader(GLuint program, GLuint shader)
{
	__GLEW_NOT_IMPL__
}

void __glewBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewCompileShader(GLuint shader)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewCreateProgram(void)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewCreateShader(GLenum type)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDeleteProgram(GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteShader(GLuint shader)
{
	__GLEW_NOT_IMPL__
}

void __glewDetachShader(GLuint program, GLuint shader)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableVertexAttribArray(GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawBuffers(GLsizei n, const GLenum* bufs)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableVertexAttribArray(GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveAttrib(GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveUniform(GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetAttribLocation(GLuint program, const GLchar* name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramiv(GLuint program, GLenum pname, GLint* param)
{
	__GLEW_NOT_IMPL__
}

void __glewGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	__GLEW_NOT_IMPL__
}

void __glewGetShaderSource(GLuint obj, GLsizei maxLength, GLsizei* length, GLchar* source)
{
	__GLEW_NOT_IMPL__
}

void __glewGetShaderiv(GLuint shader, GLenum pname, GLint* param)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetUniformLocation(GLuint program, const GLchar* name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetUniformiv(GLuint program, GLint location, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid **pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsProgram(GLuint program)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLboolean __glewIsShader(GLuint shader)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewLinkProgram(GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewShaderSource(GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths)
{
	__GLEW_NOT_IMPL__
}

void __glewStencilFuncSeparate(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
	__GLEW_NOT_IMPL__
}

void __glewStencilMaskSeparate(GLenum face, GLuint mask)
{
	__GLEW_NOT_IMPL__
}

void __glewStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1f(GLint location, GLfloat v0)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1fv(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1i(GLint location, GLint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1iv(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2i(GLint location, GLint v0, GLint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2iv(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3iv(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4iv(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUseProgram(GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewValidateProgram(GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1d(GLuint index, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1f(GLuint index, GLfloat x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1fv(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1s(GLuint index, GLshort x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1sv(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2fv(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2sv(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3fv(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3sv(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4Nbv(GLuint index, const GLbyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4Niv(GLuint index, const GLint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4Nsv(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4Nubv(GLuint index, const GLubyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4Nuiv(GLuint index, const GLuint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4Nusv(GLuint index, const GLushort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4bv(GLuint index, const GLbyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4fv(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4iv(GLuint index, const GLint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4sv(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4ubv(GLuint index, const GLubyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4uiv(GLuint index, const GLuint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4usv(GLuint index, const GLushort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginConditionalRender(GLuint id, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginTransformFeedback(GLenum primitiveMode)
{
	__GLEW_NOT_IMPL__
}

void __glewBindFragDataLocation(GLuint program, GLuint colorNumber, const GLchar* name)
{
	__GLEW_NOT_IMPL__
}

void __glewClampColor(GLenum target, GLenum clamp)
{
	__GLEW_NOT_IMPL__
}

void __glewClearBufferfi(GLenum buffer, GLint drawBuffer, GLfloat depth, GLint stencil)
{
	__GLEW_NOT_IMPL__
}

void __glewClearBufferfv(GLenum buffer, GLint drawBuffer, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewClearBufferiv(GLenum buffer, GLint drawBuffer, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewClearBufferuiv(GLenum buffer, GLint drawBuffer, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewColorMaski(GLuint buf, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	__GLEW_NOT_IMPL__
}

void __glewDisablei(GLenum cap, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewEnablei(GLenum cap, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewEndConditionalRender(void)
{
	__GLEW_NOT_IMPL__
}

void __glewEndTransformFeedback(void)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBooleani_v(GLenum pname, GLuint index, GLboolean* data)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetFragDataLocation(GLuint program, const GLchar* data)
{
	__GLEW_NOT_IMPL__
	return 0;
}

const GLubyte* __glewGetStringi(GLenum name, GLuint index)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

void __glewGetTexParameterIiv(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei size, GLenum *type, char *name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetUniformuiv(GLuint program, GLint location, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsEnabledi(GLenum cap, GLuint index)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewTexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar **varyings, GLenum bufferMode)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1ui(GLint location, GLuint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2ui(GLint location, GLuint v0, GLuint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1i(GLuint index, GLint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1iv(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1ui(GLuint index, GLuint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1uiv(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2i(GLuint index, GLint v0, GLint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2iv(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2ui(GLuint index, GLuint v0, GLuint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2uiv(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3i(GLuint index, GLint v0, GLint v1, GLint v2)
{ 
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3iv(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3ui(GLuint index, GLuint v0, GLuint v1, GLuint v2)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3uiv(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4bv(GLuint index, const GLbyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4i(GLuint index, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4iv(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4sv(GLuint index, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4ubv(GLuint index, const GLubyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4ui(GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4uiv(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4usv(GLuint index, const GLushort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewPrimitiveRestartIndex(GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribDivisor(GLuint index, GLuint divisor)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationi(GLuint buf, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
	__GLEW_NOT_IMPL__
}

void __glewMinSampleShading(GLclampf value)
{
	__GLEW_NOT_IMPL__
}

void __glewTbufferMask3DFX(GLuint mask)
{
	__GLEW_NOT_IMPL__
}

void __glewDebugMessageCallbackAMD(GLDEBUGPROCAMD callback, void* userParam)
{
	__GLEW_NOT_IMPL__
}

void __glewDebugMessageEnableAMD(GLenum category, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
	__GLEW_NOT_IMPL__
}

void __glewDebugMessageInsertAMD(GLenum category, GLenum severity, GLuint id, GLsizei length, const char* buf)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewGetDebugMessageLogAMD(GLuint count, GLsizei bufsize, GLenum* categories, GLuint* severities, GLuint* ids, GLsizei* lengths, char* message)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewBlendEquationIndexedAMD(GLuint buf, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationSeparateIndexedAMD(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFuncIndexedAMD(GLuint buf, GLenum src, GLenum dst)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFuncSeparateIndexedAMD(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteNamesAMD(GLenum identifier, GLuint num, const GLuint* names)
{
	__GLEW_NOT_IMPL__
}

void __glewGenNamesAMD(GLenum identifier, GLuint num, GLuint* names)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsNameAMD(GLenum identifier, GLuint name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewBeginPerfMonitorAMD(GLuint monitor)
{
	__GLEW_NOT_IMPL__
}

void __glewDeletePerfMonitorsAMD(GLsizei n, GLuint* monitors)
{
	__GLEW_NOT_IMPL__
}

void __glewEndPerfMonitorAMD(GLuint monitor)
{
	__GLEW_NOT_IMPL__
}

void __glewGenPerfMonitorsAMD(GLsizei n, GLuint* monitors)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPerfMonitorCounterDataAMD(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint *bytesWritten)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPerfMonitorCounterInfoAMD(GLuint group, GLuint counter, GLenum pname, void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPerfMonitorCounterStringAMD(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, char *counterString)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPerfMonitorCountersAMD(GLuint group, GLint* numCounters, GLint *maxActiveCounters, GLsizei countersSize, GLuint *counters)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPerfMonitorGroupStringAMD(GLuint group, GLsizei bufSize, GLsizei* length, char *groupString)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPerfMonitorGroupsAMD(GLint* numGroups, GLsizei groupsSize, GLuint *groups)
{
	__GLEW_NOT_IMPL__
}

void __glewSelectPerfMonitorCountersAMD(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* counterList)
{
	__GLEW_NOT_IMPL__
}

void __glewSetMultisamplefvAMD(GLenum pname, GLuint index, const GLfloat* val)
{
	__GLEW_NOT_IMPL__
}

void __glewTessellationFactorAMD(GLfloat factor)
{
	__GLEW_NOT_IMPL__
}

void __glewTessellationModeAMD(GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawElementArrayAPPLE(GLenum mode, GLint first, GLsizei count)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawRangeElementArrayAPPLE(GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count)
{
	__GLEW_NOT_IMPL__
}

void __glewElementPointerAPPLE(GLenum type, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiDrawElementArrayAPPLE(GLenum mode, const GLint* first, const GLsizei *count, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiDrawRangeElementArrayAPPLE(GLenum mode, GLuint start, GLuint end, const GLint* first, const GLsizei *count, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteFencesAPPLE(GLsizei n, const GLuint* fences)
{
	__GLEW_NOT_IMPL__
}

void __glewFinishFenceAPPLE(GLuint fence)
{
	__GLEW_NOT_IMPL__
}

void __glewFinishObjectAPPLE(GLenum object, GLint name)
{
	__GLEW_NOT_IMPL__
}

void __glewGenFencesAPPLE(GLsizei n, GLuint* fences)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsFenceAPPLE(GLuint fence)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewSetFenceAPPLE(GLuint fence)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewTestFenceAPPLE(GLuint fence)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLboolean __glewTestObjectAPPLE(GLenum object, GLuint name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewBufferParameteriAPPLE(GLenum target, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewFlushMappedBufferRangeAPPLE(GLenum target, GLintptr offset, GLsizeiptr size)
{
	__GLEW_NOT_IMPL__
}

void __glewGetObjectParameterivAPPLE(GLenum objectType, GLuint name, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLenum __glewObjectPurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLenum __glewObjectUnpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetTexParameterPointervAPPLE(GLenum target, GLenum pname, GLvoid **params)
{
	__GLEW_NOT_IMPL__
}

void __glewBindVertexArrayAPPLE(GLuint array)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteVertexArraysAPPLE(GLsizei n, const GLuint* arrays)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsVertexArrayAPPLE(GLuint array)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewFlushVertexArrayRangeAPPLE(GLsizei length, void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayParameteriAPPLE(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayRangeAPPLE(GLsizei length, void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableVertexAttribAPPLE(GLuint index, GLenum pname)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableVertexAttribAPPLE(GLuint index, GLenum pname)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsVertexAttribEnabledAPPLE(GLuint index, GLenum pname)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewMapVertexAttrib1dAPPLE(GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points)
{
	__GLEW_NOT_IMPL__
}

void __glewMapVertexAttrib1fAPPLE(GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points)
{
	__GLEW_NOT_IMPL__
}

void __glewMapVertexAttrib2dAPPLE(GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points)
{
	__GLEW_NOT_IMPL__
}

void __glewMapVertexAttrib2fAPPLE(GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points)
{
	__GLEW_NOT_IMPL__
}

void __glewClearDepthf(GLclampf d)
{
	__GLEW_NOT_IMPL__
}

void __glewDepthRangef(GLclampf n, GLclampf f)
{
	__GLEW_NOT_IMPL__
}

void __glewGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint *precision)
{
	__GLEW_NOT_IMPL__
}

void __glewReleaseShaderCompiler(void)
{
	__GLEW_NOT_IMPL__
}

void __glewShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryformat, const GLvoid*binary, GLsizei length)
{
	__GLEW_NOT_IMPL__
}

void __glewBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const char * name)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetFragDataIndex(GLuint program, const char * name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

typedef struct _cl_context *cl_context;
typedef struct _cl_event *cl_event;

GLsync __glewCreateSyncFromCLeventARB(cl_context context, cl_event event, GLbitfield flags)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewClampColorARB(GLenum target, GLenum clamp)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyBufferSubData(GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size)
{
	__GLEW_NOT_IMPL__
}

void __glewDebugMessageControlARB(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
	__GLEW_NOT_IMPL__
}

void __glewDebugMessageInsertARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* buf)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewGetDebugMessageLogARB(GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, char* messageLog)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDrawBuffersARB(GLsizei n, const GLenum* bufs)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationSeparateiARB(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationiARB(GLuint buf, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFuncSeparateiARB(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFunciARB(GLuint buf, GLenum src, GLenum dst)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount, GLint basevertex)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawArraysIndirect(GLenum mode, const void* indirect)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect)
{
	__GLEW_NOT_IMPL__
}

void __glewBindFramebuffer(GLenum target, GLuint framebuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	__GLEW_NOT_IMPL__
}

GLenum __glewCheckFramebufferStatus(GLenum target)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTextureLayer(GLenum target,GLenum attachment, GLuint texture,GLint level,GLint layer)
{
	__GLEW_NOT_IMPL__
}

void __glewGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewGenerateMipmap(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsFramebuffer(GLuint framebuffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLboolean __glewIsRenderbuffer(GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTextureARB(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTextureFaceARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTextureLayerARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameteriARB(GLuint program, GLenum pname, GLint value)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum *binaryFormat, GLvoid*binary)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameteri(GLuint program, GLenum pname, GLint value)
{
	__GLEW_NOT_IMPL__
}

void __glewGetUniformdv(GLuint program, GLint location, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1dEXT(GLuint program, GLint location, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2dEXT(GLuint program, GLint location, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1d(GLint location, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1dv(GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2d(GLint location, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2dv(GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3dv(GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4dv(GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data)
{
	__GLEW_NOT_IMPL__
}

void __glewColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
	__GLEW_NOT_IMPL__
}

void __glewColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameterf(GLenum target, GLenum pname, GLfloat params)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameteri(GLenum target, GLenum pname, GLint params)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid *table)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableParameteriv(GLenum target, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid *image)
{
	__GLEW_NOT_IMPL__
}

void __glewGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetHistogramParameteriv(GLenum target, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span)
{
	__GLEW_NOT_IMPL__
}

void __glewHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
	__GLEW_NOT_IMPL__
}

void __glewMinmax(GLenum target, GLenum internalformat, GLboolean sink)
{
	__GLEW_NOT_IMPL__
}

void __glewResetHistogram(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewResetMinmax(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawArraysInstancedARB(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawElementsInstancedARB(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribDivisorARB(GLuint index, GLuint divisor)
{
	__GLEW_NOT_IMPL__
}

void __glewFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	__GLEW_NOT_IMPL__
}

GLvoid * __glewMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

void __glewCurrentPaletteMatrixARB(GLint index)
{
	__GLEW_NOT_IMPL__
}

void __glewSampleCoverageARB(GLclampf value, GLboolean invert)
{
	__GLEW_NOT_IMPL__
}

void __glewActiveTextureARB(GLenum texture)
{
	__GLEW_NOT_IMPL__
}

void __glewClientActiveTextureARB(GLenum texture)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1dARB(GLenum target, GLdouble s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1dvARB(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1fARB(GLenum target, GLfloat s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1fvARB(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1iARB(GLenum target, GLint s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1ivARB(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1sARB(GLenum target, GLshort s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1svARB(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2dvARB(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2fvARB(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2iARB(GLenum target, GLint s, GLint t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2ivARB(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2svARB(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3dvARB(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3fvARB(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3ivARB(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3svARB(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4dvARB(GLenum target, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4fvARB(GLenum target, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4ivARB(GLenum target, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4svARB(GLenum target, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginQueryARB(GLenum target, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteQueriesARB(GLsizei n, const GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewEndQueryARB(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewGenQueriesARB(GLsizei n, GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjectivARB(GLuint id, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjectuivARB(GLuint id, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryivARB(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsQueryARB(GLuint id)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewPointParameterfARB(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameterfvARB(GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProvokingVertex(GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnColorTableARB(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* table)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnCompressedTexImageARB(GLenum target, GLint lod, GLsizei bufSize, void* img)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnConvolutionFilterARB(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* image)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnHistogramARB(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnMapdvARB(GLenum target, GLenum query, GLsizei bufSize, GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnMapfvARB(GLenum target, GLenum query, GLsizei bufSize, GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnMapivARB(GLenum target, GLenum query, GLsizei bufSize, GLint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnMinmaxARB(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnPixelMapfvARB(GLenum map, GLsizei bufSize, GLfloat* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnPixelMapuivARB(GLenum map, GLsizei bufSize, GLuint* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnPixelMapusvARB(GLenum map, GLsizei bufSize, GLushort* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnPolygonStippleARB(GLsizei bufSize, GLubyte* pattern)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnSeparableFilterARB(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void* row, GLsizei columnBufSize, GLvoid*column, GLvoid*span)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnTexImageARB(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* img)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnUniformdvARB(GLuint program, GLint location, GLsizei bufSize, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnUniformfvARB(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnUniformivARB(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetnUniformuivARB(GLuint program, GLint location, GLsizei bufSize, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewReadnPixelsARB(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewMinSampleShadingARB(GLclampf value)
{
	__GLEW_NOT_IMPL__
}

void __glewBindSampler(GLuint unit, GLuint sampler)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteSamplers(GLsizei count, const GLuint * samplers)
{
	__GLEW_NOT_IMPL__
}

void __glewGenSamplers(GLsizei count, GLuint* samplers)
{
	__GLEW_NOT_IMPL__
}

void __glewGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsSampler(GLuint sampler)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewSamplerParameteriv(GLuint sampler, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewActiveShaderProgram(GLuint pipeline, GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewBindProgramPipeline(GLuint pipeline)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewCreateShaderProgramv(GLenum type, GLsizei count, const char ** strings)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDeleteProgramPipelines(GLsizei n, const GLuint* pipelines)
{
	__GLEW_NOT_IMPL__
}

void __glewGenProgramPipelines(GLsizei n, GLuint* pipelines)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei* length, char *infoLog)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsProgramPipeline(GLuint pipeline)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewProgramUniform1d(GLuint program, GLint location, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1f(GLuint program, GLint location, GLfloat x)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1i(GLuint program, GLint location, GLint x)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1ui(GLuint program, GLint location, GLuint x)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2d(GLuint program, GLint location, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2f(GLuint program, GLint location, GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2i(GLuint program, GLint location, GLint x, GLint y)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2ui(GLuint program, GLint location, GLuint x, GLuint y)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3d(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3f(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3i(GLuint program, GLint location, GLint x, GLint y, GLint z)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3ui(GLuint program, GLint location, GLuint x, GLuint y, GLuint z)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4d(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4f(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4i(GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4ui(GLuint program, GLint location, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewValidateProgramPipeline(GLuint pipeline)
{
	__GLEW_NOT_IMPL__
}

void __glewAttachObjectARB(GLhandleARB containerObj, GLhandleARB obj)
{
	__GLEW_NOT_IMPL__
}

void __glewCompileShaderARB(GLhandleARB shaderObj)
{
	__GLEW_NOT_IMPL__
}

GLhandleARB __glewCreateProgramObjectARB(void)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLhandleARB __glewCreateShaderObjectARB(GLenum shaderType)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDeleteObjectARB(GLhandleARB obj)
{
	__GLEW_NOT_IMPL__
}

void __glewDetachObjectARB(GLhandleARB containerObj, GLhandleARB attachedObj)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveUniformARB(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint *size, GLenum *type, GLcharARB *name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetAttachedObjectsARB(GLhandleARB containerObj, GLsizei maxCount, GLsizei* count, GLhandleARB *obj)
{
	__GLEW_NOT_IMPL__
}

GLhandleARB __glewGetHandleARB(GLenum pname)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB *infoLog)
{
	__GLEW_NOT_IMPL__
}

void __glewGetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetShaderSourceARB(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB *source)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetUniformLocationARB(GLhandleARB programObj, const GLcharARB* name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetUniformfvARB(GLhandleARB programObj, GLint location, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetUniformivARB(GLhandleARB programObj, GLint location, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewLinkProgramARB(GLhandleARB programObj)
{
	__GLEW_NOT_IMPL__
}

void __glewShaderSourceARB(GLhandleARB shaderObj, GLsizei count, const GLcharARB ** string, const GLint *length)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1fARB(GLint location, GLfloat v0)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1fvARB(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1iARB(GLint location, GLint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1ivARB(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2fARB(GLint location, GLfloat v0, GLfloat v1)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2fvARB(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2iARB(GLint location, GLint v0, GLint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2ivARB(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3fvARB(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3iARB(GLint location, GLint v0, GLint v1, GLint v2)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3ivARB(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4fvARB(GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4iARB(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4ivARB(GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix2fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix3fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformMatrix4fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUseProgramObjectARB(GLhandleARB programObj)
{
	__GLEW_NOT_IMPL__
}

void __glewValidateProgramARB(GLhandleARB programObj)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, char *name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, char *name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint* values)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewGetSubroutineIndex(GLuint program, GLenum shadertype, const char* name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLint __glewGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const char* name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint* indices)
{
	__GLEW_NOT_IMPL__
}

void __glewCompileShaderIncludeARB(GLuint shader, GLsizei count, const char ** path, const GLint *length)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteNamedStringARB(GLint namelen, const char* name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedStringARB(GLint namelen, const char* name, GLsizei bufSize, GLint *stringlen, char *string)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedStringivARB(GLint namelen, const char* name, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsNamedStringARB(GLint namelen, const char* name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewNamedStringARB(GLenum type, GLint namelen, const char* name, GLint stringlen, const char *string)
{
	__GLEW_NOT_IMPL__
}

GLenum __glewClientWaitSync(GLsync GLsync,GLbitfield flags,GLuint64 timeout)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDeleteSync(GLsync GLsync)
{
	__GLEW_NOT_IMPL__
}

GLsync __glewFenceSync(GLenum condition,GLbitfield flags)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetInteger64v(GLenum pname, GLint64* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetSynciv(GLsync GLsync,GLenum pname,GLsizei bufSize,GLsizei* length, GLint *values)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsSync(GLsync GLsync)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewWaitSync(GLsync GLsync,GLbitfield flags,GLuint64 timeout)
{
	__GLEW_NOT_IMPL__
}

void __glewPatchParameterfv(GLenum pname, const GLfloat* values)
{
	__GLEW_NOT_IMPL__
}

void __glewPatchParameteri(GLenum pname, GLint value)
{
	__GLEW_NOT_IMPL__
}

void __glewTexBufferARB(GLenum target, GLenum internalformat, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexImage1DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexImage2DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexImage3DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexSubImage1DARB(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexSubImage2DARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTexSubImage3DARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCompressedTexImageARB(GLenum target, GLint lod, void* img)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultisamplefv(GLenum pname, GLuint index, GLfloat* val)
{
	__GLEW_NOT_IMPL__
}

void __glewSampleMaski(GLuint index, GLbitfield mask)
{
	__GLEW_NOT_IMPL__
}

void __glewTexImage2DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	__GLEW_NOT_IMPL__
}

void __glewTexImage3DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params)
{
	__GLEW_NOT_IMPL__
}

void __glewQueryCounter(GLuint id, GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewBindTransformFeedback(GLenum target, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteTransformFeedbacks(GLsizei n, const GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawTransformFeedback(GLenum mode, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewGenTransformFeedbacks(GLsizei n, GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsTransformFeedback(GLuint id)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewPauseTransformFeedback(void)
{
	__GLEW_NOT_IMPL__
}

void __glewResumeTransformFeedback(void)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginQueryIndexed(GLenum target, GLuint index, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream)
{
	__GLEW_NOT_IMPL__
}

void __glewEndQueryIndexed(GLenum target, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, char* uniformBlockName)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, char* uniformName)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewGetUniformBlockIndex(GLuint program, const char* uniformBlockName)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetUniformIndices(GLuint program, GLsizei uniformCount, const char** uniformNames, GLuint* uniformIndices)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	__GLEW_NOT_IMPL__
}

void __glewBindVertexArray(GLuint array)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
	__GLEW_NOT_IMPL__
}

void __glewGenVertexArrays(GLsizei n, GLuint* arrays)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsVertexArray(GLuint array)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1d(GLuint index, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2d(GLuint index, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4dv(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexBlendARB(GLint count)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferARB(GLenum target, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBufferDataARB(GLenum target, GLsizeiptrARB size, const GLvoid* data, GLenum usage)
{
	__GLEW_NOT_IMPL__
}

void __glewBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid* data)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteBuffersARB(GLsizei n, const GLuint* buffers)
{
	__GLEW_NOT_IMPL__
}

void __glewGenBuffersARB(GLsizei n, GLuint* buffers)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBufferParameterivARB(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBufferPointervARB(GLenum target, GLenum pname, GLvoid** params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid* data)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsBufferARB(GLuint buffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLvoid * __glewMapBufferARB(GLenum target, GLenum access)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

GLboolean __glewUnmapBufferARB(GLenum target)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewBindProgramARB(GLenum target, GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteProgramsARB(GLsizei n, const GLuint* programs)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableVertexAttribArrayARB(GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableVertexAttribArrayARB(GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewGenProgramsARB(GLsizei n, GLuint* programs)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramEnvParameterdvARB(GLenum target, GLuint index, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramEnvParameterfvARB(GLenum target, GLuint index, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramLocalParameterdvARB(GLenum target, GLuint index, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramLocalParameterfvARB(GLenum target, GLuint index, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramStringARB(GLenum target, GLenum pname, void* string)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramivARB(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribPointervARB(GLuint index, GLenum pname, GLvoid** pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribdvARB(GLuint index, GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribfvARB(GLuint index, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribivARB(GLuint index, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsProgramARB(GLuint program)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewProgramEnvParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParameter4dvARB(GLenum target, GLuint index, const GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParameter4fvARB(GLenum target, GLuint index, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramStringARB(GLenum target, GLenum format, GLsizei len, const void* string)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1dARB(GLuint index, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1dvARB(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1fARB(GLuint index, GLfloat x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1fvARB(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1sARB(GLuint index, GLshort x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1svARB(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2dARB(GLuint index, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2dvARB(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2fARB(GLuint index, GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2fvARB(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2sARB(GLuint index, GLshort x, GLshort y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2svARB(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3dvARB(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3fvARB(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3sARB(GLuint index, GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3svARB(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4NbvARB(GLuint index, const GLbyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4NivARB(GLuint index, const GLint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4NsvARB(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4NubARB(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4NubvARB(GLuint index, const GLubyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4NuivARB(GLuint index, const GLuint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4NusvARB(GLuint index, const GLushort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4bvARB(GLuint index, const GLbyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4dvARB(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4fvARB(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4ivARB(GLuint index, const GLint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4sARB(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4svARB(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4ubvARB(GLuint index, const GLubyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4uivARB(GLuint index, const GLuint* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4usvARB(GLuint index, const GLushort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribPointerARB(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewBindAttribLocationARB(GLhandleARB programObj, GLuint index, const GLcharARB* name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveAttribARB(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint *size, GLenum *type, GLcharARB *name)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetAttribLocationARB(GLhandleARB programObj, const GLcharARB* name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewColorP3ui(GLenum type, GLuint color)
{
	__GLEW_NOT_IMPL__
}

void __glewColorP3uiv(GLenum type, const GLuint* color)
{
	__GLEW_NOT_IMPL__
}

void __glewColorP4ui(GLenum type, GLuint color)
{
	__GLEW_NOT_IMPL__
}

void __glewColorP4uiv(GLenum type, const GLuint* color)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP1ui(GLenum texture, GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP1uiv(GLenum texture, GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP2ui(GLenum texture, GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP2uiv(GLenum texture, GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP3ui(GLenum texture, GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP3uiv(GLenum texture, GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP4ui(GLenum texture, GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordP4uiv(GLenum texture, GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalP3ui(GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalP3uiv(GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColorP3ui(GLenum type, GLuint color)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColorP3uiv(GLenum type, const GLuint* color)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP1ui(GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP1uiv(GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP2ui(GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP2uiv(GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP3ui(GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP3uiv(GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP4ui(GLenum type, GLuint coords)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordP4uiv(GLenum type, const GLuint* coords)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexP2ui(GLenum type, GLuint value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexP2uiv(GLenum type, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexP3ui(GLenum type, GLuint value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexP3uiv(GLenum type, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexP4ui(GLenum type, GLuint value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexP4uiv(GLenum type, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewDepthRangeArrayv(GLuint first, GLsizei count, const GLclampd * v)
{
	__GLEW_NOT_IMPL__
}

void __glewDepthRangeIndexed(GLuint index, GLclampd n, GLclampd f)
{
	__GLEW_NOT_IMPL__
}

void __glewGetDoublei_v(GLenum target, GLuint index, GLdouble* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFloati_v(GLenum target, GLuint index, GLfloat* data)
{
	__GLEW_NOT_IMPL__
}

void __glewScissorArrayv(GLuint first, GLsizei count, const GLint * v)
{
	__GLEW_NOT_IMPL__
}

void __glewScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewScissorIndexedv(GLuint index, const GLint * v)
{
	__GLEW_NOT_IMPL__
}

void __glewViewportArrayv(GLuint first, GLsizei count, const GLfloat * v)
{
	__GLEW_NOT_IMPL__
}

void __glewViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	__GLEW_NOT_IMPL__
}

void __glewViewportIndexedfv(GLuint index, const GLfloat * v)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2dARB(GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2dvARB(const GLdouble* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2fARB(GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2fvARB(const GLfloat* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2iARB(GLint x, GLint y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2ivARB(const GLint* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2sARB(GLshort x, GLshort y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2svARB(const GLshort* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3dARB(GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3dvARB(const GLdouble* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3fARB(GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3fvARB(const GLfloat* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3iARB(GLint x, GLint y, GLint z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3ivARB(const GLint* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3sARB(GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3svARB(const GLshort* p)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawBuffersATI(GLsizei n, const GLenum* bufs)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawElementArrayATI(GLenum mode, GLsizei count)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawRangeElementArrayATI(GLenum mode, GLuint start, GLuint end, GLsizei count)
{
	__GLEW_NOT_IMPL__
}

void __glewElementPointerATI(GLenum type, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTexBumpParameterfvATI(GLenum pname, GLfloat *param)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTexBumpParameterivATI(GLenum pname, GLint *param)
{
	__GLEW_NOT_IMPL__
}

void __glewAlphaFragmentOp1ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
	__GLEW_NOT_IMPL__
}

void __glewAlphaFragmentOp2ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
	__GLEW_NOT_IMPL__
}

void __glewAlphaFragmentOp3ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginFragmentShaderATI(void)
{
	__GLEW_NOT_IMPL__
}

void __glewBindFragmentShaderATI(GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewColorFragmentOp1ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
	__GLEW_NOT_IMPL__
}

void __glewColorFragmentOp2ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
	__GLEW_NOT_IMPL__
}

void __glewColorFragmentOp3ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteFragmentShaderATI(GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewEndFragmentShaderATI(void)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewGenFragmentShadersATI(GLuint range)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewPassTexCoordATI(GLuint dst, GLuint coord, GLenum swizzle)
{
	__GLEW_NOT_IMPL__
}

void __glewSampleMapATI(GLuint dst, GLuint interp, GLenum swizzle)
{
	__GLEW_NOT_IMPL__
}

void __glewSetFragmentShaderConstantATI(GLuint dst, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void* __glewMapObjectBufferATI(GLuint buffer)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

void __glewUnmapObjectBufferATI(GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewPNTrianglesfATI(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewPNTrianglesiATI(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewStencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
	__GLEW_NOT_IMPL__
}

void __glewStencilOpSeparateATI(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	__GLEW_NOT_IMPL__
}

void __glewArrayObjectATI(GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
	__GLEW_NOT_IMPL__
}

void __glewFreeObjectBufferATI(GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewGetArrayObjectfvATI(GLenum array, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetArrayObjectivATI(GLenum array, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetObjectBufferfvATI(GLuint buffer, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetObjectBufferivATI(GLuint buffer, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVariantArrayObjectfvATI(GLuint id, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVariantArrayObjectivATI(GLuint id, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsObjectBufferATI(GLuint buffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewNewObjectBufferATI(GLsizei size, const void* pointer, GLenum usage)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewUpdateObjectBufferATI(GLuint buffer, GLuint offset, GLsizei size, const void* pointer, GLenum preserve)
{
	__GLEW_NOT_IMPL__
}

void __glewVariantArrayObjectATI(GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribArrayObjectfvATI(GLuint index, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribArrayObjectivATI(GLuint index, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribArrayObjectATI(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset)
{
	__GLEW_NOT_IMPL__
}

void __glewClientActiveVertexStreamATI(GLenum stream)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3bATI(GLenum stream, GLbyte x, GLbyte y, GLbyte z)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3bvATI(GLenum stream, const GLbyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3dATI(GLenum stream, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3dvATI(GLenum stream, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3fATI(GLenum stream, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3fvATI(GLenum stream, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3iATI(GLenum stream, GLint x, GLint y, GLint z)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3ivATI(GLenum stream, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3sATI(GLenum stream, GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalStream3svATI(GLenum stream, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexBlendEnvfATI(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexBlendEnviATI(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2dATI(GLenum stream, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2dvATI(GLenum stream, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2fATI(GLenum stream, GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2fvATI(GLenum stream, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2iATI(GLenum stream, GLint x, GLint y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2ivATI(GLenum stream, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2sATI(GLenum stream, GLshort x, GLshort y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream2svATI(GLenum stream, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3dATI(GLenum stream, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3dvATI(GLenum stream, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3fATI(GLenum stream, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3fvATI(GLenum stream, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3iATI(GLenum stream, GLint x, GLint y, GLint z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3ivATI(GLenum stream, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3sATI(GLenum stream, GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream3svATI(GLenum stream, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4dATI(GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4dvATI(GLenum stream, const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4fATI(GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4fvATI(GLenum stream, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4iATI(GLenum stream, GLint x, GLint y, GLint z, GLint w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4ivATI(GLenum stream, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4sATI(GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexStream4svATI(GLenum stream, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetUniformBufferSizeEXT(GLuint program, GLint location)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLintptr __glewGetUniformOffsetEXT(GLuint program, GLint location)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewUniformBufferEXT(GLuint program, GLint location, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendColorEXT(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationSeparateEXT(GLenum modeRGB, GLenum modeAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendFuncSeparateEXT(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	__GLEW_NOT_IMPL__
}

void __glewBlendEquationEXT(GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewColorSubTableEXT(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyColorSubTableEXT(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewLockArraysEXT(GLint first, GLsizei count)
{
	__GLEW_NOT_IMPL__
}

void __glewUnlockArraysEXT(void)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionFilter1DEXT(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* image)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionFilter2DEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* image)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameterfEXT(GLenum target, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameterfvEXT(GLenum target, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameteriEXT(GLenum target, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewConvolutionParameterivEXT(GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyConvolutionFilter1DEXT(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyConvolutionFilter2DEXT(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewGetConvolutionFilterEXT(GLenum target, GLenum format, GLenum type, void* image)
{
	__GLEW_NOT_IMPL__
}

void __glewGetConvolutionParameterfvEXT(GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetConvolutionParameterivEXT(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetSeparableFilterEXT(GLenum target, GLenum format, GLenum type, void* row, void* column, void* span)
{
	__GLEW_NOT_IMPL__
}

void __glewSeparableFilter2DEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* row, const void* column)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTexImage1DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTexImage2DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewCullParameterdvEXT(GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewCullParameterfvEXT(GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewDepthBoundsEXT(GLclampd zmin, GLclampd zmax)
{
	__GLEW_NOT_IMPL__
}

void __glewBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
	__GLEW_NOT_IMPL__
}

GLenum __glewCheckNamedFramebufferStatusEXT(GLuint framebuffer, GLenum target)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewClientAttribDefaultEXT(GLbitfield mask)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCompressedTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableClientStateIndexedEXT(GLenum array, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableClientStateiEXT(GLenum array, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableVertexArrayAttribEXT(GLuint vaobj, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableVertexArrayEXT(GLuint vaobj, GLenum array)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableClientStateIndexedEXT(GLenum array, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableClientStateiEXT(GLenum array, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableVertexArrayAttribEXT(GLuint vaobj, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableVertexArrayEXT(GLuint vaobj, GLenum array)
{
	__GLEW_NOT_IMPL__
}

void __glewFlushMappedNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferDrawBufferEXT(GLuint framebuffer, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferDrawBuffersEXT(GLuint framebuffer, GLsizei n, const GLenum* bufs)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferReadBufferEXT(GLuint framebuffer, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewGenerateMultiTexMipmapEXT(GLenum texunit, GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewGenerateTextureMipmapEXT(GLuint texture, GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCompressedMultiTexImageEXT(GLenum texunit, GLenum target, GLint level, void* img)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCompressedTextureImageEXT(GLuint texture, GLenum target, GLint level, void* img)
{
	__GLEW_NOT_IMPL__
}

void __glewGetDoubleIndexedvEXT(GLenum target, GLuint index, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetDoublei_vEXT(GLenum pname, GLuint index, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFloatIndexedvEXT(GLenum target, GLuint index, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFloati_vEXT(GLenum pname, GLuint index, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* param)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexImageEXT(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexLevelParameterfvEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexLevelParameterivEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedBufferParameterivEXT(GLuint buffer, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedBufferPointervEXT(GLuint buffer, GLenum pname, void** params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedFramebufferAttachmentParameterivEXT(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedProgramLocalParameterIivEXT(GLuint program, GLenum target, GLuint index, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedProgramLocalParameterIuivEXT(GLuint program, GLenum target, GLuint index, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedProgramLocalParameterdvEXT(GLuint program, GLenum target, GLuint index, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedProgramLocalParameterfvEXT(GLuint program, GLenum target, GLuint index, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedProgramStringEXT(GLuint program, GLenum target, GLenum pname, void* string)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedProgramivEXT(GLuint program, GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedRenderbufferParameterivEXT(GLuint renderbuffer, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPointerIndexedvEXT(GLenum target, GLuint index, GLvoid** params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPointeri_vEXT(GLenum pname, GLuint index, GLvoid** params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTextureImageEXT(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTextureLevelParameterfvEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTextureLevelParameterivEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexArrayIntegeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, GLint* param)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexArrayIntegervEXT(GLuint vaobj, GLenum pname, GLint* param)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexArrayPointeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, GLvoid** param)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexArrayPointervEXT(GLuint vaobj, GLenum pname, GLvoid** param)
{
	__GLEW_NOT_IMPL__
}

GLvoid * __glewMapNamedBufferEXT(GLuint buffer, GLenum access)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

GLvoid * __glewMapNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	__GLEW_NOT_IMPL__
	return NULL;
}

void __glewMatrixFrustumEXT(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixLoadIdentityEXT(GLenum matrixMode)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixLoadTransposedEXT(GLenum matrixMode, const GLdouble* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixLoadTransposefEXT(GLenum matrixMode, const GLfloat* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixLoaddEXT(GLenum matrixMode, const GLdouble* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixLoadfEXT(GLenum matrixMode, const GLfloat* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixMultTransposedEXT(GLenum matrixMode, const GLdouble* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixMultTransposefEXT(GLenum matrixMode, const GLfloat* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixMultdEXT(GLenum matrixMode, const GLdouble* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixMultfEXT(GLenum matrixMode, const GLfloat* m)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixOrthoEXT(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixPopEXT(GLenum matrixMode)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixPushEXT(GLenum matrixMode)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixRotatedEXT(GLenum matrixMode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixRotatefEXT(GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixScaledEXT(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixScalefEXT(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixTranslatedEXT(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewMatrixTranslatefEXT(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexBufferEXT(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoordPointerEXT(GLenum texunit, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexEnvfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexEnviEXT(GLenum texunit, GLenum target, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexGendEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexGenfEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexGeniEXT(GLenum texunit, GLenum coord, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, const GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexParameterfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexParameteriEXT(GLenum texunit, GLenum target, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* param)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexRenderbufferEXT(GLenum texunit, GLenum target, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedBufferDataEXT(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedCopyBufferSubDataEXT(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedFramebufferRenderbufferEXT(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedFramebufferTexture1DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedFramebufferTexture2DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedFramebufferTexture3DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedFramebufferTextureEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedFramebufferTextureFaceEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedFramebufferTextureLayerEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameter4dEXT(GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameter4dvEXT(GLuint program, GLenum target, GLuint index, const GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameter4fEXT(GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameter4fvEXT(GLuint program, GLenum target, GLuint index, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameterI4iEXT(GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameterI4ivEXT(GLuint program, GLenum target, GLuint index, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameterI4uiEXT(GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameterI4uivEXT(GLuint program, GLenum target, GLuint index, const GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParameters4fvEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParametersI4ivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramLocalParametersI4uivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedProgramStringEXT(GLuint program, GLenum target, GLenum format, GLsizei len, const void* string)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedRenderbufferStorageEXT(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedRenderbufferStorageMultisampleCoverageEXT(GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewNamedRenderbufferStorageMultisampleEXT(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1fEXT(GLuint program, GLint location, GLfloat v0)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1iEXT(GLuint program, GLint location, GLint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1uiEXT(GLuint program, GLint location, GLuint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2iEXT(GLuint program, GLint location, GLint v0, GLint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix2x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix3x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformMatrix4x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	__GLEW_NOT_IMPL__
}

void __glewPushClientAttribDefaultEXT(GLbitfield mask)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureBufferEXT(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, const GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureParameterfEXT(GLuint texture, GLenum target, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, const GLfloat* param)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureParameteriEXT(GLuint texture, GLenum target, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* param)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureRenderbufferEXT(GLuint texture, GLenum target, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewUnmapNamedBufferEXT(GLuint buffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewVertexArrayColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayEdgeFlagOffsetEXT(GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayFogCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayIndexOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayMultiTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayNormalOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArraySecondaryColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayVertexAttribIOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayVertexAttribOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayVertexOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewColorMaskIndexedEXT(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableIndexedEXT(GLenum target, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableIndexedEXT(GLenum target, GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewGetBooleanIndexedvEXT(GLenum value, GLuint index, GLboolean* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetIntegerIndexedvEXT(GLenum value, GLuint index, GLint* data)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsEnabledIndexedEXT(GLenum target, GLuint index)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDrawArraysInstancedEXT(GLenum mode, GLint start, GLsizei count, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawElementsInstancedEXT(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawRangeElementsEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordPointerEXT(GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoorddEXT(GLdouble coord)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoorddvEXT(const GLdouble *coord)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordfEXT(GLfloat coord)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordfvEXT(const GLfloat *coord)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentColorMaterialEXT(GLenum face, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightModelfEXT(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightModelfvEXT(GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightModeliEXT(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightModelivEXT(GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightfEXT(GLenum light, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightfvEXT(GLenum light, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightiEXT(GLenum light, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightivEXT(GLenum light, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialfEXT(GLenum face, GLenum pname, const GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialfvEXT(GLenum face, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialiEXT(GLenum face, GLenum pname, const GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialivEXT(GLenum face, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFragmentLightfvEXT(GLenum light, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFragmentLightivEXT(GLenum light, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFragmentMaterialfvEXT(GLenum face, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFragmentMaterialivEXT(GLenum face, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewLightEnviEXT(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewBlitFramebufferEXT(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	__GLEW_NOT_IMPL__
}

void __glewRenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewBindFramebufferEXT(GLenum target, GLuint framebuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBindRenderbufferEXT(GLenum target, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

GLenum __glewCheckFramebufferStatusEXT(GLenum target)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDeleteFramebuffersEXT(GLsizei n, const GLuint* framebuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteRenderbuffersEXT(GLsizei n, const GLuint* renderbuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	__GLEW_NOT_IMPL__
}

void __glewGenFramebuffersEXT(GLsizei n, GLuint* framebuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewGenRenderbuffersEXT(GLsizei n, GLuint* renderbuffers)
{
	__GLEW_NOT_IMPL__
}

void __glewGenerateMipmapEXT(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsFramebufferEXT(GLuint framebuffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLboolean __glewIsRenderbufferEXT(GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewRenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTextureFaceEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameteriEXT(GLuint program, GLenum pname, GLint value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewBindFragDataLocationEXT(GLuint program, GLuint color, const GLchar *name)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetFragDataLocationEXT(GLuint program, const GLchar *name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetUniformuivEXT(GLuint program, GLint location, GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribIivEXT(GLuint index, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribIuivEXT(GLuint index, GLenum pname, GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1uiEXT(GLint location, GLuint v0)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1uivEXT(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2uiEXT(GLint location, GLuint v0, GLuint v1)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2uivEXT(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3uivEXT(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4uivEXT(GLint location, GLsizei count, const GLuint *value)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1iEXT(GLuint index, GLint x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1ivEXT(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1uiEXT(GLuint index, GLuint x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI1uivEXT(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2iEXT(GLuint index, GLint x, GLint y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2ivEXT(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI2uivEXT(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3ivEXT(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI3uivEXT(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4bvEXT(GLuint index, const GLbyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4ivEXT(GLuint index, const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4svEXT(GLuint index, const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4ubvEXT(GLuint index, const GLubyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4uivEXT(GLuint index, const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribI4usvEXT(GLuint index, const GLushort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribIPointerEXT(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewGetHistogramEXT(GLenum target, GLboolean reset, GLenum format, GLenum type, void* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetHistogramParameterfvEXT(GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetHistogramParameterivEXT(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMinmaxEXT(GLenum target, GLboolean reset, GLenum format, GLenum type, void* values)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMinmaxParameterfvEXT(GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMinmaxParameterivEXT(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewHistogramEXT(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
	__GLEW_NOT_IMPL__
}

void __glewMinmaxEXT(GLenum target, GLenum internalformat, GLboolean sink)
{
	__GLEW_NOT_IMPL__
}

void __glewResetHistogramEXT(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewResetMinmaxEXT(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewIndexFuncEXT(GLenum func, GLfloat ref)
{
	__GLEW_NOT_IMPL__
}

void __glewIndexMaterialEXT(GLenum face, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewApplyTextureEXT(GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureLightEXT(GLenum pname)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureMaterialEXT(GLenum face, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiDrawArraysEXT(GLenum mode, const GLint* first, const GLsizei *count, GLsizei primcount)
{
	__GLEW_NOT_IMPL__
}

void __glewSampleMaskEXT(GLclampf value, GLboolean invert)
{
	__GLEW_NOT_IMPL__
}

void __glewSamplePatternEXT(GLenum pattern)
{
	__GLEW_NOT_IMPL__
}

void __glewColorTableEXT(GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableEXT(GLenum target, GLenum format, GLenum type, void* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableParameterfvEXT(GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableParameterivEXT(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPixelTransformParameterfvEXT(GLenum target, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetPixelTransformParameterivEXT(GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewPixelTransformParameterfEXT(GLenum target, GLenum pname, const GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewPixelTransformParameterfvEXT(GLenum target, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewPixelTransformParameteriEXT(GLenum target, GLenum pname, const GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewPixelTransformParameterivEXT(GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameterfEXT(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameterfvEXT(GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewPolygonOffsetEXT(GLfloat factor, GLfloat bias)
{
	__GLEW_NOT_IMPL__
}

void __glewProvokingVertexEXT(GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginSceneEXT(void)
{
	__GLEW_NOT_IMPL__
}

void __glewEndSceneEXT(void)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3bEXT(GLbyte red, GLbyte green, GLbyte blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3bvEXT(const GLbyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3dEXT(GLdouble red, GLdouble green, GLdouble blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3dvEXT(const GLdouble *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3fEXT(GLfloat red, GLfloat green, GLfloat blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3fvEXT(const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3iEXT(GLint red, GLint green, GLint blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3ivEXT(const GLint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3sEXT(GLshort red, GLshort green, GLshort blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3svEXT(const GLshort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3ubEXT(GLubyte red, GLubyte green, GLubyte blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3ubvEXT(const GLubyte *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3uiEXT(GLuint red, GLuint green, GLuint blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3uivEXT(const GLuint *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3usEXT(GLushort red, GLushort green, GLushort blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3usvEXT(const GLushort *v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColorPointerEXT(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewActiveProgramEXT(GLuint program)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewCreateShaderProgramEXT(GLenum type, const char* string)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewUseShaderProgramEXT(GLenum type, GLuint program)
{
	__GLEW_NOT_IMPL__
}

void __glewBindImageTextureEXT(GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format)
{
	__GLEW_NOT_IMPL__
}

void __glewMemoryBarrierEXT(GLbitfield barriers)
{
	__GLEW_NOT_IMPL__
}

void __glewActiveStencilFaceEXT(GLenum face)
{
	__GLEW_NOT_IMPL__
}

void __glewTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewTexImage3DEXT(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewFramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	__GLEW_NOT_IMPL__
}

void __glewTexBufferEXT(GLenum target, GLenum internalformat, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewClearColorIiEXT(GLint red, GLint green, GLint blue, GLint alpha)
{
	__GLEW_NOT_IMPL__
}

void __glewClearColorIuiEXT(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTexParameterIivEXT(GLenum target, GLenum pname, GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewTexParameterIivEXT(GLenum target, GLenum pname, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewTexParameterIuivEXT(GLenum target, GLenum pname, const GLuint *params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewAreTexturesResidentEXT(GLsizei n, const GLuint* textures, GLboolean* residences)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewBindTextureEXT(GLenum target, GLuint texture)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteTexturesEXT(GLsizei n, const GLuint* textures)
{
	__GLEW_NOT_IMPL__
}

void __glewGenTexturesEXT(GLsizei n, GLuint* textures)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsTextureEXT(GLuint texture)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewPrioritizeTexturesEXT(GLsizei n, const GLuint* textures, const GLclampf* priorities)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureNormalEXT(GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64EXT *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64EXT *params)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginTransformFeedbackEXT(GLenum primitiveMode)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferBaseEXT(GLenum target, GLuint index, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferOffsetEXT(GLenum target, GLuint index, GLuint buffer, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferRangeEXT(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	__GLEW_NOT_IMPL__
}

void __glewEndTransformFeedbackEXT(void)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTransformFeedbackVaryingEXT(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei *size, GLenum *type, char *name)
{
	__GLEW_NOT_IMPL__
}

void __glewTransformFeedbackVaryingsEXT(GLuint program, GLsizei count, const char ** varyings, GLenum bufferMode)
{
	__GLEW_NOT_IMPL__
}

void __glewArrayElementEXT(GLint i)
{
	__GLEW_NOT_IMPL__
}

void __glewColorPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawArraysEXT(GLenum mode, GLint first, GLsizei count)
{
	__GLEW_NOT_IMPL__
}

void __glewEdgeFlagPointerEXT(GLsizei stride, GLsizei count, const GLboolean* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewIndexPointerEXT(GLenum type, GLsizei stride, GLsizei count, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalPointerEXT(GLenum type, GLsizei stride, GLsizei count, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribLdvEXT(GLuint index, GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexArrayVertexAttribLOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1dEXT(GLuint index, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1dvEXT(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2dEXT(GLuint index, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2dvEXT(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3dEXT(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3dvEXT(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4dEXT(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4dvEXT(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribLPointerEXT(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginVertexShaderEXT(void)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewBindLightParameterEXT(GLenum light, GLenum value)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewBindMaterialParameterEXT(GLenum face, GLenum value)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewBindParameterEXT(GLenum value)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewBindTexGenParameterEXT(GLenum unit, GLenum coord, GLenum value)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewBindTextureUnitParameterEXT(GLenum unit, GLenum value)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewBindVertexShaderEXT(GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteVertexShaderEXT(GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDisableVariantClientStateEXT(GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewEnableVariantClientStateEXT(GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewEndVertexShaderEXT(void)
{
	__GLEW_NOT_IMPL__
}

void __glewExtractComponentEXT(GLuint res, GLuint src, GLuint num)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewGenSymbolsEXT(GLenum dataType, GLenum storageType, GLenum range, GLuint components)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewGenVertexShadersEXT(GLuint range)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewGetInvariantBooleanvEXT(GLuint id, GLenum value, GLboolean *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetInvariantFloatvEXT(GLuint id, GLenum value, GLfloat *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetInvariantIntegervEXT(GLuint id, GLenum value, GLint *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetLocalConstantBooleanvEXT(GLuint id, GLenum value, GLboolean *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetLocalConstantFloatvEXT(GLuint id, GLenum value, GLfloat *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetLocalConstantIntegervEXT(GLuint id, GLenum value, GLint *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVariantBooleanvEXT(GLuint id, GLenum value, GLboolean *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVariantFloatvEXT(GLuint id, GLenum value, GLfloat *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVariantIntegervEXT(GLuint id, GLenum value, GLint *data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVariantPointervEXT(GLuint id, GLenum value, GLvoid **data)
{
	__GLEW_NOT_IMPL__
}

void __glewInsertComponentEXT(GLuint res, GLuint src, GLuint num)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsVariantEnabledEXT(GLuint id, GLenum cap)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewShaderOp1EXT(GLenum op, GLuint res, GLuint arg1)
{
	__GLEW_NOT_IMPL__
}

void __glewShaderOp2EXT(GLenum op, GLuint res, GLuint arg1, GLuint arg2)
{
	__GLEW_NOT_IMPL__
}

void __glewShaderOp3EXT(GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3)
{
	__GLEW_NOT_IMPL__
}

void __glewSwizzleEXT(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)
{
	__GLEW_NOT_IMPL__
}

void __glewWriteMaskEXT(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexWeightfEXT(GLfloat weight)
{
	__GLEW_NOT_IMPL__
}

GLsync __glewImportSyncEXT(GLenum external_sync_type, GLintptr external_sync, GLbitfield flags)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewFrameTerminatorGREMEDY(void)
{
	__GLEW_NOT_IMPL__
}

void __glewStringMarkerGREMEDY(GLsizei len, const void* string)
{
	__GLEW_NOT_IMPL__
}

void __glewImageTransformParameterfHP(GLenum target, GLenum pname, const GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewImageTransformParameterfvHP(GLenum target, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewImageTransformParameteriHP(GLenum target, GLenum pname, const GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewImageTransformParameterivHP(GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiModeDrawArraysIBM(const GLenum* mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiModeDrawElementsIBM(const GLenum* mode, const GLsizei *count, GLenum type, const GLvoid * const *indices, GLsizei primcount, GLint modestride)
{
	__GLEW_NOT_IMPL__
}

void __glewColorPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewEdgeFlagPointerListIBM(GLint stride, const GLboolean ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordPointerListIBM(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewIndexPointerListIBM(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalPointerListIBM(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColorPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride)
{
	__GLEW_NOT_IMPL__
}

void __glewColorPointervINTEL(GLint size, GLenum type, const void** pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalPointervINTEL(GLenum type, const void** pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordPointervINTEL(GLint size, GLenum type, const void** pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexPointervINTEL(GLint size, GLenum type, const void** pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewTexScissorFuncINTEL(GLenum target, GLenum lfunc, GLenum hfunc)
{
	__GLEW_NOT_IMPL__
}

void __glewTexScissorINTEL(GLenum target, GLclampf tlow, GLclampf thigh)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewBufferRegionEnabledEXT(void)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewDeleteBufferRegionEXT(GLenum region)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawBufferRegionEXT(GLuint region, GLint x, GLint y, GLsizei width, GLsizei height, GLint xDest, GLint yDest)
{
	__GLEW_NOT_IMPL__
}

GLuint __glewNewBufferRegionEXT(GLenum region)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewReadBufferRegionEXT(GLuint region, GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}

void __glewResizeBuffersMESA(void)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2dMESA(GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2dvMESA(const GLdouble* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2fMESA(GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2fvMESA(const GLfloat* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2iMESA(GLint x, GLint y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2ivMESA(const GLint* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2sMESA(GLshort x, GLshort y)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos2svMESA(const GLshort* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3dMESA(GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3dvMESA(const GLdouble* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3fMESA(GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3fvMESA(const GLfloat* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3iMESA(GLint x, GLint y, GLint z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3ivMESA(const GLint* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3sMESA(GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos3svMESA(const GLshort* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4dMESA(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4dvMESA(const GLdouble* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4fMESA(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4fvMESA(const GLfloat* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4iMESA(GLint x, GLint y, GLint z, GLint w)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4ivMESA(const GLint* p)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4sMESA(GLshort x, GLshort y, GLshort z, GLshort w)
{
	__GLEW_NOT_IMPL__
}

void __glewWindowPos4svMESA(const GLshort* p)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginConditionalRenderNV(GLuint id, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewEndConditionalRenderNV(void)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyImageSubDataNV(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth)
{
	__GLEW_NOT_IMPL__
}

void __glewClearDepthdNV(GLdouble depth)
{
	__GLEW_NOT_IMPL__
}

void __glewDepthBoundsdNV(GLdouble zmin, GLdouble zmax)
{
	__GLEW_NOT_IMPL__
}

void __glewDepthRangedNV(GLdouble zNear, GLdouble zFar)
{
	__GLEW_NOT_IMPL__
}

void __glewEvalMapsNV(GLenum target, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMapAttribParameterfvNV(GLenum target, GLuint index, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMapAttribParameterivNV(GLenum target, GLuint index, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMapControlPointsNV(GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, void* points)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMapParameterfvNV(GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMapParameterivNV(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMapControlPointsNV(GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const void* points)
{
	__GLEW_NOT_IMPL__
}

void __glewMapParameterfvNV(GLenum target, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewMapParameterivNV(GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetMultisamplefvNV(GLenum pname, GLuint index, GLfloat* val)
{
	__GLEW_NOT_IMPL__
}

void __glewSampleMaskIndexedNV(GLuint index, GLbitfield mask)
{
	__GLEW_NOT_IMPL__
}

void __glewTexRenderbufferNV(GLenum target, GLuint renderbuffer)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteFencesNV(GLsizei n, const GLuint* fences)
{
	__GLEW_NOT_IMPL__
}

void __glewFinishFenceNV(GLuint fence)
{
	__GLEW_NOT_IMPL__
}

void __glewGenFencesNV(GLsizei n, GLuint* fences)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFenceivNV(GLuint fence, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsFenceNV(GLuint fence)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewSetFenceNV(GLuint fence, GLenum condition)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewTestFenceNV(GLuint fence)
{
	__GLEW_NOT_IMPL__
	return 0;
}


void __glewGetProgramNamedParameterdvNV(GLuint id, GLsizei len, const GLubyte* name, GLdouble *params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramNamedParameterfvNV(GLuint id, GLsizei len, const GLubyte* name, GLfloat *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramNamedParameter4dNV(GLuint id, GLsizei len, const GLubyte* name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramNamedParameter4dvNV(GLuint id, GLsizei len, const GLubyte* name, const GLdouble v[])
{
	__GLEW_NOT_IMPL__
}

void __glewProgramNamedParameter4fNV(GLuint id, GLsizei len, const GLubyte* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramNamedParameter4fvNV(GLuint id, GLsizei len, const GLubyte* name, const GLfloat v[])
{
	__GLEW_NOT_IMPL__
}


void __glewRenderbufferStorageMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}


void __glewProgramVertexLimitNV(GLenum target, GLint limit)
{
	__GLEW_NOT_IMPL__
}


void __glewProgramEnvParameterI4iNV(GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParameterI4ivNV(GLenum target, GLuint index, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParameterI4uiNV(GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParameterI4uivNV(GLenum target, GLuint index, const GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParametersI4ivNV(GLenum target, GLuint index, GLsizei count, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramEnvParametersI4uivNV(GLenum target, GLuint index, GLsizei count, const GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameterI4iNV(GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameterI4ivNV(GLenum target, GLuint index, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameterI4uiNV(GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParameterI4uivNV(GLenum target, GLuint index, const GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParametersI4ivNV(GLenum target, GLuint index, GLsizei count, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramLocalParametersI4uivNV(GLenum target, GLuint index, GLsizei count, const GLuint *params)
{
	__GLEW_NOT_IMPL__
}


void __glewGetUniformi64vNV(GLuint program, GLint location, GLint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetUniformui64vNV(GLuint program, GLint location, GLuint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1i64NV(GLuint program, GLint location, GLint64EXT x)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1ui64NV(GLuint program, GLint location, GLuint64EXT x)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform1ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform2ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform3ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniform4ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1i64NV(GLint location, GLint64EXT x)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1ui64NV(GLint location, GLuint64EXT x)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform1ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2i64NV(GLint location, GLint64EXT x, GLint64EXT y)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform2ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform3ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	__GLEW_NOT_IMPL__
}

void __glewUniform4ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

typedef unsigned short GLhalf;

void __glewColor3hNV(GLhalf red, GLhalf green, GLhalf blue)
{
	__GLEW_NOT_IMPL__
}

void __glewColor3hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4hNV(GLhalf red, GLhalf green, GLhalf blue, GLhalf alpha)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordhNV(GLhalf fog)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordhvNV(const GLhalf* fog)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1hNV(GLenum target, GLhalf s)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord1hvNV(GLenum target, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2hNV(GLenum target, GLhalf s, GLhalf t)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord2hvNV(GLenum target, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3hNV(GLenum target, GLhalf s, GLhalf t, GLhalf r)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord3hvNV(GLenum target, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4hNV(GLenum target, GLhalf s, GLhalf t, GLhalf r, GLhalf q)
{
	__GLEW_NOT_IMPL__
}

void __glewMultiTexCoord4hvNV(GLenum target, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewNormal3hNV(GLhalf nx, GLhalf ny, GLhalf nz)
{
	__GLEW_NOT_IMPL__
}

void __glewNormal3hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3hNV(GLhalf red, GLhalf green, GLhalf blue)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColor3hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord1hNV(GLhalf s)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord1hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2hNV(GLhalf s, GLhalf t)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord3hNV(GLhalf s, GLhalf t, GLhalf r)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord3hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord4hNV(GLhalf s, GLhalf t, GLhalf r, GLhalf q)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord4hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertex2hNV(GLhalf x, GLhalf y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertex2hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertex3hNV(GLhalf x, GLhalf y, GLhalf z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertex3hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertex4hNV(GLhalf x, GLhalf y, GLhalf z, GLhalf w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertex4hvNV(const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1hNV(GLuint index, GLhalf x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1hvNV(GLuint index, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2hNV(GLuint index, GLhalf x, GLhalf y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2hvNV(GLuint index, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3hNV(GLuint index, GLhalf x, GLhalf y, GLhalf z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3hvNV(GLuint index, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4hNV(GLuint index, GLhalf x, GLhalf y, GLhalf z, GLhalf w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4hvNV(GLuint index, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs1hvNV(GLuint index, GLsizei n, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs2hvNV(GLuint index, GLsizei n, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs3hvNV(GLuint index, GLsizei n, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs4hvNV(GLuint index, GLsizei n, const GLhalf* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexWeighthNV(GLhalf weight)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexWeighthvNV(const GLhalf* weight)
{
	__GLEW_NOT_IMPL__
}


void __glewBeginOcclusionQueryNV(GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteOcclusionQueriesNV(GLsizei n, const GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewEndOcclusionQueryNV(void)
{
	__GLEW_NOT_IMPL__
}

void __glewGenOcclusionQueriesNV(GLsizei n, GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewGetOcclusionQueryivNV(GLuint id, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetOcclusionQueryuivNV(GLuint id, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsOcclusionQueryNV(GLuint id)
{
	__GLEW_NOT_IMPL__
	return 0;
}


void __glewProgramBufferParametersIivNV(GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramBufferParametersIuivNV(GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLuint *params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramBufferParametersfvNV(GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLfloat *params)
{
	__GLEW_NOT_IMPL__
}


void __glewFlushPixelDataRangeNV(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewPixelDataRangeNV(GLenum target, GLsizei length, void* pointer)
{
	__GLEW_NOT_IMPL__
}


void __glewPointParameteriNV(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewPointParameterivNV(GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}


void __glewGetVideoi64vNV(GLuint video_slot, GLenum pname, GLint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVideoivNV(GLuint video_slot, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVideoui64vNV(GLuint video_slot, GLenum pname, GLuint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVideouivNV(GLuint video_slot, GLenum pname, GLuint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewPresentFrameDualFillNV(GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3)
{
	__GLEW_NOT_IMPL__
}

void __glewPresentFrameKeyedNV(GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1)
{
	__GLEW_NOT_IMPL__
}


void __glewPrimitiveRestartIndexNV(GLuint index)
{
	__GLEW_NOT_IMPL__
}

void __glewPrimitiveRestartNV(void)
{
	__GLEW_NOT_IMPL__
}


void __glewCombinerInputNV(GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)
{
	__GLEW_NOT_IMPL__
}

void __glewCombinerOutputNV(GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum)
{
	__GLEW_NOT_IMPL__
}

void __glewCombinerParameterfNV(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewCombinerParameterfvNV(GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewCombinerParameteriNV(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewCombinerParameterivNV(GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewFinalCombinerInputNV(GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCombinerInputParameterfvNV(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCombinerInputParameterivNV(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCombinerOutputParameterfvNV(GLenum stage, GLenum portion, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCombinerOutputParameterivNV(GLenum stage, GLenum portion, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFinalCombinerInputParameterfvNV(GLenum variable, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFinalCombinerInputParameterivNV(GLenum variable, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}


void __glewCombinerStageParameterfvNV(GLenum stage, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetCombinerStageParameterfvNV(GLenum stage, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}


void __glewGetBufferParameterui64vNV(GLenum target, GLenum pname, GLuint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetIntegerui64vNV(GLenum value, GLuint64EXT* result)
{
	__GLEW_NOT_IMPL__
}

void __glewGetNamedBufferParameterui64vNV(GLuint buffer, GLenum pname, GLuint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsBufferResidentNV(GLenum target)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLboolean __glewIsNamedBufferResidentNV(GLuint buffer)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewMakeBufferNonResidentNV(GLenum target)
{
	__GLEW_NOT_IMPL__
}

void __glewMakeBufferResidentNV(GLenum target, GLenum access)
{
	__GLEW_NOT_IMPL__
}

void __glewMakeNamedBufferNonResidentNV(GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewMakeNamedBufferResidentNV(GLuint buffer, GLenum access)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformui64NV(GLuint program, GLint location, GLuint64EXT value)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramUniformui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformui64NV(GLint location, GLuint64EXT value)
{
	__GLEW_NOT_IMPL__
}

void __glewUniformui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	__GLEW_NOT_IMPL__
}


void __glewTextureBarrierNV(void)
{
	__GLEW_NOT_IMPL__
}


void __glewTexImage2DMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
	__GLEW_NOT_IMPL__
}

void __glewTexImage3DMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureImage2DMultisampleCoverageNV(GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureImage2DMultisampleNV(GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureImage3DMultisampleCoverageNV(GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
	__GLEW_NOT_IMPL__
}

void __glewTextureImage3DMultisampleNV(GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
	__GLEW_NOT_IMPL__
}


void __glewActiveVaryingNV(GLuint program, const GLchar *name)
{
	__GLEW_NOT_IMPL__
}

void __glewBeginTransformFeedbackNV(GLenum primitiveMode)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferBaseNV(GLenum target, GLuint index, GLuint buffer)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferOffsetNV(GLenum target, GLuint index, GLuint buffer, GLintptr offset)
{
	__GLEW_NOT_IMPL__
}

void __glewBindBufferRangeNV(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	__GLEW_NOT_IMPL__
}

void __glewEndTransformFeedbackNV(void)
{
	__GLEW_NOT_IMPL__
}

void __glewGetActiveVaryingNV(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTransformFeedbackVaryingNV(GLuint program, GLuint index, GLint *location)
{
	__GLEW_NOT_IMPL__
}

GLint __glewGetVaryingLocationNV(GLuint program, const GLchar *name)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewTransformFeedbackAttribsNV(GLuint count, const GLint *attribs, GLenum bufferMode)
{
	__GLEW_NOT_IMPL__
}

void __glewTransformFeedbackVaryingsNV(GLuint program, GLsizei count, const GLint *locations, GLenum bufferMode)
{
	__GLEW_NOT_IMPL__
}


void __glewBindTransformFeedbackNV(GLenum target, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteTransformFeedbacksNV(GLsizei n, const GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewDrawTransformFeedbackNV(GLenum mode, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewGenTransformFeedbacksNV(GLsizei n, GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsTransformFeedbackNV(GLuint id)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewPauseTransformFeedbackNV(void)
{
	__GLEW_NOT_IMPL__
}

void __glewResumeTransformFeedbackNV(void)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUFiniNV(void)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUGetSurfaceivNV(GLvdpauSurfaceNV surface, GLenum pname, GLsizei bufSize, GLsizei* length, GLint *values)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUInitNV(const void* vdpDevice, const GLvoid*getProcAddress)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUIsSurfaceNV(GLvdpauSurfaceNV surface)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUMapSurfacesNV(GLsizei numSurfaces, const GLvdpauSurfaceNV* surfaces)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUSurfaceAccessNV(GLvdpauSurfaceNV surface, GLenum access)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUUnmapSurfacesNV(GLsizei numSurface, const GLvdpauSurfaceNV* surfaces)
{
	__GLEW_NOT_IMPL__
}

void __glewVDPAUUnregisterSurfaceNV(GLvdpauSurfaceNV surface)
{
	__GLEW_NOT_IMPL__
}


void __glewFlushVertexArrayRangeNV(void)
{
	__GLEW_NOT_IMPL__
}


void __glewGetVertexAttribLi64vNV(GLuint index, GLenum pname, GLint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribLui64vNV(GLuint index, GLenum pname, GLuint64EXT* params)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1i64NV(GLuint index, GLint64EXT x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1i64vNV(GLuint index, const GLint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1ui64NV(GLuint index, GLuint64EXT x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL1ui64vNV(GLuint index, const GLuint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2i64NV(GLuint index, GLint64EXT x, GLint64EXT y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2i64vNV(GLuint index, const GLint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL2ui64vNV(GLuint index, const GLuint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3i64vNV(GLuint index, const GLint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL3ui64vNV(GLuint index, const GLuint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4i64vNV(GLuint index, const GLint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribL4ui64vNV(GLuint index, const GLuint64EXT* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribLFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}


void __glewBufferAddressRangeNV(GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length)
{
	__GLEW_NOT_IMPL__
}

void __glewColorFormatNV(GLint size, GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewEdgeFlagFormatNV(GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewFogCoordFormatNV(GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewGetIntegerui64i_vNV(GLenum value, GLuint index, GLuint64EXT result[])
{
	__GLEW_NOT_IMPL__
}

void __glewIndexFormatNV(GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewNormalFormatNV(GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewSecondaryColorFormatNV(GLint size, GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoordFormatNV(GLint size, GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribFormatNV(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribIFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexFormatNV(GLint size, GLenum type, GLsizei stride)
{
	__GLEW_NOT_IMPL__
}


GLboolean __glewAreProgramsResidentNV(GLsizei n, const GLuint* ids, GLboolean *residences)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewBindProgramNV(GLenum target, GLuint id)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteProgramsNV(GLsizei n, const GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewExecuteProgramNV(GLenum target, GLuint id, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGenProgramsNV(GLsizei n, GLuint* ids)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramParameterdvNV(GLenum target, GLuint index, GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramParameterfvNV(GLenum target, GLuint index, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramStringNV(GLuint id, GLenum pname, GLubyte* program)
{
	__GLEW_NOT_IMPL__
}

void __glewGetProgramivNV(GLuint id, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetTrackMatrixivNV(GLenum target, GLuint address, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribPointervNV(GLuint index, GLenum pname, GLvoid** pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribdvNV(GLuint index, GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribfvNV(GLuint index, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVertexAttribivNV(GLuint index, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLboolean __glewIsProgramNV(GLuint id)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewLoadProgramNV(GLenum target, GLuint id, GLsizei len, const GLubyte* program)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameter4dNV(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameter4dvNV(GLenum target, GLuint index, const GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameter4fNV(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameter4fvNV(GLenum target, GLuint index, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameters4dvNV(GLenum target, GLuint index, GLsizei num, const GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewProgramParameters4fvNV(GLenum target, GLuint index, GLsizei num, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewTrackMatrixNV(GLenum target, GLuint address, GLenum matrix, GLenum transform)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1dNV(GLuint index, GLdouble x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1dvNV(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1fNV(GLuint index, GLfloat x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1fvNV(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1sNV(GLuint index, GLshort x)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib1svNV(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2dNV(GLuint index, GLdouble x, GLdouble y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2dvNV(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2fNV(GLuint index, GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2fvNV(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2sNV(GLuint index, GLshort x, GLshort y)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib2svNV(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3dvNV(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3fvNV(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3sNV(GLuint index, GLshort x, GLshort y, GLshort z)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib3svNV(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4dvNV(GLuint index, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4fvNV(GLuint index, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4sNV(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4svNV(GLuint index, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4ubNV(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttrib4ubvNV(GLuint index, const GLubyte* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribPointerNV(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs1dvNV(GLuint index, GLsizei n, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs1fvNV(GLuint index, GLsizei n, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs1svNV(GLuint index, GLsizei n, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs2dvNV(GLuint index, GLsizei n, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs2fvNV(GLuint index, GLsizei n, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs2svNV(GLuint index, GLsizei n, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs3dvNV(GLuint index, GLsizei n, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs3fvNV(GLuint index, GLsizei n, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs3svNV(GLuint index, GLsizei n, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs4dvNV(GLuint index, GLsizei n, const GLdouble* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs4fvNV(GLuint index, GLsizei n, const GLfloat* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs4svNV(GLuint index, GLsizei n, const GLshort* v)
{
	__GLEW_NOT_IMPL__
}

void __glewVertexAttribs4ubvNV(GLuint index, GLsizei n, const GLubyte* v)
{
	__GLEW_NOT_IMPL__
}


void __glewBeginVideoCaptureNV(GLuint video_capture_slot)
{
	__GLEW_NOT_IMPL__
}

void __glewBindVideoCaptureStreamBufferNV(GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset)
{
	__GLEW_NOT_IMPL__
}

void __glewBindVideoCaptureStreamTextureNV(GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture)
{
	__GLEW_NOT_IMPL__
}

void __glewEndVideoCaptureNV(GLuint video_capture_slot)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVideoCaptureStreamdvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVideoCaptureStreamfvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVideoCaptureStreamivNV(GLuint video_capture_slot, GLuint stream, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetVideoCaptureivNV(GLuint video_capture_slot, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

GLenum __glewVideoCaptureNV(GLuint video_capture_slot, GLuint* sequence_num, GLuint64EXT *capture_time)
{
	__GLEW_NOT_IMPL__
	return 0;
}

void __glewVideoCaptureStreamParameterdvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble* params)
{
	__GLEW_NOT_IMPL__
}

void __glewVideoCaptureStreamParameterfvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewVideoCaptureStreamParameterivNV(GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}


void __glewClearDepthfOES(GLclampd depth)
{
	__GLEW_NOT_IMPL__
}

void __glewClipPlanefOES(GLenum plane, const GLfloat* equation)
{
	__GLEW_NOT_IMPL__
}

void __glewDepthRangefOES(GLclampf n, GLclampf f)
{
	__GLEW_NOT_IMPL__
}

void __glewFrustumfOES(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
	__GLEW_NOT_IMPL__
}

void __glewGetClipPlanefOES(GLenum plane, GLfloat* equation)
{
	__GLEW_NOT_IMPL__
}

void __glewOrthofOES(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
	__GLEW_NOT_IMPL__
}


void __glewDetailTexFuncSGIS(GLenum target, GLsizei n, const GLfloat* points)
{
	__GLEW_NOT_IMPL__
}

void __glewGetDetailTexFuncSGIS(GLenum target, GLfloat* points)
{
	__GLEW_NOT_IMPL__
}


void __glewFogFuncSGIS(GLsizei n, const GLfloat* points)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFogFuncSGIS(GLfloat* points)
{
	__GLEW_NOT_IMPL__
}


void __glewSampleMaskSGIS(GLclampf value, GLboolean invert)
{
	__GLEW_NOT_IMPL__
}

void __glewSamplePatternSGIS(GLenum pattern)
{
	__GLEW_NOT_IMPL__
}


void __glewGetSharpenTexFuncSGIS(GLenum target, GLfloat* points)
{
	__GLEW_NOT_IMPL__
}

void __glewSharpenTexFuncSGIS(GLenum target, GLsizei n, const GLfloat* points)
{
	__GLEW_NOT_IMPL__
}


void __glewTexImage4DSGIS(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei extent, GLint border, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}

void __glewTexSubImage4DSGIS(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei extent, GLenum format, GLenum type, const void* pixels)
{
	__GLEW_NOT_IMPL__
}


void __glewGetTexFilterFuncSGIS(GLenum target, GLenum filter, GLfloat* weights)
{
	__GLEW_NOT_IMPL__
}

void __glewTexFilterFuncSGIS(GLenum target, GLenum filter, GLsizei n, const GLfloat* weights)
{
	__GLEW_NOT_IMPL__
}


void __glewAsyncMarkerSGIX(GLuint marker)
{
	__GLEW_NOT_IMPL__
}

void __glewDeleteAsyncMarkersSGIX(GLuint marker, GLsizei range)
{
	__GLEW_NOT_IMPL__
}

GLint __glewFinishAsyncSGIX(GLuint* markerp)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLuint __glewGenAsyncMarkersSGIX(GLsizei range)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLboolean __glewIsAsyncMarkerSGIX(GLuint marker)
{
	__GLEW_NOT_IMPL__
	return 0;
}

GLint __glewPollAsyncSGIX(GLuint* markerp)
{
	__GLEW_NOT_IMPL__
	return 0;
}


void __glewFlushRasterSGIX(void)
{
	__GLEW_NOT_IMPL__
}


void __glewTextureFogSGIX(GLenum pname)
{
	__GLEW_NOT_IMPL__
}


void __glewFragmentColorMaterialSGIX(GLenum face, GLenum mode)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightModelfSGIX(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightModeliSGIX(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightfSGIX(GLenum light, GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentLightiSGIX(GLenum light, GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialfSGIX(GLenum face, GLenum pname, const GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialfvSGIX(GLenum face, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialiSGIX(GLenum face, GLenum pname, const GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewFragmentMaterialivSGIX(GLenum face, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFragmentLightfvSGIX(GLenum light, GLenum value, GLfloat* data)
{
	__GLEW_NOT_IMPL__
}

void __glewGetFragmentLightivSGIX(GLenum light, GLenum value, GLint* data)
{
	__GLEW_NOT_IMPL__
}


void __glewFrameZoomSGIX(GLint factor)
{
	__GLEW_NOT_IMPL__
}


void __glewPixelTexGenSGIX(GLenum mode)
{
	__GLEW_NOT_IMPL__
}


void __glewReferencePlaneSGIX(const GLdouble* equation)
{
	__GLEW_NOT_IMPL__
}


void __glewSpriteParameterfSGIX(GLenum pname, GLfloat param)
{
	__GLEW_NOT_IMPL__
}

void __glewSpriteParameteriSGIX(GLenum pname, GLint param)
{
	__GLEW_NOT_IMPL__
}

void __glewSpriteParameterivSGIX(GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}


void __glewTagSampleBufferSGIX(void)
{
	__GLEW_NOT_IMPL__
}


void __glewColorTableParameterfvSGI(GLenum target, GLenum pname, const GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewColorTableParameterivSGI(GLenum target, GLenum pname, const GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewColorTableSGI(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* table)
{
	__GLEW_NOT_IMPL__
}

void __glewCopyColorTableSGI(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableParameterfvSGI(GLenum target, GLenum pname, GLfloat* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableParameterivSGI(GLenum target, GLenum pname, GLint* params)
{
	__GLEW_NOT_IMPL__
}

void __glewGetColorTableSGI(GLenum target, GLenum format, GLenum type, void* table)
{
	__GLEW_NOT_IMPL__
}


void __glewFinishTextureSUNX(void)
{
	__GLEW_NOT_IMPL__
}


void __glewGlobalAlphaFactorbSUN(GLbyte factor)
{
	__GLEW_NOT_IMPL__
}

void __glewGlobalAlphaFactordSUN(GLdouble factor)
{
	__GLEW_NOT_IMPL__
}

void __glewGlobalAlphaFactorfSUN(GLfloat factor)
{
	__GLEW_NOT_IMPL__
}

void __glewGlobalAlphaFactoriSUN(GLint factor)
{
	__GLEW_NOT_IMPL__
}

void __glewGlobalAlphaFactorsSUN(GLshort factor)
{
	__GLEW_NOT_IMPL__
}

void __glewGlobalAlphaFactorubSUN(GLubyte factor)
{
	__GLEW_NOT_IMPL__
}

void __glewGlobalAlphaFactoruiSUN(GLuint factor)
{
	__GLEW_NOT_IMPL__
}

void __glewGlobalAlphaFactorusSUN(GLushort factor)
{
	__GLEW_NOT_IMPL__
}


void __glewReadVideoPixelsSUN(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	__GLEW_NOT_IMPL__
}


void __glewReplacementCodeubSUN(GLubyte code)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeubvSUN(const GLubyte* code)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiSUN(GLuint code)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuivSUN(const GLuint* code)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeusSUN(GLushort code)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeusvSUN(const GLushort* code)
{
	__GLEW_NOT_IMPL__
}


void __glewColor3fVertex3fSUN(GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewColor3fVertex3fvSUN(const GLfloat* c, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4fNormal3fVertex3fSUN(GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4fNormal3fVertex3fvSUN(const GLfloat* c, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4ubVertex2fSUN(GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4ubVertex2fvSUN(const GLubyte* c, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4ubVertex3fSUN(GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewColor4ubVertex3fvSUN(const GLubyte* c, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewNormal3fVertex3fSUN(GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewNormal3fVertex3fvSUN(const GLfloat* n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiColor3fVertex3fSUN(GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiColor3fVertex3fvSUN(const GLuint* rc, const GLfloat *c, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiColor4fNormal3fVertex3fSUN(GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiColor4fNormal3fVertex3fvSUN(const GLuint* rc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiColor4ubVertex3fSUN(GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiColor4ubVertex3fvSUN(const GLuint* rc, const GLubyte *c, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiNormal3fVertex3fSUN(GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiNormal3fVertex3fvSUN(const GLuint* rc, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN(const GLuint* rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN(const GLuint* rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiTexCoord2fVertex3fSUN(GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiTexCoord2fVertex3fvSUN(const GLuint* rc, const GLfloat *tc, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiVertex3fSUN(GLuint rc, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewReplacementCodeuiVertex3fvSUN(const GLuint* rc, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fColor3fVertex3fSUN(GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fColor3fVertex3fvSUN(const GLfloat* tc, const GLfloat *c, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fColor4fNormal3fVertex3fSUN(GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fColor4fNormal3fVertex3fvSUN(const GLfloat* tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fColor4ubVertex3fSUN(GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fColor4ubVertex3fvSUN(const GLfloat* tc, const GLubyte *c, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fNormal3fVertex3fSUN(GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fNormal3fVertex3fvSUN(const GLfloat* tc, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fVertex3fSUN(GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord2fVertex3fvSUN(const GLfloat* tc, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord4fColor4fNormal3fVertex4fSUN(GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord4fColor4fNormal3fVertex4fvSUN(const GLfloat* tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord4fVertex4fSUN(GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	__GLEW_NOT_IMPL__
}

void __glewTexCoord4fVertex4fvSUN(const GLfloat* tc, const GLfloat *v)
{
	__GLEW_NOT_IMPL__
}


void __glewAddSwapHintRectWIN(GLint x, GLint y, GLsizei width, GLsizei height)
{
	__GLEW_NOT_IMPL__
}


