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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "list.h"


/*
 * Class 'List'
 */

static void ListGrow(List *self)
{
	Object **new_array;
	
	int new_size;
	int index;
	int i;

	/* Create new array */
	new_size = self->size * 2;
	new_array = xcalloc(new_size, sizeof(Object *));

	/* Copy contents to new array */
	for (i = self->head, index = 0;
			index < self->count;
			i = (i + 1) % self->size, index++)
		new_array[index] = self->array[i];

	/* Update fields */
	free(self->array);
	self->array = new_array;
	self->size = new_size;
	self->head = 0;
	self->tail = self->count;
}


static int ListCompareObject(Object *o1, Object *o2)
{
	if (!o1->Compare)
	{
		ObjectDump(o1, stderr);
		panic("virtual function 'Compare' not overridden");
	}
	return o1->Compare(o1, o2);
}


#define ELEM(X) self->array[((X) + self->head) % self->size]
static void ListSortRange(List *self, int lo, int hi)
{
	Object *ptr;
	Object *tmp;

	int i = lo;
	int j = hi;
	
	ptr = ELEM(hi);
	do
	{
		while (ListCompareObject(ELEM(i), ptr) < 0)
			i++;
		while (ListCompareObject(ELEM(j), ptr) > 0)
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
		ListSortRange(self, lo, j);
	if (i < hi)
		ListSortRange(self, i, hi);
}


void ListCreate(List *self)
{
	/* Default initial size is 8 */
	ListCreateWithSize(self, 8);
}


void ListCreateWithSize(List *self, int size)
{
	/* Minimum size */
	if (size < 4)
		size = 4;
	
	/* Allocate elements */
	self->array = xcalloc(size, sizeof(Object *));
	self->size = size;

	/* Virtual functions */
	asObject(self)->Dump = ListDump;
}


void ListDestroy(List *self)
{
	free(self->array);
}


void ListDump(Object *self, FILE *f)
{
	ListDumpWithDelim(asList(self), f, "", "\n", "\n");
}


void ListDumpWithDelim(List *self, FILE *f, const char *first,
		const char *middle, const char *last)
{
	int index;
	int i;

	fprintf(f, "%s", first);
	for (i = self->head, index = 0;
			index < self->count;
			i = (i + 1) % self->size, index++)
	{
		self->array[i]->Dump(self->array[i], f);
		if (index < self->count - 1)
			fprintf(f, "%s", middle);
	}
	fprintf(f, "%s", last);
}


void ListClear(List *self)
{
	self->count = 0;
	self->head = 0;
	self->tail = 0;
	self->error = ListErrOK;
}


void ListDeleteObjects(List *self)
{
	int index;
	int i;

	for (i = self->head, index = 0;
			index < self->count;
			i = (i + 1) % self->size, index++)
		delete(self->array[i]);

	ListClear(self);
}


void ListAdd(List *self, Object *object)
{
	/* Grow list if necessary */
	if (self->count == self->size)
		ListGrow(self);

	/* Add new element */
	self->array[self->tail] = object;
	self->tail = (self->tail + 1) % self->size;
	self->count++;
	self->error = ListErrOK;
}


Object *ListGet(List *self, int index)
{
	/* Check bounds */
	if (index < 0 || index >= self->count)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Return element */
	index = (index + self->head) % self->size;
	self->error = ListErrOK;
	return self->array[index];
}


void ListSet(List *self, int index, Object *object)
{
	/* Check bounds */
	if (index < 0 || index >= self->count)
	{
		self->error = ListErrBounds;
		return;
	}

	/* Return element */
	index = (index + self->head) % self->size;
	self->array[index] = object;
	self->error = ListErrOK;
}


#define INLIST(X) (((X) + self->size) % self->size)
void ListInsert(List *self, int index, Object *object)
{
	int shiftcount;
	int pos;
	int i;

	/* Check bounds */
	if (index < 0 || index > self->count)
	{
		self->error = ListErrBounds;
		return;
	}

	/* Grow self if necessary */
	if (self->count == self->size)
		ListGrow(self);

	/* Choose whether to shift elements on the right increasing 'tail', or
	 * shift elements on the left decreasing 'head'. */
	if (index > self->count / 2)
	{
		shiftcount = self->count - index;
		for (i = 0, pos = self->tail;
				 i < shiftcount;
				 i++, pos = INLIST(pos - 1))		 
			self->array[pos] = self->array[INLIST(pos - 1)];
		self->tail = (self->tail + 1) % self->size;
	}
	else
	{
		for (i = 0, pos = self->head;
				 i < index;
				 i++, pos = (pos + 1) % self->size)
			self->array[INLIST(pos - 1)] = self->array[pos];
		self->head = INLIST(self->head - 1);
	}

	self->array[(self->head + index) % self->size] = object;
	self->count++;
	self->error = ListErrOK;
}


int ListFind(List *self, Object *object)
{
	int pos;
	int i;
	
	/* Search element */
	self->error = ListErrOK;
	for (i = 0, pos = self->head;
			i < self->count;
			i++, pos = (pos + 1) % self->size)
		if (self->array[pos] == object)
			return i;
	
	/* Element not found */
	self->error = ListErrNotFound;
	return -1;
}


Object *ListRemove(List *self, int index)
{
	Object *object;

	int shiftcount;
	int pos;
	int i;

	/* check bounds */
	if (index < 0 || index >= self->count)
	{
		self->error = ListErrBounds;
		return NULL;
	}

	/* Get element before deleting it */
	object = self->array[(self->head + index) % self->size];

	/* Delete */
	if (index > self->count / 2)
	{
		shiftcount = self->count - index - 1;
		for (i = 0, pos = (self->head + index) % self->size;
			 i < shiftcount;
			 i++, pos = (pos + 1) % self->size)
			self->array[pos] = self->array[(pos + 1) % self->size];
		self->array[pos] = NULL;
		self->tail = INLIST(self->tail - 1);
	}
	else
	{
		for (i = 0, pos = (self->head + index) % self->size;
			 i < index;
			 i++, pos = INLIST(pos - 1))
			self->array[pos] = self->array[INLIST(pos - 1)];
		self->array[self->head] = NULL;
		self->head = (self->head + 1) % self->size;
	}
	
	self->count--;
	self->error = ListErrOK;
	return object;
}


void ListSort(List *self)
{
	if (self->count)
		ListSortRange(self, 0, self->count - 1);
}

