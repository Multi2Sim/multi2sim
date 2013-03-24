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

#ifndef DRIVER_OPENGL_EDGE_H
#define DRIVER_OPENGL_EDGE_H


#include <GL/glut.h>

struct opengl_vertex_t;
struct opengl_edge_t
{
	struct opengl_vertex_t *vtx0; 	/* Y(vtx0) < Y(vtx1) */
	struct opengl_vertex_t *vtx1;
	GLfloat dx;				/* X(vtx1) - X(vtx0) */
	GLfloat dy;				/* Y(vtx1) - Y(vtx0) */
	GLfloat dxdy;				/* dx/dy */
	GLint fdxdy;				/* dx/dy in fixed-point */
	GLfloat adjy;				/* adjust from v[0]->fy to fsy, scaled */
	GLint fsx;				/* first sample point x coord */
	GLint fsy;				/* first sample point y coord */
	GLint fx0;				/* fixed pt X of lower endpoint */
	GLint lines;				/* number of lines to be sampled on this edge */	
};

struct opengl_edge_t *opengl_edge_create(struct opengl_vertex_t *vtx0, struct opengl_vertex_t *vtx1);
void opengl_edge_free(struct opengl_edge_t *edge);

#endif
