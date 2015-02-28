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

#ifndef ARCH_EVERGREEN_EMU_WORK_ITEM_H
#define ARCH_EVERGREEN_EMU_WORK_ITEM_H

#include <arch/evergreen/asm/inst.h>


/*
 * Class 'EvgWorkItem'
 */

#define EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST  2

CLASS_BEGIN(EvgWorkItem, Object)

	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	EvgWavefront *wavefront;
	EvgWorkGroup *work_group;
	EvgNDRange *ndrange;

	/* Work-item state */
	EvgInstGpr gpr[128];  /* General purpose registers */
	EvgInstGpr pv;  /* Result of last computations */

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct linked_list_t *write_task_list;

	/* LDS (Local Data Share) OQs (Output Queues) */
	struct list_t *lds_oqa;
	struct list_t *lds_oqb;

	/* This is a digest of the active mask updates for this work_item. Every time
	 * an instruction updates the active mask of a wavefront, this digest is updated
	 * for active work_items by XORing a random number common for the wavefront.
	 * At the end, work_items with different 'branch_digest' numbers can be considered
	 * divergent work_items. */
	unsigned int branch_digest;

	/* Last global memory access */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Last local memory access */
	int local_mem_access_count;  /* Number of local memory access performed by last instruction */
	unsigned int local_mem_access_addr[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	unsigned int local_mem_access_size[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	int local_mem_access_type[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */

CLASS_END(EvgWorkItem)


#define EVG_FOREACH_WORK_ITEM_IN_NDRANGE(NDRANGE, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (NDRANGE)->work_item_id_first; \
		(WORK_ITEM_ID) <= (NDRANGE)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define EVG_FOREACH_WORK_ITEM_IN_WORK_GROUP(WORK_GROUP, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WORK_GROUP)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WORK_GROUP)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(WAVEFRONT, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WAVEFRONT)->work_item_id_last; \
		(WORK_ITEM_ID)++)


void EvgWorkItemCreate(EvgWorkItem *self, EvgWavefront *wavefront);
void EvgWorkItemDestroy(EvgWorkItem *self);

void EvgWorkItemSetActive(EvgWorkItem *work_item, int active);
int EvgWorkItemGetActive(EvgWorkItem *work_item);
void EvgWorkItemSetPred(EvgWorkItem *work_item, int pred);
int EvgWorkItemGetPred(EvgWorkItem *work_item);
void EvgWorkItemUpdateBranchDigest(EvgWorkItem *work_item,
	long long inst_count, unsigned int inst_addr);


#endif

