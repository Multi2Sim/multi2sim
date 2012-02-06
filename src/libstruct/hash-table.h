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

#ifndef HASH_H
#define HASH_H

struct hash_table_t;

/* Creation and destruction */
struct hash_table_t *hash_table_create(int size, int case_sensitive);
void hash_table_free(struct hash_table_t *ht);

/* Insert a new element.
 * The key is strdup'ped, so it can be freely modified by the caller.
 * Return value: 0=success, non-0=key already exists/outofmem/data=NULL
 */
int hash_table_insert(struct hash_table_t *ht, char *key, void *data);

/* Change element data.
 * Return value: 0=success, non-0=key does not exist/data=NULL
 */
int hash_table_set(struct hash_table_t *ht, char *key, void *data);

/* Return number of elements in hash_table. */
int hash_table_count(struct hash_table_t *ht);

/* Get data associated to a key.
 * Return value: NULL=key does not exist, ptr=data */
void *hash_table_get(struct hash_table_t *ht, char *key);

/* Remove data associated to a key; the key is freed.
 * Return value: NULL=key does not exist, ptr=data removed */
void *hash_table_remove(struct hash_table_t *ht, char *key);

/* Find elements in hash table sequentially.
 * Return value: NULL=no more elements,
 *   non-NULL=key (data returned in 'data' if not NULL) */
char *hash_table_find_first(struct hash_table_t *ht, void **data);
char *hash_table_find_next(struct hash_table_t *ht, void **data);

#endif

