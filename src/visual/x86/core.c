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

#include <gtk/gtk.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/string.h>

#include "context.h"
#include "core.h"
#include "inst.h"


/*
 * Public Functions
 */

struct vi_x86_core_t *vi_x86_core_create(char *name)
{
	struct vi_x86_core_t *core;

	/* Initialize */
	core = xcalloc(1, sizeof(struct vi_x86_core_t));
	core->name = str_set(NULL, name);
	core->context_table = hash_table_create(0, FALSE);
	core->inst_table = hash_table_create(0, FALSE);

	/* Return */
	return core;
}


void vi_x86_core_free(struct vi_x86_core_t *core)
{
	struct vi_x86_inst_t *inst;

	char *inst_name;

	/* Free contexts (elements are VI_X86_CONTEXT_EMPTY). */
	hash_table_free(core->context_table);

	/* Free instructions */
	HASH_TABLE_FOR_EACH(core->inst_table, inst_name, inst)
		vi_x86_inst_free(inst);
	hash_table_free(core->inst_table);

	/* Free core */
	str_free(core->name);
	free(core);
}


void vi_x86_core_read_checkpoint(struct vi_x86_core_t *core, FILE *f)
{
	struct vi_x86_inst_t *inst;

	int num_contexts;
	int num_insts;
	int count;
	int i;

	char context_name[MAX_STRING_SIZE];

	char *inst_name;

	/* Clear table of contexts.
	 * Elements are VI_X86_CONTEXT_EMPTY, no need to free. */
	hash_table_clear(core->context_table);

	/* Create table of instructions */
	HASH_TABLE_FOR_EACH(core->inst_table, inst_name, inst)
		vi_x86_inst_free(inst);
	hash_table_clear(core->inst_table);

	/* Number of contexts */
	count = fread(&num_contexts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot read checkpoint", __FUNCTION__);

	/* Read contexts */
	for (i = 0; i < num_contexts; i++)
	{
		str_read_from_file(f, context_name, sizeof context_name);
		if (!hash_table_insert(core->context_table, context_name, VI_X86_CONTEXT_EMPTY))
			panic("%s: invalid context", __FUNCTION__);
	}

	/* Number of instructions */
	count = fread(&num_insts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot read checkpoint", __FUNCTION__);

	/* Read instructions */
	for (i = 0; i < num_insts; i++)
	{
		inst = vi_x86_inst_create(0, NULL, NULL, NULL, 0, 0);
		vi_x86_inst_read_checkpoint(inst, f);
		if (!hash_table_insert(core->inst_table, inst->name, inst))
			panic("%s: invalid instruction", __FUNCTION__);
	}
}


void vi_x86_core_write_checkpoint(struct vi_x86_core_t *core, FILE *f)
{
	struct vi_x86_context_t *context;
	struct vi_x86_inst_t *inst;

	int num_contexts;
	int num_insts;
	int count;

	char *context_name;
	char *inst_name;

	/* Number of contexts */
	num_contexts = hash_table_count(core->context_table);
	count = fwrite(&num_contexts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write checkpoint", __FUNCTION__);

	/* Contexts */
	HASH_TABLE_FOR_EACH(core->context_table, context_name, context)
		str_write_to_file(f, context_name);

	/* Number of instructions */
	num_insts = hash_table_count(core->inst_table);
	count = fwrite(&num_insts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write checkpoint", __FUNCTION__);

	/* Instructions */
	HASH_TABLE_FOR_EACH(core->inst_table, inst_name, inst)
		vi_x86_inst_write_checkpoint(inst, f);
}
