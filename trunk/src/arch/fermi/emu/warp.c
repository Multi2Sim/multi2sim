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
#include <lib/util/bit-map.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "emu.h"
#include "grid.h"
#include "machine.h"
#include "thread.h"
#include "threadblock.h"
#include "warp.h"



/*
 * Private Functions
 */


/* Comparison function to sort list */

static int frm_warp_divergence_compare(const void *elem1, const void *elem2)
{
	const int count1 = * (const int *) elem1;
	const int count2 = * (const int *) elem2;
	
	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


static void frm_warp_divergence_dump(struct frm_warp_t *warp, FILE *f)
{
	struct frm_thread_t *thread;
	struct elem_t
	{
		int count;  /* 1st field hardcoded for comparison */
		int list_index;
		unsigned int branch_digest;
	};
	struct elem_t *elem;
	struct list_t *list;
	struct hash_table_t *ht;
	char str[10];
	int i, j;

	/* Create list and hash table */
	list = list_create();
	ht = hash_table_create(20, 1);

	/* Create one 'elem' for each thread with a different branch digest, and
	 * store it into the hash table and list. */
	for (i = 0; i < warp->thread_count; i++)
	{
		thread = warp->threads[i];
		sprintf(str, "%08x", thread->branch_digest);
		elem = hash_table_get(ht, str);
		if (!elem)
		{
			elem = xcalloc(1, sizeof(struct elem_t));

			hash_table_insert(ht, str, elem);
			elem->list_index = list_count(list);
			elem->branch_digest = thread->branch_digest;
			list_add(list, elem);
		}
		elem->count++;
	}

	/* Sort divergence groups as per size */
	list_sort(list, frm_warp_divergence_compare);
	fprintf(f, "DivergenceGroups = %d\n", list_count(list));
	
	/* Dump size of groups with */
	fprintf(f, "DivergenceGroupsSize =");
	for (i = 0; i < list_count(list); i++)
	{
		elem = list_get(list, i);
		fprintf(f, " %d", elem->count);
	}
	fprintf(f, "\n\n");

	/* Dump thread ids contained in each thread divergence group */
	for (i = 0; i < list_count(list); i++)
	{
		elem = list_get(list, i);
		fprintf(f, "DivergenceGroup[%d] =", i);

		for (j = 0; j < warp->thread_count; j++)
		{
			int first, last;
			first = warp->threads[j]->branch_digest == elem->branch_digest &&
				(j == 0 || warp->threads[j - 1]->branch_digest != elem->branch_digest);
			last = warp->threads[j]->branch_digest == elem->branch_digest &&
				(j == warp->thread_count - 1 || warp->threads[j + 1]->branch_digest != elem->branch_digest);
			if (first)
				fprintf(f, " %d", j);
			else if (last)
				fprintf(f, "-%d", j);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");

	/* Free 'elem's and structures */
	for (i = 0; i < list_count(list); i++)
		free(list_get(list, i));
	list_free(list);
	hash_table_free(ht);
}



/*
 * Public Functions
 */


struct frm_warp_t *frm_warp_create()
{
	struct frm_warp_t *warp;

	/* Initialize */
	warp = xcalloc(1, sizeof(struct frm_warp_t));
	warp->active_stack = bit_map_create(FRM_MAX_STACK_SIZE * frm_emu_warp_size);
	warp->pred = bit_map_create(frm_emu_warp_size);
	/* FIXME: Remove once loop state is part of stack */
	warp->loop_depth = 0;

	/* Return */
	return warp;
}


void frm_warp_free(struct frm_warp_t *warp)
{
	/* Free warp */
	bit_map_free(warp->active_stack);
	bit_map_free(warp->pred);
	free(warp);
}


void frm_warp_dump(struct frm_warp_t *warp, FILE *f)
{
	struct frm_grid_t *grid = warp->grid;
	struct frm_threadblock_t *threadblock = warp->threadblock;

	if (!f)
		return;
	
	/* Dump warp statistics in GPU report */
	fprintf(f, "[ NDRange[%d].Wavefront[%d] ]\n\n", grid->id, warp->id);

	fprintf(f, "Name = %s\n", warp->name);
	fprintf(f, "WorkGroup = %d\n", threadblock->id);
	fprintf(f, "WorkItemFirst = %d\n", warp->thread_id_first);
	fprintf(f, "WorkItemLast = %d\n", warp->thread_id_last);
	fprintf(f, "WorkItemCount = %d\n", warp->thread_count);
	fprintf(f, "\n");

	fprintf(f, "Inst_Count = %lld\n", warp->inst_count);
	fprintf(f, "Global_Mem_Inst_Count = %lld\n", warp->global_mem_inst_count);
	fprintf(f, "Local_Mem_Inst_Count = %lld\n", warp->local_mem_inst_count);
	fprintf(f, "\n");

	frm_warp_divergence_dump(warp, f);

	fprintf(f, "\n");
}


void frm_warp_stack_push(struct frm_warp_t *warp)
{
	warp->stack_top++;
	warp->active_mask_push++;
	bit_map_copy(warp->active_stack, warp->stack_top * warp->thread_count,
		warp->active_stack, (warp->stack_top - 1) * warp->thread_count,
		warp->thread_count);
}


void frm_warp_stack_pop(struct frm_warp_t *warp, int count)
{
	if (!count)
		return;
	warp->stack_top -= count;
	warp->active_mask_pop += count;
	warp->active_mask_update = 1;
}


/* Execute one instruction in the warp */
void frm_warp_execute(struct frm_warp_t *warp)
{
	struct frm_threadblock_t *threadblock;
	struct frm_thread_t *thread;
	struct frm_inst_t *inst;

	/* Get current work-group */
	struct frm_grid_t *grid = warp->grid;
	warp = warp;
	threadblock = warp->threadblock;
	thread = NULL;
	inst = NULL;
	assert(!DOUBLE_LINKED_LIST_MEMBER(threadblock, finished, warp));

	/* Reset instruction flags */
	warp->global_mem_write = 0;
	warp->global_mem_read = 0;
	warp->local_mem_write = 0;
	warp->local_mem_read = 0;
	warp->pred_mask_update = 0;
	warp->active_mask_update = 0;
	warp->active_mask_push = 0;
	warp->active_mask_pop = 0;

	int thread_id;

	/* Decode instruction */

	int byte_index;

	inst = &(warp->inst);
	//printf("Instruction Hex: ");
	/* Print most significant byte first */
	for (byte_index = 7; byte_index >= 0; --byte_index)
	{
		//printf("%02x", *((unsigned char*)(warp->buf)+byte_index));
		inst->dword.bytes[byte_index] = *((unsigned char*)(warp->buf)+byte_index);
	}
	//printf("\n");

        FRM_FOREACH_THREAD_IN_WARP(warp, thread_id)
        {
                thread = grid->threads[thread_id];
		frm_inst_decode(inst);
        	(*frm_isa_inst_func[inst->info->inst])(thread, inst);
        }

	warp->buf += 8;

	/* Stats */
	frm_emu->inst_count++;
	warp->emu_inst_count++;
	warp->inst_count++;
	{
		warp->global_mem_inst_count++;
	}


	if (warp->finished)
	{
		/* Check if warp finished kernel execution */
		assert(DOUBLE_LINKED_LIST_MEMBER(threadblock, running, warp));
		assert(!DOUBLE_LINKED_LIST_MEMBER(threadblock, finished, warp));
		DOUBLE_LINKED_LIST_REMOVE(threadblock, running, warp);
		DOUBLE_LINKED_LIST_INSERT_TAIL(threadblock, finished, warp);

		/* Check if work-group finished kernel execution */
		if (threadblock->finished_list_count == threadblock->warp_count)
		{
			assert(DOUBLE_LINKED_LIST_MEMBER(grid, running, threadblock));
			assert(!DOUBLE_LINKED_LIST_MEMBER(grid, finished, threadblock));
			frm_threadblock_clear_status(threadblock, frm_threadblock_running);
			frm_threadblock_set_status(threadblock, frm_threadblock_finished);
		}
	}
}
