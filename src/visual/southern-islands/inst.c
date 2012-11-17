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
		{ "d", vi_si_inst_stage_decode },

		{ "bu-r", vi_si_inst_stage_branch_read },
		{ "bu-e", vi_si_inst_stage_branch_execute },
		{ "bu-w", vi_si_inst_stage_branch_writeback },

		{ "su-r", vi_si_inst_stage_scalar_read },
		{ "su-e", vi_si_inst_stage_scalar_execute },
		{ "su-w", vi_si_inst_stage_scalar_writeback },

		{ "mem-r", vi_si_inst_stage_mem_read },
		{ "mem-e", vi_si_inst_stage_mem_execute },
		{ "mem-w", vi_si_inst_stage_mem_writeback },

		{ "simd-r", vi_si_inst_stage_simd_read },
		{ "simd-e", vi_si_inst_stage_simd_execute },
		{ "simd-w", vi_si_inst_stage_simd_writeback },

		{ "lds-r", vi_si_inst_stage_lds_read },
		{ "lds-e", vi_si_inst_stage_lds_execute },
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
		{ "#44FF44", vi_si_inst_stage_decode },

		/* Orange */
		{ "#FFBE00", vi_si_inst_stage_branch_read },
		{ "#FFA500", vi_si_inst_stage_branch_execute },
		{ "#FF8C00", vi_si_inst_stage_branch_writeback },

		/* Red */
		{ "#FF8888", vi_si_inst_stage_scalar_read },
		{ "#FF5555", vi_si_inst_stage_scalar_execute },
		{ "#FF2222", vi_si_inst_stage_scalar_writeback },

		/* Turquoise */
		{ "#AFEEEE", vi_si_inst_stage_mem_read },
		{ "#40E0D0", vi_si_inst_stage_mem_execute },
		{ "#00CED1", vi_si_inst_stage_mem_writeback },

		/* Purple */
		{ "#DDA0DD", vi_si_inst_stage_simd_read },
		{ "#DA70D6", vi_si_inst_stage_simd_execute },
		{ "#BA55D3", vi_si_inst_stage_simd_writeback },

		/* Blue */
		{ "#97DEFB", vi_si_inst_stage_lds_read },
		{ "#87CEFA", vi_si_inst_stage_lds_execute },
		{ "#00BFFF", vi_si_inst_stage_lds_writeback },
	}
};


struct str_map_t vi_si_inst_stage_name_map =
{
	vi_si_inst_stage_count - 1,
	{
		{ "x", vi_si_inst_stage_stalled },

		{ "F", vi_si_inst_stage_fetch },
		{ "D", vi_si_inst_stage_decode },

		{ "BR", vi_si_inst_stage_branch_read },
		{ "BE", vi_si_inst_stage_branch_execute },
		{ "BW", vi_si_inst_stage_branch_writeback },

		{ "SR", vi_si_inst_stage_scalar_read },
		{ "SE", vi_si_inst_stage_scalar_execute },
		{ "SW", vi_si_inst_stage_scalar_writeback },

		{ "MR", vi_si_inst_stage_mem_read },
		{ "ME", vi_si_inst_stage_mem_execute },
		{ "MW", vi_si_inst_stage_mem_writeback },

		{ "VR", vi_si_inst_stage_simd_read },
		{ "VE", vi_si_inst_stage_simd_execute },
		{ "VW", vi_si_inst_stage_simd_writeback },

		{ "LR", vi_si_inst_stage_lds_read },
		{ "LE", vi_si_inst_stage_lds_execute },
		{ "LW", vi_si_inst_stage_lds_writeback },
	}
};


struct vi_si_inst_t *vi_si_inst_create(char *name, long long id, int compute_unit_id, 
	int inst_buffer_id, int work_group_id, int wavefront_id, enum vi_si_inst_stage_t stage,
        char *asm_code)

{
	struct vi_si_inst_t *inst;

	/* Initialize */
	inst = xcalloc(1, sizeof(struct vi_si_inst_t));
	inst->name = str_set(NULL, name);
	inst->id = id;
	inst->compute_unit_id = compute_unit_id;
	inst->inst_buffer_id = inst_buffer_id;
	inst->work_group_id = work_group_id;
	inst->wavefront_id = wavefront_id;
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
	str_printf(&buf, &size, "%s<b>I-%lld IB-%d WF-%d</b>%s", begin_color, inst->id, 
		inst->inst_buffer_id, inst->wavefront_id, end_color);

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

	/* Write assembly code */
	str_write_to_file(f, inst->asm_code);
}
