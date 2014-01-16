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

#ifndef KEPLER_EMU_WARP_H
#define KEPLER_EMU_WARP_H


#include <lib/class/class.h>


/*
 * Class 'KplWarp'
 */

#define KPL_WARP_SYNC_STACK_SIZE  32

typedef struct
{
	unsigned reconv_pc;
	unsigned next_path_pc;
	struct bit_map_t *active_thread_mask;
} KplWarpSyncStackEntry;


typedef struct
{
	KplWarpSyncStackEntry entries[KPL_WARP_SYNC_STACK_SIZE];
} KplWarpSyncStack;


CLASS_BEGIN(KplWarp, Object)

	/* ID */
	int id;
	int id_in_thread_block;

	/* Grid and thread_block it belongs to */
	KplGrid *grid;
	KplThreadBlock *thread_block;

	/* Threads */
	int thread_count;
	KplThread **threads;

	/* PC */
	unsigned pc;
	int target_pc;
	struct bit_map_t *taken_threads;

	/* Instructions to execute */
	struct KplInstWrap *inst;
	unsigned inst_size;
	unsigned long long *inst_buffer;
	unsigned inst_buffer_size;

	/* Synchronization stack */
	KplWarpSyncStack sync_stack;
	int sync_stack_top;

	/* Flags updated during instruction execution */
	unsigned at_barrier;
	unsigned divergent;
	unsigned finished_thread_count;
	unsigned finished;

	/* Fields below are used for architectural simulation only */

	int id_in_sm;
	int uop_id_counter;

	/* Warp instruction queues */
	struct kpl_warp_inst_queue_t *warp_inst_queue;
	struct kpl_warp_inst_queue_entry_t *warp_inst_queue_entry;

	/* Statistics */
	long long inst_count;

CLASS_END(KplWarp)


void KplWarpCreate(KplWarp *self, int id, KplThreadBlock *thread_block,
		KplGrid *grid);
void KplWarpDestroy(KplWarp *self);

void KplWarpDump(KplWarp *self, FILE *f);
void KplWarpExecute(KplWarp *self);


#endif

