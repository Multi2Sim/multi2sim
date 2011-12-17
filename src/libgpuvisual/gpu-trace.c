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


/*
 * GPU Trace
 */

char vgpu_trace_err[MAX_STRING_SIZE];

#define VGPU_STATE_CHECKPOINT_INTERVAL  500

#define VGPU_TRACE_ERROR(MSG) snprintf(vgpu_trace_err, MAX_STRING_SIZE, "%s:%d: %s", \
	gpu->trace_file_name, gpu->trace_line_number, (MSG))

#define VGPU_TRACE_LINE_MAX_TOKENS  30

struct vgpu_trace_line_token_t {
	char *var;
	char *value;
};

struct vgpu_trace_line_t {
	char line[MAX_STRING_SIZE];
	int num_tokens;
	char *command;
	struct vgpu_trace_line_token_t tokens[VGPU_TRACE_LINE_MAX_TOKENS];
};


/*
 * Return values:
 *   0 - OK
 *   1 - End of file reached
 *   2 - Other error - stored in 'vgpu_trace_err'
 */
int vgpu_trace_line_read(struct vgpu_t *gpu, struct vgpu_trace_line_t *trace_line)
{
	char *line;
	int len;
	struct vgpu_trace_line_token_t *token;

	/* Read line from file */
	do {
		line = fgets(trace_line->line, MAX_STRING_SIZE, gpu->trace_file);
		if (!line)
			return 1;
		gpu->trace_line_number++;

		/* Remove spaces and '\n' */
		len = strlen(line);
		while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\n'))
			line[len--] = '\0';
		while (len > 0 && line[0] == ' ')
			line++;

	} while (!len);

	/* Get command */
	trace_line->command = line;
	while (isalnum(*line))
		line++;
	while (isspace(*line))
		*line++ = '\0';

	/* Get tokens */
	trace_line->num_tokens = 0;
	while (*line) {

		/* New token */
		if (trace_line->num_tokens >= VGPU_TRACE_LINE_MAX_TOKENS) {
			VGPU_TRACE_ERROR("maximum number of tokens exceeded");
			return 2;
		}
		token = &trace_line->tokens[trace_line->num_tokens++];

		/* Get variable */
		token->var = line;
		while (isalnum(*line) || *line == '_' || *line == '.')
			line++;
		while (isspace(*line))
			*line++ = '\0';

		/* An '=' sign is expected here */
		if (*line != '=') {
			VGPU_TRACE_ERROR("wrong syntax");
			return 2;
		}
		while (isspace(*line) || *line == '=')
			*line++ = '\0';

		/* Read value */
		if (*line == '"') {
			*line++ = '\0';
			token->value = line;
			while (*line && *line != '"')
				line++;
			if (*line == '"')
				*line++ = '\0';
		} else {
			token->value = line;
			while (!isspace(*line))
				line++;
		}
		while (isspace(*line))
			*line++ = '\0';
	}

	/* Return success */
	return 0;
}


static int vgpu_trace_line_token_index(struct vgpu_trace_line_t *trace_line, char *var)
{
	struct vgpu_trace_line_token_t *token;
	int i;

	for (i = 0; i < trace_line->num_tokens; i++) {
		token = &trace_line->tokens[i];
		if (!strcmp(token->var, var))
			return i;
	}
	return -1;
}


int vgpu_trace_line_token_int(struct vgpu_trace_line_t *trace_line, char *var)
{
	int index;

	index = vgpu_trace_line_token_index(trace_line, var);
	if (index < 0)
		return 0;
	return atoi(trace_line->tokens[index].value);
}


char *vgpu_trace_line_token(struct vgpu_trace_line_t *trace_line, char *var)
{
	int index;

	index = vgpu_trace_line_token_index(trace_line, var);
	if (index < 0)
		return "";
	return trace_line->tokens[index].value;
}


void vgpu_trace_line_read_vliw(struct vgpu_trace_line_t *trace_line, struct vgpu_uop_t *uop)
{
	char vliw_elems[5] = { 'x', 'y', 'z', 'w', 't' };
	char vliw_elem;

	char vliw_elem_name[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];
	int i;

	uop->name[0] = '\0';
	for (i = 0; i < 5; i++) {

		/* Get VLIW slot */
		vliw_elem = vliw_elems[i];
		sprintf(str, "inst.%c", vliw_elem);
		snprintf(vliw_elem_name, sizeof(vliw_elem_name), "%s", vgpu_trace_line_token(trace_line, str));
		if (!vliw_elem_name[0])
			continue;

		/* Add to instruction name */
		snprintf(uop->vliw_slot[i], sizeof(uop->vliw_slot[i]), "%s", strtok(vliw_elem_name, " "));
		snprintf(uop->vliw_slot_args[i], sizeof(uop->vliw_slot_args[i]), "%s", strtok(NULL, ""));
	}
}


/* Return value:
 *   0 - OK
 *   1 - End of trace reached
 *   2 - Format error in line
 */
int vgpu_trace_line_process(struct vgpu_t *gpu)
{
	struct vgpu_trace_line_t trace_line;
	char *command;
	int err;

	/* Read trace line */
	err = vgpu_trace_line_read(gpu, &trace_line);
	if (err)
		return err;
	command = trace_line.command;

	/* Command 'cu' */
	if (!strcmp(command, "cu"))
	{
		struct vgpu_compute_unit_t *compute_unit;
		int compute_unit_id;
		char *action;
		
		/* Get compute unit */
		compute_unit_id = vgpu_trace_line_token_int(&trace_line, "cu");
		if (compute_unit_id < 0 || compute_unit_id >= gpu->num_compute_units) {
			VGPU_TRACE_ERROR("compute unit id out of bounds");
			return 2;
		}
		compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);

		/* Get action */
		action = vgpu_trace_line_token(&trace_line, "a");

		/* Map work-group */
		if (!strcmp(action, "map"))
		{
			struct vgpu_work_group_t *work_group;
			int work_group_id;

			/* Remove work-group from pending list */
			work_group_id = vgpu_trace_line_token_int(&trace_line, "wg");
			work_group = list_remove_at(gpu->pending_work_group_list, 0);
			if (!work_group || work_group->id != work_group_id) {
				VGPU_TRACE_ERROR("work-group not in head of pending list");
				return 2;
			}

			/* Add work-group to compute unit's list */
			if (list_index_of(compute_unit->work_group_list, work_group) >= 0) {
				VGPU_TRACE_ERROR("work-group is already in compute unit");
				return 2;
			}
			list_add(compute_unit->work_group_list, work_group);

			/* Record another mapped work-group */
			assert(gpu->num_mapped_work_groups == work_group_id);
			gpu->num_mapped_work_groups++;
		}

		/* Unmap work-group */
		else if (!strcmp(action, "unmap"))
		{
			struct vgpu_work_group_t *work_group;
			int work_group_id;
			int found, i;

			/* Remove work-group from compute unit */
			work_group_id = vgpu_trace_line_token_int(&trace_line, "wg");
			found = 0;
			for (i = 0; i < list_count(compute_unit->work_group_list); i++)
			{
				work_group = list_get(compute_unit->work_group_list, i);
				if (work_group->id == work_group_id)
				{
					found = 1;
					list_remove_at(compute_unit->work_group_list, i);
					break;
				}
			}
			if (!found)
			{
				VGPU_TRACE_ERROR("work-group is not in compute unit");
				return 2;
			}

			/* Add work-group to finished list in order */
			if (list_index_of(gpu->finished_work_group_list, work_group) >= 0)
			{
				VGPU_TRACE_ERROR("work-group is already in finished list");
				return 2;
			}
			for (i = list_count(gpu->finished_work_group_list); i > 0; i--)
			{
				struct vgpu_work_group_t *work_group_curr;
				work_group_curr = list_get(gpu->finished_work_group_list, i - 1);
				if (work_group_curr->id < work_group->id)
					break;
			}
			list_insert(gpu->finished_work_group_list, i, work_group);
		}

		/* Unknown action */
		else
		{
			VGPU_TRACE_ERROR("invalid action");
			return 2;
		}
	}

	/* Command 'clk' */
	else if (!strcmp(command, "clk"))
	{
		int cycle;

		cycle = vgpu_trace_line_token_int(&trace_line, "c");
		if (cycle != gpu->cycle + 1)
		{
			VGPU_TRACE_ERROR("invalid cycle number");
			return 2;
		}
		gpu->cycle = cycle;
		if (cycle > gpu->max_cycles)
			gpu->max_cycles = cycle;
	}

	/* Create a 'uop' */
	else if (!strcmp(command, "cf") || !strcmp(command, "alu") || !strcmp(command, "tex"))
	{
		struct vgpu_compute_unit_t *compute_unit;
		struct vgpu_uop_t *uop;
		char *stage;
		int compute_unit_id;
		int uop_id;

		compute_unit_id = vgpu_trace_line_token_int(&trace_line, "cu");
		if (compute_unit_id < 0 || compute_unit_id >= gpu->num_compute_units)
		{
			VGPU_TRACE_ERROR("invalid compute unit id");
			return 2;
		}
		compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);

		/* Stage 'fetch' for any engine */
		stage = vgpu_trace_line_token(&trace_line, "a");
		uop_id = vgpu_trace_line_token_int(&trace_line, "uop");
		if (!strcmp(stage, "fetch"))
		{
			/* Create uop */
			if (vgpu_uop_list_get(compute_unit->uop_list, uop_id))
			{
				VGPU_TRACE_ERROR("uop already exists");
				return 2;
			}
			uop = vgpu_uop_create(uop_id);
			uop->compute_unit_id = compute_unit_id;
			uop->work_group_id = vgpu_trace_line_token_int(&trace_line, "wg");
			uop->wavefront_id = vgpu_trace_line_token_int(&trace_line, "wf");
			str_single_space(uop->name, vgpu_trace_line_token(&trace_line, "inst"), sizeof(uop->name));
			if (uop_id > compute_unit->max_uops)
				compute_unit->max_uops = uop_id;

			/* Read VLIW instruction */
			if (!strcmp(command, "alu"))
				vgpu_trace_line_read_vliw(&trace_line, uop);

			/* Add it to uop list */
			err = vgpu_uop_list_add(compute_unit->uop_list, uop);
			if (err)
			{
				VGPU_TRACE_ERROR("non-sequential uop id");
				return 2;
			}
		}

		/* Any stage other than 'fetch' for any engine */
		else
		{
			/* Get uop */
			uop = vgpu_uop_list_get(compute_unit->uop_list, uop_id);
			if (!uop)
			{
				VGPU_TRACE_ERROR("invalid uop");
				return 2;
			}
		}

		/* CF Engine */
		if (!strcmp(command, "cf"))
		{
			uop->engine = VGPU_ENGINE_CF;
			uop->stage_cycle = gpu->cycle;
			if (!strcmp(stage, "fetch"))
				uop->stage = VGPU_STAGE_FETCH;
			else if (!strcmp(stage, "decode"))
				uop->stage = VGPU_STAGE_DECODE;
			else if (!strcmp(stage, "execute"))
				uop->stage = VGPU_STAGE_EXECUTE;
			else if (!strcmp(stage, "complete"))
			{
				uop->stage = VGPU_STAGE_COMPLETE;
				uop->finished = 1;
			}
			else
			{
				VGPU_TRACE_ERROR("invalid stage");
				return 2;
			}
		}

		/* ALU Engine */
		else if (!strcmp(command, "alu"))
		{
			uop->engine = VGPU_ENGINE_ALU;
			uop->stage_cycle = gpu->cycle;
			if (!strcmp(stage, "fetch"))
				uop->stage = VGPU_STAGE_FETCH;
			else if (!strcmp(stage, "decode"))
				uop->stage = VGPU_STAGE_DECODE;
			else if (!strcmp(stage, "read"))
				uop->stage = VGPU_STAGE_READ;
			else if (!strcmp(stage, "exec"))
				uop->stage = VGPU_STAGE_EXECUTE;
			else if (!strcmp(stage, "write"))
			{
				uop->stage = VGPU_STAGE_WRITE;
				uop->finished = 1;
			}
			else
			{
				VGPU_TRACE_ERROR("invalid stage");
				return 2;
			}
		}

		/* TEX Engine */
		else
		{
			uop->engine = VGPU_ENGINE_TEX;
			uop->stage_cycle = gpu->cycle;
			if (!strcmp(stage, "fetch"))
				uop->stage = VGPU_STAGE_FETCH;
			else if (!strcmp(stage, "decode"))
				uop->stage = VGPU_STAGE_DECODE;
			else if (!strcmp(stage, "read"))
				uop->stage = VGPU_STAGE_READ;
			else if (!strcmp(stage, "write")) {
				uop->stage = VGPU_STAGE_WRITE;
				uop->finished = TRUE;
			} else {
				VGPU_TRACE_ERROR("invalid stage");
				return 2;
			}
		}
	}

	/* Other command */
	else
	{
		VGPU_TRACE_ERROR("invalid command");
		return 2;
	}

	/* Success */
	return 0;
}


/* Return value
 *   0 - OK
 *   1 - Already in last cycle
 *   2 - Error found in some trace line
 */
int vgpu_trace_next_cycle(struct vgpu_t *gpu)
{
	int cycle;
	int err;
	long trace_file_pos;
	int i;

	/* Parse commands until the next 'clk' command is found.
	 * This should only happen when skipping the initialization commands before cycle 1.
	 * Otherwise, the current trace line should contain the 'clk' command. */
	cycle = gpu->cycle;
	do {
		err = vgpu_trace_line_process(gpu);
		if (err)
			return err;
	} while (gpu->cycle == cycle);

	/* First thing to do in a new cycle is remove all finished uops from the
	 * compute units' uop lists. */
	for (i = 0; i < gpu->num_compute_units; i++)
	{
		struct vgpu_compute_unit_t *compute_unit;
		struct list_t *uop_list;
		struct vgpu_uop_t *uop;

		compute_unit = list_get(gpu->compute_unit_list, i);
		uop_list = compute_unit->uop_list;
		while (list_count(uop_list)) {
			uop = list_get(uop_list, 0);
			if (uop->finished) {
				list_remove_at(uop_list, 0);
				compute_unit->last_completed_uop_id = uop->id;
				vgpu_uop_free(uop);
			} else
				break;
		}
	}

	/* Parse line until the next 'clk' command. When it is found,
	 * go back to it so that next time it will be processed first. */
	cycle = gpu->cycle;
	do {
		trace_file_pos = ftell(gpu->trace_file);
		err = vgpu_trace_line_process(gpu);
		if (err == 2)
			return 2;
	} while (gpu->cycle == cycle && !err);
	fseek(gpu->trace_file, trace_file_pos, SEEK_SET);
	gpu->trace_line_number--;
	gpu->cycle = cycle;

	/* File position should be at the end, or right at a 'clk' trace line. */
	return 0;
}


/* Return value
 *   0 - OK
 *   1 - Cycle exceeds end of file
 *   2 - Error in trace lines
 */
int vgpu_trace_cycle(struct vgpu_t *gpu, int cycle)
{
	int err;
	int checkpoint_index;
	int load_checkpoint;
	int distance_from_checkpoint;
	struct vgpu_state_checkpoint_t *checkpoint;

	/* Check if it is better to load a checkpoint or to go to the target
	 * cycle from the current GPU state. */
	load_checkpoint = TRUE;
	checkpoint_index = cycle / VGPU_STATE_CHECKPOINT_INTERVAL;
	distance_from_checkpoint = cycle - checkpoint_index * VGPU_STATE_CHECKPOINT_INTERVAL;
	if (gpu->cycle <= cycle && distance_from_checkpoint >= cycle - gpu->cycle)
		load_checkpoint = FALSE;

	/* Load checkpoint */
	if (load_checkpoint)
	{
		checkpoint = list_get(gpu->state_checkpoint_list, checkpoint_index);
		if (!checkpoint)
			return 1;
		fseek(gpu->state_file, checkpoint->state_file_pos, SEEK_SET);
		fseek(gpu->trace_file, checkpoint->trace_file_pos, SEEK_SET);
		vgpu_load_state(gpu);
		assert(gpu->cycle == checkpoint->cycle);
	}

	/* Go to cycle */
	while (gpu->cycle < cycle)
	{
		err = vgpu_trace_next_cycle(gpu);
		if (err)
			return err;
	}

	/* Success */
	return 0;
}


/* Return value:
 *   0 - OK
 *   1 - End of trace reached before intro finished
 *   2 - Format error in line
 */
int vgpu_trace_parse_intro(struct vgpu_t *gpu)
{
	int err;
	long int trace_file_pos;

	struct vgpu_trace_line_t trace_line;
	char *command;

	for (;;)
	{
		/* Get a trace line */
		trace_file_pos = ftell(gpu->trace_file);
		err = vgpu_trace_line_read(gpu, &trace_line);

		/* Errors */
		if (err == 1)
			VGPU_TRACE_ERROR("end of file reached in intro");
		if (err)
			return err;
		command = trace_line.command;

		/* Initialization command */
		if (!strcmp(command, "init"))
		{
			struct vgpu_compute_unit_t *compute_unit;
			int i;

			gpu->num_work_groups = vgpu_trace_line_token_int(&trace_line, "group_count");
			gpu->num_compute_units = vgpu_trace_line_token_int(&trace_line, "compute_units");
			for (i = 0; i < gpu->num_compute_units; i++)
			{
				compute_unit = vgpu_compute_unit_create(gpu, i);
				list_add(gpu->compute_unit_list, compute_unit);
			}
		}

		/* New element */
		else if (!strcmp(command, "new"))
		{
			char *item;

			/* Get item */
			item = vgpu_trace_line_token(&trace_line, "item");
			
			/* New work-group */
			if (!strcmp(item, "wg"))
			{
				struct vgpu_work_group_t *work_group;
				int id;

				/* Get ID */
				id = vgpu_trace_line_token_int(&trace_line, "id");
				if (id != list_count(gpu->work_group_list))
				{
					VGPU_TRACE_ERROR("unordered work-group id");
					return 2;
				}

				/* Create work_group */
				work_group = vgpu_work_group_create(id);
				list_add(gpu->work_group_list, work_group);
				list_add(gpu->pending_work_group_list, work_group);

				/* Fields */
				work_group->id = id;
				work_group->work_item_id_first = vgpu_trace_line_token_int(&trace_line, "wi_first");
				work_group->work_item_count = vgpu_trace_line_token_int(&trace_line, "wi_count");
				work_group->work_item_id_last = work_group->work_item_id_first +
					work_group->work_item_count - 1;
				work_group->wavefront_id_first = vgpu_trace_line_token_int(&trace_line, "wf_first");
				work_group->wavefront_count = vgpu_trace_line_token_int(&trace_line, "wf_count");
				work_group->wavefront_id_last = work_group->wavefront_id_first +
					work_group->wavefront_count - 1;
			}

			/* New wavefront */
			else if (!strcmp(item, "wf"))
			{
			}

			/* Not recognized */
			else
			{
				VGPU_TRACE_ERROR("invalid value for token 'item'");
				return 2;
			}
		}

		/* Assembly code */
		else if (!strcmp(command, "asm"))
		{
			int inst_idx;
			char *clause;
			char text[MAX_STRING_SIZE];

			/* Get disassembly line */
			inst_idx = vgpu_trace_line_token_int(&trace_line, "i");
			clause = vgpu_trace_line_token(&trace_line, "cl");
			if (!clause || inst_idx != list_count(gpu->kernel_source_strings))
			{
				VGPU_TRACE_ERROR("invalid disassembly");
				return 2;
			}

			/* CF clause */
			if (!strcmp(clause, "cf"))
			{
				char *inst;
				int count;
				int loop_idx;

				int size = sizeof(text);
				char *cursor = text;

				int i;

				inst = vgpu_trace_line_token(&trace_line, "inst");
				count = vgpu_trace_line_token_int(&trace_line, "cnt");
				loop_idx = vgpu_trace_line_token_int(&trace_line, "l");

				for (i = 0; i < 3 * loop_idx; i++)
					cursor += snprintf(cursor, size - (cursor - text), " ");
				cursor += snprintf(cursor, size - (cursor - text),
					"<span color=\"darkgreen\">%02d <b>%s</b></span>\n", count, inst);
				list_add(gpu->kernel_source_strings, strdup(text));
			}

			/* ALU clause */
			else if (!strcmp(clause, "alu"))
			{
				int size = sizeof(text);
				char *cursor = text;

				char *slot_name[5] = { "x", "y", "z", "w", "t" };
				char *slot_inst_name[5] = { "inst.x", "inst.y", "inst.z", "inst.w", "inst.t" };
				char *inst;

				int i, j;
				int first_printed_slot = 1;

				int count;
				int loop_idx;

				count = vgpu_trace_line_token_int(&trace_line, "cnt");
				loop_idx = vgpu_trace_line_token_int(&trace_line, "l");

				cursor += snprintf(cursor, size - (cursor - text), "<span color=\"red\">");
				for (j = 0; j < 5; j++)
				{
					inst = vgpu_trace_line_token(&trace_line, slot_inst_name[j]);
					if (!*inst)
						continue;

					for (i = 0; i < 3 * loop_idx; i++)
						cursor += snprintf(cursor, size - (cursor - text), " ");

					if (first_printed_slot)
						cursor += snprintf(cursor, size - (cursor - text), "%4d", count);
					else
						cursor += snprintf(cursor, size - (cursor - text), "%4s", "");
					first_printed_slot = 0;

					cursor += snprintf(cursor, size - (cursor - text),
						" <b><span color=\"darkred\">%s</span>: %s</b>\n", slot_name[j], inst);
				}
				cursor += snprintf(cursor, size - (cursor - text), "</span>");
				list_add(gpu->kernel_source_strings, strdup(text));
			}

			/* TEX clause */
			else if (!strcmp(clause, "tex"))
			{
				int size = sizeof(text);
				char *cursor = text;

				char *inst;
				int i;

				int count;
				int loop_idx;

				count = vgpu_trace_line_token_int(&trace_line, "cnt");
				loop_idx = vgpu_trace_line_token_int(&trace_line, "l");

				for (i = 0; i < 3 * loop_idx; i++)
					cursor += snprintf(cursor, size - (cursor - text), " ");

				inst = vgpu_trace_line_token(&trace_line, "inst");
				cursor += snprintf(cursor, size - (cursor - text), "<span color=\"blue\">");
				cursor += snprintf(cursor, size - (cursor - text), "%4d", count);
				cursor += snprintf(cursor, size - (cursor - text),
					" <b>%s</b>\n", inst);
				cursor += snprintf(cursor, size - (cursor - text), "</span>");
				list_add(gpu->kernel_source_strings, strdup(text));
			}

			/* Invalid */
			else {
				VGPU_TRACE_ERROR("invalid clause in disassembly");
				return 2;
			}
		}

		/* Command not belonging to intro anymore.
		 * Return to previous position and leave. */
		else
		{
			gpu->trace_line_number--;
			fseek(gpu->trace_file, trace_file_pos, SEEK_SET);
			break;
		}
	}

	/* Success */
	return 0;
}


int vgpu_trace_parse(struct vgpu_t *gpu)
{
	int err;

	/* Parse initial trace line */
	err = vgpu_trace_parse_intro(gpu);
	if (err)
		return err;

	/* Parse whole file */
	do
	{
		/* Make state checkpoint */
		if (gpu->cycle % VGPU_STATE_CHECKPOINT_INTERVAL == 0)
		{
			struct vgpu_state_checkpoint_t *checkpoint;
			checkpoint = calloc(1, sizeof(struct vgpu_state_checkpoint_t));
			checkpoint->cycle = gpu->cycle;
			checkpoint->state_file_pos = ftell(gpu->state_file);
			checkpoint->trace_file_pos = ftell(gpu->trace_file);
			list_add(gpu->state_checkpoint_list, checkpoint);
			vgpu_store_state(gpu);
		}

		/* Go to next cycle */
		err = vgpu_trace_next_cycle(gpu);
		if (err == 2)
			return err;
	} while (!err);

	/* Go to first cycle */
	vgpu_trace_cycle(gpu, 0);

	/* Return success */
	return 0;
}
