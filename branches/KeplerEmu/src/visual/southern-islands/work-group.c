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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <visual/common/cycle-bar.h>
#include <visual/common/state.h>

#include "compute-unit.h"
#include "gpu.h"
#include "work-group.h"


struct vi_si_work_group_t *vi_si_work_group_create(char *name, int id,
	int work_item_id_first, int work_item_count, int wavefront_id_first, int wavefront_count)
{
	struct vi_si_work_group_t *work_group;

	/* Initialize */
	work_group = xcalloc(1, sizeof(struct vi_si_work_group_t));
	work_group->name = str_set(NULL, name);
	work_group->id = id;
	work_group->work_item_id_first = work_item_id_first;
	work_group->work_item_count = work_item_count;
	work_group->wavefront_id_first = wavefront_id_first;
	work_group->wavefront_count = wavefront_count;

	/* Return */
	return work_group;
}


void vi_si_work_group_free(struct vi_si_work_group_t *work_group)
{
	str_free(work_group->name);
	free(work_group);
}


void vi_si_work_group_get_name_short(char *work_group_name, char *buf, int size)
{
	snprintf(buf, size, "%s", work_group_name);
}


void vi_si_work_group_get_desc(char *work_group_name, char *buf, int size)
{
	char *title_begin = "<span color=\"blue\"><b>";
	char *title_end = "</b></span>";

	long long cycle;

	struct vi_si_compute_unit_t *compute_unit;
	struct vi_si_work_group_t *work_group;

	int compute_unit_id;

	/* Go to current cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Look for work-group */
	LIST_FOR_EACH(vi_si_gpu->compute_unit_list, compute_unit_id)
	{
		compute_unit = list_get(vi_si_gpu->compute_unit_list, compute_unit_id);
		work_group = hash_table_get(compute_unit->work_group_table, work_group_name);
		if (work_group)
			break;
	}
	if (!work_group)
		panic("%s: %s: invalid work-group", __FUNCTION__, work_group_name);

	/* Title */
	str_printf(&buf, &size, "%sDescription for work-group %s%s\n\n",
		title_begin, work_group->name, title_end);

	/* Work-items */
	str_printf(&buf, &size, "<b>Number of work-items:</b> %d\n",
		work_group->work_item_count);
	str_printf(&buf, &size, "<b>Work-items:</b> [wi-%d...wi-%d]\n",
		work_group->work_item_id_first,
		work_group->work_item_id_first + work_group->work_item_count - 1);
	str_printf(&buf, &size, "\n");

	/* Wavefronts */
	str_printf(&buf, &size, "<b>Number of wavefronts:</b> %d\n",
		work_group->wavefront_count);
	str_printf(&buf, &size, "<b>Wavefronts:</b> [wf-%d...wf-%d]\n",
		work_group->wavefront_id_first,
		work_group->wavefront_id_first + work_group->wavefront_count - 1);
}


void vi_si_work_group_read_checkpoint(struct vi_si_work_group_t *work_group, FILE *f)
{
	int count;

	char name[MAX_STRING_SIZE];

	/* Read work-group id, WF first/count, WI first/count */
	count = 0;
	count += fread(&work_group->id, 1, sizeof(int), f);
	count += fread(&work_group->wavefront_id_first, 1, sizeof(int), f);
	count += fread(&work_group->wavefront_count, 1, sizeof(int), f);
	count += fread(&work_group->work_item_id_first, 1, sizeof(int), f);
	count += fread(&work_group->work_item_count, 1, sizeof(int), f);
	if (count != 20)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Work-group name */
	snprintf(name, sizeof name, "wg-%d", work_group->id);
	work_group->name = str_set(work_group->name, name);
}


void vi_si_work_group_write_checkpoint(struct vi_si_work_group_t *work_group, FILE *f)
{
	int count;

	/* Write work-group id, WF first/count, WI first/count */
	count = 0;
	count += fwrite(&work_group->id, 1, sizeof(int), f);
	count += fwrite(&work_group->wavefront_id_first, 1, sizeof(int), f);
	count += fwrite(&work_group->wavefront_count, 1, sizeof(int), f);
	count += fwrite(&work_group->work_item_id_first, 1, sizeof(int), f);
	count += fwrite(&work_group->work_item_count, 1, sizeof(int), f);
	if (count != 20)
		panic("%s: cannot write checkpoint", __FUNCTION__);
}
