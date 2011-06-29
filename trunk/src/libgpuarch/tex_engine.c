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


void gpu_tex_engine_fetch(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->tex_engine.wavefront;
	struct gpu_uop_t *uop;

	struct amd_inst_t *inst;
	char str1[MAX_STRING_SIZE], str2[MAX_STRING_SIZE];

	/* If wavefront finished the TEX clause, no more instructions to fetch */
	if (wavefront->clause_kind != GPU_CLAUSE_TEX)
		return;

	/* Create new uop */
	uop = gpu_uop_create();

	/* Emulate CF instruction */
	gpu_wavefront_execute(wavefront);
	inst = &wavefront->tex_inst;

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category)) {
		amd_inst_dump_buf(inst, 0, 0, str1, MAX_STRING_SIZE);
		str_single_spaces(str2, str1, MAX_STRING_SIZE);
		gpu_pipeline_debug("tex a=\"exec\" "
			"cu=%d "
			"wf=%d "
			"inst=\"%s\"\n",
			compute_unit->id,
			wavefront->id,
			str2);
	}



	/* FIXME */
	/////////////////////
	gpu_uop_free(uop);
	if (wavefront->clause_kind != GPU_CLAUSE_TEX)
		compute_unit->tex_engine.wavefront = NULL;
}


void gpu_tex_engine_run(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_wavefront_t *wavefront = compute_unit->tex_engine.wavefront;

	/* If no wavefront assigned to ALU Engine, nothing to do. */
	if (!wavefront)
		return;

	/* ALU Engine stages */
	gpu_tex_engine_fetch(compute_unit);
}
