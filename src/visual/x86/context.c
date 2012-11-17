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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/string.h>
#include <visual/common/cycle-bar.h>
#include <visual/common/state.h>

#include "context.h"
#include "cpu.h"


struct vi_x86_context_t *vi_x86_context_create(char *name, int id, int parent_id)
{
	struct vi_x86_context_t *context;

	/* Initialize */
	context = xcalloc(1, sizeof(struct vi_x86_context_t));
	context->name = str_set(NULL, name);
	context->id = id;
	context->parent_id = parent_id;
	context->creation_cycle = vi_state_get_current_cycle();

	/* Return */
	return context;
}


void vi_x86_context_free(struct vi_x86_context_t *context)
{
	str_free(context->name);
	free(context);
}


void vi_x86_context_get_name_short(char *context_name, char *buf, int size)
{
	snprintf(buf, size, "%s", context_name);
}


void vi_x86_context_get_desc(char *context_name, char *buf, int size)
{
	char *title_begin = "<span color=\"blue\"><b>";
	char *title_end = "</b></span>";

	char parent_id_str[MAX_STRING_SIZE];

	struct vi_x86_context_t *context;

	long long cycle;

	/* Go to current cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Look for context */
	context = hash_table_get(vi_x86_cpu->context_table, context_name);
	if (!context)
		panic("%s: %s: invalid context", __FUNCTION__, context_name);

	/* Title */
	str_printf(&buf, &size, "%sDescription for context %s%s\n\n",
		title_begin, context->name, title_end);

	/* Parent PID */
	if (context->parent_id)
		snprintf(parent_id_str, sizeof parent_id_str, "%d", context->parent_id);
	else
		snprintf(parent_id_str, sizeof parent_id_str, "-");

	/* Fields */
	str_printf(&buf, &size, "<b>Context PID:</b> %d\n",
		context->id);
	str_printf(&buf, &size, "<b>Parent PID:</b> %s\n",
		parent_id_str);
	str_printf(&buf, &size, "<b>Creation cycle:</b> %lld\n",
		context->creation_cycle);

}


void vi_x86_context_read_checkpoint(struct vi_x86_context_t *context, FILE *f)
{
	int count;

	char name[MAX_STRING_SIZE];

	/* Name */
	str_read_from_file(f, name, sizeof name);
	context->name = str_set(context->name, name);

	/* ID */
	count = fread(&context->id, 1, sizeof context->id, f);
	if (count != sizeof context->id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Core ID */
	count = fread(&context->core_id, 1, sizeof context->core_id, f);
	if (count != sizeof context->core_id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Thread ID */
	count = fread(&context->thread_id, 1, sizeof context->thread_id, f);
	if (count != sizeof context->thread_id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Parent PID */
	count = fread(&context->parent_id, 1, sizeof context->parent_id, f);
	if (count != sizeof context->parent_id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Creation cycle */
	count = fread(&context->creation_cycle, 1, sizeof context->creation_cycle, f);
	if (count != sizeof context->creation_cycle)
		panic("%s: cannot read checkpoint", __FUNCTION__);
}


void vi_x86_context_write_checkpoint(struct vi_x86_context_t *context, FILE *f)
{
	int count;

	/* Name */
	assert(context->name);
	str_write_to_file(f, context->name);

	/* ID */
	count = fwrite(&context->id, 1, sizeof context->id, f);
	if (count != sizeof context->id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Core ID */
	count = fwrite(&context->core_id, 1, sizeof context->core_id, f);
	if (count != sizeof context->core_id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Thread ID */
	count = fwrite(&context->thread_id, 1, sizeof context->thread_id, f);
	if (count != sizeof context->thread_id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Parent ID */
	count = fwrite(&context->parent_id, 1, sizeof context->parent_id, f);
	if (count != sizeof context->parent_id)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Creation cycle */
	count = fwrite(&context->creation_cycle, 1, sizeof context->creation_cycle, f);
	if (count != sizeof context->creation_cycle)
		panic("%s: cannot write checkpoint", __FUNCTION__);
}
