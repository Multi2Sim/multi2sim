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

#include <gpuvisual-private.h>



/* Create gpu structure based on an input trace */
struct vgpu_t *vgpu_create(char *trace_file_name)
{
	struct vgpu_t *gpu;
	int err;

	/* Create it */
	gpu = calloc(1, sizeof(struct vgpu_t));

	/* Open trace file */
	snprintf(gpu->trace_file_name, sizeof gpu->trace_file_name, "%s",
		trace_file_name);
	gpu->trace_file = fopen(trace_file_name, "rt");
	if (!gpu->trace_file) {
		snprintf(vgpu_trace_err, sizeof vgpu_trace_err,
			"%s: cannot open file", trace_file_name);
		free(gpu);
		return NULL;
	}

	/* Open status file */
	snprintf(gpu->state_file_name, sizeof gpu->state_file_name, "%s.status", trace_file_name);
	gpu->state_file = fopen(gpu->state_file_name, "w+b");
	if (!gpu->state_file) {
		snprintf(vgpu_trace_err, sizeof vgpu_trace_err,
			"%s: cannot create state file", gpu->state_file_name);
		fclose(gpu->trace_file);
		free(gpu);
		return NULL;
	}

	/* Create status text */
	gpu->status_text_size = MAX_STRING_SIZE;
	gpu->status_text = calloc(1, MAX_STRING_SIZE);

	/* Create lists */
	gpu->state_checkpoint_list = list_create_with_size(100);
	gpu->kernel_source_strings = list_create();
	gpu->compute_unit_list = list_create();
	gpu->work_group_list = list_create();
	gpu->pending_work_group_list = list_create();
	gpu->finished_work_group_list = list_create();

	/* Parse trace file */
	err = vgpu_trace_parse(gpu);
	if (err) {
		/* FIXME: free stuff created before (including what 'vgpu_trace_parse' created) */
		return NULL;
	}

	/* Return it */
	return gpu;
}


void vgpu_free(struct vgpu_t *gpu)
{
	int i;

	/* Close trace files */
	fclose(gpu->state_file);
	fclose(gpu->trace_file);

	/* State info list */
	for (i = 0; i < list_count(gpu->state_checkpoint_list); i++)
		free(list_get(gpu->state_checkpoint_list, i));
	list_free(gpu->state_checkpoint_list);

	/* Kernel source */
	for (i = 0; i < list_count(gpu->kernel_source_strings); i++)
		free(list_get(gpu->kernel_source_strings, i));
	list_free(gpu->kernel_source_strings);

	/* Work-groups */
	for (i = 0; i < list_count(gpu->work_group_list); i++)
		vgpu_work_group_free(list_get(gpu->work_group_list, i));
	list_free(gpu->work_group_list);
	list_free(gpu->finished_work_group_list);
	list_free(gpu->pending_work_group_list);

	/* Compute units */
	for (i = 0; i < list_count(gpu->compute_unit_list); i++)
		vgpu_compute_unit_free(list_get(gpu->compute_unit_list, i));
	list_free(gpu->compute_unit_list);

	/* Status text */
	free(gpu->status_text);

	/* GPU */
	free(gpu);
}


/* Labels included in the state file */
static char vgpu_state_label = 0x11;
static char vgpu_work_group_list_label = 0x22;
static char vgpu_uop_list_label = 0x33;


/* Store the GPU state in the current position in gpu->state_file */
void vgpu_store_state(struct vgpu_t *gpu)
{
	FILE *f = gpu->state_file;
	int i, j;
	int size;

	/* Cycle */
	fwrite(&vgpu_state_label, 1, 1, f);
	fwrite(&gpu->cycle, 1, sizeof(int), f);
	fwrite(&gpu->trace_line_number, 1, sizeof(int), f);

	/* Status */
	size = strlen(gpu->status_text) + 1;
	fwrite(&size, 1, sizeof(int), f);
	fwrite(gpu->status_text, 1, size, f);

	/* Compute units */
	fwrite(&gpu->num_compute_units, 1, sizeof(int), f);
	fwrite(&gpu->num_mapped_work_groups, 1, sizeof(int), f);
	for (i = 0; i < gpu->num_compute_units; i++)
	{
		struct vgpu_compute_unit_t *compute_unit;
		struct list_t *work_group_list;
		int num_work_groups;
		int num_uops;

		/* Get compute unit and work groups */
		compute_unit = list_get(gpu->compute_unit_list, i);
		work_group_list = compute_unit->work_group_list;

		/* Fields for compute unit */
		fwrite(&vgpu_work_group_list_label, 1, 1, f);
		num_work_groups = list_count(work_group_list);
		fwrite(&compute_unit->last_completed_uop_id, 1, sizeof(int), f);

		/* List of work-groups */
		fwrite(&num_work_groups, 1, sizeof(int), f);
		for (j = 0; j < num_work_groups; j++)
		{
			struct vgpu_work_group_t *work_group = list_get(work_group_list, j);
			fwrite(&work_group->id, 1, sizeof(int), f);
		}

		/* List of uops */
		fwrite(&vgpu_uop_list_label, 1, 1, f);
		num_uops = list_count(compute_unit->uop_list);
		fwrite(&num_uops, 1, sizeof(int), f);
		for (j = 0; j < num_uops; j++) {
			struct vgpu_uop_t *uop;
			int len, k;

			uop = list_get(compute_unit->uop_list, j);
			fwrite(&uop->id, 1, sizeof(int), f);
			fwrite(&uop->compute_unit_id, 1, sizeof(int), f);
			fwrite(&uop->work_group_id, 1, sizeof(int), f);
			fwrite(&uop->wavefront_id, 1, sizeof(int), f);
			fwrite(&uop->engine, 1, sizeof(int), f);
			fwrite(&uop->stage, 1, sizeof(int), f);
			fwrite(&uop->stage_cycle, 1, sizeof(int), f);
			fwrite(&uop->finished, 1, sizeof(int), f);

			/* Name */
			len = strlen(uop->name);
			fwrite(&len, 1, sizeof(int), f);
			fwrite(&uop->name, 1, len + 1, f);

			/* VLIW bundle */
			for (k = 0; k < 5; k++)
			{
				len = strlen(uop->vliw_slot[k]);
				fwrite(&len, 1, sizeof(int), f);
				fwrite(&uop->vliw_slot[k], 1, len + 1, f);

				len = strlen(uop->vliw_slot_args[k]);
				fwrite(&len, 1, sizeof(int), f);
				fwrite(&uop->vliw_slot_args[k], 1, len + 1, f);
			}
		}
	}
}


/* Load the GPU state from the current position in gpu->state_file */
void vgpu_load_state(struct vgpu_t *gpu)
{
	FILE *f = gpu->state_file;
	int num_compute_units;
	int i, j;
	char label;
	int count = 0;
	int size;

	/* Cycle */
	count += fread(&label, 1, 1, f);
	assert(label == vgpu_state_label);
	count += fread(&gpu->cycle, 1, sizeof(int), f);
	count += fread(&gpu->trace_line_number, 1, sizeof(int), f);

	/* Status text */
	count += fread(&size, 1, sizeof(int), f);
	if (size > gpu->status_text_size)
	{
		gpu->status_text_size = size;
		gpu->status_text = realloc(gpu->status_text, size);
		if (!gpu->status_text)
			fatal("%s: out of memory", __FUNCTION__);
	}
	count += fread(gpu->status_text, 1, size, f);

	/* Compute units */
	count += fread(&num_compute_units, 1, sizeof(int), f);
	assert(num_compute_units == gpu->num_compute_units);
	count += fread(&gpu->num_mapped_work_groups, 1, sizeof(int), f);
	for (i = 0; i < gpu->num_compute_units; i++)
	{
		struct vgpu_compute_unit_t *compute_unit;
		struct list_t *work_group_list;
		int num_work_groups;
		int num_uops;

		/* Get compute unit and work groups */
		compute_unit = list_get(gpu->compute_unit_list, i);
		work_group_list = compute_unit->work_group_list;

		/* Fields for compute unit */
		count += fread(&label, 1, 1, f);
		assert(label == vgpu_work_group_list_label);
		count += fread(&compute_unit->last_completed_uop_id, 1, sizeof(int), f);

		/* List of work-groups */
		list_clear(work_group_list);
		count += fread(&num_work_groups, 1, sizeof(int), f);
		for (j = 0; j < num_work_groups; j++)
		{
			struct vgpu_work_group_t *work_group;
			int work_group_id;

			count += fread(&work_group_id, 1, sizeof(int), f);
			work_group = list_get(gpu->work_group_list, work_group_id);
			assert(work_group);
			list_add(work_group_list, work_group);
		}

		/* List of uops */
		count += fread(&label, 1, 1, f);
		assert(label == vgpu_uop_list_label);
		vgpu_uop_list_clear(compute_unit->uop_list);
		count += fread(&num_uops, 1, sizeof(int), f);
		for (j = 0; j < num_uops; j++)
		{
			struct vgpu_uop_t *uop;
			int id, len, k;

			count += fread(&id, 1, sizeof(int), f);
			uop = vgpu_uop_create(id);
			count += fread(&uop->compute_unit_id, 1, sizeof(int), f);
			count += fread(&uop->work_group_id, 1, sizeof(int), f);
			count += fread(&uop->wavefront_id, 1, sizeof(int), f);
			count += fread(&uop->engine, 1, sizeof(int), f);
			count += fread(&uop->stage, 1, sizeof(int), f);
			count += fread(&uop->stage_cycle, 1, sizeof(int), f);
			count += fread(&uop->finished, 1, sizeof(int), f);
			count += fread(&len, 1, sizeof(int), f);
			count += fread(&uop->name, 1, len + 1, f);
			for (k = 0; k < 5; k++)
			{
				count += fread(&len, 1, sizeof(int), f);
				count += fread(&uop->vliw_slot[k], 1, len + 1, f);
				count += fread(&len, 1, sizeof(int), f);
				count += fread(&uop->vliw_slot_args[k], 1, len + 1, f);
			}
			list_add(compute_unit->uop_list, uop);
		}
	}

	/* The two lines below are just an excuse to use variable 'count', which
	 * accumulate the return value of all calls to 'fread'. The whole thing is
	 * simply a way of avoiding annoying compiler warnings. */
	if (!count)
		fatal("%s: read error", __FUNCTION__);

	/* Reconstruct pending list */
	list_clear(gpu->pending_work_group_list);
	for (i = gpu->num_mapped_work_groups; i < gpu->num_work_groups; i++)
		list_add(gpu->pending_work_group_list, list_get(gpu->work_group_list, i));

	/* Reconstruct finished list */
	list_clear(gpu->finished_work_group_list);
	for (i = 0; i < gpu->num_mapped_work_groups; i++)
		list_add(gpu->finished_work_group_list, list_get(gpu->work_group_list, i));
	for (i = 0; i < gpu->num_compute_units; i++)
	{
		struct vgpu_compute_unit_t *compute_unit;
		struct list_t *work_group_list;
		struct vgpu_work_group_t *work_group_curr;

		compute_unit = list_get(gpu->compute_unit_list, i);
		work_group_list = compute_unit->work_group_list;
		for (j = 0; j < list_count(work_group_list); j++)
		{
			work_group_curr = list_get(work_group_list, j);
			list_remove(gpu->finished_work_group_list, work_group_curr);
		}
	}
}


void vgpu_status_write(struct vgpu_t *gpu, char *fmt, ...)
{
	char str[MAX_LONG_STRING_SIZE];
	va_list va;
	int size;

	va_start(va, fmt);
	vsnprintf(str, sizeof str, fmt, va);
	size = strlen(gpu->status_text) + strlen(str) + 1;
	if (size > gpu->status_text_size)
	{
		gpu->status_text_size = MAX(size, gpu->status_text_size * 2);
		gpu->status_text = realloc(gpu->status_text, gpu->status_text_size);
	}
	if (!gpu->status_text)
		fatal("%s: out of memory", __FUNCTION__);
	strcat(gpu->status_text, str);
}


void vgpu_status_clear(struct vgpu_t *gpu)
{
	memset(gpu->status_text, 0, gpu->status_text_size);
}




/*
 * GPU Work-Group
 */


struct vgpu_work_group_t *vgpu_work_group_create(int id)
{
	struct vgpu_work_group_t *work_group;
	work_group = calloc(1, sizeof(struct vgpu_work_group_t));
	work_group->id = id;
	return work_group;
}


void vgpu_work_group_free(struct vgpu_work_group_t *work_group)
{
	free(work_group);
}


void work_group_get_name(void *item, char *buf, int size)
{
	struct vgpu_work_group_t *work_group = item;
	*buf = '\0';
	if (work_group)
		snprintf(buf, size, "WG-%d", work_group->id);
}


void work_group_info_popup(void *item)
{
	char buf[10000];

	char *buf_ptr = buf;
	int buf_size = sizeof buf;

	struct vgpu_work_group_t *work_group = item;
	if (!work_group)
		return;
	
	/* Text */
	str_printf(&buf_ptr, &buf_size, "<b>Work-Group %d</b>\n"
		"\n", work_group->id);
	
	/* Work-items and wavefronts */
	str_printf(&buf_ptr, &buf_size,
		"Name:         <b>WG-%d</b>\n"
		"Work-items:   [ <b>WI-%d</b> ... <b>WI-%d</b> ]  (%d work-items)\n"
		"Wavefronts:   [ <b>WF-%d</b> ... <b>WF-%d</b> ]  (%d wavefronts)\n"
		"\n",
		work_group->id,
		work_group->work_item_id_first, work_group->work_item_id_last, work_group->work_item_count,
		work_group->wavefront_id_first, work_group->wavefront_id_last, work_group->wavefront_count);
	
	/* Show */
	info_popup_show(buf);
}




/*
 * GPU Compute unit
 */

struct vgpu_compute_unit_t *vgpu_compute_unit_create(struct vgpu_t *gpu, int id)
{
	struct vgpu_compute_unit_t *compute_unit;
	compute_unit = calloc(1, sizeof(struct vgpu_compute_unit_t));
	compute_unit->gpu = gpu;
	compute_unit->id = id;
	compute_unit->work_group_list = list_create();
	compute_unit->uop_list = list_create();
	return compute_unit;
};


void vgpu_compute_unit_free(struct vgpu_compute_unit_t *compute_unit)
{
	if (compute_unit->timing_dia_active)
		timing_dia_window_hide(compute_unit);
	if (compute_unit->block_dia_active)
		block_dia_window_hide(compute_unit);
	list_free(compute_unit->work_group_list);
	vgpu_uop_list_free(compute_unit->uop_list);
	free(compute_unit);
}




/*
 * GPU Uop
 */

struct vgpu_uop_t *vgpu_uop_create(int id)
{
	struct vgpu_uop_t *uop;
	uop = calloc(1, sizeof(struct vgpu_uop_t));
	uop->id = id;
	return uop;
}


void vgpu_uop_free(struct vgpu_uop_t *uop)
{
	free(uop);
}


struct vgpu_uop_t *vgpu_uop_dup(struct vgpu_uop_t *uop)
{
	struct vgpu_uop_t *dup;
	dup = calloc(1, sizeof(struct vgpu_uop_t));
	memcpy(dup, uop, sizeof(struct vgpu_uop_t));
	return dup;
}


/* Return value:
 *   0 - OK
 *   1 - Invalid uop id (not sequential)
 */
int vgpu_uop_list_add(struct list_t *list, struct vgpu_uop_t *uop)
{
	struct vgpu_uop_t *uop_tail;

	/* Get uop at the tail of the list */
	if (list_count(list)) {
		uop_tail = list_get(list, list_count(list) - 1);
		if (uop_tail->id != uop->id - 1)
			return 1;
	}

	/* Add it */
	list_add(list, uop);
	return 0;
}


void vgpu_uop_list_clear(struct list_t *list)
{
	struct vgpu_uop_t *uop;
	while (list_count(list)) {
		uop = list_remove_at(list, 0);
		if (uop)
			vgpu_uop_free(uop);
	}
}


void vgpu_uop_list_free(struct list_t *list)
{
	vgpu_uop_list_clear(list);
	list_free(list);
}


struct vgpu_uop_t *vgpu_uop_list_get(struct list_t *list, int uop_id)
{
	struct vgpu_uop_t *uop, *uop_head;
	int index;

	/* Get uop at the head of the list */
	uop_head = list_get(list, 0);
	if (!uop_head)
		return NULL;

	/* Figure out position in list */
	if (uop_id < uop_head->id)
		return NULL;
	index = uop_id - uop_head->id;
	if (index >= list_count(list))
		return NULL;

	/* Return uop */
	uop = list_get(list, index);
	assert(uop->id == uop_id);
	return uop;
}


void vgpu_uop_list_dump(struct list_t *list, FILE *f)
{
	struct vgpu_uop_t *uop;
	int i;

	fprintf(f, "uop_list = {");
	for (i = 0; i < list_count(list); i++) {
		uop = list_get(list, i);
		fprintf(f, " %d", uop->id);
	}
	fprintf(f, " }\n");
}


void vgpu_uop_get_name(void *item, char *buf, int size)
{
	struct vgpu_uop_t *uop = item;
	*buf = '\0';
	if (uop)
		snprintf(buf, size, "I-%d", uop->id);
}


void vgpu_uop_get_markup(struct vgpu_uop_t *uop, char *buf_ptr, int buf_size)
{
	char vliw_elems[5] = { 'x', 'y', 'z', 'w', 't' };
	char *uop_color[3] = { "darkgreen", "red", "blue" };

	int i;

	if (uop->engine == VGPU_ENGINE_ALU)
	{
		str_printf(&buf_ptr, &buf_size, "<span color=\"%s\">",
			uop_color[uop->engine]);
		for (i = 0; i < 5; i++)
		{
			if (uop->vliw_slot[i][0])
				str_printf(&buf_ptr, &buf_size, "  <b>%c:</b> %s",
					vliw_elems[i], uop->vliw_slot[i]);
		}
		str_printf(&buf_ptr, &buf_size, "</span>");
	}
	else
	{
		str_printf(&buf_ptr, &buf_size, "<span color=\"%s\">%s</span>",
			uop_color[uop->engine], uop->name);
	}
}


void vgpu_uop_info_popup(void *item)
{
	char buf[10000];

	char *buf_ptr = buf;
	int buf_size = sizeof buf;

	struct vgpu_uop_t *uop = item;
	int i;

	/* Empty uop */
	if (!uop)
		return;

	/* Intro */
	str_printf(&buf_ptr, &buf_size,
		"<b>Instruction %d</b>\n\n", uop->id);
	str_printf(&buf_ptr, &buf_size,
		"Assembly:\n\n");

	/* Instruction name */
	if (uop->engine == VGPU_ENGINE_CF)
	{
		str_printf(&buf_ptr, &buf_size,
			"   <span color=\"darkgreen\"><b>%s</b></span>\n", uop->name);
	}
	else if (uop->engine == VGPU_ENGINE_TEX)
	{
		str_printf(&buf_ptr, &buf_size,
			"   <span color=\"blue\"><b>%s</b></span>\n", uop->name);
	}
	else
	{
		char vliw_slot_label[5] = { 'x', 'y', 'z', 'w', 't' };
		for (i = 0; i < 5; i++)
		{
			if (!uop->vliw_slot[i][0])
				continue;
			str_printf(&buf_ptr, &buf_size,
				"<b><span color=\"darkred\">   %c: </span><span color=\"red\">%-12s %s</span></b>   \n",
				vliw_slot_label[i], uop->vliw_slot[i], uop->vliw_slot_args[i]);
		}
	}

	/* Additional info */
	str_printf(&buf_ptr, &buf_size,
		"\n"
		"Compute unit:  <b>CU-%d</b>\n"
		"Work-group:    <b>WG-%d</b>\n"
		"Wavefront:     <b>WF-%d</b>\n"
		"\n",
		uop->compute_unit_id,
		uop->work_group_id,
		uop->wavefront_id);

	/* Show the popup */
	info_popup_show(buf);
}