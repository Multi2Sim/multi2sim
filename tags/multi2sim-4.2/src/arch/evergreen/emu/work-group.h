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

#ifndef ARCH_EVERGREEN_EMU_WORK_GROUP_H
#define ARCH_EVERGREEN_EMU_WORK_GROUP_H

#include <stdio.h>


enum evg_work_group_status_t
{
	evg_work_group_pending		= 0x0001,
	evg_work_group_running		= 0x0002,
	evg_work_group_finished		= 0x0004
};

struct evg_work_group_t
{
	/* ID */
	char *name;
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum evg_work_group_status_t status;

	/* NDRange it belongs to */
	struct evg_ndrange_t *ndrange;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* Pointers to wavefronts and work-items */
	struct evg_work_item_t **work_items;  /* Pointer to first work_item in 'kernel->work_items' */
	struct evg_wavefront_t **wavefronts;  /* Pointer to first wavefront in 'kernel->wavefronts' */

	/* Double linked lists of work-groups */
	struct evg_work_group_t *pending_list_prev;
	struct evg_work_group_t *pending_list_next;
	struct evg_work_group_t *running_list_prev;
	struct evg_work_group_t *running_list_next;
	struct evg_work_group_t *finished_list_prev;
	struct evg_work_group_t *finished_list_next;

	/* List of running wavefronts */
	struct evg_wavefront_t *running_list_head;
	struct evg_wavefront_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of wavefronts in barrier */
	struct evg_wavefront_t *barrier_list_head;
	struct evg_wavefront_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished wavefronts */
	struct evg_wavefront_t *finished_list_head;
	struct evg_wavefront_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define EVG_FOR_EACH_WORK_GROUP_IN_NDRANGE(NDRANGE, WORK_GROUP_ID) \
	for ((WORK_GROUP_ID) = (NDRANGE)->work_group_id_first; \
		(WORK_GROUP_ID) <= (NDRANGE)->work_group_id_last; \
		(WORK_GROUP_ID)++)

struct evg_work_group_t *evg_work_group_create(void);
void evg_work_group_free(struct evg_work_group_t *work_group);
void evg_work_group_dump(struct evg_work_group_t *work_group, FILE *f);

void evg_work_group_set_name(struct evg_work_group_t *work_group, char *name);

int evg_work_group_get_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status);
void evg_work_group_set_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status);
void evg_work_group_clear_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status);


#endif


