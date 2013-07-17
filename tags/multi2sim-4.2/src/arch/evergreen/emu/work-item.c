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
 * Public Functions
 */

struct evg_work_item_t *evg_work_item_create()
{
	struct evg_work_item_t *work_item;

	/* Initialize */
	work_item = xcalloc(1, sizeof(struct evg_work_item_t));
	work_item->write_task_list = linked_list_create();
	work_item->lds_oqa = list_create();
	work_item->lds_oqb = list_create();

	/* Return */
	return work_item;
}


void evg_work_item_free(struct evg_work_item_t *work_item)
{
	/* Empty LDS output queues */
	while (list_count(work_item->lds_oqa))
		free(list_dequeue(work_item->lds_oqa));
	while (list_count(work_item->lds_oqb))
		free(list_dequeue(work_item->lds_oqb));
	list_free(work_item->lds_oqa);
	list_free(work_item->lds_oqb);
	linked_list_free(work_item->write_task_list);

	/* Free work_item */
	free(work_item);
}



void evg_work_item_set_active(struct evg_work_item_t *work_item, int active)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	bit_map_set(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->id_in_wavefront, 1, !!active);
	wavefront->active_mask_update = 1;
}


int evg_work_item_get_active(struct evg_work_item_t *work_item)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	return bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->id_in_wavefront, 1);
}


void evg_work_item_set_pred(struct evg_work_item_t *work_item, int pred)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	bit_map_set(wavefront->pred, work_item->id_in_wavefront, 1, !!pred);
	wavefront->pred_mask_update = 1;
}


int evg_work_item_get_pred(struct evg_work_item_t *work_item)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	return bit_map_get(wavefront->pred, work_item->id_in_wavefront, 1);
}


/* Based on an instruction counter, instruction address, and work_item mask,
 * update (xor) branch_digest with a random number */
void evg_work_item_update_branch_digest(struct evg_work_item_t *work_item,
	long long inst_count, uint32_t inst_addr)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	uint32_t mask = 0;

	/* Update branch digest only if work_item is active */
	if (!bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->id_in_wavefront, 1))
		return;

	/* Update mask with inst_count */
	mask = (uint32_t) inst_count * 0x4919f71f;  /* Multiply by prime number to generate sparse mask */

	/* Update mask with inst_addr */
	mask += inst_addr * 0x31f2e73b;

	/* Update branch digest */
	work_item->branch_digest ^= mask;
}
