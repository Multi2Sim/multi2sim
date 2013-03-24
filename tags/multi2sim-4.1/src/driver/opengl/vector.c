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
#include <math.h>

#include "vector.h"


GLfloat *opengl_vector_create(int len)
{
	GLfloat *vct;

	/* Malloc */
	vct = xcalloc(1, len * sizeof(GLfloat));

	/* Return */
	return vct;
}

void opengl_vector_free(GLfloat *vct)
{
	free(vct);
}

/* Calculate the unit vector used in lighting module */
/* Only 3 elements are used, the 4th is ignored */
void opengl_vector_unit(GLfloat *dst, GLfloat *p1, GLfloat *p2)
{
	if (p1[3] == 0 && p2[3] !=0)
		ACC_SCALE_SCALAR_3V(dst, -1.0f, p1);
	else if (p1[3] != 0 && p2[3] ==0)
		COPY_3V(dst, p2);
	else if (p1[3] == 0 && p2[3] == 0)
		SUB_3V(dst, p2, p1);
	else
		SUB_3V(dst, p2, p1);
	
	NORMALIZE_3FV(dst);
}

