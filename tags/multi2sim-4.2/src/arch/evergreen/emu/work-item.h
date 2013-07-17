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


#define EVG_MAX_GPR_ELEM  5
#define EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST  2

struct evg_gpr_t
{
	unsigned int elem[EVG_MAX_GPR_ELEM];  /* x, y, z, w, t */
};

/* Structure describing a memory access definition */
struct evg_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	unsigned int addr;
	int size;
};

struct evg_work_item_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	struct evg_wavefront_t *wavefront;
	struct evg_work_group_t *work_group;
	struct evg_ndrange_t *ndrange;

	/* Work-item state */
	struct evg_gpr_t gpr[128];  /* General purpose registers */
	struct evg_gpr_t pv;  /* Result of last computations */

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
};

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

struct evg_work_item_t *evg_work_item_create(void);
void evg_work_item_free(struct evg_work_item_t *work_item);

/* Consult and change active/predicate bits */
void evg_work_item_set_active(struct evg_work_item_t *work_item, int active);
int evg_work_item_get_active(struct evg_work_item_t *work_item);
void evg_work_item_set_pred(struct evg_work_item_t *work_item, int pred);
int evg_work_item_get_pred(struct evg_work_item_t *work_item);
void evg_work_item_update_branch_digest(struct evg_work_item_t *work_item,
	long long inst_count, unsigned int inst_addr);


#endif

