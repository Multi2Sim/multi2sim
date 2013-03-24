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

#ifndef DRIVER_OPENGL_VECTOR_H
#define DRIVER_OPENGL_VECTOR_H


#include <GL/glut.h>

/* FIXME: move below MACROS so they can be reused */
/* Copy from Mesa macro.h and import.h */

/** single-precision inverse square root */
static inline float
INV_SQRTF(float x)
{
   /* XXX we could try Quake's fast inverse square root function here */
   return 1.0F / sqrtf(x);
}

/**********************************************************************/
/** \name 4-element vector operations */
/*@{*/

/** Zero */
#define ZERO_4V( DST )  (DST)[0] = (DST)[1] = (DST)[2] = (DST)[3] = 0

/** Test for equality */
#define TEST_EQ_4V(a,b)  ((a)[0] == (b)[0] &&   \
              (a)[1] == (b)[1] &&   \
              (a)[2] == (b)[2] &&   \
              (a)[3] == (b)[3])

/** Test for equality (unsigned bytes) */
static inline GLboolean
TEST_EQ_4UBV(const GLubyte a[4], const GLubyte b[4])
{
#if defined(__i386__)
   return *((const GLuint *) a) == *((const GLuint *) b);
#else
   return TEST_EQ_4V(a, b);
#endif
}


/** Copy a 4-element vector */
#define COPY_4V( DST, SRC )         \
do {                                \
   (DST)[0] = (SRC)[0];             \
   (DST)[1] = (SRC)[1];             \
   (DST)[2] = (SRC)[2];             \
   (DST)[3] = (SRC)[3];             \
} while (0)

/** Copy a 4-element unsigned byte vector */
static inline void
COPY_4UBV(GLubyte dst[4], const GLubyte src[4])
{
#if defined(__i386__)
   *((GLuint *) dst) = *((GLuint *) src);
#else
   /* The GLuint cast might fail if DST or SRC are not dword-aligned (RISC) */
   COPY_4V(dst, src);
#endif
}

/** Copy a 4-element float vector */
static inline void
COPY_4FV(GLfloat dst[4], const GLfloat src[4])
{
   /* memcpy seems to be most efficient */
   memcpy(dst, src, sizeof(GLfloat) * 4);
}

/** Copy \p SZ elements into a 4-element vector */
#define COPY_SZ_4V(DST, SZ, SRC)  \
do {                              \
   switch (SZ) {                  \
   case 4: (DST)[3] = (SRC)[3];   \
   case 3: (DST)[2] = (SRC)[2];   \
   case 2: (DST)[1] = (SRC)[1];   \
   case 1: (DST)[0] = (SRC)[0];   \
   }                              \
} while(0)

/** Copy \p SZ elements into a homegeneous (4-element) vector, giving
 * default values to the remaining */
#define COPY_CLEAN_4V(DST, SZ, SRC)  \
do {                                 \
      ASSIGN_4V( DST, 0, 0, 0, 1 );  \
      COPY_SZ_4V( DST, SZ, SRC );    \
} while (0)

/** Subtraction */
#define SUB_4V( DST, SRCA, SRCB )           \
do {                                        \
      (DST)[0] = (SRCA)[0] - (SRCB)[0];     \
      (DST)[1] = (SRCA)[1] - (SRCB)[1];     \
      (DST)[2] = (SRCA)[2] - (SRCB)[2];     \
      (DST)[3] = (SRCA)[3] - (SRCB)[3];     \
} while (0)

/** Addition */
#define ADD_4V( DST, SRCA, SRCB )           \
do {                                        \
      (DST)[0] = (SRCA)[0] + (SRCB)[0];     \
      (DST)[1] = (SRCA)[1] + (SRCB)[1];     \
      (DST)[2] = (SRCA)[2] + (SRCB)[2];     \
      (DST)[3] = (SRCA)[3] + (SRCB)[3];     \
} while (0)

/** Element-wise multiplication */
#define SCALE_4V( DST, SRCA, SRCB )         \
do {                                        \
      (DST)[0] = (SRCA)[0] * (SRCB)[0];     \
      (DST)[1] = (SRCA)[1] * (SRCB)[1];     \
      (DST)[2] = (SRCA)[2] * (SRCB)[2];     \
      (DST)[3] = (SRCA)[3] * (SRCB)[3];     \
} while (0)

/** In-place addition */
#define ACC_4V( DST, SRC )          \
do {                                \
      (DST)[0] += (SRC)[0];         \
      (DST)[1] += (SRC)[1];         \
      (DST)[2] += (SRC)[2];         \
      (DST)[3] += (SRC)[3];         \
} while (0)

/** Element-wise multiplication and addition */
#define ACC_SCALE_4V( DST, SRCA, SRCB )     \
do {                                        \
      (DST)[0] += (SRCA)[0] * (SRCB)[0];    \
      (DST)[1] += (SRCA)[1] * (SRCB)[1];    \
      (DST)[2] += (SRCA)[2] * (SRCB)[2];    \
      (DST)[3] += (SRCA)[3] * (SRCB)[3];    \
} while (0)

/** In-place scalar multiplication and addition */
#define ACC_SCALE_SCALAR_4V( DST, S, SRCB ) \
do {                                        \
      (DST)[0] += S * (SRCB)[0];            \
      (DST)[1] += S * (SRCB)[1];            \
      (DST)[2] += S * (SRCB)[2];            \
      (DST)[3] += S * (SRCB)[3];            \
} while (0)

/** Scalar multiplication */
#define SCALE_SCALAR_4V( DST, S, SRCB ) \
do {                                    \
      (DST)[0] = S * (SRCB)[0];         \
      (DST)[1] = S * (SRCB)[1];         \
      (DST)[2] = S * (SRCB)[2];         \
      (DST)[3] = S * (SRCB)[3];         \
} while (0)

/** In-place scalar multiplication */
#define SELF_SCALE_SCALAR_4V( DST, S ) \
do {                                   \
      (DST)[0] *= S;                   \
      (DST)[1] *= S;                   \
      (DST)[2] *= S;                   \
      (DST)[3] *= S;                   \
} while (0)

/** Assignment */
#define ASSIGN_4V( V, V0, V1, V2, V3 )  \
do {                                    \
    V[0] = V0;                          \
    V[1] = V1;                          \
    V[2] = V2;                          \
    V[3] = V3;                          \
} while(0)

/*@}*/


/**********************************************************************/
/** \name 3-element vector operations*/
/*@{*/

/** Zero */
#define ZERO_3V( DST )  (DST)[0] = (DST)[1] = (DST)[2] = 0

/** Test for equality */
#define TEST_EQ_3V(a,b)  \
   ((a)[0] == (b)[0] &&  \
    (a)[1] == (b)[1] &&  \
    (a)[2] == (b)[2])

/** Copy a 3-element vector */
#define COPY_3V( DST, SRC )         \
do {                                \
   (DST)[0] = (SRC)[0];             \
   (DST)[1] = (SRC)[1];             \
   (DST)[2] = (SRC)[2];             \
} while (0)

/** Copy a 3-element vector with cast */
#define COPY_3V_CAST( DST, SRC, CAST )  \
do {                                    \
   (DST)[0] = (CAST)(SRC)[0];           \
   (DST)[1] = (CAST)(SRC)[1];           \
   (DST)[2] = (CAST)(SRC)[2];           \
} while (0)

/** Copy a 3-element float vector */
#define COPY_3FV( DST, SRC )        \
do {                                \
   const GLfloat *_tmp = (SRC);     \
   (DST)[0] = _tmp[0];              \
   (DST)[1] = _tmp[1];              \
   (DST)[2] = _tmp[2];              \
} while (0)

/** Subtraction */
#define SUB_3V( DST, SRCA, SRCB )        \
do {                                     \
      (DST)[0] = (SRCA)[0] - (SRCB)[0];  \
      (DST)[1] = (SRCA)[1] - (SRCB)[1];  \
      (DST)[2] = (SRCA)[2] - (SRCB)[2];  \
} while (0)

/** Addition */
#define ADD_3V( DST, SRCA, SRCB )       \
do {                                    \
      (DST)[0] = (SRCA)[0] + (SRCB)[0]; \
      (DST)[1] = (SRCA)[1] + (SRCB)[1]; \
      (DST)[2] = (SRCA)[2] + (SRCB)[2]; \
} while (0)

/** In-place scalar multiplication */
#define SCALE_3V( DST, SRCA, SRCB )     \
do {                                    \
      (DST)[0] = (SRCA)[0] * (SRCB)[0]; \
      (DST)[1] = (SRCA)[1] * (SRCB)[1]; \
      (DST)[2] = (SRCA)[2] * (SRCB)[2]; \
} while (0)

/** In-place element-wise multiplication */
#define SELF_SCALE_3V( DST, SRC )   \
do {                                \
      (DST)[0] *= (SRC)[0];         \
      (DST)[1] *= (SRC)[1];         \
      (DST)[2] *= (SRC)[2];         \
} while (0)

/** In-place addition */
#define ACC_3V( DST, SRC )          \
do {                                \
      (DST)[0] += (SRC)[0];         \
      (DST)[1] += (SRC)[1];         \
      (DST)[2] += (SRC)[2];         \
} while (0)

/** Element-wise multiplication and addition */
#define ACC_SCALE_3V( DST, SRCA, SRCB )     \
do {                                        \
      (DST)[0] += (SRCA)[0] * (SRCB)[0];    \
      (DST)[1] += (SRCA)[1] * (SRCB)[1];    \
      (DST)[2] += (SRCA)[2] * (SRCB)[2];    \
} while (0)

/** Scalar multiplication */
#define SCALE_SCALAR_3V( DST, S, SRCB ) \
do {                                    \
      (DST)[0] = S * (SRCB)[0];         \
      (DST)[1] = S * (SRCB)[1];         \
      (DST)[2] = S * (SRCB)[2];         \
} while (0)

/** In-place scalar multiplication and addition */
#define ACC_SCALE_SCALAR_3V( DST, S, SRCB ) \
do {                                        \
      (DST)[0] += S * (SRCB)[0];            \
      (DST)[1] += S * (SRCB)[1];            \
      (DST)[2] += S * (SRCB)[2];            \
} while (0)

/** In-place scalar multiplication */
#define SELF_SCALE_SCALAR_3V( DST, S ) \
do {                                   \
      (DST)[0] *= S;                   \
      (DST)[1] *= S;                   \
      (DST)[2] *= S;                   \
} while (0)

/** In-place scalar addition */
#define ACC_SCALAR_3V( DST, S )     \
do {                                \
      (DST)[0] += S;                \
      (DST)[1] += S;                \
      (DST)[2] += S;                \
} while (0)

/** Assignment */
#define ASSIGN_3V( V, V0, V1, V2 )  \
do {                                \
    V[0] = V0;                      \
    V[1] = V1;                      \
    V[2] = V2;                      \
} while(0)

/*@}*/


/**********************************************************************/
/** \name 2-element vector operations*/
/*@{*/

/** Zero */
#define ZERO_2V( DST )  (DST)[0] = (DST)[1] = 0

/** Copy a 2-element vector */
#define COPY_2V( DST, SRC )         \
do {                        \
   (DST)[0] = (SRC)[0];             \
   (DST)[1] = (SRC)[1];             \
} while (0)

/** Copy a 2-element vector with cast */
#define COPY_2V_CAST( DST, SRC, CAST )      \
do {                        \
   (DST)[0] = (CAST)(SRC)[0];           \
   (DST)[1] = (CAST)(SRC)[1];           \
} while (0)

/** Copy a 2-element float vector */
#define COPY_2FV( DST, SRC )            \
do {                        \
   const GLfloat *_tmp = (SRC);         \
   (DST)[0] = _tmp[0];              \
   (DST)[1] = _tmp[1];              \
} while (0)

/** Subtraction */
#define SUB_2V( DST, SRCA, SRCB )       \
do {                        \
      (DST)[0] = (SRCA)[0] - (SRCB)[0];     \
      (DST)[1] = (SRCA)[1] - (SRCB)[1];     \
} while (0)

/** Addition */
#define ADD_2V( DST, SRCA, SRCB )       \
do {                        \
      (DST)[0] = (SRCA)[0] + (SRCB)[0];     \
      (DST)[1] = (SRCA)[1] + (SRCB)[1];     \
} while (0)

/** In-place scalar multiplication */
#define SCALE_2V( DST, SRCA, SRCB )     \
do {                        \
      (DST)[0] = (SRCA)[0] * (SRCB)[0];     \
      (DST)[1] = (SRCA)[1] * (SRCB)[1];     \
} while (0)

/** In-place addition */
#define ACC_2V( DST, SRC )          \
do {                        \
      (DST)[0] += (SRC)[0];         \
      (DST)[1] += (SRC)[1];         \
} while (0)

/** Element-wise multiplication and addition */
#define ACC_SCALE_2V( DST, SRCA, SRCB )     \
do {                        \
      (DST)[0] += (SRCA)[0] * (SRCB)[0];    \
      (DST)[1] += (SRCA)[1] * (SRCB)[1];    \
} while (0)

/** Scalar multiplication */
#define SCALE_SCALAR_2V( DST, S, SRCB )     \
do {                        \
      (DST)[0] = S * (SRCB)[0];         \
      (DST)[1] = S * (SRCB)[1];         \
} while (0)

/** In-place scalar multiplication and addition */
#define ACC_SCALE_SCALAR_2V( DST, S, SRCB ) \
do {                        \
      (DST)[0] += S * (SRCB)[0];        \
      (DST)[1] += S * (SRCB)[1];        \
} while (0)

/** In-place scalar multiplication */
#define SELF_SCALE_SCALAR_2V( DST, S )      \
do {                        \
      (DST)[0] *= S;                \
      (DST)[1] *= S;                \
} while (0)

/** In-place scalar addition */
#define ACC_SCALAR_2V( DST, S )         \
do {                        \
      (DST)[0] += S;                \
      (DST)[1] += S;                \
} while (0)

/** Assign scalers to short vectors */
#define ASSIGN_2V( V, V0, V1 )	\
do {				\
    V[0] = V0;			\
    V[1] = V1;			\
} while(0)

/*@}*/

/** Copy \p sz elements into a homegeneous (4-element) vector, giving
 * default values to the remaining components.
 * The default values are chosen based on \p type.
 */
/*static inline void
COPY_CLEAN_4V_TYPE_AS_FLOAT(GLfloat dst[4], int sz, const GLfloat src[4],
                            GLenum type)
{
   switch (type) {
   case GL_FLOAT:
      ASSIGN_4V(dst, 0, 0, 0, 1);
      break;
   case GL_INT:
      ASSIGN_4V(dst, INT_AS_FLT(0), INT_AS_FLT(0),
                     INT_AS_FLT(0), INT_AS_FLT(1));
      break;
   case GL_UNSIGNED_INT:
      ASSIGN_4V(dst, UINT_AS_FLT(0), UINT_AS_FLT(0),
                     UINT_AS_FLT(0), UINT_AS_FLT(1));
      break;
   default:
      ASSERT(0);
   }
   COPY_SZ_4V(dst, sz, src);
}
*/
/** \name Linear interpolation functions */
/*@{*/

static inline GLfloat
LINTERP(GLfloat t, GLfloat out, GLfloat in)
{
   return out + t * (in - out);
}

static inline void
INTERP_3F(GLfloat t, GLfloat dst[3], const GLfloat out[3], const GLfloat in[3])
{
   dst[0] = LINTERP( t, out[0], in[0] );
   dst[1] = LINTERP( t, out[1], in[1] );
   dst[2] = LINTERP( t, out[2], in[2] );
}

static inline void
INTERP_4F(GLfloat t, GLfloat dst[4], const GLfloat out[4], const GLfloat in[4])
{
   dst[0] = LINTERP( t, out[0], in[0] );
   dst[1] = LINTERP( t, out[1], in[1] );
   dst[2] = LINTERP( t, out[2], in[2] );
   dst[3] = LINTERP( t, out[3], in[3] );
}

/*@}*/



/** Clamp X to [MIN,MAX] */
#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )

/** Minimum of two values: */
#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )

/** Maximum of two values: */
#define MAX2( A, B )   ( (A)>(B) ? (A) : (B) )

/** Minimum and maximum of three values: */
#define MIN3( A, B, C ) ((A) < (B) ? MIN2(A, C) : MIN2(B, C))
#define MAX3( A, B, C ) ((A) > (B) ? MAX2(A, C) : MAX2(B, C))



/** Cross product of two 3-element vectors */
static inline void
CROSS3(GLfloat n[3], const GLfloat u[3], const GLfloat v[3])
{
   n[0] = u[1] * v[2] - u[2] * v[1];
   n[1] = u[2] * v[0] - u[0] * v[2];
   n[2] = u[0] * v[1] - u[1] * v[0];
}


/** Dot product of two 2-element vectors */
static inline GLfloat
DOT2(const GLfloat a[2], const GLfloat b[2])
{
   return a[0] * b[0] + a[1] * b[1];
}

static inline GLfloat
DOT3(const GLfloat a[3], const GLfloat b[3])
{
   return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline GLfloat
DOT4(const GLfloat a[4], const GLfloat b[4])
{
   return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}


static inline GLfloat
LEN_SQUARED_3FV(const GLfloat v[3])
{
   return DOT3(v, v);
}

static inline GLfloat
LEN_SQUARED_2FV(const GLfloat v[2])
{
   return DOT2(v, v);
}


static inline GLfloat
LEN_3FV(const GLfloat v[3])
{
   return sqrtf(LEN_SQUARED_3FV(v));
}

static inline GLfloat
LEN_2FV(const GLfloat v[2])
{
   return sqrtf(LEN_SQUARED_2FV(v));
}


/* Normalize a 3-element vector to unit length. */
static inline void
NORMALIZE_3FV(GLfloat v[3])
{
   GLfloat len = (GLfloat) LEN_SQUARED_3FV(v);
   if (len) {
      len = INV_SQRTF(len);
      v[0] *= len;
      v[1] *= len;
      v[2] *= len;
   }
}

GLfloat *opengl_vector_create(int len);
void opengl_vector_free(GLfloat *vct);
void opengl_vector_unit(GLfloat *dst, GLfloat *p1, GLfloat *p2);

 #endif
