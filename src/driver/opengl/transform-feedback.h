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

#ifndef DRIVER_OPENGL_TRANSFORM_FEEDBACK_H
#define DRIVER_OPENGL_TRANSFORM_FEEDBACK_H

#include <GL/glut.h>
#include <pthread.h>

#ifndef MAX_FEEDBACK_BUFFERS
#define MAX_FEEDBACK_BUFFERS 32
#endif

struct linked_list_t;
struct opengl_buffer_obj_t;

struct opengl_transform_feedback_obj_t
{
	unsigned int id;
	int ref_count;
	pthread_mutex_t ref_mutex;
	unsigned char delete_pending;
	unsigned char active;
	unsigned char paused;
	unsigned char ended_anytime;
	unsigned char ever_bound;

	/* Indexed buffer object binding points */
	unsigned int buf_id[MAX_FEEDBACK_BUFFERS];
	struct opengl_buffer_obj_t *buffers[MAX_FEEDBACK_BUFFERS];
	int offset[MAX_FEEDBACK_BUFFERS];
	unsigned int size[MAX_FEEDBACK_BUFFERS];
	unsigned int requested_size[MAX_FEEDBACK_BUFFERS];
};

struct opengl_transform_feedback_state_t
{
	unsigned int mode;	/* GL_POINTS, GL_LINES or GL_TRIANGLES */
	struct opengl_buffer_obj_t *curr_buf; /* General binding point */
	struct linked_list_t *tfo_repo;	/* Repository contains all transform feedback objects */
	struct opengl_transform_feedback_obj_t *curr_tfo;
	struct opengl_transform_feedback_obj_t *default_tfo;
};

struct opengl_transform_feedback_obj_t *opengl_transform_feedback_obj_create();
void opengl_transform_feedback_obj_free(struct opengl_transform_feedback_obj_t *tfo);
void opengl_transform_feedback_obj_detele(struct opengl_transform_feedback_obj_t *tfo);
void opengl_transform_feedback_obj_ref_update(struct opengl_transform_feedback_obj_t *tfo, int change);

struct opengl_transform_feedback_state_t *opengl_transform_feedback_state_create();
void opengl_transform_feedback_state_free(struct opengl_transform_feedback_state_t *tfst);
void opengl_transform_feedback_state_attach_buffer_obj(struct opengl_transform_feedback_state_t *tfst, struct opengl_buffer_obj_t *buf_obj);
void opengl_transform_feedback_state_attach_buffer_obj_indexed(struct opengl_transform_feedback_state_t *tfst, struct opengl_buffer_obj_t *buf_obj, unsigned int index);

struct linked_list_t *opengl_transform_feedback_obj_repo_create();
void opengl_transform_feedback_obj_repo_free(struct linked_list_t *tfo_repo);
void opengl_transform_feedback_obj_repo_add(struct linked_list_t *tfo_repo, struct opengl_transform_feedback_obj_t *tfo);
int opengl_transform_feedback_obj_repo_remove(struct linked_list_t *tfo_repo, struct opengl_transform_feedback_obj_t *tfo);
struct opengl_transform_feedback_obj_t *opengl_transform_feedback_obj_repo_get(struct linked_list_t *tfo_repo, int id);
struct opengl_transform_feedback_obj_t *opengl_transform_feedback_obj_repo_reference(struct linked_list_t *tfo_repo, int id);

#endif
