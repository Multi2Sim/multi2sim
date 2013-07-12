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

#include <arch/fermi/asm/asm.h>

#include <lib/util/string.h>


#define FRM_MAX_STACK_SIZE  32

struct frm_warp_t
{
	/* ID */
	int id;
	int id_in_thread_block;

	/* Name */
	char name[MAX_STRING_SIZE];

	/* Grid and thread_block it belongs to */
	struct frm_grid_t *grid;
	struct frm_thread_block_t *thread_block;

	/* Threads */
	int thread_count;
	struct frm_thread_t **threads;

	/* PC */
	unsigned int pc;
	int inst_size;

	/* Current instructions */
	struct frm_inst_t inst;

	/* Starting/current position in buffer */
	unsigned long long int *inst_buffer;
	unsigned int inst_buffer_index;
	unsigned int inst_buffer_size;

	/* Active mask stack */
	struct bit_map_t *active_stack;  /* FRM_MAX_STACK_SIZE * thread_count elements */
	int stack_top;

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
	struct frm_warp_t *running_list_next;
	struct frm_warp_t *running_list_prev;
	struct frm_warp_t *barrier_list_next;
	struct frm_warp_t *barrier_list_prev;
	struct frm_warp_t *finished_list_next;
	struct frm_warp_t *finished_list_prev;

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
};

#define FRM_FOREACH_WARP_IN_GRID(GRID, WARP_ID) \
	for ((WARP_ID) = (GRID)->warp_id_first; \
		(WARP_ID) <= (GRID)->warp_id_last; \
		(WARP_ID)++)
#define FRM_FOREACH_WARP_IN_THREADBLOCK(THREADBLOCK, WARP_ID) \
	for ((WARP_ID) = (THREADBLOCK)->warp_id_first; \
		(WARP_ID) <= (THREADBLOCK)->warp_id_last; \
		(WARP_ID)++)
struct frm_warp_t *frm_warp_create(void);
void frm_warp_free(struct frm_warp_t *warp);
void frm_warp_dump(struct frm_warp_t *warp, FILE *f);
void frm_warp_set_name(struct frm_warp_t *warp, char *name);
void frm_warp_stack_push(struct frm_warp_t *warp);
void frm_warp_stack_pop(struct frm_warp_t *warp, int count);
void frm_warp_execute(struct frm_warp_t *warp);


#endif

