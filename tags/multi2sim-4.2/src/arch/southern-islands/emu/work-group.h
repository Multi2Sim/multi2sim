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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WORK_GROUP_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WORK_GROUP_H

struct si_work_group_t
{
	/* ID */
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	int wavefronts_at_barrier;
	int wavefronts_completed_emu;
	int wavefronts_completed_timing;
	int finished_emu;
	int finished_timing;

	/* ND-Range metadata */
	struct si_ndrange_t *ndrange;

	/* Pointers to wavefronts and work-items */
	int work_item_count;
	int wavefront_count;
	struct si_work_item_t **work_items;  /* Pointer to first work_item in 
						'kernel->work_items' */
	struct si_wavefront_t **wavefronts;  /* Pointer to first wavefront in 
						'kernel->wavefronts' */
	struct si_wavefront_pool_t *wavefront_pool;

	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;

	/* LDS */
	struct mem_t *lds_module;

	/* Statistics */
	long long int sreg_read_count;
	long long int sreg_write_count;
	long long int vreg_read_count;
	long long int vreg_write_count;
};

struct si_work_group_t *si_work_group_create(unsigned int work_group_id, 
	struct si_ndrange_t *ndrange);
void si_work_group_free(struct si_work_group_t *work_group);
void si_work_group_dump(struct si_work_group_t *work_group, FILE *f);

#endif
