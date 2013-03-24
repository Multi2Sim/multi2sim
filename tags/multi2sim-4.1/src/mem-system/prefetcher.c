/*
 *  Multi2Sim
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>

#include "mem-system.h"
#include "prefetcher.h"
#include "cache.h"
#include "mod-stack.h"


struct str_map_t prefetcher_type_map =
{
	2, {
		{ "GHB_PC_CS", prefetcher_type_ghb_pc_cs },
		{ "GHB_PC_DC", prefetcher_type_ghb_pc_dc },
	}
};

struct prefetcher_t *prefetcher_create(int prefetcher_ghb_size, int prefetcher_it_size,
				       int prefetcher_lookup_depth, enum prefetcher_type_t type)
{
	struct prefetcher_t *pref;

	/* Initialize */
	/* The global history buffer and index table cannot be 0
	 * if the prefetcher object is created. */
	assert(prefetcher_ghb_size >= 1 && prefetcher_it_size >= 1);
	pref = xcalloc(1, sizeof(struct prefetcher_t));
	pref->ghb_size = prefetcher_ghb_size;
	pref->it_size = prefetcher_it_size;
	pref->lookup_depth = prefetcher_lookup_depth;
	pref->type = type;
	pref->ghb = xcalloc(prefetcher_ghb_size, sizeof(struct prefetcher_ghb_t));
	pref->index_table = xcalloc(prefetcher_it_size, sizeof(struct prefetcher_it_t));
	pref->ghb_head = -1;

	/* Return */
	return pref;
}

void prefetcher_free(struct prefetcher_t *pref)
{
	free(pref->ghb);
	free(pref->index_table);
	free(pref);
}

static void get_it_index_tag(struct prefetcher_t *pref, struct mod_stack_t *stack, 
			     int *it_index, unsigned *tag)
{
	if (stack->client_info)
	{
		*it_index = stack->client_info->prefetcher_eip % pref->it_size;
		*tag = stack->client_info->prefetcher_eip;
	}
	else
	{
		*it_index = -1;
		*tag = 0;
	}
}

/* Returns it_index >= 0 if any valid update is made, negative otherwise. */
static int prefetcher_update_tables(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	struct prefetcher_t *pref = target_mod->cache->prefetcher;
	int ghb_index;
	unsigned int addr = stack->addr;
	int it_index, prev;
	unsigned it_tag;

	assert(pref);
    
	/* Get the index table index */
	get_it_index_tag(pref, stack, &it_index, &it_tag);

	if (it_index < 0)
		return -1;

	assert(it_index < pref->it_size);

	ghb_index = (++(pref->ghb_head)) % pref->ghb_size;

	/* Remove the current entry in ghb_index, if its valid */
	if (pref->ghb[ghb_index].addr > 0)
	{
		prev = pref->ghb[ghb_index].prev;

		/* The prev field may point to either index table or ghb. */
		if (pref->ghb[ghb_index].prev_it_ghb == prefetcher_ptr_ghb)
		{
			/* prev_it_gb == 0 implies the previous element is in the GHB */
			assert(prev >= 0 && prev < pref->ghb_size);
			assert(pref->ghb[prev].next == ghb_index);
			pref->ghb[prev].next = -1;
		}
		else
		{
			assert(pref->ghb[ghb_index].prev_it_ghb == prefetcher_ptr_it);

			/* The element in index_table may have been replaced since this
			 * entry was put into the ghb. */
			if (prev >= 0)
			{
				assert(pref->index_table[prev].ptr == ghb_index);
				pref->index_table[prev].ptr = -1;
			}
		}
	}

	pref->ghb[ghb_index].addr = 0;
	pref->ghb[ghb_index].next = -1;
	pref->ghb[ghb_index].prev = -1;

	if (pref->index_table[it_index].tag > 0)
	{
		/* Replace entry in index_table if necessary. */
		if (pref->index_table[it_index].tag != it_tag)
		{
			mem_debug("  %lld it_index = %d, old_tag = 0x%x, new_tag = 0x%x" 
				  "prefetcher: replace index_table entry\n", stack->id, 
				  it_index, pref->index_table[it_index].tag, it_tag);

			prev = pref->index_table[it_index].ptr;

			/* The element in the ghb may have gone out by now. */
			if (prev >= 0)
			{
				/* The element that this is pointing to must be pointing back. */
				assert(pref->ghb[prev].prev_it_ghb == prefetcher_ptr_it && 
				       pref->ghb[prev].prev == it_index);
				pref->ghb[prev].prev = -1;
			}

			pref->index_table[it_index].tag = 0;
			pref->index_table[it_index].ptr = -1;
		}
	}
	else
	{
		/* Just an initialization. Tag == 0 implies the entry has never been used. */
		pref->index_table[it_index].ptr = -1;
	}

	/* Add new element into ghb. */
	pref->ghb[ghb_index].addr = addr;
	pref->ghb[ghb_index].next = pref->index_table[it_index].ptr;
	if (pref->index_table[it_index].ptr >= 0)
	{
	    prev = pref->index_table[it_index].ptr;
	    assert(pref->ghb[prev].prev_it_ghb == prefetcher_ptr_it &&
		   pref->ghb[prev].prev == it_index);
	    pref->ghb[prev].prev_it_ghb = prefetcher_ptr_ghb;
	    pref->ghb[prev].prev = ghb_index;
	}
	pref->ghb[ghb_index].prev_it_ghb = prefetcher_ptr_it;
	pref->ghb[ghb_index].prev = it_index;

	/* Make the index table entries point to current ghb_index. */
	pref->index_table[it_index].tag = it_tag;
	pref->index_table[it_index].ptr = ghb_index;

	/* Update pref->ghb_head so that its in the range possible. */
	pref->ghb_head = ghb_index;

	return it_index;
}

static void prefetcher_do_prefetch(struct mod_t *mod, struct mod_stack_t *stack,
				   unsigned int prefetch_addr)
{
	int set1, tag1, set2, tag2;

	assert(prefetch_addr > 0);

	/* Predicted prefetch_addr can go horribly wrong
	 * sometimes. Since prefetches aren't supposed to
	 * cause any kind of faults/exceptions, return. */
	if (!mod_serves_address(mod, prefetch_addr))
	{
		mem_debug("  miss_addr 0x%x, prefetch_addr 0x%x, %s : illegal prefetch\n", stack->addr,
			  prefetch_addr, mod->name);
		return;
	}

	cache_decode_address(mod->cache, stack->addr, &set1, &tag1, NULL);
	cache_decode_address(mod->cache, prefetch_addr, &set2, &tag2, NULL);

	/* If the prefetch_addr is in the same block as the missed address
	 * there is no point in prefetching. One scenario where this may
	 * happen is when we see a stride smaller than block size because
	 * of an eviction between the two accesses. */
	if (set1 == set2 && tag1 == tag2)
		return;

	/* I'm not passing back the mod_client_info structure. If this needs to be 
	 * passed in the future, make sure a copy is made (since the one that is
	 * pointed to by stack->client_info may be freed early. */
	mem_debug("  miss_addr 0x%x, prefetch_addr 0x%x, %s : prefetcher\n", stack->addr,
		  prefetch_addr, mod->name);

	mod_access(mod, mod_access_prefetch, prefetch_addr, NULL, NULL, NULL, NULL);
}

/* This function implements the GHB based PC/CS prefetching as described in the
 * 2005 paper by Nesbit and Smith. The index table lookup is based on the PC
 * of the instruction causing the miss. The GHB entries are looked at for finding
 * constant stride accesses. Based on this, prefetching is done. */
static void prefetcher_ghb_pc_cs(struct mod_t *mod, struct mod_stack_t *stack, int it_index)
{
	struct prefetcher_t *pref;
	int chain, stride, i;
	unsigned int prev_addr, cur_addr, prefetch_addr = 0;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	pref = mod->cache->prefetcher;

	chain = pref->index_table[it_index].ptr;

	/* The lookup depth must be at least 2 - which essentially means
	 * two strides have been seen so far, prefetch for the next. 
	 * It doesn't really help to prefetch on a lookup of depth 1.
	 * It is too low an accuracy and leads to lot of illegal and
	 * redundant prefetches. Hence keeping the minimum at 2. */
	assert(pref->lookup_depth >= 2);

	/* The table should've been updated before calling this function. */
	assert(pref->ghb[chain].addr == stack->addr);

	/* If there's only one element in this linked list, nothing to do. */
	if (pref->ghb[chain].next == -1)
		return;

	prev_addr = pref->ghb[chain].addr;
	chain = pref->ghb[chain].next;
	cur_addr = pref->ghb[chain].addr;
	stride = prev_addr - cur_addr;

	for (i = 2; i <= pref->lookup_depth; i++)
	{
		prev_addr = cur_addr;
		chain = pref->ghb[chain].next;

		/* The linked list (history) is smaller than the lookup depth */
		if (chain == -1)
			break;

		cur_addr = pref->ghb[chain].addr;

		/* The stride changed, can't prefetch */
		if (stride != prev_addr - cur_addr)
			break;

		/* If this is the last iteration (we've seen as much history as
		 * the lookup depth specified), then do a prefetch. */
		if (i == pref->lookup_depth)
			prefetch_addr = stack->addr + stride;
	}

	if (prefetch_addr > 0)
		prefetcher_do_prefetch(mod, stack, prefetch_addr);
}

/* This function implements the GHB based PC/DC prefetching as described in the
 * 2005 paper by Nesbit and Smith. The index table lookup is based on the PC
 * of the instruction causing the miss. The last three accesses are looked at
 * to find the last two strides (deltas). The list is then looked up backwards
 * to see if this pair of strides occurred earlier, if yes, the next stride
 * is obtained from the history there. This stride decides the new prefetch_addr. */
static void prefetcher_ghb_pc_dc(struct mod_t *mod, struct mod_stack_t *stack, int it_index)
{
	struct prefetcher_t *pref;
	int chain, chain2, stride[PREFETCHER_LOOKUP_DEPTH_MAX], i, pref_stride;
	unsigned int prev_addr, cur_addr, prefetch_addr = 0;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	pref = mod->cache->prefetcher;

	chain = pref->index_table[it_index].ptr;

	/* The lookup depth must be at least 2 - which essentially means
	 * two strides have been seen so far, predict the next stride. */ 
	assert(pref->lookup_depth >= 2 && pref->lookup_depth <= PREFETCHER_LOOKUP_DEPTH_MAX);

	/* The table should've been updated before calling this function. */
	assert(pref->ghb[chain].addr == stack->addr);

	/* Collect "lookup_depth" number of strides (deltas).
	 * This doesn't really make sense for a depth > 2, but
	 * I'll just have the code here for generality. */
	for (i = 0; i < pref->lookup_depth; i++)
	{
		prev_addr = pref->ghb[chain].addr;
		chain = pref->ghb[chain].next;

		/* The chain isn't long enough */
		if (chain == -1)
			return;

		cur_addr = pref->ghb[chain].addr;
		stride[i] = prev_addr - cur_addr;
	}

	chain = pref->index_table[it_index].ptr;
	chain = pref->ghb[chain].next;
	assert(chain != -1);

	/* "chain" now points to the second element of the list.
	 * Try to match the stride array starting from here. */
	while (chain != -1)
	{
		/* This really doesn't look realistic to implement in 
		 * hardware. Too much time consuming I feel. */
		chain2 = chain;
		for (i = 0; i < pref->lookup_depth; i++)
		{
			prev_addr = pref->ghb[chain2].addr;
			chain2 = pref->ghb[chain2].next;

			/* The chain isn't long enough and we
			 * haven't found a match till now. */
			if (chain2 == -1)
				return;

			cur_addr = pref->ghb[chain2].addr;
			if (stride[i] != prev_addr - cur_addr)
				break;
		}

		/* If we traversed the above loop full, we have a match. */
		if (i == pref->lookup_depth)
		{
		    cur_addr = pref->ghb[chain].addr;
		    assert(pref->ghb[chain].prev != -1 && 
			   pref->ghb[chain].prev_it_ghb == prefetcher_ptr_ghb);
		    chain = pref->ghb[chain].prev;
		    prev_addr = pref->ghb[chain].addr;
		    pref_stride = prev_addr - cur_addr;
		    prefetch_addr = stack->addr + pref_stride;
		    break;
		}

		chain = pref->ghb[chain].next;
	}

	if (prefetch_addr > 0)
		prefetcher_do_prefetch(mod, stack, prefetch_addr);
}

void prefetcher_access_miss(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	int it_index;

	if (target_mod->kind != mod_kind_cache || !target_mod->cache->prefetcher)
		return;

	it_index = prefetcher_update_tables(stack, target_mod);

	if (it_index < 0)
		    return;

	if (target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_cs)
	{
		/* Perform ghb based PC/CS prefetching
		 * (Program Counter based index, Constant Stride) */
		prefetcher_ghb_pc_cs(target_mod, stack, it_index);
	}
	else
	{
		assert(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc);
		/* Perform ghb based PC/DC prefetching
		 * (Program Counter based index, Delta Correlation) */
		prefetcher_ghb_pc_dc(target_mod, stack, it_index);
	}
}

void prefetcher_access_hit(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	int it_index;

	if (target_mod->kind != mod_kind_cache || !target_mod->cache->prefetcher)
		return;

	if (mod_block_get_prefetched(target_mod, stack->addr))
	{
		/* This block was prefetched. Now it has a real access. For the purposes
		 * of the prefetcher heuristic, this is still a miss. Hence, update
		 * the prefetcher tables. */
		it_index = prefetcher_update_tables(stack, target_mod);

		/* Clear the prefetched flag since we have a real access now */
		mem_debug ("  addr 0x%x %s : clearing \"prefetched\" flag\n", 
			   stack->addr, target_mod->name);
		mod_block_set_prefetched(target_mod, stack->addr, 0);

		if (it_index < 0)
			return;

		if (target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_cs)
		{
			/* Perform ghb based PC/CS prefetching
			 * (Program Counter based index, Constant Stride) */
			prefetcher_ghb_pc_cs(target_mod, stack, it_index);
		}
		else
		{
			assert(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc);
			/* Perform ghb based PC/DC prefetching
			 * (Program Counter based index, Delta Correlation) */
			prefetcher_ghb_pc_dc(target_mod, stack, it_index);
		}
	}
}
