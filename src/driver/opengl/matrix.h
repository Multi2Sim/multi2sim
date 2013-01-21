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

#ifndef DRIVER_OPENGL_MATRIX_H
#define DRIVER_OPENGL_MATRIX_H

#include <GL/gl.h>
#include <math.h> 

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#ifndef M_E
#define M_E (2.7182818284590452354)
#endif

#ifndef M_LOG2E
#define M_LOG2E     (1.4426950408889634074)
#endif

#ifndef ONE_DIV_SQRT_LN2
#define ONE_DIV_SQRT_LN2 (1.201122408786449815)
#endif

#ifndef FLT_MAX_EXP
#define FLT_MAX_EXP 128
#endif

/* Degrees to radians conversion: */
#define DEG2RAD (M_PI/180.0)

/*
 * Transform a point (column vector) by a matrix:   Q = M * P
 */
#define TRANSFORM_POINT( Q, M, P )					\
   Q[0] = M[0] * P[0] + M[4] * P[1] + M[8] *  P[2] + M[12] * P[3];	\
   Q[1] = M[1] * P[0] + M[5] * P[1] + M[9] *  P[2] + M[13] * P[3];	\
   Q[2] = M[2] * P[0] + M[6] * P[1] + M[10] * P[2] + M[14] * P[3];	\
   Q[3] = M[3] * P[0] + M[7] * P[1] + M[11] * P[2] + M[15] * P[3];


#define TRANSFORM_POINT3( Q, M, P )				\
   Q[0] = M[0] * P[0] + M[4] * P[1] + M[8] *  P[2] + M[12];	\
   Q[1] = M[1] * P[0] + M[5] * P[1] + M[9] *  P[2] + M[13];	\
   Q[2] = M[2] * P[0] + M[6] * P[1] + M[10] * P[2] + M[14];	\
   Q[3] = M[3] * P[0] + M[7] * P[1] + M[11] * P[2] + M[15];


/*
 * Transform a normal (row vector) by a matrix:  [NX NY NZ] = N * MAT
 */
#define TRANSFORM_NORMAL( TO, N, MAT )				\
do {								\
   TO[0] = N[0] * MAT[0] + N[1] * MAT[1] + N[2] * MAT[2];	\
   TO[1] = N[0] * MAT[4] + N[1] * MAT[5] + N[2] * MAT[6];	\
   TO[2] = N[0] * MAT[8] + N[1] * MAT[9] + N[2] * MAT[10];	\
} while (0)


/**
 * Transform a direction by a matrix.
 */
#define TRANSFORM_DIRECTION( TO, DIR, MAT )			\
do {								\
   TO[0] = DIR[0] * MAT[0] + DIR[1] * MAT[4] + DIR[2] * MAT[8];	\
   TO[1] = DIR[0] * MAT[1] + DIR[1] * MAT[5] + DIR[2] * MAT[9];	\
   TO[2] = DIR[0] * MAT[2] + DIR[1] * MAT[6] + DIR[2] * MAT[10];\
} while (0)

struct opengl_vertex_t;
/* OpenGL: Different kinds of 4x4 transformation matrices */
enum opengl_matrix_mode_t {
	MATRIX_GENERAL,		/* general 4x4 matrix */
	MATRIX_IDENTITY,		/* identity matrix */
	MATRIX_3D_NO_ROT,		/* orthogonal projection and others... */
	MATRIX_PERSPECTIVE,	/* perspective projection matrix */
	MATRIX_2D,			/* 2-D transformation */
	MATRIX_2D_NO_ROT,		/* 2-D scale & translate only */
	MATRIX_3D			/* 3-D transformation */
} ;

/* OpenGL Matrix */
struct opengl_matrix_t
{
	GLfloat *matrix;	/* Points to 4x4 GLfloat type arrays*/
	GLfloat *matinv;	/* Points to 4x4 GLfloat type arrays*/
	GLuint flags;
	enum opengl_matrix_mode_t matrix_mode;	
};

struct opengl_matrix_t *opengl_matrix_create(enum opengl_matrix_mode_t mode);
void opengl_matrix_free(struct opengl_matrix_t *mtx);
struct opengl_matrix_t *opengl_matrix_duplicate(struct opengl_matrix_t *mtx);
void opengl_matrix_copy(struct opengl_matrix_t *mtx_dst, struct opengl_matrix_t *mtx_src);

void opengl_matrix_mul_matrix(struct opengl_matrix_t *dst_mtx, struct opengl_matrix_t *mtx_a, struct opengl_matrix_t *mtx_b);
void opengl_matrix_vector_mul_matrix( GLfloat *product, const GLfloat *m, const GLfloat *v );
void opengl_matrix_mul_vertex(struct opengl_vertex_t *vtx, struct opengl_matrix_t *mtx);

struct opengl_matrix_t *opengl_ortho_matrix_create(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval);
void opengl_ortho_matrix_free(struct opengl_matrix_t *mtx);

struct opengl_matrix_t *opengl_frustum_matrix_create(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval);
void opengl_frustum_matrix_free(struct opengl_matrix_t *mtx);

struct opengl_matrix_t *opengl_translate_matrix_create(GLfloat x, GLfloat y, GLfloat z);
void opengl_translate_matrix_free(struct opengl_matrix_t *mtx);

struct opengl_matrix_t *opengl_rotate_matrix_create(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void opengl_rotate_matrix_free(struct opengl_matrix_t *mtx);

GLboolean opengl_matrix_invert(struct opengl_matrix_t *mtx);
GLboolean opengl_matrix_is_dirty(const struct opengl_matrix_t *mtx);
void opengl_matrix_analyse(struct opengl_matrix_t *mtx);

#endif
