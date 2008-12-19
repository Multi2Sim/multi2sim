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
#include "heap.h"


#define PARENT(X)	(((X) - 1) / 2)
#define LEFT(X)		(((X) * 2) + 1)
#define RIGHT(X)	(((X) * 2) + 2)

struct heap_elem_t {
	long long time, value;
	void *data;
};

struct heap_t {
	int size, count, current;
	int error;
	long long time;
	enum heap_time_policy_enum time_policy;
	struct heap_elem_t *elem;
};




/* Private Methods */

/* compare two heap elements */
static int heap_less_than(struct heap_t *heap, int x, int y)
{
	/* compare them by value first */
	if (heap->elem[x].value != heap->elem[y].value)
		return heap->elem[x].value < heap->elem[y].value;
	
	/* compare them by fifo time */
	if (heap->time_policy == heap_time_policy_fifo)
		return heap->elem[x].time < heap->elem[y].time;
	
	/* compare them by lifo time */
	return heap->elem[x].time > heap->elem[y].time;
}


/* grow heap */
static int heap_grow(struct heap_t *heap)
{
	struct heap_elem_t *nelem;
	int nsize = heap->size * 2;

	nelem = realloc(heap->elem, nsize * sizeof(struct heap_elem_t));
	if (!nelem)
		return 0;
	heap->elem = nelem;
	heap->size  = nsize;
	return 1;
}


/* heapify an element */
static void heapify(struct heap_t *heap, int i)
{
	int l, r, k;
	struct heap_elem_t tmp;
	
	for (;;) {
		l = LEFT(i);
		r = RIGHT(i);
		k = i;
		if (l < heap->count && heap_less_than(heap, l, k))
			k = l;
		if (r < heap->count && heap_less_than(heap, r, k))
			k = r;
		if (k == i)
			break;
		
		tmp = heap->elem[k];
		heap->elem[k] = heap->elem[i];
		heap->elem[i] = tmp;
		i = k;
	}
}




/* Public Methods */

/* creation */
struct heap_t *heap_create(int size)
{
	struct heap_t *heap;
	heap = calloc(1, sizeof(struct heap_t));
	heap->size = size < 10 ? 10 : size;
	heap->elem = calloc(heap->size, sizeof(struct heap_elem_t));
	if (!heap->elem) {
		free(heap);
		return NULL;
	}
	return heap;
}


/* destruction */
void heap_free(struct heap_t *heap)
{
	free(heap->elem);
	free(heap);
}


/* error messages */
int heap_error(struct heap_t *heap)
{
	return heap->error;
}


char *heap_error_msg(struct heap_t *heap)
{
	switch (heap->error) {
	case HEAP_ENOMEM: return "out of memory";
	case HEAP_EEMPTY: return "heap is empty";
	}
	return "";
}


int heap_count(struct heap_t *heap)
{
	return heap->count;
}


void heap_insert(struct heap_t *heap, long long value, void *data)
{
	int i;
	struct heap_elem_t tmp;
	
	/* grow heap */
	if (heap->count == heap->size && !heap_grow(heap)) {
		heap->error = HEAP_ENOMEM;
		return;
	}

	/* insert element */
	i = heap->count;
	heap->elem[i].value = value;
	heap->elem[i].data = data;
	heap->elem[i].time = heap->time++;
	while (i > 0 && heap_less_than(heap, i, PARENT(i))) {
		tmp = heap->elem[i];
		heap->elem[i] = heap->elem[PARENT(i)];
		heap->elem[PARENT(i)] = tmp;
		i = PARENT(i);
	}
	heap->count++;
	heap->error = 0;
}


long long heap_peek(struct heap_t *heap, void **data)
{
	long long value;
	
	/* heap empty */
	if (!heap->count) {
		heap->error = HEAP_EEMPTY;
		if (data)
			*data = NULL;
		return 0;
	}

	/* extract */
	value = heap->elem[0].value;
	if (data)
		*data = heap->elem[0].data;
	heap->error = 0;
	return value;
}


long long heap_extract(struct heap_t *heap, void **data)
{
	long long value;
	
	/* peek min */
	value = heap_peek(heap, data);
	if (heap->error)
		return 0;
	
	/* delete element from heap */
	heap->count--;
	heap->elem[0] = heap->elem[heap->count];
	heapify(heap, 0);
	return value;
}


void heap_time_policy(struct heap_t *heap, enum heap_time_policy_enum policy)
{
	heap->time_policy = policy;
	heap->error = 0;
}


long long heap_first(struct heap_t *heap, void **data)
{
	/* No element in the heap */
	if (!heap->count) {
		heap->error = HEAP_EELEM;
		if (data)
			*data = NULL;
		return 0;
	}
	
	/* Ok, return element */
	heap->current = 0;
	heap->error = 0;
	if (data)
		*data = heap->elem[0].data;
	return heap->elem[0].value;
}


long long heap_next(struct heap_t *heap, void **data)
{
	/* Current element out of bounds */
	if (heap->current >= heap->count - 1) {
		heap->error = HEAP_EELEM;
		if (data)
			*data = NULL;
		return 0;
	}
	
	/* Ok, return element */
	heap->current++;
	heap->error = 0;
	if (data)
		*data = heap->elem[heap->current].data;
	return heap->elem[heap->current].value;
}
