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

#ifndef RUNTIME_OPENGL_BUFFER_OBJ_H
#define RUNTIME_OPENGL_BUFFER_OBJ_H

#include <pthread.h>

#include "opengl.h"


/* To record ID of bound buffer*/
struct opengl_buffer_binding_target_t
{
	pthread_mutex_t mutex;
	unsigned int bound_buffer_id;
};

/* Buffer object binding targets */
struct opengl_buffer_binding_points_t
{
	struct opengl_buffer_binding_target_t *array_buffer;
	struct opengl_buffer_binding_target_t *atomic_counter_buffer;
	struct opengl_buffer_binding_target_t *copy_read_buffer;
	struct opengl_buffer_binding_target_t *copy_write_buffer;
	struct opengl_buffer_binding_target_t *dispatch_indirect_buffer;
	struct opengl_buffer_binding_target_t *draw_indirect_buffer;
	struct opengl_buffer_binding_target_t *element_array_buffer;
	struct opengl_buffer_binding_target_t *pixel_pack_buffer;
	struct opengl_buffer_binding_target_t *pixel_unpack_buffer;
	struct opengl_buffer_binding_target_t *shader_storage_buffer;
	struct opengl_buffer_binding_target_t *texture_buffer;
	struct opengl_buffer_binding_target_t *transform_feedback_buffer;
	struct opengl_buffer_binding_target_t *uniform_buffer;
};

/* Buffers are stored in a linked list repository */
struct opengl_buffer_obj_t
{
	/* Buffer info */
	unsigned int id;
	int ref_count;
	pthread_mutex_t ref_mutex;
	unsigned char delete_pending;

	/*
	 * This list records binding target objects bind with this buffer object 
	 * Need to unattach from binding targets when buffer is deleted
	 */
	struct list_t *bound_targets;

	/* Buffer data storage */
	void* data;
	unsigned int size;
	unsigned int usage;
	unsigned int access;

	/* Mapping info */
	unsigned int map_access_flags;
	unsigned char mapped;
	void *map_pointer; 
	unsigned int map_offset; 
	unsigned int map_length; 
};

extern struct linked_list_t *buffer_repo;

struct opengl_buffer_binding_points_t *opengl_buffer_binding_points_create();
void opengl_buffer_binding_points_free(struct opengl_buffer_binding_points_t *bbp);

struct linked_list_t *opengl_buffer_obj_repo_create();
void opengl_buffer_obj_repo_free(struct linked_list_t *buffer_obj_repo);


#endif
