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
#include "cycle-interval-report.h"
#include "vector-mem-unit.h"
#include "uop.h"
#include "warp-inst-queue.h"

void frm_vector_mem_complete(struct frm_vector_mem_unit_t *vector_mem)
{
	struct frm_uop_t *uop = NULL;
	int list_entries;
	int i;
	int list_index = 0;

	/* Process completed memory instructions */
	list_entries = list_count(vector_mem->write_buffer);

	/* Sanity check the write buffer */
	assert(list_entries <= frm_gpu_vector_mem_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->write_buffer, list_index);
		assert(uop);

		/* Uop is not ready */
		if (asTiming(frm_gpu)->cycle < uop->write_ready)
		{
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(vector_mem->write_buffer, uop);

		assert(uop->warp_inst_queue_entry->lgkm_cnt > 0);
		uop->warp_inst_queue_entry->lgkm_cnt--;

		frm_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_sm,
			uop->sm->id);

		/* Free uop */
		frm_uop_free(uop);

		/* Statistics */
		vector_mem->inst_count++;
		frm_gpu->last_complete_cycle = asTiming(frm_gpu)->cycle;
	}
}

void frm_vector_mem_write(struct frm_vector_mem_unit_t *vector_mem)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(vector_mem->mem_buffer);

	/* Sanity check the mem buffer */
	assert(list_entries <= frm_gpu_vector_mem_max_inflight_mem_accesses);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->mem_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (uop->global_mem_witness)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached. */
		if (instructions_processed > frm_gpu_vector_mem_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check write buffer */
		assert(list_count(vector_mem->write_buffer) <= 
				frm_gpu_vector_mem_write_buffer_size);

		/* Stop if the write buffer is full. */
		if (list_count(vector_mem->write_buffer) == 
				frm_gpu_vector_mem_write_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		uop->write_ready = asTiming(frm_gpu)->cycle + 
			frm_gpu_vector_mem_write_latency;

		/* In the above context, access means any of the 
		 * mod_access calls in frm_vector_mem_mem. Means all 
		 * inflight accesses for uop are done */
		if(frm_spatial_report_active)
		{
			if (uop->vector_mem_write)
			{
				frm_report_global_mem_finish(uop->sm,
						uop->num_global_mem_write);
			}
			else if (uop->vector_mem_read)
			{
				frm_report_global_mem_finish(uop->sm,
						uop->num_global_mem_read);
			}
			else
			{
				fatal("%s: invalid access kind", __FUNCTION__);
			}
		}

		list_remove(vector_mem->mem_buffer, uop);
		list_enqueue(vector_mem->write_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"mem-w\"\n", uop->id_in_sm, 
			vector_mem->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_vector_mem_mem(struct frm_vector_mem_unit_t *vector_mem)
{
	struct frm_uop_t *uop;
	struct frm_thread_uop_t *thread_uop;
	struct frm_thread_t *thread;
	int thread_id;
	int instructions_processed = 0;
	int list_entries;
	int i;
	enum mod_access_kind_t access_kind;
	int list_index = 0;

	list_entries = list_count(vector_mem->read_buffer);
	
	/* Sanity check the read buffer */
	assert(list_entries <= frm_gpu_vector_mem_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->read_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->read_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached. */
		if (instructions_processed > frm_gpu_vector_mem_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check mem buffer */
		assert(list_count(vector_mem->mem_buffer) <= 
			frm_gpu_vector_mem_max_inflight_mem_accesses);

		/* Stall if there is not room in the memory buffer */
		if (list_count(vector_mem->mem_buffer) == 
			frm_gpu_vector_mem_max_inflight_mem_accesses)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Set the access type */
		if (uop->vector_mem_write && !uop->glc)
			access_kind = mod_access_nc_store;
		else if (uop->vector_mem_write && uop->glc)
			access_kind = mod_access_store;
		else if (uop->vector_mem_read)
			access_kind = mod_access_load;
		else 
			fatal("%s: invalid access kind", __FUNCTION__);

		/* Access global memory */
		assert(!uop->global_mem_witness);
		for (thread_id = uop->warp->threads[0]->id_in_warp; 
				thread_id < uop->warp->thread_count; 
				thread_id++)
		{
			thread = uop->warp->threads[thread_id];
			thread_uop = 
				&uop->thread_uop[thread->id_in_warp];

			mod_access(vector_mem->sm->global_memory, 
				access_kind, 
				thread_uop->global_mem_access_addr,
				&uop->global_mem_witness, NULL, NULL, NULL);
			uop->global_mem_witness--;
		}

		if(frm_spatial_report_active)
		{
			if (uop->vector_mem_write)
			{
				uop->num_global_mem_write += 
					uop->global_mem_witness;
				frm_report_global_mem_inflight(uop->sm,
						uop->num_global_mem_write);
			}
			else if (uop->vector_mem_read)
			{
				uop->num_global_mem_read += 
					uop->global_mem_witness;
				frm_report_global_mem_inflight(uop->sm,
						uop->num_global_mem_read);
			}
			else
				fatal("%s: invalid access kind", __FUNCTION__);
		}

		/* Increment outstanding memory access count */
		uop->warp_inst_queue_entry->lgkm_cnt++;

		/* Transfer the uop to the mem buffer */
		list_remove(vector_mem->read_buffer, uop);
		list_enqueue(vector_mem->mem_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"mem-m\"\n", uop->id_in_sm, 
			vector_mem->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_vector_mem_read(struct frm_vector_mem_unit_t *vector_mem)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(vector_mem->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= frm_gpu_vector_mem_decode_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached. */
		if (instructions_processed > frm_gpu_vector_mem_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check the read buffer */
		assert(list_count(vector_mem->read_buffer) <= 
			frm_gpu_vector_mem_read_buffer_size);

		/* Stop if the read buffer is full. */
		if (list_count(vector_mem->read_buffer) == 
			frm_gpu_vector_mem_read_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		uop->read_ready = asTiming(frm_gpu)->cycle + 
			frm_gpu_vector_mem_read_latency;

		list_remove(vector_mem->decode_buffer, uop);
		list_enqueue(vector_mem->read_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"mem-r\"\n", uop->id_in_sm, 
			vector_mem->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_vector_mem_decode(struct frm_vector_mem_unit_t *vector_mem)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(vector_mem->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= frm_gpu_vector_mem_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > frm_gpu_vector_mem_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check the decode buffer */
		assert(list_count(vector_mem->decode_buffer) <= 
				frm_gpu_vector_mem_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(vector_mem->decode_buffer) == 
			frm_gpu_vector_mem_decode_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				vector_mem->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		uop->decode_ready = asTiming(frm_gpu)->cycle + 
			frm_gpu_vector_mem_decode_latency;

		list_remove(vector_mem->issue_buffer, uop);
		list_enqueue(vector_mem->decode_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"mem-d\"\n", uop->id_in_sm, 
			vector_mem->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_vector_mem_run(struct frm_vector_mem_unit_t *vector_mem)
{
	/* Local Data Share stages */
	frm_vector_mem_complete(vector_mem);
	frm_vector_mem_write(vector_mem);
	frm_vector_mem_mem(vector_mem);
	frm_vector_mem_read(vector_mem);
	frm_vector_mem_decode(vector_mem);
}
