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



void gpu_compute_unit_decode(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_uop_t *uop;
	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	int subwavefront_id;

	/* Check if decode stage is active */
	if (!FETCH_DECODE.do_decode)
		return;
	
	/* Decode instruction */
	uop = FETCH_DECODE.uop;
	work_group = uop->work_group;
	wavefront = uop->wavefront;
	subwavefront_id = FETCH_DECODE.subwavefront_id;

	/* Debug */
	gpu_pipeline_debug("uop "
		"a=\"stg\" "
		"id=%lld, "
		"subwf=%d, "
		"stg=\"decode\""
		"\n",
		(long long) uop->id,
		subwavefront_id);
	
	/* Send to 'read' stage */
	DECODE_READ.do_read = 1;
	DECODE_READ.uop = uop;
	DECODE_READ.subwavefront_id = FETCH_DECODE.subwavefront_id;

	/* By default, do not decode next cycle */
	FETCH_DECODE.do_decode = 0;
}

