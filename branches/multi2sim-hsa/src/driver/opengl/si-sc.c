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
#include <lib/util/misc.h>
#include <lib/mhandle/mhandle.h>

#include "si-db.h"
#include "si-sc.h"
#include "si-pa.h"

/* Forward declaration */
static void opengl_sc_pixel_info_set_wndw_cood(struct opengl_sc_pixel_info_t *pxl_info, int x, int y, int z);
static void opengl_sc_pixel_info_set_brctrc_cood(struct opengl_sc_pixel_info_t *pxl_info, 
	struct opengl_pa_triangle_t *triangle);

static struct opengl_sc_span_array_t *opengl_sc_span_array_create();
static void opengl_sc_span_array_free(struct opengl_sc_span_array_t *spn_array);

static struct opengl_sc_bounding_box_t *opengl_sc_bounding_box_create(struct opengl_pa_triangle_t *triangle);
static void opengl_sc_bounding_box_free(struct opengl_sc_bounding_box_t *bbox);

/* 
 * Private functions
 */

static void opengl_sc_pixel_info_set_wndw_cood(struct opengl_sc_pixel_info_t *pxl_info, int x, int y, int z)
{
	pxl_info->pos[X_COMP] = x;
	pxl_info->pos[Y_COMP] = y;
	pxl_info->pos[Z_COMP] = z;
	pxl_info->wndw_init = 1;
}

static void opengl_sc_pixel_info_set_brctrc_cood(struct opengl_sc_pixel_info_t *pxl_info, 
	struct opengl_pa_triangle_t *triangle)
{
	assert(pxl_info->wndw_init);

	float lamda1, lamda2, lamda3;
	float x, x1, x2, x3;
	float y, y1, y2, y3;
	float det;

	/* 
	 * Calculate barycentric coordinate based on current
	 * pixel postion and positions of 3 associated vertex 
	 */
	x = (float)pxl_info->pos[X_COMP];
	y = (float)pxl_info->pos[Y_COMP];
	x1 = triangle->vtx0->pos[X_COMP];
	x2 = triangle->vtx1->pos[X_COMP];
	x3 = triangle->vtx2->pos[X_COMP];
	y1 = triangle->vtx0->pos[Y_COMP];
	y2 = triangle->vtx1->pos[Y_COMP];
	y3 = triangle->vtx2->pos[Y_COMP];

	det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
	lamda1 = (y2 - y3) * (x - x3) + (x3 - x2) * (y - y3);
	lamda1 /= det;
	lamda2 = (y3 - y1) * (x - x3) + (x1 - x3) * (y - y3);
	lamda2 /= det;
	lamda3 = 1 - lamda1 -lamda2;

	/* Only need to store lamda2 and lamda3 as AMD rearranges the formula */
	pxl_info->brctrc_i = lamda2;
	pxl_info->brctrc_j = lamda3;

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

static struct opengl_sc_bounding_box_t *opengl_sc_bounding_box_create(struct opengl_pa_triangle_t *triangle)
{
	struct opengl_sc_bounding_box_t *bbox;
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float span;
	int snapMask;
	int s;
	int k;

	/* Allocate */
	bbox = xcalloc(1, sizeof(struct opengl_sc_bounding_box_t));	

	/* Find the length of the span */
	xmin = MIN(triangle->vtx0->pos[X_COMP], MIN(triangle->vtx1->pos[X_COMP], triangle->vtx2->pos[X_COMP]));
	xmax = MAX(triangle->vtx0->pos[X_COMP], MAX(triangle->vtx1->pos[X_COMP], triangle->vtx2->pos[X_COMP]));
	ymin = MIN(triangle->vtx0->pos[Y_COMP], MIN(triangle->vtx1->pos[Y_COMP], triangle->vtx2->pos[Y_COMP]));
	ymax = MAX(triangle->vtx0->pos[Y_COMP], MAX(triangle->vtx1->pos[Y_COMP], triangle->vtx2->pos[Y_COMP]));
	span = MAX(xmax - xmin, ymax - ymin);
	s = IROUND(span);

	/* Calculate bounding box size */
	k = 0;
	do
	{
		k++;
	} while (s > (2<<k));

	/* Snapping to nearest subpixel grid */
	snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1); 
	bbox->x0 = FixedToInt(FloatToFixed(xmin - 0.5F) & snapMask);
	bbox->y0 = FixedToInt(FloatToFixed(ymin - 0.5F) & snapMask);
	bbox->size = 2<<k;

	/* Return */
	return bbox;
}

static void opengl_sc_bounding_box_free(struct opengl_sc_bounding_box_t *bbox)
{
	free(bbox);
}

/* 
 *  Public funtions
 */

struct opengl_sc_edge_t *opengl_sc_edge_create(struct opengl_pa_vertex_t *vtx0,
		struct opengl_pa_vertex_t *vtx1)
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

/* Return PIXEL_TEST_PASS if a pixel is on the right side of an edge, PIXEL_TEST_FAIL if on the left side */
int opengl_sc_edge_func_test_pixel(struct opengl_pa_edge_func_t *edge_func, int x, int y)
{
	/* E(x,y) >= 0 meaning a pixel is on the right side of the edge or exactly on the edge */
	return edge_func->a * x + edge_func->b * y + edge_func->c >= 0.0 ? PIXEL_TEST_PASS : PIXEL_TEST_FAIL ;
}

/* Pass test only when a pixel passes on 3 edge function */
int opengl_sc_triangle_test_pixel(struct opengl_pa_triangle_t *triangle, int x, int y)
{
	if (opengl_sc_edge_func_test_pixel(triangle->edgfunc0, x, y) && 
		opengl_sc_edge_func_test_pixel(triangle->edgfunc1, x, y) &&
		opengl_sc_edge_func_test_pixel(triangle->edgfunc2, x, y))
		return PIXEL_TEST_PASS;		
	else
		return PIXEL_TEST_FAIL;

}

/* If pass test, then generate a pixel info object */
struct opengl_sc_pixel_info_t *opengl_sc_triangle_test_and_gen_pixel(struct opengl_pa_triangle_t *triangle, int x, int y, int z)
{
	struct opengl_sc_pixel_info_t *pixel;
	if (opengl_sc_triangle_test_pixel(triangle, x, y))
	{
		pixel = opengl_sc_pixel_info_create();
		opengl_sc_pixel_info_set_wndw_cood(pixel, x, y, z);
		opengl_sc_pixel_info_set_brctrc_cood(pixel, triangle);
		return pixel;
	}
	return NULL;
}

void opengl_sc_pixel_gen_and_add_to_list(struct opengl_pa_triangle_t *triangle, int x, int y, struct list_t *lst)
{
	struct opengl_sc_pixel_info_t *pixel;
	/* FIXME: Z value should be interpolated */
	pixel = opengl_sc_triangle_test_and_gen_pixel(triangle, x, y, 0);
	if (pixel)
		list_add(lst, pixel);
}

/* Resursive testing tiles , x/y are the position of the lower left of a bounding box */
void opengl_pa_triangle_tiled_pixel_gen(struct opengl_pa_triangle_t *triangle, int x, int y, int size, struct list_t *pxl_lst)
{
	int half_size;

	/* Test if bounding box size is power of 2 */
	assert(size % 2 ==0);
	half_size = size>>1;

	if (size > 2)
	{
		/* Subdivide the bounding box and test in Z pattern */
		opengl_pa_triangle_tiled_pixel_gen(triangle, x, y + half_size, half_size, pxl_lst);
		opengl_pa_triangle_tiled_pixel_gen(triangle, x + half_size, y + half_size, half_size, pxl_lst);
		opengl_pa_triangle_tiled_pixel_gen(triangle, x, y, half_size, pxl_lst);
		opengl_pa_triangle_tiled_pixel_gen(triangle, x + half_size, y, half_size, pxl_lst);
	}
	else if (size == 2)
	{
		/* Size is 2x2, so test this quad and add to list if pass pixel test */
		opengl_sc_pixel_gen_and_add_to_list(triangle, x, y+1, pxl_lst);
		opengl_sc_pixel_gen_and_add_to_list(triangle, x+1, y+1, pxl_lst);
		opengl_sc_pixel_gen_and_add_to_list(triangle, x, y, pxl_lst);
		opengl_sc_pixel_gen_and_add_to_list(triangle, x+1, y, pxl_lst);		
	}

	/* Return */
	return;
}


/* Tiled rasterizer */
struct list_t *opengl_sc_tiled_rast_triangle_gen(struct opengl_pa_triangle_t *triangle)
{
	struct opengl_sc_bounding_box_t *bbox;
	struct list_t *pxl_lst;

	/* Calculate bounding box */
	bbox = opengl_sc_bounding_box_create(triangle);

	/* Create pixel list and add pixels pass test to this list */
	pxl_lst = list_create();
	opengl_pa_triangle_tiled_pixel_gen(triangle, bbox->x0, bbox->y0, bbox->size, pxl_lst);

	/* Free */
	opengl_sc_bounding_box_free(bbox);

	/* Return */
	return pxl_lst;
}

struct list_t *opengl_sc_rast_triangle_gen_pixel_info(struct opengl_pa_triangle_t *triangle, struct opengl_depth_buffer_t *db)
{
	/* List contains info of pixels inside this triangle */
	struct list_t *pxl_lst;
	struct opengl_sc_pixel_info_t *pxl_info;

	struct opengl_sc_edge_t *edge_major;
	struct opengl_sc_edge_t *edge_top;
	struct opengl_sc_edge_t *edge_bottom;

	struct opengl_pa_vertex_t *vtx_max;
	struct opengl_pa_vertex_t *vtx_mid;
	struct opengl_pa_vertex_t *vtx_min;

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
		list_free(pxl_lst);

		/*CULLED*/
		return NULL;  
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

	/* Interpolate depth */
	float edge_major_dz = vtx_max->pos[Z_COMP] - vtx_min->pos[Z_COMP];
	float edge_bottom_dz = vtx_mid->pos[Z_COMP] - vtx_min->pos[Z_COMP];
	spn->attrStepX[2] = one_over_area * (edge_major_dz * edge_bottom->dy - edge_major->dy * edge_bottom_dz);
	spn->attrStepY[2] = one_over_area * (edge_major->dx * edge_bottom_dz - edge_major_dz * edge_bottom->dx);
	spn->zStep = SignedFloatToFixed(spn->attrStepX[2]);

	int subTriangle;
	int fxLeftEdge = 0, fxRightEdge = 0;
	int fdxLeftEdge = 0, fdxRightEdge = 0;
	int fError = 0, fdError = 0;

	unsigned int zLeft = 0;
	int fdzOuter = 0, fdzInner;


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
			const struct opengl_pa_vertex_t *vtx_lower = edge_left->vtx1;
			const int fsy = edge_left->fsy;
			const int fsx = edge_left->fsx;  /* no fractional part */
			const int fx = FixedCeil(fsx);  /* no fractional part */
			const int adjx = (int) (fx - edge_left->fx0); /* SCALED! */
			const int adjy = (int) edge_left->adjy;      /* SCALED! */			
			int idxOuter;
			float dxOuter;			
			int fdxOuter;

			fError = fx - fsx - FIXED_ONE;
			fxLeftEdge = fsx - FIXED_EPSILON;
			fdxLeftEdge = edge_left->fdxdy;
			fdxOuter = FixedFloor(fdxLeftEdge - FIXED_EPSILON);
			fdError = fdxOuter - fdxLeftEdge + FIXED_ONE;
			idxOuter = FixedToInt(fdxOuter);
			dxOuter = (float) idxOuter;
			spn->y = FixedToInt(fsy);

			/* Interpolate Z */
			float z0 = vtx_lower->pos[Z_COMP];
			float tmp = (z0 * FIXED_SCALE + spn->attrStepX[2] * adjx + spn->attrStepY[2] * adjy) + FIXED_HALF;
			if (tmp < MAX_GLUINT / 2)
				zLeft = (int) tmp;
			else
				zLeft = MAX_GLUINT / 2;
			fdzOuter = SignedFloatToFixed(spn->attrStepY[2] + dxOuter * spn->attrStepX[2]);								

		}

		if (setupRight && edge_right->lines>0) 
		{
			fxRightEdge = edge_right->fsx - FIXED_EPSILON;
			fdxRightEdge = edge_right->fdxdy;
		}

		if (lines==0)
			continue;

		/* Interpolate Z */
		fdzInner = fdzOuter + spn->zStep;		

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

			/* Interpolate Z */
			spn->z = zLeft;

			/* This is where we actually generate fragments */
			if (spn->end > 0 && spn->y >= 0)
			{
				const int len = spn->end;

				int i;
				for (i = 0; i < len; ++i)
				{
					/* Add if pass depth test */
					if(opengl_depth_buffer_test_and_set_pixel(db, spn->x, spn->y, FixedToFloat(spn->z), db->depth_func))
					{
						pxl_info = opengl_sc_pixel_info_create();
						opengl_sc_pixel_info_set_wndw_cood(pxl_info, spn->x, spn->y, spn->z);
						opengl_sc_pixel_info_set_brctrc_cood(pxl_info, triangle);
						list_add(pxl_lst, pxl_info);
					}
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
				zLeft += fdzOuter;
				fError -= FIXED_ONE;				
			}
			else
				zLeft += fdzInner;
			
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

void opengl_sc_rast_triangle_done(struct list_t *pxl_lst)
{
	struct opengl_sc_pixel_info_t *pxl_info;
	int i;

	if (pxl_lst)
	{
		LIST_FOR_EACH(pxl_lst,i)
		{
			pxl_info = list_get(pxl_lst, i);
			opengl_sc_pixel_info_free(pxl_info);
		}		
		list_free(pxl_lst);
	}
}
