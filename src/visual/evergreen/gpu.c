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


/*
 * Trace Line Processing Functions
 */


static void vi_evg_gpu_new_ndrange(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
}


static void vi_evg_gpu_new_work_group(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
}


static void vi_evg_gpu_new_wavefront(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
}


static void vi_evg_gpu_map_work_group(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_evg_work_group_t *work_group;
	struct vi_evg_compute_unit_t *compute_unit;

	char *work_group_name;

	int compute_unit_id;

	/* Create work-group */
	work_group_name = vi_trace_line_get_symbol(trace_line, "wg");
	work_group = vi_evg_work_group_create(work_group_name);

	/* Initialize */
	work_group->work_item_id_first = vi_trace_line_get_symbol_int(trace_line, "wi_first");
	work_group->work_item_count = vi_trace_line_get_symbol_int(trace_line, "wi_count");
	work_group->wavefront_id_first = vi_trace_line_get_symbol_int(trace_line, "wf_first");
	work_group->wavefront_count = vi_trace_line_get_symbol_int(trace_line, "wf_count");

	/* Compute unit */
	compute_unit_id = vi_trace_line_get_symbol_int(trace_line, "cu");
	compute_unit = list_get(vi_evg_gpu->compute_unit_list, compute_unit_id);
	if (!compute_unit)
		panic("%s: invalid compute unit id", __FUNCTION__);

	/* Insert work-group */
	if (!hash_table_insert(compute_unit->work_group_table, work_group_name, work_group))
		panic("%s: invalid work-group", __FUNCTION__);
}


static void vi_evg_gpu_unmap_work_group(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_evg_compute_unit_t *compute_unit;
	struct vi_evg_work_group_t *work_group;

	int compute_unit_id;

	char *work_group_name;

	/* Fields */
	compute_unit_id = vi_trace_line_get_symbol_int(trace_line, "cu");
	work_group_name = vi_trace_line_get_symbol(trace_line, "wg");

	/* Compute unit */
	compute_unit = list_get(vi_evg_gpu->compute_unit_list, compute_unit_id);
	if (!compute_unit)
		panic("%s: invalid compute unit id", __FUNCTION__);

	/* Unmap work-group */
	work_group = hash_table_remove(compute_unit->work_group_table, work_group_name);
	if (!work_group)
		panic("%s: invalid work-group", __FUNCTION__);
	vi_evg_work_group_free(work_group);
}


static void vi_evg_gpu_read_checkpoint(struct vi_evg_gpu_t *gpu, FILE *f)
{
	int compute_unit_id;

	struct vi_evg_compute_unit_t *compute_unit;

	/* Compute units */
	LIST_FOR_EACH(gpu->compute_unit_list, compute_unit_id)
	{
		compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);
		vi_evg_compute_unit_read_checkpoint(compute_unit, f);
	}
}


static void vi_evg_gpu_write_checkpoint(struct vi_evg_gpu_t *gpu, FILE *f)
{
	int compute_unit_id;

	struct vi_evg_compute_unit_t *compute_unit;

	/* Compute units */
	LIST_FOR_EACH(gpu->compute_unit_list, compute_unit_id)
	{
		compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);
		vi_evg_compute_unit_write_checkpoint(compute_unit, f);
	}
}




/*
 * Public Functions
 */


struct vi_evg_gpu_t *vi_evg_gpu;


void vi_evg_gpu_init(void)
{
	struct vi_trace_line_t *trace_line;

	/* Allocate */
	vi_evg_gpu = calloc(1, sizeof(struct vi_evg_gpu_t));
	if (!vi_evg_gpu)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	vi_evg_gpu->work_group_table = hash_table_create(0, FALSE);
	vi_evg_gpu->compute_unit_list = list_create();

	/* State file */
	vi_state_new_category("Evergreen GPU",
		(vi_state_read_checkpoint_func_t) vi_evg_gpu_read_checkpoint,
		(vi_state_write_checkpoint_func_t) vi_evg_gpu_write_checkpoint,
		vi_evg_gpu);

	/* Commands */
	vi_state_new_command("evg.new_ndrange",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_new_ndrange,
		vi_evg_gpu);
	vi_state_new_command("evg.new_wg",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_new_work_group,
		vi_evg_gpu);
	vi_state_new_command("evg.new_wf",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_new_wavefront,
		vi_evg_gpu);
	vi_state_new_command("evg.map_wg",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_map_work_group,
		vi_evg_gpu);
	vi_state_new_command("evg.unmap_wg",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_unmap_work_group,
		vi_evg_gpu);

	/* Parse header in state file */
	VI_STATE_FOR_EACH_HEADER(trace_line)
	{
		char *command;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);
		assert(strcmp(command, "c"));

		if (!strcmp(command, "evg.init"))
		{
			struct vi_evg_compute_unit_t *compute_unit;

			int num_compute_units;
			int i;

			char name[MAX_STRING_SIZE];

			/* Create module */
			num_compute_units = vi_trace_line_get_symbol_int(trace_line, "num_compute_units");
			for (i = 0; i < num_compute_units; i++)
			{
				snprintf(name, sizeof name, "CU-%d", i);
				compute_unit = vi_evg_compute_unit_create(name);
				list_add(vi_evg_gpu->compute_unit_list, compute_unit);
			}
		}
	}
}


void vi_evg_gpu_done(void)
{
	struct vi_evg_work_group_t *work_group;

	char *work_group_name;

	int i;

	/* Free work-groups */
	HASH_TABLE_FOR_EACH(vi_evg_gpu->work_group_table, work_group_name, work_group)
		vi_evg_work_group_free(work_group);
	hash_table_free(vi_evg_gpu->work_group_table);

	/* Free compute units */
	LIST_FOR_EACH(vi_evg_gpu->compute_unit_list, i)
		vi_evg_compute_unit_free(list_get(vi_evg_gpu->compute_unit_list, i));
	list_free(vi_evg_gpu->compute_unit_list);

	/* Free GPU */
	free(vi_evg_gpu);
}
