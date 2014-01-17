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
#include "string.h"


/*
 * Object 'hash_table_elem_t'
 */

struct hash_table_elem_t
{
	Object *key;
	Object *data;
	struct hash_table_elem_t *next;
};


static struct hash_table_elem_t *hash_table_elem_create(void)
{
	struct hash_table_elem_t *elem;

	/* Initialize */
	elem = xcalloc(1, sizeof(struct hash_table_elem_t));

	/* Return */
	return elem;
}


static void hash_table_elem_free(struct hash_table_elem_t *elem)
{
	delete(elem->key);
	free(elem);
}




/*
 * Class 'HashTable'
 */

static char *hash_table_err_virtual_functions =
	"\tA class used as a key for a hash table must override virtual\n"
	"\tfunctions 'Clone', 'Compare', and 'Hash'.\n";

static char *hash_table_err_key_class =
	"\tThe hash table accepts any class for its key. But once the first\n"
	"\tkey is inserted, all following keys must be of the same class.\n";

static char *hash_table_err_iter =
	"\tA hash table iterator is being used after an operation that\n"
	"\tChanged the hash table state. Please make sure that your code only\n"
	"\tuses iterators between insersion and deletions of elements.\n";


static struct hash_table_elem_t *HashTableFind(HashTable *self, Object *key,
		int *index_ptr)
{
	struct hash_table_elem_t *elem;
	int index;

	/* Check availability of call-backs */
	assert(self->key_class);
	assert(self->KeyClone);
	assert(self->KeyHash);
	assert(self->KeyCompare);

	/* Get index */
	index = self->KeyHash(key) % self->size;
	if (index_ptr)
		*index_ptr = index;

	/* Look for element */
	for (elem = self->array[index]; elem; elem = elem->next)
		if (!self->KeyCompare(key, elem->key))
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

	/* If we are growing, we must have some object in the table for sure.
	 * We rely on the key call-backs for re-hashing. */
	assert(self->key_class);
	assert(self->KeyClone);
	assert(self->KeyHash);
	assert(self->KeyCompare);

	/* Save old array */
	old_size = self->size;
	old_array = self->array;

	/* Allocate new array */
	self->version++;
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
			index = self->KeyHash(key) % self->size;
			elem->next = self->array[index];
			self->array[index] = elem;
		}
	}

	/* Free old vector */
	free(old_array);
}


void HashTableCreateWithSize(HashTable *self, int size)
{
	/* Initialize */
	self->version++;
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


void HashTableDump(Object *self, FILE *f)
{
	HashTable *hash_table = asHashTable(self);

	struct hash_table_elem_t *elem;

	int i;

	/* Free elements */
	for (i = 0; i < hash_table->size; i++)
	{
		for (elem = hash_table->array[i]; elem; elem = elem->next)
		{
			elem->key->Dump(elem->key, f);
			fprintf(f, " -> ");
			if (elem->data)
				elem->data->Dump(elem->data, f);
			else
				fprintf(f, "(nil)");
			fprintf(f, "\n");
		}
	}
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

	/* Reset call-backs and key class */
	self->key_class = NULL;
	self->KeyClone = NULL;
	self->KeyCompare = NULL;
	self->KeyHash = NULL;

	/* Reset count */
	self->version++;
	self->count = 0;
	self->error = HashTableErrOK;
}


void HashTableDeleteObjects(HashTable *self)
{
	struct hash_table_elem_t *elem;
	struct hash_table_elem_t *elem_next;

	int i;

	/* Free elements */
	for (i = 0; i < self->size; i++)
	{
		while ((elem = self->array[i]))
		{
			/* Delete object */
			if (elem->data)
				delete(elem->data);

			/* Free element */
			elem_next = elem->next;
			hash_table_elem_free(elem);
			self->array[i] = elem_next;
		}
	}

	/* Reset call-backs and key class */
	self->key_class = NULL;
	self->KeyClone = NULL;
	self->KeyCompare = NULL;
	self->KeyHash = NULL;

	/* Reset count */
	self->count = 0;
	self->version++;
	self->error = HashTableErrOK;
}


void HashTableSetCaseSensitive(HashTable *self, int case_sensitive)
{
	/* Change key call-backs if table is not empty */
	if (self->count)
	{
		if (self->key_class != &StringClass)
			panic("%s: only allowed for string hash tables",
					__FUNCTION__);

		/* Set new key call-backs */
		if (case_sensitive)
		{
			self->KeyCompare = StringCompare;
			self->KeyHash = StringHash;
		}
		else
		{
			self->KeyCompare = StringCompareCase;
			self->KeyHash = StringHashCase;
		}
	}
	
	/* Set new value */
	self->case_sensitive = case_sensitive;
	self->error = HashTableErrOK;
}


Object *HashTableInsert(HashTable *self, Object *key, Object *data)
{
	struct hash_table_elem_t *elem;
	int index;

#ifndef NDEBUG
	/* Check key class compatibility */
	if (self->key_class)
	{
		if (class_of(key) != self->key_class)
			panic("%s: key class mismatch ('%s' vs '%s').\n%s",
					__FUNCTION__, class_of(key)->name,
					self->key_class->name,
					hash_table_err_key_class);
	}
	else
	{
		if (!key->Clone || !key->Hash || !key->Compare)
		{
			fprintf(stderr, "key: ");
			ObjectDump(key, stderr);
			panic("%s: invalid key class '%s'.\n%s",
					__FUNCTION__, class_of(key)->name,
					hash_table_err_virtual_functions);
		}
	}
#endif

	/* Store key class */
	if (!self->key_class)
	{
		/* Set key call-backs */
		self->key_class = class_of(key);
		self->KeyClone = key->Clone;
		self->KeyCompare = key->Compare;
		self->KeyHash = key->Hash;

		/* For the particular case where the keys are of type 'String'
		 * and the hash table is characterized as case-insensitive, we
		 * override the hash and compare key call-backs. */
		if (!self->case_sensitive && isString(key))
		{
			self->KeyCompare = StringCompareCase;
			self->KeyHash = StringHashCase;
		}
	}
	
	/* Rehashing */
	if (self->count >= self->size / 2)
		HashTableGrow(self);

	/* Element must not exist */
	elem = HashTableFind(self, key, &index);
	if (elem)
	{
		self->error = HashTableErrDuplicate;
		return NULL;
	}

	/* Create element and insert at the head of collision list */
	elem = hash_table_elem_create();
	elem->key = self->KeyClone(key);
	elem->data = data;
	elem->next = self->array[index];
	self->array[index] = elem;

	/* One more element */
	self->count++;
	assert(self->count < self->size);

	/* Success */
	self->version++;
	self->error = HashTableErrOK;
	return data;
}


Object *HashTableInsertString(HashTable *self, const char *key, Object *data)
{
	Object *object;
	String string;

	new_static(&string, String, key);
	object = HashTableInsert(self, asObject(&string), data);
	delete_static(&string);

	return object;
}


Object *HashTableSet(HashTable *self, Object *key, Object *data)
{
	struct hash_table_elem_t *elem;

	/* If table is empty, don't even try to find it, since the key
	 * call-backs are not available. */
	if (!self->count)
	{
		self->error = HashTableErrNotFound;
		return NULL;
	}

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


Object *HashTableSetString(HashTable *self, const char *key, Object *data)
{
	String string;
	Object *object;

	new_static(&string, String, key);
	object = HashTableSet(self, asObject(&string), data);
	delete_static(&string);

	return object;
}


Object *HashTableGet(HashTable *self, Object *key)
{
	struct hash_table_elem_t *elem;

	/* If table is empty, don't even try to find it, since the key
	 * call-backs are not available. */
	if (!self->count)
	{
		self->error = HashTableErrNotFound;
		return NULL;
	}

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


Object *HashTableGetString(HashTable *self, const char *key)
{
	String string;
	Object *object;

	new_static(&string, String, key);
	object = HashTableGet(self, asObject(&string));
	delete_static(&string);

	return object;
}


Object *HashTableRemove(HashTable *self, Object *key)
{
	struct hash_table_elem_t *elem;
	struct hash_table_elem_t *elem_prev;

	int index;
	Object *data;

	/* Table is empty. Don't even try to find the element, since the key
	 * call-backs are not available. */
	if (!self->count)
	{
		self->error = HashTableErrNotFound;
		return NULL;
	}

	/* Find element */
	index = self->KeyHash(key) % self->size;
	elem_prev = NULL;
	for (elem = self->array[index]; elem; elem = elem->next)
	{
		/* Check if it is this element */
		if (!self->KeyCompare(key, elem->key))
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
	self->version++;

	/* Reset key class if hash table is empty */
	if (!self->count)
	{
		self->key_class = NULL;
		self->KeyClone = NULL;
		self->KeyHash = NULL;
		self->KeyCompare = NULL;
	}

	/* Return removed object */
	self->error = HashTableErrOK;
	return data;
}


Object *HashTableRemoveString(HashTable *self, const char *key)
{
	String string;
	Object *object;

	new_static(&string, String, key);
	object = HashTableRemove(self, asObject(&string));
	delete_static(&string);

	return object;
}


Object *HashTableFirst(HashTable *self, Object **data_ptr)
{
	struct hash_table_elem_t *elem;
	int index;

	/* Update iterator version */
	self->error = HashTableErrOK;
	self->iter_version = self->version;
	self->iter_index = 0;
	self->iter_elem = NULL;
	if (data_ptr)
		*data_ptr = NULL;

	/* Table is empty */
	if (!self->count)
	{
		self->error = HashTableErrEmpty;
		return NULL;
	}

	/* Find first element */
	for (index = 0; index < self->size; index++)
	{
		elem = self->array[index];
		if (elem)
		{
			self->iter_index = index;
			self->iter_elem = elem;
			if (data_ptr)
				*data_ptr = elem->data;
			return elem->key;
		}
	}

	/* Never get here */
	panic("%s: inconsistent hash table", __FUNCTION__);
	return NULL;
}


Object *HashTableNext(HashTable *self, Object **data_ptr)
{
	struct hash_table_elem_t *elem;
	int index;

	/* Check iterator version */
	if (self->iter_version != self->version)
		panic("%s: obsolete iterator\n%s", __FUNCTION__,
				hash_table_err_iter);

	/* End of enumeration reached in previous calls */
	if (!self->iter_elem)
	{
		self->error = HashTableErrEnd;
		return NULL;
	}

	/* Valid operation */
	self->error = HashTableErrOK;

	/* Continue enumeration in collision list */
	elem = self->iter_elem->next;
	if (elem)
	{
		self->iter_elem = elem;
		if (data_ptr)
			*data_ptr = elem->data;
		return elem->key;
	}

	/* Continue enumeration in vector */
	self->iter_index++;
	for (index = self->iter_index; index < self->size; index++)
	{
		elem = self->array[index];
		if (elem)
		{
			self->iter_index = index;
			self->iter_elem = elem;
			if (data_ptr)
				*data_ptr = elem->data;
			return elem->key;
		}
	}

	/* End of enumeration */
	self->error = HashTableErrEnd;
	self->iter_index = 0;
	self->iter_elem = NULL;
	if (data_ptr)
		*data_ptr = NULL;
	return NULL;
}


int HashTableIsEnd(HashTable *self)
{
	/* Check iterator version */
	if (self->iter_version != self->version)
		panic("%s: obsolete iterator\n%s", __FUNCTION__,
				hash_table_err_iter);

	/* End of hash table found */
	if (!self->iter_elem)
	{
		self->error = HashTableErrEnd;
		return 1;
	}

	/* Not the end */
	self->error = HashTableErrOK;
	return 0;
}



/*
 * Class 'HashTableIterator'
 */


void HashTableIteratorCreate(HashTableIterator *self, HashTable *table)
{
	/* Initialize */
	self->table = table;
	
	/* Initially, set the iterator's version number to a state prior to the
	 * current hash table's state in order to invalidate it until the first
	 * call to 'HashTableIteratorFirst' is performed. */
	self->version = table->version - 1;
}


void HashTableIteratorDestroy(HashTableIterator *self)
{
}


Object *HashTableIteratorFirst(HashTableIterator *self, Object **data_ptr)
{
	HashTable *table = self->table;
	struct hash_table_elem_t *elem;
	int index;

	/* Update iterator version */
	self->error = HashTableErrOK;
	self->version = table->version;
	self->index = 0;
	self->elem = NULL;
	if (data_ptr)
		*data_ptr = NULL;

	/* Table is empty */
	if (!table->count)
	{
		self->error = HashTableErrEmpty;
		return NULL;
	}

	/* Find first element */
	for (index = 0; index < table->size; index++)
	{
		elem = table->array[index];
		if (elem)
		{
			self->index = index;
			self->elem = elem;
			if (data_ptr)
				*data_ptr = elem->data;
			return elem->key;
		}
	}

	/* Never get here */
	panic("%s: inconsistent hash table", __FUNCTION__);
	return NULL;
}


Object *HashTableIteratorNext(HashTableIterator *self, Object **data_ptr)
{
	HashTable *table = self->table;
	struct hash_table_elem_t *elem;
	int index;

	/* Check iterator version */
	if (self->version != table->version)
		panic("%s: obsolete iterator\n%s", __FUNCTION__,
				hash_table_err_iter);

	/* End of enumeration reached in previous calls */
	if (!self->elem)
	{
		self->error = HashTableErrEnd;
		return NULL;
	}

	/* Valid operation */
	self->error = HashTableErrOK;

	/* Continue enumeration in collision list */
	elem = self->elem->next;
	if (elem)
	{
		self->elem = elem;
		if (data_ptr)
			*data_ptr = elem->data;
		return elem->key;
	}

	/* Continue enumeration in vector */
	self->index++;
	for (index = self->index; index < table->size; index++)
	{
		elem = table->array[index];
		if (elem)
		{
			self->index = index;
			self->elem = elem;
			if (data_ptr)
				*data_ptr = elem->data;
			return elem->key;
		}
	}

	/* End of enumeration */
	self->error = HashTableErrEnd;
	self->index = 0;
	self->elem = NULL;
	if (data_ptr)
		*data_ptr = NULL;
	return NULL;
}


int HashTableIteratorIsEnd(HashTableIterator *self)
{
	HashTable *table = self->table;

	/* Check iterator version */
	if (self->version != table->version)
		panic("%s: obsolete iterator\n%s", __FUNCTION__,
				hash_table_err_iter);

	/* End of hash table found */
	if (!self->elem)
	{
		self->error = HashTableErrEnd;
		return 1;
	}

	/* Not the end */
	self->error = HashTableErrOK;
	return 0;
}

