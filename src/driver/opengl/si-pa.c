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
#include <lib/util/misc.h>

#include "si-pa.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */

struct opengl_pa_vertex_t *opengl_pa_vertex_create()
{
	struct opengl_pa_vertex_t *vtx;

	/* Allocate */
	vtx = xcalloc(1, sizeof(struct opengl_pa_vertex_t));

	/* Return */	
	return vtx;
}

void opengl_pa_vertex_free(struct opengl_pa_vertex_t *vtx)
{
	free(vtx);
}

struct opengl_pa_triangle_t *opengl_pa_triangle_create()
{
	struct opengl_pa_triangle_t *triangle;

	/* Allocate */
	triangle = xcalloc(1, sizeof(struct opengl_pa_triangle_t));

	/* Return */
	return triangle;
}

void opengl_pa_triangle_set(struct opengl_pa_triangle_t *triangle, 
	struct opengl_pa_vertex_t *vtx0,
	struct opengl_pa_vertex_t *vtx1,
	struct opengl_pa_vertex_t *vtx2)
{
	triangle->vtx0 = vtx0;
	triangle->vtx1 = vtx1;
	triangle->vtx2 = vtx2;
	triangle->edgfunc0 = opengl_pa_edge_func_create();
	triangle->edgfunc1 = opengl_pa_edge_func_create();
	triangle->edgfunc2 = opengl_pa_edge_func_create();
	opengl_pa_edge_func_set(triangle->edgfunc0, vtx0, vtx1);
	opengl_pa_edge_func_set(triangle->edgfunc1, vtx1, vtx2);
	opengl_pa_edge_func_set(triangle->edgfunc2, vtx2, vtx0);
}

void opengl_pa_triangle_free(struct opengl_pa_triangle_t *triangle)
{
	opengl_pa_edge_func_free(triangle->edgfunc0);
	opengl_pa_edge_func_free(triangle->edgfunc1);
	opengl_pa_edge_func_free(triangle->edgfunc2);
	free(triangle);
}

/* Edge function is used to test if a pixel is in the right side of an edge */
struct opengl_pa_edge_func_t *opengl_pa_edge_func_create()
{
	struct opengl_pa_edge_func_t *edge_func;

	/* Allocate */
	edge_func = xcalloc(1, sizeof(struct opengl_pa_edge_func_t));

	/* Return */	
	return edge_func;
}

void opengl_pa_edge_func_free(struct opengl_pa_edge_func_t *edge_func)
{
	free(edge_func);
}

/* vtx0/1 are 2 homegenious points */
void opengl_pa_edge_func_set(struct opengl_pa_edge_func_t *edge_func,struct opengl_pa_vertex_t *vtx0, struct opengl_pa_vertex_t *vtx1)
{

	edge_func->a = vtx0->pos[Y_COMP] - vtx1->pos[Y_COMP];
	edge_func->b = vtx1->pos[X_COMP] - vtx0->pos[X_COMP];
	/* Reformulate to avoid precision problem */
	// edge_func->c = vtx0->pos[X_COMP] * vtx1->pos[Y_COMP] - vtx1->pos[X_COMP] * vtx0->pos[Y_COMP];
	edge_func->c = -0.5 * (edge_func->a * (vtx0->pos[X_COMP] + vtx1->pos[X_COMP]) + 
		edge_func->b * (vtx0->pos[Y_COMP] + vtx1->pos[Y_COMP]));
}

