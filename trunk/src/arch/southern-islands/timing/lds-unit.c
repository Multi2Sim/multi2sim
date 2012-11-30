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
#include <limits.h>

#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/emu/wavefront.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "compute-unit.h"
#include "gpu.h"
#include "lds-unit.h"
#include "uop.h"
#include "wavefront-pool.h"


/* Configurable by user at runtime */

int si_gpu_lds_width = 1;
int si_gpu_lds_issue_buffer_size = 4;

/*
 * Register accesses are not pipelined, so buffer size is not
 * multiplied by the latency.
 */
int si_gpu_lds_read_latency = 1;
int si_gpu_lds_read_buffer_size = 1;

int si_gpu_lds_inflight_mem_accesses = 32;

void si_lds_process_mem_accesses(struct si_lds_t *lds)
{
	struct si_uop_t *uop = NULL;
	int list_entries;
	int i;
	int list_index = 0;

	/* Process completed memory instructions */
	list_entries = list_count(lds->inflight_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_lds_inflight_mem_accesses);
	
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->inflight_buffer, list_index);
		assert(uop);

		if (!uop->local_mem_witness)
		{
			assert(uop->wavefront_pool_entry->lgkm_cnt > 0);
			uop->wavefront_pool_entry->lgkm_cnt--;

			/* Access complete, remove the uop from the queue */
			list_remove(lds->inflight_buffer, uop);

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

void si_lds_writeback(struct si_lds_t *lds)
{
	struct si_uop_t *uop = NULL;
	int list_entries;
	int i;
	int list_index = 0;

	/* Process completed memory instructions */
	list_entries = list_count(lds->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_lds_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->exec_buffer, list_index);
		assert(uop);

        /* Uop is not ready */
		if (si_gpu->cycle < uop->execute_ready)
		{
            list_index++;
			continue;
        }

        /* Access complete, remove the uop from the queue */
        list_remove(lds->exec_buffer, uop);

        si_trace("si.inst id=%lld cu=%d wf=%d stg=\"lds-w\"\n", 
            uop->id_in_compute_unit, lds->compute_unit->id, 
            uop->wavefront->id);

        /* Allow next instruction to be fetched */
        uop->wavefront_pool_entry->ready = 1;
        uop->wavefront_pool_entry->uop = NULL;
        uop->wavefront_pool_entry->cycle_fetched = INST_NOT_FETCHED;

        /* Free uop */
        if (si_tracing())
            si_gpu_uop_trash_add(uop);
        else
            si_uop_free(uop);

        /* Statistics */
        lds->inst_count++;
        si_gpu->last_complete_cycle = esim_cycle;
	}
}

void si_lds_execute(struct si_lds_t *lds)
{
	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_work_item_t *work_item;
	int work_item_id;
	int instructions_processed = 0;
	int list_entries;
	int i, j;
	enum mod_access_kind_t access_type;
    int list_index = 0;

	list_entries = list_count(lds->read_buffer);
	
	/* Sanity check the read buffer.  Register accesses are not pipelined, so
	 * buffer size is not multiplied by the latency. */
	assert(list_entries <= si_gpu_lds_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->read_buffer, list_index);
		assert(uop);

		/* Stall if the issue width has been reached. */
		if (instructions_processed == si_gpu_lds_width)
        {
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, lds->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
            continue;
        }

		/* Uop is not ready yet */
		if (si_gpu->cycle < uop->read_ready)
        {
			list_index++;
            continue;
        }

		/* Sanity check uop */
		assert(uop->local_mem_read || uop->local_mem_write);

		/* Sanity check in-flight buffer */
		assert(list_count(lds->inflight_buffer) <= si_gpu_lds_inflight_mem_accesses);

		/* Sanity check exec buffer */
		assert(list_count(lds->exec_buffer) <= si_gpu_lds_width);

		/* If there is room in the outstanding memory buffer, issue the access */
		if (list_count(lds->inflight_buffer) < si_gpu_lds_inflight_mem_accesses)
		{
			/* TODO replace this with a lightweight uop */
			struct si_uop_t *mem_uop;
		        mem_uop = si_uop_create();
		        mem_uop->wavefront = uop->wavefront;
			mem_uop->wavefront_pool_entry = uop->wavefront_pool_entry;
			mem_uop->local_mem_read = uop->local_mem_read;
			mem_uop->local_mem_write =  uop->local_mem_write;

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
						&mem_uop->local_mem_witness, NULL, NULL, NULL);
					mem_uop->local_mem_witness--;
				}
			}

			/* Increment outstanding memory access count */
			mem_uop->wavefront_pool_entry->lgkm_cnt++;

			/* Transfer the uop to the exec buffer */
			uop->execute_ready = si_gpu->cycle + 1;
			list_remove(lds->read_buffer, uop);
			list_enqueue(lds->exec_buffer, uop);

			/* Add a mem_uop to the in-flight buffer */
			list_enqueue(lds->inflight_buffer, mem_uop);

			instructions_processed++;

			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"lds-e\"\n", 
				uop->id_in_compute_unit, lds->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
		}
		else
		{
			/* Memory unit is busy, try later */
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, lds->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
            continue;
		}
	}
}

void si_lds_read(struct si_lds_t *lds)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
    int list_index = 0;
    int i;

	list_entries = list_count(lds->issue_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_lds_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(lds->issue_buffer, list_index);
		assert(uop);

		/* Stall if the width has been reached. */
		if (instructions_processed == si_gpu_lds_width)
        {
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, lds->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
            continue;
        }

		/* Stop if the read buffer is full. */
		if (list_count(lds->read_buffer) >= si_gpu_lds_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, lds->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
            continue;
		}

		/* Uop is not ready yet */
		if (si_gpu->cycle < uop->issue_ready)
        {
			list_index++;
            continue;
        }
		
		uop->read_ready = si_gpu->cycle + si_gpu_lds_read_latency;
		list_remove(lds->issue_buffer, uop);
		list_enqueue(lds->read_buffer, uop);

		instructions_processed++;

        si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"lds-r\"\n", 
            uop->id_in_compute_unit, lds->compute_unit->id, 
            uop->wavefront->id, uop->id_in_wavefront);
	}
}

void si_lds_run(struct si_lds_t *lds)
{
	/* Local Data Share stages */
	si_lds_process_mem_accesses(lds);
	si_lds_writeback(lds);
	si_lds_execute(lds);
	si_lds_read(lds);
}
