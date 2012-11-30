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

#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/list.h>
#include <arch/southern-islands/emu/emu.h>
#include <arch/southern-islands/emu/isa.h>
#include <arch/southern-islands/emu/wavefront.h>

#include "compute-unit.h"
#include "gpu.h"
#include "simd-unit.h"
#include "uop.h"
#include "wavefront-pool.h"


/* Configurable by user at runtime */

int si_gpu_simd_width = 1;
int si_gpu_simd_issue_buffer_size = 1;

/* Register accesses are not pipelined, so buffer size is not
 * multiplied by the latency. */
int si_gpu_simd_read_latency = 1;
int si_gpu_simd_read_buffer_size = 1;

/* Note that the SIMD ALU latency is the latency for one sub-wavefront
 * to execute, not an entire wavefront. */
int si_gpu_simd_alu_latency = 8;
int si_gpu_simd_num_subwavefronts = 4;

void si_simd_writeback(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(simd->exec_buffer);

	/* Sanity check alu buffer */
	assert(list_entries <= si_gpu_simd_width * si_gpu_simd_alu_latency);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(simd->exec_buffer, list_index);
		assert(uop);

		if (si_gpu->cycle < uop->execute_ready)
		{
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(simd->exec_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"simd-w\"\n", 
			uop->id_in_compute_unit, simd->compute_unit->id, 
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
		simd->inst_count++;
		si_gpu->last_complete_cycle = esim_cycle;
	}
}

void si_simd_execute(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;
	int list_index = 0;

	int unexecuted_wi_count;
	unsigned int num_mapped;
	unsigned int wi_start;
	unsigned int wi_end;
	unsigned int num_active;
	int i;
	//unsigned int num_not_issued;
	unsigned int subwavefront_size; 

	/* TODO Is it possible to get this to work with a "width" of > 1? */
	assert(si_gpu_simd_width == 1);

	/* Compute utilization coefficient */

	subwavefront_size = si_emu_wavefront_size / si_gpu_simd_num_subwavefronts;

	/* Check to see if there is a wavefront in the sub-wavefront pool */
	uop = simd->subwavefront_pool->uop;
	if (uop)
	{
		/* A sub-wavefront is being executed. The only possible sources of under
		 * utilization are unmapped work-items and inactive work items */

		/* Check how many work-items still need to be issued into the SIMD pipeline */
		unexecuted_wi_count = uop->wavefront->work_item_count - 
			subwavefront_size * simd->subwavefront_pool->num_subwavefronts_executed;
		num_mapped = (unexecuted_wi_count >= subwavefront_size) ? 
			subwavefront_size : unexecuted_wi_count;

		/* Consider all mapped functional units to determine how many are active */
		wi_start = subwavefront_size * simd->subwavefront_pool->num_subwavefronts_executed;
		wi_end = subwavefront_size * simd->subwavefront_pool->num_subwavefronts_executed + 
			num_mapped - 1;
		num_active = 0;
		for (int i = wi_start; i <= wi_end; i++)
		{
			if (si_isa_read_bitmask_sreg(uop->wavefront->work_items[i], SI_EXEC)) 
			{
				num_active++;
			}
		}

		/* Consider all active, mapped and utilized functional units for all utilization
		 * metrics of higher precedence than wki_util. Consider all active, utilized functional
		 * units and all unmapped functional units for wki_util. Consider all active,
		 * utilized functional units and all mapped yet inactive functional units for act_util */
		simd->wkg_util->cycles_utilized += num_active;
		simd->wkg_util->cycles_considered += num_active;
		simd->wvf_util->cycles_utilized += num_active;
		simd->wvf_util->cycles_considered += num_active;
		simd->rdy_util->cycles_utilized += num_active;
		simd->rdy_util->cycles_considered += num_active;
		simd->occ_util->cycles_utilized += num_active;
		simd->occ_util->cycles_considered += num_active;
		simd->wki_util->cycles_utilized += num_active;
		simd->wki_util->cycles_considered += num_active + (subwavefront_size - num_mapped);
		simd->act_util->cycles_utilized += num_active;
		simd->act_util->cycles_considered += num_mapped;

		/* Execute one sub-wavefront. */
		simd->subwavefront_pool->num_subwavefronts_executed++;
		unexecuted_wi_count -= num_mapped;

		/* Check to see if the wavefront has finished all its
		 * sub-wavefronts. If so, remove the wavefront to make room
		 * for the next, if not continue as no other wavefront can be issued. */
		assert(unexecuted_wi_count >= 0);
		if (!unexecuted_wi_count)
		{
			/* Remove the completed wavefront from the sub-wavefront pool.
			 * It will be ready once this last sub-wavefront completes. */
			// XXX DANA added -1 because the stage the last subwavefront
			//     was issued should count as cycle 1, correct?
			uop->execute_ready = si_gpu->cycle + si_gpu_simd_alu_latency - 1;
			list_enqueue(simd->exec_buffer, uop);
			simd->subwavefront_pool->uop = NULL;
			simd->subwavefront_pool->num_subwavefronts_executed = 0;
		}
	}

	list_entries = list_count(simd->read_buffer);

	/* Sanity check the read buffer. */
	assert(list_entries <= si_gpu_simd_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		/* Look to the read buffer looking for wavefronts
		 * ready to issue to the sub-wavefront pool. */
		uop = list_get(simd->read_buffer, list_index);
		assert(uop);

        /* Uop is not ready yet */
		if (si_gpu->cycle < uop->read_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached */
		if (instructions_processed == si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}
        
		/* Stall if another wavefront is still issuing sub-wavefronts 
		 * into the pipeline. */
		if (simd->subwavefront_pool->uop)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

        instructions_processed++;

		/* Add the next wavefront. */
		list_remove(simd->read_buffer, uop);
		assert(!simd->subwavefront_pool->uop);
		simd->subwavefront_pool->uop = uop;
		simd->subwavefront_pool->num_subwavefronts_executed = 0;

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"simd-e\"\n", 
			uop->id_in_compute_unit, simd->compute_unit->id, 
			uop->wavefront->id, uop->id_in_wavefront);
	}



	/* Utilization Coefficient. */

	/* Check how many sub-wavefronts have been executed
	 * and if the full SIMD width has been used. If not,
	 * it could be from occ, rdy or wvf_util. */

//	if (instructions_processed < si_gpu_simd_width)
//	{
		/* The utilized functional units have already been
		 * considered for all util counters. Determine the number
		 * of underutilized functional units from the lack of
		 * issued sub-wavefronts. */
//		num_not_issued = (si_gpu_simd_width - instructions_processed) * subwavefront_size;

		/* Check if there is at least one wavefront mapped to the IB. If not, consider
		 * wvf_util, otherwise consider either rdy or occ. */
//		if (simd->compute_unit->wavefront_pools[uop->wavefront_pool_id]->wavefront_count)
//		{
			/* Check if there exists a wavefront with the appropriate instruction waiting to be
			 * executed. If so, the underutilized functional units are considered in rdy,
			 * otherwise they are considered in occ. A wavefront is waiting to be executed
			 * if either it is waiting at the instruction buffer with a SIMD instruction next
			 * or if it is not at the instruction buffer and its previous instruction was a SIMD
			 * (meaning it is in the SIMD pipeline but has not reached execution). */
//			for (int i = 0; i < simd->compute_unit->wavefront_pools[uop->wavefront_pool_id]->wavefront_count; i++)
//				if (simd->compute_unit->wavefront_pools[uop->wavefront_pool_id]->entries[i]->ready &&
//						simd->compute_unit->wavefront_pools[uop->wavefront_pool_id]->entries[i]->wavefront->)
//				{

//					break;
//				}
//		}
//		else
//		{

//		}
//	}

}

void si_simd_read(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int instructions_processed = 0;
	int list_index = 0;
	int i;

	list_entries = list_count(simd->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= si_gpu_simd_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(simd->issue_buffer, list_index);
		assert(uop);
		assert(uop->wavefront_pool_entry->wavefront_pool == simd->wavefront_pool);	

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached */
		if (instructions_processed == si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}


		/* Stall if the read_buffer is full. */
		assert (list_count(simd->read_buffer) <= si_gpu_simd_read_buffer_size);
		if (list_count(simd->read_buffer) == si_gpu_simd_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_compute_unit, simd->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->read_ready = si_gpu->cycle + si_gpu_simd_read_latency;
		list_remove(simd->issue_buffer, uop);
		list_enqueue(simd->read_buffer, uop);

		instructions_processed++;

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"simd-r\"\n", 
			uop->id_in_compute_unit, simd->compute_unit->id, 
			uop->wavefront->id, uop->id_in_wavefront);
	}
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
	si_simd_read(simd);
}
