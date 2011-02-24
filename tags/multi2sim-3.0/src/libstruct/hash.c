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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mhandle.h>
#include "hash.h"

#define	MIN_INITIAL_SIZE	5


/*
 * TYPES
 */

/* hash table element */
struct elem_t {
	char	active;
	char	removed;
	char	*key;
	void	*data;
};


/* hash table */
struct hashtable_t {
	int	count;
	int	size;
	int	casesen;
	struct	elem_t *elem;
	int	(*cmp)(const char *, const char *);
	int	findpos;
} hash_table_t;




/*
 * PRIVATE FUNCTIONS
 */

static unsigned int hashcode(char *str, int casesen)
{
	unsigned int res = 0;
	int i, len;

	len = strlen(str);
	if (casesen)
		for (i = 0; i < len; i++)
			res = res * 37 + str[i];
	else
		for (i = 0; i < len; i++)
			res = res * 37 + tolower(str[i]);
	return res;
}


/* if key exists, return corresponding element;
 * otherwise, return element where it should be placed */
static struct elem_t *hashelem(struct hashtable_t *ht, char *key)
{
	int pos, insertpos;
	struct elem_t *e;
	
	/* find key of possible insert position */
	insertpos = -1;
	pos = hashcode(key, ht->casesen) % ht->size;
	for (;;) {
	
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


/* rehashing */
static int rehash(struct hashtable_t *ht)
{
	int osize, nsize, i;
	struct elem_t *oelem, *oe, *ne;
	
	/* create new elements vector */
	oelem = ht->elem;
	osize = ht->size;
	nsize = osize * 2;
	ht->elem = calloc(nsize, sizeof(struct elem_t));
	if (!ht->elem)
		return 0;
	ht->size = nsize;
	
	/* assign new elements */
	for (i = 0; i < osize; i++) {
		oe = &oelem[i];
		if (oe->active && !oe->removed) {
			ne = hashelem(ht, oe->key);
			ne->active = 1;
			ne->key = oe->key;
			ne->data = oe->data;
		}
	}
	
	/* free old vector */
	free(oelem);
	return 1;
}




/*
 * PUBLIC FUNCTIONS
 */

struct hashtable_t *hashtable_create(int size, int casesen)
{
	struct hashtable_t *ht;
	
	/* create */
	ht = calloc(1, sizeof(struct hashtable_t));
	if (!ht)
		return NULL;
	
	/* assign fields */
	ht->size = size < MIN_INITIAL_SIZE ? MIN_INITIAL_SIZE : size;
	ht->casesen = casesen;
	ht->cmp = casesen ? strcmp : strcasecmp;
	ht->elem = calloc(ht->size, sizeof(struct elem_t));
	if (!ht->elem) {
		free(ht);
		return NULL;
	}
	
	/* return */
	return ht;
}


void hashtable_free(struct hashtable_t *ht)
{
	int i;
	struct elem_t *e;
	
	/* free keys */
	for (i = 0; i < ht->size; i++) {
		e = &ht->elem[i];
		if (e->active && !e->removed)
			free(e->key);
	}
	
	/* free elems and hash table */
	free(ht->elem);
	free(ht);
}


int hashtable_insert(struct hashtable_t *ht, char *key, void *data)
{
	struct elem_t *e;
	
	/* data cannot be null */
	if (!data)
		return 0;
	
	/* rehashing */
	if (ht->count >= ht->size / 2)
		if (!rehash(ht))
			return 0;
	
	/* element must not exists */
	e = hashelem(ht, key);
	if (e->active && !e->removed)
		return 0;
	
	/* insert element */
	ht->count++;
	e->active = 1;
	e->removed = 0;
	e->key = strdup(key);
	e->data = data;
	return 1;
}


int hashtable_set(struct hashtable_t *ht, char *key, void *data)
{
	struct elem_t *e;
	
	/* data cannot be null */
	if (!data)
		return 0;
	
	/* element must exist */
	e = hashelem(ht, key);
	if (!e->active || e->removed)
		return 0;
	
	/* set new data */
	e->data = data;
	return 1;
}


int hashtable_count(struct hashtable_t *ht)
{
	return ht->count;
}


void *hashtable_get(struct hashtable_t *ht, char *key)
{
	struct elem_t *e;
	
	/* element must exist */
	e = hashelem(ht, key);
	if (!e->active || e->removed)
		return NULL;
	
	/* return associated data */
	return e->data;
}


void *hashtable_remove(struct hashtable_t *ht, char *key)
{
	struct elem_t *e;
	void *data;
	
	/* element must exist */
	e = hashelem(ht, key);
	if (!e->active || e->removed)
		return NULL;
	
	/* remove element */
	free(e->key);
	data = e->data;
	e->key = NULL;
	e->data = NULL;
	e->removed = 1;
	ht->count--;
	return data;
}


char *hashtable_find_first(struct hashtable_t *ht, void **data)
{
	ht->findpos = 0;
	return hashtable_find_next(ht, data);
}


char *hashtable_find_next(struct hashtable_t *ht, void **data)
{
	struct elem_t *e;
	while (ht->findpos < ht->size) {
		e = &ht->elem[ht->findpos];
		ht->findpos++;
		if (e->active && !e->removed) {
			if (data)
				*data = e->data;
			return e->key;
		}
	}
	return NULL;
}
