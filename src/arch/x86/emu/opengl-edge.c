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
#include <lib/struct/debug.h>

#include "opengl-edge.h"


struct x86_opengl_edge_t *x86_opengl_edge_create(struct x86_opengl_vertex_t *vtx0, struct x86_opengl_vertex_t *vtx1)
{
	struct x86_opengl_edge_t * edge;

	edge = calloc(1, sizeof(struct x86_opengl_edge_t));
	if (!edge)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	edge->vtx0 = vtx0;
	edge->vtx1 = vtx1;
	edge->dx = vtx1->x - vtx0->x;
	edge->dy = vtx1->y - vtx0->y;
	edge->dxdy = (GLfloat) edge->dx / edge->dy;
	edge->fdxdy = SignedFloatToFixed(edge->dxdy);
	x86_opengl_debug("\t\tEdge \t[%f, %f] - [%f, %f]\n\t\t\tdx = %f, dy = %f \n\t\t\tdxdy = %f, fdxdy = %d\n", 
		vtx0->x, vtx0->y, vtx1->x, vtx1->y,
		edge->dx, edge->dy,	edge->dxdy, edge->fdxdy);

	/* Return */
	return edge;
}

void x86_opengl_edge_free(struct x86_opengl_edge_t *edge)
{
	free(edge);
}
