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
#include <mem-system/mem-system.h>

#include "timing.h"

/* Configurable by user at runtime */

int si_gpu_vector_mem_width = 1;

int si_gpu_vector_mem_decode_buffer_size = 5;

/*
 * Register accesses are not pipelined, so buffer size is not
 * multiplied by the latency.
 */
int si_gpu_vector_mem_read_latency = 1;
int si_gpu_vector_mem_read_buffer_size = 1;

int si_gpu_vector_mem_inflight_mem_accesses = 32;

void si_vector_mem_process_mem_accesses(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop = NULL;
	int i;
	int list_entries;
	int list_index = 0;
	struct si_inst_buffer_entry_t *inst_buffer_entry;

	/* Process completed memory instructions */
	list_entries = list_count(vector_mem->inflight_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_vector_mem_inflight_mem_accesses);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->inflight_buffer, list_index);
		assert(uop);

		if (!uop->global_mem_witness)
		{
			inst_buffer_entry = uop->inst_buffer_entry; 

			assert(inst_buffer_entry->vm_cnt > 0);
			inst_buffer_entry->vm_cnt--;

			/* Access complete, remove the uop from the queue */
			list_remove(vector_mem->inflight_buffer, uop);

			/* Free uop */
			si_uop_free(uop);

			si_gpu->last_complete_cycle = esim_cycle;
		}
		else
		{
			list_index++;
		}
	}

}

void si_vector_mem_writeback(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop = NULL;
	int list_entries;

	/* Process completed memory instructions */
	list_entries = list_count(vector_mem->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_vector_mem_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(vector_mem->exec_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			list_remove(vector_mem->exec_buffer, uop);

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"mem-w\"\n", 
				uop->id_in_compute_unit, vector_mem->compute_unit->id, 
				uop->wavefront->id);

			/* Allow next instruction to be fetched */
			uop->inst_buffer_entry->ready = 1;
			uop->inst_buffer_entry->uop = NULL;
			uop->inst_buffer_entry->cycle_fetched = INST_NOT_FETCHED;

			/* Free uop */
			if (si_tracing())
				si_gpu_uop_trash_add(uop);
			else
				si_uop_free(uop);

			/* Statistics */
			vector_mem->inst_count++;

			si_gpu->last_complete_cycle = esim_cycle;
		}
		else 
		{
			break;
		}
	}
}

void si_vector_mem_execute(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_work_item_t *work_item;
	int work_item_id;
	int instructions_processed = 0;
	int list_entries;
	int i;

	list_entries = list_count(vector_mem->read_buffer);
	
	/* Sanity check the read buffer.  Register accesses are not pipelined, so
	 * buffer size is not multiplied by the latency. */
	assert(list_entries <= si_gpu_vector_mem_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_head(vector_mem->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_processed == si_gpu_vector_mem_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, vector_mem->compute_unit->id, 
				uop->wavefront->id);
			break;
		}

		/* Sanity check uop */
		assert(uop->vector_mem_read || uop->vector_mem_write);

		/* Sanity check in-flight buffer */
		assert(list_count(vector_mem->inflight_buffer) <= 
			si_gpu_vector_mem_inflight_mem_accesses);

		/* Sanity check exec buffer */
		assert(list_count(vector_mem->exec_buffer) <= si_gpu_vector_mem_width);

		/* If there is room in the outstanding memory buffer, issue the access */
		if (list_count(vector_mem->inflight_buffer) < 
			si_gpu_vector_mem_inflight_mem_accesses)
		{
			/* TODO replace this with a lightweight uop */
			struct si_uop_t *mem_uop;
		        mem_uop = si_uop_create();
		        mem_uop->wavefront = uop->wavefront;
			mem_uop->compute_unit = uop->compute_unit;
			mem_uop->inst_buffer_entry = uop->inst_buffer_entry;
			mem_uop->vector_mem_read = uop->vector_mem_read;
			mem_uop->vector_mem_write =  uop->vector_mem_write;
			mem_uop->id_in_compute_unit = uop->compute_unit->mem_uop_id_counter++;

			enum mod_access_kind_t access_kind;

			if (uop->vector_mem_write)
				access_kind = mod_access_nc_store;
			else if (uop->vector_mem_read)
				access_kind = mod_access_load;
			else 
				fatal("%s: invalid access kind", __FUNCTION__);

			/* Access global memory */
			assert(!mem_uop->global_mem_witness);
			SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
			{
				work_item = si_gpu->ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];

				mod_access(vector_mem->compute_unit->global_memory, access_kind,
					work_item_uop->global_mem_access_addr,
					&mem_uop->global_mem_witness, NULL, NULL, NULL);
				mem_uop->global_mem_witness--;
			}

			/* Increment outstanding memory access count */
			uop->inst_buffer_entry->vm_cnt++;

			/* Transfer the uop to the exec buffer */
			uop->execute_ready = si_gpu->cycle + 1;
			list_remove(vector_mem->read_buffer, uop);
			list_enqueue(vector_mem->exec_buffer, uop);

			/* Add a mem_uop to the in-flight buffer */
			list_enqueue(vector_mem->inflight_buffer, mem_uop);

			instructions_processed++;

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"mem-e\"\n", 
				uop->id_in_compute_unit, vector_mem->compute_unit->id, 
				uop->wavefront->id);
		}
		else
		{
			/* Memory unit is busy, try later */
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, vector_mem->compute_unit->id, 
				uop->wavefront->id);
			break;
		}
	}
}

void si_vector_mem_read(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;

	list_entries = list_count(vector_mem->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_vector_mem_decode_buffer_size);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(vector_mem->decode_buffer);
		assert(uop);

		/* Stop if the width has been reached. */
		if (instructions_processed == si_gpu_vector_mem_width)
			break;

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the read buffer is full. */
		if (list_count(vector_mem->read_buffer) >= si_gpu_vector_mem_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, vector_mem->compute_unit->id, 
				uop->wavefront->id);
			break;
		}
		
		uop->read_ready = si_gpu->cycle + si_gpu_vector_mem_read_latency;
		list_remove(vector_mem->decode_buffer, uop);
		list_enqueue(vector_mem->read_buffer, uop);

		instructions_processed++;

		si_trace("si.inst id=%lld cu=%d wf=%d stg=\"mem-r\"\n",
			uop->id_in_compute_unit, vector_mem->compute_unit->id,
			uop->wavefront->id);
	}
}


void si_vector_mem_run(struct si_vector_mem_unit_t *vector_mem)
{
	/* Vector Memory stages */
	si_vector_mem_process_mem_accesses(vector_mem);
	si_vector_mem_writeback(vector_mem);
	si_vector_mem_execute(vector_mem);
	si_vector_mem_read(vector_mem);
}
