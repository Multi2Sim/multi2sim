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

#ifndef ARCH_FERMI_TIMING_UOP_H
#define ARCH_FERMI_TIMING_UOP_H

#include <arch/fermi/emu/thread.h>
#include <mem-system/module.h>


#define INST_NOT_FETCHED LLONG_MAX

/* Debugging */
#define frm_stack_debug(...) debug(frm_stack_debug_category, __VA_ARGS__)
extern int frm_stack_debug_category;


/* Part of a GPU instruction specific for each thread within warp. */
struct frm_thread_uop_t
{
	/* For global memory accesses */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Flags */
	unsigned int active : 1;  /* Active after instruction emulation */

	/* LDS accesses */
	int lds_access_count;
	enum mod_access_kind_t lds_access_kind[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	unsigned int lds_access_addr[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	unsigned int lds_access_size[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
};

/* Structure representing a GPU instruction fetched in common for a warp.
 * This is the structure passed from stage to stage in the compute unit pipeline. */
struct frm_uop_t
{
	/* Fields */
	long long id;
	long long id_in_sm;
	long long id_in_warp;
	int warp_inst_queue_id;
	struct frm_warp_t *warp;       /* Wavefront it belongs to */
	struct frm_thread_block_t *thread_block;     /* Work-group it belongs to */
	struct frm_sm_t *sm; /* Compute unit it belongs to */
	struct frm_warp_inst_queue_entry_t *warp_inst_queue_entry;  /* IB entry where uop is located */
	struct frm_inst_t inst;

	/* Flags */
	unsigned int ready : 1;
	unsigned int mem_wait_inst : 1;
	unsigned int barrier_wait_inst : 1;
	unsigned int warp_last_inst : 1;   /* Last instruction in the warp */
	unsigned int vector_mem_read : 1;
	unsigned int vector_mem_write : 1;
	unsigned int scalar_mem_read : 1;
	unsigned int lds_read : 1;
	unsigned int lds_write : 1;
	unsigned int exec_mask_update : 1;

	unsigned int glc : 1;

	/* Timing */
	long long cycle_created;    /* Cycle when fetch completes */
	long long fetch_ready;      /* Cycle when fetch completes */
	long long decode_ready;     /* Cycle when decode completes */
	long long issue_ready;      /* Cycle when issue completes */
	long long read_ready;       /* Cycle when register access completes */
	long long execute_ready;    /* Cycle when execution completes */
	long long write_ready;      /* Cycle when writeback completes */

	/* Witness memory accesses */
	int global_mem_witness;
	int lds_witness;
	
	/* Added for Profiling reports*/
	int num_global_mem_read ;
	int num_global_mem_write ;


	/* Last scalar memory accesses */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Per stream-core data. This space is dynamically allocated for an uop.
	 * It should be always the last field of the structure. */
	struct frm_thread_uop_t thread_uop[0];
};


void frm_uop_init(void);
void frm_uop_done(void);

struct frm_uop_t *frm_uop_create(void);
void frm_uop_free(struct frm_uop_t *gpu_uop);

void frm_uop_list_free(struct list_t *uop_list);

#endif
