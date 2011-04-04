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

#ifndef LNLIST_H
#define LNLIST_H

/* Error constants */
#define LNLIST_ENOMEM		1
#define LNLIST_EBOUNDS		2
#define LNLIST_EELEM		3

struct lnlist_t;

/* Creation/Destruction */
struct lnlist_t *lnlist_create();
void lnlist_free(struct lnlist_t *lnlist);

/* Get the index of the current element.
 * In a list with n elements, the current element can range from 0 up to n. */
int lnlist_current(struct lnlist_t *lnlist);

/* Get the data corresponding to the current element.
 * If current element is 'n', return NULL and raise EBOUNDS. */
void *lnlist_get(struct lnlist_t *lnlist);  /* EBOUNDS */

/* Advance or rewind current element */
void lnlist_next(struct lnlist_t *lnlist);  /* EBOUNDS */
void lnlist_prev(struct lnlist_t *lnlist);  /* EBOUNDS */
void lnlist_head(struct lnlist_t *lnlist);  /* EBOUNDS */
void lnlist_tail(struct lnlist_t *lnlist);  /* EBOUNDS */
void lnlist_out(struct lnlist_t *lnlist);
void lnlist_goto(struct lnlist_t *lnlist, int index);  /* EBOUNDS */
void lnlist_find(struct lnlist_t *lnlist, void *data);  /* EELEM */

/* Number of elements and EOL check (end of list = current element == n */
int lnlist_count(struct lnlist_t *lnlist);
int lnlist_eol(struct lnlist_t *lnlist);

/* Insert an element before the current one. If current element index is n,
 * insert it at the end of the list */
void lnlist_insert(struct lnlist_t *lnlist, void *data); /* ENOMEM */

/* Add an element at the end of the list, then set the current element
 * to the next after the last one. */
void lnlist_add(struct lnlist_t *lnlist, void *data); /* ENOMEM */

/* Remove elements from the list. */
void lnlist_remove(struct lnlist_t *lnlist); /* EBOUNDS */
void lnlist_clear(struct lnlist_t *lnlist);

/* Sort */
void lnlist_sort(struct lnlist_t *lnlist,
	int (*comp)(const void *, const void *));
int lnlist_sorted(struct lnlist_t *lnlist,
	int (*comp)(const void *, const void *));

/* Error and error messages */
int lnlist_error(struct lnlist_t *lnlist);
char *lnlist_error_msg(struct lnlist_t *lnlist);

#endif
