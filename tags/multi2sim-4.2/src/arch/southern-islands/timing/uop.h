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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_UOP_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_UOP_H

#include <arch/southern-islands/emu/work-item.h>
#include <mem-system/module.h>


#define INST_NOT_FETCHED LLONG_MAX

/* Debugging */
#define si_stack_debug(...) debug(si_stack_debug_category, __VA_ARGS__)
extern int si_stack_debug_category;


/* Part of a GPU instruction specific for each work-item within wavefront. */
struct si_work_item_uop_t
{
	/* For global memory accesses */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Flags */
	unsigned int active : 1;  /* Active after instruction emulation */

	/* LDS accesses */
	int lds_access_count;
	enum mod_access_kind_t lds_access_kind[SI_MAX_LDS_ACCESSES_PER_INST];
	unsigned int lds_access_addr[SI_MAX_LDS_ACCESSES_PER_INST];
	unsigned int lds_access_size[SI_MAX_LDS_ACCESSES_PER_INST];
};

/* Structure representing a GPU instruction fetched in common for a wavefront.
 * This is the structure passed from stage to stage in the compute unit pipeline. */
struct si_uop_t
{
	/* Fields */
	long long id;
	long long id_in_compute_unit;
	long long id_in_wavefront;
	int wavefront_pool_id;
	struct si_wavefront_t *wavefront;       /* Wavefront it belongs to */
	struct si_work_group_t *work_group;     /* Work-group it belongs to */
	struct si_compute_unit_t *compute_unit; /* Compute unit it belongs to */
	struct si_wavefront_pool_entry_t *wavefront_pool_entry;  /* IB entry where uop is located */
	struct si_inst_t inst;

	/* Flags */
	unsigned int ready : 1;
	unsigned int mem_wait_inst : 1;
	unsigned int barrier_wait_inst : 1;
	unsigned int wavefront_last_inst : 1;   /* Last instruction in the WF */
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
	struct si_work_item_uop_t work_item_uop[0];
};


void si_uop_init(void);
void si_uop_done(void);

struct si_uop_t *si_uop_create(void);
void si_uop_free(struct si_uop_t *gpu_uop);

void si_uop_list_free(struct list_t *uop_list);

#endif
