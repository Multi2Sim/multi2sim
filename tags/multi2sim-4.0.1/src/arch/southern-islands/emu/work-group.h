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

enum si_work_group_status_t
{
	si_work_group_pending		= 0x0001,
	si_work_group_running		= 0x0002,
	si_work_group_finished		= 0x0004
};

struct si_work_group_t
{
	char name[30];

	/* ID */
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum si_work_group_status_t status;

	/* NDRange it belongs to */
	struct si_ndrange_t *ndrange;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* Pointers to wavefronts and work-items */
	struct si_work_item_t **work_items;  /* Pointer to first work_item in 'kernel->work_items' */
	struct si_wavefront_t **wavefronts;  /* Pointer to first wavefront in 'kernel->wavefronts' */
	struct si_wavefront_pool_t *wavefront_pool;

	/* Double linked lists of work-groups */
	struct si_work_group_t *pending_list_prev;
	struct si_work_group_t *pending_list_next;
	struct si_work_group_t *running_list_prev;
	struct si_work_group_t *running_list_next;
	struct si_work_group_t *finished_list_prev;
	struct si_work_group_t *finished_list_next;

	/* List of running wavefronts */
	struct si_wavefront_t *running_list_head;
	struct si_wavefront_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of wavefronts in barrier */
	struct si_wavefront_t *barrier_list_head;
	struct si_wavefront_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished wavefronts */
	struct si_wavefront_t *finished_list_head;
	struct si_wavefront_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;

	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define SI_FOR_EACH_WORK_GROUP_IN_NDRANGE(NDRANGE, WORK_GROUP_ID) \
	for ((WORK_GROUP_ID) = (NDRANGE)->work_group_id_first; \
		(WORK_GROUP_ID) <= (NDRANGE)->work_group_id_last; \
		(WORK_GROUP_ID)++)

struct si_work_group_t *si_work_group_create();
void si_work_group_free(struct si_work_group_t *work_group);
void si_work_group_dump(struct si_work_group_t *work_group, FILE *f);

int si_work_group_get_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);
void si_work_group_set_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);
void si_work_group_clear_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);

#endif
