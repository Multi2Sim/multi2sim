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

#ifndef LIB_CLASS_HASH_TABLE_H
#define LIB_CLASS_HASH_TABLE_H

#include "class.h"


/*
 * Class 'HashTable'
 */

typedef enum
{
	HashTableErrOK = 0,
	HashTableErrDuplicate,
	HashTableErrNotFound
} HashTableError;


CLASS_BEGIN(HashTable, Object)

	/* Number of elements inserted */
	int count;

	/* Number of entries allocated in 'array' */
	int size;

	/* Error code of last operation */
	HashTableError error;

	/* Case-insensitive comparison and hashing of keys, set to true by
	 * default. This flag should be modified only with a call to
	 * 'HashTableSetCaseSensitive'. It only has effect if the hash table
	 * keys are of type 'String'. */
	int case_sensitive;

	/* Array of elements */
	struct hash_table_elem_t **array;

CLASS_END(HashTable)


void HashTableCreate(HashTable *self);
void HashTableCreateWithSize(HashTable *self, int size);
void HashTableDestroy(HashTable *self);

/* Clear the content of the hash table. The error code is set to
 * 'HashTableErrOK'. */
void HashTableClear(HashTable *self);

/* Set a new value for the 'case_sensitive' flag. This function only has effect
 * if the keys used in the hash table are of type 'String'. The error code is
 * set to 'HashTableErrOK'. */
void HashTableSetCaseSensitive(HashTable *self, int case_sensitive);

/* Insert a new object in the hash table. If 'key' already exists in the table,
 * the error code is set to 'HashTableErrDuplicate' and NULL is returned.
 * Otherwise, object 'data' is returned. */
Object *HashTableInsert(HashTable *self, Object *key, Object *data);

/* Set an existing element in the hash table to a new value. If 'key' is not
 * present in the table, set the error code to 'HashTableErrNotFound' and return
 * NULL. Otherwise, return 'data'. */
Object *HashTableSet(HashTable *self, Object *key, Object *data);

/* Return the element associated with a key. If 'key' is not present in the
 * table, set the error code to 'HashTableErrNotFound' and return NULL. */
Object *HashTableGet(HashTable *self, Object *key);

/* Remove 'key' from the hash table and return the object associated with it. If
 * 'key' is not present in the table, set the error code to
 * 'HashTableErrNotFound' and return NULL. */
Object *HashTableRemove(HashTable *self, Object *key);

#endif

