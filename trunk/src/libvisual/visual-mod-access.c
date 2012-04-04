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
 * Public Functions
 */

struct visual_mod_access_t *visual_mod_access_create(char *name)
{
	struct visual_mod_access_t *access;

	/* Allocate */
	access = calloc(1, sizeof(struct visual_mod_access_t));
	if (!access)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	access->name = str_set(access->name, name);
	access->creation_cycle = state_file_get_cycle(visual_state_file);

	/* Return */
	return access;
}


void visual_mod_access_free(struct visual_mod_access_t *access)
{
	str_free(access->name);
	str_free(access->state);
	free(access);
}


void visual_mod_access_set_state(struct visual_mod_access_t *access, char *state)
{
	access->state = str_set(access->state, state);
	access->state_update_cycle = state_file_get_cycle(visual_state_file);
}


void visual_mod_access_read_checkpoint(struct visual_mod_access_t *access, FILE *f)
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

	/* Read creation cycle */
	count = fread(&access->creation_cycle, 1, sizeof access->creation_cycle, f);
	if (count != sizeof access->creation_cycle)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read state update cycle */
	count = fread(&access->state_update_cycle, 1, sizeof access->state_update_cycle, f);
	if (count != sizeof access->state_update_cycle)
		panic("%s: cannot read checkpoint", __FUNCTION__);
}


void visual_mod_access_write_checkpoint(struct visual_mod_access_t *access, FILE *f)
{
	int count;

	/* Write name */
	str_write_to_file(f, access->name);

	/* Write state */
	str_write_to_file(f, access->state);

	/* Write creation cycle */
	count = fwrite(&access->creation_cycle, 1, sizeof access->creation_cycle, f);
	if (count != sizeof access->creation_cycle)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write state update cycle */
	count = fwrite(&access->state_update_cycle, 1, sizeof access->state_update_cycle, f);
	if (count != sizeof access->state_update_cycle)
		panic("%s: cannot write checkpoint", __FUNCTION__);
}
