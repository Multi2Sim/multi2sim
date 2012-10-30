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

#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "opengl-matrix.h"
#include "opengl-vertex.h"


/* OpenGL identity matrix */
static GLfloat Identity[16] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

struct x86_opengl_matrix_t *x86_opengl_matrix_create(enum x86_opengl_matrix_mode_t mode)
{
	struct x86_opengl_matrix_t *mtx;

	/* Allocate */
	mtx = calloc(1, sizeof(struct x86_opengl_matrix_t));
	if(!mtx)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	mtx->matrix_mode = mode;
	mtx->matrix = calloc(1, 4*4*sizeof(GLfloat));	/* 4x4 matrix */
	switch(mode)
	{
		case MATRIX_IDENTITY:
		{
			memcpy( mtx->matrix, Identity, sizeof(Identity));
			break;
		}
		default:
			break;
	}

	/* Return */	
	return mtx;
}

void x86_opengl_matrix_free(struct x86_opengl_matrix_t *mtx)
{
	free(mtx->matrix);
	free(mtx);
}

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

static void x86_opengl_matrix_matmul4( GLfloat *product, const GLfloat *a, const GLfloat *b )
{
	GLint i;
	for (i = 0; i < 4; i++)
	{
		const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
		P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
		P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
		P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
		P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
	}
}

#undef A
#undef B
#undef P

#define A(row,col)  m[(col<<2)+row]

static void x86_opengl_matrix_matmulvec( GLfloat *product, const GLfloat *m, const GLfloat *v )
{
	product[0] = A(0,0) * v[0] + A(0,1) * v[1] + A(0,2) * v[2] + A(0,3) * v[3];
	product[1] = A(1,0) * v[0] + A(1,1) * v[1] + A(1,2) * v[2] + A(1,3) * v[3];
	product[2] = A(2,0) * v[0] + A(2,1) * v[1] + A(2,2) * v[2] + A(2,3) * v[3];
	product[3] = A(3,0) * v[0] + A(3,1) * v[1] + A(3,2) * v[2] + A(3,3) * v[3];
}

#undef A

void x86_opengl_matrix_mul_matrix(struct x86_opengl_matrix_t *dst_mtx, struct x86_opengl_matrix_t *mtx_a, struct x86_opengl_matrix_t *mtx_b)
{
	x86_opengl_matrix_matmul4(dst_mtx->matrix, mtx_a->matrix, mtx_b->matrix);
}

void x86_opengl_matrix_mul_vertex(struct x86_opengl_vertex_t *vtx, struct x86_opengl_matrix_t *mtx)
{
	GLfloat temp[4];
	temp[0] = vtx->x;
	temp[1] = vtx->y;
	temp[2] = vtx->z;
	temp[3] = vtx->w;

	x86_opengl_matrix_matmulvec(temp, mtx->matrix, temp);

	vtx->x = temp[0];
	vtx->y = temp[1];
	vtx->z = temp[2];
	vtx->w = temp[3];
}

struct x86_opengl_matrix_t *x86_opengl_ortho_matrix_create(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval)
{
	struct x86_opengl_matrix_t *mtx;

	/* Allocate */
	mtx = x86_opengl_matrix_create(MATRIX_3D_NO_ROT);

	#define M(row,col)  mtx->matrix[col*4+row]
	M(0,0) = 2.0F / (right-left);
	M(0,1) = 0.0F;
	M(0,2) = 0.0F;
	M(0,3) = -(right+left) / (right-left);

	M(1,0) = 0.0F;
	M(1,1) = 2.0F / (top-bottom);
	M(1,2) = 0.0F;
	M(1,3) = -(top+bottom) / (top-bottom);

	M(2,0) = 0.0F;
	M(2,1) = 0.0F;
	M(2,2) = -2.0F / (farval-nearval);
	M(2,3) = -(farval+nearval) / (farval-nearval);

	M(3,0) = 0.0F;
	M(3,1) = 0.0F;
	M(3,2) = 0.0F;
	M(3,3) = 1.0F;
	#undef M

	return mtx;
}
