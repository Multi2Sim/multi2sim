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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_COMPUTE_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_COMPUTE_UNIT_H

#include "branch-unit.h"
#include "lds-unit.h"
#include "scalar-unit.h"
#include "vector-mem-unit.h"


/*
 * Class 'SIComputeUnit'
 */

CLASS_BEGIN(SIComputeUnit, Object)
	
	/* IDs */
	int id;
	int subdevice_id;
	long long uop_id_counter;

	/* GPU that it belongs to */
	SIGpu *gpu;

	/* Entry points to memory hierarchy */
	struct mod_t *scalar_cache;
	struct mod_t *vector_cache;
	struct mod_t *lds_module;

	/* Hardware structures */
	unsigned int num_wavefront_pools;
	struct si_wavefront_pool_t **wavefront_pools;
	struct list_t **fetch_buffers;
	struct si_simd_t **simd_units;
	/* TODO Make these into a configurable number of structures */
	struct si_scalar_unit_t scalar_unit;
	struct si_branch_unit_t branch_unit;
	struct si_vector_mem_unit_t vector_mem_unit;
	struct si_lds_t lds_unit;

	/* Statistics */
	long long cycle;
	long long mapped_work_groups;
	long long wavefront_count;
	long long inst_count; /* Total instructions */
	long long branch_inst_count;
	long long scalar_alu_inst_count;
	long long scalar_mem_inst_count;
	/* TODO Have one SIMD inst count per SIMD unit */
	long long simd_inst_count;
	long long vector_mem_inst_count;
	long long lds_inst_count;
	long long int sreg_read_count;
	long long int sreg_write_count;
	long long int vreg_read_count;
	long long int vreg_write_count;

	/* List of currently mapped work-groups */
	int work_group_count;
	SIWorkGroup **work_groups;

	/* Compute Unit capacity state  (for concurrent command queue)*/
	/* Spatial profiling statistics */
	long long interval_cycle;
	long long interval_mapped_work_groups;
	long long interval_unmapped_work_groups;
	long long interval_alu_issued;
	long long interval_lds_issued ;
	FILE * spatial_report_file;

CLASS_END(SIComputeUnit)


void SIComputeUnitCreate(SIComputeUnit *self, SIGpu *gpu, int id);
void SIComputeUnitDestroy(SIComputeUnit *self);

void SIComputeUnitMapWorkGroup(SIComputeUnit *self, SIWorkGroup *work_group);
void SIComputeUnitUnmapWorkGroup(SIComputeUnit *self, SIWorkGroup *work_group);
SIWavefront *SIComputeUnitSchedule(SIComputeUnit *self);
void SIComputeUnitRun(SIComputeUnit *self);



/*
 * Object 'si_wavefront_pool_t'
 */

struct si_wavefront_pool_t *si_wavefront_pool_create(void);
void si_wavefront_pool_free(struct si_wavefront_pool_t *wavefront_pool);
void si_wavefront_pool_map_wavefronts(struct si_wavefront_pool_t *wavefront_pool, 
	SIWorkGroup *work_group);
void si_wavefront_pool_unmap_wavefronts(struct si_wavefront_pool_t *wavefront_pool, 
	SIWorkGroup *work_group);

#endif

