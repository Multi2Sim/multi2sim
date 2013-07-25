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

#ifndef LIB_CLASS_LIST_H
#define LIB_CLASS_ARRAY_H

#include "class.h"


/*
 * Class 'Array'
 */

typedef enum
{
	ArrayErrOK = 0,
	ArrayErrBounds,
	ArrayErrNotFound,
} ListError;


CLASS_BEGIN(Array, Object)

	/* Number of elements */
	int count;

	/* Error code */
	ListError error;

	/* Allocated size */
	int size;

	/* Pointers in list */
	int head;
	int tail;

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
 * and printing string 'first' intially, 'last' finally, and 'middle' between
 * every pair of elements. */
void ArrayDumpWithDelim(Array *self, FILE *f, const char *first,
		const char *middle, const char *last);

/* Clear the content of the array. Freeing the elements that were contained
 * before the call is the responsibility of the caller. The call always succeeds
 * and updates the error code. */
void ArrayClear(Array *self);

/* Free all objects added to the array with a call to 'delete()', and clear the
 * content of the list. This call always succeeds and updates the error code. */
void ArrayDeleteObjects(Array *self);

/* Add an object at the end of the array. This call always succeeds and updates
 * the error code. */
void ArrayAdd(Array *self, Object *object);

/* Obtain the element at position 'index'. If 'index' is out of bounds, NULL is
 * returned and the error code is set to 'ArrayErrBounds'. */
Object *ArrayGet(Array *self, int index);

/* Set the element at position 'index' to 'object'. If 'index' is out of bounds,
 * the error code is set to 'ArrayErrBounds'. */
void ArraySet(Array *self, int index, Object *object);

/* Insert element 'object' at position 'index'. If 'index' is less than 0 or
 * greater than 'Array->count', the error code is set to 'ArrayErrBounds'. */
void ArrayInsert(Array *self, int index, Object *object);

/* Return the index of the first occurrence of 'object' in the array. If not
 * found, -1 is returned and the error code is set to 'ArrayErrNotFound'. */
int ArrayFind(Array *self, Object *object);

/* Remove the object at position 'index' and return it. If 'index' is out of
 * bounds, NULL is returned and the error code is set to 'ArrayErrBounds'. */
Object *ArrayRemove(Array *self, int index);

/* Sort the content of the list using virtual function 'Compare' of the
 * contained objects. */
void ArraySort(Array *self);

#endif

