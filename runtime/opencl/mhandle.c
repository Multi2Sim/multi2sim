/*
 *  Libmhandle
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

#define HT_INITIAL_SIZE		1000

/* Corruption detection extra bytes */
#define END_MARK		0xa5
#define START_MARK		0x5a
#define CORRUPT_RANGE		2
#define CORRUPT_TOTAL		(2 * CORRUPT_RANGE)


struct item_t
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


static int initialized = 0;
static unsigned long mem_busy = 0;

/* hash table of current allocated pointers */
static struct item_t *ht;		/* linear hash table */
static int ht_size = 0, ht_count = 0;	/* size & elements in the hash table */

/* headers */
static void ht_insert(void *ptr, unsigned long size, char *at, int corrupt_info);
static void check_corruption(void *eff_ptr, unsigned long eff_size, char *at);


static void outofmem(char *at)
{
	fprintf(stderr, "\nfatal: %s: out of memory\n", at);
	abort();
}


static void initialize()
{
	/* already initialized? */
	if (initialized)
		return;
	initialized = 1;
	
	/* initialize hash table */
	ht = (struct item_t *) calloc(HT_INITIAL_SIZE, sizeof(struct item_t));
	ht_size = HT_INITIAL_SIZE;
	ht_count = 0;
	if (!ht)
		outofmem("lib mhandle (initialize)");
}


static void ht_grow()
{
	int old_size, i;
	struct item_t *old_ht;
	
	/* create new hash table */
	old_size = ht_size;
	old_ht = ht;
	ht_size = ht_size * 2;
	ht = (struct item_t *) calloc(ht_size, sizeof(struct item_t));
	if (!ht)
		outofmem("lib mhandle (resizing hash table)");
	
	/* put elements into new hash table */
	for (i = 0; i < old_size; i++)
	{
		if (old_ht[i].active && !old_ht[i].removed)
		{
			ht_count--;
			mem_busy -= old_ht[i].size;
			ht_insert(old_ht[i].ptr, old_ht[i].size, old_ht[i].at, old_ht[i].corrupt_info);
		}
	}
	free(old_ht);
}


static void ht_insert(void *ptr, unsigned long size, char *at, int corrupt_info)
{
	int idx;
	
	/* ht too small? */
	if (ht_count >= ht_size / 2)
		ht_grow();
	
	/* find position */
	idx = (unsigned long) ptr % ht_size;
	while (ht[idx].active && !ht[idx].removed)
		idx = (idx + 1) % ht_size;
	
	/* insert item */
	ht[idx].ptr = ptr;
	ht[idx].size = size;
	ht[idx].at = at;
	ht[idx].active = 1;
	ht[idx].removed = 0;
	ht[idx].corrupt_info = corrupt_info;
	mem_busy += size;
	ht_count++;
}


static struct item_t *ht_get(void *ptr)
{
	int idx;

	idx = (unsigned long) ptr % ht_size;
	while (ht[idx].ptr != ptr || !ht[idx].active || ht[idx].removed)
	{
		if (!ht[idx].active)
			return NULL;
		idx = (idx + 1) % ht_size;
	}
	return &ht[idx];
}


static unsigned long ht_remove(void *ptr, char *at)
{
	struct item_t *item;

	/* Find pointer */
	item = ht_get(ptr);
	if (!item)
	{
		fprintf(stderr, "\n%s: free: invalid pointer %p\n",
			at, ptr);
		abort();
	}
	
	/* Remove item */
	item->removed = 1;
	mem_busy -= item->size;
	ht_count--;
	return item->size;
}


static void mark_corruption(void *eff_ptr, unsigned long eff_size)
{
	unsigned long i;
	char *as_char = eff_ptr;

	for (i = 0; i < CORRUPT_RANGE; i++)
		as_char[i] = START_MARK;
	for (i = 0; i < CORRUPT_RANGE; i++)
		as_char[eff_size - CORRUPT_RANGE + i] = END_MARK;
}


static void check_corruption(void *eff_ptr, unsigned long eff_size, char *at)
{
	unsigned long i;
	unsigned char *as_char = eff_ptr;

	int corrupt = 0;
	int prev;
	int next;

	void *ptr;

	/* Corruption before & after block */
	for (i = 0; i < CORRUPT_RANGE; i++)
		if (as_char[i] != START_MARK)
			corrupt = 1;
	for (i = 0; i < CORRUPT_RANGE; i++)
		if (as_char[eff_size - CORRUPT_RANGE + i] != END_MARK)
			corrupt = 2;
	if (!corrupt)
		return;
	
	/* Find contiguous blocks */
	prev = -1;
	next = -1;
	ptr = eff_ptr + CORRUPT_RANGE;
	for (i = 0; i < ht_size; i++)
	{
		if (ht[i].active && !ht[i].removed)
		{
			if (ht[i].ptr < ptr && (prev == -1 || ht[i].ptr > ht[prev].ptr))
				prev = i;
			if (ht[i].ptr > ptr && (next == -1 || ht[i].ptr < ht[next].ptr))
				next = i;
		}
	}
	
	/* Notify */
	fprintf(stderr, "\nfatal: %s: memory corrupted %s block (%p)\n",
		at, corrupt == 1 ? "before" : "after", ptr);
	if (prev >= 0)
		fprintf(stderr, "\tprev block: %s (%p)\n", ht[prev].at, ht[prev].ptr);
	if (next >= 0)
		fprintf(stderr, "\tnext block: %s (%p)\n", ht[next].at, ht[next].ptr);
	abort();
}



/*
 * Public Functions
 */
 
void mhandle_free(void *ptr, char *at)
{
	struct item_t *item;
	void *eff_ptr;
	unsigned long eff_size;
	
	/* initialization */
	if (!ptr)
		return;
	initialize();

	/* Read item */
	item = ht_get(ptr);
	if (!item)
	{
		fprintf(stderr, "\n%s: free: invalid pointer %p\n",
			at, ptr);
		abort();
	}

	/* Check corruption */
	if (item->corrupt_info)
	{
		eff_ptr = ptr - CORRUPT_RANGE;
		eff_size = item->size + CORRUPT_TOTAL;
		check_corruption(eff_ptr, eff_size, item->at);
		memset(eff_ptr, 0, eff_size);
		free(eff_ptr);
	}
	else
	{
		memset(ptr, 0, item->size);
		free(ptr);
	}
	
	/* Remove pointer from data base */
	ht_remove(ptr, at);
}


void *mhandle_malloc(unsigned long size, char *at)
{
	void *ptr;
	void *eff_ptr;
	unsigned long eff_size;
	
	initialize();

	/* Allocate */
	eff_size = size + CORRUPT_TOTAL;
	eff_ptr = malloc(eff_size);
	if (!eff_ptr)
		outofmem(at);
	
	/* Mark corruption */
	ptr = eff_ptr + CORRUPT_RANGE;
	mark_corruption(eff_ptr, eff_size);

	/* Record pointer and return */
	ht_insert(ptr, size, at, 1);
	return ptr;
}


void *mhandle_calloc(unsigned long nmemb, unsigned long size, char *at)
{
	void *ptr;
	void *eff_ptr;

	unsigned long total;
	unsigned long eff_total;
	
	initialize();

	/* Effective size */
	total = nmemb * size;
	eff_total = total + CORRUPT_TOTAL;

	/* Allocate */
	eff_ptr = calloc(1, eff_total);
	if (!eff_ptr)
		outofmem(at);

	/* Mark corruption */
	ptr = eff_ptr + CORRUPT_RANGE;
	mark_corruption(eff_ptr, eff_total);

	/* Record pointer and return */
	ht_insert(ptr, total, at, 1);
	return ptr;
}


void *mhandle_realloc(void *ptr, unsigned long size, char *at)
{
	void *eff_ptr;
	unsigned long eff_size;
	struct item_t *item;

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
	initialize();

	/* Search pointer */
	item = ht_get(ptr);
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
	eff_ptr = ptr - CORRUPT_RANGE;
	eff_size = item->size + CORRUPT_TOTAL;

	/* Check corruption and remove old pointer */
	check_corruption(eff_ptr, eff_size, at);
	ht_remove(ptr, at);
	
	/* Reallocate */
	eff_size = size + CORRUPT_TOTAL;
	eff_ptr = realloc(eff_ptr, eff_size);
	if (!eff_ptr)
		outofmem(at);
	
	/* Mark corruption */
	ptr = eff_ptr + CORRUPT_RANGE;
	mark_corruption(eff_ptr, eff_size);
	
	/* Record pointer and return */
	ht_insert(ptr, size, at, 1);
	return ptr;
}


char *mhandle_strdup(const char *s, char *at)
{
	char *ptr;
	void *eff_ptr;

	unsigned long size = strlen(s) + 1;
	unsigned long eff_size;
	
	initialize();

	/* Allocate */
	size = strlen(s) + 1;
	eff_size = size + CORRUPT_TOTAL;
	eff_ptr = malloc(eff_size);
	if (!eff_ptr)
		outofmem(at);
	
	/* Copy string and mark corruption */
	ptr = eff_ptr + CORRUPT_RANGE;
	memcpy(ptr, s, size);
	mark_corruption(eff_ptr, eff_size);

	/* Record pointer and return */
	ht_insert(ptr, size, at, 1);
	return ptr;
}


void *__xmalloc(size_t size, char *at)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
	{
		fprintf(stderr, "%s: out of memory", at);
		abort();
	}

	return ptr;
}


void *__xcalloc(size_t nmemb, size_t size, char *at)
{
	void *ptr;

	ptr = calloc(nmemb, size);
	if (!ptr)
	{
		fprintf(stderr, "%s: out of memory", at);
		abort();
	}

	return ptr;
}


void *__xrealloc(void *ptr, size_t size, char *at)
{
	void *new_ptr;

	new_ptr = realloc(ptr, size);
	if (!new_ptr)
	{
		fprintf(stderr, "%s: out of memory", at);
		abort();
	}

	return new_ptr;
}


void *__xstrdup(const char *s, char *at)
{
	void *ptr;

	ptr = strdup(s);
	if (!ptr)
	{
		fprintf(stderr, "%s: out of memory", at);
		abort();
	}

	return ptr;
}


void __mhandle_done()
{
	int i;
	
	/* visit whole hash table to look for not freed pointers */
	for (i = 0; i < ht_size; i++)
		if (ht[i].active && !ht[i].removed)
			fprintf(stderr, "\nwarning: %s: pointer not freed", ht[i].at);
	if (ht_count)
		fprintf(stderr, "\n** %d pointers not freed (%lu bytes) **\n", ht_count, mem_busy);
	else
		assert(!mem_busy);
	
	/* free hash table & freed list */
	free(ht);
	initialized = 0;
	mem_busy = 0;
	ht_count = ht_size = 0;
}


void __mhandle_check(char *at)
{
	int i, count = 0;
	
	/* Check for corruption in all allocated blocks */
	for (i = 0; i < ht_size; i++)
	{
		if (ht[i].active && !ht[i].removed && ht[i].corrupt_info)
		{
			check_corruption(ht[i].ptr - CORRUPT_RANGE, ht[i].size, ht[i].at);
			count++;
		}
	}
	fprintf(stderr, "libmhandle: %d pointers checked for corruption\n", count);

}


unsigned long __mhandle_used_memory()
{
	return mem_busy;
}


void __mhandle_register_ptr(void *ptr, unsigned long size, char *at)
{
	ht_insert(ptr, size, at, 0);
}

