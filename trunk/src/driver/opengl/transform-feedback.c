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
	
	/* Update global tfo id */
	tfo_id += 1;

	opengl_debug("\t\tTransform Feedback Object #%d [%p] Created\n", tfo->id, tfo);

	/* Return */	
	return tfo;
}

/* Free checks no flags */
void opengl_transform_feedback_obj_free(struct opengl_transform_feedback_obj_t *tfo)
{
	/* Free */
	free(tfo);
}

/* Delete checks flags */
void opengl_transform_feedback_obj_detele(struct opengl_transform_feedback_obj_t *tfo)
{
	if (tfo->ref_count == 0)
		opengl_transform_feedback_obj_free(tfo);
}

struct opengl_transform_feedback_state_t *opengl_transform_feedback_binding_create()
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

void opengl_transform_feedback_binding_free(struct opengl_transform_feedback_state_t *tfst)
{
	opengl_transform_feedback_obj_repo_free(tfst->tfo_repo);
	free(tfst->default_tfo);
	free(tfst);
}

struct linked_list_t *opengl_transform_feedback_obj_repo_create()
{
	struct linked_list_t *lst;

	lst = linked_list_create();

	opengl_debug("\tCreated Transform Feedback Object repository [%p]\n", lst);

	return lst;
}

void opengl_transform_feedback_obj_repo_free(struct linked_list_t *tfo_repo)
{
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

	opengl_debug("\tFreed Transform Feedback Object repository [%p]\n", tfo_repo);

}

void opengl_transform_feedback_obj_repo_add(struct linked_list_t *tfo_repo, struct opengl_transform_feedback_obj_t *tfo)
{
	linked_list_add(tfo_repo, tfo);
	opengl_debug("\tInserting Transform Feedback Object #%d [%p] into Transform Feedback Object repository [%p]\n", tfo->id, tfo, tfo_repo);	
}

int opengl_transform_feedback_obj_repo_remove(struct linked_list_t *tfo_repo, struct opengl_transform_feedback_obj_t *tfo)
{
	if (tfo->ref_count != 0)
	{
		opengl_debug("\tTransform Feedback Object #%d [%p] cannot be removed immediately, as reference counter = %d\n", tfo->id, tfo, tfo->ref_count);
		return -1;
	}
	else 
	{
		/* Check if Transform Feedback Object exists */
		linked_list_find(tfo_repo, tfo);
		if (tfo_repo->error_code)
			fatal("%s: Transform Feedback Object does not exist", __FUNCTION__);
		linked_list_remove(tfo_repo);
		opengl_debug("\tTransform Feedback Object #%d [%p] removed from Transform Feedback Object table [%p]\n", tfo->id, tfo, tfo_repo);
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

