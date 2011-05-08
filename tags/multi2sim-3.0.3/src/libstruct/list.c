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
#include <mhandle.h>
#include "list.h"

struct list_t {
	int size, count;
	int error;
	int head, tail;
	void **elem;
};



#define INLIST(X) (((X) + list->size) % list->size)
#define ELEM(X) list->elem[((X) + list->head) % list->size]


/* Private Methods */

/* duplicate list size */
static int grow(struct list_t *list)
{
	void **nelem;
	int nsize, i, index;

	/* create new array */
	nsize = list->size * 2;
	nelem = calloc(nsize, sizeof(void *));
	if (!nelem)
		return 0;

	/* copy contents to new array */
	for (i = list->head, index = 0;
		index < list->count;
		i = (i + 1) % list->size, index++)
		nelem[index] = list->elem[i];

	/* update fields */
	free(list->elem);
	list->elem = nelem;
	list->size = nsize;
	list->head = 0;
	list->tail = list->count;
	return 1;
}


static void sort(struct list_t *list, int lo, int hi, int (*comp)(const void *, const void *))
{
	void *ptr, *tmp;
	int i = lo, j = hi;
	
	ptr = ELEM(hi);
	do {
		while (comp(ELEM(i), ptr) < 0)
			i++;
		while (comp(ELEM(j), ptr) > 0)
			j--;
		if (i <= j) {
			tmp = ELEM(i);
			ELEM(i) = ELEM(j);
			ELEM(j) = tmp;
			i++, j--;
		}
	} while (i <= j);
	if (lo < j)
		sort(list, lo, j, comp);
	if (i < hi)
		sort(list, i, hi, comp);
}




/* Public Methods */

/* creation */
struct list_t *list_create(int size)
{
	struct list_t *list;
	list = calloc(1, sizeof(struct list_t));
	if (!list)
		return NULL;
	list->size = size < 10 ? 10 : size;
	list->elem = calloc(list->size, sizeof(void *));
	if (!list->elem) {
		free(list);
		return NULL;
	}
	return list;
}


/* destruction */
void list_free(struct list_t *list)
{
	free(list->elem);
	free(list);
}


/* return error of last operation */
int list_error(struct list_t *list)
{
	return list->error;
}


char *list_error_msg(struct list_t *list)
{
	switch (list->error) {
	case LIST_ENOMEM: return "out of memory";
	case LIST_EBOUNDS: return "array index out of bounds";
	case LIST_EELEM: return "element not found";
	case LIST_EEMPTY: return "list is empty";
	}
	return "";
}


int list_count(struct list_t *list)
{
	return list->count;
}


/* add element at the end of the list */
void list_add(struct list_t *list, void *elem)
{
	/* grow list if necessary */
	if (list->count == list->size && !grow(list)) {
		list->error = LIST_ENOMEM;
		return;
	}

	/* add element */
	list->elem[list->tail] = elem;
	list->tail = (list->tail + 1) % list->size;
	list->count++;
	list->error = 0;
}


/* get element from list */
void *list_get(struct list_t *list, int index)
{
	/* check bounds */
	if (index < 0 || index >= list->count) {
		list->error = LIST_EBOUNDS;
		return NULL;
	}

	/* return element */
	index = (index + list->head) % list->size;
	list->error = 0;
	return list->elem[index];
}


/* set an element's value */
void list_set(struct list_t *list, int index, void *elem)
{
	/* check bounds */
	if (index < 0 || index >= list->count) {
		list->error = LIST_EBOUNDS;
		return;
	}

	/* return element */
	index = (index + list->head) % list->size;
	list->elem[index] = elem;
	list->error = 0;
}


/* insert an element in the list */
void list_insert(struct list_t *list, int index, void *elem)
{
	int shiftcount, pos, i;

	/* check bounds */
	if (index < 0 || index > list->count) {
		list->error = LIST_EBOUNDS;
		return;
	}

	/* grow list if necessary */
	if (list->count == list->size && !grow(list)) {
		list->error = LIST_ENOMEM;
		return;
	}

	/* Escoge si desplazar elementos a la derecha aumentando 'tail' o desplazar
	 * los de la izq decrementando 'head', igual q en 'arraylist_delete'.
	 */
	if (index > list->count / 2) {
		shiftcount = list->count - index;
		for (i = 0, pos = list->tail;
			 i < shiftcount;
			 i++, pos = INLIST(pos - 1))		 
			list->elem[pos] = list->elem[INLIST(pos - 1)];
		list->tail = (list->tail + 1) % list->size;
	} else {
		for (i = 0, pos = list->head;
			 i < index;
			 i++, pos = (pos + 1) % list->size)
			list->elem[INLIST(pos - 1)] = list->elem[pos];
		list->head = INLIST(list->head - 1);
	}

	list->elem[(list->head + index) % list->size] = elem;
	list->count++;
	list->error = 0;
}


/* return index of the first occurence of 'elem' in the list;
 * return -1 if 'elem' is not in the list */
int list_index_of(struct list_t *list, void *elem)
{
	int pos, i;
	
	/* search element */
	list->error = 0;
	for (i = 0, pos = list->head;
		i < list->count;
		i++, pos = (pos + 1) % list->size)
		if (list->elem[pos] == elem)
			return i;
	
	/* element not found */
	list->error = LIST_EELEM;
	return -1;
}


void *list_remove_at(struct list_t *list, int index)
{
	int shiftcount, pos, i;
	void *elem;

	/* check bounds */
	if (index < 0 || index >= list->count) {
		list->error = LIST_EBOUNDS;
		return NULL;
	}

	/* get element before deleting it */
	elem = list->elem[(list->head + index) % list->size];

	/* delete */
	if (index > list->count / 2) {
		shiftcount = list->count - index - 1;
		for (i = 0, pos = (list->head + index) % list->size;
			 i < shiftcount;
			 i++, pos = (pos + 1) % list->size)
			list->elem[pos] = list->elem[(pos + 1) % list->size];
		list->elem[pos] = NULL;
		list->tail = INLIST(list->tail - 1);
	} else {
		for (i = 0, pos = (list->head + index) % list->size;
			 i < index;
			 i++, pos = INLIST(pos - 1))
			list->elem[pos] = list->elem[INLIST(pos - 1)];
		list->elem[list->head] = NULL;
		list->head = (list->head + 1) % list->size;
	}
	
	list->count--;
	list->error = 0;
	return elem;
}


void *list_remove(struct list_t *list, void *elem)
{
	int index;
	
	index = list_index_of(list, elem);
	if (list->error)
		return NULL;
	return list_remove_at(list, index);
}


void list_clear(struct list_t *list)
{
	list->count = 0;
	list->head = 0;
	list->tail = 0;
	list->error = 0;
}


void list_sort(struct list_t *list, int (*comp)(const void *, const void *))
{
	if (list->count)
		sort(list, 0, list->count - 1, comp);
}


void list_push(struct list_t *list, void *elem)
{
	list_add(list, elem);
}


void *list_pop(struct list_t *list)
{
	if (!list->count) {
		list->error = LIST_EEMPTY;
		return NULL;
	}
	return list_remove_at(list, list->count - 1);
}


void *list_top(struct list_t *list)
{
	if (!list->count) {
		list->error = LIST_EEMPTY;
		return NULL;
	}
	return list_get(list, list->count - 1);
}


void *list_bottom(struct list_t *list)
{
	if (!list->count) {
		list->error = LIST_EEMPTY;
		return NULL;
	}
	return list_get(list, 0);
}


void list_enqueue(struct list_t *list, void *elem)
{
	list_add(list, elem);
}


void *list_dequeue(struct list_t *list)
{
	if (!list->count) {
		list->error = LIST_EEMPTY;
		return NULL;
	}
	return list_remove_at(list, 0);
}


void *list_head(struct list_t *list)
{
	if (!list->count) {
		list->error = LIST_EEMPTY;
		return NULL;
	}
	return list_get(list, 0);
}


void *list_tail(struct list_t *list)
{
	if (!list->count) {
		list->error = LIST_EEMPTY;
		return NULL;
	}
	return list_get(list, list->count - 1);
}
