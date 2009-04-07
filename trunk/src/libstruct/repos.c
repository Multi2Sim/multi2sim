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
#include "repos.h"

struct tail_t {
	int id;
	void *next;
};


static int repos_current_id = 0x1cec;

struct repos_t {
	char *name;
	int id;
	int objcount;
	int objsize;
	void *head;
};


struct repos_t *repos_create(int objsize, char *name)
{
	struct repos_t *repos;
	if (objsize <= 0)
		return NULL;
	repos = calloc(1, sizeof(struct repos_t));
	if (!repos)
		return NULL;
	repos->id = repos_current_id++;
	repos->name = name;
	repos->objsize = objsize;
	return repos;
}


void repos_free(struct repos_t *repos)
{
	void *next;
	struct tail_t *tail;
	
	while (repos->head) {
		tail = repos->head + repos->objsize;
		next = tail->next;
		free(repos->head);
		repos->head = next;
		repos->objcount--;
	}
	if (repos->objcount) {
		fprintf(stderr, "warning: %s.repos_free: %d objects from "
			"this repository were not freed\n",
			repos->name, repos->objcount);
	}
	free(repos);
}


void *repos_create_object(struct repos_t *repos)
{
	struct repos_obj_t *obj;
	struct tail_t *tail;
	
	if (!repos->head) {
		repos->head = calloc(1, repos->objsize + sizeof(struct tail_t));
		if (!repos->head)
			return NULL;
		repos->objcount++;
	}
	obj = repos->head;
	tail = repos->head + repos->objsize;
	repos->head = tail->next;
	tail->id = repos->id;
	return obj;
}


void repos_free_object(struct repos_t *repos, void *obj)
{
	struct tail_t *tail;
	
	if (!obj)
		return;
	if (!repos_allocated_object(repos, obj)) {
		fprintf(stderr, "panic: %s.repos_free_object: freed object not valid\n",
			repos->name);
		abort();
	}
	bzero(obj, repos->objsize + sizeof(struct tail_t));
	tail = obj + repos->objsize;
	tail->next = repos->head;
	repos->head = obj;
}


int repos_allocated_object(struct repos_t *repos, void *obj)
{
	struct tail_t *tail;
	if (!obj)
		return 0;
	tail = obj + repos->objsize;
	return tail->id == repos->id;
}
