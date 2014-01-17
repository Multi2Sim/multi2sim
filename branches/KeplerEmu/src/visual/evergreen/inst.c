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


struct str_map_t vi_evg_inst_cat_map =
{
	3, {
		{ "cf", vi_evg_inst_cat_cf },
		{ "alu", vi_evg_inst_cat_alu },
		{ "tex", vi_evg_inst_cat_tex }
	}
};


struct str_map_t vi_evg_inst_stage_map =
{
	vi_evg_inst_stage_count - 1,
	{
		{ "cf-fe", vi_evg_inst_stage_cf_fetch },
		{ "cf-de", vi_evg_inst_stage_cf_decode },
		{ "cf-ex", vi_evg_inst_stage_cf_execute },
		{ "cf-co", vi_evg_inst_stage_cf_complete },

		{ "alu-fe", vi_evg_inst_stage_alu_fetch },
		{ "alu-de", vi_evg_inst_stage_alu_decode },
		{ "alu-rd", vi_evg_inst_stage_alu_read },
		{ "alu-ex", vi_evg_inst_stage_alu_execute },
		{ "alu-wr", vi_evg_inst_stage_alu_write },

		{ "tex-fe", vi_evg_inst_stage_tex_fetch },
		{ "tex-de", vi_evg_inst_stage_tex_decode },
		{ "tex-rd", vi_evg_inst_stage_tex_read },
		{ "tex-wr", vi_evg_inst_stage_tex_write }
	}
};


struct str_map_t vi_evg_inst_stage_color_map =
{
	vi_evg_inst_stage_count - 1,
	{
		{ "#88ff88", vi_evg_inst_stage_cf_fetch },
		{ "#44ff44", vi_evg_inst_stage_cf_decode },
		{ "#22ff22", vi_evg_inst_stage_cf_execute },
		{ "#00ff00", vi_evg_inst_stage_cf_complete },

		{ "#ff8888", vi_evg_inst_stage_alu_fetch },
		{ "#ff6666", vi_evg_inst_stage_alu_decode },
		{ "#ff4444", vi_evg_inst_stage_alu_read },
		{ "#ff2222", vi_evg_inst_stage_alu_execute },
		{ "#ff0000", vi_evg_inst_stage_alu_write },

		{ "#ddddff", vi_evg_inst_stage_tex_fetch },
		{ "#bbbbff", vi_evg_inst_stage_tex_decode },
		{ "#9999ff", vi_evg_inst_stage_tex_read },
		{ "#7777ff", vi_evg_inst_stage_tex_write }
	}
};


struct str_map_t vi_evg_inst_stage_name_map =
{
	vi_evg_inst_stage_count - 1,
	{
		{ "FE", vi_evg_inst_stage_cf_fetch },
		{ "DE", vi_evg_inst_stage_cf_decode },
		{ "EX", vi_evg_inst_stage_cf_execute },
		{ "CO", vi_evg_inst_stage_cf_complete },

		{ "FE", vi_evg_inst_stage_alu_fetch },
		{ "DE", vi_evg_inst_stage_alu_decode },
		{ "RD", vi_evg_inst_stage_alu_read },
		{ "EX", vi_evg_inst_stage_alu_execute },
		{ "WR", vi_evg_inst_stage_alu_write },

		{ "FE", vi_evg_inst_stage_tex_fetch },
		{ "DE", vi_evg_inst_stage_tex_decode },
		{ "RD", vi_evg_inst_stage_tex_read },
		{ "WR", vi_evg_inst_stage_tex_write }
	}
};


struct vi_evg_inst_t *vi_evg_inst_create(char *name, long long id,
	int compute_unit_id, int work_group_id, int wavefront_id,
	enum vi_evg_inst_cat_t cat, enum vi_evg_inst_stage_t stage,
	char *asm_code, char *asm_code_x, char *asm_code_y,
	char *asm_code_z, char *asm_code_w, char *asm_code_t)
{
	struct vi_evg_inst_t *inst;

	/* Initialize */
	inst = xcalloc(1, sizeof(struct vi_evg_inst_t));
	inst->name = str_set(NULL, name);
	inst->id = id;
	inst->compute_unit_id = compute_unit_id;
	inst->work_group_id = work_group_id;
	inst->wavefront_id = wavefront_id;
	inst->cat = cat;
	inst->stage = stage;

	inst->asm_code = str_set(NULL, asm_code);
	inst->asm_code_x = str_set(NULL, asm_code_x);
	inst->asm_code_y = str_set(NULL, asm_code_y);
	inst->asm_code_z = str_set(NULL, asm_code_z);
	inst->asm_code_w = str_set(NULL, asm_code_w);
	inst->asm_code_t = str_set(NULL, asm_code_t);

	/* Return */
	return inst;
}


void vi_evg_inst_free(struct vi_evg_inst_t *inst)
{
	str_free(inst->name);
	str_free(inst->asm_code);
	str_free(inst->asm_code_x);
	str_free(inst->asm_code_y);
	str_free(inst->asm_code_z);
	str_free(inst->asm_code_w);
	str_free(inst->asm_code_t);
	free(inst);
}


void vi_evg_inst_get_markup(struct vi_evg_inst_t *inst, char *buf, int size)
{
	char *begin_color;
	char *end_color;

	/* Color */
	end_color = "</span>";
	switch (inst->cat)
	{

	case vi_evg_inst_cat_cf:
		begin_color = "<span color=\"darkgreen\">";
		break;

	case vi_evg_inst_cat_alu:
		begin_color = "<span color=\"red\">";
		break;

	case vi_evg_inst_cat_tex:
		begin_color = "<span color=\"blue\">";
		break;

	default:
		panic("%s: invalid category", __FUNCTION__);
	}

	/* Instruction ID */
	str_printf(&buf, &size, "%s<b>I-%lld</b>%s",
		begin_color, inst->id, end_color);

	/* Assembly */
	if (inst->asm_code && *inst->asm_code)
		str_printf(&buf, &size, " %s", inst->asm_code);

	/* VLIW slots */
	if (inst->asm_code_x && *inst->asm_code_x)
		str_printf(&buf, &size, " <b>x:</b> %s", inst->asm_code_x);
	if (inst->asm_code_y && *inst->asm_code_y)
		str_printf(&buf, &size, " <b>y:</b> %s", inst->asm_code_y);
	if (inst->asm_code_z && *inst->asm_code_z)
		str_printf(&buf, &size, " <b>z:</b> %s", inst->asm_code_z);
	if (inst->asm_code_w && *inst->asm_code_w)
		str_printf(&buf, &size, " <b>w:</b> %s", inst->asm_code_w);
	if (inst->asm_code_y && *inst->asm_code_t)
		str_printf(&buf, &size, " <b>t:</b> %s", inst->asm_code_t);
}


void vi_evg_inst_read_checkpoint(struct vi_evg_inst_t *inst, FILE *f)
{
	int count;

	char name[MAX_STRING_SIZE];
	char asm_code[MAX_STRING_SIZE];
	char asm_code_x[MAX_STRING_SIZE];
	char asm_code_y[MAX_STRING_SIZE];
	char asm_code_z[MAX_STRING_SIZE];
	char asm_code_w[MAX_STRING_SIZE];
	char asm_code_t[MAX_STRING_SIZE];

	/* Read instruction id */
	count = fread(&inst->id, 1, sizeof inst->id, f);
	if (count != sizeof inst->id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read category */
	count = fread(&inst->cat, 1, sizeof inst->cat, f);
	if (count != sizeof inst->cat)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read stage */
	count = fread(&inst->stage, 1, sizeof inst->stage, f);
	if (count != sizeof inst->stage)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read assembly code */
	str_read_from_file(f, asm_code, sizeof asm_code);
	str_read_from_file(f, asm_code_x, sizeof asm_code_x);
	str_read_from_file(f, asm_code_y, sizeof asm_code_y);
	str_read_from_file(f, asm_code_z, sizeof asm_code_z);
	str_read_from_file(f, asm_code_w, sizeof asm_code_w);
	str_read_from_file(f, asm_code_t, sizeof asm_code_t);
	inst->asm_code = str_set(inst->asm_code, asm_code);
	inst->asm_code_x = str_set(inst->asm_code_x, asm_code_x);
	inst->asm_code_y = str_set(inst->asm_code_y, asm_code_y);
	inst->asm_code_z = str_set(inst->asm_code_z, asm_code_z);
	inst->asm_code_w = str_set(inst->asm_code_w, asm_code_w);
	inst->asm_code_t = str_set(inst->asm_code_t, asm_code_t);

	/* Instruction name */
	snprintf(name, sizeof name, "i-%lld", inst->id);
	inst->name = str_set(inst->name, name);
}


void vi_evg_inst_write_checkpoint(struct vi_evg_inst_t *inst, FILE *f)
{
	int count;

	/* Write instruction id */
	count = fwrite(&inst->id, 1, sizeof inst->id, f);
	if (count != sizeof inst->id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write category */
	count = fwrite(&inst->cat, 1, sizeof inst->cat, f);
	if (count != sizeof inst->cat)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write stage */
	count = fwrite(&inst->stage, 1, sizeof inst->stage, f);
	if (count != sizeof inst->stage)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write assembly code */
	str_write_to_file(f, inst->asm_code);
	str_write_to_file(f, inst->asm_code_x);
	str_write_to_file(f, inst->asm_code_y);
	str_write_to_file(f, inst->asm_code_z);
	str_write_to_file(f, inst->asm_code_w);
	str_write_to_file(f, inst->asm_code_t);
}
