/*
 *  Multi2Sim Tools
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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>

#include "inst.h"


struct str_map_t vi_si_inst_stage_map =
{
	vi_si_inst_stage_count - 1,
	{
		{ "s", vi_si_inst_stage_stalled },

		{ "f", vi_si_inst_stage_fetch },
		{ "i", vi_si_inst_stage_issue },

		{ "bu-d", vi_si_inst_stage_branch_decode },
		{ "bu-r", vi_si_inst_stage_branch_read },
		{ "bu-e", vi_si_inst_stage_branch_execute },
		{ "bu-w", vi_si_inst_stage_branch_writeback },

		{ "su-d", vi_si_inst_stage_scalar_decode },
		{ "su-r", vi_si_inst_stage_scalar_read },
		{ "su-e", vi_si_inst_stage_scalar_execute },
		{ "su-m", vi_si_inst_stage_scalar_memory },
		{ "su-w", vi_si_inst_stage_scalar_writeback },

		{ "mem-d", vi_si_inst_stage_mem_decode },
		{ "mem-r", vi_si_inst_stage_mem_read },
		{ "mem-m", vi_si_inst_stage_mem_memory },
		{ "mem-w", vi_si_inst_stage_mem_writeback },

		{ "simd-d", vi_si_inst_stage_simd_decode },
		{ "simd-e", vi_si_inst_stage_simd_execute },

		{ "lds-d", vi_si_inst_stage_lds_decode },
		{ "lds-r", vi_si_inst_stage_lds_read },
		{ "lds-m", vi_si_inst_stage_lds_memory },
		{ "lds-w", vi_si_inst_stage_lds_writeback },
	}
};


struct str_map_t vi_si_inst_stage_color_map =
{
	vi_si_inst_stage_count - 1,
	{
		/* White */
		{ "#FFFFFF", vi_si_inst_stage_stalled },

		/* Green */
		{ "#88FF88", vi_si_inst_stage_fetch },
		{ "#00FF00", vi_si_inst_stage_issue },

		/* Orange */
		{ "#FFFE00", vi_si_inst_stage_branch_decode },
		{ "#FFBE00", vi_si_inst_stage_branch_read },
		{ "#FFA500", vi_si_inst_stage_branch_execute },
		{ "#FF8C00", vi_si_inst_stage_branch_writeback },

		/* Red */
		{ "#FFBBBB", vi_si_inst_stage_scalar_decode },
		{ "#FF8888", vi_si_inst_stage_scalar_read },
		{ "#FF5555", vi_si_inst_stage_scalar_execute },
		{ "#FF2222", vi_si_inst_stage_scalar_memory },
		{ "#FF0000", vi_si_inst_stage_scalar_writeback },

		/* Turquoise */
		{ "#AFEEEE", vi_si_inst_stage_mem_decode },
		{ "#08E8DE", vi_si_inst_stage_mem_read },
		{ "#48D1CC", vi_si_inst_stage_mem_memory },
		{ "#00CED1", vi_si_inst_stage_mem_writeback },

		/* Purple */
		{ "#DDA0DD", vi_si_inst_stage_simd_decode },
		{ "#DA70D6", vi_si_inst_stage_simd_execute },

		/* Blue */
		{ "#A4D3EE", vi_si_inst_stage_lds_decode },
		{ "#97DEFB", vi_si_inst_stage_lds_read },
		{ "#87CEFA", vi_si_inst_stage_lds_memory },
		{ "#00BFFF", vi_si_inst_stage_lds_writeback },
	}
};


struct str_map_t vi_si_inst_stage_name_map =
{
	vi_si_inst_stage_count - 1,
	{
		{ "x", vi_si_inst_stage_stalled },

		{ "F", vi_si_inst_stage_fetch },
		{ "I", vi_si_inst_stage_issue },

		{ "BD", vi_si_inst_stage_branch_decode },
		{ "BR", vi_si_inst_stage_branch_read },
		{ "BE", vi_si_inst_stage_branch_execute },
		{ "BW", vi_si_inst_stage_branch_writeback },

		{ "SD", vi_si_inst_stage_scalar_decode },
		{ "SR", vi_si_inst_stage_scalar_read },
		{ "SE", vi_si_inst_stage_scalar_execute },
		{ "SM", vi_si_inst_stage_scalar_memory },
		{ "SW", vi_si_inst_stage_scalar_writeback },

		{ "MD", vi_si_inst_stage_mem_decode },
		{ "MR", vi_si_inst_stage_mem_read },
		{ "MM", vi_si_inst_stage_mem_memory },
		{ "MW", vi_si_inst_stage_mem_writeback },

		{ "VD", vi_si_inst_stage_simd_decode },
		{ "VE", vi_si_inst_stage_simd_execute },

		{ "LD", vi_si_inst_stage_lds_decode },
		{ "LR", vi_si_inst_stage_lds_read },
		{ "LM", vi_si_inst_stage_lds_memory },
		{ "LW", vi_si_inst_stage_lds_writeback },
	}
};


struct vi_si_inst_t *vi_si_inst_create(char *name, long long id,
	int compute_unit_id, int wfp_id, int work_group_id, 
	int wavefront_id, enum vi_si_inst_stage_t stage, 
	long long int uop_id_in_wavefront, char *asm_code)

{
	struct vi_si_inst_t *inst;

	/* Initialize */
	inst = xcalloc(1, sizeof(struct vi_si_inst_t));
	inst->name = str_set(NULL, name);
	inst->id = id;
	inst->compute_unit_id = compute_unit_id;
	inst->wfp_id = wfp_id;
	inst->work_group_id = work_group_id;
	inst->wavefront_id = wavefront_id;
	inst->uop_id_in_wavefront = uop_id_in_wavefront;
	inst->stage = stage;

	inst->asm_code = str_set(NULL, asm_code);

	/* Return */
	return inst;
}


void vi_si_inst_free(struct vi_si_inst_t *inst)
{
	str_free(inst->name);
	str_free(inst->asm_code);
	free(inst);
}


void vi_si_inst_get_markup(struct vi_si_inst_t *inst, char *buf, int size)
{
	char *begin_color;
	char *end_color;

	/* Color */
	begin_color = "<span color=\"red\">";
	end_color = "</span>";

	/* Instruction ID */
	str_printf(&buf, &size, "%s<b>I-%lld WFP-%d WF-%d UOP-%lld</b>%s", 
		begin_color, inst->id, inst->wfp_id, inst->wavefront_id, 
		inst->uop_id_in_wavefront, end_color);

	/* Assembly */
	if (inst->asm_code && *inst->asm_code)
		str_printf(&buf, &size, " %s", inst->asm_code);
}


void vi_si_inst_read_checkpoint(struct vi_si_inst_t *inst, FILE *f)
{
	int count;

	char name[MAX_STRING_SIZE];
	char asm_code[MAX_STRING_SIZE];

	/* Read instruction id */
	count = fread(&inst->id, 1, sizeof inst->id, f);
	if (count != sizeof inst->id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read stage */
	count = fread(&inst->stage, 1, sizeof inst->stage, f);
	if (count != sizeof inst->stage)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read wavefront pool ID */
	count = fread(&inst->wfp_id, 1, sizeof inst->wfp_id, f);
	if (count != sizeof inst->wfp_id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read work group ID */
	count = fread(&inst->work_group_id, 1, sizeof inst->work_group_id, f);
	if (count != sizeof inst->work_group_id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read wavefront ID */
	count = fread(&inst->wavefront_id, 1, sizeof inst->wavefront_id, f);
	if (count != sizeof inst->wavefront_id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read UOP's ID in wavefront */
	count = fread(&inst->uop_id_in_wavefront, 1, 
		sizeof inst->uop_id_in_wavefront, f);
	if (count != sizeof inst->uop_id_in_wavefront)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read assembly code */
	str_read_from_file(f, asm_code, sizeof asm_code);
	inst->asm_code = str_set(inst->asm_code, asm_code);

	/* Instruction name */
	snprintf(name, sizeof name, "i-%lld", inst->id);
	inst->name = str_set(inst->name, name);
}


void vi_si_inst_write_checkpoint(struct vi_si_inst_t *inst, FILE *f)
{
	int count;

	/* Write instruction id */
	count = fwrite(&inst->id, 1, sizeof inst->id, f);
	if (count != sizeof inst->id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write stage */
	count = fwrite(&inst->stage, 1, sizeof inst->stage, f);
	if (count != sizeof inst->stage)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write wavefront pool ID */
	count = fwrite(&inst->wfp_id, 1, sizeof inst->wfp_id, f);
	if (count != sizeof inst->wfp_id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write work group ID */
	count = fwrite(&inst->work_group_id, 1, sizeof inst->work_group_id, f);
	if (count != sizeof inst->work_group_id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write wavefront ID */
	count = fwrite(&inst->wavefront_id, 1, sizeof inst->wavefront_id, f);
	if (count != sizeof inst->wavefront_id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write UOP's ID in wavefront */
	count = fwrite(&inst->uop_id_in_wavefront, 1, 
		sizeof inst->uop_id_in_wavefront, f);
	if (count != sizeof inst->uop_id_in_wavefront)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write assembly code */
	str_write_to_file(f, inst->asm_code);
}
