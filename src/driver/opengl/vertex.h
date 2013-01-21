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

#ifndef DRIVER_OPENGL_VERTEX_H
#define DRIVER_OPENGL_VERTEX_H

#include <GL/gl.h>

#include "color-channel.h"
 

#define X_COMP 0
#define Y_COMP 1
#define Z_COMP 2
#define W_COMP 3

#define R_COMP  X_COMP
#define G_COMP Y_COMP
#define B_COMP Z_COMP
#define A_COMP W_COMP

/* OpenGL vertex */
struct opengl_vertex_t
{
	GLfloat pos[4];
	GLchan color[4];
	GLfloat normal[4];
};

/* OpenGL vertex group */
struct opengl_vertex_group_t
{
	/* Primitive type is defined in gl.h */
	GLenum primitive_type;

	/* Element in this list has data type opengl_vertex_t */
	struct list_t *vertex_list;
};

/* OpenGL vertex buffer */
struct opengl_vertex_buffer_t
{
	/* Element in this list has data type opengl_vertex_group_t */
	struct list_t *vertex_groups;
	/* Points to current vertex group */
	struct opengl_vertex_group_t *current_vertex_group;
};

struct opengl_vertex_t *opengl_vertex_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void opengl_vertex_free(struct opengl_vertex_t *vtx);

void opengl_vertex_set_color(GLchan *color, struct opengl_vertex_t *vtx);
int opengl_vertex_get_color(struct opengl_vertex_t *vtx);

void opengl_vertex_set_normal(GLfloat *nrml, struct opengl_vertex_t *vtx);

struct opengl_vertex_group_t *opengl_vertex_group_create(GLenum primitive_type);
void opengl_vertex_group_free(struct opengl_vertex_group_t *vtxgp);

struct opengl_vertex_buffer_t *opengl_vertex_buffer_create();
void opengl_vertex_buffer_free(struct opengl_vertex_buffer_t *vtxbf);
void opengl_vertex_buffer_add_vertex_group(struct opengl_vertex_buffer_t *vtxbf, struct opengl_vertex_group_t *vtxgp);
void opengl_vertex_buffer_add_vertex(struct opengl_vertex_buffer_t *vtxbf, struct opengl_vertex_t *vtx);

#endif
