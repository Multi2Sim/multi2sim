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

#ifndef DRIVER_OPENGL_RASTERIZER_H
#define DRIVER_OPENGL_RASTERIZER_H

#include <GL/glut.h>


typedef GLint GLfixed;

#define MAX_GLUINT	0xffffffff

/* Convert float to int by rounding to nearest integer, away from zero */
static inline int IROUND(float f)
{
   return (int) ((f >= 0.0F) ? (f + 0.5F) : (f - 0.5F));
}

#define SUB_PIXEL_BITS 4

#define SWRAST_MAX_WIDTH 16384

/* Fixed point arithmetic macros */
#ifndef FIXED_FRAC_BITS
#define FIXED_FRAC_BITS 11
#endif

#define FIXED_SHIFT     FIXED_FRAC_BITS
#define FIXED_ONE       (1 << FIXED_SHIFT)
#define FIXED_HALF      (1 << (FIXED_SHIFT-1))
#define FIXED_FRAC_MASK (FIXED_ONE - 1)
#define FIXED_INT_MASK  (~FIXED_FRAC_MASK)
#define FIXED_EPSILON   1
#define FIXED_SCALE     ((float) FIXED_ONE)
#define FIXED_DBL_SCALE ((double) FIXED_ONE)
#define FloatToFixed(X) (IROUND((X) * FIXED_SCALE))
#define FixedToDouble(X) ((X) * (1.0 / FIXED_DBL_SCALE))
#define IntToFixed(I)   ((I) << FIXED_SHIFT)
#define FixedToInt(X)   ((X) >> FIXED_SHIFT)
#define FixedToUns(X)   (((unsigned int)(X)) >> FIXED_SHIFT)
#define FixedCeil(X)    (((X) + FIXED_ONE - FIXED_EPSILON) & FIXED_INT_MASK)
#define FixedFloor(X)   ((X) & FIXED_INT_MASK)
#define FixedToFloat(X) ((X) * (1.0F / FIXED_SCALE))
#define PosFloatToFixed(X)      FloatToFixed(X)
#define SignedFloatToFixed(X)   FloatToFixed(X)

struct opengl_context_t;
struct opengl_vertex_t;

void opengl_rasterizer_draw_triangle(struct opengl_context_t *ctx, struct opengl_vertex_t *vtx0, struct opengl_vertex_t *vtx1, struct opengl_vertex_t *vtx2);
/* Bresenham's line algorithm */
void opengl_rasterizer_draw_line(struct opengl_context_t *ctx, GLint x1, GLint y1, GLint x2, GLint y2, GLuint color);

#endif
