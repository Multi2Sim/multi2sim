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

#ifndef RUNTIME_OPENGL_VERTEX_ARRAY_OBJ_H
#define RUNTIME_OPENGL_VERTEX_ARRAY_OBJ_H

#include "opengl.h"

#include <pthread.h>

#ifndef MAX_VERTEX_ATTRIBS
#define MAX_VERTEX_ATTRIBS 24
#endif

struct linked_list_t;
struct opengl_buffer_obj_t;

/* Generic vertex attribute */
struct opengl_vertex_attrib_t
{
	int size;	/* Note, this is the number of component, from 1 to 4 or BGRA */
	unsigned int type;
	unsigned int format;
	unsigned int stride;
	unsigned int stride_byte;
	unsigned int pointer;
	unsigned char enabled;
	unsigned char normalized;
	unsigned char integer;
	unsigned int instance_divisor;
	unsigned int element_size;
	unsigned int max_element;
	struct opengl_buffer_obj_t *vbo;
};

struct opengl_vertex_array_obj_t
{
	unsigned int id;
	int ref_count;
	pthread_mutex_t ref_mutex;
	unsigned char delete_pending;
	
	/* Array of generic vertex attribute */
	unsigned int attribs_count;
	struct opengl_vertex_attrib_t **attribs;
};

void opengl_vertex_attrib_attach_buffer_obj(struct opengl_vertex_attrib_t *vattrib, struct opengl_buffer_obj_t *buffer_obj);
void opengl_vertex_attrib_detach_buffer_obj(struct opengl_vertex_attrib_t *vattrib, struct opengl_buffer_obj_t *buffer_obj);

struct linked_list_t *opengl_vertex_array_obj_repo_create();
void opengl_vertex_array_obj_repo_free(struct linked_list_t *vtx_array_obj_repo);


#endif
