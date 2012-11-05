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
#include <stdlib.h>

#include <lib/util/debug.h>
#include <mem-system/mem-system.h>
/* Ugly */
#include <arch/x86/timing/uop.h>

#include "prefetcher.h"
#include "module.h"
#include "cache.h"
#include "nmoesi-protocol.h"
#include "mod-stack.h"


struct prefetcher_t *prefetcher_create(int prefetcher_ghb_size, int prefetcher_it_size)
{
	struct prefetcher_t *pref;

	/* Create object */
	pref = calloc(1, sizeof(struct prefetcher_t));
	if (!pref)
		fatal("%s: out of memroy", __FUNCTION__);

	/* Initialize */
	/* The global history buffer and index table cannot be 0
	 * if the prefetcher object is created. */
	assert(prefetcher_ghb_size >= 1 && prefetcher_it_size >= 1);
	pref->ghb_size = prefetcher_ghb_size;
	pref->it_size = prefetcher_it_size;
	pref->ghb = calloc(prefetcher_ghb_size, sizeof(struct prefetcher_ghb_t));
	pref->index_table = calloc(prefetcher_it_size, sizeof(struct prefetcher_it_t));
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

/* Currently works only for x86_uop_t being the event_queue_item
 * TODO: Need to fix this. */
static void get_it_index_tag(struct prefetcher_t *pref, struct mod_stack_t *stack, 
			     int *it_index, unsigned *tag)
{
	while (stack && stack->ret_stack)
		stack = stack->ret_stack;

	if (stack->event_queue && stack->event_queue_item)
	{
		struct x86_uop_t *uop = (struct x86_uop_t *) stack->event_queue_item;
		*it_index = uop->eip % pref->it_size;
		*tag = uop->eip;
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
				  "prefetcher: replace index_table entry", stack->id, 
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

void prefetcher_access_miss(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	int it_index;

	/* Disabling prefetching as the code is not yet complete. */
	return;

	if (target_mod->kind != mod_kind_cache || !target_mod->cache->prefetcher)
		return;

	it_index = prefetcher_update_tables(stack, target_mod);

	if (it_index < 0)
		    return;

}
