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
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"


void FrmGridCreate(FrmGrid *self, FrmEmu *emu, struct cuda_function_t *function)
{
	/* Initialize grid */
	self->emu = emu;
	self->id = list_count(emu->grids);
	self->name = new(String, function->name);
	self->state = FrmGridPending;
	self->function = function;
	self->num_gpr = function->num_gpr;

	/* Add to list */
	list_add(emu->grids, self);
}


void FrmGridDestroy(FrmGrid *self)
{
	FrmEmu *emu = self->emu;
        int i;

	/* Run free notify call-back */
	if (self->free_notify_func)
		self->free_notify_func(self->free_notify_data);

	/* Remove from lists */
	list_remove(emu->pending_grids, self);
	list_remove(emu->running_grids, self);
	list_remove(emu->finished_grids, self);
	list_remove(emu->grids, self);

        /* Free thread_blocks */
        for (i = 0; i < self->thread_block_count; i++)
                delete(self->thread_blocks[i]);
        free(self->thread_blocks);
	list_free(self->pending_thread_blocks);
	list_free(self->running_thread_blocks);
	list_free(self->finished_thread_blocks);

        /* Rest */
	delete(self->name);
}



/* Write initial values in constant buffer 0 (CB0) */
/* FIXME: constant memory should be member of 'frm_emu' or 'grid'? */
void FrmGridSetupConstantMemory(FrmGrid *self)
{
	/* FIXME: built-in consts */
        FrmEmuConstMemWrite(self->emu, 0x8, &self->thread_block_size3[0]);
}


void FrmGridSetupArguments(FrmGrid *self)
{
	struct cuda_function_t *function = self->function;
	struct cuda_function_arg_t *arg;
	int i;
	int offset = 0x20;

	/* Kernel arguments */
	for (i = 0; i < function->arg_count; i++)
	{
		arg = function->arg_array[i];
		assert(arg);

		FrmEmuConstMemWrite(self->emu, offset, &(arg->value));
		offset += 0x4;
	}
}


void FrmGridDump(FrmGrid *self, FILE *f)
{
	//	FrmThreadBlock *thread_block;
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

static void FrmGridSetupArrays(FrmGrid *self)
{
	FrmThreadBlock *thread_block;
	FrmWarp *warp;
	FrmThread *thread;

	int bid;  /* Thread block ID */
	int wid;  /* Warp ID iterator */
	int tid;  /* Thread ID iterator */

	/* Create array/lists of thread blocks */
	self->thread_blocks = (FrmThreadBlock **)xcalloc(
			self->thread_block_count, 
			sizeof(FrmThreadBlock *));
	self->pending_thread_blocks = list_create();
	self->running_thread_blocks = list_create();
	self->finished_thread_blocks = list_create();

	for (bid = 0; bid < self->thread_block_count; bid++)
	{
		/* Create new thread block */
		thread_block = new(FrmThreadBlock, self);
		self->thread_blocks[bid] = thread_block;

		/* Initialize thread block */
		thread_block->id = bid;
		snprintf(thread_block->name, sizeof(thread_block->name), 
				"thread-block[g%d-b%d]", 
				self->id, thread_block->id);

		/* Add to pending list */
		list_add(self->pending_thread_blocks, thread_block);

		/* Create array/lists of warps */
		thread_block->warp_count = 
			(self->thread_block_size + frm_emu_warp_size - 1) /
			frm_emu_warp_size;
		thread_block->warps = (FrmWarp **)xcalloc(
				thread_block->warp_count, 
				sizeof(FrmWarp *));
		thread_block->running_warps = list_create();
		thread_block->finished_warps = list_create();

		for (wid = 0; wid < thread_block->warp_count; wid++)
		{
			/* Create new warp */
			warp = new(FrmWarp, thread_block);
			thread_block->warps[wid] = warp;

			/* Initialize warp */
			warp->id = wid + bid * thread_block->warp_count;
			warp->id_in_thread_block = wid;
			snprintf(warp->name, sizeof(warp->name),
					"warp[g%d-b%d-w%d]",
					self->id, thread_block->id,
					warp->id_in_thread_block);
			warp->inst_buffer = self->function->inst_bin;
			warp->inst_buffer_size =
				self->function->inst_bin_size;
			if (wid < thread_block->warp_count - 1)
				warp->thread_count = frm_emu_warp_size;
			else
				warp->thread_count = self->thread_block_size - 
					(thread_block->warp_count - 1) *
					frm_emu_warp_size;
			warp->threads = (FrmThread **)xcalloc(
					warp->thread_count, 
					sizeof(FrmThread *));

			/* Add to running list */
			list_add(thread_block->running_warps, warp);
		}

		/* Create array/lists of threads */
		thread_block->thread_count = self->thread_block_size;
		thread_block->threads = (FrmThread **) xcalloc(
				thread_block->thread_count, 
				sizeof(FrmThread *));

		for (tid = 0; tid < thread_block->thread_count; tid++)
		{
			/* Create new thread */
			thread = new(FrmThread, thread_block->
					warps[tid / frm_emu_warp_size]);
			thread_block->threads[tid] = thread;

			/* Initialize thread */
			thread->id = tid + bid * thread_block->thread_count;
			thread->id_in_warp = tid % frm_emu_warp_size;
			thread->id_in_thread_block = tid;

			/* Save thread IDs in special register R0 */
			thread->sr[FrmInstSRegTidX].u32 = tid %
				self->thread_block_size3[0];
			thread->sr[FrmInstSRegTidY].u32 = tid /
				self->thread_block_size3[0];
			thread->sr[FrmInstSRegTidZ].u32 = tid /
				(self->thread_block_size3[0] *
				 self->thread_block_size3[1]);

			/* Save thread block IDs in special register R1 */
			thread->sr[FrmInstSRegCTAidX].u32 = bid %
				self->thread_block_count3[0];
			thread->sr[FrmInstSRegCTAidY].u32 = bid /
				self->thread_block_count3[0];
			thread->sr[FrmInstSRegCTAidZ].u32 = bid /
				(self->thread_block_count3[0] *
				 self->thread_block_count3[1]);

			/* Set predicate register #7 to 1 */
			thread->pr[7] = 1;

			/* Link thread with warp */
			thread->warp->threads[thread->id_in_warp] = thread;
		}
	}
}


void FrmGridSetupSize(FrmGrid *self, unsigned int *thread_block_count,
		unsigned int *thread_block_size)
{
	int i;

	/* Thread block counts */
	for (i = 0; i < 3; i++)
		self->thread_block_count3[i] = thread_block_count[i];
	self->thread_block_count = self->thread_block_count3[0] *
		self->thread_block_count3[1] *
		self->thread_block_count3[2];

	/* Thread block sizes */
	for (i = 0; i < 3; i++)
		self->thread_block_size3[i] = thread_block_size[i];
	self->thread_block_size = self->thread_block_size3[0] *
		self->thread_block_size3[1] *
		self->thread_block_size3[2];

	/* Calculate grid sizes */
	for (i = 0; i < 3; i++)
		self->thread_count3[i] = thread_block_count[i] *
			thread_block_size[i];
	self->thread_count = self->thread_count3[0] * 
		self->thread_count3[1] *
		self->thread_count3[2];

	/* Allocate thread blocks, warps, and threads */
	FrmGridSetupArrays(self);

	/* Debug */
	frm_isa_debug("%s:%d: block count = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, self->thread_block_count3[0],
			self->thread_block_count3[1],
			self->thread_block_count3[2]);
	frm_isa_debug("%s:%d: block size = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, self->thread_block_size3[0], 
			self->thread_block_size3[1],
			self->thread_block_size3[2]);
	frm_isa_debug("%s:%d: grid size = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, self->thread_count3[0], 
			self->thread_count3[1], self->thread_count3[2]);
}


