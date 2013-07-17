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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WORK_ITEM_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WORK_ITEM_H

#include <arch/southern-islands/asm/asm.h>


#define SI_MAX_LDS_ACCESSES_PER_INST  2


/* Structure describing a memory access definition */
struct si_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	unsigned int addr;
	int size;
};

struct si_work_item_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;

	/* Work-item state */
	union si_reg_t vreg[256];  /* Vector general purpose registers */

	/* Last global memory access */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Last LDS access */
	int lds_access_count;  /* Number of LDS access by last instruction */
	unsigned int lds_access_addr[SI_MAX_LDS_ACCESSES_PER_INST];
	unsigned int lds_access_size[SI_MAX_LDS_ACCESSES_PER_INST];
	int lds_access_type[SI_MAX_LDS_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */
};

#define SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(WAVEFRONT, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = 0; \
		(WORK_ITEM_ID) < si_emu_wavefront_size; \
		(WORK_ITEM_ID)++)

struct si_work_item_t *si_work_item_create(void);
void si_work_item_free(struct si_work_item_t *work_item);

#endif
