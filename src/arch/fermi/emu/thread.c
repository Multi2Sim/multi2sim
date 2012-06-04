/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <stdio.h>
#include <string.h>
#include <list.h>
#include <debug.h>
#include <misc.h>
#include <elf-format.h>

#include <fermi-asm.h>



/*
 * CUDA thread (Pixel)
 */

#define FRM_MAX_GPR_ELEM  5
#define FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST  2

struct frm_gpr_t
{
	uint32_t elem[FRM_MAX_GPR_ELEM];  /* x, y, z, w, t */
};

/* Structure describing a memory access definition */
struct frm_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	uint32_t addr;
	int size;
};

struct frm_thread_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_warp;
	int id_in_threadblock;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_threadblock_3d[3];  /* local 3D IDs */

	/* Warp, Threadblock, and Grid where it belongs */
	struct frm_warp_t *warp;
	struct frm_threadblock_t *threadblock;
	struct frm_grid_t *grid;

	/* Thread state */
	struct frm_gpr_t gpr[128];  /* General purpose registers */
	struct frm_gpr_t pv;  /* Result of last computations */

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct linked_list_t *write_task_list;

	/* LDS (Local Data Share) OQs (Output Queues) */
	struct list_t *lds_oqa;
	struct list_t *lds_oqb;

	/* This is a digest of the active mask updates for this thread. Every time
	 * an instruction updates the active mask of a warp, this digest is updated
	 * for active threads by XORing a random number common for the warp.
	 * At the end, threads with different 'branch_digest' numbers can be considered
	 * divergent threads. */
	uint32_t branch_digest;

	/* Last global memory access */
	uint32_t global_mem_access_addr;
	uint32_t global_mem_access_size;

	/* Last local memory access */
	int local_mem_access_count;  /* Number of local memory access performed by last instruction */
	uint32_t local_mem_access_addr[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_size[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	int local_mem_access_type[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */
};

#define FRM_FOREACH_THREAD_IN_GRID(GRID, THREAD_ID) \
	for ((THREAD_ID) = (GRID)->thread_id_first; \
		(THREAD_ID) <= (GRID)->thread_id_last; \
		(THREAD_ID)++)

#define FRM_FOREACH_THREAD_IN_THREADBLOCK(THREADBLOCK, THREAD_ID) \
	for ((THREAD_ID) = (THREADBLOCK)->thread_id_first; \
		(THREAD_ID) <= (THREADBLOCK)->thread_id_last; \
		(THREAD_ID)++)

#define FRM_FOREACH_THREAD_IN_WARP(WARP, THREAD_ID) \
	for ((THREAD_ID) = (WARP)->thread_id_first; \
		(THREAD_ID) <= (WARP)->thread_id_last; \
		(THREAD_ID)++)

struct frm_thread_t *frm_thread_create(void);
void frm_thread_free(struct frm_thread_t *thread);

/* Consult and change active/predicate bits */
void frm_thread_set_active(struct frm_thread_t *thread, int active);
int frm_thread_get_active(struct frm_thread_t *thread);
void frm_thread_set_pred(struct frm_thread_t *thread, int pred);
int frm_thread_get_pred(struct frm_thread_t *thread);
void frm_thread_update_branch_digest(struct frm_thread_t *thread,
	long long inst_count, uint32_t inst_addr);

