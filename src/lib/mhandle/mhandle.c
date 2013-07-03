/*
 *  Multi2Sim's Memory Management Library
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Initial size for hash table */
#define MHANDLE_HASH_TABLE_SIZE  1000

/* Corruption detection extra bytes */
#define MHANDLE_MARK_END		0xa5
#define MHANDLE_MARK_START		0x5a
#define MHANDLE_CORRUPT_RANGE		2
#define MHANDLE_CORRUPT_TOTAL		(2 * MHANDLE_CORRUPT_RANGE)


struct mhandle_item_t
{
	/* item fields */
	void *ptr;
	unsigned long size;
	char *at;
	
	/* hash table fields */
	unsigned int removed : 1;
	unsigned int active : 1;
	unsigned int corrupt_info : 1;
};


static int mhandle_initialized = 0;
static unsigned long mhandle_mem_used = 0;

/* Hash table of current allocated pointers */
static struct mhandle_item_t *mhandle_hash_table;  /* Linear hash table */
static int mhandle_hash_table_size;  /* Allocated size for hash table */
static int mhandle_hash_table_count;  /* Number of elements */

/* Forward declarations */
static void mhandle_hash_table_insert(void *ptr, unsigned long size, char *at, int corrupt_info);




/*
 * Private Functions
 */

static void mhandle_out_of_memory(char *at)
{
	fprintf(stderr, "\nfatal: %s: out of memory\n", at);
	abort();
}


static void mhandle_init(void)
{
	/* Already initialized */
	if (mhandle_initialized)
		return;
	mhandle_initialized = 1;
	
	/* Initialize hash table */
	mhandle_hash_table = calloc(MHANDLE_HASH_TABLE_SIZE, sizeof(struct mhandle_item_t));
	mhandle_hash_table_size = MHANDLE_HASH_TABLE_SIZE;
	mhandle_hash_table_count = 0;
	if (!mhandle_hash_table)
		mhandle_out_of_memory("mhandle_init");
}


static void mhandle_hash_table_grow(void)
{
	int old_size, i;
	struct mhandle_item_t *old_ht;
	
	/* create new hash table */
	old_size = mhandle_hash_table_size;
	old_ht = mhandle_hash_table;
	mhandle_hash_table_size = mhandle_hash_table_size * 2;
	mhandle_hash_table = (struct mhandle_item_t *) calloc(mhandle_hash_table_size, sizeof(struct mhandle_item_t));
	if (!mhandle_hash_table)
		mhandle_out_of_memory("lib mhandle (resizing hash table)");
	
	/* put elements into new hash table */
	for (i = 0; i < old_size; i++)
	{
		if (old_ht[i].active && !old_ht[i].removed)
		{
			mhandle_hash_table_count--;
			mhandle_mem_used -= old_ht[i].size;
			mhandle_hash_table_insert(old_ht[i].ptr, old_ht[i].size,
				old_ht[i].at, old_ht[i].corrupt_info);
		}
	}
	free(old_ht);
}


static void mhandle_hash_table_insert(void *ptr, unsigned long size, char *at, int corrupt_info)
{
	struct mhandle_item_t *item;
	int index;
	
	/* Hash table too small */
	if (mhandle_hash_table_count >= mhandle_hash_table_size / 2)
		mhandle_hash_table_grow();
	
	/* Find position */
	index = (unsigned long) ptr % mhandle_hash_table_size;
	while (mhandle_hash_table[index].active && !mhandle_hash_table[index].removed)
		index = (index + 1) % mhandle_hash_table_size;
	
	/* Insert item */
	item = &mhandle_hash_table[index];
	item->ptr = ptr;
	item->size = size;
	item->at = at;
	item->active = 1;
	item->removed = 0;
	item->corrupt_info = corrupt_info;
	mhandle_mem_used += size;
	mhandle_hash_table_count++;
}


static struct mhandle_item_t *mhandle_hash_table_get(void *ptr)
{
	int idx;

	idx = (unsigned long) ptr % mhandle_hash_table_size;
	while (mhandle_hash_table[idx].ptr != ptr || !mhandle_hash_table[idx].active
			|| mhandle_hash_table[idx].removed)
	{
		if (!mhandle_hash_table[idx].active)
			return NULL;
		idx = (idx + 1) % mhandle_hash_table_size;
	}
	return &mhandle_hash_table[idx];
}


static unsigned long mhandle_hash_table_remove(void *ptr, char *at)
{
	struct mhandle_item_t *item;

	/* Find pointer */
	item = mhandle_hash_table_get(ptr);
	if (!item)
	{
		fprintf(stderr, "\n%s: free: invalid pointer %p\n",
			at, ptr);
		abort();
	}
	
	/* Remove item */
	item->removed = 1;
	mhandle_mem_used -= item->size;
	mhandle_hash_table_count--;
	return item->size;
}


static void mhandle_mark_corrupt(void *eff_ptr, unsigned long eff_size)
{
	unsigned long i;
	char *as_char = eff_ptr;

	for (i = 0; i < MHANDLE_CORRUPT_RANGE; i++)
		as_char[i] = MHANDLE_MARK_START;
	for (i = 0; i < MHANDLE_CORRUPT_RANGE; i++)
		as_char[eff_size - MHANDLE_CORRUPT_RANGE + i] = MHANDLE_MARK_END;
}


static void mhandle_check_corrupt(void *eff_ptr, unsigned long eff_size, char *at)
{
	unsigned long i;
	unsigned long size;
	unsigned char *as_char;

	int corrupt = 0;
	int prev;
	int next;

	void *ptr;

	/* Calculate original pointer and size */
	ptr = eff_ptr + MHANDLE_CORRUPT_RANGE;
	size = eff_size - MHANDLE_CORRUPT_TOTAL;
	as_char = eff_ptr;

	/* Corruption before & after block */
	assert(size > 0);
	for (i = 0; i < MHANDLE_CORRUPT_RANGE; i++)
		if (as_char[i] != MHANDLE_MARK_START)
			corrupt = 1;
	for (i = 0; i < MHANDLE_CORRUPT_RANGE; i++)
		if (as_char[eff_size - MHANDLE_CORRUPT_RANGE + i] != MHANDLE_MARK_END)
			corrupt = 2;
	if (!corrupt)
		return;
	
	/* Find contiguous blocks */
	prev = -1;
	next = -1;
	for (i = 0; i < mhandle_hash_table_size; i++)
	{
		if (mhandle_hash_table[i].active && !mhandle_hash_table[i].removed)
		{
			if (mhandle_hash_table[i].ptr < ptr && (prev == -1 ||
					mhandle_hash_table[i].ptr > mhandle_hash_table[prev].ptr))
				prev = i;
			if (mhandle_hash_table[i].ptr > ptr && (next == -1 ||
					mhandle_hash_table[i].ptr < mhandle_hash_table[next].ptr))
				next = i;
		}
	}
	
	/* Notify */
	fprintf(stderr, "\nfatal: %s: memory corrupted %s block (%p, %ld bytes)\n",
		at, corrupt == 1 ? "before" : "after", ptr, size);
	if (prev >= 0)
		fprintf(stderr, "\tprev block: %s (%p, %ld bytes)\n",
			mhandle_hash_table[prev].at,
			mhandle_hash_table[prev].ptr,
			mhandle_hash_table[prev].size);
	if (next >= 0)
	{
		fprintf(stderr, "\tnext block: %s (%p, %ld bytes)\n",
			mhandle_hash_table[next].at,
			mhandle_hash_table[next].ptr,
			mhandle_hash_table[next].size);
	}
	abort();
}




/*
 * Public Functions
 */
 
void mhandle_free(void *ptr, char *at)
{
	struct mhandle_item_t *item;
	void *eff_ptr;
	unsigned long eff_size;
	
	/* initialization */
	if (!ptr)
		return;
	mhandle_init();

	/* Read item */
	item = mhandle_hash_table_get(ptr);
	if (!item)
	{
		fprintf(stderr, "\n%s: free: invalid pointer %p\n",
			at, ptr);
		abort();
	}

	/* Check corruption */
	if (item->corrupt_info)
	{
		eff_ptr = ptr - MHANDLE_CORRUPT_RANGE;
		eff_size = item->size + MHANDLE_CORRUPT_TOTAL;
		mhandle_check_corrupt(eff_ptr, eff_size, item->at);
		memset(eff_ptr, 0, eff_size);
		free(eff_ptr);
	}
	else
	{
		memset(ptr, 0, item->size);
		free(ptr);
	}
	
	/* Remove pointer from data base */
	mhandle_hash_table_remove(ptr, at);
}


void *mhandle_malloc(unsigned long size, char *at)
{
	void *ptr;
	void *eff_ptr;
	unsigned long eff_size;
	
	mhandle_init();

	/* Allocate */
	eff_size = size + MHANDLE_CORRUPT_TOTAL;
	eff_ptr = malloc(eff_size);
	if (!eff_ptr)
		mhandle_out_of_memory(at);
	
	/* Mark corruption */
	ptr = eff_ptr + MHANDLE_CORRUPT_RANGE;
	mhandle_mark_corrupt(eff_ptr, eff_size);

	/* Record pointer and return */
	mhandle_hash_table_insert(ptr, size, at, 1);
	return ptr;
}


void *mhandle_calloc(unsigned long nmemb, unsigned long size, char *at)
{
	void *ptr;
	void *eff_ptr;

	unsigned long total;
	unsigned long eff_total;
	
	mhandle_init();

	/* Effective size */
	total = nmemb * size;
	eff_total = total + MHANDLE_CORRUPT_TOTAL;

	/* Allocate */
	eff_ptr = calloc(1, eff_total);
	if (!eff_ptr)
		mhandle_out_of_memory(at);

	/* Mark corruption */
	ptr = eff_ptr + MHANDLE_CORRUPT_RANGE;
	mhandle_mark_corrupt(eff_ptr, eff_total);

	/* Record pointer and return */
	mhandle_hash_table_insert(ptr, total, at, 1);
	return ptr;
}


void *mhandle_realloc(void *ptr, unsigned long size, char *at)
{
	void *eff_ptr;
	unsigned long eff_size;
	struct mhandle_item_t *item;

	/* Equivalent to malloc for NULL pointer */
	if (!ptr)
		return mhandle_malloc(size, at);

	/* Equivalent to free for size zero */
	if (!size)
	{
		mhandle_free(ptr, at);
		return NULL;
	}
	
	/* Reallocate */
	mhandle_init();

	/* Search pointer */
	item = mhandle_hash_table_get(ptr);
	if (!item)
	{
		fprintf(stderr, "\n%s: realloc: invalid pointer %p\n", at, ptr);
		abort();
	}

	/* Reallocation not supported for pointers registered externally, i.e.,
	 * those that don't have corruption information. */
	if (!item->corrupt_info)
	{
		fprintf(stderr, "\n%s: realloc: not supported for pointers not allocated\n"
			"\twith malloc/calloc/realloc/strdup (%p)", at, ptr);
		abort();
	}

	/* Effective sizes */
	eff_ptr = ptr - MHANDLE_CORRUPT_RANGE;
	eff_size = item->size + MHANDLE_CORRUPT_TOTAL;

	/* Check corruption and remove old pointer */
	mhandle_check_corrupt(eff_ptr, eff_size, at);
	mhandle_hash_table_remove(ptr, at);
	
	/* Reallocate */
	eff_size = size + MHANDLE_CORRUPT_TOTAL;
	eff_ptr = realloc(eff_ptr, eff_size);
	if (!eff_ptr)
		mhandle_out_of_memory(at);
	
	/* Mark corruption */
	ptr = eff_ptr + MHANDLE_CORRUPT_RANGE;
	mhandle_mark_corrupt(eff_ptr, eff_size);
	
	/* Record pointer and return */
	mhandle_hash_table_insert(ptr, size, at, 1);
	return ptr;
}


char *mhandle_strdup(const char *s, char *at)
{
	char *ptr;
	void *eff_ptr;

	unsigned long size = strlen(s) + 1;
	unsigned long eff_size;

	mhandle_init();

	/* Allocate */
	size = strlen(s) + 1;
	eff_size = size + MHANDLE_CORRUPT_TOTAL;
	eff_ptr = malloc(eff_size);
	if (!eff_ptr)
		mhandle_out_of_memory(at);
	
	/* Copy string and mark corruption */
	ptr = eff_ptr + MHANDLE_CORRUPT_RANGE;
	memcpy(ptr, s, size);
	mhandle_mark_corrupt(eff_ptr, eff_size);

	/* Record pointer and return */
	mhandle_hash_table_insert(ptr, size, at, 1);
	return ptr;
}


void *__xmalloc(size_t size, char *at)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
		mhandle_out_of_memory(at);

	return ptr;
}


void *__xcalloc(size_t nmemb, size_t size, char *at)
{
	void *ptr;

	ptr = calloc(nmemb, size);
	if (!ptr)
		mhandle_out_of_memory(at);

	return ptr;
}


void *__xrealloc(void *ptr, size_t size, char *at)
{
	void *new_ptr;

	new_ptr = realloc(ptr, size);
	if (!new_ptr)
		mhandle_out_of_memory(at);

	return new_ptr;
}


void *__xstrdup(const char *s, char *at)
{
	void *ptr;

	ptr = strdup(s);
	if (!ptr)
		mhandle_out_of_memory(at);

	return ptr;
}


void __mhandle_done()
{
	int i;
	
	/* Visit whole hash table to look for not freed pointers */
	for (i = 0; i < mhandle_hash_table_size; i++)
		if (mhandle_hash_table[i].active && !mhandle_hash_table[i].removed)
			fprintf(stderr, "\nwarning: %s: pointer not freed", mhandle_hash_table[i].at);
	
	/* Summary message */
	if (mhandle_hash_table_count)
		fprintf(stderr, "\n** %d pointers not freed (%lu bytes) **\n",
			mhandle_hash_table_count, mhandle_mem_used);
	
	/* Free hash table */
	assert(mhandle_hash_table_count || !mhandle_mem_used);
	free(mhandle_hash_table);
	mhandle_initialized = 0;
	mhandle_mem_used = 0;
	mhandle_hash_table_count = 0;
	mhandle_hash_table_size = 0;
}


void __mhandle_check(char *at)
{
	int i;
	int count = 0;

	void *eff_ptr;
	unsigned long eff_size;
	
	/* Check for corruption in all allocated blocks */
	for (i = 0; i < mhandle_hash_table_size; i++)
	{
		if (mhandle_hash_table[i].active && !mhandle_hash_table[i].removed
			&& mhandle_hash_table[i].corrupt_info)
		{
			eff_ptr = mhandle_hash_table[i].ptr - MHANDLE_CORRUPT_RANGE;
			eff_size = mhandle_hash_table[i].size + MHANDLE_CORRUPT_TOTAL;
			mhandle_check_corrupt(eff_ptr, eff_size, mhandle_hash_table[i].at);
			count++;
		}
	}
	fprintf(stderr, "libmhandle: %d pointers checked for corruption\n", count);

}


unsigned long __mhandle_used_memory()
{
	return mhandle_mem_used;
}


void __mhandle_register_ptr(void *ptr, unsigned long size, char *at)
{
	mhandle_hash_table_insert(ptr, size, at, 0);
}

