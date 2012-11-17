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

#include "opengl-span.h"


struct x86_opengl_span_t *x86_opengl_span_create()
{
	struct x86_opengl_span_t *spn;

	spn = xcalloc(1, sizeof(struct x86_opengl_span_t));

	return spn;
}

void x86_opengl_span_free(struct x86_opengl_span_t *spn)
{
	free(spn);
}

void x86_opengl_span_interpolate_z(struct x86_opengl_span_t *spn)
{
	const GLuint n = spn->end;
	GLuint i;

	/* Deep Z buffer, no fixed->int shift */
	GLuint zval = spn->z;
	GLuint *z = spn->array->z;
	for (i = 0; i < n; i++) 
	{
		z[i] = zval;
		zval += spn->zStep;
	}

}
