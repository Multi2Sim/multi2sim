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

#include "Emu.h"
#include "Grid.h"
#include "isa.h"
#include "Thread.h"
#include "ThreadBlock.h"
#include "Warp.h"


void KplGridCreate(KplGrid *self, KplEmu *emu, struct cuda_function_t *function)
{
	/* Initialize grid */
	self->emu = emu;
	self->id = list_count(emu->grids);
	//self->name = new(String, function->name);
	self->state = KplGridPending;
	self->function = function;
	self->num_gpr = function->num_gpr;

	/* Add to list */
	list_add(emu->grids, self);
}

void KplGridDestroy(KplGrid *self)
{
	KplEmu *emu = self->emu;
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
/* FIXME: constant memory should be member of 'kpl_emu' or 'grid'? */
void KplGridSetupConstantMemory(KplGrid *self)
{
	/* FIXME: built-in consts */
        KplEmuConstMemWrite(self->emu, 0x8, &self->thread_block_size3[0]);
}


void KplGridSetupArguments(KplGrid *self)
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

		KplEmuConstMemWrite(self->emu, offset, &(arg->value));
		offset += 0x4;
	}
}


//static void KplGridSetupArrays(KplGrid *self)
//{
//	KplThreadBlock *thread_block;
//	KplWarp *warp;
//	KplThread *thread;

//	int bid;  /* Thread block ID */
//	int wid;  /* Warp ID iterator */
//	int tid;  /* Thread ID iterator */

	/* Create array/lists of thread blocks */
//	self->thread_blocks = (KplThreadBlock **)xcalloc(
//			self->thread_block_count, 
//			sizeof(KplThreadBlock *));
//	self->pending_thread_blocks = list_create();
//	self->running_thread_blocks = list_create();
//	self->finished_thread_blocks = list_create();

//	for (bid = 0; bid < self->thread_block_count; bid++)
//	{
		/* Create new thread block */
		//thread_block = new(KplThreadBlock, self);
//		self->thread_blocks[bid] = thread_block;

		/* Initialize thread block */
//		thread_block->id = bid;
//		snprintf(thread_block->name, sizeof(thread_block->name), 
//				"thread-block[g%d-b%d]", 
//				self->id, thread_block->id);

		/* Add to pending list */
//		list_add(self->pending_thread_blocks, thread_block);

		/* Create array/lists of warps */
//		thread_block->warp_count = 
//			(self->thread_block_size + kpl_emu_warp_size - 1) /
//			kpl_emu_warp_size;
//		thread_block->warps = (KplWarp **)xcalloc(
//				thread_block->warp_count, 
//				sizeof(KplWarp *));
//		thread_block->running_warps = list_create();
//		thread_block->finished_warps = list_create();

//		for (wid = 0; wid < thread_block->warp_count; wid++)
//		{
			/* Create new warp */
			//warp = new(KplWarp, thread_block);
//			thread_block->warps[wid] = warp;

			/* Initialize warp */
//			warp->id = wid + bid * thread_block->warp_count;
//			warp->id_in_thread_block = wid;
//			snprintf(warp->name, sizeof(warp->name),
//					"warp[g%d-b%d-w%d]",
//					self->id, thread_block->id,
//					warp->id_in_thread_block);
//			warp->inst_buffer = self->function->inst_bin;
//			warp->inst_buffer_size =
//				self->function->inst_bin_size;
//			if (wid < thread_block->warp_count - 1)
//				warp->thread_count = kpl_emu_warp_size;
//			else
//				warp->thread_count = self->thread_block_size - 
//					(thread_block->warp_count - 1) *
//					kpl_emu_warp_size;
//			warp->threads = (KplThread **)xcalloc(
//					warp->thread_count, 
//					sizeof(KplThread *));
//			warp->sync_stack.entries[0].active_thread_mask
//				= ((unsigned long long int)1 << warp->thread_count) - 1;

			/* Add to running list */
//			list_add(thread_block->running_warps, warp);
//		}

		/* Create array/lists of threads */
//		thread_block->thread_count = self->thread_block_size;
//		thread_block->threads = (KplThread **) xcalloc(
//				thread_block->thread_count, 
//				sizeof(KplThread *));

//		for (tid = 0; tid < thread_block->thread_count; tid++)
//		{
			/* Create new thread */
			//thread = new(KplThread, thread_block->
			//		warps[tid / kpl_emu_warp_size]);
//			thread_block->threads[tid] = thread;

			/* Initialize thread */
//			thread->id = tid + bid * thread_block->thread_count;
//			thread->id_in_warp = tid % kpl_emu_warp_size;
//			thread->id_in_thread_block = tid;

			/* Save thread IDs in special register R0 */
//			thread->sr[33].u32 = tid %
//				self->thread_block_size3[0];
//			thread->sr[34].u32 = tid /
//				self->thread_block_size3[0];
//			thread->sr[35].u32 = tid /
//				(self->thread_block_size3[0] *
//				 self->thread_block_size3[1]);

			/* Save thread block IDs in special register R1 */
//			thread->sr[37].u32 = bid %
//				self->thread_block_count3[0];
//			thread->sr[38].u32 = bid /
//				self->thread_block_count3[0];
//			thread->sr[39].u32 = bid /
//				(self->thread_block_count3[0] *
//				 self->thread_block_count3[1]);

			/* Set predicate register #7 to 1 */
//			thread->pr[7] = 1;

			/* Link thread with warp */
//			thread->warp->threads[thread->id_in_warp] = thread;
//		}
//	}
//}

void KplGridSetupSize(KplGrid *self, unsigned int *thread_block_count,
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
	KplGridSetupArrays(self);

	/* Debug */
	kpl_isa_debug("%s:%d: block count = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, self->thread_block_count3[0],
			self->thread_block_count3[1],
			self->thread_block_count3[2]);
	kpl_isa_debug("%s:%d: block size = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, self->thread_block_size3[0], 
			self->thread_block_size3[1],
			self->thread_block_size3[2]);
	kpl_isa_debug("%s:%d: grid size = (%d,%d,%d)\n", 
			__FUNCTION__, __LINE__, self->thread_count3[0], 
			self->thread_count3[1], self->thread_count3[2]);
}



