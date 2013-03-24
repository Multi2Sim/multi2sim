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

#include <driver/glut/frame-buffer.h>
#include <lib/util/debug.h>

#include "buffers.h"
#include "edge.h"
#include "light.h"
#include "opengl.h"
#include "rasterizer.h"
#include "span.h"
#include "vertex.h"


#ifndef CLAMP_INTERPOLANT
#define CLAMP_INTERPOLANT(CHANNEL, CHANNELSTEP, LEN)		\
do {								\
   GLfixed endVal = spn->CHANNEL + (LEN) * spn->CHANNELSTEP;	\
   if (endVal < 0) {						\
      spn->CHANNEL -= endVal;					\
   }								\
   if (spn->CHANNEL < 0) {					\
      spn->CHANNEL = 0;						\
   }								\
} while (0)
#endif

void opengl_rasterizer_draw_triangle(struct opengl_context_t *ctx,
		struct opengl_vertex_t *vtx0, struct opengl_vertex_t *vtx1,
		struct opengl_vertex_t *vtx2)
{
	struct opengl_edge_t *edge_major;
	struct opengl_edge_t *edge_top;
	struct opengl_edge_t *edge_bottom;

	struct opengl_vertex_t *vtx_max;
	struct opengl_vertex_t *vtx_mid;
	struct opengl_vertex_t *vtx_min;

	struct opengl_span_t *spn;

	spn = opengl_span_create();

	GLfloat one_over_area;
	GLfixed vtx_min_fx, vtx_min_fy;
	GLfixed vtx_mid_fx, vtx_mid_fy;
	GLfixed vtx_max_fx, vtx_max_fy;
	GLint scan_from_left_to_right;

	const GLint snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1); /* for x/y coord snapping */

	const GLfixed fy0 = FloatToFixed(vtx0->pos[Y_COMP] - 0.5F) & snapMask;
	const GLfixed fy1 = FloatToFixed(vtx1->pos[Y_COMP] - 0.5F) & snapMask;
	const GLfixed fy2 = FloatToFixed(vtx2->pos[Y_COMP] - 0.5F) & snapMask;

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

	vtx_min_fx = FloatToFixed(vtx_min->pos[X_COMP] + 0.5F) & snapMask;
	vtx_mid_fx = FloatToFixed(vtx_mid->pos[X_COMP] + 0.5F) & snapMask;
	vtx_max_fx = FloatToFixed(vtx_max->pos[X_COMP] + 0.5F) & snapMask;

	/* Create edges */
	edge_major = opengl_edge_create(vtx_max, vtx_min);
	edge_top = opengl_edge_create(vtx_max, vtx_mid);
	edge_bottom = opengl_edge_create(vtx_mid, vtx_min);

	/* compute deltas for each edge:  vertex[upper] - vertex[lower] */
	edge_major->dx = FixedToFloat(vtx_max_fx - vtx_min_fx);
	edge_major->dy = FixedToFloat(vtx_max_fy - vtx_min_fy);
	edge_top->dx = FixedToFloat(vtx_max_fx - vtx_mid_fx);
	edge_top->dy = FixedToFloat(vtx_max_fy - vtx_mid_fy);
	edge_bottom->dx = FixedToFloat(vtx_mid_fx - vtx_min_fx);
	edge_bottom->dy = FixedToFloat(vtx_mid_fy - vtx_min_fy);

	/* Compute area */	
      	const GLfloat area = edge_major->dx * edge_bottom->dy - edge_bottom->dx * edge_major->dy;
      	one_over_area = 1.0f / area;

      	/* Edge setup */
	edge_major->fsy = FixedCeil(vtx_min_fy);
	edge_major->lines = FixedToInt(FixedCeil(vtx_max_fy - edge_major->fsy));
	if (edge_major->lines > 0) {
		edge_major->dxdy = edge_major->dx / edge_major->dy;
		edge_major->fdxdy = SignedFloatToFixed(edge_major->dxdy);		
		edge_major->adjy = (GLfloat) (edge_major->fsy - vtx_min_fy);  /* SCALED! */
		edge_major->fx0 = vtx_min_fx;
		edge_major->fsx = edge_major->fx0 + (GLfixed) (edge_major->adjy * edge_major->dxdy);
	}
	else
	{
		/* Free edges */
		opengl_edge_free(edge_major);
		opengl_edge_free(edge_top);
		opengl_edge_free(edge_bottom);

		/* Free span*/
		opengl_span_free(spn);
		return;  /*CULLED*/
	}



	edge_top->fsy = FixedCeil(vtx_mid_fy);
	edge_top->lines = FixedToInt(FixedCeil(vtx_max_fy - edge_top->fsy));
	if (edge_top->lines > 0)
	{
		edge_top->dxdy = edge_top->dx / edge_top->dy;
		edge_top->fdxdy = SignedFloatToFixed(edge_top->dxdy);		
		edge_top->adjy = (GLfloat) (edge_top->fsy - vtx_mid_fy); /* SCALED! */
		edge_top->fx0 = vtx_mid_fx;
		edge_top->fsx = edge_top->fx0 + (GLfixed) (edge_top->adjy * edge_top->dxdy);
	}

	edge_bottom->fsy = FixedCeil(vtx_min_fy);
	edge_bottom->lines = FixedToInt(FixedCeil(vtx_mid_fy - edge_bottom->fsy));
	if (edge_bottom->lines > 0)
	{
		edge_bottom->dxdy = edge_bottom->dx / edge_bottom->dy;
		edge_bottom->fdxdy = SignedFloatToFixed(edge_bottom->dxdy);		
		edge_bottom->adjy = (GLfloat) (edge_bottom->fsy - vtx_min_fy);  /* SCALED! */
		edge_bottom->fx0 = vtx_min_fx;
		edge_bottom->fsx = edge_bottom->fx0 + (GLfixed) (edge_bottom->adjy * edge_bottom->dxdy);
	}

	/* Decide scan direction */
	scan_from_left_to_right = (one_over_area < 0.0F);
	opengl_debug("\t\tScan from Left to Right= %d\n", scan_from_left_to_right);

	/* Interpolate depth */
	if (ctx->context_cap->is_depth_test)
	{
		GLfloat edge_major_dz = vtx_max->pos[Z_COMP] - vtx_min->pos[Z_COMP];
		GLfloat edge_bottom_dz = vtx_mid->pos[Z_COMP] - vtx_min->pos[Z_COMP];
		spn->attrStepX[FRAG_ATTRIB_WPOS][2] = one_over_area * (edge_major_dz * edge_bottom->dy - edge_major->dy * edge_bottom_dz);
		spn->attrStepY[FRAG_ATTRIB_WPOS][2] = one_over_area * (edge_major->dx * edge_bottom_dz - edge_major_dz * edge_bottom->dx);
		spn->zStep = SignedFloatToFixed(spn->attrStepX[FRAG_ATTRIB_WPOS][2]);
	}

	/* Interpolate RGBA color*/
	if (ctx->light->ShadeModel == GL_SMOOTH) 
	{
		GLfloat eMaj_dr = (GLfloat) (vtx_max->color[R_COMP] - vtx_min->color[R_COMP]);
		GLfloat eBot_dr = (GLfloat) (vtx_mid->color[R_COMP] - vtx_min->color[R_COMP]);
		GLfloat eMaj_dg = (GLfloat) (vtx_max->color[G_COMP] - vtx_min->color[G_COMP]);
		GLfloat eBot_dg = (GLfloat) (vtx_mid->color[G_COMP] - vtx_min->color[G_COMP]);
		GLfloat eMaj_db = (GLfloat) (vtx_max->color[B_COMP] - vtx_min->color[B_COMP]);
		GLfloat eBot_db = (GLfloat) (vtx_mid->color[B_COMP] - vtx_min->color[B_COMP]);

		spn->attrStepX[FRAG_ATTRIB_COL0][0] = one_over_area * (eMaj_dr * edge_bottom->dy - edge_major->dy * eBot_dr);
		spn->attrStepY[FRAG_ATTRIB_COL0][0] = one_over_area * (edge_major->dx * eBot_dr - eMaj_dr * edge_bottom->dx);
		spn->attrStepX[FRAG_ATTRIB_COL0][1] = one_over_area * (eMaj_dg * edge_bottom->dy - edge_major->dy * eBot_dg);
		spn->attrStepY[FRAG_ATTRIB_COL0][1] = one_over_area * (edge_major->dx * eBot_dg - eMaj_dg * edge_bottom->dx);
		spn->attrStepX[FRAG_ATTRIB_COL0][2] = one_over_area * (eMaj_db * edge_bottom->dy - edge_major->dy * eBot_db);
		spn->attrStepY[FRAG_ATTRIB_COL0][2] = one_over_area * (edge_major->dx * eBot_db - eMaj_db * edge_bottom->dx);

		spn->redStep   = SignedFloatToFixed(spn->attrStepX[FRAG_ATTRIB_COL0][0]);
		spn->greenStep = SignedFloatToFixed(spn->attrStepX[FRAG_ATTRIB_COL0][1]);
		spn->blueStep  = SignedFloatToFixed(spn->attrStepX[FRAG_ATTRIB_COL0][2]);

		GLfloat eMaj_da = (GLfloat) (vtx_max->color[A_COMP] - vtx_min->color[A_COMP]);
		GLfloat eBot_da = (GLfloat) (vtx_mid->color[A_COMP] - vtx_min->color[A_COMP]);

		spn->attrStepX[FRAG_ATTRIB_COL0][3] = one_over_area * (eMaj_da * edge_bottom->dy - edge_major->dy * eBot_da);
		spn->attrStepY[FRAG_ATTRIB_COL0][3] = one_over_area * (edge_major->dx * eBot_da - eMaj_da * edge_bottom->dx);	
		spn->alphaStep = SignedFloatToFixed(spn->attrStepX[FRAG_ATTRIB_COL0][3]);
	}
	else
	{
		spn->attrStepX[FRAG_ATTRIB_COL0][0] = spn->attrStepY[FRAG_ATTRIB_COL0][0] = 0.0F;
		spn->attrStepX[FRAG_ATTRIB_COL0][1] = spn->attrStepY[FRAG_ATTRIB_COL0][1] = 0.0F;
		spn->attrStepX[FRAG_ATTRIB_COL0][2] = spn->attrStepY[FRAG_ATTRIB_COL0][2] = 0.0F;
		spn->attrStepX[FRAG_ATTRIB_COL0][3] = spn->attrStepY[FRAG_ATTRIB_COL0][3] = 0.0F;
		spn->redStep   = 0;
		spn->greenStep = 0;
		spn->blueStep  = 0;
		spn->alphaStep = 0;
	}

	GLint subTriangle;
	GLfixed fxLeftEdge = 0, fxRightEdge = 0;
	GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
	GLfixed fError = 0, fdError = 0;

	GLuint zLeft = 0;
	GLfixed fdzOuter = 0, fdzInner;
	GLint rLeft = 0, fdrOuter = 0, fdrInner = 0;
	GLint gLeft = 0, fdgOuter = 0, fdgInner = 0;
	GLint bLeft = 0, fdbOuter = 0, fdbInner = 0;
	GLint aLeft = 0, fdaOuter = 0, fdaInner = 0;

	/* Setup order of edges */
	for (subTriangle=0; subTriangle<=1; subTriangle++)
	{
		struct opengl_edge_t *edge_left;
		struct opengl_edge_t *edge_right;
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
				return;
		}

		if (setupLeft && edge_left->lines > 0)
		{
			const struct opengl_vertex_t *vtx_lower = edge_left->vtx1;
			opengl_debug("\t\tVertex Lower RGBA = [%d, %d, %d, %d]\n", vtx_lower->color[R_COMP], vtx_lower->color[G_COMP], vtx_lower->color[B_COMP], vtx_lower->color[A_COMP]);
			const GLfixed fsy = edge_left->fsy;
			const GLfixed fsx = edge_left->fsx;  /* no fractional part */
			const GLfixed fx = FixedCeil(fsx);  /* no fractional part */
			const GLfixed adjx = (GLfixed) (fx - edge_left->fx0); /* SCALED! */
			const GLfixed adjy = (GLfixed) edge_left->adjy;      /* SCALED! */
			GLint idxOuter;
			GLfloat dxOuter;
			GLfixed fdxOuter;

			opengl_debug("\t\tfsy = %d, fsx = %d\n", fsy, fsx);
			opengl_debug("\t\tfx = %d, adjx = %d, adjy = %d\n", fx, adjx, adjy);


			fError = fx - fsx - FIXED_ONE;
			fxLeftEdge = fsx - FIXED_EPSILON;
			fdxLeftEdge = edge_left->fdxdy;
			fdxOuter = FixedFloor(fdxLeftEdge - FIXED_EPSILON);
			fdError = fdxOuter - fdxLeftEdge + FIXED_ONE;
			idxOuter = FixedToInt(fdxOuter);
			dxOuter = (GLfloat) idxOuter;
			spn->y = FixedToInt(fsy);

			/* TODO: add support for depthbit > 16 */
			/* Interpolate Z */
			if (ctx->context_cap->is_depth_test)
			{
				GLfloat z0 = vtx_lower->pos[Z_COMP];
				GLfloat tmp = (z0 * FIXED_SCALE + spn->attrStepX[FRAG_ATTRIB_WPOS][2] * adjx + spn->attrStepY[FRAG_ATTRIB_WPOS][2] * adjy) + FIXED_HALF;
				if (tmp < MAX_GLUINT / 2)
					zLeft = (GLfixed) tmp;
				else
					zLeft = MAX_GLUINT / 2;
				fdzOuter = SignedFloatToFixed(spn->attrStepY[FRAG_ATTRIB_WPOS][2] + dxOuter * spn->attrStepX[FRAG_ATTRIB_WPOS][2]);								
			}

			/* Interpolate RGBA */
			if (ctx->light->ShadeModel == GL_SMOOTH)
			{
				rLeft = (GLint)(ChanToFixed(vtx_lower->color[R_COMP])
								+ spn->attrStepX[FRAG_ATTRIB_COL0][0] * adjx
								+ spn->attrStepY[FRAG_ATTRIB_COL0][0] * adjy) + FIXED_HALF;

				gLeft = (GLint)(ChanToFixed(vtx_lower->color[G_COMP])
								+ spn->attrStepX[FRAG_ATTRIB_COL0][1] * adjx
								+ spn->attrStepY[FRAG_ATTRIB_COL0][1] * adjy) + FIXED_HALF;
				bLeft = (GLint)(ChanToFixed(vtx_lower->color[B_COMP])
								+ spn->attrStepX[FRAG_ATTRIB_COL0][2] * adjx
								+ spn->attrStepY[FRAG_ATTRIB_COL0][2] * adjy) + FIXED_HALF;

				opengl_debug("\t\tVertex Lower R = %d, StepX_R = %f, StepY_R = %f\n", 
					vtx_lower->color[R_COMP], spn->attrStepX[FRAG_ATTRIB_COL0][0], spn->attrStepY[FRAG_ATTRIB_COL0][0]);
				
				opengl_debug("\t\tVertex Lower G = %d, StepX_G = %f, StepY_G = %f\n", 
					vtx_lower->color[G_COMP], spn->attrStepX[FRAG_ATTRIB_COL0][1], spn->attrStepY[FRAG_ATTRIB_COL0][1]);

				opengl_debug("\t\tVertex Lower B = %d, StepX_B = %f, StepY_B = %f\n", 
					vtx_lower->color[B_COMP], spn->attrStepX[FRAG_ATTRIB_COL0][2], spn->attrStepY[FRAG_ATTRIB_COL0][2]);

				fdrOuter = SignedFloatToFixed(spn->attrStepY[FRAG_ATTRIB_COL0][0]
									+ dxOuter * spn->attrStepX[FRAG_ATTRIB_COL0][0]);
				fdgOuter = SignedFloatToFixed(spn->attrStepY[FRAG_ATTRIB_COL0][1]
									+ dxOuter * spn->attrStepX[FRAG_ATTRIB_COL0][1]);
				fdbOuter = SignedFloatToFixed(spn->attrStepY[FRAG_ATTRIB_COL0][2]
									+ dxOuter * spn->attrStepX[FRAG_ATTRIB_COL0][2]);

				opengl_debug("\t\trLeft = %d, gLeft = %d, bLeft = %d\n", rLeft, gLeft, bLeft);

				aLeft = (GLint)(ChanToFixed(vtx_lower->color[A_COMP])
								+ spn->attrStepX[FRAG_ATTRIB_COL0][3] * adjx
								+ spn->attrStepY[FRAG_ATTRIB_COL0][3] * adjy) + FIXED_HALF;
				fdaOuter = SignedFloatToFixed(spn->attrStepY[FRAG_ATTRIB_COL0][3]
								+ dxOuter * spn->attrStepX[FRAG_ATTRIB_COL0][3]);
			} else {
				rLeft = ChanToFixed(vtx2->color[R_COMP]);
				gLeft = ChanToFixed(vtx2->color[G_COMP]);
				bLeft = ChanToFixed(vtx2->color[B_COMP]);
				fdrOuter = fdgOuter = fdbOuter = 0;

				aLeft = ChanToFixed(vtx2->color[A_COMP]);
				fdaOuter = 0;
			}	
		}



		if (setupRight && edge_right->lines>0) 
		{
			fxRightEdge = edge_right->fsx - FIXED_EPSILON;
			fdxRightEdge = edge_right->fdxdy;
		}

		if (lines==0)
			continue;

		/* Interpolate Z */
		if (ctx->context_cap->is_depth_test)
		{
			fdzInner = fdzOuter + spn->zStep;		
		}

		/* Interpolate RGBA */
		if (ctx->light->ShadeModel == GL_SMOOTH)
		{
			fdrInner = fdrOuter + spn->redStep;
			fdgInner = fdgOuter + spn->greenStep;
			fdbInner = fdbOuter + spn->blueStep;
			fdaInner = fdaOuter + spn->alphaStep;
		}
			
		opengl_debug("\t\tfdrOuter = %d, fdrInner = %d\n", fdrOuter, fdrInner);
		opengl_debug("\t\tfdgOuter = %d, fdgInner = %d\n", fdgOuter, fdgInner);
		opengl_debug("\t\tfdbOuter = %d, fdbInner = %d\n", fdbOuter, fdbInner);
		opengl_debug("\t\trLeft = %d, gLeft = %d, bLeft = %d\n", rLeft, gLeft, bLeft);

		/* Rasterize setup */
		while (lines > 0)
		{

			/* initialize the spn->interpolants to the leftmost value */
			/* ff = fixed-pt fragment */
			const GLint right = FixedToInt(fxRightEdge);
			spn->x = FixedToInt(fxLeftEdge);
			if (right <= spn->x)
				spn->end = 0;
			else
				spn->end = right - spn->x;

			/* Interpolate Z */
			if (ctx->context_cap->is_depth_test)
			{
				spn->z = zLeft;
				opengl_debug("\t\tSpan Z = %d zStep = %d\n", spn->z, spn->zStep );
			}

			/* Interpolate RGBA */
			if (ctx->light->ShadeModel == GL_SMOOTH)
			{
				spn->red = rLeft;
				spn->green = gLeft;
				spn->blue = bLeft;
				spn->alpha = aLeft;
			}


			/* This is where we actually generate fragments */
			if (spn->end > 0 && spn->y >= 0)
			{
				/* Mesa len = spn->end -1 */
				const GLint len = spn->end;
				opengl_debug("\t\tSpan from [%d, %d] with length %d\n", spn->x, spn->y, len);

				/* Interpolate RGBA */
				if (ctx->light->ShadeModel == GL_SMOOTH)
				{
					CLAMP_INTERPOLANT(red, redStep, len);
					CLAMP_INTERPOLANT(green, greenStep, len);
					CLAMP_INTERPOLANT(blue, blueStep, len);
					CLAMP_INTERPOLANT(alpha, alphaStep, len);

					opengl_debug("\t\tSpan RGBA [%d, %d, %d, %d]\n", FixedToChan(spn->red), 
												         FixedToChan(spn->blue), 
												         FixedToChan(spn->green), 
												         FixedToChan(spn->alpha));
					opengl_debug("\t\tRGBA Step [%d, %d, %d, %d]\n", FixedToChan(spn->redStep), 
													     FixedToChan(spn->blueStep), 
													     FixedToChan(spn->greenStep), 
													     FixedToChan(spn->alphaStep));
				}


				int i;
				for (i = 0; i < len; ++i)
				{
					/* Convert to Integer */
					GLint red = FixedToChan(spn->red) ;
					GLint green = FixedToChan(spn->green);
					GLint blue = FixedToChan(spn->blue);
					// GLint alpha = FixedToInt(spn->alpha);

					int color = (red << 16) + (green << 8)+ blue;
					/* Depth Test */
					// GLfixed z_val = spn->z;
					if (ctx->draw_buffer->depth_buffer->buffer[spn->y * ctx->draw_buffer->depth_buffer->width + spn->x] < spn->z)
					{
						GLint idx = spn->y * ctx->draw_buffer->depth_buffer->width + spn->x;
						if (idx <= ctx->draw_buffer->depth_buffer->width * ctx->draw_buffer->depth_buffer->height)
						{
							ctx->draw_buffer->depth_buffer->buffer[spn->y * ctx->draw_buffer->depth_buffer->width + spn->x] = spn->z;
							glut_frame_buffer_pixel(spn->x, spn->y, color);
						}
						else
							opengl_debug("\t\tOut of bound! [%d, %d] > [%d, %d]\n", 
												spn->x, spn->y, 
												ctx->draw_buffer->depth_buffer->width, ctx->draw_buffer->depth_buffer->height );
					};

					spn->z += spn->zStep;
					spn->red += spn->redStep;
					spn->green += spn->greenStep;
					spn->blue += spn->blueStep;
					spn->alpha += spn->alphaStep;
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

				/* Interpolate Z */
				if (ctx->context_cap->is_depth_test)
					zLeft += fdzOuter;

				/* Interpolate RGBA */
				if (ctx->light->ShadeModel == GL_SMOOTH)
				{
					rLeft += fdrOuter;
					gLeft += fdgOuter;
					bLeft += fdbOuter;
					aLeft += fdaOuter;
				}
			} else {

				/* Interpolate Z */
				if (ctx->context_cap->is_depth_test)
					zLeft += fdzInner;

				/* Interpolate RGBA */
				if (ctx->light->ShadeModel == GL_SMOOTH)
				{
					rLeft += fdrInner;
					gLeft += fdgInner;
					bLeft += fdbInner;
					aLeft += fdaInner;
				}

			}
		} /*while lines>0*/
	}


	/* Free edges */
	opengl_edge_free(edge_major);
	opengl_edge_free(edge_top);
	opengl_edge_free(edge_bottom);

	/* Free span*/
	opengl_span_free(spn);

}

/* TODO: use Mesa algorithm instead */
/* Bresenham's line algorithm */
void opengl_rasterizer_draw_line(struct opengl_context_t *ctx, GLint x1, GLint y1, GLint x2, GLint y2, GLuint color)
{

	GLint s_x;
	GLint s_y;
	GLint e_x;
	GLint e_y;

	if (x1 == x2)
	{
		opengl_debug("\t\tSlope = infinite\n");
		s_y = y1 < y2 ? y1 : y2;
		e_y = y1 > y2 ? y1: y2;
		glut_frame_buffer_pixel(x1, s_y, color);
		while(s_y < e_y)
		{ 
			s_y++;
			glut_frame_buffer_pixel(x1, s_y, color);				
		}
		return;
	}

	if (x1 < x2)
	{
		s_x = x1;
		s_y = y1;
		e_x = x2;
		e_y = y2;
	} else {
		s_x = x2;
		s_y = y2;
		e_x = x1;
		e_y = y1;
	}

	GLfloat m = (GLfloat)(y2 - y1) / (x2 - x1);
	opengl_debug("\t\tSlope = %f\n", m);

	if (m >= 0.0f && m < 1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dy -dx;
		GLint incrE = 2*dy;
		GLint incrNE = 2*(dy-dx);
		GLint x = s_x;
		GLint y = s_y;
		glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			x++;
			if ( e <= 0)
				e += incrE;
			else
			{
				y++;
				e += incrNE;
			}
			glut_frame_buffer_pixel(x, y, color);
		}
	}

	if ( m >= 1.0f )
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dx -dy;
		GLint incrE = 2*dx;
		GLint incrNE = 2*(dx-dy);
		GLint x = s_x;
		GLint y = s_y;
		glut_frame_buffer_pixel(x, y, color);
		while(y < e_y)
		{
			y++;
			if ( e <= 0)
				e += incrE;
			else
			{
				x++;
				e += incrNE;
			}
			glut_frame_buffer_pixel(x, y, color);
		}
	}

	if (m < 0.0f && m > -1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dy -dx;
		GLint incrE = 2*dy;
		GLint incrNE = 2*(dy-dx);
		GLint x = s_x;
		GLint y = s_y;
		glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			x++;
			if ( e <= 0)
				e += incrE;
			else
			{
				y--;
				e += incrNE;
			}
			glut_frame_buffer_pixel(x, y, color);
		}
	}

	if (m <= -1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dx -dy;
		GLint incrE = 2*dx;
		GLint incrNE = 2*(dx-dy);
		GLint x = s_x;
		GLint y = s_y;
		glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			y--;
			if ( e >= 0)
				e -= incrE;
			else
			{
				x++;
				e -= incrNE;
			}
			glut_frame_buffer_pixel(x, y, color);
		}
	}
}
