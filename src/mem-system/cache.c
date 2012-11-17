/*
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

#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "cache.h"
#include "mem-system.h"
#include "prefetcher.h"


/*
 * Public Variables
 */

struct str_map_t cache_policy_map =
{
	3, {
		{ "LRU", cache_policy_lru },
		{ "FIFO", cache_policy_fifo },
		{ "Random", cache_policy_random }
	}
};

struct str_map_t cache_block_state_map =
{
	6, {
		{ "N", cache_block_noncoherent },
		{ "M", cache_block_modified },
		{ "O", cache_block_owned },
		{ "E", cache_block_exclusive },
		{ "S", cache_block_shared },
		{ "I", cache_block_invalid }
	}
};




/*
 * Private Functions
 */

enum cache_waylist_enum
{
	cache_waylist_head,
	cache_waylist_tail
};

static void cache_update_waylist(struct cache_set_t *set,
	struct cache_block_t *blk, enum cache_waylist_enum where)
{
	if (!blk->way_prev && !blk->way_next)
	{
		assert(set->way_head == blk && set->way_tail == blk);
		return;
		
	}
	else if (!blk->way_prev)
	{
		assert(set->way_head == blk && set->way_tail != blk);
		if (where == cache_waylist_head)
			return;
		set->way_head = blk->way_next;
		blk->way_next->way_prev = NULL;
		
	}
	else if (!blk->way_next)
	{
		assert(set->way_head != blk && set->way_tail == blk);
		if (where == cache_waylist_tail)
			return;
		set->way_tail = blk->way_prev;
		blk->way_prev->way_next = NULL;
		
	}
	else
	{
		assert(set->way_head != blk && set->way_tail != blk);
		blk->way_prev->way_next = blk->way_next;
		blk->way_next->way_prev = blk->way_prev;
	}

	if (where == cache_waylist_head)
	{
		blk->way_next = set->way_head;
		blk->way_prev = NULL;
		set->way_head->way_prev = blk;
		set->way_head = blk;
	}
	else
	{
		blk->way_prev = set->way_tail;
		blk->way_next = NULL;
		set->way_tail->way_next = blk;
		set->way_tail = blk;
	}
}





/*
 * Public Functions
 */


struct cache_t *cache_create(char *name, unsigned int num_sets, unsigned int block_size,
	unsigned int assoc, enum cache_policy_t policy)
{
	struct cache_t *cache;
	struct cache_block_t *block;
	unsigned int set, way;

	/* Initialize */
	cache = xcalloc(1, sizeof(struct cache_t));
	cache->name = xstrdup(name);
	cache->num_sets = num_sets;
	cache->block_size = block_size;
	cache->assoc = assoc;
	cache->policy = policy;

	/* Derived fields */
	assert(!(num_sets & (num_sets - 1)));
	assert(!(block_size & (block_size - 1)));
	assert(!(assoc & (assoc - 1)));
	cache->log_block_size = log_base2(block_size);
	cache->block_mask = block_size - 1;
	
	/* Initialize array of sets */
	cache->sets = xcalloc(num_sets, sizeof(struct cache_set_t));
	for (set = 0; set < num_sets; set++)
	{
		/* Initialize array of blocks */
		cache->sets[set].blocks = xcalloc(assoc, sizeof(struct cache_block_t));
		cache->sets[set].way_head = &cache->sets[set].blocks[0];
		cache->sets[set].way_tail = &cache->sets[set].blocks[assoc - 1];
		for (way = 0; way < assoc; way++)
		{
			block = &cache->sets[set].blocks[way];
			block->way = way;
			block->way_prev = way ? &cache->sets[set].blocks[way - 1] : NULL;
			block->way_next = way < assoc - 1 ? &cache->sets[set].blocks[way + 1] : NULL;
		}
	}
	
	/* Return it */
	return cache;
}


void cache_free(struct cache_t *cache)
{
	unsigned int set;

	for (set = 0; set < cache->num_sets; set++)
		free(cache->sets[set].blocks);
	free(cache->sets);
	free(cache->name);
	if (cache->prefetcher)
		prefetcher_free(cache->prefetcher);
	free(cache);
}


/* Return {set, tag, offset} for a given address */
void cache_decode_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, 
	unsigned int *offset_ptr)
{
	PTR_ASSIGN(set_ptr, (addr >> cache->log_block_size) % cache->num_sets);
	PTR_ASSIGN(tag_ptr, addr & ~cache->block_mask);
	PTR_ASSIGN(offset_ptr, addr & cache->block_mask);
}


/* Look for a block in the cache. If it is found and its state is other than 0,
 * the function returns 1 and the state and way of the block are also returned.
 * The set where the address would belong is returned anyways. */
int cache_find_block(struct cache_t *cache, unsigned int addr, int *set_ptr, int *way_ptr, 
	int *state_ptr)
{
	int set, tag, way;

	/* Locate block */
	tag = addr & ~cache->block_mask;
	set = (addr >> cache->log_block_size) % cache->num_sets;
	PTR_ASSIGN(set_ptr, set);
	PTR_ASSIGN(state_ptr, 0);  /* Invalid */
	for (way = 0; way < cache->assoc; way++)
		if (cache->sets[set].blocks[way].tag == tag && cache->sets[set].blocks[way].state)
			break;
	
	/* Block not found */
	if (way == cache->assoc)
		return 0;
	
	/* Block found */
	PTR_ASSIGN(way_ptr, way);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
	return 1;
}


/* Set the tag and state of a block.
 * If replacement policy is FIFO, update linked list in case a new
 * block is brought to cache, i.e., a new tag is set. */
void cache_set_block(struct cache_t *cache, int set, int way, int tag, int state)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	mem_trace("mem.set_block cache=\"%s\" set=%d way=%d tag=0x%x state=\"%s\"\n",
			cache->name, set, way, tag,
			str_map_value(&cache_block_state_map, state));

	if (cache->policy == cache_policy_fifo
		&& cache->sets[set].blocks[way].tag != tag)
		cache_update_waylist(&cache->sets[set],
			&cache->sets[set].blocks[way],
			cache_waylist_head);
	cache->sets[set].blocks[way].tag = tag;
	cache->sets[set].blocks[way].state = state;
}


void cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);
	PTR_ASSIGN(tag_ptr, cache->sets[set].blocks[way].tag);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
}


/* Update LRU counters, i.e., rearrange linked list in case
 * replacement policy is LRU. */
void cache_access_block(struct cache_t *cache, int set, int way)
{
	int move_to_head;
	
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	/* A block is moved to the head of the list for LRU policy.
	 * It will also be moved if it is its first access for FIFO policy, i.e., if the
	 * state of the block was invalid. */
	move_to_head = cache->policy == cache_policy_lru ||
		(cache->policy == cache_policy_fifo && !cache->sets[set].blocks[way].state);
	if (move_to_head && cache->sets[set].blocks[way].way_prev)
		cache_update_waylist(&cache->sets[set],
			&cache->sets[set].blocks[way],
			cache_waylist_head);
}


/* Return the way of the block to be replaced in a specific set,
 * depending on the replacement policy */
int cache_replace_block(struct cache_t *cache, int set)
{
	//struct cache_block_t *block;

	/* Try to find an invalid block. Do this in the LRU order, to avoid picking the
	 * MRU while its state has not changed to valid yet. */
	assert(set >= 0 && set < cache->num_sets);
	/*
	for (block = cache->sets[set].way_tail; block; block = block->way_prev)
		if (!block->state)
			return block->way;
	*/

	/* LRU and FIFO replacement: return block at the
	 * tail of the linked list */
	if (cache->policy == cache_policy_lru ||
		cache->policy == cache_policy_fifo)
	{
		int way = cache->sets[set].way_tail->way;
		cache_update_waylist(&cache->sets[set], cache->sets[set].way_tail, 
			cache_waylist_head);

		return way;
	}
	
	/* Random replacement */
	assert(cache->policy == cache_policy_random);
	return random() % cache->assoc;
}


void cache_set_transient_tag(struct cache_t *cache, int set, int way, int tag)
{
	struct cache_block_t *block;

	/* Set transient tag */
	block = &cache->sets[set].blocks[way];
	block->transient_tag = tag;
}

