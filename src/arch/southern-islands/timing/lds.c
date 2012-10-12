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

#include <lib/esim/esim.h>
#include <lib/esim/trace.h>

#include "timing.h"


int si_gpu_lds_inflight_mem_accesses = 32;
int si_gpu_lds_issue_width = 1;
int si_gpu_lds_reg_latency = 1;

void si_lds_writeback(struct si_lds_t *lds)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process completed memory instructions */
	list_count = linked_list_count(lds->mem_out_buffer);
	linked_list_head(lds->mem_out_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(lds->mem_out_buffer);
		assert(uop);

		if (!uop->local_mem_witness)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(lds->mem_out_buffer);

			si_trace("si.inst id=%lld cu=%d stg=\"lds-w\"\n", uop->id_in_compute_unit,
							lds->compute_unit->id);

			/* Make the wavefront active again */
			wavefront = uop->wavefront;
			wavefront->ready = 1;

			/* Free uop */
			if (si_tracing())
				si_gpu_uop_trash_add(uop);
			else
				si_uop_free(uop);
		}
		else
		{
			linked_list_next(lds->mem_out_buffer);
		}
	}

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_lds_execute(struct si_lds_t *lds)
{
	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_work_item_t *work_item;
	int work_item_id;
	int instructions_issued = 0;
	int list_count;
	int i, j;
	enum mod_access_kind_t access_type;

	/* Look through the memory execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(lds->mem_exec_buffer);
	linked_list_head(lds->mem_exec_buffer);
	for (i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(lds->mem_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_lds_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}

		assert(linked_list_count(lds->mem_out_buffer) <=
			si_gpu_lds_inflight_mem_accesses);

		/* If there is room in the outstanding memory buffer, issue the access */
		if (linked_list_count(lds->mem_out_buffer) < si_gpu_lds_inflight_mem_accesses)
		{
			/* Access local memory */
			SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
			{
				work_item = si_gpu->ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				for (j = 0; j < work_item_uop->local_mem_access_count; j++)
				{
					if (work_item->local_mem_access_type[j] == 1)
						access_type = mod_access_load;
					else if (work_item->local_mem_access_type[j] == 2)
						access_type = mod_access_store;
					else
						fatal("%s: invalid lds access type", __FUNCTION__);

					mod_access(lds->compute_unit->local_memory, access_type,
						work_item_uop->local_mem_access_addr[j],
						&uop->local_mem_witness, NULL, NULL);
					uop->local_mem_witness--;
				}
			}

			/* Transfer the uop to the outstanding memory access buffer */
			linked_list_remove(lds->mem_exec_buffer);
			linked_list_add(lds->mem_out_buffer, uop);

			instructions_issued++;
			lds->inst_count++;
			lds->wavefront_count++;

			si_trace("si.inst id=%lld cu=%d stg=\"lds-e\"\n", uop->id_in_compute_unit,
							lds->compute_unit->id);
		}
		else
		{
			/* Memory unit is busy, try later */
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}
	}
}

void si_lds_read(struct si_lds_t *lds)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_count = linked_list_count(lds->read_buffer);
	linked_list_head(lds->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(lds->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_lds_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}

		/* Issue the uop if the exec_buffer is not full */
		if (linked_list_count(lds->mem_exec_buffer) < si_gpu_lds_issue_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_lds_reg_latency;
			linked_list_remove(lds->read_buffer);
			linked_list_add(lds->mem_exec_buffer, uop);

			instructions_issued++;

			si_trace("si.inst id=%lld cu=%d stg=\"lds-r\"\n", uop->id_in_compute_unit,
					lds->compute_unit->id);
		}
		else
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}
	}
}

void si_lds_run(struct si_lds_t *lds)
{
	/* Local Data Share stages */
	si_lds_writeback(lds);
	si_lds_execute(lds);
	si_lds_read(lds);
}
