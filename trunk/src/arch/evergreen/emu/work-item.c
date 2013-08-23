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

#include "wavefront.h"
#include "work-item.h"



/*
 * Class 'EvgWorkItem'
 */

void EvgWorkItemCreate(EvgWorkItem *self, EvgWavefront *wavefront)
{
	/* Initialize */
	self->wavefront = wavefront;
	self->work_group = wavefront->work_group;
	self->ndrange = wavefront->ndrange;
	self->write_task_list = linked_list_create();
	self->lds_oqa = list_create();
	self->lds_oqb = list_create();
}


void EvgWorkItemDestroy(EvgWorkItem *self)
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



void EvgWorkItemSetActive(EvgWorkItem *self, int active)
{
	EvgWavefront *wavefront = self->wavefront;

	assert(self->id_in_wavefront >= 0 && self->id_in_wavefront < wavefront->work_item_count);
	bit_map_set(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ self->id_in_wavefront, 1, !!active);
	wavefront->active_mask_update = 1;
}


int EvgWorkItemGetActive(EvgWorkItem *self)
{
	EvgWavefront *wavefront = self->wavefront;

	assert(self->id_in_wavefront >= 0 && self->id_in_wavefront < wavefront->work_item_count);
	return bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ self->id_in_wavefront, 1);
}


void EvgWorkItemSetPred(EvgWorkItem *self, int pred)
{
	EvgWavefront *wavefront = self->wavefront;

	assert(self->id_in_wavefront >= 0 && self->id_in_wavefront < wavefront->work_item_count);
	bit_map_set(wavefront->pred, self->id_in_wavefront, 1, !!pred);
	wavefront->pred_mask_update = 1;
}


int EvgWorkItemGetPred(EvgWorkItem *self)
{
	EvgWavefront *wavefront = self->wavefront;

	assert(self->id_in_wavefront >= 0 && self->id_in_wavefront < wavefront->work_item_count);
	return bit_map_get(wavefront->pred, self->id_in_wavefront, 1);
}


/* Based on an instruction counter, instruction address, and work_item mask,
 * update (xor) branch_digest with a random number */
void EvgWorkItemUpdateBranchDigest(EvgWorkItem *self,
	long long inst_count, unsigned int inst_addr)
{
	EvgWavefront *wavefront = self->wavefront;
	unsigned int mask = 0;

	/* Update branch digest only if work_item is active */
	if (!bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ self->id_in_wavefront, 1))
		return;

	/* Update mask with inst_count */
	mask = (unsigned int) inst_count * 0x4919f71f;  /* Multiply by prime number to generate sparse mask */

	/* Update mask with inst_addr */
	mask += inst_addr * 0x31f2e73b;

	/* Update branch digest */
	self->branch_digest ^= mask;
}
