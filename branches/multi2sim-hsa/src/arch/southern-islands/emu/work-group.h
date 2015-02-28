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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WORK_GROUP_OLD_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WORK_GROUP_OLD_H


/*
 * Class 'SIWorkGroup'
 */

CLASS_BEGIN(SIWorkGroup, Object)

	/* ID */
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	int wavefronts_at_barrier;
	int wavefronts_completed_emu;
	int wavefronts_completed_timing;
	int finished_emu;
	int finished_timing;

	/* ND-Range that it belongs to */
	SINDRange *ndrange;

	/* Pointers to wavefronts and work-items */
	int work_item_count;
	int wavefront_count;
	SIWorkItem **work_items;  /* Pointer to first work_item in 
						'kernel->work_items' */
	SIWavefront **wavefronts;  /* Pointer to first wavefront in 
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

CLASS_END(SIWorkGroup)


void SIWorkGroupCreate(SIWorkGroup *self, unsigned int id, SINDRange *ndrange);
void SIWorkGroupDestroy(SIWorkGroup *self);

void SIWorkGroupDump(Object *self, FILE *f);

#endif
