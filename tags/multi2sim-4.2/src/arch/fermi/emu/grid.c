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


#include <driver/cuda/function.h>
#include <driver/cuda/function-arg.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"


struct frm_grid_t *frm_grid_create(struct cuda_function_t *function)
{
	struct frm_grid_t *grid;

	/* Create new grid */
	grid = xcalloc(1, sizeof(struct frm_grid_t));

	/* Initialize grid */
	grid->id = list_count(frm_emu->grids);
	strncpy(grid->name, function->name, MAX_STRING_SIZE);
	grid->status = frm_grid_pending;
	grid->function = function;
	grid->num_gpr_used = function->num_gpr_used;

	/* Add to list */
	list_add(frm_emu->grids, grid);

	/* Return */
	return grid;
}


void frm_grid_free(struct frm_grid_t *grid)
{
        int i;

	/* Run free notify call-back */
	if (grid->free_notify_func)
		grid->free_notify_func(grid->free_notify_data);

	/* Remove from lists */
	list_remove(frm_emu->pending_grids, grid);
	list_remove(frm_emu->running_grids, grid);
	list_remove(frm_emu->finished_grids, grid);
	list_remove(frm_emu->grids, grid);

        /* Free thread_blocks */
        for (i = 0; i < grid->thread_block_count; i++)
                frm_thread_block_free(grid->thread_blocks[i]);
        free(grid->thread_blocks);
	list_free(grid->pending_thread_blocks);
	list_free(grid->running_thread_blocks);
	list_free(grid->finished_thread_blocks);

        /* Free grid */
        free(grid);
}



/* Write initial values in constant buffer 0 (CB0) */
/* FIXME: constant memory should be member of 'frm_emu' or 'grid'? */
void frm_grid_setup_const_mem(struct frm_grid_t *grid)
{
	/* FIXME: built-in consts */
        frm_isa_const_mem_write(0x8, &grid->block_size3[0]);
}


void frm_grid_setup_args(struct frm_grid_t *grid)
{
	struct cuda_function_t *function = grid->function;
	struct cuda_function_arg_t *arg;
	int i;
	int offset = 0x20;

	/* Kernel arguments */
	for (i = 0; i < list_count(function->arg_list); i++)
	{
		arg = list_get(function->arg_list, i);
		assert(arg);

		/* Process argument depending on its type */
		if (arg->kind == CUDA_FUNCTION_ARG_KIND_POINTER)
		{
			if (arg->mem_scope == CUDA_MEM_SCOPE_GLOBAL)
			{
                                frm_isa_const_mem_write(offset, &(arg->value));
				offset += 0x4;
				continue;
			}
			else if (arg->mem_scope == CUDA_MEM_SCOPE_LOCAL)
			{
				offset += 0x4;
				continue;
			}
			else
				fatal("%s: argument in memory scope %d not supported",
					__FUNCTION__, arg->mem_scope);
		}
		else
			fatal("%s: argument type not recognized", __FUNCTION__);
	}
}


void frm_grid_dump(struct frm_grid_t *grid, FILE *f)
{
//	struct frm_thread_block_t *thread_block;
//	int thread_block_id;
//	int tid, last_thread_id;
//	uint32_t branch_digest, last_branch_digest;
//	int branch_digest_count;
//
//	if (!f)
//		return;
//	
//	fprintf(f, "[ Grid[%d] ]\n\n", grid->id);
//	fprintf(f, "Name = %s\n", grid->name);
//	fprintf(f, "ThreadBlockFirst = %d\n", 0);
//	fprintf(f, "ThreadBlockLast = %d\n", grid->block_count - 1);
//	fprintf(f, "ThreadBlockCount = %d\n", grid->block_count);
//
//	/* Branch digests */
//	branch_digest_count = 0;
//	last_thread_id = 0;
//	last_branch_digest = grid->thread_blocks[0]->threads[0]->branch_digest;
//	for (tid = 1; tid <= grid->grid_size; tid++)
//	{
//		branch_digest = tid < grid->grid_size ? grid->threads[tid]->branch_digest : 0;
//		if (tid == grid->thread_count || branch_digest != last_branch_digest)
//		{
//			fprintf(f, "BranchDigest[%d] = %d %d %08x\n", branch_digest_count,
//				last_thread_id, tid - 1, last_branch_digest);
//			last_thread_id = tid;
//			last_branch_digest = branch_digest;
//			branch_digest_count++;
//		}
//	}
//	fprintf(f, "BranchDigestCount = %d\n", branch_digest_count);
//	fprintf(f, "\n");
//
//	/* Thread block */
//	FRM_FOR_EACH_THREADBLOCK_IN_GRID(grid, thread_block_id)
//	{
//		thread_block = grid->thread_blocks[thread_block_id];
//		frm_thread_block_dump(thread_block, f);
//	}
}

static void frm_grid_setup_arrays(struct frm_grid_t *grid)
{
	struct frm_thread_block_t *thread_block;
	struct frm_warp_t *warp;
	struct frm_thread_t *thread;

	int bid;  /* Thread block ID */
	int wid;  /* Warp ID iterator */
	int tid;  /* Thread ID iterator */

	/* Create array/lists of thread blocks */
	grid->thread_block_count = grid->block_count;
	grid->thread_blocks = (struct frm_thread_block_t **)xcalloc(
			grid->block_count, 
			sizeof(struct frm_thread_block_t *));
	grid->pending_thread_blocks = list_create();
	grid->running_thread_blocks = list_create();
	grid->finished_thread_blocks = list_create();

	for (bid = 0; bid < grid->block_count; bid++)
	{
		/* Create new thread block */
		thread_block = frm_thread_block_create();
		grid->thread_blocks[bid] = thread_block;

		/* Initialize thread block */
		thread_block->id = bid;
		snprintf(thread_block->name, sizeof(thread_block->name), 
				"thread-block[g%d-b%d]", 
				grid->id, thread_block->id);
		thread_block->grid = grid;

		/* Add to pending list */
		list_add(grid->pending_thread_blocks, thread_block);

		/* Create array/lists of warps */
		thread_block->warp_count = 
			(grid->block_size + frm_emu_warp_size - 1) /
			frm_emu_warp_size;
		thread_block->warps = (struct frm_warp_t **)xcalloc(
				thread_block->warp_count, 
				sizeof(struct frm_warp_t *));
		thread_block->running_warps = list_create();
		thread_block->finished_warps = list_create();

		for (wid = 0; wid < thread_block->warp_count; wid++)
		{
			/* Create new warp */
			warp = frm_warp_create();
			thread_block->warps[wid] = warp;

			/* Initialize warp */
			warp->id = wid + bid * thread_block->warp_count;
			warp->id_in_thread_block = wid;
			snprintf(warp->name, sizeof(warp->name),
					"warp[g%d-b%d-w%d]",
					grid->id, thread_block->id,
					warp->id_in_thread_block);
			warp->grid = grid;
			warp->thread_block = thread_block;
			warp->inst_buffer = grid->function->inst_buffer;
			warp->inst_buffer_size =
				grid->function->inst_buffer_size;
			if (wid < thread_block->warp_count - 1)
				warp->thread_count = frm_emu_warp_size;
			else
				warp->thread_count = grid->block_size - 
					(thread_block->warp_count - 1) *
					frm_emu_warp_size;
			warp->threads = (struct frm_thread_t **)xcalloc(
					warp->thread_count, 
					sizeof(struct frm_thread_t *));

			/* Add to running list */
			list_add(thread_block->running_warps, warp);
		}

		/* Create array/lists of threads */
		thread_block->thread_count = grid->block_size;
		thread_block->threads = (struct frm_thread_t **)xcalloc(
				thread_block->thread_count, 
				sizeof(struct frm_thread_t *));

		for (tid = 0; tid < thread_block->thread_count; tid++)
		{
			/* Create new thread */
			thread = frm_thread_create();
			thread_block->threads[tid] = thread;

			/* Initialize thread */
			thread->id = tid + bid * thread_block->thread_count;
			thread->id_in_warp = tid % frm_emu_warp_size;
			thread->id_in_thread_block = tid;
			thread->warp = thread_block->
				warps[tid / frm_emu_warp_size];
			thread->thread_block = thread_block;
			thread->grid = grid;

			/* Save thread IDs in special register R0 */
			thread->sr[FRM_SR_Tid_X].v.i = tid % 
				grid->block_size3[0];
			thread->sr[FRM_SR_Tid_Y].v.i = tid / 
				grid->block_size3[0];
			thread->sr[FRM_SR_Tid_Z].v.i = tid / 
				(grid->block_size3[0] * grid->block_size3[1]);

			/* Save thread block IDs in special register R1 */
			thread->sr[FRM_SR_CTAid_X].v.i = bid % 
				grid->block_count3[0];
			thread->sr[FRM_SR_CTAid_Y].v.i = bid / 
				grid->block_count3[0];
			thread->sr[FRM_SR_CTAid_Z].v.i = bid / 
				(grid->block_count3[0] * grid->block_count3[1]);

			/* Set predicate register #7 to 1 */
			thread->pr[7] = 1;

			/* Link thread with warp */
			thread->warp->threads[thread->id_in_warp] = thread;
		}
	}
}


void frm_grid_setup_size(struct frm_grid_t *grid, unsigned int *block_count,
		unsigned int *block_size)
{
	int i;

	/* Thread block counts */
	for (i = 0; i < 3; i++)
		grid->block_count3[i] = block_count[i];
	grid->block_count = grid->block_count3[0] * grid->block_count3[1] *
		grid->block_count3[2];

	/* Thread block sizes */
	for (i = 0; i < 3; i++)
		grid->block_size3[i] = block_size[i];
	grid->block_size = grid->block_size3[0] * grid->block_size3[1] *
		grid->block_size3[2];

	/* Calculate grid sizes */
	for (i = 0; i < 3; i++)
		grid->grid_size3[i] = block_count[i] * block_size[i];
	grid->grid_size = grid->grid_size3[0] * grid->grid_size3[1] *
		grid->grid_size3[2];

	/* Allocate thread blocks, warps, and threads */
	frm_grid_setup_arrays(grid);

	/* Debug */
	frm_isa_debug("%s:%d: block count = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, grid->block_count3[0],
			grid->block_count3[1], grid->block_count3[2]);
	frm_isa_debug("%s:%d: block size = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, grid->block_size3[0], 
			grid->block_size3[1], grid->block_size3[2]);
	frm_isa_debug("%s:%d: grid size = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, grid->grid_size3[0], 
			grid->grid_size3[1], grid->grid_size3[2]);
}


