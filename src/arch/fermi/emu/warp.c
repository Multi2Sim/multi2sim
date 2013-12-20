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
#include <lib/util/debug.h>
#include <lib/util/bit-map.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "machine.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"



/*
 * Private Functions
 */


/* Comparison function to sort list */

static int FrmWarpCompareDivergence(const void *elem1, const void *elem2)
{
	const int count1 = * (const int *) elem1;
	const int count2 = * (const int *) elem2;

	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


static void FrmWarpDumpDivergence(FrmWarp *self, FILE *f)
{
	FrmThread *thread;
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
	for (i = 0; i < self->thread_count; i++)
	{
		thread = self->threads[i];
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
	list_sort(list, FrmWarpCompareDivergence);
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

		for (j = 0; j < self->thread_count; j++)
		{
			int first, last;
			first = self->threads[j]->branch_digest ==
				elem->branch_digest &&
				(j == 0 || self->threads[j - 1]->branch_digest
				 != elem->branch_digest);
			last = self->threads[j]->branch_digest ==
				elem->branch_digest &&
				(j == self->thread_count - 1 || self->threads[j
				 + 1]->branch_digest != elem->branch_digest);
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

void FrmWarpCreate(FrmWarp *self, FrmThreadBlock *thread_block)
{
	FrmGrid *grid = thread_block->grid;
	FrmEmu *emu = grid->emu;

	/* Initialize */
	self->thread_block = thread_block;
	self->grid = thread_block->grid;
	self->sync_stack_top = 0;
	self->sync_stack_pushed = 0;
	self->sync_stack_popped = 0;

	/* FIXME: Remove once loop state is part of stack */
	self->loop_depth = 0;
	self->inst = FrmInstWrapCreate(emu->as);
}


void FrmWarpDestroy(FrmWarp *self)
{
	/* Free warp */
	free(self->threads);
	bit_map_free(self->pred);
	FrmInstWrapFree(self->inst);
}


void FrmWarpDump(FrmWarp *self, FILE *f)
{
	FrmGrid *grid;
	FrmThreadBlock *thread_block;

	grid = self->grid;
	thread_block = self->thread_block;

	if (!f)
		return;

	/* Dump warp statistics in GPU report */
	fprintf(f, "[ Grid[%d].ThreadBlock[%d].Warp[%d] ]\n\n", 
			grid->id, thread_block->id,
			self->id_in_thread_block);

	fprintf(f, "Name = %s\n", self->name);
	fprintf(f, "ThreadFirst = %d\n", self->threads[0]->id_in_warp);
	fprintf(f, "ThreadLast = %d\n", self->threads[self->thread_count - 1]->id_in_warp);
	fprintf(f, "ThreadCount = %d\n", self->thread_count);
	fprintf(f, "\n");

	fprintf(f, "InstCount = %lld\n", self->inst_count);
	fprintf(f, "GlobalMemInstCount = %lld\n",
			self->global_mem_inst_count);
	fprintf(f, "LocalMemInstCount = %lld\n", self->local_mem_inst_count);
	fprintf(f, "\n");

	FrmWarpDumpDivergence(self, f);

	fprintf(f, "\n");
}


void FrmWarpPush(FrmWarp *self)
{
//	warp->stack_top++;
//	warp->active_mask_push++;
//	bit_map_copy(warp->active_stack, warp->stack_top * warp->thread_count,
//			warp->active_stack, (warp->stack_top - 1) *
//			warp->thread_count, warp->thread_count);
}


void FrmWarpPop(FrmWarp *self, int count)
{
//	if (!count)
//		return;
//	warp->stack_top -= count;
//	warp->active_mask_pop += count;
//	warp->active_mask_update = 1;
}


/* Execute one instruction in the warp */
void FrmWarpExecute(FrmWarp *self)
{
	FrmEmu *emu;
	FrmGrid *grid;
	FrmThreadBlock *thread_block;
	FrmThread *thread;

	struct FrmInstWrap *inst;
	FrmInstBytes bytes;
	FrmInstOp opcode;

	int thread_id;

	/* Get current arch, grid, and thread-block */
	grid = self->grid;
	emu = grid->emu;
	thread_block = self->thread_block;
	thread = NULL;
	inst = NULL;
	assert(list_index_of(thread_block->finished_warps, self) == -1);

	/* Reset instruction flags */
	self->global_mem_write = 0;
	self->global_mem_read = 0;
	self->local_mem_write = 0;
	self->local_mem_read = 0;
	self->pred_mask_update = 0;
	self->active_mask_update = 0;
	self->active_mask_push = 0;
	self->active_mask_pop = 0;
	self->inst_size = 8;
	self->at_barrier = 0;

	/* Get instruction */
	inst = self->inst;
	bytes.word[0] = self->inst_buffer[self->pc / self->inst_size] >> 32;
	bytes.word[1] = self->inst_buffer[self->pc / self->inst_size];
	frm_isa_debug("%s:%d: warp[%d] executes instruction [0x%x] 0x%0llx\n", 
			__FUNCTION__, __LINE__, self->id, self->pc,
			bytes.dword);

	/* Decode instruction */
	FrmInstWrapDecode(inst, self->pc, &bytes);
	opcode = FrmInstWrapGetOpcode(inst);
	if (!opcode)
		fatal("%s: unrecognized instruction (%08x %08x)",
			__FUNCTION__, bytes.word[0], bytes.word[1]);

	/* Execute instruction */
	for (thread_id = 0; thread_id < self->thread_count; thread_id++)
	{
		thread = self->threads[thread_id];
		emu->inst_func[opcode](thread, inst);
	}

	/* Finish */
	if (self->finished)
	{
		/* Check if warp finished kernel execution */
		assert(list_index_of(thread_block->running_warps, self) != -1);
		assert(list_index_of(thread_block->finished_warps, self) == -1);
		list_remove(thread_block->running_warps, self);
		list_add(thread_block->finished_warps, self);

		/* Check if thread block finished kernel execution */
		if (list_count(thread_block->finished_warps) ==
				thread_block->warp_count)
		{
			assert(list_index_of(grid->running_thread_blocks,
						thread_block) != -1);
			assert(list_index_of(grid->finished_thread_blocks,
						thread_block) == -1);
			list_remove(grid->running_thread_blocks, thread_block);
			list_add(grid->finished_thread_blocks, thread_block);

			/* Check if grid finished kernel execution */
			if (list_count(grid->finished_thread_blocks) == 
					grid->thread_block_count)
			{
				assert(list_index_of(emu->running_grids, grid) != -1);
				assert(list_index_of(emu->finished_grids, grid) == -1);
				list_remove(emu->running_grids, grid);
				list_add(emu->finished_grids, grid);
			}
		}
	}

	/* Increment the PC */
	self->pc += self->inst_size;

	/* Stats */
	asEmu(emu)->instructions++;
	self->emu_inst_count++;
	self->inst_count++;
}


