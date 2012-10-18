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


int si_gpu_branch_unit_reg_latency = 1;
int si_gpu_branch_unit_exec_latency = 4;
int si_gpu_branch_unit_width = 1;

void si_branch_unit_writeback(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int list_entries;

	/* Process completed instructions */
	list_entries = list_count(branch_unit->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_branch_unit_exec_latency * si_gpu_branch_unit_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(branch_unit->exec_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			list_remove(branch_unit->exec_buffer, uop);

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"bu-w\"\n", 
				uop->id_in_compute_unit, branch_unit->compute_unit->id, 
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
			branch_unit->inst_count++;
			branch_unit->wavefront_count++;
			si_gpu->last_complete_cycle = esim_cycle;
		}
		else
		{
			break;
		}
	}
}

void si_branch_unit_execute(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;

	list_entries = list_count(branch_unit->read_buffer);

	/* Sanity check the read buffer.  Register accesses are not pipelined, so
	 * buffer size is not multiplied by the latency. */
	assert(list_entries <= si_gpu_branch_unit_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(branch_unit->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_processed == si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, branch_unit->compute_unit->id, 
				uop->wavefront->id);
			break;
		}

		/* Branch */
		uop->execute_ready = si_gpu->cycle + si_gpu_branch_unit_exec_latency;

		/* Transfer the uop to the outstanding execution buffer */
		list_remove(branch_unit->read_buffer, uop);
		list_enqueue(branch_unit->exec_buffer, uop);

		instructions_processed++;
		si_trace("si.inst id=%lld cu=%d wf=%d stg=\"bu-e\"\n", uop->id_in_compute_unit,
			branch_unit->compute_unit->id, uop->wavefront->id);
	}
}

void si_branch_unit_read(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;

	list_entries = list_count(branch_unit->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_decode_latency * si_gpu_decode_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(branch_unit->decode_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_processed == si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, branch_unit->compute_unit->id, 
				uop->wavefront->id);
			break;
		}

		/* Issue the uop if the read buffer is not full */
		if (list_count(branch_unit->read_buffer) < si_gpu_branch_unit_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_branch_unit_reg_latency;
			list_remove(branch_unit->decode_buffer, uop);
			list_enqueue(branch_unit->read_buffer, uop);

			instructions_processed++;

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"bu-r\"\n", 
				uop->id_in_compute_unit, branch_unit->compute_unit->id, 
				uop->wavefront->id);
		}
		else
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, branch_unit->compute_unit->id, 
				uop->wavefront->id);
			break;
		}
	}
}

void si_branch_unit_run(struct si_branch_unit_t *branch_unit)
{
	si_branch_unit_writeback(branch_unit);
	si_branch_unit_execute(branch_unit);
	si_branch_unit_read(branch_unit);
}
