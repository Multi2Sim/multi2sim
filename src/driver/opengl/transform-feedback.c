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

#include <assert.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>

#include "opengl.h"
#include "buffer-obj.h"
#include "transform-feedback.h"

int tfo_id = 1;

struct opengl_transform_feedback_obj_t *opengl_transform_feedback_obj_create()
{
	struct opengl_transform_feedback_obj_t *tfo;

	/* Allocate */
	tfo = xcalloc(1, sizeof(struct opengl_transform_feedback_obj_t));

	/* Initialization */
	tfo->id = tfo_id;
	tfo->ref_count = 0;
	pthread_mutex_init(&tfo->ref_mutex, NULL);
	
	/* Update global tfo id */
	tfo_id += 1;

	/* Debug */
	opengl_debug("\t\tTransform Feedback Object #%d [%p] Created\n", tfo->id, tfo);

	/* Return */	
	return tfo;
}

/* Free checks no flags */
void opengl_transform_feedback_obj_free(struct opengl_transform_feedback_obj_t *tfo)
{
	/* Free */
	pthread_mutex_destroy(&tfo->ref_mutex);
	free(tfo);
}

/* Update reference count */
void opengl_transform_feedback_obj_ref_update(struct opengl_transform_feedback_obj_t *tfo, int change)
{
	int count;

	pthread_mutex_lock(&tfo->ref_mutex);
	tfo->ref_count += change;
	count = tfo->ref_count;
	pthread_mutex_unlock(&tfo->ref_mutex);

	if (count < 0)
		panic("%s: number of references is negative", __FUNCTION__);
}

/* Delete checks flags */
void opengl_transform_feedback_obj_detele(struct opengl_transform_feedback_obj_t *tfo)
{
	/* Mark as delete pinding */
	tfo->delete_pending = GL_TRUE;

	/* Free object when it is not in use and active */
	if (!tfo->ref_count && !tfo->active)
		opengl_transform_feedback_obj_free(tfo);
}

struct opengl_transform_feedback_state_t *opengl_transform_feedback_state_create()
{
	struct opengl_transform_feedback_state_t *tfst;
	struct opengl_transform_feedback_obj_t *default_tfo;

	/* Allocate */
	tfst = xcalloc(1, sizeof(struct opengl_transform_feedback_state_t));
	default_tfo = xcalloc(1, sizeof(struct opengl_transform_feedback_obj_t));

	/* Initialize */
	default_tfo->id = 0;
	tfst->tfo_repo = opengl_transform_feedback_obj_repo_create();
	tfst->curr_tfo = default_tfo;
	tfst->default_tfo = default_tfo;

	/* Return */
	return tfst;
}

void opengl_transform_feedback_state_free(struct opengl_transform_feedback_state_t *tfst)
{
	opengl_transform_feedback_obj_repo_free(tfst->tfo_repo);
	free(tfst->default_tfo);
	free(tfst);
}

void opengl_transform_feedback_state_attach_buffer_obj(struct opengl_transform_feedback_state_t *tfst, struct opengl_buffer_obj_t *buf_obj)
{
	/* Only bind to general binding point */
	tfst->curr_buf = buf_obj;

	/* Debug */
	opengl_debug("\t\tBuffer Object #%d [%p] bind to Transform Feedback general binding point\n", 
		buf_obj->id, buf_obj);
}

void opengl_transform_feedback_state_attach_buffer_obj_indexed(struct opengl_transform_feedback_state_t *tfst, struct opengl_buffer_obj_t *buf_obj, unsigned int index)
{
	struct opengl_transform_feedback_obj_t *tfo;

	if (index < MAX_FEEDBACK_BUFFERS)
	{
		/* Attach to general binding point */
		opengl_transform_feedback_state_attach_buffer_obj(tfst, buf_obj);

		/* Attach to indexed binding point inside the currently bound TFO*/
		tfo = tfst->curr_tfo;
		if (tfo->id == 0)
		{
			opengl_debug("\t\tTransform Feedback Object ID #0 cannot be used as a binding target\n");
			return;
		}
		else
		{
			tfo->buf_id[index] = buf_obj->id;
			tfo->buffers[index] = buf_obj;
			tfo->size[index] = buf_obj->data_size;
			/* FIXME: offset and requested size array */
			opengl_debug("\t\tBuffer Object #%d [%p] attach to Transform Feedback Object #%d [%p]: binding point idx[%d]\n", 
				buf_obj->id, buf_obj, tfo->id, tfo, index);		
		}

	}
	else
		opengl_debug("\t\tIndex %d out of range: [0 - %d]\n", index, MAX_FEEDBACK_BUFFERS);
}

struct linked_list_t *opengl_transform_feedback_obj_repo_create()
{
	struct linked_list_t *lst;

	/* Create repository */
	lst = linked_list_create();

	/* Debug */
	opengl_debug("\tCreated Transform Feedback Object repository [%p]\n", lst);

	/* Return */
	return lst;
}

void opengl_transform_feedback_obj_repo_free(struct linked_list_t *tfo_repo)
{
	/* Debug */
	opengl_debug("\tFree Transform Feedback Object repository [%p]\n", tfo_repo);

	struct opengl_transform_feedback_obj_t *tfo;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(tfo_repo)
	{
		tfo = linked_list_get(tfo_repo);
		assert(tfo);
		opengl_transform_feedback_obj_free(tfo);
	}

	/* Free tfo repository */
	linked_list_free(tfo_repo);
}

void opengl_transform_feedback_obj_repo_add(struct linked_list_t *tfo_repo, struct opengl_transform_feedback_obj_t *tfo)
{
	/* Add a TFO to repository */
	linked_list_add(tfo_repo, tfo);

	/* Debug */
	opengl_debug("\tInserting Transform Feedback Object #%d [%p] into Transform Feedback Object repository [%p]\n", 
		tfo->id, tfo, tfo_repo);	
}

int opengl_transform_feedback_obj_repo_remove(struct linked_list_t *tfo_repo, struct opengl_transform_feedback_obj_t *tfo)
{
	if (tfo->ref_count != 0)
	{
		tfo->delete_pending = 1;
		opengl_debug("\tTransform Feedback Object #%d [%p] cannot be removed immediately, as reference counter = %d\n", 
			tfo->id, tfo, tfo->ref_count);
		return -1;
	}
	else 
	{
		/* Find, remove */
		linked_list_find(tfo_repo, tfo);
		if (tfo_repo->error_code)
			fatal("%s: Transform Feedback Object does not exist", __FUNCTION__);
		linked_list_remove(tfo_repo);
		opengl_debug("\tTransform Feedback Object #%d [%p] removed from Transform Feedback Object table [%p]\n", 
			tfo->id, tfo, tfo_repo);
		return 1;
	}
}

struct opengl_transform_feedback_obj_t *opengl_transform_feedback_obj_repo_get(struct linked_list_t *tfo_repo, int id)
{
	struct opengl_transform_feedback_obj_t *tfo;

	/* Search Transform Feedback Object */
	LINKED_LIST_FOR_EACH(tfo_repo)
	{
		tfo = linked_list_get(tfo_repo);
		assert(tfo);
		if (tfo->id == id)
			return tfo;
	}

	/* Not found */
	fatal("%s: requested Transform Feedback Object does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}

struct opengl_transform_feedback_obj_t *opengl_transform_feedback_obj_repo_reference(struct linked_list_t *tfo_repo, int id)
{
	struct opengl_transform_feedback_obj_t *tfo;

	/* Get transform feedback object and update the reference count */
	tfo = opengl_transform_feedback_obj_repo_get(tfo_repo, id);
	if (!tfo->delete_pending)
	{
		opengl_transform_feedback_obj_ref_update(tfo, 1);
		return tfo;
	}
	else
	{
		opengl_debug("\tCannot reference Transform Feedback Object #%d [%p]: delete pending\n", 
			tfo->id, tfo);
		return NULL;	
	}
}