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

#include <lib/class/class.h>
#include <lib/util/string.h>


/*
 * Class 'FrmThreadBlock'
 */

typedef enum
{
	FrmThreadBlockInvalid = 0,
	FrmThreadBlockPending = 0x0001,
	FrmThreadBlockRunning = 0x0002,
	FrmThreadBlockFinished = 0x0004
} FrmThreadBlockState;


CLASS_BEGIN(FrmThreadBlock, Object)

	/* ID */
	int id;
	int id_3d[3];

	/* Name */
	char name[MAX_STRING_SIZE];

	/* Status */
	FrmThreadBlockState state;

	/* Grid it belongs to */
	FrmGrid *grid;

	/* SM it is mapped to */
	struct frm_sm_t *sm;

	/* Double linked lists of thread_blocks */
	FrmThreadBlock *pending_list_prev;
	FrmThreadBlock *pending_list_next;
	FrmThreadBlock *running_list_prev;
	FrmThreadBlock *running_list_next;
	FrmThreadBlock *finished_list_prev;
	FrmThreadBlock *finished_list_next;

	/* Array of warps */
	int warp_count;
	FrmWarp **warps;

	/* List of warps */
	struct list_t *running_warps;
	struct list_t *finished_warps;

	/* Array of threads */
	int thread_count;
	FrmThread **threads;

	/* Barrier information */
	unsigned int num_warps_at_barrier;

	/* Fields introduced for architectural simulation */
	int id_in_sm;
	int sm_finished_count;  /* like 'finished_list_count', but when warp reaches Complete stage */

	/* Shared memory */
	struct mem_t *shared_mem;

CLASS_END(FrmThreadBlock)


#define FRM_FOR_EACH_THREADBLOCK_IN_GRID(GRID, THREADBLOCK_ID) \
	for ((THREADBLOCK_ID) = (GRID)->thread_block_id_first; \
		(THREADBLOCK_ID) <= (GRID)->thread_block_id_last; \
		(THREADBLOCK_ID)++)

void FrmThreadBlockCreate(FrmThreadBlock *self);
void FrmThreadBlockDestroy(FrmThreadBlock *self);

void FrmThreadBlockDump(FrmThreadBlock *self, FILE *f);


#endif

