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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mhandle.h>
#include <debug.h>
#include <hash-table.h>

#define	MIN_INITIAL_SIZE  5


/*
 * Types
 */

/* Hash table element */
struct hash_table_elem_t
{
	char active;
	char removed;
	char *key;
	void *data;
};


/* Hash table */
struct hash_table_t
{
	int count;
	int size;
	int case_sensitive;
	struct hash_table_elem_t *elem;
	int (*cmp)(const char *, const char *);
	int findpos;
} hash_table_t;




/*
 * PRIVATE FUNCTIONS
 */

static unsigned int hashcode(char *str, int case_sensitive)
{
	unsigned int res = 0;
	int i, len;

	len = strlen(str);
	if (case_sensitive)
		for (i = 0; i < len; i++)
			res = res * 37 + str[i];
	else
		for (i = 0; i < len; i++)
			res = res * 37 + tolower(str[i]);
	return res;
}


/* if key exists, return corresponding element;
 * otherwise, return element where it should be placed */
static struct hash_table_elem_t *hashelem(struct hash_table_t *ht, char *key)
{
	int pos, insertpos;
	struct hash_table_elem_t *e;
	
	/* find key of possible insert position */
	insertpos = -1;
	pos = hashcode(key, ht->case_sensitive) % ht->size;
	for (;;)
	{
	
		/* we run out of collision list */
		e = &ht->elem[pos];
		if (!e->active)
			break;
			
		/* the element is in hash table */
		if (!e->removed && !ht->cmp(e->key, key))
			return e;
			
		/* a possible insert position */
		if (e->removed && insertpos < 0)
			insertpos = pos;
		
		/* visit next element of collision list */
		pos = (pos + 1) % ht->size;
	}
	
	/* key is not in hash table; return insert pos */
	return insertpos < 0 ? &ht->elem[pos] :
		&ht->elem[insertpos];
}


/* Rehashing */
static void rehash(struct hash_table_t *ht)
{
	int osize, nsize, i;
	struct hash_table_elem_t *oelem, *oe, *ne;
	
	/* Create new elements vector */
	oelem = ht->elem;
	osize = ht->size;
	nsize = osize * 2;
	ht->size = nsize;
	ht->elem = calloc(nsize, sizeof(struct hash_table_elem_t));
	if (!ht->elem)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Assign new elements */
	for (i = 0; i < osize; i++)
	{
		oe = &oelem[i];
		if (oe->active && !oe->removed)
		{
			ne = hashelem(ht, oe->key);
			ne->active = 1;
			ne->key = oe->key;
			ne->data = oe->data;
		}
	}
	
	/* Free old vector */
	free(oelem);
}




/*
 * Public Functions
 */

struct hash_table_t *hash_table_create(int size, int case_sensitive)
{
	struct hash_table_t *ht;
	
	/* Create */
	ht = calloc(1, sizeof(struct hash_table_t));
	if (!ht)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Assign fields */
	ht->size = size < MIN_INITIAL_SIZE ? MIN_INITIAL_SIZE : size;
	ht->case_sensitive = case_sensitive;
	ht->cmp = case_sensitive ? strcmp : strcasecmp;
	ht->elem = calloc(ht->size, sizeof(struct hash_table_elem_t));
	if (!ht->elem)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Return */
	return ht;
}


void hash_table_free(struct hash_table_t *ht)
{
	int i;
	struct hash_table_elem_t *e;
	
	/* Free keys */
	for (i = 0; i < ht->size; i++) {
		e = &ht->elem[i];
		if (e->active && !e->removed)
			free(e->key);
	}
	
	/* Free elems and hash table */
	free(ht->elem);
	free(ht);
}


int hash_table_insert(struct hash_table_t *ht, char *key, void *data)
{
	struct hash_table_elem_t *e;
	
	/* Data cannot be null */
	if (!data)
		return 0;
	
	/* Rehashing */
	if (ht->count >= ht->size / 2)
		rehash(ht);
	
	/* Element must not exists */
	e = hashelem(ht, key);
	if (e->active && !e->removed)
		return 0;
	
	/* Insert element */
	ht->count++;
	e->active = 1;
	e->removed = 0;
	e->key = strdup(key);
	if (!e->key)
		fatal("%s: out of memory", __FUNCTION__);
	e->data = data;
	return 1;
}


int hash_table_set(struct hash_table_t *ht, char *key, void *data)
{
	struct hash_table_elem_t *e;
	
	/* Data cannot be null */
	if (!data)
		return 0;
	
	/* Element must exist */
	e = hashelem(ht, key);
	if (!e->active || e->removed)
		return 0;
	
	/* Set new data */
	e->data = data;
	return 1;
}


int hash_table_count(struct hash_table_t *ht)
{
	return ht->count;
}


void *hash_table_get(struct hash_table_t *ht, char *key)
{
	struct hash_table_elem_t *e;
	
	/* Element must exist */
	e = hashelem(ht, key);
	if (!e->active || e->removed)
		return NULL;
	
	/* Return associated data */
	return e->data;
}


void *hash_table_remove(struct hash_table_t *ht, char *key)
{
	struct hash_table_elem_t *e;
	void *data;
	
	/* Element must exist */
	e = hashelem(ht, key);
	if (!e->active || e->removed)
		return NULL;
	
	/* Remove element */
	free(e->key);
	data = e->data;
	e->key = NULL;
	e->data = NULL;
	e->removed = 1;
	ht->count--;
	return data;
}


char *hash_table_find_first(struct hash_table_t *ht, void **data_ptr)
{
	ht->findpos = 0;
	return hash_table_find_next(ht, data_ptr);
}


char *hash_table_find_next(struct hash_table_t *ht, void **data_ptr)
{
	struct hash_table_elem_t *e;
	while (ht->findpos < ht->size)
	{
		e = &ht->elem[ht->findpos];
		ht->findpos++;
		if (e->active && !e->removed)
		{
			if (data_ptr)
				*data_ptr = e->data;
			return e->key;
		}
	}

	/* No more elements found */
	if (data_ptr)
		*data_ptr = NULL;
	return NULL;
}

