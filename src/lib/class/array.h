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

#ifndef LIB_CLASS_ARRAY_H
#define LIB_CLASS_ARRAY_H

#include "class.h"


/*
 * Class 'Array'
 */

#define ArrayForEach(_array, _object, _class) \
	for ((_object) = as##_class(ArrayHead((_array))); \
		!ArrayIsEnd((_array)); \
		(_object) = as##_class(ArrayNext((_array))))

typedef enum
{
	ArrayErrOK = 0,
	ArrayErrBounds,
	ArrayErrNotFound,
	ArrayErrEmpty,
	ArrayErrEnd
} ArrayError;


CLASS_BEGIN(Array, Object)

	/* Number of elements */
	int count;

	/* Error code */
	ArrayError error;

	/* Allocated size */
	int size;

	/* Pointers in list */
	int head;
	int tail;

	/* Internal iterator */
	int index;

	/* Elements */
	Object **array;

CLASS_END(Array)


void ArrayCreate(Array *self);
void ArrayCreateWithSize(Array *self, int size);
void ArrayDestroy(Array *self);

/* Dump the content of the array by calling each object's 'Dump' virtual method
 * followed by a newline character. */
void ArrayDump(Object *self, FILE *f);

/* Dump the content of the array by calling each object's 'Dump' virtual method
 * and printing string 'first' initially, 'last' finally, and 'middle' between
 * every pair of elements. */
void ArrayDumpWithDelim(Array *self, FILE *f, const char *first,
		const char *middle, const char *last);

/* Clear the content of the array. Freeing the elements that were contained
 * before the call is the responsibility of the caller. Set the error code to
 * 'ArrayErrOK' and the internal iterator to 0. */
void ArrayClear(Array *self);

/* Free all objects added to the array with a call to 'delete()', and clear the
 * content of the list. This call always succeeds and updates the error code. */
void ArrayDeleteObjects(Array *self);

/* Add an object at the end of the array and set the error code to 'ArrayErrOK'.
 * The internal iterator is set to point to the last element of the array. */
void ArrayAdd(Array *self, Object *object);

/* Obtain the element at position 'index'. If 'index' is out of bounds, NULL is
 * returned and the error code is set to 'ArrayErrBounds'. */
Object *ArrayGet(Array *self, int index);

/* Set the element at position 'index' to 'object'. If 'index' is out of bounds,
 * the error code is set to 'ArrayErrBounds'. The internal iterator is moved to
 * position 'index'. */
void ArraySet(Array *self, int index, Object *object);

/* Insert element 'object' at position 'index'. If 'index' is less than 0 or
 * greater than 'Array->count', the error code is set to 'ArrayErrBounds'. Set
 * the internal iterator to 'index'. */
void ArrayInsert(Array *self, int index, Object *object);

/* Return the index of the first occurrence of 'object' in the array. If not
 * found, -1 is returned and the error code is set to 'ArrayErrNotFound'. */
int ArrayFind(Array *self, Object *object);

/* Remove the object at position 'index' and return it. If 'index' is out of
 * bounds, NULL is returned and the error code is set to 'ArrayErrBounds'. The
 * internal iterator is set to 'index' (which is past the end of the array if
 * the removed element was the last). */
Object *ArrayRemove(Array *self, int index);

/* Sort the content of the list using virtual function 'Compare' of the
 * contained objects. */
void ArraySort(Array *self);

/* Set the internal iterator to the first element of the array and return the
 * object at that position. If the array is empty, return NULL and set the error
 * code to 'ArrayErrEmpty'. */
Object *ArrayHead(Array *self);

/* Set the internal iterator to the last element of the array and return the
 * object at that position. If the array is empty, return NULL and set the error
 * code to 'ArrayErrEmpty'. */
Object *ArrayTail(Array *self);

/* Move the internal iterator one position forward and return the object at that
 * position. If the end of the array is reached for the first time, return NULL
 * and set the error code to 'ArrayErrEnd'. If the iterator was already at the
 * end of the array, return NULL as well and set the error code to
 * 'ArrayErrBounds'. */
Object *ArrayNext(Array *self);

/* Move the internal iterator one position backward and return the object at
 * that position. If the iterator was already at the beginning of the list, NULL
 * is returned and the error code is set to 'ArrayErrBounds'. */
Object *ArrayPrev(Array *self);

/* Return true if the internal iterator points past the end of the array. If so,
 * the error code is set to 'ArrayErrEnd'. */
int ArrayIsEnd(Array *self);


#endif

