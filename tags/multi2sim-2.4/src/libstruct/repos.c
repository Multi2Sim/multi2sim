/*
 *  Libstruct
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "repos.h"

struct objtail_t {
	int id;
	int status;  /* 0=free, 1=allocated */
	void *prev;
	void *next;
};


struct repos_t {
	char *name;
	int id;
	int objsize;
	void *alloc_head;
	void *unalloc_head;
};


struct repos_t *repos_create(int objsize, char *name)
{
	struct repos_t *repos;
	if (objsize <= 0)
		return NULL;
	repos = calloc(1, sizeof(struct repos_t));
	if (!repos)
		return NULL;
	repos->id = random();
	repos->name = name;
	repos->objsize = objsize;
	return repos;
}


void repos_free_dump(struct repos_t *repos, void(*dump)(void *, FILE *))
{
	void *obj, *next_obj;
	struct objtail_t *objtail;
	int count = 0;

	/* Free objects in unallocated list */
	for (obj = repos->unalloc_head; obj; obj = next_obj) {
		objtail = obj + repos->objsize;
		next_obj = objtail->next;
		free(obj);
	}

	/* Free objects in allocated list */
	count = 0;
	for (obj = repos->alloc_head; obj; obj = next_obj) {
		objtail = obj + repos->objsize;
		next_obj = objtail->next;
		count++;
		if (dump) {
			fprintf(stderr, "warning: %s: object not freed: ", repos->name);
			dump(obj, stderr);
			fprintf(stderr, "\n");
		}
		free(obj);
	}
	if (count)
		fprintf(stderr, "warning: %s: %d objects from "
			"this repository were not freed\n",
			repos->name, count);
	free(repos);
}


void repos_free(struct repos_t *repos)
{
	repos_free_dump(repos, NULL);
}


void *repos_create_object(struct repos_t *repos)
{
	void *obj, *next_obj;
	struct objtail_t *objtail, *next_objtail;
	
	/* No unallocated object available. Create a new object.
	 * Insert it into the unallocated list head. */
	if (!repos->unalloc_head) {
		obj = calloc(1, repos->objsize + sizeof(struct objtail_t));
		if (!obj)
			return NULL;
		objtail = obj + repos->objsize;
		objtail->id = repos->id;
		repos->unalloc_head = obj;
	}

	/* Remove the first unallocated object from the list */
	obj = repos->unalloc_head;
	objtail = obj + repos->objsize;
	assert(!objtail->prev);
	assert(!objtail->status);
	next_obj = objtail->next;
	next_objtail = next_obj + repos->objsize;
	if (next_obj)
		next_objtail->prev = NULL;
	repos->unalloc_head = next_obj;

	/* Clear object and insert it into the allocated list head */
	memset(obj, 0, repos->objsize);
	next_obj = repos->alloc_head;
	next_objtail = next_obj + repos->objsize;
	if (next_obj)
		next_objtail->prev = obj;
	objtail->next = next_obj;
	objtail->status = 1;
	repos->alloc_head = obj;

	/* Return allocated object */
	return obj;
}


void repos_free_object(struct repos_t *repos, void *obj)
{
	struct objtail_t *objtail, *prev_objtail, *next_objtail;
	void *prev_obj, *next_obj;
	
	/* Integrity */
	if (!obj)
		return;
	if (!repos_allocated_object(repos, obj)) {
		fprintf(stderr, "panic: %s.repos_free_object: freed object not valid\n",
			repos->name);
		abort();
	}

	/* Check that object was allocated */
	objtail = obj + repos->objsize;
	if (!objtail->status) {
		fprintf(stderr, "panic: %s.repos_free_object: object was not allocated\n",
			repos->name);
		abort();
	}

	/* Remove object from allocated list */
	prev_obj = objtail->prev;
	prev_objtail = prev_obj + repos->objsize;
	next_obj = objtail->next;
	next_objtail = next_obj + repos->objsize;
	if (prev_obj)
		prev_objtail->next = next_obj;
	if (next_obj)
		next_objtail->prev = prev_obj;
	if (repos->alloc_head == obj)
		repos->alloc_head = next_obj;
	
	/* Insert object into unallocated list head */
	next_obj = repos->unalloc_head;
	next_objtail = obj + repos->objsize;
	if (next_obj)
		next_objtail->prev = obj;
	objtail->prev = NULL;
	objtail->next = next_obj;
	objtail->status = 0;
	repos->unalloc_head = obj;
}


int repos_allocated_object(struct repos_t *repos, void *obj)
{
	struct objtail_t *objtail;
	if (!obj)
		return 0;
	objtail = obj + repos->objsize;
	return objtail->id == repos->id && objtail->status;
}

