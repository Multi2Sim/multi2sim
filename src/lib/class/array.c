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

#include "array.h"


/*
 * Class 'Array'
 */

static void ArrayGrow(Array *self)
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


static int ArrayCompareObject(Object *o1, Object *o2)
{
	if (!o1->Compare)
	{
		ObjectDump(o1, stderr);
		panic("%s: virtual function 'Compare' not overridden",
				__FUNCTION__);
	}
	return o1->Compare(o1, o2);
}


#define ELEM(X) self->array[((X) + self->head) % self->size]
static void ArraySortRange(Array *self, int lo, int hi)
{
	Object *ptr;
	Object *tmp;

	int i = lo;
	int j = hi;
	
	ptr = ELEM(hi);
	do
	{
		while (ArrayCompareObject(ELEM(i), ptr) < 0)
			i++;
		while (ArrayCompareObject(ELEM(j), ptr) > 0)
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
		ArraySortRange(self, lo, j);
	if (i < hi)
		ArraySortRange(self, i, hi);
}


void ArrayCreate(Array *self)
{
	/* Default initial size is 8 */
	ArrayCreateWithSize(self, 8);
}


void ArrayCreateWithSize(Array *self, int size)
{
	/* Minimum size */
	if (size < 4)
		size = 4;
	
	/* Allocate elements */
	self->array = xcalloc(size, sizeof(Object *));
	self->size = size;

	/* Virtual functions */
	asObject(self)->Dump = ArrayDump;
}


void ArrayDestroy(Array *self)
{
	free(self->array);
}


void ArrayDump(Object *self, FILE *f)
{
	ArrayDumpWithDelim(asArray(self), f, "", "\n", "\n");
}


void ArrayDumpWithDelim(Array *self, FILE *f, const char *first,
		const char *middle, const char *last)
{
	int index;
	int i;

	fprintf(f, "%s", first);
	for (i = self->head, index = 0;
			index < self->count;
			i = (i + 1) % self->size, index++)
	{
		if (self->array[i])
			self->array[i]->Dump(self->array[i], f);
		else
			fprintf(f, "(nil)");
		if (index < self->count - 1)
			fprintf(f, "%s", middle);
	}
	fprintf(f, "%s", last);
}


void ArrayClear(Array *self)
{
	self->count = 0;
	self->head = 0;
	self->tail = 0;
	self->error = ArrayErrOK;
	self->index = 0;
}


void ArrayDeleteObjects(Array *self)
{
	int index;
	int i;

	for (i = self->head, index = 0;
			index < self->count;
			i = (i + 1) % self->size, index++)
		if (self->array[i])
			delete(self->array[i]);

	ArrayClear(self);
}


void ArrayAdd(Array *self, Object *object)
{
	/* Grow array if necessary */
	if (self->count == self->size)
		ArrayGrow(self);

	/* Add new element */
	self->array[self->tail] = object;
	self->tail = (self->tail + 1) % self->size;
	self->count++;
	self->error = ArrayErrOK;
	self->index = self->count - 1;
}


Object *ArrayGet(Array *self, int index)
{
	/* Check bounds */
	if (index < 0 || index >= self->count)
	{
		self->error = ArrayErrBounds;
		return NULL;
	}

	/* Return element */
	index = (index + self->head) % self->size;
	self->error = ArrayErrOK;
	return self->array[index];
}


void ArraySet(Array *self, int index, Object *object)
{
	/* Check bounds */
	if (index < 0 || index >= self->count)
	{
		self->error = ArrayErrBounds;
		return;
	}

	/* Return element */
	index = (index + self->head) % self->size;
	self->array[index] = object;
	self->error = ArrayErrOK;
	self->index = index;
}


#define INLIST(X) (((X) + self->size) % self->size)
void ArrayInsert(Array *self, int index, Object *object)
{
	int shiftcount;
	int pos;
	int i;

	/* Check bounds */
	if (index < 0 || index > self->count)
	{
		self->error = ArrayErrBounds;
		return;
	}

	/* Grow self if necessary */
	if (self->count == self->size)
		ArrayGrow(self);

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
	self->error = ArrayErrOK;
	self->index = index;
}


int ArrayFind(Array *self, Object *object)
{
	int pos;
	int i;
	
	/* Search element */
	self->error = ArrayErrOK;
	for (i = 0, pos = self->head;
			i < self->count;
			i++, pos = (pos + 1) % self->size)
		if (self->array[pos] == object)
			return i;
	
	/* Element not found */
	self->error = ArrayErrNotFound;
	return -1;
}


Object *ArrayRemove(Array *self, int index)
{
	Object *object;

	int shiftcount;
	int pos;
	int i;

	/* check bounds */
	if (index < 0 || index >= self->count)
	{
		self->error = ArrayErrBounds;
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
	self->error = ArrayErrOK;
	self->index = index;
	return object;
}


void ArraySort(Array *self)
{
	if (self->count)
		ArraySortRange(self, 0, self->count - 1);
}


Object *ArrayHead(Array *self)
{
	/* Empty array */
	if (!self->count)
	{
		self->error = ArrayErrEmpty;
		return NULL;
	}

	/* Go to head */
	self->error = ArrayErrOK;
	self->index = 0;
	
	/* Return current object */
	return self->array[0];
}


Object *ArrayTail(Array *self)
{
	/* Empty array */
	if (!self->count)
	{
		self->error = ArrayErrEmpty;
		return NULL;
	}

	/* Go to last element */
	self->index = self->count - 1;
	self->error = ArrayErrOK;

	/* Return current object */
	return self->array[self->index];
}

Object *ArrayNext(Array *self)
{
	/* Check bounds */
	assert(self->index <= self->count);
	if (self->index == self->count)
	{
		self->error = ArrayErrBounds;
		return NULL;
	}

	/* Go to next element */
	self->index++;
	
	/* End of list reached */
	if (self->index == self->count)
	{
		self->error = ArrayErrEnd;
		return NULL;
	}

	/* Return current object */
	self->error = ArrayErrOK;
	return self->array[self->index];
}


Object *ArrayPrev(Array *self)
{
	/* Check bounds */
	if (!self->index)
	{
		self->error = ArrayErrBounds;
		return NULL;
	}

	/* Move to previous element */
	self->error = ArrayErrOK;
	self->index--;

	/* Return current object */
	return self->array[self->index];
}


int ArrayIsEnd(Array *self)
{
	/* End of list */
	assert(self->index <= self->count);
	if (self->index == self->count)
	{
		self->error = ArrayErrEnd;
		return 1;
	}

	/* Not the end */
	self->error = ArrayErrOK;
	return 0;
}

