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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>

#include "thread.h"
#include "warp.h"



/*
 * Public Functions
 */

void FrmThreadCreate(FrmThread *self)
{
	/* Initialize */
	self->write_task_list = linked_list_create();
	self->lds_oqa = list_create();
	self->lds_oqb = list_create();
}


void FrmThreadDestroy(FrmThread *self)
{
	/* Empty LDS output queues */
	while (list_count(self->lds_oqa))
		free(list_dequeue(self->lds_oqa));
	while (list_count(self->lds_oqb))
		free(list_dequeue(self->lds_oqb));
	list_free(self->lds_oqa);
	list_free(self->lds_oqb);
	linked_list_free(self->write_task_list);
}


int FrmThreadGetPred(FrmThread *self)
{
	FrmWarp *warp = self->warp;

	assert(self->id_in_warp >= 0 && self->id_in_warp < warp->thread_count);
	return bit_map_get(warp->pred, self->id_in_warp, 1);
}


void FrmThreadSetPred(FrmThread *self, int pred)
{
	FrmWarp *warp = self->warp;

	assert(self->id_in_warp >= 0 && self->id_in_warp < warp->thread_count);
	bit_map_set(warp->pred, self->id_in_warp, 1, !!pred);
	warp->pred_mask_update = 1;
}


/* Based on an instruction counter, instruction address, and thread mask,
 * update (xor) branch_digest with a random number */
void FrmThreadUpdateBranchDigest(FrmThread *self,
	long long inst_count, unsigned int inst_addr)
{
//	FrmWarp *warp = self->warp;
//	unsigned int mask = 0;
//
//	/* Update branch digest only if thread is active */
//	if (!bit_map_get(warp->active_stack, warp->stack_top * warp->thread_count
//		+ thread->id_in_warp, 1))
//		return;
//
//	/* Update mask with inst_count */
//	mask = (unsigned int) inst_count * 0x4919f71f;  /* Multiply by prime number to generate sparse mask */
//
//	/* Update mask with inst_addr */
//	mask += inst_addr * 0x31f2e73b;
//
//	/* Update branch digest */
//	thread->branch_digest ^= mask;
}

