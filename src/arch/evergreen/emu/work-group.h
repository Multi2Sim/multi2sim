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


/*
 * Class 'EvgWorkGroup'
 */

typedef enum
{
	EvgWorkGroupPending = 0x01,
	EvgWorkGroupRunning = 0x02,
	EvgWorkGroupFinished = 0x04
} EvgWorkGroupState;


CLASS_BEGIN(EvgWorkGroup, Object)

	/* ID */
	char *name;
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	EvgWorkGroupState status;

	/* NDRange it belongs to */
	EvgNDRange *ndrange;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* Pointers to wavefronts and work-items */
	EvgWorkItem **work_items;  /* Pointer to first work_item in 'kernel->work_items' */
	EvgWavefront **wavefronts;  /* Pointer to first wavefront in 'kernel->wavefronts' */

	/* Double linked lists of work-groups */
	EvgWorkGroup *pending_list_prev;
	EvgWorkGroup *pending_list_next;
	EvgWorkGroup *running_list_prev;
	EvgWorkGroup *running_list_next;
	EvgWorkGroup *finished_list_prev;
	EvgWorkGroup *finished_list_next;

	/* List of running wavefronts */
	EvgWavefront *running_list_head;
	EvgWavefront *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of wavefronts in barrier */
	EvgWavefront *barrier_list_head;
	EvgWavefront *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished wavefronts */
	EvgWavefront *finished_list_head;
	EvgWavefront *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;

CLASS_END(EvgWorkGroup)


#define EVG_FOR_EACH_WORK_GROUP_IN_NDRANGE(NDRANGE, WORK_GROUP_ID) \
	for ((WORK_GROUP_ID) = (NDRANGE)->work_group_id_first; \
		(WORK_GROUP_ID) <= (NDRANGE)->work_group_id_last; \
		(WORK_GROUP_ID)++)

void EvgWorkGroupCreate(EvgWorkGroup *self, EvgNDRange *ndrange);
void EvgWorkGroupDestroy(EvgWorkGroup *self);

void EvgWorkGroupDump(EvgWorkGroup *self, FILE *f);

void EvgWorkGroupSetName(EvgWorkGroup *self, char *name);

int EvgWorkGroupGetState(EvgWorkGroup *self, EvgWorkGroupState state);
void EvgWorkGroupSetState(EvgWorkGroup *self, EvgWorkGroupState state);
void EvgWorkGroupClearState(EvgWorkGroup *self, EvgWorkGroupState state);


#endif


