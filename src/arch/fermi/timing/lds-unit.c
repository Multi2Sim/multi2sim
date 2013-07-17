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


#include <arch/fermi/emu/warp.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "sm.h"
#include "gpu.h"
#include "lds-unit.h"
#include "uop.h"
#include "warp-inst-queue.h"


void frm_lds_complete(struct frm_lds_t *lds)
{
	struct frm_uop_t *uop = NULL;
	int list_entries;
	int i;
	int list_index = 0;

	/* Process completed memory instructions */
	list_entries = list_count(lds->write_buffer);

	/* Sanity check the write buffer */
	assert(list_entries <= frm_gpu_lds_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->write_buffer, list_index);
		assert(uop);

		/* Uop is not ready */
		if (asTiming(frm_gpu)->cycle < uop->write_ready)
		{
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(lds->write_buffer, uop);

		assert(uop->warp_inst_queue_entry->lgkm_cnt > 0);
		uop->warp_inst_queue_entry->lgkm_cnt--;

		frm_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_sm,
			uop->sm->id);

		/* Free uop */
		frm_uop_free(uop);

		/* Statistics */
		lds->inst_count++;
		frm_gpu->last_complete_cycle = asTiming(frm_gpu)->cycle;
	}
}

void frm_lds_write(struct frm_lds_t *lds)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(lds->mem_buffer);

	/* Sanity check the mem buffer */
	assert(list_entries <= frm_gpu_lds_max_inflight_mem_accesses);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->mem_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (uop->lds_witness)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached */
		if (instructions_processed > frm_gpu_lds_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check the write buffer */
		assert(list_count(lds->write_buffer) <= 
			frm_gpu_lds_write_buffer_size);

		/* Stop if the write buffer is full */
		if (list_count(lds->write_buffer) >= 
			frm_gpu_lds_write_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		uop->write_ready = asTiming(frm_gpu)->cycle + frm_gpu_lds_write_latency;
		list_remove(lds->mem_buffer, uop);
		list_enqueue(lds->write_buffer, uop);

		instructions_processed++;

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"lds-w\"\n", uop->id_in_sm, 
			lds->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_lds_mem(struct frm_lds_t *lds)
{
	struct frm_uop_t *uop;
	struct frm_thread_uop_t *thread_uop;
	struct frm_thread_t *thread;
	int thread_id;
	int instructions_processed = 0;
	int list_entries;
	int i, j;
	enum mod_access_kind_t access_type;
	int list_index = 0;

	list_entries = list_count(lds->read_buffer);
	
	/* Sanity check the read buffer */
	assert(list_entries <= frm_gpu_lds_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->read_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->read_ready)
        	{
			list_index++;
			continue;
        	}

		/* Stall if the width has been reached. */
		if (instructions_processed > frm_gpu_lds_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check uop */
		assert(uop->lds_read || uop->lds_write);

		/* Sanity check mem buffer */
		assert(list_count(lds->mem_buffer) <= 
			frm_gpu_lds_max_inflight_mem_accesses);

		/* Stall if there is no room in the memory buffer */
		if (list_count(lds->mem_buffer) == 
			frm_gpu_lds_max_inflight_mem_accesses)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Access local memory */
		for (thread_id = uop->warp->threads[0]->id_in_warp; 
				thread_id < uop->warp->thread_count; 
				thread_id++)
		{
			thread = uop->warp->threads[thread_id];
			thread_uop = 
				&uop->thread_uop[thread->id_in_warp];

			for (j = 0; j < thread_uop->lds_access_count; j++)
			{
				if (thread->lds_access_type[j] == 1)
				{
					access_type = mod_access_load;
				}
				else if (thread->lds_access_type[j] == 2)
				{
					access_type = mod_access_store;
				}
				else
				{
					fatal("%s: invalid lds access type", 
						__FUNCTION__);
				}

				mod_access(lds->sm->lds_module, 
					access_type, 
					thread_uop->lds_access_addr[j],
					&uop->lds_witness, NULL, NULL, NULL);
				uop->lds_witness--;
			}
		}

		/* Increment outstanding memory access count */
		uop->warp_inst_queue_entry->lgkm_cnt++;

		/* Transfer the uop to the mem buffer */
		list_remove(lds->read_buffer, uop);
		list_enqueue(lds->mem_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"lds-m\"\n", uop->id_in_sm, 
			lds->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_lds_read(struct frm_lds_t *lds)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(lds->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= frm_gpu_lds_decode_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Stall if the width has been reached. */
		if (instructions_processed > frm_gpu_lds_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Stop if the read buffer is full. */
		if (list_count(lds->read_buffer) >= frm_gpu_lds_read_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Uop is not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}
		
		uop->read_ready = asTiming(frm_gpu)->cycle + frm_gpu_lds_read_latency;
		list_remove(lds->decode_buffer, uop);
		list_enqueue(lds->read_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"lds-r\"\n", uop->id_in_sm, 
			lds->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_lds_decode(struct frm_lds_t *lds)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(lds->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= frm_gpu_lds_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > frm_gpu_lds_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check the decode buffer */
		assert(list_count(lds->decode_buffer) <= 
			frm_gpu_lds_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(lds->decode_buffer) == 
			frm_gpu_lds_decode_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				lds->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		uop->decode_ready = asTiming(frm_gpu)->cycle + frm_gpu_lds_decode_latency;
		list_remove(lds->issue_buffer, uop);
		list_enqueue(lds->decode_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"lds-d\"\n", uop->id_in_sm, 
			lds->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_lds_run(struct frm_lds_t *lds)
{
	/* Local Data Share stages */
	frm_lds_complete(lds);
	frm_lds_write(lds);
	frm_lds_mem(lds);
	frm_lds_read(lds);
	frm_lds_decode(lds);
}
