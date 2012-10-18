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


int si_gpu_simd_alu_latency = 8;
int si_gpu_simd_reg_latency = 1;
int si_gpu_simd_width = 1;
int si_gpu_simd_num_subwavefronts = 4;

void si_simd_writeback(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;

	/* Process completed ALU instructions */
	list_entries = list_count(simd->exec_buffer);

	/* Sanity check the exec buffer */
	assert(list_entries <= (si_gpu_simd_alu_latency + si_gpu_simd_num_subwavefronts - 1) * 
			si_gpu_simd_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(simd->exec_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			list_remove(simd->exec_buffer, uop);

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"simd-w\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
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
			simd->inst_count++;
			simd->wavefront_count++;
			si_gpu->last_complete_cycle = esim_cycle;
		}
		else
		{
			break;
		}
	}
}

void si_simd_execute(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;

	/* Look through the ALU execution buffer looking for wavefronts ready to execute */
	list_entries = list_count(simd->read_buffer);

	/* Sanity check the read buffer.  Register accesses are not pipelined, so
	 * buffer size is not multiplied by the latency. */
	assert(list_entries <= si_gpu_simd_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(simd->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_processed == si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id);
			break;
		}

		/* Stall if exec buffer is full */
		if (list_count(simd->exec_buffer) == 
			(si_gpu_simd_alu_latency+si_gpu_simd_num_subwavefronts-1) * 
			si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id);
			break;
		}

		/* If the uop is ready to begin execution, set the ALU ready and execution
		 * ready cycles */
		uop->execute_ready = si_gpu->cycle + si_gpu_simd_num_subwavefronts + 
			si_gpu_simd_alu_latency - 1;

		list_remove(simd->read_buffer, uop);
		list_enqueue(simd->exec_buffer, uop);

		instructions_processed++;

		si_trace("si.inst id=%lld cu=%d wf=%d stg=\"simd-e\"\n", 
			uop->id_in_compute_unit, simd->compute_unit->id, 
			uop->wavefront->id);
	}
}

void si_simd_read(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;

	list_entries = list_count(simd->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_decode_latency * si_gpu_decode_width);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(simd->decode_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_processed == si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id);
			break;
		}

		/* Issue the uop if the read buffer is not full */
		if (list_count(simd->read_buffer) < si_gpu_simd_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_simd_reg_latency;
			list_remove(simd->decode_buffer, uop);
			list_enqueue(simd->read_buffer, uop);

			instructions_processed++;

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"simd-r\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id);
		}
		else
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id);
			break;
		}
	}
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
	si_simd_read(simd);
}
