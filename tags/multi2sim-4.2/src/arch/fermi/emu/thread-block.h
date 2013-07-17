/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FERMI_EMU_THREADBLOCK_H
#define FERMI_EMU_THREADBLOCK_H

#include <lib/util/string.h>


enum frm_thread_block_status_t
{
	frm_thread_block_pending	= 0x0001,
	frm_thread_block_running	= 0x0002,
	frm_thread_block_finished	= 0x0004
};

struct frm_thread_block_t
{
	/* ID */
	int id;
	int id_3d[3];

	/* Name */
	char name[MAX_STRING_SIZE];

	/* Status */
	enum frm_thread_block_status_t status;

	/* Grid it belongs to */
	struct frm_grid_t *grid;

	/* SM it is mapped to */
	struct frm_sm_t *sm;

	/* Double linked lists of thread_blocks */
	struct frm_thread_block_t *pending_list_prev;
	struct frm_thread_block_t *pending_list_next;
	struct frm_thread_block_t *running_list_prev;
	struct frm_thread_block_t *running_list_next;
	struct frm_thread_block_t *finished_list_prev;
	struct frm_thread_block_t *finished_list_next;

	/* Array of warps */
	int warp_count;
	struct frm_warp_t **warps;

	/* List of warps */
	struct list_t *running_warps;
	struct list_t *finished_warps;

	/* Array of threads */
	int thread_count;
	struct frm_thread_t **threads;

	/* Barrier information */
	unsigned int num_warps_at_barrier;

	/* Fields introduced for architectural simulation */
	int id_in_sm;
	int sm_finished_count;  /* like 'finished_list_count', but when warp reaches Complete stage */

	/* Shared memory */
	struct mem_t *shared_mem;
};

#define FRM_FOR_EACH_THREADBLOCK_IN_GRID(GRID, THREADBLOCK_ID) \
	for ((THREADBLOCK_ID) = (GRID)->thread_block_id_first; \
		(THREADBLOCK_ID) <= (GRID)->thread_block_id_last; \
		(THREADBLOCK_ID)++)

struct frm_thread_block_t *frm_thread_block_create(void);
void frm_thread_block_free(struct frm_thread_block_t *thread_block);
void frm_thread_block_dump(struct frm_thread_block_t *thread_block, FILE *f);


#endif

