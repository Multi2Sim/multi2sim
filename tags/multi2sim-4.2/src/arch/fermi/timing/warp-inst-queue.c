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


#include <lib/mhandle/mhandle.h>

#include "gpu.h"
#include "warp-inst-queue.h"

struct frm_warp_inst_queue_t *frm_warp_inst_queue_create()
{
	struct frm_warp_inst_queue_t *warp_inst_queue;
	int i;

	/* Initialize */
	warp_inst_queue = xcalloc(1, sizeof(struct frm_warp_inst_queue_t));
	warp_inst_queue->entries = xcalloc(
		frm_gpu_max_warps_per_sm, 
		sizeof(struct frm_warp_inst_queue_entry_t*));

	for (i = 0; i < frm_gpu_max_warps_per_sm; i++) 
	{
		warp_inst_queue->entries[i] = xcalloc(1, 
			sizeof(struct frm_warp_inst_queue_entry_t));
		warp_inst_queue->entries[i]->id_in_warp_inst_queue = i;
		warp_inst_queue->entries[i]->warp_inst_queue = warp_inst_queue;
	}

	/* Return */
	return warp_inst_queue;
}

void frm_warp_inst_queue_free(struct frm_warp_inst_queue_t *warp_inst_queue)
{
	int i;

	for (i = 0; i < frm_gpu_max_warps_per_sm; i++) 
		free(warp_inst_queue->entries[i]);

	free(warp_inst_queue->entries);
	free(warp_inst_queue);
}

void frm_warp_inst_queue_unmap_warp(
		struct frm_warp_inst_queue_t *warp_inst_queue, 
		struct frm_warp_t *warp)
{
//	int wf_id_in_ib;
//	int i;
//
//	/* Reset mapped warps */
//	assert(warp_inst_queue->warp_count >= 
//		thread_block->warp_count);
//
//	for (i = 0; i < thread_block->warp_count; i++) 
//	{
//		warp = thread_block->warps[i];
//		wf_id_in_ib = 
//			warp->warp_inst_queue_entry->id_in_warp_inst_queue;
//
//		/* TODO Add complete flag to slots in instruction buffer */
//		/* TODO Check that all slots are complete before setting NULL */
//		assert(warp_inst_queue->entries[wf_id_in_ib]->warp);
//		assert(warp_inst_queue->entries[wf_id_in_ib]->warp->id == 
//			warp->id);
//		warp_inst_queue->entries[wf_id_in_ib]->valid = 0;
//		warp_inst_queue->entries[wf_id_in_ib]->warp_finished = 0;
//		warp_inst_queue->entries[wf_id_in_ib]->warp = NULL;
//	}
//	warp_inst_queue->warp_count -= thread_block->warp_count;
}
