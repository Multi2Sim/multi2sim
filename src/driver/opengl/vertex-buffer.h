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

#ifndef DRIVER_OPENGL_VERTEX_BUFFER_H
#define DRIVER_OPENGL_VERTEX_BUFFER_H

#include <GL/glut.h>

struct linked_list_t;

/* VBOs are stored in a hash table, the id is the key */
struct opengl_vertex_buffer_obj_t
{
	GLuint			id;
	GLint 			ref_count;
	GLboolean		delete_pending;
	void*			data;
	GLenum		usage;
};

struct opengl_vertex_buffer_obj_t *opengl_vertex_buffer_obj_create();
void opengl_vertex_buffer_obj_free(struct opengl_vertex_buffer_obj_t *vbo);
void opengl_vertex_buffer_obj_detele(struct opengl_vertex_buffer_obj_t *vbo);
void opengl_vertex_buffer_obj_data(struct opengl_vertex_buffer_obj_t *vbo, GLsizeiptr size, const GLvoid *data, GLenum usage);

struct linked_list_t *opengl_vertex_buffer_obj_repo_create();
void opengl_vertex_buffer_obj_repo_free(struct linked_list_t *vbo_repo);
void opengl_vertex_buffer_obj_repo_add(struct linked_list_t *vbo_repo, struct opengl_vertex_buffer_obj_t *vbo);
int opengl_vertex_buffer_obj_repo_remove(struct linked_list_t *vbo_repo, struct opengl_vertex_buffer_obj_t *vbo);
struct opengl_vertex_buffer_obj_t *opengl_vertex_buffer_obj_repo_get(struct linked_list_t *vbo_repo, int id);

#endif
