/*
 *  Libstruct
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/* Error constants */
enum linked_list_error_t
{
	LINKED_LIST_ERR_OK = 0,
	LINKED_LIST_ERR_MEM,
	LINKED_LIST_ERR_BOUNDS,
	LINKED_LIST_ERR_NOT_FOUND
};

struct linked_list_t;

/* Creation/Destruction */
struct linked_list_t *linked_list_create();
void linked_list_free(struct linked_list_t *list);

/* Get the index of the current element.
 * In a list with n elements, the current element can range from 0 up to n. */
int linked_list_current(struct linked_list_t *list);

/* Get the data corresponding to the current element.
 * If current element is 'n', return NULL and raise EBOUNDS. */
void *linked_list_get(struct linked_list_t *list);  /* EBOUNDS */

/* Advance or rewind current element */
void linked_list_next(struct linked_list_t *list);  /* EBOUNDS */
void linked_list_prev(struct linked_list_t *list);  /* EBOUNDS */
void linked_list_next_circular(struct linked_list_t *list);  /* EBOUNDS */
void linked_list_prev_circular(struct linked_list_t *list);  /* EBOUNDS */
void linked_list_head(struct linked_list_t *list);  /* EBOUNDS */
void linked_list_tail(struct linked_list_t *list);  /* EBOUNDS */
void linked_list_out(struct linked_list_t *list);
void linked_list_goto(struct linked_list_t *list, int index);  /* EBOUNDS */
void linked_list_find(struct linked_list_t *list, void *data);  /* EELEM */

/* Number of elements and EOL check (end of list = current element == n */
int linked_list_count(struct linked_list_t *list);
int linked_list_is_end(struct linked_list_t *list);

/* Insert an element before the current one. If current element index is n,
 * insert it at the end of the list */
void linked_list_insert(struct linked_list_t *list, void *data); /* ENOMEM */

/* Add an element at the end of the list, then set the current element
 * to the next after the last one. */
void linked_list_add(struct linked_list_t *list, void *data); /* ENOMEM */

/* Remove elements from the list. */
void linked_list_remove(struct linked_list_t *list); /* EBOUNDS */
void linked_list_clear(struct linked_list_t *list);

/* Sort */
void linked_list_sort(struct linked_list_t *list,
	int (*comp)(const void *, const void *));
int linked_list_sorted(struct linked_list_t *list,
	int (*comp)(const void *, const void *));

/* Error and error messages */
int linked_list_error(struct linked_list_t *list);
char *linked_list_error_msg(struct linked_list_t *list);

#endif
