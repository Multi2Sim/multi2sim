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


void gpu_mem_access(struct gpu_uop_t *uop, int subwavefront_id)
{
	struct gpu_wavefront_t *wavefront = uop->wavefront;
	struct gpu_ndrange_t *ndrange = wavefront->ndrange;
	struct gpu_work_item_uop_t *work_item_uop;
	struct gpu_work_item_t *work_item;
	int work_item_id;
	
	//printf("WF %d, SubWF %d:", wavefront->id, subwavefront_id);
	FOREACH_WORK_ITEM_IN_SUBWAVEFRONT(wavefront, subwavefront_id, work_item_id) {
		work_item = ndrange->work_items[work_item_id];
		work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];

		gpu_cache_write(uop->compute_unit->id, work_item_uop->global_mem_access_addr,
			work_item_uop->global_mem_access_size);
		//printf(" %d(%d)", work_item_uop->global_mem_access_addr,
			//work_item_uop->global_mem_access_size);
	}
	//printf("\n");
}


void gpu_compute_unit_read(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_uop_t *uop;
	struct gpu_ndrange_t *ndrange;
	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	int subwavefront_id;

	/* Check if read stage is active */
	if (!DECODE_READ.do_read)
		return;
	
	/* Get instruction */
	uop = DECODE_READ.uop;
	work_group = uop->work_group;
	wavefront = uop->wavefront;
	ndrange = work_group->ndrange;
	subwavefront_id = DECODE_READ.subwavefront_id;

	/* Debug */
	gpu_pipeline_debug("uop "
		"a=\"stg\" "
		"id=%lld "
		"subwf=%d "
		"stg=\"read\""
		"\n",
		(long long) uop->id,
		subwavefront_id);
	
	/* Access to global memory */
	if (uop->global_mem_access) {
		///printf("Global memory access: WF=%d, SubWF=%d, instr='%s'\n", wavefront->id, subwavefront_id, uop->inst.info->name);
		gpu_mem_access(uop, subwavefront_id);
	}
	
	/* Send to 'execute' stage */
	READ_EXECUTE.do_execute = 1;
	READ_EXECUTE.uop = uop;
	READ_EXECUTE.subwavefront_id = subwavefront_id;

	/* By default, do not read next cycle */
	DECODE_READ.do_read = 0;
}

