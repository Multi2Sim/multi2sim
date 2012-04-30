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

#include <visual-x86.h>


struct string_map_t vi_x86_inst_stage_map =
{
	vi_x86_inst_stage_count - 1,
	{
		{ "fe", vi_x86_inst_stage_fetch },
		{ "de", vi_x86_inst_stage_decode },
		{ "di", vi_x86_inst_stage_dispatch },
		{ "ex", vi_x86_inst_stage_execute },
		{ "wr", vi_x86_inst_stage_writeback },
		{ "co", vi_x86_inst_stage_commit },
		{ "sq", vi_x86_inst_stage_squash }
	}
};


struct string_map_t vi_x86_inst_stage_name_map =
{
	vi_x86_inst_stage_count - 1,
	{
		{ "FE", vi_x86_inst_stage_fetch },
		{ "DEC", vi_x86_inst_stage_decode },
		{ "DI", vi_x86_inst_stage_dispatch },
		{ "EX", vi_x86_inst_stage_execute },
		{ "WB", vi_x86_inst_stage_writeback },
		{ "CO", vi_x86_inst_stage_commit },
		{ "x", vi_x86_inst_stage_squash }
	}
};


struct string_map_t vi_x86_inst_stage_color_map =
{
	vi_x86_inst_stage_count - 1,
	{
		{ "#4444ff", vi_x86_inst_stage_fetch },
		{ "#8888ff", vi_x86_inst_stage_decode },
		{ "#cccc88", vi_x86_inst_stage_dispatch },
		{ "#cccc00", vi_x86_inst_stage_execute },
		{ "#ccff44", vi_x86_inst_stage_writeback },
		{ "#44ff44", vi_x86_inst_stage_commit },
		{ "red", vi_x86_inst_stage_squash }
	}
};


struct vi_x86_inst_t *vi_x86_inst_create(long long id, char *name,
	char *asm_code, char *asm_micro_code,
	enum vi_x86_inst_stage_t stage)
{
	struct vi_x86_inst_t *inst;

	/* Allocate */
	inst = calloc(1, sizeof(struct vi_x86_inst_t));
	if (!inst)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	inst->id = id;
	inst->name = str_set(NULL, name);
	inst->asm_code = str_set(NULL, asm_code);
	inst->asm_micro_code = str_set(NULL, asm_micro_code);
	inst->stage = stage;

	/* Return */
	return inst;
}


void vi_x86_inst_free(struct vi_x86_inst_t *inst)
{
	str_free(inst->name);
	str_free(inst->asm_code);
	str_free(inst->asm_micro_code);
	free(inst);
}


void vi_x86_inst_read_checkpoint(struct vi_x86_inst_t *inst, FILE *f)
{
	int count;

	char name[MAX_STRING_SIZE];
	char asm_code[MAX_STRING_SIZE];
	char asm_micro_code[MAX_STRING_SIZE];

	/* ID */
	count = fread(&inst->id, 1, sizeof inst->id, f);
	if (count != sizeof inst->id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Stage */
	count = fread(&inst->stage, 1, sizeof inst->stage, f);
	if (count != sizeof inst->stage)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Assembly code */
	str_read_from_file(f, asm_code, sizeof asm_code);
	str_read_from_file(f, asm_micro_code, sizeof asm_micro_code);
	inst->asm_code = str_set(inst->asm_code, asm_code);
	inst->asm_micro_code = str_set(inst->asm_micro_code, asm_micro_code);

	/* Instruction name */
	snprintf(name, sizeof name, "i-%lld", inst->id);
	inst->name = str_set(inst->name, name);
}


void vi_x86_inst_write_checkpoint(struct vi_x86_inst_t *inst, FILE *f)
{
	int count;

	/* ID */
	count = fwrite(&inst->id, 1, sizeof inst->id, f);
	if (count != sizeof inst->id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Stage */
	count = fwrite(&inst->stage, 1, sizeof inst->stage, f);
	if (count != sizeof inst->stage)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Assembly code */
	str_write_to_file(f, inst->asm_code);
	str_write_to_file(f, inst->asm_micro_code);
}
