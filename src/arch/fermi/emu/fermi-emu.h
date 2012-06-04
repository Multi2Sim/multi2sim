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

#ifndef FERMI_EMU_H
#define FERMI_EMU_H

#include <stdio.h>
#include <string.h>
#include <list.h>
#include <debug.h>
#include <misc.h>
#include <elf-format.h>

#include <fermi-asm.h>



/* Execute CUDA call */
int frm_cuda_call();



/*
 * CUDA Grid
 */

struct frm_grid_t
{
	/* ID */
	char *name;
	int id;  /* Sequential grid ID (given by frm_emu->grid_count counter) */

	/* CUDA kernel associated */
	struct frm_cuda_kernel_t *kernel;

	/* Pointers to threadblocks, warps, and threads */
	struct frm_threadblock_t **threadblocks;
	struct frm_warp_t **warps;
	struct frm_thread_t **threads;

	/* IDs of threads contained */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* IDs of warps contained */
	int warp_id_first;
	int warp_id_last;
	int warp_count;

	/* IDs of threadblocks contained */
	int threadblock_id_first;
	int threadblock_id_last;
	int threadblock_count;
	
	/* Size of threadblocks */
	int warps_per_threadblock;  /* = ceil(local_size / frm_emu_warp_size) */

	/* List of pending threadblocks */
	struct frm_threadblock_t *pending_list_head;
	struct frm_threadblock_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running threadblocks */
	struct frm_threadblock_t *running_list_head;
	struct frm_threadblock_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished threadblocks */
	struct frm_threadblock_t *finished_list_head;
	struct frm_threadblock_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in kernel function. */
	uint32_t local_mem_top;
};

struct frm_grid_t *frm_grid_create(struct frm_cuda_kernel_t *kernel);
void frm_grid_free(struct frm_grid_t *grid);
void frm_grid_dump(struct frm_grid_t *grid, FILE *f);

void frm_grid_setup_threads(struct frm_grid_t *grid);
void frm_grid_setup_const_mem(struct frm_grid_t *grid);
void frm_grid_setup_args(struct frm_grid_t *grid);
void frm_grid_run(struct frm_grid_t *grid);




/*
 * CUDA Threadblock
 */

enum frm_threadblock_status_t
{
	frm_threadblock_pending		= 0x0001,
	frm_threadblock_running		= 0x0002,
	frm_threadblock_finished	= 0x0004
};

struct frm_threadblock_t
{
	char name[30];

	/* ID */
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum frm_threadblock_status_t status;

	/* Grid it belongs to */
	struct frm_grid_t *grid;

	/* IDs of threads contained */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* IDs of warps contained */
	int warp_id_first;
	int warp_id_last;
	int warp_count;

	/* Pointers to warps and threads */
	struct frm_thread_t **threads;  /* Pointer to first thread in 'kernel->threads' */
	struct frm_warp_t **warps;  /* Pointer to first warp in 'kernel->warps' */

	/* Double linked lists of threadblocks */
	struct frm_threadblock_t *pending_list_prev;
	struct frm_threadblock_t *pending_list_next;
	struct frm_threadblock_t *running_list_prev;
	struct frm_threadblock_t *running_list_next;
	struct frm_threadblock_t *finished_list_prev;
	struct frm_threadblock_t *finished_list_next;

	/* List of running warps */
	struct frm_warp_t *running_list_head;
	struct frm_warp_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of warps in barrier */
	struct frm_warp_t *barrier_list_head;
	struct frm_warp_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished warps */
	struct frm_warp_t *finished_list_head;
	struct frm_warp_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define FRM_FOR_EACH_THREADBLOCK_IN_GRID(GRID, THREADBLOCK_ID) \
	for ((THREADBLOCK_ID) = (GRID)->threadblock_id_first; \
		(THREADBLOCK_ID) <= (GRID)->threadblock_id_last; \
		(THREADBLOCK_ID)++)

struct frm_threadblock_t *frm_threadblock_create();
void frm_threadblock_free(struct frm_threadblock_t *threadblock);
void frm_threadblock_dump(struct frm_threadblock_t *threadblock, FILE *f);

int frm_threadblock_get_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status);
void frm_threadblock_set_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status);
void frm_threadblock_clear_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status);




/*
 * GPU Warp
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




/*
 * GPU thread (Pixel)
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



void frm_emu_disasm(char *path);

#endif

