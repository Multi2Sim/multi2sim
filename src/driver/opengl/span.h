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

#ifndef DRIVER_OPENGL_SPAN_H
#define DRIVER_OPENGL_SPAN_H

#include <GL/glut.h>

#include "context.h"
#include "rasterizer.h"


struct opengl_span_array_t
{
	GLuint  z[SWRAST_MAX_WIDTH];  /**< fragment Z coords */	
};

struct opengl_span_t
{
	/* Coord of first fragment in horizontal span/run */
	GLint x;
	GLint y;

	/* Number of fragments in the span */
	GLuint end;

	GLfloat attrStart[FRAG_ATTRIB_MAX][4];   /**< initial value */
	GLfloat attrStepX[FRAG_ATTRIB_MAX][4];   /**< dvalue/dx */
	GLfloat attrStepY[FRAG_ATTRIB_MAX][4];   /**< dvalue/dy */

	GLfixed red, redStep;
	GLfixed green, greenStep;
	GLfixed blue, blueStep;
	GLfixed alpha, alphaStep;
	GLfixed z, zStep;

	struct opengl_span_array_t *array;

};

struct opengl_span_t *opengl_span_create();
void opengl_span_free(struct opengl_span_t *spn);

void opengl_span_interpolate_z(struct opengl_span_t *spn);

#endif
