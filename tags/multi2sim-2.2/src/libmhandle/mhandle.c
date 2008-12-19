/*
 *  Libmhandle
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define HT_INITIAL_SIZE		1000
#define FREED_INITIAL_SIZE	1000

/* corruption detection extra bytes */
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

/* list of freed pointers */
static struct item_t *freed;
static int freed_count = 0, freed_size = 0;

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
	
	/* initialize freed list */
	freed = (struct item_t *) calloc(FREED_INITIAL_SIZE, sizeof(struct item_t));
	freed_size = FREED_INITIAL_SIZE;
	freed_count = 0;
	if (!freed)
		outofmem("lib mhandle (initialize)");
}


static void free_error(void *ptr, char *at)
{
	int i, found;
	
	/* look for 'ptr' in freed list */
	for (i = found = 0; i < freed_count && !found; i++)
		found = freed[i].ptr == ptr;
	if (found)
		fprintf(stderr, "\n%s: pointer freed twice\n", at);
	else
		fprintf(stderr, "\n%s: pointer freed without previous allocation\n", at);
	abort();
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
		if (!ht[idx].active)
			free_error(ptr, at);
		idx = (idx + 1) % ht_size;
	}
	
	/* add freed pointer to freed list */
	if (freed_count == freed_size) {
		freed_size *= 2;
		freed = (struct item_t *) realloc(freed,
			freed_size * sizeof(struct item_t));
		if (!freed)
			outofmem("lib mhandle (resizing freed list)");
	}
	freed[freed_count].at = at;
	freed[freed_count].ptr = ptr;
	freed[freed_count].size = ht[idx].size;
	freed_count++;
	
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
	int corrupt = 0, prev, next, fprev, fnext;
	
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
	prev = next = fprev = fnext = -1;
	for (i = 0; i < ht_size; i++) {
		if (ht[i].active && !ht[i].removed) {
			if (ht[i].ptr < ptr && (prev == -1 || ht[i].ptr > ht[prev].ptr))
				prev = i;
			if (ht[i].ptr > ptr && (next == -1 || ht[i].ptr < ht[next].ptr))
				next = i;
		}
	}
	for (i = 0; i < freed_count; i++) {
		if (freed[i].ptr < ptr && (fprev == -1 || freed[i].ptr > freed[fprev].ptr))
			fprev = i;
		if (freed[i].ptr > ptr && (fnext == -1 || freed[i].ptr < freed[fnext].ptr))
			fnext = i;
	}
	
	/* message */
	fprintf(stderr, "\nfatal: %s: memory corrupted %s block (%p)\n",
		at, corrupt == 1 ? "before" : "after", ptr);
	if (prev >= 0)
		fprintf(stderr, "\tprev block: %s (%p)\n", ht[prev].at, ht[prev].ptr);
	if (next >= 0)
		fprintf(stderr, "\tnext block: %s (%p)\n", ht[next].at, ht[next].ptr);
	if (fprev >= 0)
		fprintf(stderr, "\tfreed prev block: %s (%p)\n", freed[fprev].at, ht[fprev].ptr);
	if (fnext >= 0)
		fprintf(stderr, "\tfreed next block: %s (%p)\n", ht[fnext].at, ht[fnext].ptr);
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
	free(freed);
	initialized = 0;
	mem_busy = 0;
	ht_count = ht_size = 0;
	freed_count = freed_size = 0;
}


void __mhandle_check(char *at)
{
	int i;
	
	/* check for corruption in all allocated blocks */
	for (i = 0; i < ht_size; i++)
		if (ht[i].active && !ht[i].removed)
			check_corruption(ht[i].ptr, ht[i].size, ht[i].at);
}


unsigned long __mhandle_used_memory()
{
	return mem_busy;
}

