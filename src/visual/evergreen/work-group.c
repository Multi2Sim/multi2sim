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


struct vi_evg_work_group_t *vi_evg_work_group_create(char *name)
{
	struct vi_evg_work_group_t *work_group;

	/* Allocate */
	work_group = calloc(1, sizeof(struct vi_evg_work_group_t));
	if (!work_group)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	work_group->name = str_set(NULL, name);

	/* Return */
	return work_group;
}


void vi_evg_work_group_free(struct vi_evg_work_group_t *work_group)
{
	str_free(work_group->name);
	free(work_group);
}
