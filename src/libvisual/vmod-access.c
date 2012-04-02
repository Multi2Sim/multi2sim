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

#include <visual-private.h>


/*
 * Module access
 */

struct vmod_access_t
{
	char *name;
	char *state;

	long long state_update_cycle;
};


struct vmod_access_t *vmod_access_create(char *name)
{
	struct vmod_access_t *access;

	/* Allocate */
	access = calloc(1, sizeof(struct vmod_access_t));
	if (!access)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	access->name = str_set(access->name, name);

	/* Return */
	return access;
}


void vmod_access_free(struct vmod_access_t *access)
{
	str_free(access->name);
	str_free(access->state);
	free(access);
}


void vmod_access_get_name_str(struct vmod_access_t *access, char *buf, int size)
{
	/* Name */
	str_printf(&buf, &size, "<b>%s</b>", access->name);

	/* State */
	if (access->state && *access->state)
		str_printf(&buf, &size, " (%s:%lld)", access->state,
			state_file_get_cycle(visual_state_file) - access->state_update_cycle);
}


void vmod_access_get_desc_str(struct vmod_access_t *access, char *buf, int size)
{
	char *title_format_begin = "<span color=\"blue\"><b>";
	char *title_format_end = "</b></span>";

	/* Title */
	str_printf(&buf, &size, "%sDescription for access %s%s\n\n",
		title_format_begin, access->name, title_format_end);

	/* Fields */
	str_printf(&buf, &size, "%sName:%s %s\n", title_format_begin,
		title_format_end, access->name);

	/* State */
	if (access->state && *access->state)
	{
		str_printf(&buf, &size, "%sState:%s %s\n", title_format_begin,
			title_format_end, access->state);
		str_printf(&buf, &size, "%sState update cycle:%s %lld (%lld cycles ago)\n",
			title_format_begin, title_format_end, access->state_update_cycle,
			state_file_get_cycle(visual_state_file) - access->state_update_cycle);
	}
}


char *vmod_access_get_name(struct vmod_access_t *access)
{
	return access->name;
}


void vmod_access_set_state(struct vmod_access_t *access, char *state)
{
	access->state = str_set(access->state, state);
	access->state_update_cycle = state_file_get_cycle(visual_state_file);
}


void vmod_access_read_checkpoint(struct vmod_access_t *access, FILE *f)
{
	char name[MAX_STRING_SIZE];
	char state[MAX_STRING_SIZE];

	int count;

	/* Read name */
	str_read_from_file(f, name, sizeof name);
	access->name = str_set(access->name, name);

	/* Read state */
	str_read_from_file(f, state, sizeof state);
	access->state = str_set(access->state, state);

	/* Read state update cycle */
	count = fread(&access->state_update_cycle, 1, sizeof access->state_update_cycle, f);
	if (count != sizeof access->state_update_cycle)
		panic("%s: cannot read checkpoint", __FUNCTION__);
}


void vmod_access_write_checkpoint(struct vmod_access_t *access, FILE *f)
{
	int count;

	/* Write name */
	str_write_to_file(f, access->name);

	/* Write state */
	str_write_to_file(f, access->state);

	/* Write state update cycle */
	count = fwrite(&access->state_update_cycle, 1, sizeof access->state_update_cycle, f);
	if (count != sizeof access->state_update_cycle)
		panic("%s: cannot write checkpoint", __FUNCTION__);
}
