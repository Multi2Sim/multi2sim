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

#ifndef LIB_UTIL_HEAP_H
#define LIB_UTIL_HEAP_H

/* error constants */
#define HEAP_EEMPTY	1
#define HEAP_EELEM	2


struct heap_t;

/* heap extraction policy for elements with same value:
 * fifo: oldest inserted value is extracted first
 * lifo: youngest value first */
enum heap_time_policy_enum {
	heap_time_policy_fifo = 0,	/* default */
	heap_time_policy_lifo
};

/* creation and destruction */
struct heap_t *heap_create(int size);
void heap_free(struct heap_t *heap);

/* return error occurred in last heap operation;
 * 0 means success */
int heap_error(struct heap_t *heap);
char *heap_error_msg(struct heap_t *heap);

/* heap operations */
int heap_count(struct heap_t *heap);
void heap_insert(struct heap_t *heap, long long value, void *data);
long long heap_extract(struct heap_t *heap, void **data);
long long heap_peek(struct heap_t *heap, void **data);  /* EEMPTY */
void heap_time_policy(struct heap_t *heap, enum heap_time_policy_enum policy);

/* heap enumeration */
long long heap_first(struct heap_t *heap, void **data);  /* EELEM */
long long heap_next(struct heap_t *heap, void **data);  /* EELEM */


#endif
