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

	/* Return */
	return core;
}


void vi_x86_core_free(struct vi_x86_core_t *core)
{
	str_free(core->name);
	free(core);
}
