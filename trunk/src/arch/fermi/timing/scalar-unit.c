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

#include <arch/common/arch.h>
#include <arch/fermi/emu/emu.h>
#include <arch/fermi/emu/grid.h>
#include <arch/fermi/emu/warp.h>
#include <arch/fermi/emu/thread-block.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/list.h>

#include "sm.h"
#include "gpu.h"
#include "scalar-unit.h"
#include "uop.h"
#include "warp-pool.h"


void frm_scalar_unit_complete(struct frm_scalar_unit_t *scalar_unit)
{
	struct frm_uop_t *uop = NULL;
	int i;
	int list_entries;
	int list_index = 0;
	unsigned int complete;
	struct frm_grid_t *grid= frm_gpu->grid;
	struct frm_thread_block_t *thread_block;
	struct frm_warp_t *warp;
	int warp_id;

	/* Process completed memory instructions */
	list_entries = list_count(scalar_unit->write_buffer);

	/* Sanity check the write buffer */
	assert(list_entries <= frm_gpu_scalar_unit_write_buffer_size);
	
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->write_buffer, list_index);
		assert(uop);

		if (arch_fermi->cycle < uop->write_ready)
		{
			/* Uop is not ready yet */
			list_index++;
			continue;
		}

		/* If this is the last instruction and there are outstanding
		 * memory operations, wait for them to complete */
		if (uop->warp_last_inst && 
			(uop->warp_pool_entry->lgkm_cnt || 
			 uop->warp_pool_entry->vm_cnt || 
			 uop->warp_pool_entry->exp_cnt)) 
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);

			list_index++;
			continue;
		}

		/* Decrement the outstanding memory access count */
		if (uop->scalar_mem_read)
		{
			assert(uop->warp_pool_entry->lgkm_cnt > 0);
			uop->warp_pool_entry->lgkm_cnt--;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(scalar_unit->write_buffer, uop);

		/* The next instruction can be fetched */
		uop->warp_pool_entry->ready = 1;

		/* Check for "wait" instruction */
		if (uop->mem_wait_inst)
		{
			/* If a wait instruction was executed and there are 
			 * outstanding memory accesses, set the warp to 
			 * waiting */
			uop->warp_pool_entry->wait_for_mem = 1;
		}

		/* Check for "barrier" instruction */
		if (uop->barrier_wait_inst)
		{
			/* Set a flag to wait until all warps have 
			 * reached the barrier */
			uop->warp_pool_entry->wait_for_barrier = 1;

			/* Check if all warps have reached the barrier */
			complete = 1;
			thread_block = uop->warp->thread_block;
			FRM_FOREACH_WARP_IN_THREADBLOCK(thread_block, 
				warp_id)
			{
				warp = grid->warps[warp_id];
				
				if (!warp->warp_pool_entry->
					wait_for_barrier)
				{
					complete = 0;
				}
			}

			/* If all warps have reached the barrier, 
			 * clear their flags */
			if (complete)
			{
				FRM_FOREACH_WARP_IN_THREADBLOCK(thread_block, 
					warp_id)
				{
					warp = grid->
						warps[warp_id];
					warp->warp_pool_entry->
						wait_for_barrier = 0;
				}
			}
		}

		/* Check for "end" instruction */
		if (uop->warp_last_inst)
		{
			/* If the uop completes the warp, set a bit 
			 * so that the hardware wont try to fetch any 
			 * more instructions for it */
			uop->thread_block->sm_finished_count++;
			uop->warp_pool_entry->warp_finished = 1;

			/* Check if warp finishes a work-group */
			assert(uop->thread_block);
			assert(uop->thread_block->sm_finished_count <=
				uop->thread_block->warp_count);
			//if (uop->thread_block->sm_finished_count == 
			//		uop->thread_block->warp_count)
			{
				frm_sm_unmap_thread_block(
					scalar_unit->sm,
					uop->thread_block);
			}
		}

		frm_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_sm,
			uop->sm->id);

		/* Free uop */
		frm_uop_free(uop);

		/* Statistics */
		scalar_unit->inst_count++;
		frm_gpu->last_complete_cycle = arch_fermi->cycle;
	}
}

void frm_scalar_unit_write(struct frm_scalar_unit_t *scalar_unit)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(scalar_unit->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= frm_gpu_scalar_unit_exec_buffer_size);

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
			if (instructions_processed > frm_gpu_scalar_unit_width)
			{
				frm_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_sm, 
					scalar_unit->sm->id, 
					uop->warp->id, 
					uop->id_in_warp);
				list_index++;
				continue;
			}

			/* Sanity check write buffer */
			assert(list_count(scalar_unit->write_buffer) <= 
				frm_gpu_scalar_unit_write_buffer_size);

			/* Stall if there is not room in the exec buffer */
			if (list_count(scalar_unit->write_buffer) == 
				frm_gpu_scalar_unit_write_buffer_size)
			{
				frm_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_sm, 
					scalar_unit->sm->id, 
					uop->warp->id, 
					uop->id_in_warp);
				list_index++;
				continue;
			}
			
			uop->write_ready = arch_fermi->cycle + 
				frm_gpu_scalar_unit_write_latency;

			list_remove(scalar_unit->exec_buffer, uop);
			list_enqueue(scalar_unit->write_buffer, uop);

			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-w\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
		}
		else /* ALU instruction */ 
		{
			/* Uop is not ready yet */
			if (arch_fermi->cycle < uop->execute_ready)
			{
				list_index++;
				continue;
			}

			/* Stall if the width has been reached */
			if (instructions_processed > frm_gpu_scalar_unit_width)
			{
				frm_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_sm, 
					scalar_unit->sm->id, 
					uop->warp->id, 
					uop->id_in_warp);
				list_index++;
				continue;
			}

			/* Sanity check write buffer */
			assert(list_count(scalar_unit->write_buffer) <= 
				frm_gpu_scalar_unit_write_buffer_size);

			/* Stall if the write buffer is full */
			if (list_count(scalar_unit->write_buffer) == 
				frm_gpu_scalar_unit_write_buffer_size)
			{
				frm_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_sm, 
					scalar_unit->sm->id, 
					uop->warp->id, 
					uop->id_in_warp);
				list_index++;
				continue;
			}

			uop->write_ready = arch_fermi->cycle + 
				frm_gpu_scalar_unit_write_latency;

			list_remove(scalar_unit->exec_buffer, uop);
			list_enqueue(scalar_unit->write_buffer, uop);

			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-w\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
		}
	}
}

void frm_scalar_unit_execute(struct frm_scalar_unit_t *scalar_unit)
{
	struct frm_uop_t *uop;
	int list_entries;
	int list_index = 0;
	int instructions_processed = 0;
	int i;

	list_entries = list_count(scalar_unit->read_buffer);

	/* Sanity check the read buffer. */
	assert(list_entries <= frm_gpu_scalar_unit_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->read_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (arch_fermi->cycle < uop->read_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached */
		if (instructions_processed > frm_gpu_scalar_unit_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check exec buffer */
		assert(list_count(scalar_unit->exec_buffer) <= 
			frm_gpu_scalar_unit_exec_buffer_size);

		/* Stall if there is not room in the exec buffer */
		if (list_count(scalar_unit->exec_buffer) == 
			frm_gpu_scalar_unit_exec_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		if (uop->scalar_mem_read) 
		{
			/* Access global memory */
			uop->global_mem_witness--;
			/* FIXME Get rid of dependence on warp here */
			uop->global_mem_access_addr =
				uop->warp->scalar_thread->
				global_mem_access_addr;
			mod_access(scalar_unit->sm->global_memory,
				mod_access_load, uop->global_mem_access_addr,
				&uop->global_mem_witness, NULL, NULL, NULL);

			/* Increment outstanding memory access count */
			uop->warp_pool_entry->lgkm_cnt++;

			/* Transfer the uop to the execution buffer */
			list_remove(scalar_unit->read_buffer, uop);
			list_enqueue(scalar_unit->exec_buffer, uop);

			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-m\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, 
				uop->id_in_warp);
			continue;

		}
		else /* ALU Instruction */
		{
			uop->execute_ready = arch_fermi->cycle + 
				frm_gpu_scalar_unit_exec_latency;

			/* Transfer the uop to the execution buffer */
			list_remove(scalar_unit->read_buffer, uop);
			list_enqueue(scalar_unit->exec_buffer, uop);

			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"su-e\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
			continue;
		}
	}
}

void frm_scalar_unit_read(struct frm_scalar_unit_t *scalar_unit)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(scalar_unit->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= frm_gpu_scalar_unit_decode_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (arch_fermi->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the decode width has been reached */
		if (instructions_processed > frm_gpu_scalar_unit_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		assert(list_count(scalar_unit->read_buffer) <= 
			frm_gpu_scalar_unit_read_buffer_size);

		/* Stall if the read buffer is full */
		if (list_count(scalar_unit->read_buffer) == 
			frm_gpu_scalar_unit_read_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		uop->read_ready = arch_fermi->cycle + 
			frm_gpu_scalar_unit_read_latency;

		list_remove(scalar_unit->decode_buffer, uop);
		list_enqueue(scalar_unit->read_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"su-r\"\n", uop->id_in_sm, 
			scalar_unit->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_scalar_unit_decode(struct frm_scalar_unit_t *scalar_unit)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(scalar_unit->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= frm_gpu_scalar_unit_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(scalar_unit->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (arch_fermi->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > frm_gpu_scalar_unit_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		assert(list_count(scalar_unit->decode_buffer) <= 
				frm_gpu_scalar_unit_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(scalar_unit->decode_buffer) == 
				frm_gpu_scalar_unit_decode_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				scalar_unit->sm->id, 
				uop->warp->id, uop->id_in_warp);
			list_index++;
			continue;
		}

		uop->decode_ready = arch_fermi->cycle + 
			frm_gpu_scalar_unit_decode_latency;

		list_remove(scalar_unit->issue_buffer, uop);
		list_enqueue(scalar_unit->decode_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"su-d\"\n", uop->id_in_sm, 
			scalar_unit->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_scalar_unit_run(struct frm_scalar_unit_t *scalar_unit)
{
	frm_scalar_unit_complete(scalar_unit);
	frm_scalar_unit_write(scalar_unit);
	frm_scalar_unit_execute(scalar_unit);
	frm_scalar_unit_read(scalar_unit);
	frm_scalar_unit_decode(scalar_unit);
}

