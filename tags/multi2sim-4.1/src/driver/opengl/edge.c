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

#include "edge.h"
#include "opengl.h"
#include "vertex.h"


struct opengl_edge_t *opengl_edge_create(struct opengl_vertex_t *vtx0,
		struct opengl_vertex_t *vtx1)
{
	struct opengl_edge_t * edge;

	edge = xcalloc(1, sizeof(struct opengl_edge_t));

	/* Initialize */
	edge->vtx0 = vtx0;
	edge->vtx1 = vtx1;
	opengl_debug("\t\tEdge \t[%f, %f] - [%f, %f]\n\n",
			vtx0->pos[X_COMP], vtx0->pos[Y_COMP],
			vtx1->pos[X_COMP], vtx1->pos[Y_COMP]);

	/* Return */
	return edge;
}

void opengl_edge_free(struct opengl_edge_t *edge)
{
	free(edge);
}
