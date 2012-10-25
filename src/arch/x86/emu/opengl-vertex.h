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
#ifndef ARCH_X86_EMU_OPENGL_VERTEX_H
#define ARCH_X86_EMU_OPENGL_VERTEX_H

#include "emu.h"
#include <mem-system/mem-system.h>
#include <GL/gl.h>
#include "opengl-color-channel.h"
 
#define R_COMP 0
#define G_COMP 1
#define B_COMP 2
#define A_COMP 3

/* OpenGL vertex */
struct x86_opengl_vertex_t
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
	GLchan color[4];
};

/* OpenGL vertex group */
struct x86_opengl_vertex_group_t
{
	/* Primitive type is defined in gl.h */
	GLenum primitive_type;
	/* Element in this list has data type x86_opengl_vertex_t */
	struct list_t *vertex_list;
};

/* OpenGL vertex buffer */
struct x86_opengl_vertex_buffer_t
{
	/* Element in this list has data type x86_opengl_vertex_group_t */
	struct list_t *vertex_groups;
	/* Points to current vertex group */
	struct x86_opengl_vertex_group_t *current_vertex_group;
};

void x86_opengl_vertex_set_color(GLchan *color, struct x86_opengl_vertex_t *vtx);
int x86_opengl_vertex_get_color(struct x86_opengl_vertex_t *vtx);

struct x86_opengl_vertex_t *x86_opengl_vertex_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void x86_opengl_vertex_free(struct x86_opengl_vertex_t *vtx);

struct x86_opengl_vertex_group_t *x86_opengl_vertex_group_create(GLenum primitive_type);
void x86_opengl_vertex_group_free(struct x86_opengl_vertex_group_t *vtxgp);

struct x86_opengl_vertex_buffer_t *x86_opengl_vertex_buffer_create();
void x86_opengl_vertex_buffer_free(struct x86_opengl_vertex_buffer_t *vtxbf);
void x86_opengl_vertex_buffer_add_vertex_group(struct x86_opengl_vertex_buffer_t *vtxbf, struct x86_opengl_vertex_group_t *vtxgp);
void x86_opengl_vertex_buffer_add_vertex(struct x86_opengl_vertex_buffer_t *vtxbf, struct x86_opengl_vertex_t *vtx);

#endif
