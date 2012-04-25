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


/*
 * Public Functions
 */

struct vi_x86_core_t *vi_x86_core_create(char *name)
{
	struct vi_x86_core_t *core;

	/* Allocate */
	core = calloc(1, sizeof(struct vi_x86_core_t));
	if (!core)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	core->name = str_set(NULL, name);
	core->context_table = hash_table_create(0, FALSE);

	/* Return */
	return core;
}


void vi_x86_core_free(struct vi_x86_core_t *core)
{
	/* Free contexts (elements are VI_X86_CONTEXT_EMPTY). */
	hash_table_free(core->context_table);

	/* Free core */
	str_free(core->name);
	free(core);
}


void vi_x86_core_read_checkpoint(struct vi_x86_core_t *core, FILE *f)
{
	int num_contexts;
	int count;
	int i;

	char context_name[MAX_STRING_SIZE];

	/* Clear table of contexts */
	hash_table_clear(core->context_table);

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
}


void vi_x86_core_write_checkpoint(struct vi_x86_core_t *core, FILE *f)
{
	struct vi_x86_context_t *context;

	int num_contexts;
	int count;

	char *context_name;

	/* Number of contexts */
	num_contexts = hash_table_count(core->context_table);
	count = fwrite(&num_contexts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot read checkpoint", __FUNCTION__);

	/* Contexts */
	HASH_TABLE_FOR_EACH(core->context_table, context_name, context)
		str_write_to_file(f, context_name);
}
