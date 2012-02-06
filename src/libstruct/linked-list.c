/*
 *  Libstruct
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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
#include <linked-list.h>
#include <repos.h>


struct linked_list_elem_t
{
	struct linked_list_elem_t *prev, *next;
	void *data;
};


struct linked_list_t
{
	struct repos_t *elem_repos;
	struct linked_list_elem_t *head, *tail, *current;
	int icurrent, count;
	int error;
};


/* Creation */
struct linked_list_t *linked_list_create()
{
	struct linked_list_t *list;
	list = calloc(1, sizeof(struct linked_list_t));
	if (!list)
		return NULL;

	/* FIXME: get rid of repository */
	list->elem_repos = repos_create(sizeof(struct linked_list_elem_t), "linked_list_elem_repos");
	if (!list->elem_repos) {
		free(list);
		return NULL;
	}
	return list;
}


/* Destruction */
void linked_list_free(struct linked_list_t *list)
{
	linked_list_clear(list);
	repos_free(list->elem_repos);
	free(list);
}


int linked_list_current(struct linked_list_t *list)
{
	list->error = 0;
	return list->icurrent;
}


void *linked_list_get(struct linked_list_t *list)
{
	if (list->icurrent == list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return NULL;
	}
	list->error = 0;
	return list->current->data;
}


void linked_list_next(struct linked_list_t *list)
{
	if (list->icurrent == list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	list->error = 0;
	list->icurrent++;
	list->current = list->current->next;
}


void linked_list_prev(struct linked_list_t *list)
{
	if (!list->icurrent) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	list->error = 0;
	list->icurrent--;
	list->current = list->current ? list->current->prev :
		list->tail;
}


void linked_list_next_circular(struct linked_list_t *list)
{
	if (!list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	if (list->icurrent >= list->count - 1) {
		list->icurrent = 0;
		list->current = list->head;
	} else {
		list->icurrent++;
		list->current = list->current->next;
	}
	list->error = 0;
}


void linked_list_prev_circular(struct linked_list_t *list)
{
	if (!list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	if (!list->icurrent) {
		list->icurrent = list->count - 1;
		list->current = list->tail;
	} else {
		list->icurrent--;
		list->current = list->current ? list->current->prev :
			list->tail;
	}
	list->error = 0;
}


void linked_list_head(struct linked_list_t *list)
{
	if (!list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	list->error = 0;
	list->icurrent = 0;
	list->current = list->head;
}


void linked_list_tail(struct linked_list_t *list)
{
	if (!list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	list->icurrent = list->count - 1;
	list->current = list->tail;
	list->error = 0;
}


void linked_list_out(struct linked_list_t *list)
{
	list->error = 0;
	list->icurrent = list->count;
	list->current = NULL;
}


void linked_list_goto(struct linked_list_t *list, int index)
{
	if (index < 0 || index > list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	list->error = 0;
	while (list->icurrent < index) {
		list->icurrent++;
		list->current = list->current->next;
	}
	while (list->icurrent > index) {
		list->icurrent--;
		list->current = list->current ? list->current->prev :
			list->tail;
	}
}


void linked_list_find(struct linked_list_t *list, void *data)
{
	list->error = 0;
	list->icurrent = 0;
	list->current = list->head;
	while (list->current && list->current->data != data) {
		list->icurrent++;
		list->current = list->current->next;
	}
	if (!list->current)
		list->error = LINKED_LIST_ERR_NOT_FOUND;
}


int linked_list_count(struct linked_list_t *list)
{
	list->error = 0;
	return list->count;
}


int linked_list_is_end(struct linked_list_t *list)
{
	list->error = 0;
	return list->icurrent == list->count;
}


void linked_list_insert(struct linked_list_t *list, void *data)
{
	struct linked_list_elem_t *elem;
	
	/* create new linked_list_elem_t */
	elem = repos_create_object(list->elem_repos);
	if (!elem) {
		list->error = LINKED_LIST_ERR_MEM;
		return;
	}
	elem->data = data;
	
	/* insert it */
	if (!list->count) {
	
		/* no element in the list */
		list->current = list->head = list->tail = elem;
		
	} else if (list->current == list->head) {
	
		/* insert at the beginning */
		elem->next = list->head;
		list->head->prev = elem;
		list->head = elem;
		
	} else if (!list->current) {
		
		/* insert at the end */
		elem->prev = list->tail;
		list->tail->next = elem;
		list->tail = elem;
		
	} else {
		
		/* insert in the middle */
		elem->prev = list->current->prev;
		elem->next = list->current;
		list->current->prev = elem;
		elem->prev->next = elem;
		
	}
	
	/* update list status */
	list->error = 0;
	list->count++;
	list->current = elem;
}


void linked_list_add(struct linked_list_t *list, void *data)
{
	linked_list_out(list);
	linked_list_insert(list, data);
	if (!list->error)
		linked_list_out(list);
}


void linked_list_remove(struct linked_list_t *list)
{
	struct linked_list_elem_t *elem;
	
	/* check bounds */
	if (list->icurrent == list->count) {
		list->error = LINKED_LIST_ERR_BOUNDS;
		return;
	}
	
	/* remove current element */
	elem = list->current;
	if (list->count == 1) {
		list->tail = list->head = NULL;
	} else if (elem == list->head) {
		elem->next->prev = NULL;
		list->head = elem->next;
	} else if (elem == list->tail) {
		elem->prev->next = NULL;
		list->tail = elem->prev;
	} else {
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;
	}
	
	/* update list status */
	list->error = 0;
	list->count--;
	list->current = elem->next;
	repos_free_object(list->elem_repos, elem);
}


void linked_list_clear(struct linked_list_t *list)
{
	struct linked_list_elem_t *elem, *next;
	
	elem = list->head;
	while (elem) {
		next = elem->next;
		repos_free_object(list->elem_repos, elem);
		elem = next;
	}
	
	list->error = 0;
	list->icurrent = 0;
	list->count = 0;
	list->tail = list->head = list->current = NULL;
}


static void sort(struct linked_list_elem_t **array, int lo, int hi, int (*comp)(const void *, const void *))
{
	struct linked_list_elem_t *ptr, *tmp;
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


void linked_list_sort(struct linked_list_t *list, int (*comp)(const void *, const void *))
{
	struct linked_list_elem_t **array;
	int i;
	
	/* Empty lists are not sorted */
	list->error = 0;
	if (!list->count)
		return;
	
	/* Build an array with the pointers */
	array = calloc(list->count, sizeof(struct linked_list_elem_t *));
	list->current = list->head;
	for (i = 0; i < list->count; i++) {
		array[i] = list->current;
		list->current = list->current->next;
	}
	
	/* Sort array */
	sort(array, 0, list->count - 1, comp);
	
	/* Rebuild linked list */
	list->head = array[0];
	list->tail = array[list->count - 1];
	for (i = 0; i < list->count; i++) {
		array[i]->prev = i > 0 ? array[i - 1] : NULL;
		array[i]->next = i < list->count - 1 ? array[i + 1] : NULL;
	}
	free(array);
	
	/* Set the first element as current element */
	list->icurrent = 0;
	list->current = list->head;
}


int linked_list_sorted(struct linked_list_t *list,
	int (*comp)(const void *, const void *))
{
	struct linked_list_elem_t *elem, *prev;
	int i= 0;
	list->error = 0;
	if (!list->head)
		return 1;
	prev = list->head;
	elem = list->head->next;
	while (elem) {
		i++;
		if (comp(prev->data, elem->data) > 0)
			return 0;
		prev = elem;
		elem = elem->next;
	}
	return 1;
}


int linked_list_error(struct linked_list_t *list)
{
	return list->error;
}


char *linked_list_error_msg(struct linked_list_t *list)
{
	switch (list->error) {

	case LINKED_LIST_ERR_MEM:
		return "out of memory";

	case LINKED_LIST_ERR_BOUNDS:
		return "array index out of bounds";

	default:
		return "";
	}
}
