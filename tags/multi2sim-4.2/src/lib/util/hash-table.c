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

#include <assert.h>
#include <ctype.h>

#include <lib/mhandle/mhandle.h>

#include "debug.h"
#include "hash-table.h"


#define	HASH_TABLE_MIN_INITIAL_SIZE  5



/*
 * Hash Table Element
 */

struct hash_table_elem_t
{
	char *key;
	void *data;
	struct hash_table_elem_t *next;
};


struct hash_table_elem_t *hash_table_elem_create(char *key, void *data)
{
	struct hash_table_elem_t *elem;

	/* Initialize */
	elem = xcalloc(1, sizeof(struct hash_table_elem_t));
	elem->key = xstrdup(key);
	elem->data = data;

	/* Return */
	return elem;
}


void hash_table_elem_free(struct hash_table_elem_t *elem)
{
	free(elem->key);
	free(elem);
}




/*
 * Hash Table
 */

struct hash_table_t
{
	int count;
	int size;
	int case_sensitive;

	int find_op;
	int find_index;

	struct hash_table_elem_t *find_elem;

	struct hash_table_elem_t **elem_vector;

	int (*str_compare_func)(const char *, const char *);
};


static int hash_table_get_index(struct hash_table_t *table, char *key)
{
	unsigned int c;
	unsigned int hash;
	unsigned int prime;

	hash = 5381;
	prime = 16777619;
	while (*key)
	{
		c = * (unsigned char *) key;
		if (!table->case_sensitive)
			c = tolower(c);
		hash = (hash ^ c) * prime;
		key++;
	}
	hash %= table->size;

	return hash;
}


static void hash_table_grow(struct hash_table_t *table)
{
	int old_size;
	int index;
	int i;

	struct hash_table_elem_t **old_elem_vector;
	struct hash_table_elem_t *elem;

	/* Save old vector */
	old_size = table->size;
	old_elem_vector = table->elem_vector;

	/* Allocate new vector */
	table->size = old_size * 2;
	table->elem_vector = xcalloc(table->size, sizeof(void *));

	/* Move elements to new vector */
	for (i = 0; i < old_size; i++)
	{
		while ((elem = old_elem_vector[i]))
		{
			/* Remove from old vector */
			old_elem_vector[i] = elem->next;

			/* Insert in new vector */
			index = hash_table_get_index(table, elem->key);
			elem->next = table->elem_vector[index];
			table->elem_vector[index] = elem;
		}
	}

	/* Free old vector */
	free(old_elem_vector);
}


static struct hash_table_elem_t *hash_table_find(struct hash_table_t *table, char *key, int *index_ptr)
{
	struct hash_table_elem_t *elem;

	int index;

	/* Get index */
	index = hash_table_get_index(table, key);
	if (index_ptr)
		*index_ptr = index;

	/* Look for element */
	for (elem = table->elem_vector[index]; elem; elem = elem->next)
		if (!table->str_compare_func(key, elem->key))
			return elem;

	/* Not found */
	return NULL;
}


struct hash_table_t *hash_table_create(int size, int case_sensitive)
{
	struct hash_table_t *table;

	/* Assign fields */
	table = xcalloc(1, sizeof(struct hash_table_t));
	table->size = size < HASH_TABLE_MIN_INITIAL_SIZE ? HASH_TABLE_MIN_INITIAL_SIZE : size;
	table->case_sensitive = case_sensitive;
	table->str_compare_func = case_sensitive ? strcmp : strcasecmp;

	/* Return */
	table->elem_vector = xcalloc(table->size, sizeof(void *));
	return table;
}


void hash_table_free(struct hash_table_t *table)
{
	/* Clear table */
	hash_table_clear(table);

	/* Free element vector and hash table */
	free(table->elem_vector);
	free(table);
}


void hash_table_clear(struct hash_table_t *table)
{
	struct hash_table_elem_t *elem;
	struct hash_table_elem_t *elem_next;

	int i;

	/* No find operation */
	table->find_op = 0;

	/* Free elements */
	for (i = 0; i < table->size; i++)
	{
		while ((elem = table->elem_vector[i]))
		{
			elem_next = elem->next;
			hash_table_elem_free(elem);
			table->elem_vector[i] = elem_next;
		}
	}

	/* Reset count */
	table->count = 0;
}


int hash_table_insert(struct hash_table_t *table, char *key, void *data)
{
	struct hash_table_elem_t *elem;

	int index;

	/* No find operation */
	table->find_op = 0;

	/* Data cannot be null */
	if (!data)
		return 0;

	/* Rehashing */
	if (table->count >= table->size / 2)
		hash_table_grow(table);

	/* Element must not exists */
	elem = hash_table_find(table, key, &index);
	if (elem)
		return 0;

	/* Create element and insert at the head of collision list */
	elem = hash_table_elem_create(key, data);
	elem->next = table->elem_vector[index];
	table->elem_vector[index] = elem;

	/* One more element */
	table->count++;
	assert(table->count < table->size);

	/* Success */
	return 1;
}


int hash_table_set(struct hash_table_t *table, char *key, void *data)
{
	struct hash_table_elem_t *elem;

	/* Data cannot be null */
	if (!data)
		return 0;

	/* Find element */
	elem = hash_table_find(table, key, NULL);
	if (!elem)
		return 0;

	/* Set new data, success */
	elem->data = data;
	return 1;
}


int hash_table_count(struct hash_table_t *table)
{
	return table->count;
}


void *hash_table_get(struct hash_table_t *table, char *key)
{
	struct hash_table_elem_t *elem;

	/* Find element */
	elem = hash_table_find(table, key, NULL);
	if (!elem)
		return NULL;

	/* Return data */
	return elem->data;
}


void *hash_table_remove(struct hash_table_t *table, char *key)
{
	struct hash_table_elem_t *elem;
	struct hash_table_elem_t *elem_prev;

	int index;

	void *data;

	/* No find operation */
	table->find_op = 0;

	/* Find element */
	index = hash_table_get_index(table, key);
	elem_prev = NULL;
	for (elem = table->elem_vector[index]; elem; elem = elem->next)
	{
		/* Check if it is this element */
		if (!table->str_compare_func(elem->key, key))
			break;

		/* Record previous element */
		elem_prev = elem;
	}

	/* Element not found */
	if (!elem)
		return NULL;

	/* Delete element from collision list */
	if (elem_prev)
		elem_prev->next = elem->next;
	else
		table->elem_vector[index] = elem->next;

	/* Free element */
	data = elem->data;
	hash_table_elem_free(elem);

	/* One less element */
	assert(table->count > 0);
	table->count--;

	/* Return associated data */
	return data;
}


char *hash_table_find_first(struct hash_table_t *table, void **data_ptr)
{
	struct hash_table_elem_t *elem;

	int index;

	/* Record find operation */
	table->find_op = 1;
	table->find_index = 0;
	table->find_elem = NULL;
	if (data_ptr)
		*data_ptr = NULL;

	/* Table is empty */
	if (!table->count)
		return NULL;

	/* Find first element */
	for (index = 0; index < table->size; index++)
	{
		elem = table->elem_vector[index];
		if (elem)
		{
			table->find_index = index;
			table->find_elem = elem;
			if (data_ptr)
				*data_ptr = elem->data;
			return elem->key;
		}
	}

	/* Never get here */
	panic("%s: inconsistent hash table", __FUNCTION__);
	return NULL;
}


char *hash_table_find_next(struct hash_table_t *table, void **data_ptr)
{
	struct hash_table_elem_t *elem;

	int index;

	/* Not allowed if last operation is not 'hash_table_find_xxx' operation. */
	if (!table->find_op)
		panic("%s: hash table enumeration interrupted", __FUNCTION__);

	/* End of enumeration reached in previous calls */
	if (!table->find_elem)
		return NULL;

	/* Continue enumeration in collision list */
	elem = table->find_elem->next;
	if (elem)
	{
		table->find_elem = elem;
		if (data_ptr)
			*data_ptr = elem->data;
		return elem->key;
	}

	/* Continue enumeration in vector */
	table->find_index++;
	for (index = table->find_index; index < table->size; index++)
	{
		elem = table->elem_vector[index];
		if (elem)
		{
			table->find_index = index;
			table->find_elem = elem;
			if (data_ptr)
				*data_ptr = elem->data;
			return elem->key;
		}
	}

	/* No element found */
	table->find_index = 0;
	table->find_elem = NULL;
	if (data_ptr)
		*data_ptr = NULL;
	return NULL;
}
