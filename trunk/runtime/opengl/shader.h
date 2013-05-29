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

#ifndef RUNTIME_OPENGL_SHADER_H
#define RUNTIME_OPENGL_SHADER_H

#include <pthread.h>
#include "opengl.h"

struct list_t;
struct linked_list_t;
struct opengl_si_shader_t;

/* Shader objects are stored in a linked list repository */
struct opengl_shader_obj_t
{
	unsigned int id;
	unsigned int type;
	int ref_count;
	pthread_mutex_t mutex;
	unsigned char delete_pending;

	/* 
	 * This list record programs that current shader bound to,
	 * need to detach from programs before deleting
	*/
	struct list_t *bound_programs;

	struct list_t *source; /* Source code */
	struct opengl_si_shader_t *shader; /* Shader binary */
};

extern struct linked_list_t *shader_repo;

void opengl_shader_obj_ref_update(struct opengl_shader_obj_t *shader_obj, int change);

struct linked_list_t *opengl_shader_obj_repo_create();
void opengl_shader_obj_repo_free(struct linked_list_t *shader_obj_repo);
struct opengl_shader_obj_t *opengl_shader_obj_repo_get_shader(struct linked_list_t *shader_obj_repo, unsigned int id);

#endif
