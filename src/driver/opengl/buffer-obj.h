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

#ifndef DRIVER_OPENGL_BUFFER_OBJ_H
#define DRIVER_OPENGL_BUFFER_OBJ_H

#include <GL/glut.h>
#include <pthread.h>

struct linked_list_t;

/* Buffers are stored in a linked list repository */
struct opengl_buffer_obj_t
{
	unsigned int id;
	int ref_count;
	pthread_mutex_t ref_mutex;
	unsigned char delete_pending;

	/* Data */
	void* data;
	unsigned int data_size;
	unsigned int usage;

	/* Mapping to user address space */
	unsigned int map_access_flags; /* Access permission R, W, R/W */
	unsigned int map_pointer; /* User space address of mapping */
	unsigned int map_length; /* User space mapping size */
};

struct opengl_buffer_obj_t *opengl_buffer_obj_create();
void opengl_buffer_obj_free(struct opengl_buffer_obj_t *buf_obj);
void opengl_buffer_obj_detele(struct opengl_buffer_obj_t *buf_obj);
void opengl_buffer_obj_ref_update(struct opengl_buffer_obj_t *buf_obj, int change);
void opengl_buffer_obj_data(struct opengl_buffer_obj_t *buf_obj, unsigned int size, const void *data, unsigned int usage);

struct linked_list_t *opengl_buffer_obj_repo_create();
void opengl_buffer_obj_repo_free(struct linked_list_t *buf_obj_repo);
void opengl_buffer_obj_repo_add(struct linked_list_t *buf_obj_repo, struct opengl_buffer_obj_t *buf_obj);
int opengl_buffer_obj_repo_remove(struct linked_list_t *buf_obj_repo, struct opengl_buffer_obj_t *buf_obj);
struct opengl_buffer_obj_t *opengl_buffer_obj_repo_get(struct linked_list_t *buf_obj_repo, int id);
struct opengl_buffer_obj_t *opengl_buffer_obj_repo_reference(struct linked_list_t *buf_obj_repo, int id);

#endif
