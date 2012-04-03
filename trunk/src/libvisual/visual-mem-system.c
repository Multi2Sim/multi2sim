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

#include <visual-private.h>


/*
 * Trace Line Processing Functions
 */


static void visual_mem_system_set_transient_tag(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
}


static void visual_mem_system_set_block(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	struct visual_mod_t *mod;

	char *mod_name;
	char *state;

	int set;
	int way;

	unsigned int tag;

	/* Get fields */
	mod_name = trace_line_get_symbol_value(trace_line, "cache");
	set = trace_line_get_symbol_value_int(trace_line, "set");
	way = trace_line_get_symbol_value_int(trace_line, "way");
	tag = trace_line_get_symbol_value_hex(trace_line, "tag");
	state = trace_line_get_symbol_value(trace_line, "state");

	/* Get module */
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Set block */
	visual_mod_block_set(mod, set, way, tag, state);
}


static void visual_mem_system_set_sharer(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	char *mod_name;

	struct visual_mod_t *mod;

	int x;
	int y;
	int z;
	int sharer;

	/* Get module */
	mod_name = trace_line_get_symbol_value(trace_line, "dir");
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");
	sharer = trace_line_get_symbol_value_int(trace_line, "sharer");

	/* Set sharer */
	visual_mod_dir_entry_set_sharer(mod, x, y, z, sharer);
}


static void visual_mem_system_clear_sharer(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	char *mod_name;

	struct visual_mod_t *mod;

	int x;
	int y;
	int z;
	int sharer;

	/* Get module */
	mod_name = trace_line_get_symbol_value(trace_line, "dir");
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");
	sharer = trace_line_get_symbol_value_int(trace_line, "sharer");

	/* Set sharer */
	visual_mod_dir_entry_clear_sharer(mod, x, y, z, sharer);
}


static void visual_mem_system_clear_all_sharers(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	char *mod_name;

	struct visual_mod_t *mod;

	int x;
	int y;
	int z;

	/* Get module */
	mod_name = trace_line_get_symbol_value(trace_line, "dir");
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");

	/* Set sharer */
	visual_mod_dir_entry_clear_all_sharers(mod, x, y, z);
}


static void visual_mem_system_set_owner(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	char *mod_name;

	struct visual_mod_t *mod;

	int x;
	int y;
	int z;
	int owner;

	/* Get module */
	mod_name = trace_line_get_symbol_value(trace_line, "dir");
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");
	owner = trace_line_get_symbol_value_int(trace_line, "owner");

	/* Set sharer */
	visual_mod_dir_entry_set_owner(mod, x, y, z, owner);
}


static void visual_mem_system_new_access(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	char *name;
	char *state;

	struct visual_mod_access_t *access;

	/* Read fields */
	name = trace_line_get_symbol_value(trace_line, "name");
	state = trace_line_get_symbol_value(trace_line, "state");

	/* Create new access */
	access = visual_mod_access_create(name);
	visual_mod_access_set_state(access, state);

	/* Add access to list */
	hash_table_insert(visual_mem_system->access_table, access->name, access);
}


static void visual_mem_system_end_access(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	struct visual_mod_access_t *access;

	char *name;

	/* Read fields */
	name = trace_line_get_symbol_value(trace_line, "name");

	/* Find access */
	access = hash_table_remove(visual_mem_system->access_table, name);
	if (!access)
		panic("%s: access not found", __FUNCTION__);

	/* Free access */
	visual_mod_access_free(access);
}


static void visual_mem_system_new_access_mod(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	struct visual_mod_access_t *access;
	struct visual_mod_t *mod;

	char *mod_name;
	char *access_name;

	/* Read fields */
	mod_name = trace_line_get_symbol_value(trace_line, "mod");
	access_name = trace_line_get_symbol_value(trace_line, "access");

	/* Module */
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Create new access and add to list */
	access = visual_mod_access_create(access_name);
	hash_table_insert(mod->access_table, access->name, access);
}


static void visual_mem_system_end_access_mod(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	struct visual_mod_access_t *access;
	struct visual_mod_t *mod;

	char *mod_name;
	char *access_name;

	/* Read fields */
	mod_name = trace_line_get_symbol_value(trace_line, "mod");
	access_name = trace_line_get_symbol_value(trace_line, "access");

	/* Module */
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, mod_name);

	/* Remove access */
	access = hash_table_remove(mod->access_table, access_name);
	if (!access)
		panic("%s: %s: access not found", __FUNCTION__, access_name);

	/* Free access */
	visual_mod_access_free(access);
}


static void visual_mem_system_new_access_block(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	struct visual_mod_access_t *access;
	struct visual_mod_t *mod;

	char *mod_name;
	char *access_name;

	int set;
	int way;

	/* Read fields */
	mod_name = trace_line_get_symbol_value(trace_line, "cache");
	access_name = trace_line_get_symbol_value(trace_line, "access");
	set = trace_line_get_symbol_value_int(trace_line, "set");
	way = trace_line_get_symbol_value_int(trace_line, "way");

	/* Module */
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, mod_name);

	/* Create access and add to cache block */
	access = visual_mod_access_create(access_name);
	visual_mod_add_access(mod, set, way, access);
}


static void visual_mem_system_end_access_block(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	struct visual_mod_access_t *access;
	struct visual_mod_t *mod;

	char *mod_name;
	char *access_name;

	int set;
	int way;

	/* Read fields */
	mod_name = trace_line_get_symbol_value(trace_line, "cache");
	access_name = trace_line_get_symbol_value(trace_line, "access");
	set = trace_line_get_symbol_value_int(trace_line, "set");
	way = trace_line_get_symbol_value_int(trace_line, "way");

	/* Cache */
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, mod_name);

	/* Remove access */
	access = visual_mod_remove_access(mod, set, way, access_name);
	if (!access)
		panic("%s: %s: invalid access", __FUNCTION__, access_name);

	/* Free access */
	visual_mod_access_free(access);
}


static void visual_mem_system_access(struct visual_mem_system_t *system,
	struct trace_line_t *trace_line)
{
	struct visual_mod_access_t *access;

	char *name;
	char *state;

	/* Read fields */
	name = trace_line_get_symbol_value(trace_line, "name");
	state = trace_line_get_symbol_value(trace_line, "state");

	/* Find access */
	access = hash_table_get(visual_mem_system->access_table, name);
	if (!access)
		panic("%s: %s: access not found", __FUNCTION__, name);

	/* Update access */
	visual_mod_access_set_state(access, state);
}


static void visual_mem_system_read_checkpoint(struct visual_mem_system_t *system, FILE *f)
{
	char mod_name[MAX_STRING_SIZE];

	struct visual_mod_t *mod;
	struct visual_mod_access_t *access;

	char *access_name;

	int num_accesses;
	int count;
	int i;

	/* Empty access list */
	HASH_TABLE_FOR_EACH(visual_mem_system->access_table, access_name, access)
		visual_mod_access_free(access);
	hash_table_clear(visual_mem_system->access_table);

	/* Read number of accesses */
	count = fread(&num_accesses, 1, 4, f);
	if (count != 4)
		fatal("%s: error reading from checkpoint", __FUNCTION__);

	/* Read accesses */
	for (i = 0; i < num_accesses; i++)
	{
		access = visual_mod_access_create(NULL);
		visual_mod_access_read_checkpoint(access, f);
		hash_table_insert(visual_mem_system->access_table, access->name, access);
	}

	/* Read modules */
	for (i = 0; i < hash_table_count(visual_mem_system->mod_table); i++)
	{
		/* Get module */
		str_read_from_file(f, mod_name, sizeof mod_name);
		mod = hash_table_get(visual_mem_system->mod_table, mod_name);
		if (!mod)
			panic("%s: %s: invalid module name", __FUNCTION__, mod_name);

		/* Read module checkpoint */
		visual_mod_read_checkpoint(mod, f);
	}
}


static void visual_mem_system_write_checkpoint(struct visual_mem_system_t *system, FILE *f)
{
	struct visual_mod_t *mod;
	struct visual_mod_access_t *access;

	char *mod_name;
	char *access_name;

	int num_accesses;
	int count;

	/* Write number of accesses */
	num_accesses = hash_table_count(visual_mem_system->access_table);
	count = fwrite(&num_accesses, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

	/* Write accesses */
	HASH_TABLE_FOR_EACH(visual_mem_system->access_table, access_name, access)
		visual_mod_access_write_checkpoint(access, f);

	/* Write modules */
	HASH_TABLE_FOR_EACH(visual_mem_system->mod_table, mod_name, mod)
	{
		str_write_to_file(f, mod->name);
		visual_mod_write_checkpoint(mod, f);
	}
}



/*
 * Public Functions
 */


struct visual_mem_system_t *visual_mem_system;


void visual_mem_system_init(void)
{
	struct trace_line_t *trace_line;

	/* State file */
	state_file_new_category(visual_state_file, "Memory hierarchy",
		(state_file_read_checkpoint_func_t) visual_mem_system_read_checkpoint,
		(state_file_write_checkpoint_func_t) visual_mem_system_write_checkpoint,
		NULL, visual_mem_system);
	/* Replace with (state_file_refresh_func_t) visual_mem_system_widget_refresh */

	state_file_new_command(visual_state_file, "mem.set_transient_tag",
		(state_file_process_trace_line_func_t) visual_mem_system_set_transient_tag,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.set_block",
		(state_file_process_trace_line_func_t) visual_mem_system_set_block,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.set_sharer",
		(state_file_process_trace_line_func_t) visual_mem_system_set_sharer,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.clear_sharer",
		(state_file_process_trace_line_func_t) visual_mem_system_clear_sharer,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.clear_all_sharers",
		(state_file_process_trace_line_func_t) visual_mem_system_clear_all_sharers,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.set_owner",
		(state_file_process_trace_line_func_t) visual_mem_system_set_owner,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.new_access",
		(state_file_process_trace_line_func_t) visual_mem_system_new_access,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.end_access",
		(state_file_process_trace_line_func_t) visual_mem_system_end_access,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.new_access_mod",
		(state_file_process_trace_line_func_t) visual_mem_system_new_access_mod,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.end_access_mod",
		(state_file_process_trace_line_func_t) visual_mem_system_end_access_mod,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.new_access_block",
		(state_file_process_trace_line_func_t) visual_mem_system_new_access_block,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.end_access_block",
		(state_file_process_trace_line_func_t) visual_mem_system_end_access_block,
		visual_mem_system);
	state_file_new_command(visual_state_file, "mem.access",
		(state_file_process_trace_line_func_t) visual_mem_system_access,
		visual_mem_system);

	/* Allocate */
	visual_mem_system = calloc(1, sizeof(struct visual_mem_system_t));
	if (!visual_mem_system)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	visual_mem_system->mod_table = hash_table_create(0, FALSE);
	visual_mem_system->net_table = hash_table_create(0, FALSE);
	visual_mem_system->access_table = hash_table_create(0, FALSE);
	visual_mem_system->mod_level_list = list_create();

	/* Parse header in state file */
	STATE_FILE_FOR_EACH_HEADER(visual_state_file, trace_line)
	{
		char *command;

		/* Get command */
		command = trace_line_get_command(trace_line);
		assert(strcmp(command, "c"));

		if (!strcmp(command, "mem.new_mod"))
		{
			struct visual_mod_t *mod;
			struct list_t *mod_level;

			/* Create module */
			mod = visual_mod_create(trace_line);
			hash_table_insert(visual_mem_system->mod_table, mod->name, mod);
			if (mod->level < 1)
				panic("%s: %s: invalid level (%d)", __FUNCTION__, mod->name, mod->level);

			/* Add to level list */
			while (visual_mem_system->mod_level_list->count < mod->level)
				list_add(visual_mem_system->mod_level_list, list_create());
			mod_level = list_get(visual_mem_system->mod_level_list, mod->level - 1);
			list_add(mod_level, mod);
		}
		else if (!strcmp(command, "mem.new_net"))
		{
			struct visual_net_t *net;

			net = visual_net_create(trace_line);
			hash_table_insert(visual_mem_system->net_table, net->name, net);
		}
	}
}


void visual_mem_system_done(void)
{
	struct visual_mod_t *mod;
	struct visual_net_t *net;
	struct visual_mod_access_t *access;

	char *mod_name;
	char *net_name;
	char *access_name;

	int i;

	/* Free modules */
	HASH_TABLE_FOR_EACH(visual_mem_system->mod_table, mod_name, mod)
		visual_mod_free(mod);
	hash_table_free(visual_mem_system->mod_table);

	/* Free networks */
	HASH_TABLE_FOR_EACH(visual_mem_system->net_table, net_name, net)
		visual_net_free(net);
	hash_table_free(visual_mem_system->net_table);

	/* Free accesses */
	HASH_TABLE_FOR_EACH(visual_mem_system->access_table, access_name, access)
		visual_mod_access_free(access);
	hash_table_free(visual_mem_system->access_table);

	/* Free levels */
	LIST_FOR_EACH(visual_mem_system->mod_level_list, i)
		list_free(list_get(visual_mem_system->mod_level_list, i));
	list_free(visual_mem_system->mod_level_list);

	/* Rest */
	free(visual_mem_system);
}


/* Return the access name in the current cycle set in the state file */
void visual_mem_system_get_access_name_long(char *access_name, char *buf, int size)
{
	struct visual_mod_access_t *access;

	/* Look for access */
	access = hash_table_get(visual_mem_system->access_table, access_name);
	if (!access)
		panic("%s: %s: invalid access", __FUNCTION__, access_name);

	/* Name */
	str_printf(&buf, &size, "<b>%s</b>", access->name);

	/* State */
	if (access->state && *access->state)
		str_printf(&buf, &size, " (%s:%lld)", access->state,
			state_file_get_cycle(visual_state_file) - access->state_update_cycle);
}


void visual_mem_system_get_access_name_short(char *access_name, char *buf, int size)
{
	struct visual_mod_access_t *access;

	/* Look for access */
	access = hash_table_get(visual_mem_system->access_table, access_name);
	if (!access)
		panic("%s: %s: invalid access", __FUNCTION__, access_name);

	/* Name */
	str_printf(&buf, &size, "%s", access->name);
}


void visual_mem_system_get_access_desc(char *access_name, char *buf, int size)
{
	char *title_format_begin = "<span color=\"blue\"><b>";
	char *title_format_end = "</b></span>";

	struct visual_mod_access_t *access;

	/* Look for access */
	access = hash_table_get(visual_mem_system->access_table, access_name);
	if (!access)
		panic("%s: %s: invalid access", __FUNCTION__, access_name);

	/* Title */
	str_printf(&buf, &size, "%sDescription for access %s%s\n\n",
		title_format_begin, access->name, title_format_end);

	/* Fields */
	str_printf(&buf, &size, "%sName:%s %s\n", title_format_begin,
		title_format_end, access->name);

	/* State */
	if (access->state && *access->state)
	{
		str_printf(&buf, &size, "%sState:%s %s\n", title_format_begin,
			title_format_end, access->state);
		str_printf(&buf, &size, "%sState update cycle:%s %lld (%lld cycles ago)\n",
			title_format_begin, title_format_end, access->state_update_cycle,
			state_file_get_cycle(visual_state_file) - access->state_update_cycle);
	}
}
