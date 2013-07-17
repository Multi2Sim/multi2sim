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


#include <arch/fermi/emu/thread-block.h>
#include <lib/esim/trace.h>
#include <lib/util/list.h>
#include <arch/fermi/emu/warp.h>

#include "sm.h"
#include "gpu.h"
#include "simd-unit.h"
#include "uop.h"
#include "warp-inst-queue.h"


void frm_simd_complete(struct frm_simd_t *simd)
{
	struct frm_uop_t *uop;
	int num_insts;
	int list_index = 0;
	int i;

	num_insts = list_count(simd->exec_buffer);

	assert(num_insts <= frm_gpu_simd_exec_buffer_size);

	for (i = 0; i < num_insts; i++)
	{
		uop = list_get(simd->exec_buffer, list_index);
		assert(uop);

		/* Uop not ready */
		if (asTiming(frm_gpu)->cycle < uop->execute_ready)
		{
			list_index++;
			continue;
		}

		/* Check if the last instruction */
		if (uop->warp_last_inst)
		{
			uop->warp_inst_queue_entry->warp_finished = 1;
			uop->thread_block->sm_finished_count++;

			/* Check if warp finishes a work-group */
			assert(uop->thread_block);
			assert(uop->thread_block->sm_finished_count <=
				uop->thread_block->warp_count);
			//if (uop->thread_block->sm_finished_count == 
			//		uop->thread_block->warp_count)
			{
				frm_sm_unmap_thread_block(
					simd->sm,
					uop->thread_block);
			}
		}

		/* Access complete, remove the uop from the queue */
		list_remove(simd->exec_buffer, uop);

		frm_trace("frm.end_inst id=%lld cu=%d\n", uop->id_in_sm,
			uop->sm->id);

		/* Free uop */
		frm_uop_free(uop);

		/* Statistics */
		simd->inst_count++;
		frm_gpu->last_complete_cycle = asTiming(frm_gpu)->cycle;
	}
}

void frm_simd_execute(struct frm_simd_t *simd)
{
	struct frm_uop_t *uop;
	int num_insts;
	int list_index = 0;
	int instructions_processed = 0;
	int i;

	num_insts = list_count(simd->decode_buffer);

	/* Sanity check the decode buffer */
	assert(num_insts <= frm_gpu_simd_decode_buffer_size);

	for (i = 0; i < num_insts; i++)
	{
		uop = list_get(simd->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached */
		if (instructions_processed > frm_gpu_simd_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				simd->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check exec buffer */
		assert(list_count(simd->exec_buffer) <= 
			frm_gpu_simd_exec_buffer_size);

		/* Stall if SIMD unit is full */
		if (list_count(simd->exec_buffer) == 
			frm_gpu_simd_exec_buffer_size)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				simd->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Includes time for pipelined read-exec-write of 
		 * all subwarps */
		uop->execute_ready = asTiming(frm_gpu)->cycle + frm_gpu_simd_exec_latency;

		/* Transfer the uop to the outstanding execution buffer */
		list_remove(simd->decode_buffer, uop);
		list_enqueue(simd->exec_buffer, uop);

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"simd-e\"\n", uop->id_in_sm, 
			simd->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_simd_decode(struct frm_simd_t *simd)
{
	struct frm_uop_t *uop;
	int instructions_processed = 0;
	int num_insts;
	int list_index = 0;
	int i;

	num_insts = list_count(simd->issue_buffer);

	/* Sanity check the issue buffer */
	assert(num_insts <= frm_gpu_simd_issue_buffer_size);

	for (i = 0; i < num_insts; i++)
	{
		uop = list_get(simd->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(frm_gpu)->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > frm_gpu_simd_width)
		{
			frm_trace("frm.inst id=%lld sm=%d w=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				simd->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Sanity check the decode buffer */
		assert(list_count(simd->decode_buffer) <= 
				frm_gpu_simd_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(simd->decode_buffer) == 
			frm_gpu_simd_decode_buffer_size)
		{
			frm_trace("frm.inst id=%lld sm=%d w=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_sm, 
				simd->sm->id, uop->warp->id, 
				uop->id_in_warp);
			list_index++;
			continue;
		}

		uop->decode_ready = asTiming(frm_gpu)->cycle + frm_gpu_simd_decode_latency;
		list_remove(simd->issue_buffer, uop);
		list_enqueue(simd->decode_buffer, uop);

		frm_trace("frm.inst id=%lld sm=%d w=%d uop_id=%lld "
			"stg=\"simd-d\"\n", uop->id_in_sm, 
			simd->sm->id, uop->warp->id, 
			uop->id_in_warp);
	}
}

void frm_simd_run(struct frm_simd_t *simd)
{
	/* SIMD stages */
	frm_simd_complete(simd);
	frm_simd_execute(simd);
	frm_simd_decode(simd);
}
