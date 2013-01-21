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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "matrix.h"
#include "opengl.h"
#include "vector.h"
#include "vertex.h"


#define MAT_FLAG_IDENTITY       0     /**< is an identity matrix flag.
                                       *   (Not actually used - the identity
                                       *   matrix is identified by the absense
                                       *   of all other flags.)
                                       */
#define MAT_FLAG_GENERAL        0x1   /**< is a general matrix flag */
#define MAT_FLAG_ROTATION       0x2   /**< is a rotation matrix flag */
#define MAT_FLAG_TRANSLATION    0x4   /**< is a translation matrix flag */
#define MAT_FLAG_UNIFORM_SCALE  0x8   /**< is an uniform scaling matrix flag */
#define MAT_FLAG_GENERAL_SCALE  0x10  /**< is a general scaling matrix flag */
#define MAT_FLAG_GENERAL_3D     0x20  /**< general 3D matrix flag */
#define MAT_FLAG_PERSPECTIVE    0x40  /**< is a perspective proj matrix flag */
#define MAT_FLAG_SINGULAR       0x80  /**< is a singular matrix flag */
#define MAT_DIRTY_TYPE          0x100  /**< matrix type is dirty */
#define MAT_DIRTY_FLAGS         0x200  /**< matrix flags are dirty */
#define MAT_DIRTY_INVERSE       0x400  /**< matrix inverse is dirty */

/** angle preserving matrix flags mask */
#define MAT_FLAGS_ANGLE_PRESERVING (MAT_FLAG_ROTATION | \
				    MAT_FLAG_TRANSLATION | \
				    MAT_FLAG_UNIFORM_SCALE)

/** geometry related matrix flags mask */
#define MAT_FLAGS_GEOMETRY (MAT_FLAG_GENERAL | \
			    MAT_FLAG_ROTATION | \
			    MAT_FLAG_TRANSLATION | \
			    MAT_FLAG_UNIFORM_SCALE | \
			    MAT_FLAG_GENERAL_SCALE | \
			    MAT_FLAG_GENERAL_3D | \
			    MAT_FLAG_PERSPECTIVE | \
	                    MAT_FLAG_SINGULAR)

/** length preserving matrix flags mask */
#define MAT_FLAGS_LENGTH_PRESERVING (MAT_FLAG_ROTATION | \
				     MAT_FLAG_TRANSLATION)


/** 3D (non-perspective) matrix flags mask */
#define MAT_FLAGS_3D (MAT_FLAG_ROTATION | \
		      MAT_FLAG_TRANSLATION | \
		      MAT_FLAG_UNIFORM_SCALE | \
		      MAT_FLAG_GENERAL_SCALE | \
		      MAT_FLAG_GENERAL_3D)

/** dirty matrix flags mask */
#define MAT_DIRTY          (MAT_DIRTY_TYPE | \
			    MAT_DIRTY_FLAGS | \
			    MAT_DIRTY_INVERSE)


#define TEST_MAT_FLAGS(mat, a)  \
    ((MAT_FLAGS_GEOMETRY & (~(a)) & ((mat)->flags) ) == 0)


/* OpenGL identity matrix */
static GLfloat Identity[16] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

struct opengl_matrix_t *opengl_matrix_create(enum opengl_matrix_mode_t mode)
{
	opengl_debug("\tCreating Matrix ... \n" );
	struct opengl_matrix_t *mtx;

	/* Allocate */
	mtx = xcalloc(1, sizeof(struct opengl_matrix_t));
	if(!mtx)
		fatal("%s: out of memory", __FUNCTION__);
	opengl_debug("\t\tmtx ptr = %p\n", mtx);

	/* Initialize */
	mtx->matrix_mode = mode;
	mtx->matrix = xcalloc(1, 16 * sizeof(GLfloat));	/* 4x4 matrix */
	mtx->matinv = xcalloc(1, 16 * sizeof(GLfloat));	/* 4x4 matrix */

	opengl_debug("\t\tmtx->matrix ptr = %p\n", mtx->matrix);

	switch(mode)
	{
		case MATRIX_IDENTITY:
		{
			memcpy( mtx->matrix, Identity, 16 * sizeof(GLfloat));
			break;
		}
		default:
			break;
	}

	/* Return */	
	return mtx;
}

void opengl_matrix_free(struct opengl_matrix_t *mtx)
{
	opengl_debug("\tFreeing Matrix ... \n" );
	opengl_debug("\t\tmtx ptr = %p\n", mtx);
	opengl_debug("\t\tmtx->matrix ptr = %p\n", mtx->matrix);

	free(mtx->matrix);
	free(mtx->matinv);
	free(mtx);
}

struct opengl_matrix_t *opengl_matrix_duplicate(struct opengl_matrix_t *mtx_src)
{
	if(!mtx_src)
		fatal("%s: out of memory", __FUNCTION__);	
	opengl_debug("\tDuplicating Matrix ... \n" );
	struct opengl_matrix_t *mtx;

	/* Allocate */
	mtx = opengl_matrix_create(mtx_src->matrix_mode);
	if(!mtx)
		fatal("%s: out of memory", __FUNCTION__);
	opengl_debug("\t\tmtx ptr = %p\n", mtx);

	/* Duplicate */
	memcpy(mtx->matrix, mtx_src->matrix, 16 * sizeof(GLfloat));
	memcpy(mtx->matinv, mtx_src->matinv, 16 * sizeof(GLfloat));

	/* Return */	
	return mtx;
}

void opengl_matrix_copy(struct opengl_matrix_t *mtx_dst, struct opengl_matrix_t *mtx_src)
{
	/* Copy */
	mtx_dst->matrix_mode = mtx_src->matrix_mode;
	memcpy(mtx_dst->matrix, mtx_src->matrix, 16 * sizeof(GLfloat));
	memcpy(mtx_dst->matinv, mtx_src->matinv, 16 * sizeof(GLfloat));

}

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]
static void opengl_matrix_matmul4( GLfloat *product, const GLfloat *a, const GLfloat *b )
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
void opengl_matrix_vector_mul_matrix( GLfloat *product, const GLfloat *v, const GLfloat *m)
{
	product[0] = A(0,0) * v[0] + A(0,1) * v[1] + A(0,2) * v[2] + A(0,3) * v[3];
	product[1] = A(1,0) * v[0] + A(1,1) * v[1] + A(1,2) * v[2] + A(1,3) * v[3];
	product[2] = A(2,0) * v[0] + A(2,1) * v[1] + A(2,2) * v[2] + A(2,3) * v[3];
	product[3] = A(3,0) * v[0] + A(3,1) * v[1] + A(3,2) * v[2] + A(3,3) * v[3];
}
#undef A

void opengl_matrix_mul_matrix(struct opengl_matrix_t *dst_mtx, struct opengl_matrix_t *mtx_a, struct opengl_matrix_t *mtx_b)
{
	opengl_matrix_matmul4(dst_mtx->matrix, mtx_a->matrix, mtx_b->matrix);
}

void opengl_matrix_mul_vertex(struct opengl_vertex_t *vtx, struct opengl_matrix_t *mtx)
{
	GLfloat temp[4];
	temp[0] = vtx->pos[X_COMP];
	temp[1] = vtx->pos[Y_COMP];
	temp[2] = vtx->pos[Z_COMP];
	temp[3] = vtx->pos[W_COMP];

	opengl_matrix_vector_mul_matrix(temp, temp, mtx->matrix);

	vtx->pos[X_COMP] = temp[0];
	vtx->pos[Y_COMP] = temp[1];
	vtx->pos[Z_COMP] = temp[2];
	vtx->pos[W_COMP] = temp[3];
}

#define M(row,col)  mtx->matrix[col*4+row]
struct opengl_matrix_t *opengl_ortho_matrix_create(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval)
{
	struct opengl_matrix_t *mtx;

	/* Allocate */
	mtx = opengl_matrix_create(MATRIX_3D_NO_ROT);

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

	return mtx;
}
#undef M

void opengl_ortho_matrix_free(struct opengl_matrix_t *mtx)
{
	opengl_matrix_free(mtx);
}


#define M(row,col)  mtx->matrix[col*4+row]
struct opengl_matrix_t *opengl_frustum_matrix_create(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval)
{
	struct opengl_matrix_t *mtx;
	GLfloat x;
	GLfloat y;
	GLfloat a;
	GLfloat b;
	GLfloat c;
	GLfloat d;

	/* Allocate */
	mtx = opengl_matrix_create(MATRIX_PERSPECTIVE);

	/* Caculate perspective matrix */
	x = (2.0F*nearval) / (right-left);
	y = (2.0F*nearval) / (top-bottom);
	a = (right+left) / (right-left);
	b = (top+bottom) / (top-bottom);
	c = -(farval+nearval) / ( farval-nearval);
	d = -(2.0F*farval*nearval) / (farval-nearval);  /* error? */

	M(0,0) = x;
	M(0,1) = 0.0F;  
	M(0,2) = a;
	M(0,3) = 0.0F;

	M(1,0) = 0.0F;
	M(1,1) = y;
	 M(1,2) = b;
	 M(1,3) = 0.0F;

	M(2,0) = 0.0F;
	M(2,1) = 0.0F;
	M(2,2) = c;
	M(2,3) = d;

	M(3,0) = 0.0F;
	M(3,1) = 0.0F;
	M(3,2) = -1.0F;
	M(3,3) = 0.0F;
	return mtx;

}
#undef M

void opengl_frustum_matrix_free(struct opengl_matrix_t *mtx)
{
	opengl_matrix_free(mtx);
}

#define M(row,col)  mtx->matrix[col*4+row]
struct opengl_matrix_t *opengl_translate_matrix_create(GLfloat x, GLfloat y, GLfloat z)
{
	struct opengl_matrix_t *mtx;

	/* Allocate */
	mtx = opengl_matrix_create(MATRIX_PERSPECTIVE);

	M(0,0) = 1.0F;
	M(0,1) = 0.0F;  
	M(0,2) = 0.0F;
	M(0,3) = x;

	M(1,0) = 0.0F;
	M(1,1) = 1.0F;
	 M(1,2) = 0.0F;
	 M(1,3) = y;

	M(2,0) = 0.0F;
	M(2,1) = 0.0F;
	M(2,2) = 1.0F;
	M(2,3) = z;

	M(3,0) = 0.0F;
	M(3,1) = 0.0F;
	M(3,2) = 0.0F;
	M(3,3) = 1.0F;

	return mtx;

}
#undef M

void opengl_translate_matrix_free(struct opengl_matrix_t *mtx)
{
	opengl_matrix_free(mtx);
}

#define M(row,col)  mtx->matrix[col*4+row]
struct opengl_matrix_t *opengl_rotate_matrix_create(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	struct opengl_matrix_t *mtx;
	GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
	GLboolean optimized;

	/* Allocate */
	mtx = opengl_matrix_create(MATRIX_3D);

	s = (GLfloat) sin( angle * DEG2RAD );
	c = (GLfloat) cos( angle * DEG2RAD );

	memcpy(mtx->matrix, Identity, sizeof(GLfloat)*16);
	optimized = GL_FALSE;


	if (x == 0.0F)
	{
		if (y == 0.0F)
		{
			if (z != 0.0F) 
			{
				optimized = GL_TRUE;
				/* rotate only around z-axis */
				M(0,0) = c;
				M(1,1) = c;
				if (z < 0.0F)
				{
					M(0,1) = s;
					M(1,0) = -s;
				}
				else
				{
					M(0,1) = -s;
					M(1,0) = s;
				}
			}
		}
		else if (z == 0.0F)
		{
			optimized = GL_TRUE;
			/* rotate only around y-axis */
			M(0,0) = c;
			M(2,2) = c;
			if (y < 0.0F)
			{
				M(0,2) = -s;
				M(2,0) = s;
			}
			else
			{
				M(0,2) = s;
				M(2,0) = -s;
			}
		}
	}
	else if (y == 0.0F) 
	{
		if (z == 0.0F)
		{
			optimized = GL_TRUE;
			/* rotate only around x-axis */
			M(1,1) = c;
			M(2,2) = c;
			if (x < 0.0F)
			{
				M(1,2) = s;
				M(2,1) = -s;
			}
			 else 
			{
				M(1,2) = -s;
				M(2,1) = s;
			}
		}
	}

	if (!optimized)
	{
		const GLfloat mag = sqrtf(x * x + y * y + z * z);

		/* no rotation, leave mat as-is */
		if (mag <= 1.0e-4)
			return mtx;

		x /= mag;
		y /= mag;
		z /= mag;


		/*
		*     Arbitrary axis rotation matrix.
		*
		*  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
		*  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
		*  (which is about the X-axis), and the two composite transforms
		*  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
		*  from the arbitrary axis to the X-axis then back.  They are
		*  all elementary rotations.
		*
		*  Rz' is a rotation about the Z-axis, to bring the axis vector
		*  into the x-z plane.  Then Ry' is applied, rotating about the
		*  Y-axis to bring the axis vector parallel with the X-axis.  The
		*  rotation about the X-axis is then performed.  Ry and Rz are
		*  simply the respective inverse transforms to bring the arbitrary
		*  axis back to its original orientation.  The first transforms
		*  Rz' and Ry' are considered inverses, since the data from the
		*  arbitrary axis gives you info on how to get to it, not how
		*  to get away from it, and an inverse must be applied.
		*
		*  The basic calculation used is to recognize that the arbitrary
		*  axis vector (x, y, z), since it is of unit length, actually
		*  represents the sines and cosines of the angles to rotate the
		*  X-axis to the same orientation, with theta being the angle about
		*  Z and phi the angle about Y (in the order described above)
		*  as follows:
		*
		*  cos ( theta ) = x / sqrt ( 1 - z^2 )
		*  sin ( theta ) = y / sqrt ( 1 - z^2 )
		*
		*  cos ( phi ) = sqrt ( 1 - z^2 )
		*  sin ( phi ) = z
		*
		*  Note that cos ( phi ) can further be inserted to the above
		*  formulas:
		*
		*  cos ( theta ) = x / cos ( phi )
		*  sin ( theta ) = y / sin ( phi )
		*
		*  ...etc.  Because of those relations and the standard trigonometric
		*  relations, it is pssible to reduce the transforms down to what
		*  is used below.  It may be that any primary axis chosen will give the
		*  same results (modulo a sign convention) using thie method.
		*
		*  Particularly nice is to notice that all divisions that might
		*  have caused trouble when parallel to certain planes or
		*  axis go away with care paid to reducing the expressions.
		*  After checking, it does perform correctly under all cases, since
		*  in all the cases of division where the denominator would have
		*  been zero, the numerator would have been zero as well, giving
		*  the expected result.
		*/

		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * s;
		ys = y * s;
		zs = z * s;
		one_c = 1.0F - c;

		/* We already hold the identity-matrix so we can skip some statements */
		M(0,0) = (one_c * xx) + c;
		M(0,1) = (one_c * xy) - zs;
		M(0,2) = (one_c * zx) + ys;
		/*    M(0,3) = 0.0F; */

		M(1,0) = (one_c * xy) + zs;
		M(1,1) = (one_c * yy) + c;
		M(1,2) = (one_c * yz) - xs;
		/*    M(1,3) = 0.0F; */

		M(2,0) = (one_c * zx) - ys;
		M(2,1) = (one_c * yz) + xs;
		M(2,2) = (one_c * zz) + c;
		/*    M(2,3) = 0.0F; */

		/*
		M(3,0) = 0.0F;
		M(3,1) = 0.0F;
		M(3,2) = 0.0F;
		M(3,3) = 1.0F;
		*/
	}
	return mtx;

}
#undef M

void opengl_rotate_matrix_free(struct opengl_matrix_t *mtx)
{
	opengl_matrix_free(mtx);
}

#define MAT(m,r,c) (m)[(c)*4+(r)]

#define SWAP_ROWS(a, b) { GLfloat *_tmp = a; (a)=(b); (b)=_tmp; }
static GLboolean opengl_invert_matrix_general(struct opengl_matrix_t *mtx)
{
	const GLfloat *m = mtx->matrix;
	GLfloat *out = mtx->matinv;
	GLfloat wtmp[4][8];
	GLfloat m0, m1, m2, m3, s;
	GLfloat *r0, *r1, *r2, *r3;

	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

	r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
	r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
	r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

	r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
	r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
	r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

	r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
	r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
	r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

	r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
	r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
	r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

	/* choose pivot - or die */
	if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
	if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
	if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
	if (0.0 == r0[0])  return GL_FALSE;

	/* eliminate first variable     */
	m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
	s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
	s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
	s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
	s = r0[5];
	if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
	s = r0[6];
	if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
	s = r0[7];
	if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

	/* choose pivot - or die */
	if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
	if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
	if (0.0 == r1[1])  return GL_FALSE;

	/* eliminate second variable */
	m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
	r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
	s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
	s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
	s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
	s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

	/* choose pivot - or die */
	if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
	if (0.0 == r2[2])  return GL_FALSE;

	/* eliminate third variable */
	m3 = r3[2]/r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
	r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
	r3[7] -= m3 * r2[7];

	/* last check */
	if (0.0 == r3[3]) return GL_FALSE;

	s = 1.0F/r3[3];             /* now back substitute row 3 */
	r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

	m2 = r2[3];                 /* now back substitute row 2 */
	s  = 1.0F/r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
	r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
	r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
	r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

	m1 = r1[2];                 /* now back substitute row 1 */
	s  = 1.0F/r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
	r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
	r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

	m0 = r0[1];                 /* now back substitute row 0 */
	s  = 1.0F/r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
	r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

	MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
	MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
	MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
	MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
	MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
	MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
	MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
	MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7];

	return GL_TRUE;
}
#undef SWAP_ROWS

static GLboolean opengl_invert_matrix_3d_general(struct opengl_matrix_t *mtx)
{
	const GLfloat *in = mtx->matrix;
	GLfloat *out = mtx->matinv;
	GLfloat pos, neg, t;
	GLfloat det;

	/* Calculate the determinant of upper left 3x3 submatrix and
	* determine if the matrix is singular.
	*/
	pos = neg = 0.0;
	t =  MAT(in,0,0) * MAT(in,1,1) * MAT(in,2,2);
	if (t >= 0.0) pos += t; else neg += t;

	t =  MAT(in,1,0) * MAT(in,2,1) * MAT(in,0,2);
	if (t >= 0.0) pos += t; else neg += t;

	t =  MAT(in,2,0) * MAT(in,0,1) * MAT(in,1,2);
	if (t >= 0.0) pos += t; else neg += t;

	t = -MAT(in,2,0) * MAT(in,1,1) * MAT(in,0,2);
	if (t >= 0.0) pos += t; else neg += t;

	t = -MAT(in,1,0) * MAT(in,0,1) * MAT(in,2,2);
	if (t >= 0.0) pos += t; else neg += t;

	t = -MAT(in,0,0) * MAT(in,2,1) * MAT(in,1,2);
	if (t >= 0.0) pos += t; else neg += t;

	det = pos + neg;

	if (fabs(det) < 1e-25)
	  return GL_FALSE;

	det = 1.0F / det;
	MAT(out,0,0) = (  (MAT(in,1,1)*MAT(in,2,2) - MAT(in,2,1)*MAT(in,1,2) )*det);
	MAT(out,0,1) = (- (MAT(in,0,1)*MAT(in,2,2) - MAT(in,2,1)*MAT(in,0,2) )*det);
	MAT(out,0,2) = (  (MAT(in,0,1)*MAT(in,1,2) - MAT(in,1,1)*MAT(in,0,2) )*det);
	MAT(out,1,0) = (- (MAT(in,1,0)*MAT(in,2,2) - MAT(in,2,0)*MAT(in,1,2) )*det);
	MAT(out,1,1) = (  (MAT(in,0,0)*MAT(in,2,2) - MAT(in,2,0)*MAT(in,0,2) )*det);
	MAT(out,1,2) = (- (MAT(in,0,0)*MAT(in,1,2) - MAT(in,1,0)*MAT(in,0,2) )*det);
	MAT(out,2,0) = (  (MAT(in,1,0)*MAT(in,2,1) - MAT(in,2,0)*MAT(in,1,1) )*det);
	MAT(out,2,1) = (- (MAT(in,0,0)*MAT(in,2,1) - MAT(in,2,0)*MAT(in,0,1) )*det);
	MAT(out,2,2) = (  (MAT(in,0,0)*MAT(in,1,1) - MAT(in,1,0)*MAT(in,0,1) )*det);

	/* Do the translation part */
	MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0) +
		     MAT(in,1,3) * MAT(out,0,1) +
		     MAT(in,2,3) * MAT(out,0,2) );
	MAT(out,1,3) = - (MAT(in,0,3) * MAT(out,1,0) +
		     MAT(in,1,3) * MAT(out,1,1) +
		     MAT(in,2,3) * MAT(out,1,2) );
	MAT(out,2,3) = - (MAT(in,0,3) * MAT(out,2,0) +
		     MAT(in,1,3) * MAT(out,2,1) +
		     MAT(in,2,3) * MAT(out,2,2) );

	return GL_TRUE;
}

static GLboolean opengl_invert_matrix_3d(struct opengl_matrix_t *mtx)
{
	const GLfloat *in = mtx->matrix;
	GLfloat *out = mtx->matinv;

	if (!TEST_MAT_FLAGS(mtx, MAT_FLAGS_ANGLE_PRESERVING))
		return opengl_invert_matrix_3d_general( mtx );

	if (mtx->flags & MAT_FLAG_UNIFORM_SCALE)
	{
		GLfloat scale = (MAT(in,0,0) * MAT(in,0,0) +
		               MAT(in,0,1) * MAT(in,0,1) +
		               MAT(in,0,2) * MAT(in,0,2));

		if (scale == 0.0)
		 return GL_FALSE;

		scale = 1.0F / scale;

		/* Transpose and scale the 3 by 3 upper-left submatrix. */
		MAT(out,0,0) = scale * MAT(in,0,0);
		MAT(out,1,0) = scale * MAT(in,0,1);
		MAT(out,2,0) = scale * MAT(in,0,2);
		MAT(out,0,1) = scale * MAT(in,1,0);
		MAT(out,1,1) = scale * MAT(in,1,1);
		MAT(out,2,1) = scale * MAT(in,1,2);
		MAT(out,0,2) = scale * MAT(in,2,0);
		MAT(out,1,2) = scale * MAT(in,2,1);
		MAT(out,2,2) = scale * MAT(in,2,2);
	}
	else if (mtx->flags & MAT_FLAG_ROTATION)
	{
		/* Transpose the 3 by 3 upper-left submatrix. */
		MAT(out,0,0) = MAT(in,0,0);
		MAT(out,1,0) = MAT(in,0,1);
		MAT(out,2,0) = MAT(in,0,2);
		MAT(out,0,1) = MAT(in,1,0);
		MAT(out,1,1) = MAT(in,1,1);
		MAT(out,2,1) = MAT(in,1,2);
		MAT(out,0,2) = MAT(in,2,0);
		MAT(out,1,2) = MAT(in,2,1);
		MAT(out,2,2) = MAT(in,2,2);
	}
	else 
	{
		/* pure translation */
		memcpy( out, Identity, sizeof(Identity) );
		MAT(out,0,3) = - MAT(in,0,3);
		MAT(out,1,3) = - MAT(in,1,3);
		MAT(out,2,3) = - MAT(in,2,3);
		return GL_TRUE;
	}

	if (mtx->flags & MAT_FLAG_TRANSLATION) 
	{
		/* Do the translation part */
		MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0) +
			MAT(in,1,3) * MAT(out,0,1) +
			MAT(in,2,3) * MAT(out,0,2) );
		MAT(out,1,3) = - (MAT(in,0,3) * MAT(out,1,0) +
			MAT(in,1,3) * MAT(out,1,1) +
			MAT(in,2,3) * MAT(out,1,2) );
		MAT(out,2,3) = - (MAT(in,0,3) * MAT(out,2,0) +
			MAT(in,1,3) * MAT(out,2,1) +
			MAT(in,2,3) * MAT(out,2,2) );
	}
	else 
	{
		MAT(out,0,3) = MAT(out,1,3) = MAT(out,2,3) = 0.0;
	}

	return GL_TRUE;
}

static GLboolean opengl_invert_matrix_identity(struct opengl_matrix_t *mtx)
{
	memcpy( mtx->matinv, Identity, sizeof(Identity) );
	return GL_TRUE;
}

static GLboolean opengl_invert_matrix_3d_no_rot(struct opengl_matrix_t *mtx)
{
	const GLfloat *in = mtx->matrix;
	GLfloat *out = mtx->matinv;

	if (MAT(in,0,0) == 0 || MAT(in,1,1) == 0 || MAT(in,2,2) == 0 )
		return GL_FALSE;

	memcpy( out, Identity, 16 * sizeof(GLfloat) );
	MAT(out,0,0) = 1.0F / MAT(in,0,0);
	MAT(out,1,1) = 1.0F / MAT(in,1,1);
	MAT(out,2,2) = 1.0F / MAT(in,2,2);

	if (mtx->flags & MAT_FLAG_TRANSLATION)
	{
		MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0));
		MAT(out,1,3) = - (MAT(in,1,3) * MAT(out,1,1));
		MAT(out,2,3) = - (MAT(in,2,3) * MAT(out,2,2));
	}

	return GL_TRUE;
}

static GLboolean opengl_invert_matrix_2d_no_rot(struct opengl_matrix_t *mtx)
{
	const GLfloat *in = mtx->matrix;
	GLfloat *out = mtx->matinv;

	if (MAT(in,0,0) == 0 || MAT(in,1,1) == 0)
	return GL_FALSE;

	memcpy( out, Identity, 16 * sizeof(GLfloat) );
	MAT(out,0,0) = 1.0F / MAT(in,0,0);
	MAT(out,1,1) = 1.0F / MAT(in,1,1);

	if (mtx->flags & MAT_FLAG_TRANSLATION)
	{
		MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0));
		MAT(out,1,3) = - (MAT(in,1,3) * MAT(out,1,1));
	}

	return GL_TRUE;
}
#undef MAT

typedef GLboolean (*inv_mat_func)(struct opengl_matrix_t *mtx);

static inv_mat_func inv_mat_tab[7] = {
	opengl_invert_matrix_general,
	opengl_invert_matrix_identity,
	opengl_invert_matrix_3d_no_rot,
	#if 0
	/* Don't use this function for now - it fails when the projection matrix
	* is premultiplied by a translation (ala Chromium's tilesort SPU).
	*/
	opengl_invert_matrix_perspective,
	#else
	opengl_invert_matrix_general,
	#endif
	opengl_invert_matrix_3d,	
	opengl_invert_matrix_2d_no_rot,
	opengl_invert_matrix_3d
};

GLboolean opengl_matrix_invert(struct opengl_matrix_t *mtx)
{
	if (inv_mat_tab[mtx->matrix_mode](mtx))
	{
		mtx->flags &= ~MAT_FLAG_SINGULAR;
		return GL_TRUE;
	} 
	else
	{
		mtx->flags |= MAT_FLAG_SINGULAR;
		memcpy( mtx->matinv, Identity, sizeof(Identity) );
		return GL_FALSE;
	}	
}

GLboolean opengl_matrix_is_dirty(const struct opengl_matrix_t *mtx)
{
	return (mtx->flags & MAT_DIRTY) ? GL_TRUE : GL_FALSE;
}


/* Matrix analysis */
#define ZERO(x) (1<<x)
#define ONE(x)  (1<<(x+16))

#define MASK_NO_TRX      (ZERO(12) | ZERO(13) | ZERO(14))
#define MASK_NO_2D_SCALE ( ONE(0)  | ONE(5))

#define MASK_IDENTITY    ( ONE(0)  | ZERO(4)  | ZERO(8)  | ZERO(12) |\
			  ZERO(1)  |  ONE(5)  | ZERO(9)  | ZERO(13) |\
			  ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_2D_NO_ROT   (           ZERO(4)  | ZERO(8)  |           \
			  ZERO(1)  |            ZERO(9)  |           \
			  ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_2D          (                      ZERO(8)  |           \
			                        ZERO(9)  |           \
			  ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )


#define MASK_3D_NO_ROT   (           ZERO(4)  | ZERO(8)  |           \
			  ZERO(1)  |            ZERO(9)  |           \
			  ZERO(2)  | ZERO(6)  |                      \
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_3D          (                                           \
			                                             \
			                                             \
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )


#define MASK_PERSPECTIVE (           ZERO(4)  |            ZERO(12) |\
			  ZERO(1)  |                       ZERO(13) |\
			  ZERO(2)  | ZERO(6)  |                      \
			  ZERO(3)  | ZERO(7)  |            ZERO(15) )

#define SQ(x) ((x)*(x))


/**
 * Determine type and flags from scratch.  
 *
 * \param mtx matrix.
 * 
 * This is expensive enough to only want to do it once.
 */
static void opengl_analyse_from_scratch( struct opengl_matrix_t *mtx )
{
	const GLfloat *m = mtx->matrix;
	GLuint mask = 0;
	GLuint i;

	for (i = 0 ; i < 16 ; i++) {
	if (m[i] == 0.0) mask |= (1<<i);
	}

	if (m[0] == 1.0F) mask |= (1<<16);
	if (m[5] == 1.0F) mask |= (1<<21);
	if (m[10] == 1.0F) mask |= (1<<26);
	if (m[15] == 1.0F) mask |= (1<<31);

	mtx->flags &= ~MAT_FLAGS_GEOMETRY;

	/* Check for translation - no-one really cares
	*/
	if ((mask & MASK_NO_TRX) != MASK_NO_TRX)
		mtx->flags |= MAT_FLAG_TRANSLATION;

	/* Do the real work
	*/
	if (mask == (GLuint) MASK_IDENTITY)
		mtx->matrix_mode = MATRIX_IDENTITY;
	else if ((mask & MASK_2D_NO_ROT) == (GLuint) MASK_2D_NO_ROT) 
	{
		mtx->matrix_mode = MATRIX_2D_NO_ROT;

		if ((mask & MASK_NO_2D_SCALE) != MASK_NO_2D_SCALE)
		mtx->flags |= MAT_FLAG_GENERAL_SCALE;
	}
	else if ((mask & MASK_2D) == (GLuint) MASK_2D)
	{
		GLfloat mm = DOT2(m, m);
		GLfloat m4m4 = DOT2(m+4,m+4);
		GLfloat mm4 = DOT2(m,m+4);

		mtx->matrix_mode = MATRIX_2D;

		/* Check for scale */
		if (SQ(mm-1) > SQ(1e-6) || SQ(m4m4-1) > SQ(1e-6))
			mtx->flags |= MAT_FLAG_GENERAL_SCALE;

		/* Check for rotation */
		if (SQ(mm4) > SQ(1e-6))
			mtx->flags |= MAT_FLAG_GENERAL_3D;
		else
			mtx->flags |= MAT_FLAG_ROTATION;

	}
	else if ((mask & MASK_3D_NO_ROT) == (GLuint) MASK_3D_NO_ROT)
	{
		mtx->matrix_mode = MATRIX_3D_NO_ROT;

		/* Check for scale */
		if (SQ(m[0]-m[5]) < SQ(1e-6) && SQ(m[0]-m[10]) < SQ(1e-6)) 
		{
			if (SQ(m[0]-1.0) > SQ(1e-6))
				mtx->flags |= MAT_FLAG_UNIFORM_SCALE;
		}
		else
		{
			mtx->flags |= MAT_FLAG_GENERAL_SCALE;
		}
	}
	else if ((mask & MASK_3D) == (GLuint) MASK_3D)
	{
		GLfloat c1 = DOT3(m,m);
		GLfloat c2 = DOT3(m+4,m+4);
		GLfloat c3 = DOT3(m+8,m+8);
		GLfloat d1 = DOT3(m, m+4);
		GLfloat cp[3];

		mtx->matrix_mode = MATRIX_3D;

		/* Check for scale */
		if (SQ(c1-c2) < SQ(1e-6) && SQ(c1-c3) < SQ(1e-6))
		{
			if (SQ(c1-1.0) > SQ(1e-6))
				mtx->flags |= MAT_FLAG_UNIFORM_SCALE;
			/* else no scale at all */
		}
		else {
			mtx->flags |= MAT_FLAG_GENERAL_SCALE;
		}

		/* Check for rotation */
		if (SQ(d1) < SQ(1e-6))
		{
			CROSS3( cp, m, m+4 );
			SUB_3V( cp, cp, (m+8) );
			if (LEN_SQUARED_3FV(cp) < SQ(1e-6))
				mtx->flags |= MAT_FLAG_ROTATION;
			else
				mtx->flags |= MAT_FLAG_GENERAL_3D;
		}
		else 
		{
			mtx->flags |= MAT_FLAG_GENERAL_3D; /* shear, etc */
		}
	}
	else if ((mask & MASK_PERSPECTIVE) == MASK_PERSPECTIVE && m[11]==-1.0F)
	{
		mtx->matrix_mode = MATRIX_PERSPECTIVE;
		mtx->flags |= MAT_FLAG_GENERAL;
	}
	else
	{
		mtx->matrix_mode = MATRIX_GENERAL;
		mtx->flags |= MAT_FLAG_GENERAL;
	}
}

/**
 * Analyze a matrix given that its flags are accurate.
 * 
 * This is the more common operation, hopefully.
 */
static void opengl_analyse_from_flags( struct opengl_matrix_t *mtx )
{
	const GLfloat *m = mtx->matrix;

	if (TEST_MAT_FLAGS(mtx, 0)) {
		mtx->matrix_mode = MATRIX_IDENTITY;
	}
	else if (TEST_MAT_FLAGS(mtx, (MAT_FLAG_TRANSLATION |
					MAT_FLAG_UNIFORM_SCALE |
					MAT_FLAG_GENERAL_SCALE))) {
		if ( m[10]==1.0F && m[14]==0.0F ) {
			mtx->matrix_mode = MATRIX_2D_NO_ROT;
		}
		else {
			mtx->matrix_mode = MATRIX_3D_NO_ROT;
		}
	}
	else if (TEST_MAT_FLAGS(mtx, MAT_FLAGS_3D)) {
		if (                                 m[ 8]==0.0F
				&&                             m[ 9]==0.0F
				&& m[2]==0.0F && m[6]==0.0F && m[10]==1.0F && m[14]==0.0F) {
			mtx->matrix_mode = MATRIX_2D;
		}
		else {
			mtx->matrix_mode = MATRIX_3D;
		}
	}
	else if (                 m[4]==0.0F                 && m[12]==0.0F
			&& m[1]==0.0F                               && m[13]==0.0F
			&& m[2]==0.0F && m[6]==0.0F
			&& m[3]==0.0F && m[7]==0.0F && m[11]==-1.0F && m[15]==0.0F) {
		mtx->matrix_mode = MATRIX_PERSPECTIVE;
	}
	else {
		mtx->matrix_mode = MATRIX_GENERAL;
	}
}

/**
 * Analyze and update a matrix.
 *
 * \param mtx matrix.
 *
 * If the matrix type is dirty then calls either analyse_from_scratch() or
 * analyse_from_flags() to determine its type, according to whether the flags
 * are dirty or not, respectively. If the matrix has an inverse and it's dirty
 * then calls matrix_invert(). Finally clears the dirty flags.
 */
void opengl_matrix_analyse(struct opengl_matrix_t *mtx)
{
	if (mtx->flags & MAT_DIRTY_TYPE)
	{
		if (mtx->flags & MAT_DIRTY_FLAGS)
			opengl_analyse_from_scratch( mtx );
		else
			opengl_analyse_from_flags( mtx );
	}

	if (mtx->matinv && (mtx->flags & MAT_DIRTY_INVERSE)) {
		opengl_matrix_invert( mtx );
		mtx->flags &= ~MAT_DIRTY_INVERSE;
	}

	mtx->flags &= ~(MAT_DIRTY_FLAGS | MAT_DIRTY_TYPE);
}
