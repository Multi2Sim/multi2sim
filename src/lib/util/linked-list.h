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

#ifndef LIB_UTIL_LINKED_LIST_H
#define LIB_UTIL_LINKED_LIST_H


/* Error constants */
enum linked_list_error_t
{
	LINKED_LIST_ERR_OK = 0,
	LINKED_LIST_ERR_BOUNDS,
	LINKED_LIST_ERR_EMPTY,
	LINKED_LIST_ERR_NOT_FOUND
};


/* Element in linked list */
struct linked_list_elem_t
{
	struct linked_list_elem_t *prev;
	struct linked_list_elem_t *next;
	void *data;
};


/* Linked list */
struct linked_list_t
{
	/* Public */
	int count;
	int error_code;

	/* Private */
	struct linked_list_elem_t *head;
	struct linked_list_elem_t *tail;
	struct linked_list_elem_t *current;
	int current_index;

	/* Version number of the list state. Whenever a function call changes an
	 * element of the list (insert/remove/update), the version number will
	 * increase. */
	int version;
};


/** Iterate through all element of linked list.
 *
 * @param list
 */
#define LINKED_LIST_FOR_EACH(list) \
	for (linked_list_head(list); \
		!linked_list_is_end(list); \
		linked_list_next(list))


/** Create a linked list.
 *
 * @return
 * 	Linked list object.
 */
struct linked_list_t *linked_list_create(void);


/* Free linked list.
 *
 * @param list
 * 	Linked list object
 */
void linked_list_free(struct linked_list_t *list);


/** Get the index of the current element in the list. In a list with N elements,
 * the current element can range between [0..N], where N is a virtual empty
 * element at the end of the list. The return value of this function can also
 * be obtained in the 'current_index' field.
 *
 * @param list
 * 	List object
 *
 * @return
 * 	The function returns a value between 0 and N. The error code is set to
 * 	LINKED_LIST_ERR_OK.
 */
int linked_list_current(struct linked_list_t *list);


/** Get the data associated with the current value.
 * If the current element is not NULL, this function returns the same value as
 * 'linked_list->current->data'.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	The function returns the data in the current object, or NULL if the
 * 	current index is N. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_BOUNDS
 * 		Current element index is N.
 */
void *linked_list_get(struct linked_list_t *list);


/** Move current element forward one position.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_BOUNDS
 * 		Current element was already N.
 */
void linked_list_next(struct linked_list_t *list);


/** Move current element back one position.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_BOUNDS
 * 		Current element was already 0.
 */
void linked_list_prev(struct linked_list_t *list);


/** Move current element forward one position, treating the list as a circular
 * list. If the current element is N-1 or N, it is moved to position 0.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_EMPTY
 * 		The list is empty.
 */
void linked_list_next_circular(struct linked_list_t *list);


/** Move current element back one position, treating the list as a circular
 * list. If the current element is 0, it is moved to position N - 1.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_EMPTY
 * 		The list is empty.
 */
void linked_list_prev_circular(struct linked_list_t *list);


/** Move current element to position 0.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_EMPTY
 * 		The list is empty.
 */
void linked_list_head(struct linked_list_t *list);


/** Move current element to position N - 1.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_EMPTY
 * 		The list is empty.
 */
void linked_list_tail(struct linked_list_t *list);


/** Move current element to position N.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to LINKED_LIST_ERR_OK.
 */
void linked_list_out(struct linked_list_t *list);


/** Set the current element to a given position.
 *
 * @param list
 * 	List object.
 * @param index
 * 	Value between [0..N] specifying the list position to go to. If 'index' is
 * 	N, the current position is set to a virtual NULL element at the end of the
 * 	list.
 *
 * @return
 *      The element is returned at the position of the list given in 'index', or
 *      NULL if 'index' points to the list tail. The error code is set to one
 *      of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_BOUNDS
 * 		Index out of bounds.
 */
void *linked_list_goto(struct linked_list_t *list, int index);


/** Find the first occurrence of an element in the list.
 *
 * @param list
 * 	List object.
 * @param data
 * 	Data to find.
 *
 * @return
 * 	If the value in 'data' is found, return 'data' itself. Otherwise, return
 * 	NULL. The error code is set to one of the following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error. The value was found, and the current element is set to its
 * 		first occurrence.
 * 	LINKED_LIST_ERR_NOT_FOUND
 * 		Element not found.
 */
void *linked_list_find(struct linked_list_t *list, void *data);


/** Return number of elements in the list. The value returned by this function can
 * be also obtained in 'linked_list->count'.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	The number of elements in the list is returned. The error code is set to
 * 	LINKED_LIST_ERR_OK.
 */
int linked_list_count(struct linked_list_t *list);


/** Return 0 if the current element is a value between [0..N - 1], and return
 * non-zero if the current element is N.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	Boolean value representing whether the current element is past the end of
 * 	the list. The error code is set to LINKED_LIST_ERR_OK.
 */
int linked_list_is_end(struct linked_list_t *list);


/** Insert an element before the current element. The inserted element becomes
 * the current element in the list.
 *
 * @param list
 * 	List object.
 * @param data
 * 	Element to insert.
 *
 * @return
 * 	No value is returned. The error code is set to LINKED_LIST_ERR_OK.
 */
void linked_list_insert(struct linked_list_t *list, void *data);


/** Insert an element at the end of the list. The current element is set to
 * N (past the end of the list).
 *
 * @param list
 * 	List object.
 * @param data
 * 	Element to insert.
 *
 * @return
 * 	No value is returned. The error code is set to LINKED_LIST_ERR_OK.
 */
void linked_list_add(struct linked_list_t *list, void *data);


/** Remove the current element from the list.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	The function returned the data associated with the element removed, or
 * 	NULL if no element was removed. The error code is set to one of the
 * 	following values:
 *
 * 	LINKED_LIST_ERR_OK
 * 		No error.
 * 	LINKED_LIST_ERR_BOUNDS
 * 		The current element is N (past the end of the list).
 */
void *linked_list_remove(struct linked_list_t *list);


/** Empty the list.
 *
 * @param list
 * 	List object.
 *
 * @return
 * 	No value is returned. The error code is set to LINKED_LIST_ERR_OK.
 */
void linked_list_clear(struct linked_list_t *list);


/** Sort the list.
 *
 * @param list
 * 	List object.
 * @param comp
 * 	Call-back function comparing two elements of the list.
 *
 * @return
 * 	No value is returned. The error code is set to LINKED_LIST_ERR_OK.
 */
void linked_list_sort(struct linked_list_t *list,
	int (*comp)(const void *, const void *));


/** Check if a list is sorted.
 *
 * @param list
 * 	List object.
 * @param comp
 * 	Call-back function comparing two elements of the list.
 *
 * @return
 * 	The function returns 0 if the list is not sorted, and non-zero if
 * 	the list is sorted. The error code is set to LINKED_LIST_ERR_OK.
 */
int linked_list_sorted(struct linked_list_t *list,
	int (*comp)(const void *, const void *));




/*
 * Linked List Iterator Object
 *
 * This is an object pointing to an internal position of the linked list. It is
 * used for list traversal purposes without affecting the internal position
 * stored in the linked list. During the lifetime of an iterator, the linked
 * list shouldn't change its state. If it does, a panic message will be issues
 * by the iterator function.
 */

#define LINKED_LIST_ITER_FOR_EACH(iter) \
	for (linked_list_iter_head((iter)); \
		!linked_list_iter_is_end((iter)); \
		linked_list_iter_next((iter)))

struct linked_list_iter_t;

struct linked_list_iter_t *linked_list_iter_create(struct linked_list_t *list);
void linked_list_iter_free(struct linked_list_iter_t *iter);

void linked_list_iter_head(struct linked_list_iter_t *iter);
void linked_list_iter_next(struct linked_list_iter_t *iter);
int linked_list_iter_is_end(struct linked_list_iter_t *iter);
void *linked_list_iter_get(struct linked_list_iter_t *iter);

/* Places the iterator at the first position of the list where 'data' is
 * contained. The function returns true if 'data' is found. Otherwise, it
 * returns false and places the iterator at the end of the list. */
int linked_list_iter_find(struct linked_list_iter_t *iter, void *data);

#endif

