/*
 *  Libstruct
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


#include <lib/mhandle/mhandle.h>

#include "list.h"


#define INLIST(X) (((X) + list->size) % list->size)
#define ELEM(X) list->elem[((X) + list->head) % list->size]


/*
 * Private Functions
 */


/* Duplicate list size */
static void list_grow(struct list_t *list)
{
	void **nelem;
	int nsize, i, index;

	/* Create new array */
	nsize = list->size * 2;
	nelem = xcalloc(nsize, sizeof(void *));

	/* Copy contents to new array */
	for (i = list->head, index = 0;
		index < list->count;
		i = (i + 1) % list->size, index++)
		nelem[index] = list->elem[i];

	/* Update fields */
	free(list->elem);
	list->elem = nelem;
	list->size = nsize;
	list->head = 0;
	list->tail = list->count;
}


static void sort(struct list_t *list, int lo, int hi,
	int (*comp)(const void *, const void *))
{
	void *ptr, *tmp;
	int i = lo, j = hi;
	
	ptr = ELEM(hi);
	do
	{
		while (comp(ELEM(i), ptr) < 0)
			i++;
		while (comp(ELEM(j), ptr) > 0)
			j--;
		if (i <= j)
		{
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




/*
 * Public Functions
 */

struct list_t *list_create_with_size(int size)
{
	struct list_t *list;

	/* Create vector of elements */
	list = xcalloc(1, sizeof(struct list_t));
	list->size = size < 4 ? 4 : size;
	list->elem = xcalloc(list->size, sizeof(void *));
	
	/* Return list */
	return list;
}


/* If no initial size specified, create a list with 8 elements. */
struct list_t *list_create(void)
{
	return list_create_with_size(8);
}


void list_free(struct list_t *list)
{
	free(list->elem);
	free(list);
}


int list_count(struct list_t *list)
{
	return list->count;
}


void list_add(struct list_t *list, void *elem)
{
	/* Grow list if necessary */
	if (list->count == list->size)
		list_grow(list);

	/* Add element */
	list->elem[list->tail] = elem;
	list->tail = (list->tail + 1) % list->size;
	list->count++;
	list->error_code = LIST_ERR_OK;
}


void *list_get(struct list_t *list, int index)
{
	/* Check bounds */
	if (index < 0 || index >= list->count)
	{
		list->error_code = LIST_ERR_BOUNDS;
		return NULL;
	}

	/* Return element */
	index = (index + list->head) % list->size;
	list->error_code = LIST_ERR_OK;
	return list->elem[index];
}


void list_set(struct list_t *list, int index, void *elem)
{
	/* check bounds */
	if (index < 0 || index >= list->count)
	{
		list->error_code = LIST_ERR_BOUNDS;
		return;
	}

	/* Return element */
	index = (index + list->head) % list->size;
	list->elem[index] = elem;
	list->error_code = LIST_ERR_OK;
}


void list_insert(struct list_t *list, int index, void *elem)
{
	int shiftcount;
	int pos;
	int i;

	/* Check bounds */
	if (index < 0 || index > list->count)
	{
		list->error_code = LIST_ERR_BOUNDS;
		return;
	}

	/* Grow list if necessary */
	if (list->count == list->size)
		list_grow(list);

	/* Choose whether to shift elements on the right increasing 'tail', or
	 * shift elements on the left decreasing 'head'. */
	if (index > list->count / 2)
	{
		shiftcount = list->count - index;
		for (i = 0, pos = list->tail;
			 i < shiftcount;
			 i++, pos = INLIST(pos - 1))		 
			list->elem[pos] = list->elem[INLIST(pos - 1)];
		list->tail = (list->tail + 1) % list->size;
	}
	else
	{
		for (i = 0, pos = list->head;
			 i < index;
			 i++, pos = (pos + 1) % list->size)
			list->elem[INLIST(pos - 1)] = list->elem[pos];
		list->head = INLIST(list->head - 1);
	}

	list->elem[(list->head + index) % list->size] = elem;
	list->count++;
	list->error_code = LIST_ERR_OK;
}


int list_index_of(struct list_t *list, void *elem)
{
	int pos;
	int i;
	
	/* Search element */
	list->error_code = LIST_ERR_OK;
	for (i = 0, pos = list->head;
		i < list->count;
		i++, pos = (pos + 1) % list->size)
	{
		if (list->elem[pos] == elem)
			return i;
	}
	
	/* Element not found */
	list->error_code = LIST_ERR_NOT_FOUND;
	return -1;
}


void *list_remove_at(struct list_t *list, int index)
{
	int shiftcount;
	int pos;
	int i;
	void *elem;

	/* check bounds */
	if (index < 0 || index >= list->count)
	{
		list->error_code = LIST_ERR_BOUNDS;
		return NULL;
	}

	/* Get element before deleting it */
	elem = list->elem[(list->head + index) % list->size];

	/* Delete */
	if (index > list->count / 2)
	{
		shiftcount = list->count - index - 1;
		for (i = 0, pos = (list->head + index) % list->size;
			 i < shiftcount;
			 i++, pos = (pos + 1) % list->size)
			list->elem[pos] = list->elem[(pos + 1) % list->size];
		list->elem[pos] = NULL;
		list->tail = INLIST(list->tail - 1);
	}
	else
	{
		for (i = 0, pos = (list->head + index) % list->size;
			 i < index;
			 i++, pos = INLIST(pos - 1))
			list->elem[pos] = list->elem[INLIST(pos - 1)];
		list->elem[list->head] = NULL;
		list->head = (list->head + 1) % list->size;
	}
	
	list->count--;
	list->error_code = LIST_ERR_OK;
	return elem;
}


void *list_remove(struct list_t *list, void *elem)
{
	int index;
	
	/* Get index of element */
	index = list_index_of(list, elem);
	if (list->error_code)
		return NULL;
	
	/* Delete element at found position */
	return list_remove_at(list, index);
}


void list_clear(struct list_t *list)
{
	list->count = 0;
	list->head = 0;
	list->tail = 0;
	list->error_code = LIST_ERR_OK;
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
	if (!list->count)
	{
		list->error_code = LIST_ERR_EMPTY;
		return NULL;
	}
	return list_remove_at(list, list->count - 1);
}


void *list_top(struct list_t *list)
{
	if (!list->count)
	{
		list->error_code = LIST_ERR_EMPTY;
		return NULL;
	}
	return list_get(list, list->count - 1);
}


void *list_bottom(struct list_t *list)
{
	if (!list->count)
	{
		list->error_code = LIST_ERR_EMPTY;
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
	if (!list->count)
	{
		list->error_code = LIST_ERR_EMPTY;
		return NULL;
	}
	return list_remove_at(list, 0);
}


void *list_head(struct list_t *list)
{
	if (!list->count)
	{
		list->error_code = LIST_ERR_EMPTY;
		return NULL;
	}
	return list_get(list, 0);
}


void *list_tail(struct list_t *list)
{
	if (!list->count)
	{
		list->error_code = LIST_ERR_EMPTY;
		return NULL;
	}
	return list_get(list, list->count - 1);
}

