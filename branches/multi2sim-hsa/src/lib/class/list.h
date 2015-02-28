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
#define LIB_CLASS_LIST_H

#include "class.h"


/*
 * Class 'List'
 */

#define ListForEach(_list, _object, _class) \
	for ((_object) = as##_class(ListHead((_list))); \
		!ListIsEnd((_list)); \
		(_object) = as##_class(ListNext((_list))))

typedef enum
{
	ListErrOK = 0,
	ListErrBounds,
	ListErrEnd,
	ListErrNotFound,
	ListErrEmpty
} ListError;

struct list_elem_t;

CLASS_BEGIN(List, Object)

	/* Public */
	int count;
	int error;

	/* Private */
	struct list_elem_t *head;
	struct list_elem_t *tail;
	struct list_elem_t *current;
	int current_index;

	/* Version number of the list state. Whenever a function call changes an
	 * element of the list (insert/remove/update), the version number will
	 * increase. Version numbers are associated with list iterators, which
	 * will invalidate an iterator when the list state changes. */
	int version;

CLASS_END(List)


void ListCreate(List *self);
void ListDestroy(List *self);

/* Dump the content of the list by calling each object's 'Dump' virtual method
 * of each object followed by a newline character. */
void ListDump(Object *self, FILE *f);

/* Dump the content of the list by calling each object's 'Dump' virtual method
 * and printing string 'first' initially, 'last' finally, and 'middle' between
 * every pair of elements. */
void ListDumpWithDelim(List *self, FILE *f, const char *first,
		const char *middle, const char *last);

/* Clear the content of the list without destroying contained objects. The error
 * code is updated to 'ListErrOK'. */
void ListClear(List *self);

/* Clear the content of the list and destroy all objects contained in it by
 * invoking 'delete()'. The error code is set to 'ListErrOK'. */
void ListDeleteObjects(List *self);

/* Return the current element of the list. If the internal list iterator points
 * to the end of the list, NULL is returned and the error code is set to
 * 'ListErrEnd'. */
Object *ListGet(List *self);

/* Advance the internal list iterator one position forward and return the
 * current object. If the end of the list is reached for the first time, NULL is
 * returned and the error code is set to 'ListErrEnd'. If the current position
 * was already the end of the list, return NULL as well and set the error code
 * to 'ListErrBounds'. */
Object *ListNext(List *self);

/* Move the internal list iterator one position backward and return the current
 * object. If the iterator was already at the beginning of the list, NULL is
 * returned and the error code is set to 'ListErrBounds'. */
Object *ListPrev(List *self);

/* Move internal list iterator one position forward. If the current position was
 * the list tail or past the list tail, it will be moved to the head. The
 * current object is returned. If the list is empty, NULL is returned and the
 * error code is set to 'ListErrEmpty'. */
Object *ListNextCircular(List *self);

/* Move internal list iterator one position backward. If the current position
 * was the list head, move it to the list tail. The current object is returned.
 * If the list is empty, NULL is returned and the error code is set to
 * 'ListErrEmpty'. */
Object *ListPrevCircular(List *self);

/* Move internal list iterator to the head of the list and return the object at
 * that position. If the list is empty, return NULL and set the error code to
 * 'ListErrEmpty'. */
Object *ListHead(List *self);

/* Move internal list iterator to the tail of the list and return the object at
 * that position. If the list is empty, return NULL and set the error code to
 * 'ListErrEmpty'. */
Object *ListTail(List *self);

/* Place the internal list iterator after the last element of the list. This
 * function always succeeds, and sets the error code to 'ListErrOK'. */
void ListOut(List *self);

/* Place the internal list iterator in position 'index' and return the object in
 * that position. If the iterator is placed in the position immediately
 * following the tail, NULL is returned and the error code is set to
 * 'ListErrEnd'. For any other invalid position, NULL is returned as well and
 * the error code is set to 'ListErrBounds'. */
Object *ListGoto(List *self, int index);

/* Place the internal list iterator at the position of the first occurrence of
 * 'object' in the list, and return 'object'. If 'object' is not in the list,
 * return NULL and set the error code to 'ListErrNotFound'. */
Object *ListFind(List *self, Object *object);

/* Return whether the list iterator is past the tail. If so, the error code is
 * set to 'ListErrEnd'. */
int ListIsEnd(List *self);

/* Insert an object before the element pointed to by the internal list
 * iterator. If the iterator points past the list tail, the object is added at
 * the end of the list. After the operation, the list iterator points to the new
 * element, and the error code is set to 'ListErrOK'. The function returns the
 * same value passed in argument 'object'. */
Object *ListInsert(List *self, Object *object);

/* Insert an object at the end of the list, place the list iterator past the
 * list tail, set the error code to 'ListErrOK', and return the object just
 * inserted. */
Object *ListAdd(List *self, Object *object);

/* Remove the element at the position pointed to by the internal iterator of the
 * list, and return the removed object. The list iterator is placed at the
 * position immediately following the removed element. If the iterator was
 * initially pointing past the list tail, NULL is returned and the error code is
 * set to 'ListErrBounds'. */
Object *ListRemove(List *self);

/* Sort the content of the list using the 'Compare' virtual function of the
 * objects contained in it. The error code is set to 'ListErrOK'. */
void ListSort(List *self);



/*
 * Class 'ListIterator'
 */

#define ListIteratorForEach(_iterator, _object, _class) \
	for ((_object) = as##_class(ListIteratorHead((_iterator))); \
		!ListIteratorIsEnd((_iterator)); \
		(_object) = as##_class(ListIteratorNext((_iterator))))

CLASS_BEGIN(ListIterator, Object)

	/* Associated list */
	List *list;
	
	/* Version of the iterator. This version is checked in every call
	 * against the version of the list for sanity purposes. */
	int version;

	/* Error code of last operation */
	ListError error;

	/* Current position of the iterator */
	int index;

	/* Element of the list that the iterator points to. */
	struct list_elem_t *elem;

CLASS_END(ListIterator)


/* Create an iterator for 'list' and make it initially point to its head. */
void ListIteratorCreate(ListIterator *self, List *list);

/* Destructor */
void ListIteratorDestroy(ListIterator *self);

/* Place iterator at the head of the list and return the object at that
 * position. If the list is empty, return NULL and set the error code of the
 * iterator to 'ListErrEmpty'. */
Object *ListIteratorHead(ListIterator *self);

/* Place iterator at the tail of the list and return the object at that
 * position. If the list is empty, return NULL and set the error code of the
 * iterator to 'ListErrEmpty'. */
Object *ListIteratorTail(ListIterator *self);

/* Place iterator past the tail of the list, and set the error code of the
 * iterator to 'ListErrOK'. */
void ListIteratorOut(ListIterator *self);

/* Move iterator to next element in the list, and return the object at that
 * position. If the list tail is passed for the first time, return NULL and set
 * the error code of the iterator to 'ListErrEnd'. If the iterator was already
 * past the end of the list before this call, return NULL and set the error code
 * of the iterator to 'ListErrBounds'. */
Object *ListIteratorNext(ListIterator *self);

/* Move iterator to the previous element in the list and return the object at
 * that position. If the iterator was already pointing to the head, NULL is
 * returned and the error code of the iterator is set to 'ListErrBounds'. */
Object *ListIteratorPrev(ListIterator *self);

/* Move iterator to position 'index' in the list and return the object at that
 * position. If 'index' is an invalid value greater than the position past the
 * end of the list, NULL is returned and the error code of the iterator is set
 * to 'ListErrBounds'. If the value of 'index' places the iterator at the
 * position right past the end of the list, NULL is returned and the error code
 * of the iterator is set to 'ListErrEnd'. */
Object *ListIteratorGoto(ListIterator *self, int index);

/* Move iterator to the first occurrence of 'object' in the list and return the
 * object. If the object is not in the list, return NULL and set the error code
 * of the iterator to 'ListErrNotFound'. */
Object *ListIteratorFind(ListIterator *self, Object *object);

/* Return true if the iterator points to the position past the end of the list.
 * If so, set the error code of the iterator to 'ListErrEnd'. */
int ListIteratorIsEnd(ListIterator *self);

/* Return the object pointed to by the list iterator. If the iterator is
 * pointing past the list tail, NULL is returned and the error code of the
 * iterator is set to 'ListErrEnd'. */
Object *ListIteratorGet(ListIterator *self);

#endif

