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

#ifndef DRIVER_OPENGL_VERTEX_ARRAY_H
#define DRIVER_OPENGL_VERTEX_ARRAY_H


#include <GL/glut.h>

#ifndef GL_MAX_VERTEX_ATTRIB_BINDINGS
#define GL_MAX_VERTEX_ATTRIB_BINDINGS 32
#endif

#define OPENGL_ATTACHED_BUFFER_ID_TABLE_INIT_SIZE 16

struct linked_list_t;
struct opengl_context_t;
struct opengl_vertex_buffer_obj_t;

struct opengl_vertex_client_array_t
{
	GLint size; /* the number of components per generic vertex attribute */
	GLenum type;
	GLenum format;
	GLsizei	 stride;
	GLsizei	 stride_byte;
	const GLubyte *ptr;
	GLboolean enabled;
	GLboolean normalized;
	GLboolean integer;
	GLuint instance_divisor;
	GLuint element_size;
	GLuint	max_element;
	struct opengl_vertex_buffer_obj_t *vbo;
};

struct opengl_vertex_array_obj_t
{
	GLint id;
	GLint ref_count;
	GLboolean delete_pending;
	struct opengl_vertex_client_array_t vtx_attrib[GL_MAX_VERTEX_ATTRIB_BINDINGS];
};

/* VAO/VBO binding point, it's a part of OpenGL context */
struct opengl_vertex_array_attrib_t
{
	struct opengl_vertex_array_obj_t *curr_vao;			/* Current VAO bound to OpenGL context */
	struct opengl_vertex_array_obj_t *default_vao;		/* Default VAO has id = 0 and initially is bound to curr_vao */
	struct opengl_vertex_buffer_obj_t *curr_vbo;			/* Current VBO bound to OpenGL context */
};

struct opengl_vertex_array_obj_t *opengl_vertex_array_obj_create();
void opengl_vertex_array_obj_free(struct opengl_vertex_array_obj_t *vao);
void opengl_vertex_array_obj_detele(struct opengl_vertex_array_obj_t *vao);
void opengl_vertex_array_obj_bind(struct opengl_vertex_array_obj_t *vao, struct opengl_context_t *ctx);
void opengl_vertex_array_obj_unbind(struct opengl_vertex_array_obj_t *vao, struct opengl_context_t *ctx);

struct linked_list_t *opengl_vertex_array_obj_repo_create();
void opengl_vertex_array_obj_repo_free(struct linked_list_t *vao_repo);
struct opengl_vertex_array_obj_t *opengl_vertex_array_obj_repo_get(struct linked_list_t *vao_repo, int id);
void opengl_vertex_array_obj_repo_add(struct linked_list_t *vao_repo, struct opengl_vertex_array_obj_t *vao);
int opengl_vertex_array_obj_repo_remove(struct linked_list_t *vao_repo, struct opengl_vertex_array_obj_t *vao);

void opengl_vertex_client_array_set_element_size(struct opengl_vertex_client_array_t *vtx_attrib, GLint size, GLenum type);

struct opengl_vertex_array_attrib_t *opengl_vertex_array_attrib_create();
void opengl_vertex_array_attrib_free(struct opengl_vertex_array_attrib_t *vao_attrib);

#endif
