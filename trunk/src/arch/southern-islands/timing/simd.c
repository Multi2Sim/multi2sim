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

int si_gpu_simd_width = 1;

int si_gpu_simd_decode_buffer_size = 5;

/*
 * Register accesses are not pipelined, so buffer size is not
 * multiplied by the latency.
 */
int si_gpu_simd_read_latency = 1;
int si_gpu_simd_read_buffer_size = 1;

/* Note that the SIMD ALU latency is the latency for one sub-wavefront
 * to execute, not an entire wavefront.
 */
int si_gpu_simd_alu_latency = 8;
int si_gpu_simd_num_subwavefronts = 4;

void si_simd_writeback(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;

	list_entries = list_count(simd->exec_buffer);

	/* Sanity check alu buffer */
	assert(list_count(simd->exec_buffer) <=
			si_gpu_simd_width * si_gpu_simd_alu_latency);

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
			si_gpu->last_complete_cycle = esim_cycle;
		}
		else
		{
			/* The uop has not been fully executed yet. It is safe
			 * to assume that no other uop is ready either */
			break;
		}
	}
}

void si_simd_execute(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;
	int num_subwavefronts;
	int subwavefront_size = si_emu_wavefront_size / si_gpu_simd_num_subwavefronts;

	list_entries = list_count(simd->read_buffer);

	/* Sanity check the read buffer. */
	assert(list_entries <= si_gpu_simd_read_buffer_size);

	for(instructions_processed = 0; instructions_processed < si_gpu_simd_width;)
	{
		/* Check to see if there is a wavefront in the sub-wavefront pool. */
		uop = simd->subwavefront_pool->uop;
		if(uop)
		{
			/* Execute one sub-wavefront. */
			simd->subwavefront_pool->num_subwavefronts_executed++;
			instructions_processed++;

			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"simd-e\"\n",
					uop->id_in_compute_unit, simd->compute_unit->id,
					uop->wavefront->id);

			/* Check to see if the wavefront has finished all its
			 * sub-wavefronts. If so remove the wavefront to make room
			 * for the next, if not continue as no other wavefront can be issued.
			 *
			 * num_subwavefronts = ceil(work_item_count / subwavefront_size)
			 * subwavefront_size = wavefront_size / num_subwavefronts
			 */
			num_subwavefronts = (uop->wavefront->work_item_count % subwavefront_size) ?
					uop->wavefront->work_item_count / subwavefront_size + 1 :
					uop->wavefront->work_item_count / subwavefront_size;
			if(simd->subwavefront_pool->num_subwavefronts_executed >=
					num_subwavefronts)
			{
				/* Remove the completed wavefront from the sub-wavefront pool.
				 * It will be ready once this last sub-wavefront completes. */
				uop->execute_ready = si_gpu->cycle + si_gpu_simd_alu_latency;
				list_enqueue(simd->exec_buffer, uop);
				simd->subwavefront_pool->uop = NULL;
				simd->subwavefront_pool->num_subwavefronts_executed = 0;
			}
			else
				continue;
		}

		/* Look to the read buffer looking for wavefronts
		 * ready to issue to the sub-wavefront pool. */

		/* Stop if there are no waiting wavefronts. */
		if(!list_entries)
			break;

		uop = list_head(simd->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Add the next wavefront. */
		list_remove(simd->read_buffer, uop);
		simd->subwavefront_pool->uop = uop;
		simd->subwavefront_pool->num_subwavefronts_executed = 0;
	}
}

void si_simd_read(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;

	list_entries = list_count(simd->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_simd_decode_buffer_size);

	for (int i = 0; i < list_entries; i++)
	{
		uop = list_head(simd->decode_buffer);
		assert(uop);

		/* Stop if the issue width has been reached */
		if (instructions_processed == si_gpu_simd_width)
			break;

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the read_buffer is full. */
		if (list_count(simd->read_buffer) >= si_gpu_simd_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n",
				uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		uop->read_ready = si_gpu->cycle + si_gpu_simd_read_latency;
		list_remove(simd->decode_buffer, uop);
		list_enqueue(simd->read_buffer, uop);

		instructions_processed++;

		si_trace("si.inst id=%lld cu=%d wf=%d stg=\"simd-r\"\n",
			uop->id_in_compute_unit, simd->compute_unit->id,
			uop->wavefront->id);
	}
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
	si_simd_read(simd);
}
