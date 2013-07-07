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

#ifndef ARCH_FERMI_TIMING_SM_H
#define ARCH_FERMI_TIMING_SM_H

#include "branch-unit.h"
#include "lds-unit.h"
#include "vector-mem-unit.h"


struct frm_reg_file_t;

void frm_reg_file_init(struct frm_sm_t *sm);
void frm_reg_file_done(struct frm_sm_t *sm);

struct frm_thread_block_t;
void frm_reg_file_map_thread_block(struct frm_sm_t *sm,
	struct frm_thread_block_t *thread_block);
void frm_reg_file_unmap_thread_block(struct frm_sm_t *sm,
	struct frm_thread_block_t *thread_block);

struct frm_thread_t;
int frm_reg_file_rename(struct frm_sm_t *sm,
	struct frm_thread_t *thread, int logical_register);
void frm_reg_file_inverse_rename(struct frm_sm_t *sm,
	int physical_register, struct frm_thread_t **thread, int *logical_register);



struct frm_sm_t
{
	/* IDs */
	int id;
	long long uop_id_counter;  

	/* Double linked list of compute units */
	struct frm_sm_t *sm_ready_list_prev;
	struct frm_sm_t *sm_ready_list_next;
	struct frm_sm_t *sm_busy_list_prev;
	struct frm_sm_t *sm_busy_list_next;

	/* Entry points to memory hierarchy */
	struct mod_t *global_memory;
	struct mod_t *lds_module;

	/* Hardware structures */
	unsigned int num_warp_inst_queues;
	struct frm_warp_inst_queue_t **warp_inst_queues;
	struct list_t **fetch_buffers;
	unsigned int num_simd_units;
	struct frm_simd_t **simd_units;
	/* TODO Make these into a configurable number of structures */
	struct frm_branch_unit_t branch_unit;
	struct frm_vector_mem_unit_t vector_mem_unit;
	struct frm_lds_t lds_unit;

	/* Statistics */
	long long cycle;
	long long mapped_thread_blocks;
	long long warp_count;
	long long inst_count; /* Total instructions */
	long long branch_inst_count;
	/* TODO Have one SIMD inst count per SIMD unit */
	long long simd_inst_count;
	long long vector_mem_inst_count;
	long long lds_inst_count;

	/* List of currently mapped thread blocks */
	struct frm_thread_block_t **thread_blocks;
	int thread_block_count;

	/* List of currently mapped warps */
	struct frm_warp_t **warps;

	/* Compute Unit capacity state  (for concurrent CQ)*/
	/* Statistics for spatial report */

	/* Spatial profiling statistics */
	long long interval_cycle;
	long long interval_mapped_thread_blocks;
	FILE * spatial_report_file;


};

struct frm_sm_t *frm_sm_create(void);
void frm_sm_free(struct frm_sm_t *gpu_sm);
void frm_sm_map_thread_block(struct frm_sm_t *sm, 
	struct frm_thread_block_t *thread_block);
void frm_sm_unmap_thread_block(struct frm_sm_t *sm, 
	struct frm_thread_block_t *thread_block);
struct frm_warp_t *frm_sm_schedule(struct frm_sm_t *sm);
void frm_sm_run(struct frm_sm_t *sm);

struct frm_warp_inst_queue_t *frm_warp_inst_queue_create();
void frm_warp_inst_queue_free(struct frm_warp_inst_queue_t *warp_inst_queue);
void frm_warp_inst_queue_map_warps(struct frm_warp_inst_queue_t *warp_inst_queue, 
	struct frm_thread_block_t *thread_block);
void frm_warp_inst_queue_unmap_warps(struct frm_warp_inst_queue_t *warp_inst_queue, 
	struct frm_thread_block_t *thread_block);

#endif
