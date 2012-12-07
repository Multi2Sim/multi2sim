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

#include <arch/southern-islands/emu/wavefront.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/list.h>

#include "branch-unit.h"
#include "compute-unit.h"
#include "gpu.h"
#include "uop.h"
#include "wavefront-pool.h"


/* Configurable by user at runtime */

int si_gpu_branch_unit_width = 1;
int si_gpu_branch_unit_issue_buffer_size = 4;

/* 
 * Register accesses are not pipelined, so buffer size is not
 * multiplied by the latency.
 */
int si_gpu_branch_unit_read_latency = 1;
int si_gpu_branch_unit_read_buffer_size = 1;

/* Note that the SIMD ALU latency is the latency for one sub-wavefront to
 * execute, not an entire wavefront. */
int si_gpu_branch_unit_exec_latency = 4;

void si_branch_unit_writeback(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;

	int list_index = 0;
	int list_entries;
	int i;

	list_entries = list_count(branch_unit->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_branch_unit_exec_latency * si_gpu_branch_unit_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->exec_buffer, list_index);
		assert(uop);

		if (si_gpu->cycle < uop->execute_ready)
		{
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(branch_unit->exec_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"bu-w\"\n",
			uop->id_in_compute_unit, branch_unit->compute_unit->id,
			uop->wavefront->id, uop->id_in_wavefront);

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
		branch_unit->inst_count++;
		si_gpu->last_complete_cycle = esim_cycle;
	}
}

void si_branch_unit_execute(struct si_branch_unit_t *branch_unit)
{
	int instructions_processed = 0;
	int list_index = 0;

	struct si_uop_t *uop;

	int list_entries;
	int i;

	list_entries = list_count(branch_unit->read_buffer);

	/* Sanity check the read buffer.  Register accesses are not
	 * pipelined, so buffer size is not multiplied by the latency. */
	assert(list_entries <= si_gpu_branch_unit_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->read_buffer, list_index);
		assert(uop);

		/* Stall if the issue width has been reached. */
		if (instructions_processed == si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", uop->id_in_compute_unit, branch_unit->compute_unit->id, uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Uop is not ready yet */
		if (si_gpu->cycle < uop->read_ready)
		{
			list_index++;
			continue;
		}

		/* Branch */
		uop->execute_ready = si_gpu->cycle + si_gpu_branch_unit_exec_latency;

		/* Transfer the uop to the outstanding execution buffer */
		list_remove(branch_unit->read_buffer, uop);
		list_enqueue(branch_unit->exec_buffer, uop);

		instructions_processed++;
		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"bu-e\"\n",
			uop->id_in_compute_unit, branch_unit->compute_unit->id,
			uop->wavefront->id, uop->id_in_wavefront);
	}
}

void si_branch_unit_read(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;

	int list_entries;
	int instructions_processed = 0;
	int list_index = 0;
	int i;

	list_entries = list_count(branch_unit->issue_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_branch_unit_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->issue_buffer, list_index);
		assert(uop);

		/* Uop not ready yet */
		if (si_gpu->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed == si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n",
				uop->id_in_compute_unit, branch_unit->compute_unit->id,
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}


		/* Stall if the read buffer is full. */
		if (list_count(branch_unit->read_buffer) >=
			si_gpu_branch_unit_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n",
				uop->id_in_compute_unit, branch_unit->compute_unit->id,
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->read_ready = si_gpu->cycle + si_gpu_branch_unit_read_latency;
		list_remove(branch_unit->issue_buffer, uop);
		list_enqueue(branch_unit->read_buffer, uop);

		instructions_processed++;

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"bu-r\"\n",
			uop->id_in_compute_unit, branch_unit->compute_unit->id,
			uop->wavefront->id, uop->id_in_wavefront);
	}
}

void si_branch_unit_run(struct si_branch_unit_t *branch_unit)
{
	si_branch_unit_writeback(branch_unit);
	si_branch_unit_execute(branch_unit);
	si_branch_unit_read(branch_unit);
}
