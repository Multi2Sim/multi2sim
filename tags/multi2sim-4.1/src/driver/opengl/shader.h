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

#ifndef DRIVER_OPENGL_SHADER_H
#define DRIVER_OPENGL_SHADER_H


#include <GL/glut.h>
#include <pthread.h>

#define OPENGL_SHADER_TABLE_INIT_SIZE	16
struct linked_list_t;
struct si_ndrange_t;

/* Shaders are stored in a linked list */
struct opengl_shader_t
{
	GLenum type;
	GLuint id;
	GLint ref_count;
	pthread_mutex_t ref_mutex;
	GLboolean delete_pending;
	
	void *isa_buffer;
};

struct opengl_shader_t *opengl_shader_create(GLenum type);
void opengl_shader_free(struct opengl_shader_t *shdr);
void opengl_shader_detele(struct opengl_shader_t *shdr);
void opengl_shader_ref_update(struct opengl_shader_t *shdr, int change);

struct linked_list_t *opengl_shader_repo_create();
void opengl_shader_repo_free(struct linked_list_t *shdr_tbl);
void opengl_shader_repo_add(struct linked_list_t *shdr_tbl, struct opengl_shader_t *shdr);
int opengl_shader_repo_remove(struct linked_list_t *shdr_tbl, struct opengl_shader_t *shdr);
struct opengl_shader_t *opengl_shader_repo_get(struct linked_list_t *shdr_tbl, int id);
struct opengl_shader_t *opengl_shader_repo_reference(struct linked_list_t *shdr_tbl, int id);

#endif
