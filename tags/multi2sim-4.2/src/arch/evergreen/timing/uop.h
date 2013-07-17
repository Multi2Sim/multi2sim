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

#ifndef ARCH_EVERGREEN_TIMING_UOP_H
#define ARCH_EVERGREEN_TIMING_UOP_H

#include <arch/evergreen/emu/work-item.h>
#include <mem-system/module.h>


/* Debugging */
#define evg_stack_debug(...) debug(evg_stack_debug_category, __VA_ARGS__)
extern int evg_stack_debug_category;


/* Part of a GPU instruction specific for each work-item within wavefront. */
struct evg_work_item_uop_t
{
	/* For global memory accesses */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Flags */
	unsigned int active : 1;  /* Active after instruction emulation */

	/* Local memory access */
	int local_mem_access_count;
	enum mod_access_kind_t local_mem_access_kind[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	unsigned int local_mem_access_addr[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	unsigned int local_mem_access_size[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
};

#define EVG_UOP_MAX_IDEP      (3 * 5)
#define EVG_UOP_MAX_ODEP      (3 * 5)

#define EVG_UOP_DEP_NONE         0
#define EVG_UOP_DEP_REG_FIRST    1
#define EVG_UOP_DEP_REG(X)       ((X) + 1)
#define EVG_UOP_DEP_REG_LAST     128
#define EVG_UOP_DEP_PV           129
#define EVG_UOP_DEP_PS           130
#define EVG_UOP_DEP_LDS          131
#define EVG_UOP_DEP_COUNT        132

/* Structure representing a GPU instruction fetched in common for a wavefront.
 * This is the structure passed from stage to stage in the compute unit pipeline. */
struct evg_uop_t
{
	/* Fields */
	long long id;
	long long id_in_compute_unit;
	struct evg_wavefront_t *wavefront;  /* Wavefront it belongs to */
	struct evg_work_group_t *work_group;  /* Work-group it belongs to */
	struct evg_compute_unit_t *compute_unit;  /* Compute unit it belongs to */
	struct evg_uop_t *cf_uop;  /* For ALU/TEX uops, CF uop that triggered clause */
	int length;  /* Number of bytes occupied by ALU group */
	int vliw_slots;  /* Number of slots for ALU group, or 1 for CF/TEX instructions */

	/* CF instruction flags */
	unsigned int alu_clause_trigger : 1;  /* Instruction triggers ALU clause */
	unsigned int tex_clause_trigger : 1;  /* Instruction triggers TEX clause */
	unsigned int no_clause_trigger : 1;  /* Instruction does not trigger secondary clause */

	/* ALU group flags */
	unsigned int ready : 1;

	/* Flags */
	unsigned int last : 1;  /* Last instruction in the clause */
	unsigned int wavefront_last : 1;  /* Last instruction in the wavefront */
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int active_mask_update : 1;
	int active_mask_push;  /* Number of entries the stack was pushed */
	int active_mask_pop;  /* Number of entries the stack was popped */
	int active_mask_stack_top;  /* Top of stack */

	/* Witness memory accesses */
	long long inst_mem_ready;  /* Cycle when instruction memory access completes */
	int global_mem_witness;
	int local_mem_witness;
	/* Added for Profiling reports*/
	int num_global_mem_read ;
	int num_global_mem_write ;

	/* ALU Engine - subwavefronts */
	int subwavefront_count;
	int exec_subwavefront_count;
	int write_subwavefront_count;

	/* ALU instructions - input/output dependencies */
	int idep[EVG_UOP_MAX_IDEP];
	int odep[EVG_UOP_MAX_ODEP];
	int idep_count;
	int odep_count;

	/* Double linked lists of producer-consumers */
	struct evg_uop_t *dep_list_next;
	struct evg_uop_t *dep_list_prev;
	struct evg_uop_t *dep_list_head;
	struct evg_uop_t *dep_list_tail;
	int dep_list_count;
	int dep_list_max;

	/* Per stream-core data. This space is dynamically allocated for an uop.
	 * It should be always the last field of the structure. */
	struct evg_work_item_uop_t work_item_uop[0];
};

void evg_uop_init(void);
void evg_uop_done(void);

struct evg_alu_group_t;
struct evg_uop_t *evg_uop_create(void);
struct evg_uop_t *evg_uop_create_from_alu_group(struct evg_alu_group_t *alu_group);
void evg_uop_free(struct evg_uop_t *gpu_uop);

void evg_uop_list_free(struct linked_list_t *gpu_uop_list);
void evg_uop_dump_dep_list(char *buf, int size, int *dep_list, int dep_count);

void evg_uop_save_active_mask(struct evg_uop_t *uop);
void evg_uop_debug_active_mask(struct evg_uop_t *uop);


#endif

