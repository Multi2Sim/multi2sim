/*
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

#include <sdram.h>
#include <stdlib.h>
#include <debug.h>
#include <string.h>


struct mem_ctrl_t *mem_ctrl_create(char *name)
{
	struct mem_ctrl_t *mem_ctrl;

	/* Create memory controller */
	mem_ctrl = calloc(1, sizeof(struct mem_ctrl_t));
	if (!mem_ctrl)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	mem_ctrl->name = strdup(name);
	if (!mem_ctrl->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Return */
	return mem_ctrl;
}


void mem_ctrl_free(struct mem_ctrl_t *mem_ctrl)
{
	free(mem_ctrl->name);
	free(mem_ctrl);
}
