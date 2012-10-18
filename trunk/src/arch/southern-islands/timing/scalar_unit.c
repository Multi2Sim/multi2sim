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


/* Configurable by user at runtime */
int si_gpu_scalar_unit_reg_latency = 1;
int si_gpu_scalar_unit_exec_latency = 4;
int si_gpu_scalar_unit_width = 1;
int si_gpu_scalar_unit_inflight_mem_accesses = 32;

void si_scalar_unit_process_mem_accesses(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop = NULL;
	int i;
	int list_entries;
	int list_index = 0;

	/* Process completed memory instructions */
	list_entries = list_count(scalar_unit->inflight_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_scalar_unit_inflight_mem_accesses);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->inflight_buffer, list_index);
		assert(uop);

		if (!uop->global_mem_witness)
		{

			assert(uop->inst_buffer_entry->lgkm_cnt > 0);
			uop->inst_buffer_entry->lgkm_cnt--;

			/* Access complete, remove the uop from the queue */
			list_remove(scalar_unit->inflight_buffer, uop);

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

void si_scalar_unit_writeback(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop = NULL;
	int i;
	int list_entries;
	int list_index = 0;

	/* Process completed memory instructions */
	list_entries = list_count(scalar_unit->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_scalar_unit_exec_latency * si_gpu_scalar_unit_width);
	
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->exec_buffer, list_index);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* If this is the last instruction and there are outstanding
			 * memory operations, wait for them to complete */
			if (uop->wavefront_last_inst && 
				(uop->inst_buffer_entry->lgkm_cnt || 
				 uop->inst_buffer_entry->vm_cnt || 
				 uop->inst_buffer_entry->exp_cnt)) 
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, scalar_unit->compute_unit->id, 
					uop->wavefront->id);

				list_index++;
				continue;
			}

			/* Access complete, remove the uop from the queue */
			list_remove(scalar_unit->exec_buffer, uop);

			/* Instruction is complete */
			uop->inst_buffer_entry->ready = 1;
			uop->inst_buffer_entry->uop = NULL;
			uop->inst_buffer_entry->cycle_fetched = INST_NOT_FETCHED;

			/* Check for "wait" instruction */
			if (uop->wait_inst)
			{
				/* If a wait instruction was executed and there are 
				 * outstanding memory accesses, set the wavefront to waiting */
				uop->inst_buffer_entry->wait_for_mem = 1;
			}

			/* Check for "end" instruction */
			if (uop->wavefront_last_inst)
			{
				/* If the uop completes the wavefront, set a bit so that the 
				 * hardware wont try to fetch any more instructions for it */
				uop->work_group->compute_unit_finished_count++;
				uop->inst_buffer_entry->wavefront_finished = 1;

				/* Check if wavefront finishes a work-group */
				assert(uop->work_group);
				assert(uop->work_group->compute_unit_finished_count <=
					uop->work_group->wavefront_count);
				if (uop->work_group->compute_unit_finished_count == 
						uop->work_group->wavefront_count)
				{
					si_compute_unit_unmap_work_group(scalar_unit->compute_unit,
						uop->work_group);
				}
			}


			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"su-w\"\n", 
				uop->id_in_compute_unit, scalar_unit->compute_unit->id, 
				uop->wavefront->id);

			/* Free uop */
			if (si_tracing())
				si_gpu_uop_trash_add(uop);
			else
				si_uop_free(uop);

			/* Statistics */
			scalar_unit->inst_count++;
			scalar_unit->wavefront_count++;
			si_gpu->last_complete_cycle = esim_cycle;
		}
		else
		{
			list_index++;
		}
	}
}

void si_scalar_unit_execute(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;

	/* Look through the read buffer looking for wavefronts ready to execute */
	list_entries = list_count(scalar_unit->read_buffer);

	/* Sanity check the read buffer.  Register accesses are not pipelined, so
	 * buffer size is not multiplied by the latency. */
	assert(list_entries <= si_gpu_scalar_unit_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(scalar_unit->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_processed == si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, scalar_unit->compute_unit->id, 
				uop->wavefront->id);
			break;
		}

		/* Memory instructions require another step */
		if (uop->scalar_mem_read)
		{
			/* Sanity check in-flight buffer */
			assert(list_count(scalar_unit->inflight_buffer) <= 
				si_gpu_scalar_unit_inflight_mem_accesses);

			/* Sanity check exec buffer */
			assert(list_count(scalar_unit->exec_buffer) <= si_gpu_scalar_unit_width * 
				si_gpu_scalar_unit_exec_latency);

			/* If there is room in the outstanding memory buffer, issue the access */
			if (list_count(scalar_unit->inflight_buffer) < 
				si_gpu_scalar_unit_inflight_mem_accesses &&
				list_count(scalar_unit->exec_buffer) < 
				si_gpu_scalar_unit_width * si_gpu_scalar_unit_exec_latency)
			{
				/* TODO replace this with a lightweight uop */
				struct si_uop_t *mem_uop;
				mem_uop = si_uop_create();
				mem_uop->wavefront = uop->wavefront;
				mem_uop->compute_unit = uop->compute_unit;
				mem_uop->inst_buffer_entry = uop->inst_buffer_entry;
				mem_uop ->scalar_mem_read = uop->scalar_mem_read;
				mem_uop->id_in_compute_unit = 
					uop->compute_unit->mem_uop_id_counter++;

				/* Access global memory */
				mem_uop->global_mem_witness--;
				/* FIXME Get rid of dependence on wavefront here */
				mem_uop->global_mem_access_addr =
					uop->wavefront->scalar_work_item->global_mem_access_addr;
				mod_access(scalar_unit->compute_unit->global_memory,
					mod_access_load, uop->global_mem_access_addr,
					&mem_uop->global_mem_witness, NULL, NULL);

				/* Increment outstanding memory access count */
				mem_uop->inst_buffer_entry->lgkm_cnt++;

				/* Transfer the uop to the exec buffer */
				list_remove(scalar_unit->read_buffer, uop);
				list_enqueue(scalar_unit->exec_buffer, uop);

				/* Add a mem_uop to the in-flight buffer */
				list_enqueue(scalar_unit->inflight_buffer, mem_uop);

				instructions_processed++;

				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"su-e\"\n", 
					uop->id_in_compute_unit, scalar_unit->compute_unit->id,
					uop->wavefront->id);
			}
			else 
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, scalar_unit->compute_unit->id,
					uop->wavefront->id);
				break;
			}

		}
		/* ALU Instruction */
		else
		{
			/* Sanity check exec buffer */
			assert(list_count(scalar_unit->exec_buffer) <= si_gpu_scalar_unit_width * 
				si_gpu_scalar_unit_exec_latency);

			if (list_count(scalar_unit->exec_buffer) < 
				si_gpu_scalar_unit_width * si_gpu_scalar_unit_exec_latency)
			{
				uop->execute_ready = si_gpu->cycle + 
					si_gpu_scalar_unit_exec_latency;

				/* Transfer the uop to the outstanding execution buffer */
				list_remove(scalar_unit->read_buffer, uop);
				list_enqueue(scalar_unit->exec_buffer, uop);

				instructions_processed++;
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"su-e\"\n", 
					uop->id_in_compute_unit, scalar_unit->compute_unit->id,
					uop->wavefront->id);
			}
			else
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, scalar_unit->compute_unit->id,
					uop->wavefront->id);
				break;
			}
		}
	}
}

void si_scalar_unit_read(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int i;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_entries = list_count(scalar_unit->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_decode_latency * si_gpu_decode_width);

	for (i = 0; i < list_entries; i++) 
	{
		uop = list_head(scalar_unit->decode_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the issue width has been reached */
		if (instructions_processed == si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id, uop->wavefront->id);
			break;
		}

		/* Issue the uop if the read buffer is not full */
		if (list_count(scalar_unit->read_buffer) < si_gpu_scalar_unit_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_scalar_unit_reg_latency;
			list_remove(scalar_unit->decode_buffer, uop);
			list_enqueue(scalar_unit->read_buffer, uop);

			instructions_processed++;

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"su-r\"\n", 
				uop->id_in_compute_unit, scalar_unit->compute_unit->id, 
				uop->wavefront->id);
		}
		else
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id, uop->wavefront->id);
			break;
		}

	}
}

void si_scalar_unit_run(struct si_scalar_unit_t *scalar_unit)
{
	si_scalar_unit_process_mem_accesses(scalar_unit);
	si_scalar_unit_writeback(scalar_unit);
	si_scalar_unit_execute(scalar_unit);
	si_scalar_unit_read(scalar_unit);
}

