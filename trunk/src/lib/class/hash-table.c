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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "hash-table.h"


/*
 * Object 'hash_table_elem_t'
 */

struct hash_table_elem_t
{
	Object *key;
	Object *data;
	struct hash_table_elem_t *next;
};


struct hash_table_elem_t *hash_table_elem_create(Object *key, Object *data)
{
	struct hash_table_elem_t *elem;

	/* Initialize */
	elem = xcalloc(1, sizeof(struct hash_table_elem_t));
	elem->key = key;
	elem->data = data;

	/* Return */
	return elem;
}


void hash_table_elem_free(struct hash_table_elem_t *elem)
{
	free(elem);
}




/*
 * Class 'HashTable'
 */

static struct hash_table_elem_t *HashTableFind(HashTable *self, Object *key,
		int *index_ptr)
{
	struct hash_table_elem_t *elem;
	int index;

	/* Get index */
	index = key->Hash(key) % self->size;
	if (index_ptr)
		*index_ptr = index;

	/* Look for element */
	for (elem = self->array[index]; elem; elem = elem->next)
		if (!key->Compare(key, elem->key))
			return elem;

	/* Not found */
	return NULL;
}


static void HashTableGrow(HashTable *self)
{
	int old_size;
	int index;
	int i;

	struct hash_table_elem_t **old_array;
	struct hash_table_elem_t *elem;

	Object *key;

	/* Save old array */
	old_size = self->size;
	old_array = self->array;

	/* Allocate new array */
	self->size = old_size * 2;
	self->array = xcalloc(self->size, sizeof(void *));

	/* Move elements to new vector */
	for (i = 0; i < old_size; i++)
	{
		while ((elem = old_array[i]))
		{
			/* Remove from old vector */
			old_array[i] = elem->next;

			/* Insert in new vector */
			key = elem->key;
			index = key->Hash(key) % self->size;
			elem->next = self->array[index];
			self->array[index] = elem;
		}
	}

	/* Free old vector */
	free(old_array);
}


static void HashTableValidateKey(HashTable *self, Object *key)
{
#ifndef NDEBUG
	/* Check for valid 'key' object */
	if (!key->Clone || !key->Hash || !key->Compare)
	{
		fprintf(stderr, "key: ");
		ObjectDump(key, stderr);
		panic("%s: object 'key' must override virtual functions "
				"'Clone', 'Compare', and 'Hash'",
				__FUNCTION__);
	}
#endif
}


void HashTableCreateWithSize(HashTable *self, int size)
{
	/* Initialize */
	self->size = size < 10 ? 10 : size;
	self->case_sensitive = 1;
	self->array = xcalloc(self->size, sizeof(void *));
}


void HashTableCreate(HashTable *self)
{
	HashTableCreateWithSize(self, 0);
}


void HashTableDestroy(HashTable *self)
{
	HashTableClear(self);
	free(self->array);
}


void HashTableClear(HashTable *self)
{
	struct hash_table_elem_t *elem;
	struct hash_table_elem_t *elem_next;

	int i;

	/* Free elements */
	for (i = 0; i < self->size; i++)
	{
		while ((elem = self->array[i]))
		{
			elem_next = elem->next;
			hash_table_elem_free(elem);
			self->array[i] = elem_next;
		}
	}

	/* Reset count */
	self->count = 0;
	self->error = HashTableErrOK;
}


void HashTableSetCaseSensitive(HashTable *self, int case_sensitive)
{
	/* Only allowed if hash table is empty */
	if (self->count)
		panic("%s: only allowed for empty hash table",
				__FUNCTION__);
	
	/* Set new value */
	self->case_sensitive = case_sensitive;
	self->error = HashTableErrOK;
}


Object *HashTableInsert(HashTable *self, Object *key, Object *data)
{
	struct hash_table_elem_t *elem;
	int index;

	/* Check valid key */
	HashTableValidateKey(self, key);

	/* Rehashing */
	if (self->count >= self->size / 2)
		HashTableGrow(self);

	/* Element must not exists */
	elem = HashTableFind(self, key, &index);
	if (elem)
	{
		self->error = HashTableErrDuplicate;
		return NULL;
	}

	/* Create element and insert at the head of collision list */
	elem = hash_table_elem_create(key, data);
	elem->next = self->array[index];
	self->array[index] = elem;

	/* One more element */
	self->count++;
	assert(self->count < self->size);

	/* Success */
	self->error = HashTableErrOK;
	return data;
}


Object *HashTableSet(HashTable *self, Object *key, Object *data)
{
	struct hash_table_elem_t *elem;

	/* Check valid key */
	HashTableValidateKey(self, key);

	/* Find element */
	elem = HashTableFind(self, key, NULL);
	if (!elem)
	{
		self->error = HashTableErrNotFound;
		return NULL;
	}

	/* Set new data, success */
	elem->data = data;
	self->error = HashTableErrOK;
	return data;
}


Object *HashTableGet(HashTable *self, Object *key)
{
	struct hash_table_elem_t *elem;

	/* Check valid key */
	HashTableValidateKey(self, key);

	/* Find element */
	elem = HashTableFind(self, key, NULL);
	if (!elem)
	{
		self->error = HashTableErrNotFound;
		return NULL;
	}

	/* Return data */
	self->error = HashTableErrOK;
	return elem->data;
}


Object *HashTableRemove(HashTable *self, Object *key)
{
	struct hash_table_elem_t *elem;
	struct hash_table_elem_t *elem_prev;

	int index;
	Object *data;

	/* Check valid key */
	HashTableValidateKey(self, key);

	/* Find element */
	index = key->Hash(key) % self->size;
	elem_prev = NULL;
	for (elem = self->array[index]; elem; elem = elem->next)
	{
		/* Check if it is this element */
		if (!key->Compare(key, elem->key))
			break;

		/* Record previous element */
		elem_prev = elem;
	}

	/* Element not found */
	if (!elem)
	{
		self->error = HashTableErrNotFound;
		return NULL;
	}

	/* Delete element from bucket list */
	if (elem_prev)
		elem_prev->next = elem->next;
	else
		self->array[index] = elem->next;

	/* Free element */
	data = elem->data;
	hash_table_elem_free(elem);

	/* One less element */
	assert(self->count > 0);
	self->count--;

	/* Return removed object */
	self->error = HashTableErrOK;
	return data;
}

