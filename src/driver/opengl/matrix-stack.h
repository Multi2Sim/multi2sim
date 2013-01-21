
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

#ifndef DRIVER_OPENGL_MATRIX_STACK_H
#define DRIVER_OPENGL_MATRIX_STACK_H

#include <GL/gl.h>


/* Maximum modelview matrix stack depth */
#define MAX_MODELVIEW_STACK_DEPTH 32
/* Maximum projection matrix stack depth */
#define MAX_PROJECTION_STACK_DEPTH 32
/* Maximum texture matrix stack depth */
#define MAX_TEXTURE_STACK_DEPTH 10
/* FIXME: Maximum color matrix stack depth */ 
#define MAX_COLOR_STACK_DEPTH 32

/* OpenGL Matrix Stack */
struct opengl_matrix_stack_t
{
	struct list_t *stack;
	GLint depth;
	GLint max_depth;
};

struct opengl_matrix_t;

struct opengl_matrix_stack_t *opengl_matrix_stack_create(GLenum mode);
void opengl_matrix_stack_free(struct opengl_matrix_stack_t *mtx_stack);
int opengl_matrix_stack_push(struct opengl_matrix_stack_t *mtx_stack, struct opengl_matrix_t *mtx);
struct opengl_matrix_t *opengl_matrix_stack_pop(struct opengl_matrix_stack_t *mtx_stack);
struct opengl_matrix_t *opengl_matrix_stack_top(struct opengl_matrix_stack_t *mtx_stack);
struct opengl_matrix_t *opengl_matrix_stack_bottom(struct opengl_matrix_stack_t *mtx_stack);

#endif


