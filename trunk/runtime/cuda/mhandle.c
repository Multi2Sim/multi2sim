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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define HT_INITIAL_SIZE		1000

/* Corruption detection extra bytes */
#define END_MARK		0xa5
#define START_MARK		0x5a
#define CORRUPT_RANGE		2
#define CORRUPT_TOTAL		(2 * CORRUPT_RANGE)


struct item_t {

	/* item fields */
	void *ptr;
	unsigned long size;
	char *at;
	
	/* hash table fields */
	int removed : 1;
	int active : 1;
};


static int initialized = 0;
static unsigned long mem_busy = 0;

/* hash table of current allocated pointers */
static struct item_t *ht;		/* linear hash table */
static int ht_size = 0, ht_count = 0;	/* size & elements in the hash table */

/* headers */
static void ht_insert(void *ptr, unsigned long size, char *at);
static void check_corruption(void *ptr, unsigned long size, char *at);


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
	for (i = 0; i < old_size; i++) {
		if (old_ht[i].active && !old_ht[i].removed) {
			ht_count--;
			mem_busy -= old_ht[i].size;
			ht_insert(old_ht[i].ptr, old_ht[i].size, old_ht[i].at);
		}
	}
	free(old_ht);
}


static void ht_insert(void *ptr, unsigned long size, char *at)
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
	mem_busy += size;
	ht_count++;
}


static unsigned long ht_remove(void *ptr, char *at)
{
	int idx;
	
	/* find position */
	idx = (unsigned long) ptr % ht_size;
	while (ht[idx].ptr != ptr || !ht[idx].active || ht[idx].removed) {
		if (!ht[idx].active) {
			fprintf(stderr, "\n%s: free: invalid pointer %p\n",
				at, ptr);
			abort();
		}
		idx = (idx + 1) % ht_size;
	}
	
	/* check corruption */
	check_corruption(ptr, ht[idx].size, at);
	
	/* remove item */
	ht[idx].removed = 1;
	mem_busy -= ht[idx].size;
	ht_count--;
	return ht[idx].size;
}


static void mark_corruption(void *ptr, unsigned long size)
{
	unsigned long i;
	
	for (i = 0; i < CORRUPT_RANGE; i++)
		((unsigned char *) ptr)[i] = START_MARK;
	for (i = CORRUPT_RANGE + size; i < CORRUPT_TOTAL + size; i++)
		((unsigned char *) ptr)[i] = END_MARK;
}


static void check_corruption(void *ptr, unsigned long size, char *at)
{
	unsigned long i;
	int corrupt = 0, prev, next;
	
	/* corruption before & after block */
	for (i = 0; i < CORRUPT_RANGE; i++)
		if (((unsigned char *) ptr)[i] != START_MARK)
			corrupt = 1;
	for (i = CORRUPT_RANGE + size; i < CORRUPT_TOTAL + size; i++)
		if (((unsigned char *) ptr)[i] != END_MARK)
			corrupt = 2;
	if (!corrupt)
		return;
	
	/* find contiguous blocks */
	prev = next = -1;
	for (i = 0; i < ht_size; i++) {
		if (ht[i].active && !ht[i].removed) {
			if (ht[i].ptr < ptr && (prev == -1 || ht[i].ptr > ht[prev].ptr))
				prev = i;
			if (ht[i].ptr > ptr && (next == -1 || ht[i].ptr < ht[next].ptr))
				next = i;
		}
	}
	
	/* message */
	fprintf(stderr, "\nfatal: %s: memory corrupted %s block (%p)\n",
		at, corrupt == 1 ? "before" : "after", ptr);
	if (prev >= 0)
		fprintf(stderr, "\tprev block: %s (%p)\n", ht[prev].at, ht[prev].ptr);
	if (next >= 0)
		fprintf(stderr, "\tnext block: %s (%p)\n", ht[next].at, ht[next].ptr);
	abort();
}



/*
 * PUBLIC METHODS
 */
 
void mhandle_free(void *ptr, char *at)
{
	unsigned long size;
	
	/* initialization */
	if (!ptr)
		return;
	initialize();
	
	/* delete pointer from database & check corruption*/
	ptr -= CORRUPT_RANGE;
	size = ht_remove(ptr, at);
	
	/* clear memory & free pointer */
	bzero(ptr, size + CORRUPT_TOTAL);
	free(ptr);
}


void *mhandle_malloc(unsigned long size, char *at)
{
	void *ptr;
	
	initialize();
	ptr = malloc(size + CORRUPT_TOTAL);
	if (!ptr)
		outofmem(at);
	mark_corruption(ptr, size);
	ht_insert(ptr, size, at);

	return ptr + CORRUPT_RANGE;
}


void *mhandle_calloc(unsigned long nmemb, unsigned long size, char *at)
{
	void *ptr;
	unsigned long total = nmemb * size;
	
	initialize();
	ptr = calloc(1, total + CORRUPT_TOTAL);
	if (!ptr)
		outofmem(at);
	mark_corruption(ptr, total);
	ht_insert(ptr, total, at);
	
	return ptr + CORRUPT_RANGE;
}


void *mhandle_realloc(void *ptr, unsigned long size, char *at)
{
	/* equivalent to malloc or free*/
	if (!ptr)
		return mhandle_malloc(size, at);
	if (!size) {
		mhandle_free(ptr, at);
		return NULL;
	}
	
	/* realloc */
	initialize();
	ptr -= CORRUPT_RANGE;
	ht_remove(ptr, at);
	ptr = realloc(ptr, size + CORRUPT_TOTAL);
	if (!ptr)
		outofmem(at);
	mark_corruption(ptr, size);
	ht_insert(ptr, size, at);
	return ptr + CORRUPT_RANGE;
}


char *mhandle_strdup(const char *s, char *at)
{
	char *ptr;
	unsigned long size = strlen(s) + 1;
	
	initialize();
	ptr = malloc(size + CORRUPT_TOTAL);
	if (!ptr)
		outofmem(at);
	memcpy(ptr + CORRUPT_RANGE, s, size);
	mark_corruption(ptr, size);
	ht_insert(ptr, size, at);
	
	return ptr + CORRUPT_RANGE;
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
	
	/* check for corruption in all allocated blocks */
	for (i = 0; i < ht_size; i++) {
		if (ht[i].active && !ht[i].removed) {
			check_corruption(ht[i].ptr, ht[i].size, ht[i].at);
			count++;
		}
	}
	fprintf(stderr, "libmhandle: %d pointers checked for corruption\n", count);

}


unsigned long __mhandle_used_memory()
{
	return mem_busy;
}

