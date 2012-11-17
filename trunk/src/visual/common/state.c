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
#include <lib/util/file.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "state.h"
#include "trace.h"



/*
 * State checkpoint
 */

#define VI_STATE_CHECKPOINT_INTERVAL  500
#define VI_STATE_PROGRESS_INTERVAL  100000

struct vi_state_checkpoint_t
{
	long long cycle;

	/* Position in files */
	long int unzipped_trace_file_offset;
	long int checkpoint_file_offset;
};


struct vi_state_checkpoint_t *vi_state_checkpoint_create(long long cycle,
	long int unzipped_trace_file_offset, long int checkpoint_file_offset)
{
	struct vi_state_checkpoint_t *checkpoint;

	/* Initialize */
	checkpoint = xcalloc(1, sizeof(struct vi_state_checkpoint_t));
	checkpoint->cycle = cycle;
	checkpoint->unzipped_trace_file_offset = unzipped_trace_file_offset;
	checkpoint->checkpoint_file_offset = checkpoint_file_offset;
	
	/* Return */
	return checkpoint;
}


void vi_state_checkpoint_free(struct vi_state_checkpoint_t *checkpoint)
{
	free(checkpoint);
}




/*
 * State Command
 */

struct vi_state_command_t
{
	vi_state_process_trace_line_func_t process_trace_line_func;
	void *user_data;
};


struct vi_state_command_t *vi_state_command_create(
	vi_state_process_trace_line_func_t process_trace_line_func,
	void *user_data)
{
	struct vi_state_command_t *command;

	/* Initialize */
	command = xcalloc(1, sizeof(struct vi_state_command_t));
	command->process_trace_line_func = process_trace_line_func;
	command->user_data = user_data;

	/* Return */
	return command;
}


void vi_state_command_free(struct vi_state_command_t *command)
{
	free(command);
}




/*
 * State Category
 */

struct vi_state_category_t
{
	char *name;

	vi_state_read_checkpoint_func_t read_checkpoint_func;
	vi_state_write_checkpoint_func_t write_checkpoint_func;

	/* Argument to pass to the read/write checkpoint functions */
	void *user_data;
};


struct vi_state_category_t *vi_state_category_create(char *name,
	vi_state_read_checkpoint_func_t read_checkpoint_func,
	vi_state_write_checkpoint_func_t write_checkpoint_func,
	void *user_data)
{
	struct vi_state_category_t *category;

	/* Initialize */
	category = xcalloc(1, sizeof(struct vi_state_category_t));
	category->name = xstrdup(name);
	category->read_checkpoint_func = read_checkpoint_func;
	category->write_checkpoint_func = write_checkpoint_func;
	category->user_data = user_data;

	/* Return */
	return category;
}


void vi_state_category_free(struct vi_state_category_t *category)
{
	free(category->name);
	free(category);
}




/*
 * State File
 */

struct vi_state_t
{
	/* Uncompressed trace file */
	char *unzipped_trace_file_name;
	FILE *unzipped_trace_file;

	/* Checkpoint file */
	char *checkpoint_file_name;
	FILE *checkpoint_file;

	/* Cycles */
	long long cycle;
	long long num_cycles;

	/* List of categories */
	struct list_t *category_list;

	/* List of stored checkpoints */
	struct list_t *checkpoint_list;

	/* List of possible commands and functions handling them. Each element of
	 * the table is of type 'vi_state_process_trace_line_func_t'. */
	struct hash_table_t *command_table;

	/* Enumeration of header trace lines */
	struct vi_trace_line_t *header_trace_line;
	long int header_trace_line_offset;

	/* Enumeration of body trace lines */
	struct vi_trace_line_t *body_trace_line;
	long int body_trace_line_offset;
};


static struct vi_state_t *vi_state;


static void vi_state_read_checkpoint(int index)
{
	struct vi_state_checkpoint_t *checkpoint;
	struct vi_state_category_t *category;
	int i;

	/* Get checkpoint */
	checkpoint = list_get(vi_state->checkpoint_list, index);
	if (!checkpoint)
		panic("%s: invalid checkpoint index", __FUNCTION__);

	/* Set file positions */
	fseek(vi_state->checkpoint_file, checkpoint->checkpoint_file_offset, SEEK_SET);
	fseek(vi_state->unzipped_trace_file, checkpoint->unzipped_trace_file_offset, SEEK_SET);
	vi_state->cycle = checkpoint->cycle;

	/* Read checkpoint for every category */
	LIST_FOR_EACH(vi_state->category_list, i)
	{
		category = list_get(vi_state->category_list, i);
		category->read_checkpoint_func(category->user_data,
			vi_state->checkpoint_file);
	}
}


static void vi_state_write_checkpoint(void)
{
	struct vi_state_category_t *category;
	int i;

	/* Write checkpoint for every category */
	LIST_FOR_EACH(vi_state->category_list, i)
	{
		category = list_get(vi_state->category_list, i);
		category->write_checkpoint_func(category->user_data,
			vi_state->checkpoint_file);
	}
}


void vi_state_init(char *trace_file_name)
{
	struct vi_trace_t *trace_file;
	struct vi_trace_line_t *trace_line;

	int num_trace_lines;

	char buf[MAX_STRING_SIZE];

	/* Create */
	vi_state = xcalloc(1, sizeof(struct vi_state_t));
	
	/* Create uncompressed trace file */
	vi_state->unzipped_trace_file = file_create_temp(buf, sizeof buf);
	vi_state->unzipped_trace_file_name = xstrdup(buf);

	/* Create checkpoint file */
	vi_state->checkpoint_file = file_create_temp(buf, sizeof buf);
	vi_state->checkpoint_file_name = xstrdup(buf);

	/* Initialize */
	vi_state->checkpoint_list = list_create();
	vi_state->category_list = list_create();
	vi_state->command_table = hash_table_create(0, FALSE);

	/* Unpack trace */
	num_trace_lines = 0;
	trace_file = vi_trace_create(trace_file_name);
	while ((trace_line = vi_trace_line_create_from_trace(trace_file)))
	{
		/* Copy trace */
		vi_trace_line_dump(trace_line, vi_state->unzipped_trace_file);
		if (!strcmp(vi_trace_line_get_command(trace_line), "c"))
			vi_state->num_cycles = vi_trace_line_get_symbol_long_long(trace_line, "clk");
		vi_trace_line_free(trace_line);

		/* Show progress */
		num_trace_lines++;
		if (num_trace_lines % VI_STATE_PROGRESS_INTERVAL == 1)
		{
			printf("Uncompressing trace (%.1fMB, %lld cycles)   \r",
				ftell(vi_state->unzipped_trace_file) / 1.048e6, vi_state->num_cycles);
			fflush(stdout);
		}
	}
	vi_trace_free(trace_file);

	/* Final progress */
	printf("Uncompressing trace (%.1fMB, %lld cycles)   \n",
		ftell(vi_state->unzipped_trace_file) / 1.048e6, vi_state->num_cycles);
	fflush(stdout);
}


void vi_state_done(void)
{
	struct vi_state_category_t *category;
	struct vi_state_checkpoint_t *checkpoint;
	struct vi_state_command_t *command;

	char *command_name;

	int i;

	/* Close and delete uncompressed trace file */
	fclose(vi_state->unzipped_trace_file);
	unlink(vi_state->unzipped_trace_file_name);

	/* Close and detele checkpoint file */
	fclose(vi_state->checkpoint_file);
	unlink(vi_state->checkpoint_file_name);

	/* Free checkpoints */
	LIST_FOR_EACH(vi_state->checkpoint_list, i)
	{
		checkpoint = list_get(vi_state->checkpoint_list, i);
		vi_state_checkpoint_free(checkpoint);
	}
	list_free(vi_state->checkpoint_list);

	/* Free categories */
	LIST_FOR_EACH(vi_state->category_list, i)
	{
		category = list_get(vi_state->category_list, i);
		vi_state_category_free(category);
	}
	list_free(vi_state->category_list);

	/* Free command table */
	HASH_TABLE_FOR_EACH(vi_state->command_table, command_name, command)
		vi_state_command_free(command);
	hash_table_free(vi_state->command_table);

	/* Free current header trace line if any */
	if (vi_state->header_trace_line)
		vi_trace_line_free(vi_state->header_trace_line);

	/* Free current body trace line if any */
	if (vi_state->body_trace_line)
		vi_trace_line_free(vi_state->body_trace_line);

	/* Free */
	free(vi_state->unzipped_trace_file_name);
	free(vi_state->checkpoint_file_name);
	free(vi_state);
}


long long vi_state_get_num_cycles(void)
{
	return vi_state->num_cycles;
}


long long vi_state_get_current_cycle(void)
{
	return vi_state->cycle;
}


void vi_state_create_checkpoints(void)
{
	struct vi_trace_line_t *trace_line;

	long long last_checkpoint_cycle;
	long unzipped_trace_file_size;

	int num_trace_lines;

	/* Get unzipped trace file size */
	fseek(vi_state->unzipped_trace_file, 0, SEEK_END);
	unzipped_trace_file_size = ftell(vi_state->unzipped_trace_file);

	/* Initialize */
	last_checkpoint_cycle = -VI_STATE_CHECKPOINT_INTERVAL;
	fseek(vi_state->unzipped_trace_file, 0, SEEK_SET);

	/* Parse uncompressed trace file */
	num_trace_lines = 0;
	vi_state->cycle = 0;
	while ((trace_line = vi_trace_line_create_from_file(vi_state->unzipped_trace_file)))
	{
		struct vi_state_checkpoint_t *checkpoint;
		struct vi_state_command_t *state_command;

		char *command;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);

		/* New cycle command */
		if (!strcasecmp(command, "c"))
		{
			vi_state->cycle = atoll(vi_trace_line_get_symbol(trace_line, "clk"));
			while (vi_state->cycle >= last_checkpoint_cycle + VI_STATE_CHECKPOINT_INTERVAL)
			{
				last_checkpoint_cycle += VI_STATE_CHECKPOINT_INTERVAL;
				checkpoint = vi_state_checkpoint_create(last_checkpoint_cycle,
					vi_trace_line_get_offset(trace_line),
					ftell(vi_state->checkpoint_file));
				list_add(vi_state->checkpoint_list, checkpoint);
				vi_state_write_checkpoint();
			}
		}

		/* Process trace line if it is not a header  */
		else if (last_checkpoint_cycle >= 0)
		{
			state_command = hash_table_get(vi_state->command_table, command);
			if (!state_command)
				fatal("%s: unknown command '%s'", __FUNCTION__, command);
			state_command->process_trace_line_func(state_command->user_data, trace_line);
		}

		/* Progress */
		num_trace_lines++;
		if (num_trace_lines % VI_STATE_PROGRESS_INTERVAL == 1)
		{
			printf("Creating checkpoints (%.1fMB, %.1f%%)   \r",
				ftell(vi_state->checkpoint_file) / 1.048e6,
				unzipped_trace_file_size ?
				(double) ftell(vi_state->unzipped_trace_file) * 100.0 /
				unzipped_trace_file_size : 0.0);
			fflush(stdout);
		}

		/* Free trace line */
		vi_trace_line_free(trace_line);
	}

	/* Progress */
	printf("Creating checkpoints (%.1fMB, 100%%)   \n",
		ftell(vi_state->checkpoint_file) / 1.048e6);
	fflush(stdout);

	/* No checkpoint created - assume trace file empty */
	if (!list_count(vi_state->checkpoint_list))
		fatal("empty trace");

	/* Load first checkpoint */
	vi_state_read_checkpoint(0);
}


void vi_state_new_category(char *name,
	vi_state_read_checkpoint_func_t read_checkpoint_func,
	vi_state_write_checkpoint_func_t write_checkpoint_func,
	void *user_data)
{
	struct vi_state_category_t *category;

	category = vi_state_category_create(name, read_checkpoint_func,
		write_checkpoint_func, user_data);
	list_add(vi_state->category_list, category);
}


void vi_state_new_command(char *command_name,
	vi_state_process_trace_line_func_t process_trace_line_func,
	void *user_data)
{
	struct vi_state_command_t *command;

	command = vi_state_command_create(process_trace_line_func, user_data);
	if (!hash_table_insert(vi_state->command_table, command_name, command))
		fatal("%s: cannot insert command", __FUNCTION__);
}


struct vi_trace_line_t *vi_state_header_first(void)
{
	vi_state->header_trace_line_offset = 0;
	return vi_state_header_next();
}


struct vi_trace_line_t *vi_state_header_next(void)
{
	struct vi_trace_line_t *trace_line;

	/* Free current header trace line if any */
	if (vi_state->header_trace_line)
	{
		vi_trace_line_free(vi_state->header_trace_line);
		vi_state->header_trace_line = NULL;
	}

	/* No more header trace lines */
	if (vi_state->header_trace_line_offset < 0)
		return NULL;

	/* Read trace line */
	fseek(vi_state->unzipped_trace_file, vi_state->header_trace_line_offset, SEEK_SET);
	trace_line = vi_trace_line_create_from_file(vi_state->unzipped_trace_file);
	if (!trace_line)
	{
		vi_state->header_trace_line_offset = -1;
		return NULL;
	}

	/* Header is over */
	if (!strcmp(vi_trace_line_get_command(trace_line), "c"))
	{
		vi_trace_line_free(trace_line);
		vi_state->header_trace_line_offset = -1;
		return NULL;
	}

	/* Save trace line and return */
	vi_state->header_trace_line = trace_line;
	vi_state->header_trace_line_offset = ftell(vi_state->unzipped_trace_file);
	return trace_line;
}


struct vi_trace_line_t *vi_state_trace_line_first(long long cycle)
{
	long int trace_file_offset;

	int checkpoint_index;

	long long checkpoint_cycle;

	struct vi_state_checkpoint_t *checkpoint;

	/* Release previous body trace line if any */
	if (vi_state->body_trace_line)
	{
		vi_trace_line_free(vi_state->body_trace_line);
		vi_state->body_trace_line = NULL;
	}

	/* Invalid cycle */
	if (cycle > vi_state->num_cycles)
		return NULL;

	/* Store current position in trace file */
	trace_file_offset = ftell(vi_state->unzipped_trace_file);

	/* Get closest checkpoint */
	checkpoint_index = cycle / VI_STATE_CHECKPOINT_INTERVAL;
	checkpoint_cycle = (long long) checkpoint_index * VI_STATE_CHECKPOINT_INTERVAL;
	checkpoint = list_get(vi_state->checkpoint_list, checkpoint_index);
	if (!checkpoint)
		panic("%s: invalid checkpoint index", __FUNCTION__);

	/* Set position in trace file */
	fseek(vi_state->unzipped_trace_file, checkpoint->unzipped_trace_file_offset, SEEK_SET);
	vi_state->body_trace_line_offset = checkpoint->unzipped_trace_file_offset;
	for (;;)
	{
		/* Read trace line */
		vi_state->body_trace_line = vi_trace_line_create_from_file(vi_state->unzipped_trace_file);
		vi_state->body_trace_line_offset = checkpoint->unzipped_trace_file_offset;
		if (!vi_state->body_trace_line || checkpoint_cycle == cycle)
			break;

		/* Check if target cycle is exceeded */
		if (!strcmp(vi_trace_line_get_command(vi_state->body_trace_line), "c") &&
			vi_trace_line_get_symbol_long_long(vi_state->body_trace_line, "clk") >= cycle)
			break;

		/* Free trace line */
		vi_trace_line_free(vi_state->body_trace_line);
		vi_state->body_trace_line = NULL;
	}

	/* Return to original position in trace file */
	fseek(vi_state->unzipped_trace_file, trace_file_offset, SEEK_SET);
	return vi_state->body_trace_line;
}


struct vi_trace_line_t *vi_state_trace_line_next(void)
{
	long long trace_file_offset;

	/* Store current position in trace file */
	trace_file_offset = ftell(vi_state->unzipped_trace_file);

	/* Release previous body trace line if any */
	if (vi_state->body_trace_line)
	{
		vi_trace_line_free(vi_state->body_trace_line);
		vi_state->body_trace_line = NULL;
	}

	/* Get next trace line */
	fseek(vi_state->unzipped_trace_file, vi_state->body_trace_line_offset, SEEK_SET);
	vi_state->body_trace_line = vi_trace_line_create_from_file(vi_state->unzipped_trace_file);
	vi_state->body_trace_line_offset = ftell(vi_state->unzipped_trace_file);

	/* Return to original position in trace file */
	fseek(vi_state->unzipped_trace_file, trace_file_offset, SEEK_SET);
	return vi_state->body_trace_line;
}


void vi_state_go_to_cycle(long long cycle)
{
	long long checkpoint_cycle;
	int checkpoint_index;

	/* If we are already in this cycle, do nothing */
	if (vi_state->cycle == cycle)
		return;

	/* Load a checkpoint */
	checkpoint_index = cycle / VI_STATE_CHECKPOINT_INTERVAL;
	checkpoint_cycle = (long long) checkpoint_index * VI_STATE_CHECKPOINT_INTERVAL;
	if (cycle < vi_state->cycle || checkpoint_cycle > vi_state->cycle)
		vi_state_read_checkpoint(checkpoint_index);

	/* Go to cycle */
	for (;;)
	{
		struct vi_trace_line_t *trace_line;
		struct vi_state_command_t *state_command;
		long int unzipped_trace_file_pos;
		char *command;

		/* Read a trace line */
		unzipped_trace_file_pos = ftell(vi_state->unzipped_trace_file);
		trace_line = vi_trace_line_create_from_file(vi_state->unzipped_trace_file);
		if (!trace_line)
			break;

		/* New cycle */
		command = vi_trace_line_get_command(trace_line);
		if (!strcmp(command, "c"))
		{
			long long new_cycle;

			/* Get new cycle number */
			new_cycle = vi_trace_line_get_symbol_long_long(trace_line, "clk");

			/* If we passed the target cycle, done */
			if (new_cycle > cycle)
			{
				fseek(vi_state->unzipped_trace_file, unzipped_trace_file_pos, SEEK_SET);
				vi_trace_line_free(trace_line);
				break;
			}
			else
			{
				vi_state->cycle = new_cycle;
			}
		}
		else
		{
			/* Process trace line */
			state_command = hash_table_get(vi_state->command_table, command);
			if (!state_command)
				fatal("%s: unknown command '%s'", __FUNCTION__, command);
			state_command->process_trace_line_func(state_command->user_data, trace_line);
		}

		/* Free trace line */
		vi_trace_line_free(trace_line);
	}

	/* Cycle reached */
	vi_state->cycle = cycle;
}
