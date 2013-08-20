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


#include <assert.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/mhandle/mhandle.h>

#include "scan-converter.h"

/* Forward declaration */
static void opengl_sc_pixel_info_set_wndw_cood(struct opengl_sc_pixel_info_t *pxl_info, int i, int j);
static void opengl_sc_pixel_info_set_brctrc_cood(struct opengl_sc_pixel_info_t *pxl_info, 
	struct opengl_sc_triangle_t *triangle);
static struct opengl_sc_span_array_t *opengl_sc_span_array_create();
static void opengl_sc_span_array_free(struct opengl_sc_span_array_t *spn_array);

/* 
 * Private functions
 */

static void opengl_sc_pixel_info_set_wndw_cood(struct opengl_sc_pixel_info_t *pxl_info, int i, int j)
{
	pxl_info->wndw_i = i;
	pxl_info->wndw_j = j;
	pxl_info->wndw_init = 1;
}

static void opengl_sc_pixel_info_set_brctrc_cood(struct opengl_sc_pixel_info_t *pxl_info, 
	struct opengl_sc_triangle_t *triangle)
{
	assert(!pxl_info->wndw_init);

	/* 
	 * Calculate barycentric coordinate based on current
	 * pixel postion and positions of 3 associated vertex 
	 */

	/* FIXME: TODO */

}

static struct opengl_sc_span_array_t *opengl_sc_span_array_create()
{
	struct opengl_sc_span_array_t *spn_array;

	/* Allocate */
	spn_array = xcalloc(1, sizeof(struct opengl_sc_span_array_t));

	/* Return */	
	return spn_array;
}

static void opengl_sc_span_array_free(struct opengl_sc_span_array_t *spn_array)
{
	free(spn_array);
}


/* 
 *  Public funtions
 */

struct opengl_sc_vertex_t *opengl_sc_vertex_create()
{
	struct opengl_sc_vertex_t *vtx;

	/* Allocate */
	vtx = xcalloc(1, sizeof(struct opengl_sc_vertex_t));

	/* Return */	
	return vtx;
}

void opengl_sc_vertex_free(struct opengl_sc_vertex_t *vtx)
{
	free(vtx);
}

struct opengl_sc_edge_t *opengl_sc_edge_create(struct opengl_sc_vertex_t *vtx0,
		struct opengl_sc_vertex_t *vtx1)
{
	struct opengl_sc_edge_t * edge;

	edge = xcalloc(1, sizeof(struct opengl_sc_edge_t));

	/* Initialize */
	edge->vtx0 = vtx0;
	edge->vtx1 = vtx1;

	/* Return */
	return edge;
}

void opengl_sc_edge_free(struct opengl_sc_edge_t *edge)
{
	free(edge);
}


struct opengl_sc_span_t *opengl_sc_span_create()
{
	struct opengl_sc_span_t *spn;

	/* Allocate */
	spn = xcalloc(1, sizeof(struct opengl_sc_span_t));

	/* Initialize */
	spn->array = opengl_sc_span_array_create();

	/* Return */	
	return spn;
}

void opengl_sc_span_free(struct opengl_sc_span_t *spn)
{
	opengl_sc_span_array_free(spn->array);
	free(spn);
}

void opengl_sc_span_interpolate_z(struct opengl_sc_span_t *spn)
{
	const unsigned int n = spn->end;
	unsigned int i;

	/* Deep Z buffer, no fixed->int shift */
	unsigned int zval = spn->z;
	unsigned int *z = spn->array->z;
	for (i = 0; i < n; i++) 
	{
		z[i] = zval;
		zval += spn->zStep;
	}

}

struct opengl_sc_pixel_info_t *opengl_sc_pixel_info_create()
{
	struct opengl_sc_pixel_info_t *pxl_info;

	/* Allocate */
	pxl_info = xcalloc(1, sizeof(struct opengl_sc_pixel_info_t));

	/* Return */	
	return pxl_info;
}

void opengl_sc_pixel_info_free(struct opengl_sc_pixel_info_t *pxl_info)
{
	free(pxl_info);
}

struct list_t *opengl_sc_triangle(struct opengl_sc_triangle_t *triangle)
{
	/* List contains info of pixels inside this triangle */
	struct list_t *pxl_lst;
	struct opengl_sc_pixel_info_t *pxl_info;

	struct opengl_sc_edge_t *edge_major;
	struct opengl_sc_edge_t *edge_top;
	struct opengl_sc_edge_t *edge_bottom;

	struct opengl_sc_vertex_t *vtx_max;
	struct opengl_sc_vertex_t *vtx_mid;
	struct opengl_sc_vertex_t *vtx_min;

	struct opengl_sc_span_t *spn;

	pxl_lst = list_create();
	spn = opengl_sc_span_create();

	float one_over_area;
	int vtx_min_fx, vtx_min_fy;
	int vtx_mid_fx, vtx_mid_fy;
	int vtx_max_fx, vtx_max_fy;
	int scan_from_left_to_right;

	const int snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1); /* for x/y coord snapping */

	const int fy0 = FloatToFixed(triangle->vtx0->pos[Y_COMP] - 0.5F) & snapMask;
	const int fy1 = FloatToFixed(triangle->vtx1->pos[Y_COMP] - 0.5F) & snapMask;
	const int fy2 = FloatToFixed(triangle->vtx2->pos[Y_COMP] - 0.5F) & snapMask;

	/* Find the order of vertex */
	if (fy0 <= fy1)
	{
		if (fy1 <= fy2)
		{
			/* y0 < y1 < y2 */
			vtx_max = triangle->vtx2; vtx_mid = triangle->vtx1; vtx_min = triangle->vtx0;
			vtx_max_fy = fy2; vtx_mid_fy = fy1; vtx_min_fy = fy0;

		}
		else if (fy2 <= fy0)
		{
			/* y2 < y0 < y1 */
			vtx_max = triangle->vtx1; vtx_mid = triangle->vtx0; vtx_min = triangle->vtx2;
			vtx_max_fy = fy1; vtx_mid_fy = fy0; vtx_min_fy = fy2;
		}
		else {
			/* y0 < y2 < y1 */
			vtx_max = triangle->vtx1; vtx_mid = triangle->vtx2; vtx_min = triangle->vtx0;
			vtx_max_fy = fy1; vtx_mid_fy = fy2; vtx_min_fy = fy0;
		}
	}
	else {
		if (fy0 <= fy2)
		{
			/* y1 < y0 < y2 */
			vtx_max = triangle->vtx2; vtx_mid = triangle->vtx0; vtx_min = triangle->vtx1;
			vtx_max_fy = fy2; vtx_mid_fy = fy0; vtx_min_fy = fy1;
		}
		else if (fy2 <= fy1)
		{
			/* y2 < y1 < y0 */
			vtx_max = triangle->vtx0; vtx_mid = triangle->vtx1; vtx_min = triangle->vtx2;
			vtx_max_fy = fy0; vtx_mid_fy = fy1; vtx_min_fy = fy2;
		}
		else {
			/* y1 < y2 < y0 */
			vtx_max = triangle->vtx0; vtx_mid = triangle->vtx2; vtx_min = triangle->vtx1;
			vtx_max_fy = fy0; vtx_mid_fy = fy2; vtx_min_fy = fy1;
		}
	}

	vtx_min_fx = FloatToFixed(vtx_min->pos[X_COMP] + 0.5F) & snapMask;
	vtx_mid_fx = FloatToFixed(vtx_mid->pos[X_COMP] + 0.5F) & snapMask;
	vtx_max_fx = FloatToFixed(vtx_max->pos[X_COMP] + 0.5F) & snapMask;

	/* Create edges */
	edge_major = opengl_sc_edge_create(vtx_max, vtx_min);
	edge_top = opengl_sc_edge_create(vtx_max, vtx_mid);
	edge_bottom = opengl_sc_edge_create(vtx_mid, vtx_min);

	/* compute deltas for each edge:  vertex[upper] - vertex[lower] */
	edge_major->dx = FixedToFloat(vtx_max_fx - vtx_min_fx);
	edge_major->dy = FixedToFloat(vtx_max_fy - vtx_min_fy);
	edge_top->dx = FixedToFloat(vtx_max_fx - vtx_mid_fx);
	edge_top->dy = FixedToFloat(vtx_max_fy - vtx_mid_fy);
	edge_bottom->dx = FixedToFloat(vtx_mid_fx - vtx_min_fx);
	edge_bottom->dy = FixedToFloat(vtx_mid_fy - vtx_min_fy);

	/* Compute area */	
      	const float area = edge_major->dx * edge_bottom->dy - edge_bottom->dx * edge_major->dy;
      	one_over_area = 1.0f / area;

      	/* Edge setup */
	edge_major->fsy = FixedCeil(vtx_min_fy);
	edge_major->lines = FixedToInt(FixedCeil(vtx_max_fy - edge_major->fsy));
	if (edge_major->lines > 0) {
		edge_major->dxdy = edge_major->dx / edge_major->dy;
		edge_major->fdxdy = SignedFloatToFixed(edge_major->dxdy);		
		edge_major->adjy = (float) (edge_major->fsy - vtx_min_fy);  /* SCALED! */
		edge_major->fx0 = vtx_min_fx;
		edge_major->fsx = edge_major->fx0 + (int) (edge_major->adjy * edge_major->dxdy);
	}
	else
	{
		/* Free edges */
		opengl_sc_edge_free(edge_major);
		opengl_sc_edge_free(edge_top);
		opengl_sc_edge_free(edge_bottom);

		/* Free span*/
		opengl_sc_span_free(spn);
		return NULL;  /*CULLED*/
	}

	edge_top->fsy = FixedCeil(vtx_mid_fy);
	edge_top->lines = FixedToInt(FixedCeil(vtx_max_fy - edge_top->fsy));
	if (edge_top->lines > 0)
	{
		edge_top->dxdy = edge_top->dx / edge_top->dy;
		edge_top->fdxdy = SignedFloatToFixed(edge_top->dxdy);		
		edge_top->adjy = (float) (edge_top->fsy - vtx_mid_fy); /* SCALED! */
		edge_top->fx0 = vtx_mid_fx;
		edge_top->fsx = edge_top->fx0 + (int) (edge_top->adjy * edge_top->dxdy);
	}

	edge_bottom->fsy = FixedCeil(vtx_min_fy);
	edge_bottom->lines = FixedToInt(FixedCeil(vtx_mid_fy - edge_bottom->fsy));
	if (edge_bottom->lines > 0)
	{
		edge_bottom->dxdy = edge_bottom->dx / edge_bottom->dy;
		edge_bottom->fdxdy = SignedFloatToFixed(edge_bottom->dxdy);		
		edge_bottom->adjy = (float) (edge_bottom->fsy - vtx_min_fy);  /* SCALED! */
		edge_bottom->fx0 = vtx_min_fx;
		edge_bottom->fsx = edge_bottom->fx0 + (int) (edge_bottom->adjy * edge_bottom->dxdy);
	}

	/* Decide scan direction */
	scan_from_left_to_right = (one_over_area < 0.0F);

	int subTriangle;
	int fxLeftEdge = 0, fxRightEdge = 0;
	int fdxLeftEdge = 0, fdxRightEdge = 0;
	int fError = 0, fdError = 0;

	/* Setup order of edges */
	for (subTriangle=0; subTriangle<=1; subTriangle++)
	{
		struct opengl_sc_edge_t *edge_left;
		struct opengl_sc_edge_t *edge_right;
		int setupLeft, setupRight;
		int lines;

		if (subTriangle==0) {
			/* bottom half */
			if (scan_from_left_to_right) {
				edge_left = edge_major;
				edge_right = edge_bottom;
				lines = edge_right->lines;
				setupLeft = 1;
				setupRight = 1;
			}
			else {
				edge_left = edge_bottom;
				edge_right = edge_major;
				lines = edge_left->lines;
				setupLeft = 1;
				setupRight = 1;
			}
		} 
		else {
			/* top half */
			if (scan_from_left_to_right) 
			{
				edge_left = edge_major;
				edge_right = edge_top;
				lines = edge_right->lines;
				setupLeft = 0;
				setupRight = 1;
			}
			else {
				edge_left = edge_top;
				edge_right = edge_major;
				lines = edge_left->lines;
				setupLeft = 1;
				setupRight = 0;
			}
			if (lines == 0)
				return NULL;
		}

		if (setupLeft && edge_left->lines > 0)
		{
			const int fsy = edge_left->fsy;
			const int fsx = edge_left->fsx;  /* no fractional part */
			const int fx = FixedCeil(fsx);  /* no fractional part */
			int fdxOuter;

			fError = fx - fsx - FIXED_ONE;
			fxLeftEdge = fsx - FIXED_EPSILON;
			fdxLeftEdge = edge_left->fdxdy;
			fdxOuter = FixedFloor(fdxLeftEdge - FIXED_EPSILON);
			fdError = fdxOuter - fdxLeftEdge + FIXED_ONE;
			spn->y = FixedToInt(fsy);
		}

		if (setupRight && edge_right->lines>0) 
		{
			fxRightEdge = edge_right->fsx - FIXED_EPSILON;
			fdxRightEdge = edge_right->fdxdy;
		}

		if (lines==0)
			continue;

		/* Rasterize setup */
		while (lines > 0)
		{

			/* initialize the spn->interpolants to the leftmost value */
			/* ff = fixed-pt fragment */
			const int right = FixedToInt(fxRightEdge);
			spn->x = FixedToInt(fxLeftEdge);
			if (right <= spn->x)
				spn->end = 0;
			else
				spn->end = right - spn->x;


			/* This is where we actually generate fragments */
			if (spn->end > 0 && spn->y >= 0)
			{
				const int len = spn->end;

				int i;
				for (i = 0; i < len; ++i)
				{
					pxl_info = opengl_sc_pixel_info_create();
					opengl_sc_pixel_info_set_wndw_cood(pxl_info, spn->x, spn->y);
					opengl_sc_pixel_info_set_brctrc_cood(pxl_info, triangle);
					list_add(pxl_lst, pxl_info);
					spn->z += spn->zStep;
					spn->x++;
				}
			}

			/*
			* Advance to the next scan line.  Compute the new edge coordinates, and adjust the
			* pixel-center x coordinate so that it stays on or inside the major edge.
			*/
			spn->y++;
			lines--;

			fxLeftEdge += fdxLeftEdge;
			fxRightEdge += fdxRightEdge;

			fError += fdError;

			if (fError >= 0) 
			{
				fError -= FIXED_ONE;

			}
		} /*while lines>0*/
	}

	/* Free edges */
	opengl_sc_edge_free(edge_major);
	opengl_sc_edge_free(edge_top);
	opengl_sc_edge_free(edge_bottom);

	/* Free span*/
	opengl_sc_span_free(spn);

	/* Return */
	return pxl_lst;
}
