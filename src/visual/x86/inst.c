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


struct str_map_t vi_x86_inst_stage_map =
{
	vi_x86_inst_stage_count - 1,
	{
		{ "fe", vi_x86_inst_stage_fetch },
		{ "dec", vi_x86_inst_stage_decode },
		{ "di", vi_x86_inst_stage_dispatch },
		{ "i", vi_x86_inst_stage_issue },
		{ "wb", vi_x86_inst_stage_writeback },
		{ "co", vi_x86_inst_stage_commit },
		{ "sq", vi_x86_inst_stage_squash }
	}
};


struct str_map_t vi_x86_inst_stage_name_map =
{
	vi_x86_inst_stage_count - 1,
	{
		{ "FE", vi_x86_inst_stage_fetch },
		{ "DEC", vi_x86_inst_stage_decode },
		{ "DI", vi_x86_inst_stage_dispatch },
		{ "I", vi_x86_inst_stage_issue },
		{ "WB", vi_x86_inst_stage_writeback },
		{ "CO", vi_x86_inst_stage_commit },
		{ "Squash", vi_x86_inst_stage_squash }
	}
};


struct str_map_t vi_x86_inst_stage_color_map =
{
	vi_x86_inst_stage_count - 1,
	{
		{ "#CCFFFF", vi_x86_inst_stage_fetch },
		{ "#CCFFCC", vi_x86_inst_stage_decode },
		{ "#CCFF99", vi_x86_inst_stage_dispatch },
		{ "#CCFF66", vi_x86_inst_stage_issue },
		{ "#CCCC66", vi_x86_inst_stage_writeback },
		{ "#CC9933", vi_x86_inst_stage_commit },
		{ "#FF0000", vi_x86_inst_stage_squash }
	}
};


struct vi_x86_inst_t *vi_x86_inst_create(long long id, char *name,
	char *asm_code, char *asm_micro_code, int spec_mode,
	enum vi_x86_inst_stage_t stage)
{
	struct vi_x86_inst_t *inst;

	/* Initialize */
	inst = xcalloc(1, sizeof(struct vi_x86_inst_t));
	inst->id = id;
	inst->name = str_set(NULL, name);
	inst->asm_code = str_set(NULL, asm_code);
	inst->asm_micro_code = str_set(NULL, asm_micro_code);
	inst->spec_mode = spec_mode;
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

	/* Speculative mode */
	count = fread(&inst->spec_mode, 1, sizeof inst->spec_mode, f);
	if (count != sizeof inst->spec_mode)
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

	/* Speculative mode */
	count = fwrite(&inst->spec_mode, 1, sizeof inst->spec_mode, f);
	if (count != sizeof inst->spec_mode)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Stage */
	count = fwrite(&inst->stage, 1, sizeof inst->stage, f);
	if (count != sizeof inst->stage)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Assembly code */
	str_write_to_file(f, inst->asm_code);
	str_write_to_file(f, inst->asm_micro_code);
}
