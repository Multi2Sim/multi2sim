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


#include <arch/southern-islands/emu/wavefront.h>
#include <arch/southern-islands/emu/work-group.h>
#include <lib/esim/trace.h>
#include <lib/util/list.h>

#include "compute-unit.h"
#include "gpu.h"
#include "scalar-unit.h"
#include "uop.h"
#include "wavefront-pool.h"


void si_scalar_unit_complete(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop = NULL;
	int i;
	int list_entries;
	int list_index = 0;
	unsigned int barrier_complete;
	int wavefront_id;

	/* Process completed instructions */
	list_entries = list_count(scalar_unit->write_buffer);

	/* Sanity check the write buffer */
	assert(list_entries <= si_gpu_scalar_unit_write_buffer_size);
	
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->write_buffer, list_index);
		assert(uop);

		if (asTiming(si_gpu)->cycle < uop->write_ready)
		{
			/* Uop is not ready yet */
			list_index++;
			continue;
		}

		/* If this is the last instruction and there are outstanding
		 * memory operations, wait for them to complete */
		if (uop->wavefront_last_inst && 
			(uop->wavefront_pool_entry->lgkm_cnt || 
			 uop->wavefront_pool_entry->vm_cnt || 
			 uop->wavefront_pool_entry->exp_cnt)) 
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);

			list_index++;
			continue;
		}

		/* Decrement the outstanding memory access count */
		if (uop->scalar_mem_read)
		{
			assert(uop->wavefront_pool_entry->lgkm_cnt > 0);
			uop->wavefront_pool_entry->lgkm_cnt--;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(scalar_unit->write_buffer, uop);

		/* Scalar ALU instructions must complete before the next
		 * instruction can be fetched */
		if (!uop->scalar_mem_read)
		{
			uop->wavefront_pool_entry->ready = 1;
		}

		/* Check for "wait" instruction */
		if (uop->mem_wait_inst)
		{
			/* If a wait instruction was executed and there are 
			 * outstanding memory accesses, set the wavefront to 
			 * waiting */
			uop->wavefront_pool_entry->wait_for_mem = 1;
		}

		/* Check for "barrier" instruction */
		if (uop->barrier_wait_inst)
		{
			/* Set a flag to wait until all wavefronts have 
			 * reached the barrier */
			assert(!uop->wavefront_pool_entry->wait_for_barrier);
			uop->wavefront_pool_entry->wait_for_barrier = 1;

			/* Check if all wavefronts have reached the barrier */
			barrier_complete = 1;
			SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(uop->work_group, 
				wavefront_id)
			{
				if(!uop->work_group->wavefronts[wavefront_id]->
					wavefront_pool_entry->wait_for_barrier)
				{
					barrier_complete = 0;
				}
			}

			/* If all wavefronts have reached the barrier, 
			 * clear their flags */
			if (barrier_complete)
			{
				SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(
					uop->work_group, wavefront_id)
				{
					assert(uop->work_group->
						wavefronts[wavefront_id]->
						wavefront_pool_entry->
						wait_for_barrier);
					uop->work_group->
						wavefronts[wavefront_id]->
						wavefront_pool_entry->
						wait_for_barrier = 0;
				}
			}
		}

		/* Check for "end" instruction */
		if (uop->wavefront_last_inst)
		{
			/* If the uop completes the wavefront, set a bit 
			 * so that the hardware wont try to fetch any 
			 * more instructions for it */
			uop->wavefront_pool_entry->wavefront_finished = 1;
			uop->work_group->wavefronts_completed_timing++;

			if (uop->work_group->wavefronts_completed_timing ==
				uop->work_group->wavefront_count)
			{
				uop->work_group->finished_timing = 1;
			}

			/* Check if wavefront finishes a work-group */
			assert(uop->work_group);
			assert(uop->work_group->wavefronts_completed_timing <=
				uop->work_group->wavefront_count);
			if (uop->work_group->finished_timing)
			{
				assert(uop->work_group->
					wavefronts_completed_timing == 
					uop->work_group->wavefront_count);

				si_compute_unit_unmap_work_group(
					scalar_unit->compute_unit,
					uop->work_group);
			}
		}

		si_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_compute_unit,
			uop->compute_unit->id);

		/* Free uop */
		si_uop_free(uop);

		/* Statistics */
		scalar_unit->inst_count++;
		si_gpu->last_complete_cycle = asTiming(si_gpu)->cycle;
	}
}

void si_scalar_unit_write(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(scalar_unit->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_scalar_unit_exec_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->exec_buffer, list_index);
		assert(uop);

		instructions_processed++;

		if (uop->scalar_mem_read)
		{
			/* Check if the access is complete */
			if (uop->global_mem_witness)
			{
				list_index++;
				continue;
			}

			/* Stall if the width has been reached */
			if (instructions_processed > si_gpu_scalar_unit_width)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_compute_unit, 
					scalar_unit->compute_unit->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			/* Sanity check write buffer */
			assert(list_count(scalar_unit->write_buffer) <= 
				si_gpu_scalar_unit_write_buffer_size);

			/* Stall if there is not room in the exec buffer */
			if (list_count(scalar_unit->write_buffer) == 
				si_gpu_scalar_unit_write_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_compute_unit, 
					scalar_unit->compute_unit->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}
			
			uop->write_ready = asTiming(si_gpu)->cycle + 
				si_gpu_scalar_unit_write_latency;

			list_remove(scalar_unit->exec_buffer, uop);
			list_enqueue(scalar_unit->write_buffer, uop);

			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-w\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
		}
		else /* ALU instruction */ 
		{
			/* Uop is not ready yet */
			if (asTiming(si_gpu)->cycle < uop->execute_ready)
			{
				list_index++;
				continue;
			}

			/* Stall if the width has been reached */
			if (instructions_processed > si_gpu_scalar_unit_width)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_compute_unit, 
					scalar_unit->compute_unit->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			/* Sanity check write buffer */
			assert(list_count(scalar_unit->write_buffer) <= 
				si_gpu_scalar_unit_write_buffer_size);

			/* Stall if the write buffer is full */
			if (list_count(scalar_unit->write_buffer) == 
				si_gpu_scalar_unit_write_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_compute_unit, 
					scalar_unit->compute_unit->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			uop->write_ready = asTiming(si_gpu)->cycle + 
				si_gpu_scalar_unit_write_latency;

			list_remove(scalar_unit->exec_buffer, uop);
			list_enqueue(scalar_unit->write_buffer, uop);

			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-w\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
		}
	}
}

void si_scalar_unit_execute(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int list_entries;
	int list_index = 0;
	int instructions_processed = 0;
	int i;

	list_entries = list_count(scalar_unit->read_buffer);

	/* Sanity check the read buffer. */
	assert(list_entries <= si_gpu_scalar_unit_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->read_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(si_gpu)->cycle < uop->read_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached */
		if (instructions_processed > si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check exec buffer */
		assert(list_count(scalar_unit->exec_buffer) <= 
			si_gpu_scalar_unit_exec_buffer_size);

		/* Stall if there is not room in the exec buffer */
		if (list_count(scalar_unit->exec_buffer) == 
			si_gpu_scalar_unit_exec_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		if (uop->scalar_mem_read) 
		{
			/* Access global memory */
			uop->global_mem_witness--;
			/* FIXME Get rid of dependence on wavefront here */
			uop->global_mem_access_addr =
				uop->wavefront->scalar_work_item->
				global_mem_access_addr;
			mod_access(scalar_unit->compute_unit->scalar_cache,
				mod_access_load, uop->global_mem_access_addr,
				&uop->global_mem_witness, NULL, NULL, NULL);

			/* Transfer the uop to the execution buffer */
			list_remove(scalar_unit->read_buffer, uop);
			list_enqueue(scalar_unit->exec_buffer, uop);

			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-m\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, 
				uop->id_in_wavefront);
			continue;

		}
		else /* ALU Instruction */
		{
			uop->execute_ready = asTiming(si_gpu)->cycle + 
				si_gpu_scalar_unit_exec_latency;

			/* Transfer the uop to the execution buffer */
			list_remove(scalar_unit->read_buffer, uop);
			list_enqueue(scalar_unit->exec_buffer, uop);

			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-e\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			continue;
		}
	}
}

void si_scalar_unit_read(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(scalar_unit->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_scalar_unit_decode_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(si_gpu)->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the decode width has been reached */
		if (instructions_processed > si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		assert(list_count(scalar_unit->read_buffer) <= 
			si_gpu_scalar_unit_read_buffer_size);

		/* Stall if the read buffer is full */
		if (list_count(scalar_unit->read_buffer) == 
			si_gpu_scalar_unit_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->read_ready = asTiming(si_gpu)->cycle + 
			si_gpu_scalar_unit_read_latency;

		list_remove(scalar_unit->decode_buffer, uop);
		list_enqueue(scalar_unit->read_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"su-r\"\n", uop->id_in_compute_unit, 
			scalar_unit->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_scalar_unit_decode(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(scalar_unit->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= si_gpu_scalar_unit_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(si_gpu)->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		assert(list_count(scalar_unit->decode_buffer) <= 
				si_gpu_scalar_unit_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(scalar_unit->decode_buffer) == 
				si_gpu_scalar_unit_decode_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->decode_ready = asTiming(si_gpu)->cycle + 
			si_gpu_scalar_unit_decode_latency;

		list_remove(scalar_unit->issue_buffer, uop);
		list_enqueue(scalar_unit->decode_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"su-d\"\n", uop->id_in_compute_unit, 
			scalar_unit->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_scalar_unit_run(struct si_scalar_unit_t *scalar_unit)
{
	si_scalar_unit_complete(scalar_unit);
	si_scalar_unit_write(scalar_unit);
	si_scalar_unit_execute(scalar_unit);
	si_scalar_unit_read(scalar_unit);
	si_scalar_unit_decode(scalar_unit);
}

