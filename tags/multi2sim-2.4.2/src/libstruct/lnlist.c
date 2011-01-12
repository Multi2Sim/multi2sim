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
#include <assert.h>
#include <mhandle.h>
#include "lnlist.h"
#include "repos.h"


struct elem_t {
	struct elem_t *prev, *next;
	void *data;
};


struct lnlist_t {
	struct repos_t *elem_repos;
	struct elem_t *head, *tail, *current;
	int icurrent, count;
	int error;
};


/* Creation */
struct lnlist_t *lnlist_create()
{
	struct lnlist_t *lnlist;
	lnlist = calloc(1, sizeof(struct lnlist_t));
	if (!lnlist)
		return NULL;
	lnlist->elem_repos = repos_create(sizeof(struct elem_t), "lnlist_elem_repos");
	if (!lnlist->elem_repos) {
		free(lnlist);
		return NULL;
	}
	return lnlist;
}


/* Destruction */
void lnlist_free(struct lnlist_t *lnlist)
{
	lnlist_clear(lnlist);
	repos_free(lnlist->elem_repos);
	free(lnlist);
}


int lnlist_current(struct lnlist_t *lnlist)
{
	lnlist->error = 0;
	return lnlist->icurrent;
}


void *lnlist_get(struct lnlist_t *lnlist)
{
	if (lnlist->icurrent == lnlist->count) {
		lnlist->error = LNLIST_EBOUNDS;
		return NULL;
	}
	lnlist->error = 0;
	return lnlist->current->data;
}


void lnlist_next(struct lnlist_t *lnlist)
{
	if (lnlist->icurrent == lnlist->count) {
		lnlist->error = LNLIST_EBOUNDS;
		return;
	}
	lnlist->error = 0;
	lnlist->icurrent++;
	lnlist->current = lnlist->current->next;
}


void lnlist_prev(struct lnlist_t *lnlist)
{
	if (!lnlist->icurrent) {
		lnlist->error = LNLIST_EBOUNDS;
		return;
	}
	lnlist->error = 0;
	lnlist->icurrent--;
	lnlist->current = lnlist->current ? lnlist->current->prev :
		lnlist->tail;
}


void lnlist_head(struct lnlist_t *lnlist)
{
	if (!lnlist->count) {
		lnlist->error = LNLIST_EBOUNDS;
		return;
	}
	lnlist->error = 0;
	lnlist->icurrent = 0;
	lnlist->current = lnlist->head;
}


void lnlist_tail(struct lnlist_t *lnlist)
{
	if (!lnlist->count) {
		lnlist->error = LNLIST_EBOUNDS;
		return;
	}
	lnlist->icurrent = lnlist->count - 1;
	lnlist->current = lnlist->tail;
	lnlist->error = 0;
}


void lnlist_out(struct lnlist_t *lnlist)
{
	lnlist->error = 0;
	lnlist->icurrent = lnlist->count;
	lnlist->current = NULL;
}


void lnlist_goto(struct lnlist_t *lnlist, int index)
{
	if (index < 0 || index > lnlist->count) {
		lnlist->error = LNLIST_EBOUNDS;
		return;
	}
	lnlist->error = 0;
	while (lnlist->icurrent < index) {
		lnlist->icurrent++;
		lnlist->current = lnlist->current->next;
	}
	while (lnlist->icurrent > index) {
		lnlist->icurrent--;
		lnlist->current = lnlist->current ? lnlist->current->prev :
			lnlist->tail;
	}
}


void lnlist_find(struct lnlist_t *lnlist, void *data)
{
	lnlist->error = 0;
	lnlist->icurrent = 0;
	lnlist->current = lnlist->head;
	while (lnlist->current && lnlist->current->data != data) {
		lnlist->icurrent++;
		lnlist->current = lnlist->current->next;
	}
	if (!lnlist->current)
		lnlist->error = LNLIST_EELEM;
}


int lnlist_count(struct lnlist_t *lnlist)
{
	lnlist->error = 0;
	return lnlist->count;
}


int lnlist_eol(struct lnlist_t *lnlist)
{
	lnlist->error = 0;
	return lnlist->icurrent == lnlist->count;
}


void lnlist_insert(struct lnlist_t *lnlist, void *data)
{
	struct elem_t *elem;
	
	/* create new elem_t */
	elem = repos_create_object(lnlist->elem_repos);
	if (!elem) {
		lnlist->error = LNLIST_ENOMEM;
		return;
	}
	elem->data = data;
	
	/* insert it */
	if (!lnlist->count) {
	
		/* no element in the list */
		lnlist->current = lnlist->head = lnlist->tail = elem;
		
	} else if (lnlist->current == lnlist->head) {
	
		/* insert at the beginning */
		elem->next = lnlist->head;
		lnlist->head->prev = elem;
		lnlist->head = elem;
		
	} else if (!lnlist->current) {
		
		/* insert at the end */
		elem->prev = lnlist->tail;
		lnlist->tail->next = elem;
		lnlist->tail = elem;
		
	} else {
		
		/* insert in the middle */
		elem->prev = lnlist->current->prev;
		elem->next = lnlist->current;
		lnlist->current->prev = elem;
		elem->prev->next = elem;
		
	}
	
	/* update list status */
	lnlist->error = 0;
	lnlist->count++;
	lnlist->current = elem;
}


void lnlist_add(struct lnlist_t *lnlist, void *data)
{
	lnlist_out(lnlist);
	lnlist_insert(lnlist, data);
	if (!lnlist->error)
		lnlist_out(lnlist);
}


void lnlist_remove(struct lnlist_t *lnlist)
{
	struct elem_t *elem;
	
	/* check bounds */
	if (lnlist->icurrent == lnlist->count) {
		lnlist->error = LNLIST_EBOUNDS;
		return;
	}
	
	/* remove current element */
	elem = lnlist->current;
	if (lnlist->count == 1) {
		lnlist->tail = lnlist->head = NULL;
	} else if (elem == lnlist->head) {
		elem->next->prev = NULL;
		lnlist->head = elem->next;
	} else if (elem == lnlist->tail) {
		elem->prev->next = NULL;
		lnlist->tail = elem->prev;
	} else {
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;
	}
	
	/* update list status */
	lnlist->error = 0;
	lnlist->count--;
	lnlist->current = elem->next;
	repos_free_object(lnlist->elem_repos, elem);
}


void lnlist_clear(struct lnlist_t *lnlist)
{
	struct elem_t *elem, *next;
	
	elem = lnlist->head;
	while (elem) {
		next = elem->next;
		repos_free_object(lnlist->elem_repos, elem);
		elem = next;
	}
	
	lnlist->error = 0;
	lnlist->icurrent = 0;
	lnlist->count = 0;
	lnlist->tail = lnlist->head = lnlist->current = NULL;
}


static void sort(struct elem_t **array, int lo, int hi, int (*comp)(const void *, const void *))
{
	struct elem_t *ptr, *tmp;
	int i = lo, j = hi;
	
	ptr = array[(lo + hi) / 2];
	do {
		while (comp(array[i]->data, ptr->data) < 0)
			i++;
		while (comp(array[j]->data, ptr->data) > 0)
			j--;
		if (i <= j) {
			tmp = array[i];
			array[i] = array[j];
			array[j] = tmp;
			i++, j--;
		}
	} while (i <= j);
	if (lo < j)
		sort(array, lo, j, comp);
	if (i < hi)
		sort(array, i, hi, comp);
}


void lnlist_sort(struct lnlist_t *lnlist, int (*comp)(const void *, const void *))
{
	struct elem_t **array;
	int i;
	
	/* Empty lists are not sorted */
	lnlist->error = 0;
	if (!lnlist->count)
		return;
	
	/* Build an array with the pointers */
	array = calloc(lnlist->count, sizeof(struct elem_t *));
	lnlist->current = lnlist->head;
	for (i = 0; i < lnlist->count; i++) {
		array[i] = lnlist->current;
		lnlist->current = lnlist->current->next;
	}
	
	/* Sort array */
	sort(array, 0, lnlist->count - 1, comp);
	
	/* Rebuild linked list */
	lnlist->head = array[0];
	lnlist->tail = array[lnlist->count - 1];
	for (i = 0; i < lnlist->count; i++) {
		array[i]->prev = i > 0 ? array[i - 1] : NULL;
		array[i]->next = i < lnlist->count - 1 ? array[i + 1] : NULL;
	}
	free(array);
	
	/* Set the first element as current element */
	lnlist->icurrent = 0;
	lnlist->current = lnlist->head;
}


int lnlist_sorted(struct lnlist_t *lnlist,
	int (*comp)(const void *, const void *))
{
	struct elem_t *elem, *prev;
	int i= 0;
	lnlist->error = 0;
	if (!lnlist->head)
		return 1;
	prev = lnlist->head;
	elem = lnlist->head->next;
	while (elem) {
		i++;
		if (comp(prev->data, elem->data) > 0)
			return 0;
		prev = elem;
		elem = elem->next;
	}
	return 1;
}


int lnlist_error(struct lnlist_t *lnlist)
{
	return lnlist->error;
}


char *lnlist_error_msg(struct lnlist_t *lnlist)
{
	switch (lnlist->error) {
	case LNLIST_ENOMEM: return "out of memory";
	case LNLIST_EBOUNDS: return "array index out of bounds";
	}
	return "";
}
