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
#include <lib/esim/trace.h>
#include <lib/util/list.h>

#include "branch-unit.h"
#include "compute-unit.h"
#include "gpu.h"
#include "uop.h"
#include "wavefront-pool.h"

void si_branch_unit_complete(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(branch_unit->write_buffer);

	/* Sanity check the write buffer */
	assert(list_entries <= si_gpu_branch_unit_write_latency * 
		si_gpu_branch_unit_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->write_buffer, list_index);
		assert(uop);

		if (asTiming(si_gpu)->cycle < uop->write_ready)
		{
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(branch_unit->write_buffer, uop);

		si_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_compute_unit,
			uop->compute_unit->id);

		/* Allow next instruction to be fetched */
		uop->wavefront_pool_entry->ready = 1;

		/* Free uop */
		si_uop_free(uop);

		/* Statistics */
		branch_unit->inst_count++;
		si_gpu->last_complete_cycle = asTiming(si_gpu)->cycle;
	}
}

void si_branch_unit_write(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(branch_unit->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= si_gpu_branch_unit_exec_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->exec_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(si_gpu)->cycle < uop->execute_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached. */
		if (instructions_processed > si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check the write buffer */
		assert(list_count(branch_unit->write_buffer) <= 
			si_gpu_branch_unit_write_buffer_size);

		/* Stall if the write buffer is full. */
		if (list_count(branch_unit->write_buffer) == 
			si_gpu_branch_unit_write_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->write_ready = asTiming(si_gpu)->cycle + 
			si_gpu_branch_unit_write_latency;
		list_remove(branch_unit->exec_buffer, uop);
		list_enqueue(branch_unit->write_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"bu-w\"\n", uop->id_in_compute_unit, 
			branch_unit->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_branch_unit_execute(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;
	int list_index = 0;
	int i;

	list_entries = list_count(branch_unit->read_buffer);

	/* Sanity check the read buffer */
	assert(list_entries <= si_gpu_branch_unit_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->read_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(si_gpu)->cycle < uop->read_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check the exec buffer */
		assert(list_count(branch_unit->exec_buffer) <= 
				si_gpu_branch_unit_exec_buffer_size);

		/* Stall if the exec buffer is full. */
		if (list_count(branch_unit->exec_buffer) == 
				si_gpu_branch_unit_exec_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Branch */
		uop->execute_ready = asTiming(si_gpu)->cycle + 
			si_gpu_branch_unit_exec_latency;

		/* Transfer the uop to the outstanding execution buffer */
		list_remove(branch_unit->read_buffer, uop);
		list_enqueue(branch_unit->exec_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"bu-e\"\n", uop->id_in_compute_unit, 
			branch_unit->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_branch_unit_read(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(branch_unit->decode_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= si_gpu_branch_unit_decode_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(si_gpu)->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check the read buffer */
		assert(list_count(branch_unit->read_buffer) <= 
				si_gpu_branch_unit_read_buffer_size);

		/* Stall if the read buffer is full. */
		if (list_count(branch_unit->read_buffer) == 
			si_gpu_branch_unit_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->read_ready = asTiming(si_gpu)->cycle + 
			si_gpu_branch_unit_read_latency;

		list_remove(branch_unit->decode_buffer, uop);
		list_enqueue(branch_unit->read_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"bu-r\"\n", uop->id_in_compute_unit, 
			branch_unit->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_branch_unit_decode(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(branch_unit->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= si_gpu_branch_unit_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(branch_unit->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(si_gpu)->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check the decode buffer */
		assert(list_count(branch_unit->decode_buffer) <= 
			si_gpu_branch_unit_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(branch_unit->decode_buffer) == 
			si_gpu_branch_unit_decode_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				branch_unit->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->decode_ready = asTiming(si_gpu)->cycle + 
			si_gpu_branch_unit_decode_latency;

		list_remove(branch_unit->issue_buffer, uop);
		list_enqueue(branch_unit->decode_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"bu-d\"\n", uop->id_in_compute_unit, 
			branch_unit->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_branch_unit_run(struct si_branch_unit_t *branch_unit)
{
	si_branch_unit_complete(branch_unit);
	si_branch_unit_write(branch_unit);
	si_branch_unit_execute(branch_unit);
	si_branch_unit_read(branch_unit);
	si_branch_unit_decode(branch_unit);
}
