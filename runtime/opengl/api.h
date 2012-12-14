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


#ifndef RUNTIME_OPENGL_API_H
#define RUNTIME_OPENGL_API_H


/* System call for OpenGL runtime */
#define OPENGL_SYSCALL_CODE  327


/* List of OpenGL runtime calls */
enum opengl_call_t
{
	opengl_call_invalid,

	opengl_call_glDrawBuffer,
	opengl_call_glReadBuffer,
	opengl_call_glEnable,
	opengl_call_glViewport,
	opengl_call_glMatrixMode,		/* 5 */
	opengl_call_glLoadIdentity,
	opengl_call_glOrtho,
	opengl_call_glClear,
	opengl_call_glBegin,
	opengl_call_glEnd,			/* 10 */
	opengl_call_glVertex2f,
	opengl_call_glVertex3f,
	opengl_call_glFlush,
	opengl_call_glColor3f,
	opengl_call_glLightfv,		/* 15 */
	opengl_call_glGenLists,
	opengl_call_glNewList,
	opengl_call_glEndList,
	opengl_call_glMaterialfv,
	opengl_call_glShadeModel,		/* 20 */
	opengl_call_glNormal3f,
	opengl_call_glFrustum,
	opengl_call_glTranslatef,
	opengl_call_glPushMatrix,
	opengl_call_glPopMatrix,		/* 25 */
	opengl_call_glRotatef,
	opengl_call_glCallList,
	opengl_call_glClearColor,

	opengl_call_count
};

#endif

