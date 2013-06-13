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

#ifndef RUNTIME_OPENGL_PROGRAM_H
#define RUNTIME_OPENGL_PROGRAM_H

#include <pthread.h>
#include "opengl.h"

struct list_t;
struct linked_list_t;
struct opengl_shader_obj_t;

struct opengl_uniform_t
{
	unsigned int size;
	void *data;
};

struct opengl_program_obj_t
{
	unsigned int id;
	int ref_count;
	pthread_mutex_t mutex;
	unsigned char delete_pending;
	
	struct list_t *shaders;
	void *binary;
	unsigned int binary_size;
};

extern struct linked_list_t *program_repo;

struct linked_list_t *opengl_program_obj_repo_create();
void opengl_program_obj_repo_free(struct linked_list_t *program_obj_repo);
struct opengl_program_obj_t *opengl_program_obj_repo_get_program(struct linked_list_t *program_obj_repo, unsigned int id);

#endif
