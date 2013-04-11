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

#include <arch/fermi/emu/grid.h>
#include <arch/fermi/emu/warp.h>
#include <arch/fermi/emu/thread-block.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "gpu.h"
#include "warp-pool.h"

struct frm_warp_pool_t *frm_warp_pool_create()
{
	struct frm_warp_pool_t *warp_pool;
	int i;

	/* Initialize */
	warp_pool = xcalloc(1, sizeof(struct frm_warp_pool_t));
	warp_pool->entries = xcalloc(
		frm_gpu_max_warps_per_sm, 
		sizeof(struct frm_warp_pool_entry_t*));

	for (i = 0; i < frm_gpu_max_warps_per_sm; i++) 
	{
		warp_pool->entries[i] = xcalloc(1, 
			sizeof(struct frm_warp_pool_entry_t));
		warp_pool->entries[i]->id_in_warp_pool = i;
		warp_pool->entries[i]->warp_pool = warp_pool;
	}

	/* Return */
	return warp_pool;
}

void frm_warp_pool_free(struct frm_warp_pool_t *warp_pool)
{
	int i;

	for (i = 0; i < frm_gpu_max_warps_per_sm; i++) 
		free(warp_pool->entries[i]);

	free(warp_pool->entries);
	free(warp_pool);
}

void frm_warp_pool_map_warps(struct frm_warp_pool_t *warp_pool,
	struct frm_thread_block_t *thread_block)
{
	struct frm_grid_t *grid = thread_block->grid;
	struct frm_warp_t *warp;
	int first_entry;
	int i;

	/* Determine starting ID for warps in the instruction buffer */
	first_entry = (thread_block->id_in_sm) * (grid->warps_per_thread_block);

	/* Assign warps a slot in the instruction buffer */
	for (i = 0; i < grid->warps_per_thread_block; i++) 
	{
		warp = thread_block->warps[i];
		warp->warp_pool_entry =
			warp_pool->entries[first_entry + i];
		assert(!warp->warp_pool_entry->valid);

		frm_gpu_debug("w[%d] assigned to warp pool entry [%d]\n", i,
				warp->warp_pool_entry->id_in_warp_pool);

		/* Set initial state */
		warp->warp_pool_entry->valid = 1;
		warp->warp_pool_entry->ready = 1;
		warp->warp_pool_entry->warp = warp;

		warp_pool->warp_count++;
	}
}

void frm_warp_pool_unmap_warps(struct frm_warp_pool_t 
	*warp_pool, struct frm_thread_block_t *thread_block)
{
	struct frm_grid_t *grid = thread_block->grid;
	struct frm_warp_t *warp;
	int wf_id_in_ib;
	int i;

	/* Reset mapped warps */
	assert(warp_pool->warp_count >= 
		grid->warps_per_thread_block);

	for (i = 0; i < grid->warps_per_thread_block; i++) 
	{
		warp = thread_block->warps[i];
		wf_id_in_ib = 
			warp->warp_pool_entry->id_in_warp_pool;

		/* TODO Add complete flag to slots in instruction buffer */
		/* TODO Check that all slots are complete before setting NULL */
		assert(warp_pool->entries[wf_id_in_ib]->warp);
		assert(warp_pool->entries[wf_id_in_ib]->warp->id == 
			warp->id);
		warp_pool->entries[wf_id_in_ib]->valid = 0;
		warp_pool->entries[wf_id_in_ib]->warp_finished = 0;
		warp_pool->entries[wf_id_in_ib]->warp = NULL;
	}
	warp_pool->warp_count -= grid->warps_per_thread_block;
}
