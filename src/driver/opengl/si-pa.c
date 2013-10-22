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
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "opengl.h"
#include "si-pa.h"


/*
 * Private Functions
 */



/*
 * Public Functions
 */

struct opengl_pa_viewport_t *opengl_pa_viewport_create()
{
	struct opengl_pa_viewport_t *vwpt;

	/* Allocate */
	vwpt = xcalloc(1, sizeof(struct opengl_pa_viewport_t));

	/* Return */	
	return vwpt;
}

void opengl_pa_viewport_free(struct opengl_pa_viewport_t *vwpt)
{
	free(vwpt);
}

void opengl_pa_viewport_set(struct opengl_pa_viewport_t *vwpt, int x, int y, int width, int height)
{
	vwpt->x = x;
	vwpt->y = y;
	vwpt->width = width;
	vwpt->height = height;
}

void opengl_pa_viewport_apply(struct opengl_pa_viewport_t *vwpt, struct opengl_pa_vertex_t *vtx)
{
	vtx->pos[X_COMP] = 0.5 * vwpt->width * (vtx->pos[X_COMP] + 1) + vwpt->x; 
	vtx->pos[Y_COMP] = 0.5 * vwpt->height * (vtx->pos[Y_COMP] + 1) + vwpt->y; 
}

struct opengl_pa_depth_range_t *opengl_pa_depth_range_create()
{
	struct opengl_pa_depth_range_t *dprg;

	/* Allocate */
	dprg = xcalloc(1, sizeof(struct opengl_pa_depth_range_t));

	/* Return */	
	return dprg;
}

void opengl_pa_depth_range_free(struct opengl_pa_depth_range_t *dprg)
{
	free(dprg);
}

void opengl_pa_depth_range_apply(struct opengl_pa_depth_range_t *dprg, struct opengl_pa_vertex_t *vtx)
{
	vtx->pos[Z_COMP] = 0.5 * (dprg->f - dprg->n) * vtx->pos[Z_COMP] + 0.5 * (dprg->f + dprg->n); 
}


struct opengl_pa_vertex_t *opengl_pa_vertex_create(float x, float y, float z, float w)
{
	struct opengl_pa_vertex_t *vtx;

	/* Allocate */
	vtx = xcalloc(1, sizeof(struct opengl_pa_vertex_t));

	/* Initialize */
	vtx->pos[X_COMP] = x;
	vtx->pos[Y_COMP] = y;
	vtx->pos[Z_COMP] = z;
	vtx->pos[W_COMP] = w;

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
void opengl_pa_edge_func_set(struct opengl_pa_edge_func_t *edge_func, 
	struct opengl_pa_vertex_t *vtx0, struct opengl_pa_vertex_t *vtx1)
{

	edge_func->a = vtx0->pos[Y_COMP] - vtx1->pos[Y_COMP];
	edge_func->b = vtx1->pos[X_COMP] - vtx0->pos[X_COMP];
	/* Reformulate to avoid precision problem */
	/* Original: edge_func->c = vtx0->pos[X_COMP] * vtx1->pos[Y_COMP] - vtx1->pos[X_COMP] * vtx0->pos[Y_COMP] */
	edge_func->c = -0.5 * (edge_func->a * (vtx0->pos[X_COMP] + vtx1->pos[X_COMP]) + 
		edge_func->b * (vtx0->pos[Y_COMP] + vtx1->pos[Y_COMP]));
}

struct opengl_pa_primitive_t *opengl_pa_primitives_create(enum opengl_pa_primitive_mode_t mode, 
	struct list_t *pos_lst, struct opengl_pa_viewport_t *vwpt)
{
	struct opengl_pa_primitive_t *prmtv;
	float *pos0;
	float *pos1;
	float *pos2;
	struct opengl_pa_vertex_t *vtx0;
	struct opengl_pa_vertex_t *vtx1;
	struct opengl_pa_vertex_t *vtx2;
	struct opengl_pa_triangle_t *triangle;
	int i;

	prmtv = xcalloc(1, sizeof(struct opengl_pa_primitive_t));
	prmtv->list = list_create();

	switch(mode)
	{

	case OPENGL_PA_TRIANGLES:
		prmtv->mode = OPENGL_PA_TRIANGLES;
		if (list_count(pos_lst) % 3)
			panic("Position data size is not multiples of 3!\n");
		for (i = 0; i < list_count(pos_lst) / 3; ++i)
		{
			/* Get raw postion data */
			pos0 = list_get(pos_lst, i);
			pos1 = list_get(pos_lst, i + 1);
			pos2 = list_get(pos_lst, i + 2);

			opengl_debug("\toriginal data\n");
			opengl_debug("\t%f %f %f %f\n", pos0[0], pos0[1], pos0[2], pos0[3]);
			opengl_debug("\t%f %f %f %f\n", pos1[0], pos1[1], pos1[2], pos1[3]);
			opengl_debug("\t%f %f %f %f\n", pos2[0], pos2[1], pos2[2], pos2[3]);

			/* Create vertices */
			vtx0 = opengl_pa_vertex_create(pos0[0], pos0[1], pos0[2], pos0[3]);
			vtx1 = opengl_pa_vertex_create(pos1[0], pos1[1], pos1[2], pos1[3]);
			vtx2 = opengl_pa_vertex_create(pos2[0], pos2[1], pos2[2], pos2[3]);

			/* Apply viewport */
			opengl_pa_viewport_apply(vwpt, vtx0);
			opengl_pa_viewport_apply(vwpt, vtx1);
			opengl_pa_viewport_apply(vwpt, vtx2);

			/* Generate triangle */
			triangle = opengl_pa_triangle_create();
			opengl_pa_triangle_set(triangle, vtx0, vtx1, vtx2);

			opengl_debug("\tafter viewport\n");
			opengl_debug("\t%f %f %f %f\n", vtx0->pos[0], vtx0->pos[1], vtx0->pos[2], vtx0->pos[3]);
			opengl_debug("\t%f %f %f %f\n", vtx1->pos[0], vtx1->pos[1], vtx1->pos[2], vtx1->pos[3]);
			opengl_debug("\t%f %f %f %f\n", vtx2->pos[0], vtx2->pos[1], vtx2->pos[2], vtx2->pos[3]);

			/* Add to primitive list */
			list_add(prmtv->list, triangle);
		}
		break;
	default:
		prmtv->mode = OPENGL_PA_INVALID;
		break;

	}

	/* Return */
	return prmtv;
}

void opengl_pa_primitives_free(struct opengl_pa_primitive_t *prmtv)
{
	struct opengl_pa_triangle_t *triangle;
	int i;

	switch(prmtv->mode)
	{

	case OPENGL_PA_TRIANGLES:
	{
		LIST_FOR_EACH(prmtv->list, i)
		{
			triangle = list_get(prmtv->list, i);
			opengl_pa_vertex_free(triangle->vtx0);
			opengl_pa_vertex_free(triangle->vtx1);
			opengl_pa_vertex_free(triangle->vtx2);
			opengl_pa_triangle_free(triangle);
		}
		break;		
	}
	
	default:
		break;
	}

	list_free(prmtv->list);
	free(prmtv);
}

enum opengl_pa_primitive_mode_t opengl_pa_primitive_get_mode(int mode)
{
	switch(mode)
	{

	case 0x0004:
		return OPENGL_PA_TRIANGLES;
	default:
		return OPENGL_PA_INVALID;
	}
}

