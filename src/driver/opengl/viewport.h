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

#ifndef DRIVER_OPENGL_VIEWPORT_H
#define DRIVER_OPENGL_VIEWPORT_H

#include <GL/glut.h>


/* OpenGL Viewport attribute */
struct opengl_viewport_attributes_t
{
	/* Position */
	GLint x;
	GLint y;
	/* Size */
	GLsizei width;
	GLsizei height;

	/* Depth buffer range */
	GLfloat near;
	GLfloat far;

	/* Mapping transformation as a matrix. */
	// GLmatrix _WindowMap; 
};


struct opengl_viewport_attributes_t *opengl_viewport_create(void);
void opengl_viewport_free(struct opengl_viewport_attributes_t *vpt);

#endif
