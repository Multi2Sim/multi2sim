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

#include "opengl-rast-triangle.h"

void x86_opengl_rasterizer_draw_tiangle(struct x86_opengl_context_t *ctx, struct x86_opengl_vertex_t *vtx0, struct x86_opengl_vertex_t *vtx1, struct x86_opengl_vertex_t *vtx2)
{
	struct x86_opengl_edge_t *edge_major;
	struct x86_opengl_edge_t *edge_top;
	struct x86_opengl_edge_t *edge_buttom;

	struct x86_opengl_vertex_t *vtx_max;
	struct x86_opengl_vertex_t *vtx_mid;
	struct x86_opengl_vertex_t *vtx_min;

	struct x86_opengl_span_t *spn;

	spn = x86_opengl_span_create();

	GLfloat one_over_area;
	GLfixed vtx_min_fx, vtx_min_fy;
	GLfixed vtx_mid_fx, vtx_mid_fy;
	GLfixed vtx_max_fy;
	GLint scan_from_left_to_right;

	const GLint snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1); /* for x/y coord snapping */

	const GLfixed fy0 = FloatToFixed(vtx0->y - 0.5F) & snapMask;
	const GLfixed fy1 = FloatToFixed(vtx1->y - 0.5F) & snapMask;
	const GLfixed fy2 = FloatToFixed(vtx2->y - 0.5F) & snapMask;

	/* Find the order of vertex */
	if (fy0 <= fy1)
	{
		if (fy1 <= fy2)
		{
			/* y0 < y1 < y2 */
			vtx_max = vtx2; vtx_mid = vtx1; vtx_min = vtx0;
			vtx_max_fy = fy2; vtx_mid_fy = fy1; vtx_min_fy = fy0;

		}
		else if (fy2 <= fy0)
		{
			/* y2 < y0 < y1 */
			vtx_max = vtx1; vtx_mid = vtx0; vtx_min = vtx2;
			vtx_max_fy = fy1; vtx_mid_fy = fy0; vtx_min_fy = fy2;
		}
		else {
			/* y0 < y2 < y1 */
			vtx_max = vtx1; vtx_mid = vtx2; vtx_min = vtx0;
			vtx_max_fy = fy1; vtx_mid_fy = fy2; vtx_min_fy = fy0;
		}
	}
	else {
		if (fy0 <= fy2)
		{
			/* y1 < y0 < y2 */
			vtx_max = vtx2; vtx_mid = vtx0; vtx_min = vtx1;
			vtx_max_fy = fy2; vtx_mid_fy = fy0; vtx_min_fy = fy1;
		}
		else if (fy2 <= fy1)
		{
			/* y2 < y1 < y0 */
			vtx_max = vtx0; vtx_mid = vtx1; vtx_min = vtx2;
			vtx_max_fy = fy0; vtx_mid_fy = fy1; vtx_min_fy = fy2;
		}
		else {
			/* y1 < y2 < y0 */
			vtx_max = vtx0; vtx_mid = vtx2; vtx_min = vtx1;
			vtx_max_fy = fy0; vtx_mid_fy = fy2; vtx_min_fy = fy1;
		}
	}

	vtx_min_fx = FloatToFixed(vtx_min->x + 0.5F) & snapMask;
	vtx_mid_fx = FloatToFixed(vtx_mid->x + 0.5F) & snapMask;
	// vtx_max_fx = FloatToFixed(vtx_max->x + 0.5F) & snapMask;

	/* Create edges */
	edge_major = x86_opengl_edge_create(vtx_max, vtx_min);
	edge_top = x86_opengl_edge_create(vtx_max, vtx_mid);
	edge_buttom = x86_opengl_edge_create(vtx_mid, vtx_min);

	/* Compute area */	
      	const GLfloat area = edge_major->dx * edge_buttom->dy - edge_buttom->dx * edge_major->dy;
      	one_over_area = 1.0f / area;

      	/* Edge setup */
	edge_major->fsy = FixedCeil(vtx_min_fy);
	edge_major->lines = FixedToInt(FixedCeil(vtx_max_fy - edge_major->fsy));
	if (edge_major->lines > 0) {
		edge_major->adjy = (GLfloat) (edge_major->fsy - vtx_min_fy);  /* SCALED! */
		edge_major->fx0 = vtx_min_fx;
		edge_major->fsx = edge_major->fx0 + (GLfixed) (edge_major->adjy * edge_major->dxdy);
	}
	else
		return;  /*CULLED*/


	edge_top->fsy = FixedCeil(vtx_mid_fy);
	edge_top->lines = FixedToInt(FixedCeil(vtx_max_fy - edge_top->fsy));
	if (edge_top->lines > 0) {
		edge_top->adjy = (GLfloat) (edge_top->fsy - vtx_mid_fy); /* SCALED! */
		edge_top->fx0 = vtx_mid_fx;
		edge_top->fsx = edge_top->fx0 + (GLfixed) (edge_top->adjy * edge_top->dxdy);
	}

	edge_buttom->fsy = FixedCeil(vtx_min_fy);
	edge_buttom->lines = FixedToInt(FixedCeil(vtx_mid_fy - edge_buttom->fsy));
	if (edge_buttom->lines > 0) {
		edge_buttom->adjy = (GLfloat) (edge_buttom->fsy - vtx_min_fy);  /* SCALED! */
		edge_buttom->fx0 = vtx_min_fx;
		edge_buttom->fsx = edge_buttom->fx0 + (GLfixed) (edge_buttom->adjy * edge_buttom->dxdy);
	}

	/* Decide scan direction */
	scan_from_left_to_right = (one_over_area < 0.0F);

	/* Interpolate */
	/* Set up interpolation parameter */
	GLfloat edge_major_dz = vtx_max->z - vtx_min->z;
	GLfloat edge_buttom_dz = vtx_mid->z - vtx_min->z;
	spn->attrStepX[2] = one_over_area * (edge_major_dz * edge_buttom->dy - edge_major->dy * edge_buttom_dz);
	spn->attrStepY[2] = one_over_area * (edge_major->dx * edge_buttom_dz - edge_major_dz * edge_buttom->dx);
	spn->zStep = SignedFloatToFixed(spn->attrStepX[2]);

	spn->red = vtx2->color[R_COMP];
	spn->green = vtx2->color[G_COMP];
	spn->blue = vtx2->color[B_COMP];

	GLint subTriangle;
	GLfixed fxLeftEdge = 0, fxRightEdge = 0;
	GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
	GLfixed fError = 0, fdError = 0;

	GLuint zLeft = 0;
	GLfixed fdzOuter = 0, fdzInner;	

	/* Setup order of edges */
	for (subTriangle=0; subTriangle<=1; subTriangle++)
	{
		struct x86_opengl_edge_t *edge_left;
		struct x86_opengl_edge_t *edge_right;
		int setupLeft, setupRight;
		int lines;

		if (subTriangle==0) {
			/* bottom half */
			if (scan_from_left_to_right) {
				edge_left = edge_major;
				edge_right = edge_buttom;
				lines = edge_right->lines;
				setupLeft = 1;
				setupRight = 1;
			}
			else {
				edge_left = edge_buttom;
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
				return;
		}

		if (setupLeft && edge_left->lines > 0)
		{
			const struct x86_opengl_vertex_t *vtx_lower = edge_left->vtx0;
			const GLfixed fsy = edge_left->fsy;
			const GLfixed fsx = edge_left->fsx;  /* no fractional part */
			const GLfixed fx = FixedCeil(fsx);  /* no fractional part */
			const GLfixed adjx = (GLfixed) (fx - edge_left->fx0); /* SCALED! */
			const GLfixed adjy = (GLfixed) edge_left->adjy;      /* SCALED! */
			GLint idxOuter;
			GLfloat dxOuter;
			GLfixed fdxOuter;

			fError = fx - fsx - FIXED_ONE;
			fxLeftEdge = fsx - FIXED_EPSILON;
			fdxLeftEdge = edge_left->fdxdy;
			fdxOuter = FixedFloor(fdxLeftEdge - FIXED_EPSILON);
			fdError = fdxOuter - fdxLeftEdge + FIXED_ONE;
			idxOuter = FixedToInt(fdxOuter);
			dxOuter = (GLfloat) idxOuter;
			spn->y = FixedToInt(fsy);

			GLfloat z0 = vtx_lower->z;
			GLfloat tmp = (z0 * FIXED_SCALE + spn->attrStepX[2] * adjx+ spn->attrStepY[2] * adjy) + FIXED_HALF;
			if (tmp < MAX_GLUINT / 2)
				zLeft = (GLfixed) tmp;
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

		fdzInner = fdzOuter + spn->zStep;

		/* Rasterize setup */
		while (lines > 0)
		{
			/* initialize the span interpolants to the leftmost value */
			/* ff = fixed-pt fragment */
			const GLint right = FixedToInt(fxRightEdge);
			spn->x = FixedToInt(fxLeftEdge);
			if (right <= spn->x)
				spn->end = 0;
			else
				spn->end = right - spn->x;

			spn->z = zLeft;

			x86_opengl_debug("\t\tSpan: Z = %d zStep = %d\n", spn->z, spn->zStep );
			/* This is where we actually generate fragments */
			if (spn->end > 0 && spn->y >= 0)
			{
				const GLint len = spn->end - 1;
				(void) len;
				x86_opengl_debug("\t\tSpan: [%d, %d] with length %d\n", spn->x, spn->y, len);
				int i;
				for (i = 0; i < len; ++i)
				{
					int color = (spn->red << 16) + (spn->green << 8)+ spn->blue;

					if (ctx->draw_buffer->depth_buffer->buffer[spn->x * ctx->draw_buffer->depth_buffer->width + spn->y] < spn->z)
					{
						ctx->draw_buffer->depth_buffer->buffer[spn->x * ctx->draw_buffer->depth_buffer->width + spn->y] = spn->z;
						x86_glut_frame_buffer_pixel(spn->x, spn->y, color);
					};
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
			if (fError >= 0) {
			fError -= FIXED_ONE;

			zLeft += fdzOuter;

			} else {

			zLeft += fdzInner;

			}
		} /*while lines>0*/
	}


	/* Free edges */
	x86_opengl_edge_free(edge_major);
	x86_opengl_edge_free(edge_top);
	x86_opengl_edge_free(edge_buttom);

	/* Free span */
	x86_opengl_span_free(spn);

}
