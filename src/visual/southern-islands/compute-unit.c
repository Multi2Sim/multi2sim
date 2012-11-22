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

#include "compute-unit.h"
#include "inst.h"
#include "work-group.h"


struct vi_si_compute_unit_t *vi_si_compute_unit_create(char *name)
{
	struct vi_si_compute_unit_t *compute_unit;

	/* Initialize */
	compute_unit = xcalloc(1, sizeof(struct vi_si_compute_unit_t));
	compute_unit->name = xstrdup(name);
	compute_unit->work_group_table = hash_table_create(0, FALSE);
	compute_unit->inst_table = hash_table_create(0, FALSE);

	/* Return */
	return compute_unit;
}


void vi_si_compute_unit_free(struct vi_si_compute_unit_t *compute_unit)
{
	struct vi_si_work_group_t *work_group;
	struct vi_si_inst_t *inst;

	char *work_group_name;
	char *inst_name;

	/* Free work-groups */
	HASH_TABLE_FOR_EACH(compute_unit->work_group_table, work_group_name, work_group)
		vi_si_work_group_free(work_group);
	hash_table_free(compute_unit->work_group_table);

	/* Free instructions */
	HASH_TABLE_FOR_EACH(compute_unit->inst_table, inst_name, inst)
		vi_si_inst_free(inst);
	hash_table_free(compute_unit->inst_table);

	/* Free compute unit */
	free(compute_unit->name);
	free(compute_unit);
}


void vi_si_compute_unit_read_checkpoint(struct vi_si_compute_unit_t *compute_unit, FILE *f)
{
	char *work_group_name;
	char *inst_name;

	struct vi_si_work_group_t *work_group;
	struct vi_si_inst_t *inst;

	int num_work_groups;
	int num_insts;

	int count;
	int i;

	/* Empty work-group list */
	HASH_TABLE_FOR_EACH(compute_unit->work_group_table, work_group_name, work_group)
		vi_si_work_group_free(work_group);
	hash_table_clear(compute_unit->work_group_table);

	/* Empty instruction list */
	HASH_TABLE_FOR_EACH(compute_unit->inst_table, inst_name, inst)
		vi_si_inst_free(inst);
	hash_table_clear(compute_unit->inst_table);

	/* Read number of work-groups */
	count = fread(&num_work_groups, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot read checkpoint", __FUNCTION__);

	/* Read work-groups */
	for (i = 0; i < num_work_groups; i++)
	{
		work_group = vi_si_work_group_create(NULL, 0, 0, 0, 0, 0);
		vi_si_work_group_read_checkpoint(work_group, f);
		if (!hash_table_insert(compute_unit->work_group_table, work_group->name, work_group))
			panic("%s: invalid work-group in checkpoint", __FUNCTION__);
	}

	/* Read number of instructions */
	count = fread(&num_insts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot read checkpoint", __FUNCTION__);

	/* Read instructions */
	for (i = 0; i < num_insts; i++)
	{
		inst = vi_si_inst_create(NULL, 0, 0, 0, 0, 0, 0, 0, NULL);
		vi_si_inst_read_checkpoint(inst, f);
		if (!hash_table_insert(compute_unit->inst_table, inst->name, inst))
			panic("%s: invalid instruction in checkpoint", __FUNCTION__);
	}
}


void vi_si_compute_unit_write_checkpoint(struct vi_si_compute_unit_t *compute_unit, FILE *f)
{
	int num_work_groups;
	int num_insts;

	int count;

	char *work_group_name;
	char *inst_name;

	struct vi_si_work_group_t *work_group;
	struct vi_si_inst_t *inst;

	/* Write number of work-groups */
	num_work_groups = hash_table_count(compute_unit->work_group_table);
	count = fwrite(&num_work_groups, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write checkpoint", __FUNCTION__);

	/* Write work-groups */
	HASH_TABLE_FOR_EACH(compute_unit->work_group_table, work_group_name, work_group)
		vi_si_work_group_write_checkpoint(work_group, f);

	/* Write number of instructions */
	num_insts = hash_table_count(compute_unit->inst_table);
	count = fwrite(&num_insts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write checkpoint", __FUNCTION__);

	/* Write instructions */
	HASH_TABLE_FOR_EACH(compute_unit->inst_table, inst_name, inst)
		vi_si_inst_write_checkpoint(inst, f);
}
