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

	/* Class of table keys. This value is set after the first insertion in
	 * the table, and cleared when the last element is removed. Once a key
	 * is inserted, the following keys must be of the same class. */
	struct class_t *key_class;

	/* Array of elements */
	struct hash_table_elem_t **array;

	/* Call-back functions used to clone, compare, and hash keys. */
	Object *(*KeyClone)(Object *);
	int (*KeyCompare)(Object *, Object *);
	unsigned int (*KeyHash)(Object *);

CLASS_END(HashTable)

void HashTableCreate(HashTable *self);
void HashTableCreateWithSize(HashTable *self, int size);
void HashTableDestroy(HashTable *self);

/* Clear the content of the hash table. The error code is set to
 * 'HashTableErrOK'. Freeing the objects is a responsibility of the caller. */
void HashTableClear(HashTable *self);

/* Clear the content of the hash table and delete all objects by invoking their
 * destructors. The error code is set to 'HashTableErrOK'. */
void HashTableDeleteObjects(HashTable *self);

/* Set a new value for the 'case_sensitive' flag. This function only has effect
 * if the keys used in the hash table are of type 'String'. The error code is
 * set to 'HashTableErrOK'. */
void HashTableSetCaseSensitive(HashTable *self, int case_sensitive);

/* Insert a new object in the hash table. If 'key' already exists in the table,
 * the error code is set to 'HashTableErrDuplicate' and NULL is returned.
 * Otherwise, object 'data' is returned. Object 'key' is cloned internally, so
 * the caller can manipulate the original instance without affecting the
 * content of the table. Object 'data' is not cloned, so any change in the
 * caller will affect the instance kept in the hash table.
 * The second function covers the special common case where the key is a
 * string. */
Object *HashTableInsert(HashTable *self, Object *key, Object *data);
Object *HashTableInsertString(HashTable *self, const char *key, Object *data);

/* Set an existing element in the hash table to a new value. If 'key' is not
 * present in the table, set the error code to 'HashTableErrNotFound' and return
 * NULL. Otherwise, return 'data'. The second version of the function can be
 * used when the key is a string. */
Object *HashTableSet(HashTable *self, Object *key, Object *data);
Object *HashTableSetString(HashTable *self, const char *key, Object *data);

/* Return the element associated with a key. If 'key' is not present in the
 * table, set the error code to 'HashTableErrNotFound' and return NULL. The
 * second version of the function can be used when the key is a string. */
Object *HashTableGet(HashTable *self, Object *key);
Object *HashTableGetString(HashTable *self, const char *key);

/* Remove 'key' from the hash table and return the object associated with it. If
 * 'key' is not present in the table, set the error code to
 * 'HashTableErrNotFound' and return NULL. The second function dels with the
 * common case where the key is a string. */
Object *HashTableRemove(HashTable *self, Object *key);
Object *HashTableRemoveString(HashTable *self, const char *key);

#endif

