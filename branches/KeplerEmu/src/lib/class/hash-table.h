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
	HashTableErrNotFound,
	HashTableErrEmpty,
	HashTableErrEnd
} HashTableError;


#define HashTableForEach(_table, _key, _class) \
	for ((_key) = as##_class(HashTableFirst((_table), NULL)); \
		!HashTableIsEnd((_table)); \
		(_key) = as##_class(HashTableNext((_table), NULL)))


CLASS_BEGIN(HashTable, Object)

	/* Number of elements inserted */
	int count;

	/* Number of entries allocated in 'array' */
	int size;

	/* Error code of last operation */
	HashTableError error;

	/* Hash table state version number. Every time an operation is run that
	 * modifies the table state, this variable is incremented. Iterators use
	 * this version number to invalidate themselves upon a version mismatch.
	 */
	int version;

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

	/* Internal iterator */
	int iter_version;
	int iter_index;
	struct hash_table_elem_t *iter_elem;

	/* Call-back functions used to clone, compare, and hash keys. */
	Object *(*KeyClone)(Object *);
	int (*KeyCompare)(Object *, Object *);
	unsigned int (*KeyHash)(Object *);

CLASS_END(HashTable)

void HashTableCreate(HashTable *self);
void HashTableCreateWithSize(HashTable *self, int size);
void HashTableDestroy(HashTable *self);

void HashTableDump(Object *self, FILE *f);

/* Clear the content of the hash table. The error code is set to
 * 'HashTableErrOK'. Freeing the objects is a responsibility of the caller.
 * All iterators related with the hash table are invalidated after this
 * operation. */
void HashTableClear(HashTable *self);

/* Clear the content of the hash table and delete all objects by invoking their
 * destructors. The error code is set to 'HashTableErrOK'.
 * All iterators related with the hash table are invalidated after this
 * operations. */
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
 * string.
 * All iterators associated with the hash table are invalidated after this
 * operation. */
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
 * common case where the key is a string.
 * All iterators associated with the hash table are invalidated after this
 * operation. */
Object *HashTableRemove(HashTable *self, Object *key);
Object *HashTableRemoveString(HashTable *self, const char *key);

/* Place the internal iterator at the first element of the hash table, and
 * return the key stored in it. The data for that element is optionally
 * returned in 'data_ptr' if not NULL. If the table is empty, the internal error
 * code is set to 'HashTableErrEmpty' and NULL is returned. */
Object *HashTableFirst(HashTable *self, Object **data_ptr);

/* Move the internal iterator one position forward, and return the key
 * stored at that position. The data for that element is optionally returned in
 * 'data_ptr' if not NULL. If the iterator went past the last element, set the
 * error code to 'HashTableErrEnd' and return NULL. */
Object *HashTableNext(HashTable *self, Object **data_ptr);

/* Return true if the internal iterator points past the last element of the
 * table. If so, the error code is set to 'HashTableErrEnd'. */
int HashTableIsEnd(HashTable *self);



/*
 * Class 'HashTableIterator'
 */

#define HashTableIteratorForEach(_iter, _key, _class) \
	for ((_key) = as##_class(HashTableIteratorFirst((_iter), NULL)); \
		!HashTableIteratorIsEnd((_iter)); \
		(_key) = as##_class(HashTableIteratorNext((_iter), NULL)))

CLASS_BEGIN(HashTableIterator, Object)

	/* Hash table associated */
	HashTable *table;

	/* State version number of the hash table for which this is a valid
	 * iterator. The iterator is invalidated (and shouldn't be further used)
	 * whenever any operation is done on the hash table that changes its
	 * state. */
	int version;

	/* Error code for iterator */
	HashTableError error;

	/* Iterator position, given by a position in the hash table array and
	 * the hash table element in the bucket list at that position. */
	int index;
	struct hash_table_elem_t *elem;

CLASS_END(HashTableIterator)


/* Create an iterator associated with the hash table. The iterator is not valid
 * until a call to 'HashTableIteratorFirst' is performed. */
void HashTableIteratorCreate(HashTableIterator *self, HashTable *table);

/* Destructor */
void HashTableIteratorDestroy(HashTableIterator *self);

/* Place the iterator at the first element of the hash table, and return the key
 * stored in it. The associated data is optionally returned in 'data_ptr' if not
 * NULL. If the hash table is empty, return NULL and set the iterator's error
 * code to 'HashTableErrEmpty'. */
Object *HashTableIteratorFirst(HashTableIterator *self, Object **data_ptr);

/* Move the iterator one position forward in the hash table, and return the key
 * stored at that position. The associated data is optionally returned in
 * 'data_ptr' if not NULL. If the iterator has moved past the last element in
 * the table, return NULL and set the iterator's error code to 'HashTableErrEnd'.
 */
Object *HashTableIteratorNext(HashTableIterator *self, Object **data_ptr);

/* Return true if the iterator points past the last element in the table. If so,
 * set the iterator's error code to 'HashTableErrEnd'. */
int HashTableIteratorIsEnd(HashTableIterator *self);

#endif

