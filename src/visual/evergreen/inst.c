/*
 *  Multi2Sim Tools
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <visual-evergreen.h>


struct string_map_t vi_evg_inst_cat_map =
{
	3, {
		{ "cf", vi_evg_inst_cat_cf },
		{ "alu", vi_evg_inst_cat_alu },
		{ "tex", vi_evg_inst_cat_tex }
	}
};


struct vi_evg_inst_t *vi_evg_inst_create(char *name, long long id,
	int compute_unit_id, int work_group_id, int wavefront_id,
	enum vi_evg_inst_cat_t cat, char *asm_code, char *asm_code_x,
	char *asm_code_y, char *asm_code_z, char *asm_code_w,
	char *asm_code_t)
{
	struct vi_evg_inst_t *inst;

	/* Allocate */
	inst = calloc(1, sizeof(struct vi_evg_inst_t));
	if (!inst)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	inst->name = str_set(NULL, name);
	inst->id = id;
	inst->compute_unit_id = compute_unit_id;
	inst->work_group_id = work_group_id;
	inst->wavefront_id = wavefront_id;
	inst->cat = cat;

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

	/* Write instruction id */
	count = fwrite(&inst->cat, 1, sizeof inst->cat, f);
	if (count != sizeof inst->cat)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write assembly code */
	str_write_to_file(f, inst->asm_code);
	str_write_to_file(f, inst->asm_code_x);
	str_write_to_file(f, inst->asm_code_y);
	str_write_to_file(f, inst->asm_code_z);
	str_write_to_file(f, inst->asm_code_w);
	str_write_to_file(f, inst->asm_code_t);
}
