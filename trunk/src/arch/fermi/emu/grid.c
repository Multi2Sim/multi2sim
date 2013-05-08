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

#include <arch/common/arch.h>
#include <driver/cuda/function.h>
#include <driver/cuda/function-arg.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"


struct frm_grid_t *frm_grid_create(struct cuda_function_t *function)
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

	/* Run free notify call-back */
	if (grid->free_notify_func)
		grid->free_notify_func(grid->free_notify_data);

	/* Clear all states that affect lists. */
	frm_grid_clear_status(grid, frm_grid_pending);
	frm_grid_clear_status(grid, frm_grid_running);
	frm_grid_clear_status(grid, frm_grid_finished);

	/* Extract from ND-Range list in Southern Islands emulator */
	assert(DOUBLE_LINKED_LIST_MEMBER(frm_emu, grid, grid));
	DOUBLE_LINKED_LIST_REMOVE(frm_emu, grid, grid);

        /* Free thread_blocks */
        for (i = 0; i < grid->thread_block_count; i++)
                frm_thread_block_free(grid->thread_blocks[i]);
        free(grid->thread_blocks);

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

        /* Add grid to lists */
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

        /* Remove grid from lists */
        if (status & frm_grid_pending)
                DOUBLE_LINKED_LIST_REMOVE(frm_emu, pending_grid, grid);
        if (status & frm_grid_running)
                DOUBLE_LINKED_LIST_REMOVE(frm_emu, running_grid, grid);
        if (status & frm_grid_finished)
                DOUBLE_LINKED_LIST_REMOVE(frm_emu, finished_grid, grid);

        /* Update status */
        grid->status &= ~status;
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


void frm_grid_run(struct frm_grid_t *grid)
{
	struct frm_thread_block_t *thread_block, *thread_block_next;
	struct frm_warp_t *warp, *warp_next;
	unsigned long long int cycle = 0;

	/* Set all ready thread_blocks to running */
	while ((thread_block = grid->pending_list_head))
	{
		frm_thread_block_clear_status(thread_block, frm_thread_block_pending);
		frm_thread_block_set_status(thread_block, frm_thread_block_running);
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
		if (frm_emu_max_inst && arch_fermi->inst_count >= frm_emu_max_inst)
			esim_finish = esim_finish_frm_max_inst;

		/* Stop if any reason met */
		if (esim_finish)
			break;

		/* Next cycle */
		cycle++;

		/* Execute an instruction from each work-group */
		for (thread_block = grid->running_list_head; thread_block; thread_block = thread_block_next)
		{
			/* Save next running work-group */
			thread_block_next = thread_block->running_list_next;

			/* Run an instruction from each warp */
			for (warp = thread_block->running_list_head; warp; warp = warp_next)
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
	struct frm_thread_block_t *thread_block;
	int thread_block_id;
	int thread_id, last_thread_id;
	uint32_t branch_digest, last_branch_digest;
	int branch_digest_count;

	if (!f)
		return;
	
	fprintf(f, "[ Grid[%d] ]\n\n", grid->id);
	fprintf(f, "Name = %s\n", grid->name);
	fprintf(f, "ThreadBlockFirst = %d\n", grid->thread_block_id_first);
	fprintf(f, "ThreadBlockLast = %d\n", grid->thread_block_id_last);
	fprintf(f, "ThreadBlockCount = %d\n", grid->thread_block_count);
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
	FRM_FOR_EACH_THREADBLOCK_IN_GRID(grid, thread_block_id)
	{
		thread_block = grid->thread_blocks[thread_block_id];
		frm_thread_block_dump(thread_block, f);
	}
}

static void frm_grid_setup_arrays(struct frm_grid_t *grid)
{
	struct frm_thread_block_t *thread_block;
	struct frm_warp_t *warp;
	struct frm_thread_t *thread;

	int gidx, gidy, gidz;  /* 3D work-group ID iterators */
	int lidx, lidy, lidz;  /* 3D work-item local ID iterators */

	int tid;  /* Global ID iterator */
	int gid;  /* Group ID iterator */
	int wid;  /* Wavefront ID iterator */
	int lid;  /* Local ID iterator */

	/* Array of work-groups */
	grid->thread_block_count = grid->block_count;
	grid->thread_block_id_first = 0;
	grid->thread_block_id_last = grid->thread_block_count - 1;
	grid->thread_blocks = xcalloc(grid->thread_block_count, sizeof(void *));
	for (gid = 0; gid < grid->block_count; gid++)
	{
		grid->thread_blocks[gid] = frm_thread_block_create();
		thread_block = grid->thread_blocks[gid];
	}

	/* Array of warps */
	grid->warps_per_thread_block = 
		(grid->block_size + frm_emu_warp_size - 1) /
		frm_emu_warp_size;
	grid->warp_count = grid->warps_per_thread_block * 
		grid->thread_block_count;
	grid->warp_id_first = 0;
	grid->warp_id_last = grid->warp_count - 1;
	assert(grid->warps_per_thread_block > 0 && 
		grid->warp_count > 0);
	grid->warps = xcalloc(grid->warp_count, sizeof(void *));

	for (wid = 0; wid < grid->warp_count; wid++)
	{
		gid = wid / grid->warps_per_thread_block;
		grid->warps[wid] = frm_warp_create();
		warp = grid->warps[wid];
		thread_block = grid->thread_blocks[gid];

		warp->id = wid;
		warp->id_in_thread_block = wid % 
			grid->warps_per_thread_block;
		warp->grid = grid;
		warp->thread_block = thread_block;
                warp->inst_buffer = grid->function->inst_buffer;
                warp->inst_buffer_size = grid->function->inst_buffer_size;
		DOUBLE_LINKED_LIST_INSERT_TAIL(thread_block, running, warp);
	}

	/* Array of work-items */
	grid->thread_count = grid->grid_size;
	grid->thread_id_first = 0;
	grid->thread_id_last = grid->thread_count - 1;
	grid->threads = xcalloc(grid->thread_count, sizeof(void *));
	tid = 0;
	gid = 0;
	for (gidz = 0; gidz < grid->block_count3[2]; gidz++)
	{
		for (gidy = 0; gidy < grid->block_count3[1]; gidy++)
		{
			for (gidx = 0; gidx < grid->block_count3[0]; gidx++)
			{
				/* Assign work-group ID */
				thread_block = grid->thread_blocks[gid];
				thread_block->grid = grid;
				thread_block->id_3d[0] = gidx;
				thread_block->id_3d[1] = gidy;
				thread_block->id_3d[2] = gidz;
				thread_block->id = gid;
				frm_thread_block_set_status(thread_block, frm_thread_block_pending);

				/* First, last, and number of work-items in work-group */
				thread_block->thread_id_first = tid;
				thread_block->thread_id_last = tid + grid->block_size;
				thread_block->thread_count = grid->block_size;
				thread_block->threads = &grid->threads[tid];
				snprintf(thread_block->name, sizeof(thread_block->name), "work-group[i%d-i%d]",
					thread_block->thread_id_first, thread_block->thread_id_last);

				/* First ,last, and number of warps in work-group */
				thread_block->warp_id_first = gid * grid->warps_per_thread_block;
				thread_block->warp_id_last = thread_block->warp_id_first + grid->warps_per_thread_block - 1;
				thread_block->warp_count = grid->warps_per_thread_block;
				thread_block->warps = &grid->warps[thread_block->warp_id_first];
				/* Iterate through work-items */
				lid = 0;
				for (lidz = 0; lidz < grid->block_size3[2]; lidz++)
				{
					for (lidy = 0; lidy < grid->block_size3[1]; lidy++)
					{
						for (lidx = 0; lidx < grid->block_size3[0]; lidx++)
						{
							/* Wavefront ID */
							wid = gid * grid->warps_per_thread_block +
								lid / frm_emu_warp_size;
							assert(wid < grid->warp_count);
							warp = grid->warps[wid];
							
							/* Create work-item */
							grid->threads[tid] = frm_thread_create();
							thread = grid->threads[tid];
							thread->grid = grid;

							/* Global IDs */
							thread->id_3d[0] = gidx * grid->block_size3[0] + lidx;
							thread->id_3d[1] = gidy * grid->block_size3[1] + lidy;
							thread->id_3d[2] = gidz * grid->block_size3[2] + lidz;
							thread->id = tid;

							/* Local IDs */
							thread->id_in_thread_block_3d[0] = lidx;
							thread->id_in_thread_block_3d[1] = lidy;
							thread->id_in_thread_block_3d[2] = lidz;
							thread->id_in_thread_block = lid;

							/* Other */
							thread->id_in_warp = thread->id_in_thread_block % frm_emu_warp_size;
							thread->thread_block = grid->thread_blocks[gid];
							thread->warp = grid->warps[wid];

							/* First, last, and number of work-items in warp */
							if (!warp->thread_count)
							{
								warp->thread_id_first = tid;
								warp->threads = &grid->threads[tid];
							}
							warp->thread_count++;
							warp->thread_id_last = tid;

                                                        /* Save local IDs in register R0 */
                                                        thread->sr[FRM_SR_Tid_X].v.i = lidx;  /* R0.x */
                                                        thread->sr[FRM_SR_Tid_Y].v.i = lidy;  /* R0.y */
                                                        thread->sr[FRM_SR_Tid_Z].v.i = lidz;  /* R0.z */

                                                        /* Save thread_block IDs in register R1 */
                                                        thread->sr[FRM_SR_CTAid_X].v.i = gidx;  /* R1.x */
                                                        thread->sr[FRM_SR_CTAid_Y].v.i = gidy;  /* R1.y */
                                                        thread->sr[FRM_SR_CTAid_Z].v.i = gidz;  /* R1.z */

							/* Next work-item */
							tid++;
							lid++;
						}
					}
				}

				/* Next work-group */
				gid++;
			}
		}
	}

	/* Initialize the warps */
	for (wid = 0; wid < grid->warp_count; wid++)
	{
		/* Assign names to warps */
		warp = grid->warps[wid];
		snprintf(warp->name, sizeof(warp->name),
			"warp[i%d-i%d]",
			warp->thread_id_first,
			warp->thread_id_last);
	}

	/* Debug */
	frm_isa_debug("block_size = %d (%d,%d,%d)\n", grid->block_size,
		grid->block_size3[0], grid->block_size3[1],
		grid->block_size3[2]);
	frm_isa_debug("grid_size = %d (%d,%d,%d)\n", grid->grid_size,
		grid->grid_size3[0], grid->grid_size3[1],
		grid->grid_size3[2]);
	frm_isa_debug("block_count = %d (%d,%d,%d)\n", grid->block_count,
		grid->block_count3[0], grid->block_count3[1],
		grid->block_count3[2]);
	frm_isa_debug("warp_count = %d\n", grid->warp_count);
	frm_isa_debug("warps_per_thread_block = %d\n",
		grid->warps_per_thread_block);
	frm_isa_debug("\n");
}


void frm_grid_setup_size(struct frm_grid_t *grid,
		unsigned int *grid_size,
		unsigned int *block_size,
		int work_dim)
{
	int i;

	/* Default value */
	grid->grid_size3[1] = 1;
	grid->grid_size3[2] = 1;
	grid->block_size3[1] = 1;
	grid->block_size3[2] = 1;

	/* Grid sizes */
	for (i = 0; i < work_dim; i++)
		grid->grid_size3[i] = grid_size[i];
	grid->grid_size = grid->grid_size3[0] *
			grid->grid_size3[1] * grid->grid_size3[2];

	/* Thread block sizes */
	for (i = 0; i < work_dim; i++)
	{
		grid->block_size3[i] = block_size[i];
		if (grid->block_size3[i] < 1)
			fatal("%s: local work size must be greater than 0",
					__FUNCTION__);
	}
	grid->block_size = grid->block_size3[0] * grid->block_size3[1] * grid->block_size3[2];

	/* Check valid grid/thread block sizes */
	if (grid->grid_size3[0] < 1 || grid->grid_size3[1] < 1
			|| grid->grid_size3[2] < 1)
		fatal("%s: invalid global size", __FUNCTION__);
	if (grid->block_size3[0] < 1 || grid->block_size3[1] < 1
			|| grid->block_size3[2] < 1)
		fatal("%s: invalid local size", __FUNCTION__);

	/* Check divisibility of global by local sizes */
	if ((grid->grid_size3[0] % grid->block_size3[0])
			|| (grid->grid_size3[1] % grid->block_size3[1])
			|| (grid->grid_size3[2] % grid->block_size3[2]))
		fatal("%s: global work sizes must be multiples of local sizes",
				__FUNCTION__);

	/* Calculate number of groups */
	for (i = 0; i < 3; i++)
		grid->block_count3[i] = grid->grid_size3[i] / grid->block_size3[i];
	grid->block_count = grid->block_count3[0] * grid->block_count3[1] * grid->block_count3[2];

	/* Allocate work-group, warp, and thread arrays */
	frm_grid_setup_arrays(grid);
}


