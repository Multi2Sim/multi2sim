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

#include <memvisual-private.h>


#define STATE_CHECKPOINT_INTERVAL  500
#define STATE_PROGRESS_INTERVAL  100000


/*
 * State checkpoint
 */

struct state_checkpoint_t
{
	long long cycle;

	/* Position in files */
	long int unzipped_trace_file_offset;
	long int checkpoint_file_offset;
};


struct state_checkpoint_t *state_checkpoint_create(long long cycle,
	long int unzipped_trace_file_offset, long int checkpoint_file_offset)
{
	struct state_checkpoint_t *checkpoint;

	/* Allocate */
	checkpoint = calloc(1, sizeof(struct state_checkpoint_t));
	if (!checkpoint)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	checkpoint->cycle = cycle;
	checkpoint->unzipped_trace_file_offset = unzipped_trace_file_offset;
	checkpoint->checkpoint_file_offset = checkpoint_file_offset;
	
	/* Return */
	return checkpoint;
}


void state_checkpoint_free(struct state_checkpoint_t *checkpoint)
{
	free(checkpoint);
}




/*
 * State Command
 */

struct state_command_t
{
	state_file_process_trace_line_func_t process_trace_line_func;
	void *user_data;
};


struct state_command_t *state_command_create(
	state_file_process_trace_line_func_t process_trace_line_func,
	void *user_data)
{
	struct state_command_t *command;

	/* Allocate */
	command = calloc(1, sizeof(struct state_command_t));
	if (!command)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	command->process_trace_line_func = process_trace_line_func;
	command->user_data = user_data;

	/* Return */
	return command;
}


void state_command_free(struct state_command_t *command)
{
	free(command);
}




/*
 * State Category
 */

struct state_category_t
{
	char *name;

	state_file_read_checkpoint_func_t read_checkpoint_func;
	state_file_write_checkpoint_func_t write_checkpoint_func;
	state_file_refresh_func_t refresh_func;

	/* Argument to pass to the read/write checkpoint functions */
	void *user_data;
};


struct state_category_t *state_category_create(char *name,
	state_file_read_checkpoint_func_t read_checkpoint_func,
	state_file_write_checkpoint_func_t write_checkpoint_func,
	state_file_refresh_func_t refresh_func,
	void *user_data)
{
	struct state_category_t *category;

	/* Allocate */
	category = calloc(1, sizeof(struct state_category_t));
	if (!category)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	category->name = strdup(name);
	if (!category->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	category->read_checkpoint_func = read_checkpoint_func;
	category->write_checkpoint_func = write_checkpoint_func;
	category->refresh_func = refresh_func;
	category->user_data = user_data;

	/* Return */
	return category;
}


void state_category_free(struct state_category_t *category)
{
	free(category->name);
	free(category);
}




/*
 * State File
 */

struct state_file_t
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
	 * the table is of type 'state_file_process_trace_line_func_t'. */
	struct hash_table_t *command_table;

	/* Enumeration of header trace lines */
	struct trace_line_t *header_trace_line;
	long int header_trace_line_offset;
};


static void state_file_read_checkpoint(struct state_file_t *file, int index)
{
	struct state_checkpoint_t *checkpoint;
	struct state_category_t *category;
	int i;

	/* Get checkpoint */
	checkpoint = list_get(file->checkpoint_list, index);
	if (!checkpoint)
		panic("%s: invalid checkpoint index", __FUNCTION__);

	/* Set file positions */
	fseek(file->checkpoint_file, checkpoint->checkpoint_file_offset, SEEK_SET);
	fseek(file->unzipped_trace_file, checkpoint->unzipped_trace_file_offset, SEEK_SET);
	file->cycle = checkpoint->cycle;

	/* Read checkpoint for every category */
	LIST_FOR_EACH(file->category_list, i)
	{
		category = list_get(file->category_list, i);
		category->read_checkpoint_func(category->user_data,
			file->checkpoint_file);
	}
}


static void state_file_write_checkpoint(struct state_file_t *file)
{
	struct state_category_t *category;
	int i;

	/* Write checkpoint for every category */
	LIST_FOR_EACH(file->category_list, i)
	{
		category = list_get(file->category_list, i);
		category->write_checkpoint_func(category->user_data,
			file->checkpoint_file);
	}
}


struct state_file_t *state_file_create(char *trace_file_name)
{
	struct state_file_t *file;
	struct trace_file_t *trace_file;
	struct trace_line_t *trace_line;

	int num_trace_lines;

	char buf[MAX_STRING_SIZE];

	/* Create */
	file = calloc(1, sizeof(struct state_file_t));
	if (!file)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Create uncompressed trace file */
	file->unzipped_trace_file = create_temp_file(buf, sizeof buf);
	file->unzipped_trace_file_name = strdup(buf);
	if (!file->unzipped_trace_file_name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Create checkpoint file */
	file->checkpoint_file = create_temp_file(buf, sizeof buf);
	file->checkpoint_file_name = strdup(buf);
	if (!file->checkpoint_file_name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	file->checkpoint_list = list_create();
	file->category_list = list_create();
	file->command_table = hash_table_create(0, FALSE);

	/* Unpack trace */
	num_trace_lines = 0;
	trace_file = trace_file_create(trace_file_name);
	while ((trace_line = trace_line_create_from_trace_file(trace_file)))
	{
		/* Copy trace */
		trace_line_dump(trace_line, file->unzipped_trace_file);
		if (!strcmp(trace_line_get_command(trace_line), "c"))
			file->num_cycles = trace_line_get_symbol_value_long_long(trace_line, "clk");
		trace_line_free(trace_line);

		/* Show progress */
		num_trace_lines++;
		if (num_trace_lines % STATE_PROGRESS_INTERVAL == 1)
		{
			printf("Uncompressing trace (%.1fMB, %lld cycles)   \r",
				ftell(file->unzipped_trace_file) / 1.048e6, file->num_cycles);
			fflush(stdout);
		}
	}
	trace_file_free(trace_file);

	/* Final progress */
	printf("Uncompressing trace (%.1fMB, %lld cycles)   \n",
		ftell(file->unzipped_trace_file) / 1.048e6, file->num_cycles);
	fflush(stdout);

	/* Return */
	return file;
}


void state_file_free(struct state_file_t *file)
{
	struct state_category_t *category;
	struct state_checkpoint_t *checkpoint;
	struct state_command_t *command;

	char *command_name;

	int i;

	/* Close and delete uncompressed trace file */
	fclose(file->unzipped_trace_file);
	unlink(file->unzipped_trace_file_name);

	/* Close and detele checkpoint file */
	fclose(file->checkpoint_file);
	unlink(file->checkpoint_file_name);

	/* Free checkpoints */
	LIST_FOR_EACH(file->checkpoint_list, i)
	{
		checkpoint = list_get(file->checkpoint_list, i);
		state_checkpoint_free(checkpoint);
	}
	list_free(file->checkpoint_list);

	/* Free categories */
	LIST_FOR_EACH(file->category_list, i)
	{
		category = list_get(file->category_list, i);
		state_category_free(category);
	}
	list_free(file->category_list);

	/* Free command table */
	HASH_TABLE_FOR_EACH(file->command_table, command_name, command)
		state_command_free(command);
	hash_table_free(file->command_table);

	/* Free current header trace line if any */
	if (file->header_trace_line)
		trace_line_free(file->header_trace_line);

	/* Free */
	free(file->unzipped_trace_file_name);
	free(file->checkpoint_file_name);
	free(file);
}


long long state_file_get_num_cycles(struct state_file_t *file)
{
	return file->num_cycles;
}


void state_file_create_checkpoints(struct state_file_t *file)
{
	struct trace_line_t *trace_line;

	long long last_checkpoint_cycle;
	long unzipped_trace_file_size;

	int num_trace_lines;

	/* Get unzipped trace file size */
	fseek(file->unzipped_trace_file, 0, SEEK_END);
	unzipped_trace_file_size = ftell(file->unzipped_trace_file);

	/* Initialize */
	last_checkpoint_cycle = -STATE_CHECKPOINT_INTERVAL;
	fseek(file->unzipped_trace_file, 0, SEEK_SET);

	/* Parse uncompressed trace file */
	num_trace_lines = 0;
	while ((trace_line = trace_line_create_from_file(file->unzipped_trace_file)))
	{
		struct state_checkpoint_t *checkpoint;
		struct state_command_t *state_command;

		char *command;

		long long cycle;

		/* Get command */
		command = trace_line_get_command(trace_line);

		/* New cycle command */
		if (!strcasecmp(command, "c"))
		{
			cycle = atoll(trace_line_get_symbol_value(trace_line, "clk"));
			while (cycle >= last_checkpoint_cycle + STATE_CHECKPOINT_INTERVAL)
			{
				last_checkpoint_cycle += STATE_CHECKPOINT_INTERVAL;
				checkpoint = state_checkpoint_create(last_checkpoint_cycle,
					trace_line_get_offset(trace_line),
					ftell(file->checkpoint_file));
				list_add(file->checkpoint_list, checkpoint);
				state_file_write_checkpoint(file);
			}
		}

		/* Process trace line if it is not a header  */
		else if (last_checkpoint_cycle >= 0)
		{
			state_command = hash_table_get(file->command_table, command);
			if (!state_command)
				fatal("%s: unknown command '%s'", __FUNCTION__, command);
			state_command->process_trace_line_func(state_command->user_data, trace_line);
		}

		/* Progress */
		num_trace_lines++;
		if (num_trace_lines % STATE_PROGRESS_INTERVAL == 1)
		{
			printf("Creating checkpoints (%.1fMB, %.1f%%)   \r",
				ftell(file->checkpoint_file) / 1.048e6,
				unzipped_trace_file_size ?
				(double) ftell(file->unzipped_trace_file) * 100.0 /
				unzipped_trace_file_size : 0.0);
			fflush(stdout);
		}

		/* Free trace line */
		trace_line_free(trace_line);
	}

	/* Progress */
	printf("Creating checkpoints (%.1fMB, 100%%)   \n",
		ftell(file->checkpoint_file) / 1.048e6);
	fflush(stdout);

	/* Load first checkpoint */
	state_file_read_checkpoint(file, 0);
}


void state_file_new_category(struct state_file_t *file, char *name,
	state_file_read_checkpoint_func_t read_checkpoint_func,
	state_file_write_checkpoint_func_t write_checkpoint_func,
	state_file_refresh_func_t refresh_func,
	void *user_data)
{
	struct state_category_t *category;

	category = state_category_create(name, read_checkpoint_func,
		write_checkpoint_func, refresh_func, user_data);
	list_add(file->category_list, category);
}


void state_file_new_command(struct state_file_t *file, char *command_name,
	state_file_process_trace_line_func_t process_trace_line_func,
	void *user_data)
{
	struct state_command_t *command;

	command = state_command_create(process_trace_line_func, user_data);
	if (!hash_table_insert(file->command_table, command_name, command))
		fatal("%s: cannot insert command", __FUNCTION__);
}


struct trace_line_t *state_file_header_first(struct state_file_t *file)
{
	file->header_trace_line_offset = 0;
	return state_file_header_next(file);
}


struct trace_line_t *state_file_header_next(struct state_file_t *file)
{
	struct trace_line_t *trace_line;

	/* Free current header trace line if any */
	if (file->header_trace_line)
	{
		trace_line_free(file->header_trace_line);
		file->header_trace_line = NULL;
	}

	/* No more header trace lines */
	if (file->header_trace_line_offset < 0)
		return NULL;

	/* Read trace line */
	fseek(file->unzipped_trace_file, file->header_trace_line_offset, SEEK_SET);
	trace_line = trace_line_create_from_file(file->unzipped_trace_file);
	if (!trace_line)
	{
		file->header_trace_line_offset = -1;
		return NULL;
	}

	/* Header is over */
	if (!strcmp(trace_line_get_command(trace_line), "c"))
	{
		trace_line_free(trace_line);
		file->header_trace_line_offset = -1;
		return NULL;
	}

	/* Save trace line and return */
	file->header_trace_line = trace_line;
	file->header_trace_line_offset = ftell(file->unzipped_trace_file);
	return trace_line;
}


/* Invoke the refresh call-back function for every category. */
void state_file_refresh(struct state_file_t *file)
{
	struct state_category_t *category;
	int i;

	LIST_FOR_EACH(file->category_list, i)
	{
		category = list_get(file->category_list, i);
		category->refresh_func(category->user_data);
	}
}


void state_file_go_to_cycle(struct state_file_t *file, long long cycle)
{
	long long checkpoint_cycle;
	int checkpoint_index;

	/* If we are already in this cycle, do nothing */
	if (file->cycle == cycle)
		return;

	/* Load a checkpoint */
	checkpoint_index = cycle / STATE_CHECKPOINT_INTERVAL;
	checkpoint_cycle = checkpoint_index * STATE_CHECKPOINT_INTERVAL;
	if (cycle < file->cycle || checkpoint_cycle > file->cycle)
		state_file_read_checkpoint(file, checkpoint_index);

	/* Go to cycle */
	for (;;)
	{
		struct trace_line_t *trace_line;
		struct state_command_t *state_command;
		long int unzipped_trace_file_pos;
		char *command;

		/* Read a trace line */
		unzipped_trace_file_pos = ftell(file->unzipped_trace_file);
		trace_line = trace_line_create_from_file(file->unzipped_trace_file);
		if (!trace_line)
			break;

		/* New cycle */
		command = trace_line_get_command(trace_line);
		if (!strcmp(command, "c"))
		{
			/* If we passed the target cycle, done */
			if (trace_line_get_symbol_value_long_long(trace_line, "clk") > cycle)
			{
				fseek(file->unzipped_trace_file, unzipped_trace_file_pos, SEEK_SET);
				trace_line_free(trace_line);
				break;
			}
		}
		else
		{
			/* Process trace line */
			state_command = hash_table_get(file->command_table, command);
			if (!state_command)
				fatal("%s: unknown command '%s'", __FUNCTION__, command);
			state_command->process_trace_line_func(state_command->user_data, trace_line);
		}

		/* Free trace line */
		trace_line_free(trace_line);
	}

	/* Cycle reached */
	file->cycle = cycle;
}
