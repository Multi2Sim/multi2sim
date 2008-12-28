/*
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

#include "cachesystem.h"


struct string_map_t cache_policy_map = {
	3, {
		{ "LRU",        cache_policy_lru },
		{ "FIFO",       cache_policy_fifo },
		{ "Random",     cache_policy_random }
	}
};


int cache_log2(uint32_t x) {
	int result = 0;
	if (!x)
		abort();
	while (!(x & 1)) {
		x >>= 1;
		result++;
	}
	if (x != 1)
		abort();
	return result;
}


enum cache_waylist_enum {
	cache_waylist_head,
	cache_waylist_tail
};


static void cache_update_waylist(struct cache_set_t *set,
	struct cache_blk_t *blk, enum cache_waylist_enum where)
{
	if (!blk->way_prev && !blk->way_next) {
		assert(set->way_head == blk && set->way_tail == blk);
		return;
		
	} else if (!blk->way_prev) {
		assert(set->way_head == blk && set->way_tail != blk);
		if (where == cache_waylist_head)
			return;
		set->way_head = blk->way_next;
		blk->way_next->way_prev = NULL;
		
	} else if (!blk->way_next) {
		assert(set->way_head != blk && set->way_tail == blk);
		if (where == cache_waylist_tail)
			return;
		set->way_tail = blk->way_prev;
		blk->way_prev->way_next = NULL;
		
	} else {
		assert(set->way_head != blk && set->way_tail != blk);
		blk->way_prev->way_next = blk->way_next;
		blk->way_next->way_prev = blk->way_prev;
	}

	if (where == cache_waylist_head) {
		blk->way_next = set->way_head;
		blk->way_prev = NULL;
		set->way_head->way_prev = blk;
		set->way_head = blk;
	} else {
		blk->way_prev = set->way_tail;
		blk->way_next = NULL;
		set->way_tail->way_next = blk;
		set->way_tail = blk;
	}
}


struct cache_t *cache_create(uint32_t nsets, uint32_t bsize, uint32_t assoc,
	enum cache_policy_enum policy)
{
	struct cache_t *cache;
	struct cache_blk_t *blk;
	uint32_t set, way;

	/* Create cache */
	cache = calloc(1, sizeof(struct cache_t));
	cache->nsets = nsets;
	cache->bsize = bsize;
	cache->assoc = assoc;
	cache->policy = policy;

	/* Derivated fields */
	assert(!(nsets & (nsets - 1)));
	assert(!(bsize & (bsize - 1)));
	assert(!(assoc & (assoc - 1)));
	cache->logbsize = cache_log2(bsize);
	cache->bmask = bsize - 1;
	
	/* Create sets and blocks */
	cache->sets = calloc(nsets, sizeof(struct cache_set_t));
	for (set = 0; set < nsets; set++) {
		cache->sets[set].blks = calloc(assoc, sizeof(struct cache_blk_t));
		cache->sets[set].way_head = &cache->sets[set].blks[0];
		cache->sets[set].way_tail = &cache->sets[set].blks[assoc - 1];
		for (way = 0; way < assoc; way++) {
			blk = &cache->sets[set].blks[way];
			blk->way = way;
			blk->way_prev = way ? &cache->sets[set].blks[way - 1] : NULL;
			blk->way_next = way < assoc - 1 ? &cache->sets[set].blks[way + 1] : NULL;
		}
	}
	
	/* Return it */
	return cache;
}


void cache_free(struct cache_t *cache)
{
	uint32_t set;
	for (set = 0; set < cache->nsets; set++)
		free(cache->sets[set].blks);
	free(cache->sets);
	free(cache);
}


/* Return {set, tag, offset} for a given address */
void cache_decode_address(struct cache_t *cache, uint32_t addr,
	uint32_t *pset, uint32_t *ptag, uint32_t *poffset)
{
	PTR_ASSIGN(pset, (addr >> cache->logbsize) % cache->nsets);
	PTR_ASSIGN(ptag, addr & ~cache->bmask);
	PTR_ASSIGN(poffset, addr & cache->bmask);
}


/* Look for a block in the cache. If it is found and its state is other than 0,
 * the function returns 1 and the status and way of the block are also returned.
 * The set where the address would belong is returned anyways. */
int cache_find_block(struct cache_t *cache, uint32_t addr,
	uint32_t *pset, uint32_t *pway, int *pstatus)
{
	uint32_t set, tag, way;

	/* Locate block */
	tag = addr & ~cache->bmask;
	set = (addr >> cache->logbsize) % cache->nsets;
	PTR_ASSIGN(pset, set);
	PTR_ASSIGN(pstatus, moesi_status_invalid);
	for (way = 0; way < cache->assoc; way++)
		if (cache->sets[set].blks[way].tag == tag && cache->sets[set].blks[way].status)
			break;
	
	/* Block not found */
	if (way == cache->assoc)
		return 0;
	
	/* Block found */
	PTR_ASSIGN(pway, way);
	PTR_ASSIGN(pstatus, cache->sets[set].blks[way].status);
	return 1;
}


/* Set the tag and status of a block.
 * If replacement policy is FIFO, update linked list in case a new
 * block is brought to cache, i.e., a new tag is set. */
void cache_set_block(struct cache_t *cache, uint32_t set, uint32_t way,
	uint32_t tag, int status)
{
	assert(set >= 0 && set < cache->nsets);
	assert(way >= 0 && way < cache->assoc);
	assert(set == (tag >> cache->logbsize) % cache->nsets || !status);
	if (cache->policy == cache_policy_fifo
		&& cache->sets[set].blks[way].tag != tag)
		cache_update_waylist(&cache->sets[set],
			&cache->sets[set].blks[way],
			cache_waylist_head);
	cache->sets[set].blks[way].tag = tag;
	cache->sets[set].blks[way].status = status;
}


void cache_get_block(struct cache_t *cache, uint32_t set, uint32_t way,
	uint32_t *ptag, int *pstatus)
{
	assert(set >= 0 && set < cache->nsets);
	assert(way >= 0 && way < cache->assoc);
	PTR_ASSIGN(ptag, cache->sets[set].blks[way].tag);
	PTR_ASSIGN(pstatus, cache->sets[set].blks[way].status);
}


/* Update LRU counters, i.e., rearrange linked list in case
 * replacement policy is LRU. */
void cache_access_block(struct cache_t *cache, uint32_t set, uint32_t way)
{
	assert(set >= 0 && set < cache->nsets);
	assert(way >= 0 && way < cache->assoc);
	if (cache->policy == cache_policy_lru
		&& cache->sets[set].blks[way].way_prev)
		cache_update_waylist(&cache->sets[set],
			&cache->sets[set].blks[way],
			cache_waylist_head);
}


/* Return the way of the block to be replaced in a specific set,
 * depending on the replacement policy */
uint32_t cache_replace_block(struct cache_t *cache, uint32_t set)
{
	struct cache_blk_t *blk;
	uint32_t way;

	/* Try to find an invalid block */
	assert(set >= 0 && set < cache->nsets);
	for (way = 0; way < cache->assoc; way++) {
		blk = &cache->sets[set].blks[way];
		if (!blk->status)
			return way;
	}

	/* LRU and FIFO replacement: return block at the
	 * head of the linked list */
	if (cache->policy == cache_policy_lru ||
		cache->policy == cache_policy_fifo)
		return cache->sets[set].way_tail->way;
	
	/* Random replacement */
	assert(cache->policy == cache_policy_random);
	return random() % cache->assoc;
}


void cache_set_transient_tag(struct cache_t *cache, uint32_t set, uint32_t way, uint32_t tag)
{
	struct cache_blk_t *blk;
	blk = &cache->sets[set].blks[way];
	blk->transient_tag = tag;
}

