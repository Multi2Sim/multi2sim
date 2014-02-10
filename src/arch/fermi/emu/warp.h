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

#ifndef FERMI_EMU_WARP_H
#define FERMI_EMU_WARP_H

#include <lib/class/class.h>


/*
 * Class 'FrmWarp'
 */

#define FRM_WARP_SYNC_STACK_SIZE  64

typedef struct
{
	unsigned reconv_pc;
	struct bit_map_t *active_thread_mask;
} FrmWarpSyncStackEntry;

typedef struct
{
	FrmWarpSyncStackEntry entries[FRM_WARP_SYNC_STACK_SIZE];
} FrmWarpSyncStack;

CLASS_BEGIN(FrmWarp, Object)

	/* ID */
	int id;
	int id_in_thread_block;

	/* Grid and thread-block it belongs to */
	FrmGrid *grid;
	FrmThreadBlock *thread_block;

	/* Threads */
	int thread_count;
	FrmThread **threads;

	/* PC */
	unsigned pc;
	unsigned target_pc;
	unsigned cont_pc;

	/* Instructions to execute */
	struct FrmInstWrap *inst;
	unsigned inst_size;
	unsigned long long *inst_buffer;
	unsigned inst_buffer_size;

	/* Synchronization stack */
	FrmWarpSyncStack sync_stack;
	int sync_stack_top;

	/* Flags updated during instruction execution */
	struct bit_map_t *taken_thread_map;
	unsigned at_barrier;
	struct bit_map_t *finished_thread_map;
	unsigned finished;

	/* Fields below are used for architectural simulation only */

	int id_in_sm;
	int uop_id_counter;

	/* Warp instruction queues */
	struct frm_warp_inst_queue_t *warp_inst_queue;
	struct frm_warp_inst_queue_entry_t *warp_inst_queue_entry;

	/* Statistics */
	long long inst_count;

CLASS_END(FrmWarp)


void FrmWarpCreate(FrmWarp *self, int id, FrmThreadBlock *thread_block,
		FrmGrid *grid);
void FrmWarpDestroy(FrmWarp *self);

void FrmWarpDump(FrmWarp *self, FILE *f);
void FrmWarpExecute(FrmWarp *self);


#endif

