/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include "list.h"



/*
 * Class 'List'
 */

/* One element of the list */
struct list_elem_t
{
	struct list_elem_t *prev;
	struct list_elem_t *next;
	Object *object;
};


static int ListCompareObject(Object *o1, Object *o2)
{
	if (!o1->Compare)
	{
		ObjectDump(o1, stderr);
		panic("%s: virtual function 'Compare' not overridden",
				__FUNCTION__);
	}
	return o1->Compare(o1, o2);
}


static void ListSortRange(struct list_elem_t **array, int lo, int hi)
{
	struct list_elem_t *ptr;
	struct list_elem_t *tmp;

	int i = lo;
	int j = hi;
	
	ptr = array[(lo + hi) / 2];
	do
	{
		while (ListCompareObject(array[i]->object, ptr->object) < 0)
			i++;
		while (ListCompareObject(array[j]->object, ptr->object) > 0)
			j--;
		if (i <= j)
		{
			tmp = array[i];
			array[i] = array[j];
			array[j] = tmp;
			i++;
			j--;
		}
	} while (i <= j);

	if (lo < j)
		ListSortRange(array, lo, j);
	if (i < hi)
		ListSortRange(array, i, hi);
}


void ListCreate(List *self)
{
	/* Virtual functions */
	asObject(self)->Dump = ListDump;
}


void ListDestroy(List *self)
{
	ListClear(self);
}


void ListDump(Object *self, FILE *f)
{
	ListDumpWithDelim(asList(self), f, "", "\n", "\n");
}


void ListDumpWithDelim(List *self, FILE *f, const char *first,
		const char *middle, const char *last)
{
	struct list_elem_t *elem;

	fprintf(f, "%s", first);
	for (elem = self->head; elem; elem = elem->next)
	{
		if (elem->object)
			elem->object->Dump(elem->object, f);
		else
			fprintf(f, "(nil)");
		if (elem->next)
			fprintf(f, "%s", middle);
	}
	fprintf(f, "%s", last);
}


void ListClear(List *self)
{
	struct list_elem_t *elem;
	struct list_elem_t *next;
	
	/* Free all elements */
	elem = self->head;
	while (elem)
	{
		next = elem->next;
		free(elem);
		elem = next;
	}
	
	/* Update self state */
	self->error = ListErrOK;
	self->current_index = 0;
	self->count = 0;
	self->tail = NULL;
	self->head = NULL;
	self->current = NULL;
	self->version++;
}


void ListDeleteObjects(List *self)
{
	Object *object;

	/* Remove and free all objects */
	ListHead(self);
	while (self->count)
	{
		object = ListRemove(self);
		if (object)
			delete(object);
	}
	
	/* Always succeed */
	self->error = ListErrOK;
}


Object *ListGet(List *self)
{
	/* Check boundaries */
	if (self->current_index == self->count)
	{
		self->error = ListErrEnd;
		return NULL;
	}

	/* Success */
	self->error = ListErrOK;
	return self->current->object;
}


Object *ListNext(List *self)
{
	/* Check bounds */
	if (self->current_index == self->count)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Go to next element */
	self->current_index++;
	self->current = self->current->next;
	
	/* End of list reached */
	if (self->current_index == self->count)
	{
		self->error = ListErrEnd;
		return NULL;
	}

	/* Return current object */
	assert(self->current);
	self->error = ListErrOK;
	return self->current->object;
}


Object *ListPrev(List *self)
{
	/* Check bounds */
	if (!self->current_index)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Move to previous element */
	self->error = ListErrOK;
	self->current_index--;
	self->current = self->current ? self->current->prev : self->tail;

	/* Return current object */
	assert(self->current);
	return self->current->object;
}


Object *ListNextCircular(List *self)
{
	/* List empty */
	if (!self->count)
	{
		self->error = ListErrEmpty;
		return NULL;
	}

	/* Go to next element */
	if (self->current_index >= self->count - 1)
	{
		self->current_index = 0;
		self->current = self->head;
	}
	else
	{
		self->current_index++;
		self->current = self->current->next;
	}

	/* Return object */
	self->error = ListErrOK;
	assert(self->current);
	return self->current->object;
}


Object *ListPrevCircular(List *self)
{
	/* Empty list */
	if (!self->count)
	{
		self->error = ListErrEmpty;
		return NULL;
	}

	/* Go to previous element */
	if (!self->current_index)
	{
		self->current_index = self->count - 1;
		self->current = self->tail;
	}
	else
	{
		self->current_index--;
		self->current = self->current ? self->current->prev :
			self->tail;
	}
	
	/* Return object */
	self->error = ListErrOK;
	assert(self->current);
	return self->current->object;
}


Object *ListHead(List *self)
{
	/* Empty list */
	if (!self->count)
	{
		self->error = ListErrEmpty;
		return NULL;
	}

	/* Go to head */
	self->error = ListErrOK;
	self->current_index = 0;
	self->current = self->head;
	
	/* Return current object */
	assert(self->current);
	return self->current->object;
}


Object *ListTail(List *self)
{
	/* Empty list */
	if (!self->count)
	{
		self->error = ListErrEmpty;
		return NULL;
	}

	/* Go to tail */
	self->current_index = self->count - 1;
	self->current = self->tail;
	self->error = ListErrOK;

	/* Return current object */
	assert(self->current);
	return self->current->object;
}


void ListOut(List *self)
{
	self->error = ListErrOK;
	self->current_index = self->count;
	self->current = NULL;
}


Object *ListGoto(List *self, int index)
{
	/* Check bounds */
	if (index < 0 || index > self->count)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Go forward */
	while (self->current_index < index)
	{
		self->current_index++;
		self->current = self->current->next;
	}

	/* Go backward */
	while (self->current_index > index)
	{
		self->current_index--;
		self->current = self->current ? self->current->prev :
			self->tail;
	}

	/* End of list */
	if (index == self->count)
	{
		self->error = ListErrEnd;
		return NULL;
	}

	/* Return current object */
	assert(self->current);
	self->error = ListErrOK;
	return self->current->object;
}


Object *ListFind(List *self, Object *object)
{
	/* Search object */
	self->current_index = 0;
	self->current = self->head;
	while (self->current && self->current->object != object)
	{
		self->current_index++;
		self->current = self->current->next;
	}

	/* Not found */
	if (!self->current)
	{
		self->error = ListErrNotFound;
		return NULL;
	}

	/* Found */
	self->error = ListErrOK;
	assert(self->current);
	return self->current->object;
}


int ListIsEnd(List *self)
{
	/* End of list */
	if (self->current_index == self->count)
	{
		self->error = ListErrEnd;
		return 1;
	}

	/* Not the end */
	self->error = ListErrOK;
	return 0;
}


Object *ListInsert(List *self, Object *object)
{
	struct list_elem_t *elem;
	
	/* Create a new element */
	elem = xcalloc(1, sizeof(struct list_elem_t));
	elem->object = object;
	
	/* Insert it */
	if (!self->count)
	{
		/* No element in the self */
		self->current = elem;
		self->head = elem;
		self->tail = elem;
	}
	else if (self->current == self->head)
	{
		/* Insert at the head */
		elem->next = self->head;
		self->head->prev = elem;
		self->head = elem;
	}
	else if (!self->current)
	{
		/* Insert at the tail */
		elem->prev = self->tail;
		self->tail->next = elem;
		self->tail = elem;
	}
	else
	{
		/* Insert in the middle */
		elem->prev = self->current->prev;
		elem->next = self->current;
		self->current->prev = elem;
		elem->prev->next = elem;
		
	}
	
	/* Update state */
	self->error = ListErrOK;
	self->count++;
	self->current = elem;
	self->version++;
	return object;
}


Object *ListAdd(List *self, Object *object)
{
	ListOut(self);
	ListInsert(self, object);
	ListOut(self);
	return object;
}


Object *ListRemove(List *self)
{
	struct list_elem_t *elem;
	Object *object;
	
	/* Check bounds */
	if (self->current_index == self->count)
	{
		self->error = ListErrBounds;
		return NULL;
	}
	
	/* Remove current element */
	elem = self->current;
	assert(elem);
	object = elem->object;
	if (self->count == 1)
	{
		self->tail = NULL;
		self->head = NULL;
	}
	else if (elem == self->head)
	{
		elem->next->prev = NULL;
		self->head = elem->next;
	}
	else if (elem == self->tail)
	{
		elem->prev->next = NULL;
		self->tail = elem->prev;
	}
	else
	{
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;
	}
	
	/* Update list state */
	assert(self->count > 0);
	self->error = ListErrOK;
	self->count--;
	self->current = elem->next;
	self->version++;
	free(elem);
	return object;
}


void ListSort(List *self)
{
	struct list_elem_t **array;
	int i;
	
	/* No need to sort an empty list */
	self->error = ListErrOK;
	if (!self->count)
		return;
	
	/* Convert linked list into array */
	array = xcalloc(self->count, sizeof(struct list_elem_t *));
	self->current = self->head;
	for (i = 0; i < self->count; i++)
	{
		array[i] = self->current;
		self->current = self->current->next;
	}
	
	/* Sort array */
	ListSortRange(array, 0, self->count - 1);
	
	/* Rebuild linked list */
	self->head = array[0];
	self->tail = array[self->count - 1];
	for (i = 0; i < self->count; i++)
	{
		array[i]->prev = i > 0 ? array[i - 1] : NULL;
		array[i]->next = i < self->count - 1 ? array[i + 1] : NULL;
	}
	free(array);
	
	/* Set the first element as current element */
	self->current_index = 0;
	self->current = self->head;
	self->version++;
}




/*
 * Class 'ListIterator'
 */

static void ListIteratorCheckVersion(ListIterator *self)
{
	/* Check */
	if (self->version == self->list->version)
		return;
	
	/* Version mismatch */
	panic("%s: version mismatch for linked list iterator.\n"
		"\tAn iterator has been created to traverse a linked list, but the\n"
		"\tcontent of the list has been modified during the iterator's\n"
		"\tlifetime. This behavior is not allowed, since it can produce\n"
		"\tinconsistencies.\n", __FUNCTION__);
}


void ListIteratorCreate(ListIterator *self, List *list)
{
	self->list = list;
	self->version = list->version;
	self->elem = list->head;
}


void ListIteratorDestroy(ListIterator *self)
{
}


Object *ListIteratorHead(ListIterator *self)
{
	List *list = self->list;

	/* Check iterator version */
	ListIteratorCheckVersion(self);

	/* Empty list */
	if (!list->count)
	{
		self->error = ListErrEmpty;
		return NULL;
	}

	/* Go to head */
	self->error = ListErrOK;
	self->index = 0;
	self->elem = list->head;
	
	/* Return current object */
	assert(self->elem);
	return self->elem->object;
}


Object *ListIteratorTail(ListIterator *self)
{
	List *list = self->list;

	/* Check iterator version */
	ListIteratorCheckVersion(self);

	/* Empty list */
	if (!list->count)
	{
		self->error = ListErrEmpty;
		return NULL;
	}

	/* Go to tail */
	self->index = list->count - 1;
	self->elem = list->tail;
	self->error = ListErrOK;

	/* Return current object */
	assert(self->elem);
	return self->elem->object;
}


void ListIteratorOut(ListIterator *self)
{
	List *list = self->list;

	/* Check iterator version */
	ListIteratorCheckVersion(self);
	
	/* Move past tail */
	self->error = ListErrOK;
	self->index = list->count;
	self->elem = NULL;
}


Object *ListIteratorNext(ListIterator *self)
{
	List *list = self->list;

	/* Check iterator version */
	ListIteratorCheckVersion(self);

	/* Check bounds */
	if (self->index == list->count)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Go to next element */
	self->index++;
	self->elem = self->elem->next;
	
	/* End of list reached */
	if (self->index == list->count)
	{
		self->error = ListErrEnd;
		return NULL;
	}

	/* Return current object */
	assert(self->elem);
	self->error = ListErrOK;
	return self->elem->object;
}


Object *ListIteratorPrev(ListIterator *self)
{
	List *list = self->list;

	/* Check iterator version */
	ListIteratorCheckVersion(self);

	/* Check bounds */
	if (!self->index)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Move to previous element */
	self->error = ListErrOK;
	self->index--;
	self->elem = self->elem ? self->elem->prev : list->tail;

	/* Return current object */
	assert(self->elem);
	return self->elem->object;
}


Object *ListIteratorGoto(ListIterator *self, int index)
{
	List *list = self->list;

	/* Check iterator version */
	ListIteratorCheckVersion(self);

	/* Check bounds */
	if (index < 0 || index > list->count)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Go forward */
	while (self->index < index)
	{
		self->index++;
		self->elem = self->elem->next;
	}

	/* Go backward */
	while (self->index > index)
	{
		self->index--;
		self->elem = self->elem ? self->elem->prev : list->tail;
	}

	/* End of list */
	if (index == list->count)
	{
		self->error = ListErrEnd;
		return NULL;
	}

	/* Return current object */
	assert(self->elem);
	self->error = ListErrOK;
	return self->elem->object;
}


Object *ListIteratorFind(ListIterator *self, Object *object)
{
	List *list = self->list;

	/* Search object */
	self->index = 0;
	self->elem = list->head;
	while (self->elem && self->elem->object != object)
	{
		self->index++;
		self->elem = self->elem->next;
	}

	/* Not found */
	if (!self->elem)
	{
		self->error = ListErrNotFound;
		return NULL;
	}

	/* Found */
	self->error = ListErrOK;
	assert(self->elem);
	return self->elem->object;
}


int ListIteratorIsEnd(ListIterator *self)
{
	List *list = self->list;

	/* End of list */
	if (self->index == list->count)
	{
		self->error = ListErrEnd;
		return 1;
	}

	/* Not the end */
	self->error = ListErrOK;
	return 0;
}


Object *ListIteratorGet(ListIterator *self)
{
	List *list = self->list;

	/* Check iterator version */
	ListIteratorCheckVersion(self);

	/* Check boundaries */
	if (self->index == list->count)
	{
		self->error = ListErrEnd;
		return NULL;
	}

	/* Success */
	self->error = ListErrOK;
	return self->elem->object;
}

