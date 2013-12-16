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

#include <arch/fermi/asm/Wrapper.h>
#include <lib/class/class.h>
#include <lib/util/string.h>


/*
 * Class 'FrmWarp'
 */

#define FRM_WARP_SYNC_STACK_SIZE  32

typedef struct
{
	unsigned int reconv_pc;
	unsigned int next_path_pc;
	unsigned int active_thread_mask;
} FrmWarpSyncStackEntry;


typedef struct
{
	FrmWarpSyncStackEntry entries[FRM_WARP_SYNC_STACK_SIZE];
} FrmWarpSyncStack;


CLASS_BEGIN(FrmWarp, Object)

	/* ID */
	int id;
	int id_in_thread_block;

	/* Name */
	char name[MAX_STRING_SIZE];

	/* Grid and thread_block it belongs to */
	FrmGrid *grid;
	FrmThreadBlock *thread_block;

	/* Threads */
	int thread_count;
	FrmThread **threads;

	/* PC */
	unsigned int pc;
	int inst_size;

	/* Current instructions */
	struct FrmInstWrap *inst;

	/* Starting/current position in buffer */
	unsigned long long int *inst_buffer;
	unsigned int inst_buffer_index;
	unsigned int inst_buffer_size;

	/* Sync stack */
	FrmWarpSyncStackEntry new_entry;
	FrmWarpSyncStack sync_stack;
	int sync_stack_top;
	int sync_stack_pushed;
	int sync_stack_popped;
	unsigned int divergent;
	unsigned int taken;

	unsigned int at_barrier_thread_count;
	unsigned int finished_thread_count;

	/* Predicate mask */
	struct bit_map_t *pred;  /* thread_count elements */

	/* Flags updated during instruction execution */
	unsigned int vector_mem_read : 1;
	unsigned int vector_mem_write : 1;
	unsigned int scalar_mem_read : 1;
	unsigned int lds_read : 1;
	unsigned int lds_write : 1;
	unsigned int mem_wait : 1;
	unsigned int at_barrier : 1;
	unsigned int barrier : 1;
	unsigned int finished : 1;
	unsigned int vector_mem_glc : 1;

	/* Loop counters */
	/* FIXME: Include this as part of the stack to handle nested loops */
	int loop_depth;
	int loop_max_trip_count;
	int loop_trip_count;
	int loop_start;
	int loop_step;
	int loop_index;

	/* Flags updated during instruction execution */
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int pred_mask_update : 1;
	unsigned int push_before_done : 1;  /* Indicates whether the stack has been pushed after PRED_SET* instr. */
	unsigned int active_mask_update : 1;
	int active_mask_push;  /* Number of entries the stack was pushed */
	int active_mask_pop;  /* Number of entries the stack was popped */

	/* Linked lists */
	FrmWarp *running_list_next;
	FrmWarp *running_list_prev;
	FrmWarp *barrier_list_next;
	FrmWarp *barrier_list_prev;
	FrmWarp *finished_list_next;
	FrmWarp *finished_list_prev;

	/* To measure simulation performance */
	long long emu_inst_count;  /* Total emulated instructions */
	long long emu_time_start;
	long long emu_time_end;


	/* Fields introduced for architectural simulation */
	int id_in_sm;
	int alu_engine_in_flight;  /* Number of in-flight uops in ALU engine */
	long long sched_when;  /* GPU cycle when warp was last scheduled */
	int uop_id_counter;
	struct frm_warp_inst_queue_t *warp_inst_queue;
	struct frm_warp_inst_queue_entry_t *warp_inst_queue_entry;


	/* Periodic report - used by architectural simulation */
	FILE *periodic_report_file;  /* File where report is dumped */
	long long periodic_report_cycle;  /* Last cycle when periodic report was updated */
	int periodic_report_inst_count;  /* Number of instructions in this interval */
	int periodic_report_local_mem_accesses;  /* Number of local memory accesses in this interval */
	int periodic_report_global_mem_writes;  /* Number of global memory writes in this interval */
	int periodic_report_global_mem_reads;  /* Number of global memory reads in this interval */


	/* Statistics */
	long long inst_count;  /* Total number of instructions */
	long long global_mem_inst_count;  /* Instructions accessing global memory */
	long long local_mem_inst_count;  /* Instructions accessing local memory */

CLASS_END(FrmWarp)


#define FRM_FOREACH_WARP_IN_GRID(GRID, WARP_ID) \
	for ((WARP_ID) = (GRID)->warp_id_first; \
		(WARP_ID) <= (GRID)->warp_id_last; \
		(WARP_ID)++)
#define FRM_FOREACH_WARP_IN_THREADBLOCK(THREADBLOCK, WARP_ID) \
	for ((WARP_ID) = (THREADBLOCK)->warp_id_first; \
		(WARP_ID) <= (THREADBLOCK)->warp_id_last; \
		(WARP_ID)++)


void FrmWarpCreate(FrmWarp *self, FrmThreadBlock *thread_block);
void FrmWarpDestroy(FrmWarp *self);

void FrmWarpDump(FrmWarp *self, FILE *f);

void FrmWarpPush(FrmWarp *self);
void FrmWarpPop(FrmWarp *self, int count);

void FrmWarpExecute(FrmWarp *self);


#endif
