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

#include <driver/cuda/function.h>
#include <driver/cuda/function-arg.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "thread.h"
#include "threadblock.h"
#include "warp.h"


struct frm_grid_t *frm_grid_create(struct frm_cuda_function_t *function)
{
	struct frm_grid_t *grid;

	/* Initialize */
	grid = xcalloc(1, sizeof(struct frm_grid_t));
	DOUBLE_LINKED_LIST_INSERT_TAIL(frm_emu, grid, grid);
	grid->id = 0;
	strncpy(grid->name, function->name, MAX_STRING_SIZE);
	grid->function = function;

	/* Return */
	return grid;
}


void frm_grid_free(struct frm_grid_t *grid)
{
        int i;

        /* Free threadblocks */
        for (i = 0; i < grid->threadblock_count; i++)
                frm_threadblock_free(grid->threadblocks[i]);
        free(grid->threadblocks);

        /* Free warps */
        for (i = 0; i < grid->warp_count; i++)
                frm_warp_free(grid->warps[i]);
        free(grid->warps);

        /* Free threads */
        for (i = 0; i < grid->thread_count; i++)
                frm_thread_free(grid->threads[i]);
        free(grid->threads);

        /* Free grid */
        free(grid);
}


int frm_grid_get_status(struct frm_grid_t *grid, enum frm_grid_status_t status)
{
        return (grid->status & status) > 0;
}


void frm_grid_set_status(struct frm_grid_t *grid, enum frm_grid_status_t status)
{
        /* Get only the new bits */
        status &= ~grid->status;

        /* Add ND-Range to lists */
        if (status & frm_grid_pending)
                DOUBLE_LINKED_LIST_INSERT_TAIL(frm_emu, pending_grid, grid);
        if (status & frm_grid_running)
                DOUBLE_LINKED_LIST_INSERT_TAIL(frm_emu, running_grid, grid);
        if (status & frm_grid_finished)
                DOUBLE_LINKED_LIST_INSERT_TAIL(frm_emu, finished_grid, grid);

        /* Update it */
        grid->status |= status;
}


void frm_grid_clear_status(struct frm_grid_t *grid, enum frm_grid_status_t status)
{
        /* Get only the bits that are set */
        status &= grid->status;

        /* Remove ND-Range from lists */
        if (status & frm_grid_pending)
                DOUBLE_LINKED_LIST_REMOVE(frm_emu, pending_grid, grid);
        if (status & frm_grid_running)
                DOUBLE_LINKED_LIST_REMOVE(frm_emu, running_grid, grid);
        if (status & frm_grid_finished)
                DOUBLE_LINKED_LIST_REMOVE(frm_emu, finished_grid, grid);

        /* Update status */
        grid->status &= ~status;
}


void frm_grid_setup_threads(struct frm_grid_t *grid)
{
	struct frm_cuda_function_t *function = grid->function;

	struct frm_threadblock_t *threadblock;
	struct frm_warp_t *warp;
	struct frm_thread_t *thread;

	int bidx, bidy, bidz;  /* 3D threadblock ID iterators */
	int lidx, lidy, lidz;  /* 3D thread local ID iterators */

	int tid;  /* Global ID iterator */
	int bid;  /* Threadblock ID iterator */
	int wid;  /* Warp ID iterator */
	int lid;  /* Local ID iterator */

	/* Array of threadblocks */
	grid->threadblock_count = function->group_count;
	grid->threadblock_id_first = 0;
	grid->threadblock_id_last = grid->threadblock_count - 1;
	grid->threadblocks = xcalloc(grid->threadblock_count, sizeof(void *));
	for (bid = 0; bid < grid->threadblock_count; bid++)
		grid->threadblocks[bid] = frm_threadblock_create();
	
	/* Array of warps */
	grid->warps_per_threadblock = (function->local_size + frm_emu_warp_size - 1) / frm_emu_warp_size;
	grid->warp_count = grid->warps_per_threadblock * grid->threadblock_count;
	grid->warp_id_first = 0;
	grid->warp_id_last = grid->warp_count - 1;
	assert(grid->warps_per_threadblock > 0 && grid->warp_count > 0);
	grid->warps = xcalloc(grid->warp_count, sizeof(void *));
	for (wid = 0; wid < grid->warp_count; wid++)
	{
		bid = wid / grid->warps_per_threadblock;
		grid->warps[wid] = frm_warp_create();
		warp = grid->warps[wid];
		threadblock = grid->threadblocks[bid];

		warp->id = wid;
		warp->id_in_threadblock = wid % grid->warps_per_threadblock;
		warp->grid = grid;
		warp->threadblock = threadblock;
		DOUBLE_LINKED_LIST_INSERT_TAIL(threadblock, running, warp);
	}

	/* Array of threads */
	grid->thread_count = function->global_size;
	grid->thread_id_first = 0;
	grid->thread_id_last = grid->thread_count - 1;
	grid->threads = xcalloc(grid->thread_count, sizeof(void *));
	tid = 0;
	bid = 0;
	for (bidz = 0; bidz < function->group_count3[2]; bidz++)
	{
		for (bidy = 0; bidy < function->group_count3[1]; bidy++)
		{
			for (bidx = 0; bidx < function->group_count3[0]; bidx++)
			{
				/* Assign threadblock ID */
				threadblock = grid->threadblocks[bid];
				threadblock->grid = grid;
				threadblock->id_3d[0] = bidx;
				threadblock->id_3d[1] = bidy;
				threadblock->id_3d[2] = bidz;
				threadblock->id = bid;
				frm_threadblock_set_status(threadblock, frm_threadblock_pending);

				/* First, last, and number of threads in threadblock */
				threadblock->thread_id_first = tid;
				threadblock->thread_id_last = tid + function->local_size - 1;
				threadblock->thread_count = function->local_size;
				threadblock->threads = &grid->threads[tid];
				snprintf(threadblock->name, sizeof(threadblock->name), "threadblock[i%d-i%d]",
					threadblock->thread_id_first, threadblock->thread_id_last);

				/* First ,last, and number of warps in threadblock */
				threadblock->warp_id_first = bid * grid->warps_per_threadblock;
				threadblock->warp_id_last = threadblock->warp_id_first + grid->warps_per_threadblock - 1;
				threadblock->warp_count = grid->warps_per_threadblock;
				threadblock->warps = &grid->warps[threadblock->warp_id_first];

				/* Iterate through threads */
				lid = 0;
				for (lidz = 0; lidz < function->local_size3[2]; lidz++)
				{
					for (lidy = 0; lidy < function->local_size3[1]; lidy++)
					{
						for (lidx = 0; lidx < function->local_size3[0]; lidx++)
						{
							/* Warp ID */
							wid = bid * grid->warps_per_threadblock +
								lid / frm_emu_warp_size;
							assert(wid < grid->warp_count);
							warp = grid->warps[wid];
							
							/* Create thread */
							grid->threads[tid] = frm_thread_create();
							thread = grid->threads[tid];
							thread->grid = grid;

							/* Global IDs */
							thread->id_3d[0] = bidx * function->local_size3[0] + lidx;
							thread->id_3d[1] = bidy * function->local_size3[1] + lidy;
							thread->id_3d[2] = bidz * function->local_size3[2] + lidz;
							thread->id = tid;

							/* Local IDs */
							thread->id_in_threadblock_3d[0] = lidx;
							thread->id_in_threadblock_3d[1] = lidy;
							thread->id_in_threadblock_3d[2] = lidz;
							thread->id_in_threadblock = lid;

							/* Other */
							thread->id_in_warp = thread->id_in_threadblock % frm_emu_warp_size;
							thread->threadblock = grid->threadblocks[bid];
							thread->warp = grid->warps[wid];

							/* First, last, and number of threads in warp */
							if (!warp->thread_count) {
								warp->thread_id_first = tid;
								warp->threads = &grid->threads[tid];
							}
							warp->thread_count++;
							warp->thread_id_last = tid;
							bit_map_set(warp->active_stack, thread->id_in_warp, 1, 1);

                                                        /* Save local IDs in register R0 */
                                                        thread->sr[FRM_SR_Tid_X].v.i = lidx;  /* R0.x */
                                                        thread->sr[FRM_SR_Tid_Y].v.i = lidy;  /* R0.y */
                                                        thread->sr[FRM_SR_Tid_Z].v.i = lidz;  /* R0.z */

                                                        /* Save threadblock IDs in register R1 */
                                                        thread->sr[FRM_SR_CTAid_X].v.i = bidx;  /* R1.x */
                                                        thread->sr[FRM_SR_CTAid_Y].v.i = bidy;  /* R1.y */
                                                        thread->sr[FRM_SR_CTAid_Z].v.i = bidz;  /* R1.z */

							/* Next thread */
							tid++;
							lid++;
						}
					}
				}

				/* Next threadblock */
				bid++;
			}
		}
	}

	/* Assign names to warps */
	for (wid = 0; wid < grid->warp_count; wid++)
	{
		warp = grid->warps[wid];
		snprintf(warp->name, sizeof(warp->name), "warp[i%d-i%d]",
			warp->thread_id_first, warp->thread_id_last);

		/* Initialize warp program counter */
                warp->buf_start = function->function_buffer.ptr;
                warp->buf = warp->buf_start;
                warp->buf_size = function->function_buffer.size;
	}

	/* Debug */
	printf("local_size = %d (%d,%d,%d)\n", function->local_size, function->local_size3[0],
		function->local_size3[1], function->local_size3[2]);
	printf("global_size = %d (%d,%d,%d)\n", function->global_size, function->global_size3[0],
		function->global_size3[1], function->global_size3[2]);
	printf("group_count = %d (%d,%d,%d)\n", function->group_count, function->group_count3[0],
		function->group_count3[1], function->group_count3[2]);
	printf("warp_count = %d\n", grid->warp_count);
	printf("warps_per_threadblock = %d\n", grid->warps_per_threadblock);
	printf(" tid tid2 tid1 tid0   bid bid2 bid1 bid0   lid lid2 lid1 lid0  warp            work-group\n");
	for (tid = 0; tid < grid->thread_count; tid++)
	{
		thread = grid->threads[tid];
		warp = thread->warp;
		threadblock = thread->threadblock;
		printf("%4d %4d %4d %4d  ", thread->id, thread->id_3d[2],
			thread->id_3d[1], thread->id_3d[0]);
		printf("%4d %4d %4d %4d  ", threadblock->id, threadblock->id_3d[2],
			threadblock->id_3d[1], threadblock->id_3d[0]);
		printf("%4d %4d %4d %4d  ", thread->id_in_threadblock, thread->id_in_threadblock_3d[2],
			thread->id_in_threadblock_3d[1], thread->id_in_threadblock_3d[0]);
		printf("%20s.%-4d  ", warp->name, thread->id_in_warp);
		printf("%20s.%-4d\n", threadblock->name, thread->id_in_threadblock);
	}

}


/* Write initial values in constant buffer 0 (CB0) */
/* FIXME: constant memory should be member of 'frm_emu' or 'grid'? */
void frm_grid_setup_const_mem(struct frm_grid_t *grid)
{
        struct frm_cuda_function_t *function = grid->function;

	/* FIXME: built-in consts */
        frm_isa_const_mem_write(0x8, &function->local_size3[0]);
}


void frm_grid_setup_args(struct frm_grid_t *grid)
{
	struct frm_cuda_function_t *function = grid->function;
	struct frm_cuda_function_arg_t *arg;
	int i;
	int offset = 0x20;

	/* Kernel arguments */
	for (i = 0; i < list_count(function->arg_list); i++)
	{
		arg = list_get(function->arg_list, i);
		assert(arg);

		/* Process argument depending on its type */
		if (arg->kind == FRM_CUDA_FUNCTION_ARG_KIND_POINTER)
		{
			if (arg->mem_scope == FRM_CUDA_MEM_SCOPE_GLOBAL)
			{
                                frm_isa_const_mem_write(offset, &(arg->value));
				offset += 0x4;
				continue;
			}
			else if (arg->mem_scope == FRM_CUDA_MEM_SCOPE_LOCAL)
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


void frm_grid_run(struct frm_grid_t *grid)
{
	struct frm_threadblock_t *threadblock, *threadblock_next;
	struct frm_warp_t *warp, *warp_next;
	unsigned long long int cycle = 0;

	/* Set all ready threadblocks to running */
	while ((threadblock = grid->pending_list_head))
	{
		frm_threadblock_clear_status(threadblock, frm_threadblock_pending);
		frm_threadblock_set_status(threadblock, frm_threadblock_running);
	}
                /* Set is in state 'running' */
                frm_grid_clear_status(grid, frm_grid_pending);
                frm_grid_set_status(grid, frm_grid_running);


	/* Execution loop */
	while (grid->running_list_head)
	{
		/* Stop if maximum number of GPU cycles exceeded */
		if (frm_emu_max_cycles && cycle >= frm_emu_max_cycles)
			esim_finish = esim_finish_frm_max_cycles;

		/* Stop if maximum number of GPU instructions exceeded */
		if (frm_emu_max_inst && frm_emu->inst_count >= frm_emu_max_inst)
			esim_finish = esim_finish_frm_max_inst;

		/* Stop if any reason met */
		if (esim_finish)
			break;

		/* Next cycle */
		cycle++;

		/* Execute an instruction from each work-group */
		for (threadblock = grid->running_list_head; threadblock; threadblock = threadblock_next)
		{
			/* Save next running work-group */
			threadblock_next = threadblock->running_list_next;

			/* Run an instruction from each warp */
			for (warp = threadblock->running_list_head; warp; warp = warp_next)
			{
				/* Save next running warp */
				warp_next = warp->running_list_next;

				/* Execute instruction in warp */
				frm_warp_execute(warp);
			}
		}
	}

	/* Dump stats */
	frm_grid_dump(grid, stdout);

	/* Stop if maximum number of functions reached */
	//if (frm_emu_max_functions && frm_emu->grid_count >= frm_emu_max_functions)
	//	x86_emu_finish = x86_emu_finish_max_gpu_functions;
}


void frm_grid_dump(struct frm_grid_t *grid, FILE *f)
{
	struct frm_threadblock_t *threadblock;
	int threadblock_id;
	int thread_id, last_thread_id;
	uint32_t branch_digest, last_branch_digest;
	int branch_digest_count;

	if (!f)
		return;
	
	fprintf(f, "[ Grid[%d] ]\n\n", grid->id);
	fprintf(f, "Name = %s\n", grid->name);
	fprintf(f, "ThreadBlockFirst = %d\n", grid->threadblock_id_first);
	fprintf(f, "ThreadBlockLast = %d\n", grid->threadblock_id_last);
	fprintf(f, "ThreadBlockCount = %d\n", grid->threadblock_count);
	fprintf(f, "WarpFirst = %d\n", grid->warp_id_first);
	fprintf(f, "WarpLast = %d\n", grid->warp_id_last);
	fprintf(f, "WarpCount = %d\n", grid->warp_count);
	fprintf(f, "ThreadFirst = %d\n", grid->thread_id_first);
	fprintf(f, "ThreadLast = %d\n", grid->thread_id_last);
	fprintf(f, "ThreadCount = %d\n", grid->thread_count);

	/* Branch digests */
	assert(grid->thread_count);
	branch_digest_count = 0;
	last_thread_id = 0;
	last_branch_digest = grid->threads[0]->branch_digest;
	for (thread_id = 1; thread_id <= grid->thread_count; thread_id++)
	{
		branch_digest = thread_id < grid->thread_count ? grid->threads[thread_id]->branch_digest : 0;
		if (thread_id == grid->thread_count || branch_digest != last_branch_digest)
		{
			fprintf(f, "BranchDigest[%d] = %d %d %08x\n", branch_digest_count,
				last_thread_id, thread_id - 1, last_branch_digest);
			last_thread_id = thread_id;
			last_branch_digest = branch_digest;
			branch_digest_count++;
		}
	}
	fprintf(f, "BranchDigestCount = %d\n", branch_digest_count);
	fprintf(f, "\n");

	/* Threadblock */
	FRM_FOR_EACH_THREADBLOCK_IN_GRID(grid, threadblock_id)
	{
		threadblock = grid->threadblocks[threadblock_id];
		frm_threadblock_dump(threadblock, f);
	}
}

