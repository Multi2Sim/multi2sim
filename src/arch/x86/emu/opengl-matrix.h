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

#ifndef X86_OPENGL_MATRIX_H
#define X86_OPENGL_MATRIX_H

#include <GL/gl.h>

#include <mem-system/mem-system.h>

#include "opengl-vertex.h"
#include "emu.h"


/* OpenGL: Different kinds of 4x4 transformation matrices */
enum x86_opengl_matrix_mode_t {
	MATRIX_GENERAL,		/* general 4x4 matrix */
	MATRIX_IDENTITY,		/* identity matrix */
	MATRIX_3D_NO_ROT,		/* orthogonal projection and others... */
	MATRIX_PERSPECTIVE,	/* perspective projection matrix */
	MATRIX_2D,			/* 2-D transformation */
	MATRIX_2D_NO_ROT,		/* 2-D scale & translate only */
	MATRIX_3D			/* 3-D transformation */
} ;

/* OpenGL Matrix */
struct x86_opengl_matrix_t
{
	GLfloat *matrix;	/* Points to 4x4 GLfloat type arrays*/
	enum x86_opengl_matrix_mode_t matrix_mode;	
};

struct x86_opengl_matrix_t *x86_opengl_matrix_create(enum x86_opengl_matrix_mode_t mode);
void x86_opengl_matrix_free(struct x86_opengl_matrix_t *mtx);

void x86_opengl_matrix_mul_matrix(struct x86_opengl_matrix_t *dst_mtx, struct x86_opengl_matrix_t *mtx_a, struct x86_opengl_matrix_t *mtx_b);
void x86_opengl_matrix_mul_vertex(struct x86_opengl_vertex_t *vtx, struct x86_opengl_matrix_t *mtx);

#endif
