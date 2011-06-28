/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpuarch.h>


void gpu_compute_unit_read(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_uop_t *uop;
	struct gpu_ndrange_t *ndrange;
	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	struct gpu_work_item_t *work_item;
	struct gpu_work_item_uop_t *work_item_uop;
	int subwavefront_id;
	int work_item_id;

	/* Check if input is ready */
	if (!DECODE_READ.input_ready)
		return;

	/* Check if execute stage is ready */
	if (READ_EXECUTE.input_ready)
		return;
	
	/* Get instruction */
	uop = DECODE_READ.uop;
	work_group = uop->work_group;
	wavefront = uop->wavefront;
	ndrange = work_group->ndrange;
	subwavefront_id = DECODE_READ.subwavefront_id;

	/* Debug */
	if (!DECODE_READ.stall_cycle) {
		gpu_pipeline_debug("uop "
			"a=\"stg\" "
			"id=%lld "
			"subwf=%d "
			"stg=\"read\""
			"\n",
			(long long) uop->id,
			subwavefront_id);
	}
	
	/* Read from global memory */
	if (uop->global_mem_read) {

		/* If this is the first cycle, perform the read */
		if (!DECODE_READ.stall_cycle) {
			FOREACH_WORK_ITEM_IN_SUBWAVEFRONT(wavefront, subwavefront_id, work_item_id) {
				work_item = ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				gpu_cache_access(
					uop->compute_unit->id,  /* memory hierarchy entry point */
					1,  /* read access */
					work_item_uop->global_mem_access_addr,
					work_item_uop->global_mem_access_size,
					&uop->global_mem_access_witness);  /* witness */
				uop->global_mem_access_witness--;
			}
		}

		/* Stall read stage until read is complete for all work-items.
		 * This is reflected in variable 'uop->global_mem_access_witness' */
		if (uop->global_mem_access_witness) {
			DECODE_READ.stall_cycle++;
			return;
		}
	}
	
	/* Read from local memory */

	/* Send to execute stage */
	READ_EXECUTE.input_ready = 1;
	READ_EXECUTE.uop = uop;
	READ_EXECUTE.subwavefront_id = subwavefront_id;

	/* Instruction consumed by the read stage */
	DECODE_READ.stall_cycle = 0;
	DECODE_READ.input_ready = 0;
}

