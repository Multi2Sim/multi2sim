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


struct vi_evg_inst_t *vi_evg_inst_create(char *name, long long id,
	int compute_unit_id, int work_group_id, int wavefront_id, char *asm_code)
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
	inst->asm_code = str_set(NULL, asm_code);

	/* Return */
	return inst;
}


void vi_evg_inst_free(struct vi_evg_inst_t *inst)
{
	str_free(inst->name);
	str_free(inst->asm_code);
	free(inst);
}


void vi_evg_inst_read_checkpoint(struct vi_evg_inst_t *inst, FILE *f)
{
	int count;

	char name[MAX_STRING_SIZE];
	char asm_code[MAX_STRING_SIZE];

	/* Read instruction id */
	count = fread(&inst->id, 1, sizeof inst->id, f);
	if (count != sizeof inst->id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read assembly code */
	str_read_from_file(f, asm_code, sizeof asm_code);
	inst->asm_code = str_set(inst->asm_code, asm_code);

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

	/* Write assembly code */
	str_write_to_file(f, inst->asm_code);
}
