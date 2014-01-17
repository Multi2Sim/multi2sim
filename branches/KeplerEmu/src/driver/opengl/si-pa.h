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

#ifndef DRIVER_OPENGL_SI_PA_H
#define DRIVER_OPENGL_SI_PA_H

/* 
 * Primary functionality for primitive assembler 
 *     1. Clipping
 *     2. Viewport tranform
 *     3. Primitive rasterization set-up
 */

#define X_COMP 0
#define Y_COMP 1
#define Z_COMP 2
#define W_COMP 3

enum opengl_pa_primitive_mode_t
{
	OPENGL_PA_POINTS = 0,
	OPENGL_PA_LINE_STRIP, 
	OPENGL_PA_LINE_LOOP, 
	OPENGL_PA_LINES, 
	OPENGL_PA_LINE_STRIP_ADJACENCY, 
	OPENGL_PA_LINES_ADJACENCY, 
	OPENGL_PA_TRIANGLE_STRIP, 
	OPENGL_PA_TRIANGLE_FAN, 
	OPENGL_PA_TRIANGLES, 
	OPENGL_PA_TRIANGLE_STRIP_ADJACENCY, 
	OPENGL_PA_TRIANGLES_ADJACENCY,
	OPENGL_PA_PATCHES,
	OPENGL_PA_INVALID
};


/* Forward declaration */
struct opengl_pa_edge_func_t;
struct list_t;

/* Viewport is used to transform vertex from normalized device coordinate to window coordinate */
struct opengl_pa_viewport_t
{
	int x;
	int y;
	int width;
	int height;
};

/* DepthRange is used to transform vertex from normalized device coordinate to window coordinate */
struct opengl_pa_depth_range_t
{
	double n;
	double f;
};

/* Initially, vertex positions are in normalized device coordinate */
struct opengl_pa_vertex_t
{
	float pos[4];
};

/* Used to check if a pixel is inside the triangle */
struct opengl_pa_edge_func_t
{
	float a;
	float b;
	float c;
};

/* Assembled triangle */
struct opengl_pa_triangle_t
{
	/* 3 verticies */
	struct opengl_pa_vertex_t *vtx0;
	struct opengl_pa_vertex_t *vtx1;
	struct opengl_pa_vertex_t *vtx2;

	/* And 3 edge functions */
	struct opengl_pa_edge_func_t *edgfunc0;
	struct opengl_pa_edge_func_t *edgfunc1;
	struct opengl_pa_edge_func_t *edgfunc2;	
};

/* Primitive list */
struct opengl_pa_primitive_t
{
	enum opengl_pa_primitive_mode_t mode;
	struct list_t *list;
};

struct opengl_pa_viewport_t *opengl_pa_viewport_create();
void opengl_pa_viewport_free(struct opengl_pa_viewport_t *vwpt);
void opengl_pa_viewport_set(struct opengl_pa_viewport_t *vwpt, int x, int y, int width, int height);
void opengl_pa_viewport_apply(struct opengl_pa_viewport_t *vwpt, struct opengl_pa_vertex_t *vtx);

struct opengl_pa_depth_range_t *opengl_pa_depth_range_t();
void opengl_pa_depth_range_free(struct opengl_pa_depth_range_t *vwpt);
void opengl_pa_depth_range_apply(struct opengl_pa_depth_range_t *vwpt, struct opengl_pa_vertex_t *vtx);

struct opengl_pa_vertex_t *opengl_pa_vertex_create(float x, float y, float z, float w);
void opengl_pa_vertex_free(struct opengl_pa_vertex_t *vtx);

/* Element in pos_lst is a 4 float array */
struct opengl_pa_primitive_t *opengl_pa_primitives_create(enum opengl_pa_primitive_mode_t mode, 
	struct list_t *pos_lst, struct opengl_pa_viewport_t *vwpt);
void opengl_pa_primitives_free(struct opengl_pa_primitive_t *prmtv);
enum opengl_pa_primitive_mode_t opengl_pa_primitive_get_mode(int mode);

struct opengl_pa_triangle_t *opengl_pa_triangle_create();
void opengl_pa_triangle_free(struct opengl_pa_triangle_t *triangle);
void opengl_pa_triangle_set(struct opengl_pa_triangle_t *triangle, struct opengl_pa_vertex_t *vtx0,
	struct opengl_pa_vertex_t *vtx1, struct opengl_pa_vertex_t *vtx2);

struct opengl_pa_edge_func_t *opengl_pa_edge_func_create();
void opengl_pa_edge_func_free(struct opengl_pa_edge_func_t *edge_func);
void opengl_pa_edge_func_set(struct opengl_pa_edge_func_t *edge_func, 
	struct opengl_pa_vertex_t *vtx0, struct opengl_pa_vertex_t *vtx1);


#endif
