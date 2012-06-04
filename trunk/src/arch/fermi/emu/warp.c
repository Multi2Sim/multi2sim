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
 * CUDA Warp
 */

#define FRM_MAX_STACK_SIZE  32

/* Warp */
struct frm_warp_t
{
	/* ID */
	char name[30];
	int id;
	int id_in_threadblock;

	/* IDs of threads it contains */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* Grid and Threadblock it belongs to */
	struct frm_grid_t *grid;
	struct frm_threadblock_t *threadblock;

	/* Pointer to threads */
	struct frm_thread_t **threads;  /* Pointer to first threads in 'kernel->threads' */

	/* Current instructions */
	struct frm_inst_t cf_inst;
	struct frm_inst_t tex_inst;

	/* Starting/current CF buffer */
	void *cf_buf_start;
	void *cf_buf;

	/* Active mask stack */
	struct bit_map_t *active_stack;  /* FRM_MAX_STACK_SIZE * thread_count elements */
	int stack_top;

	/* Predicate mask */
	struct bit_map_t *pred;  /* thread_count elements */

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
	int id_in_compute_unit;
	int alu_engine_in_flight;  /* Number of in-flight uops in ALU engine */
	long long sched_when;  /* GPU cycle when warp was last scheduled */


	/* Periodic report - used by architectural simulation */

	FILE *periodic_report_file;  /* File where report is dumped */
	long long periodic_report_vliw_bundle_count;  /* Number of VLIW bundles (or non-ALU instructions) reported */
	long long periodic_report_cycle;  /* Last cycle when periodic report was updated */
	int periodic_report_inst_count;  /* Number of instructions (VLIW slots) in this interval */
	int periodic_report_local_mem_accesses;  /* Number of local memory accesses in this interval */


	/* Statistics */

	long long inst_count;  /* Total number of instructions */
	long long global_mem_inst_count;  /* Instructions (CF or TC) accessing global memory */
	long long local_mem_inst_count;  /* Instructions (ALU) accessing local memory */

	long long cf_inst_count;  /* Number of CF inst executed */
	long long cf_inst_global_mem_write_count;  /* Number of instructions writing to global mem (they are CF inst) */

	long long alu_group_count;  /* Number of ALU instruction groups (VLIW) */
	long long alu_group_size[5];  /* Distribution of group sizes (alu_group_size[0] is the number of groups with 1 inst) */
	long long alu_inst_count;  /* Number of ALU instructions */
	long long alu_inst_local_mem_count;  /* Instructions accessing local memory (ALU) */

	long long tc_inst_count;
	long long tc_inst_global_mem_read_count;  /* Number of instructions reading from global mem (they are TC inst) */
};

#define FRM_FOREACH_WARP_IN_GRID(GRID, WARP_ID) \
	for ((WARP_ID) = (GRID)->warp_id_first; \
		(WARP_ID) <= (GRID)->warp_id_last; \
		(WARP_ID)++)

#define FRM_FOREACH_WARP_IN_THREADBLOCK(THREADBLOCK, WARP_ID) \
	for ((WARP_ID) = (THREADBLOCK)->warp_id_first; \
		(WARP_ID) <= (THREADBLOCK)->warp_id_last; \
		(WARP_ID)++)

struct frm_warp_t *frm_warp_create();
void frm_warp_free(struct frm_warp_t *warp);
void frm_warp_dump(struct frm_warp_t *warp, FILE *f);

void frm_warp_stack_push(struct frm_warp_t *warp);
void frm_warp_stack_pop(struct frm_warp_t *warp, int count);
void frm_warp_execute(struct frm_warp_t *warp);

