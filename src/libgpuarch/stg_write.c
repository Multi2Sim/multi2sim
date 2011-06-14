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



void gpu_compute_unit_write(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_uop_t *uop;
	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	int subwavefront_id;

	/* Check if write stage is active */
	if (!EXECUTE_WRITE.do_write)
		return;
	
	/* Get instruction */
	uop = EXECUTE_WRITE.uop;
	work_group = uop->work_group;
	wavefront = uop->wavefront;
	subwavefront_id = EXECUTE_WRITE.subwavefront_id;

	/* Debug */
	gpu_pipeline_debug("uop "
		"a=\"stg\" "
		"id=%lld "
		"subwf=%d "
		"cu=%d "
		"stg=\"write\""
		"\n",
		(long long) uop->id,
		subwavefront_id,
		compute_unit->id);
	
	/* Last 'subwavefront_id', free uop.
	 * If it is the last uop in work-group, set compute unit as idle.*/
	if (subwavefront_id == uop->subwavefront_count - 1) {
		if (uop->last) {
			DOUBLE_LINKED_LIST_REMOVE(gpu, busy, compute_unit);
			DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, idle, compute_unit);
			gpu_pipeline_debug("cu "
				"cu=\"%d\" "
				"wg=\"%d\" "
				"a=\"finish\""
				"\n",
				compute_unit->id,
				work_group->id);
		}
		gpu_uop_free(uop);
	}
	
	/* By default, do not write next cycle */
	EXECUTE_WRITE.do_write = 0;
}

