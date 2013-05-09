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

/* General buffer object binding targets */
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

/* Indexed buffer object binding targets */
struct opengl_indexed_buffer_binding_points_t
{
	unsigned int max_indexed_targets;
	struct opengl_buffer_binding_target_t **atomic_counter_buffer;
	struct opengl_buffer_binding_target_t **shader_storage_buffer;
	struct opengl_buffer_binding_target_t **transform_feedback_buffer;
	struct opengl_buffer_binding_target_t **uniform_buffer;
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
	 * These lists record where buffer object get referenced
	 * Need to dereference when buffer is deleted
	 */
	struct list_t *bound_targets; /* Components with type opengl_buffer_binding_target_t */
	struct list_t *bound_vattribs; /* Components with  type opengl_vertex_attrib_t */

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

void opengl_buffer_obj_ref_update(struct opengl_buffer_obj_t *buffer_obj, int change);

struct opengl_buffer_binding_points_t *opengl_buffer_binding_points_create();
void opengl_buffer_binding_points_free(struct opengl_buffer_binding_points_t *bbp);
struct opengl_buffer_binding_target_t *opengl_buffer_binding_points_get_target(
	struct opengl_buffer_binding_points_t *bbp, unsigned int target_id);

struct opengl_indexed_buffer_binding_points_t *opengl_indexed_buffer_binding_points_create(unsigned int max_indexed_target);
void opengl_indexed_buffer_binding_points_free(struct opengl_indexed_buffer_binding_points_t *idx_bbp);

struct linked_list_t *opengl_buffer_obj_repo_create();
void opengl_buffer_obj_repo_free(struct linked_list_t *buffer_obj_repo);
struct opengl_buffer_obj_t *opengl_buffer_obj_repo_get(struct linked_list_t *buffer_obj_repo, unsigned int id);


#endif
