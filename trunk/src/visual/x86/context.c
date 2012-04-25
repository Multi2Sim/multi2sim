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


struct vi_x86_context_t *vi_x86_context_create(char *name, int id)
{
	struct vi_x86_context_t *context;

	/* Allocate */
	context = calloc(1, sizeof(struct vi_x86_context_t));
	if (!context)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	context->name = str_set(NULL, name);
	context->id = id;

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
	snprintf(buf, size, "%s", context_name);
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
	if (count != sizeof context->id)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* ID */
	count = fread(&context->thread_id, 1, sizeof context->thread_id, f);
	if (count != sizeof context->id)
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
}
