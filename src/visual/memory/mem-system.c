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

#include <assert.h>
#include <gtk/gtk.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>

#include "mem-system.h"
#include "mod.h"
#include "mod-access.h"
#include "net.h"


/*
 * Trace Line Processing Functions
 */


static void vi_mem_system_set_block(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	struct vi_mod_t *mod;

	char *mod_name;
	char *state;

	int set;
	int way;

	unsigned int tag;

	/* Get fields */
	mod_name = vi_trace_line_get_symbol(trace_line, "cache");
	set = vi_trace_line_get_symbol_int(trace_line, "set");
	way = vi_trace_line_get_symbol_int(trace_line, "way");
	tag = vi_trace_line_get_symbol_hex(trace_line, "tag");
	state = vi_trace_line_get_symbol(trace_line, "state");

	/* Get module */
	mod = hash_table_get(vi_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Set block */
	vi_mod_block_set(mod, set, way, tag, state);
}


static void vi_mem_system_set_sharer(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	char *mod_name;

	struct vi_mod_t *mod;

	int x;
	int y;
	int z;
	int sharer;

	/* Get module */
	mod_name = vi_trace_line_get_symbol(trace_line, "dir");
	mod = hash_table_get(vi_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = vi_trace_line_get_symbol_int(trace_line, "x");
	y = vi_trace_line_get_symbol_int(trace_line, "y");
	z = vi_trace_line_get_symbol_int(trace_line, "z");
	sharer = vi_trace_line_get_symbol_int(trace_line, "sharer");

	/* Set sharer */
	vi_mod_dir_entry_set_sharer(mod, x, y, z, sharer);
}


static void vi_mem_system_clear_sharer(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	char *mod_name;

	struct vi_mod_t *mod;

	int x;
	int y;
	int z;
	int sharer;

	/* Get module */
	mod_name = vi_trace_line_get_symbol(trace_line, "dir");
	mod = hash_table_get(vi_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = vi_trace_line_get_symbol_int(trace_line, "x");
	y = vi_trace_line_get_symbol_int(trace_line, "y");
	z = vi_trace_line_get_symbol_int(trace_line, "z");
	sharer = vi_trace_line_get_symbol_int(trace_line, "sharer");

	/* Set sharer */
	vi_mod_dir_entry_clear_sharer(mod, x, y, z, sharer);
}


static void vi_mem_system_clear_all_sharers(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	char *mod_name;

	struct vi_mod_t *mod;

	int x;
	int y;
	int z;

	/* Get module */
	mod_name = vi_trace_line_get_symbol(trace_line, "dir");
	mod = hash_table_get(vi_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = vi_trace_line_get_symbol_int(trace_line, "x");
	y = vi_trace_line_get_symbol_int(trace_line, "y");
	z = vi_trace_line_get_symbol_int(trace_line, "z");

	/* Set sharer */
	vi_mod_dir_entry_clear_all_sharers(mod, x, y, z);
}


static void vi_mem_system_set_owner(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	char *mod_name;

	struct vi_mod_t *mod;

	int x;
	int y;
	int z;
	int owner;

	/* Get module */
	mod_name = vi_trace_line_get_symbol(trace_line, "dir");
	mod = hash_table_get(vi_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: invalid module name '%s'", __FUNCTION__, mod_name);

	/* Directory entry and sharer */
	x = vi_trace_line_get_symbol_int(trace_line, "x");
	y = vi_trace_line_get_symbol_int(trace_line, "y");
	z = vi_trace_line_get_symbol_int(trace_line, "z");
	owner = vi_trace_line_get_symbol_int(trace_line, "owner");

	/* Set sharer */
	vi_mod_dir_entry_set_owner(mod, x, y, z, owner);
}


static void vi_mem_system_new_access(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	char *name;
	char *state;

	struct vi_mod_access_t *access;

	unsigned int address;

	/* Read fields */
	name = vi_trace_line_get_symbol(trace_line, "name");
	state = vi_trace_line_get_symbol(trace_line, "state");
	address = vi_trace_line_get_symbol_hex(trace_line, "addr");

	/* Create new access */
	access = vi_mod_access_create(name, address);
	vi_mod_access_set_state(access, state);

	/* Add access to list */
	hash_table_insert(vi_mem_system->access_table, access->name, access);
}


static void vi_mem_system_end_access(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	struct vi_mod_access_t *access;

	char *name;

	/* Read fields */
	name = vi_trace_line_get_symbol(trace_line, "name");

	/* Find access */
	access = hash_table_remove(vi_mem_system->access_table, name);
	if (!access)
		panic("%s: access not found", __FUNCTION__);

	/* Free access */
	vi_mod_access_free(access);
}


static void vi_mem_system_new_access_block(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	struct vi_mod_access_t *access;
	struct vi_mod_t *mod;

	char *mod_name;
	char *access_name;

	int set;
	int way;

	/* Read fields */
	mod_name = vi_trace_line_get_symbol(trace_line, "cache");
	access_name = vi_trace_line_get_symbol(trace_line, "access");
	set = vi_trace_line_get_symbol_int(trace_line, "set");
	way = vi_trace_line_get_symbol_int(trace_line, "way");

	/* Module */
	mod = hash_table_get(vi_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, mod_name);

	/* Check if access is already in module. If it is there, increase
	 * the number of links. Otherwise, create it. */
	access = hash_table_get(mod->access_table, access_name);
	if (access)
	{
		access->num_links++;
	}
	else
	{
		access = vi_mod_access_create(access_name, 0);
		hash_table_insert(mod->access_table, access_name, access);
	}

	/* Add access to block */
	access = vi_mod_access_create(access_name, 0);
	vi_mod_add_access(mod, set, way, access);
}


static void vi_mem_system_end_access_block(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	struct vi_mod_access_t *access;
	struct vi_mod_t *mod;

	char *mod_name;
	char *access_name;

	int set;
	int way;

	/* Read fields */
	mod_name = vi_trace_line_get_symbol(trace_line, "cache");
	access_name = vi_trace_line_get_symbol(trace_line, "access");
	set = vi_trace_line_get_symbol_int(trace_line, "set");
	way = vi_trace_line_get_symbol_int(trace_line, "way");

	/* Module */
	mod = hash_table_get(vi_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, mod_name);

	/* Find access in module */
	access = hash_table_get(mod->access_table, access_name);
	if (!access)
		panic("%s: module %s: access %s: invalid access",
			__FUNCTION__, mod_name, access_name);

	/* If access has more than one reference, just decrease it.
	 * Otherwise, remove it. */
	assert(access->num_links >= 0);
	if (access->num_links)
		access->num_links--;
	else
	{
		access = hash_table_remove(mod->access_table, access_name);
		vi_mod_access_free(access);
	}

	/* Remove access from block */
	access = vi_mod_remove_access(mod, set, way, access_name);
	if (!access)
		panic("%s: %s: invalid access", __FUNCTION__, access_name);

	/* Free access */
	vi_mod_access_free(access);
}


static void vi_mem_system_access(struct vi_mem_system_t *system,
	struct vi_trace_line_t *trace_line)
{
	struct vi_mod_access_t *access;

	char *name;
	char *state;

	/* Read fields */
	name = vi_trace_line_get_symbol(trace_line, "name");
	state = vi_trace_line_get_symbol(trace_line, "state");

	/* Find access */
	access = hash_table_get(vi_mem_system->access_table, name);
	if (!access)
		panic("%s: %s: access not found", __FUNCTION__, name);

	/* Update access */
	vi_mod_access_set_state(access, state);
}


static void vi_mem_system_read_checkpoint(struct vi_mem_system_t *system, FILE *f)
{
	char mod_name[MAX_STRING_SIZE];

	struct vi_mod_t *mod;
	struct vi_mod_access_t *access;

	char *access_name;

	int num_accesses;
	int count;
	int i;

	/* Empty access list */
	HASH_TABLE_FOR_EACH(vi_mem_system->access_table, access_name, access)
		vi_mod_access_free(access);
	hash_table_clear(vi_mem_system->access_table);

	/* Read number of accesses */
	count = fread(&num_accesses, 1, 4, f);
	if (count != 4)
		fatal("%s: error reading from checkpoint", __FUNCTION__);

	/* Read accesses */
	for (i = 0; i < num_accesses; i++)
	{
		access = vi_mod_access_create(NULL, 0);
		vi_mod_access_read_checkpoint(access, f);
		hash_table_insert(vi_mem_system->access_table, access->name, access);
	}

	/* Read modules */
	for (i = 0; i < hash_table_count(vi_mem_system->mod_table); i++)
	{
		/* Get module */
		str_read_from_file(f, mod_name, sizeof mod_name);
		mod = hash_table_get(vi_mem_system->mod_table, mod_name);
		if (!mod)
			panic("%s: %s: invalid module name", __FUNCTION__, mod_name);

		/* Read module checkpoint */
		vi_mod_read_checkpoint(mod, f);
	}
}


static void vi_mem_system_write_checkpoint(struct vi_mem_system_t *system, FILE *f)
{
	struct vi_mod_t *mod;
	struct vi_mod_access_t *access;

	char *mod_name;
	char *access_name;

	int num_accesses;
	int count;

	/* Write number of accesses */
	num_accesses = hash_table_count(vi_mem_system->access_table);
	count = fwrite(&num_accesses, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

	/* Write accesses */
	HASH_TABLE_FOR_EACH(vi_mem_system->access_table, access_name, access)
		vi_mod_access_write_checkpoint(access, f);

	/* Write modules */
	HASH_TABLE_FOR_EACH(vi_mem_system->mod_table, mod_name, mod)
	{
		str_write_to_file(f, mod->name);
		vi_mod_write_checkpoint(mod, f);
	}
}



/*
 * Public Functions
 */


struct vi_mem_system_t *vi_mem_system;

/* Version of the memory system trace consumer. The major version must match with the
 * trace producer, while the minor version should be equal or higher. See
 * 'glut.c' for the version number assignment, and code modification policies.
 * See 'src/mem-system/config.c' for the trace producer version.
 */

#define VI_MEM_SYSTEM_TRACE_VERSION_MAJOR	1
#define VI_MEM_SYSTEM_TRACE_VERSION_MINOR	678

static char *err_vi_mem_system_trace_version =
	"\tThe memory system trace file has been created with an incompatible version\n"
	"\tof Multi2Sim. Please rerun the simulation with the same Multi2Sim\n"
	"\tversion used to visualize the trace.\n";

void vi_mem_system_init(void)
{
	struct vi_trace_line_t *trace_line;

	/* State file */
	vi_state_new_category("Memory hierarchy",
		(vi_state_read_checkpoint_func_t) vi_mem_system_read_checkpoint,
		(vi_state_write_checkpoint_func_t) vi_mem_system_write_checkpoint,
		vi_mem_system);

	/* Commands */
	vi_state_new_command("mem.set_block",
		(vi_state_process_trace_line_func_t) vi_mem_system_set_block,
		vi_mem_system);
	vi_state_new_command("mem.set_sharer",
		(vi_state_process_trace_line_func_t) vi_mem_system_set_sharer,
		vi_mem_system);
	vi_state_new_command("mem.clear_sharer",
		(vi_state_process_trace_line_func_t) vi_mem_system_clear_sharer,
		vi_mem_system);
	vi_state_new_command("mem.clear_all_sharers",
		(vi_state_process_trace_line_func_t) vi_mem_system_clear_all_sharers,
		vi_mem_system);
	vi_state_new_command("mem.set_owner",
		(vi_state_process_trace_line_func_t) vi_mem_system_set_owner,
		vi_mem_system);
	vi_state_new_command("mem.new_access",
		(vi_state_process_trace_line_func_t) vi_mem_system_new_access,
		vi_mem_system);
	vi_state_new_command("mem.end_access",
		(vi_state_process_trace_line_func_t) vi_mem_system_end_access,
		vi_mem_system);
	vi_state_new_command("mem.new_access_block",
		(vi_state_process_trace_line_func_t) vi_mem_system_new_access_block,
		vi_mem_system);
	vi_state_new_command("mem.end_access_block",
		(vi_state_process_trace_line_func_t) vi_mem_system_end_access_block,
		vi_mem_system);
	vi_state_new_command("mem.access",
		(vi_state_process_trace_line_func_t) vi_mem_system_access,
		vi_mem_system);

	/* Initialize */
	vi_mem_system = xcalloc(1, sizeof(struct vi_mem_system_t));
	vi_mem_system->mod_table = hash_table_create(0, FALSE);
	vi_mem_system->net_table = hash_table_create(0, FALSE);
	vi_mem_system->access_table = hash_table_create(0, FALSE);
	vi_mem_system->mod_level_list = list_create();

	/* Parse header in state file */
	VI_STATE_FOR_EACH_HEADER(trace_line)
	{
		char *command;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);
		assert(strcmp(command, "c"));

		if (!strcmp(command, "mem.init"))
		{
			char *version;

			int version_major = 0;
			int version_minor = 0;

			/* Check version compatibility */
			version = vi_trace_line_get_symbol(trace_line, "version");
			if (version)
				sscanf(version, "%d.%d", &version_major, &version_minor);
			if (version_major != VI_MEM_SYSTEM_TRACE_VERSION_MAJOR ||
				version_minor > VI_MEM_SYSTEM_TRACE_VERSION_MINOR)
				fatal("incompatible memory system trace version.\n"
					"\tTrace generation v. %d.%d / Trace consumer v. %d.%d\n%s",
					version_major, version_minor, VI_MEM_SYSTEM_TRACE_VERSION_MAJOR,
					VI_MEM_SYSTEM_TRACE_VERSION_MINOR, err_vi_mem_system_trace_version);
		}
		else if (!strcmp(command, "mem.new_mod"))
		{
			struct vi_mod_t *mod;
			struct list_t *mod_level;

			/* Create module */
			mod = vi_mod_create(trace_line);
			hash_table_insert(vi_mem_system->mod_table, mod->name, mod);
			if (mod->level < 1)
				panic("%s: %s: invalid level (%d)", __FUNCTION__, mod->name, mod->level);

			/* Add to level list */
			while (vi_mem_system->mod_level_list->count < mod->level)
				list_add(vi_mem_system->mod_level_list, list_create());
			mod_level = list_get(vi_mem_system->mod_level_list, mod->level - 1);
			list_add(mod_level, mod);
		}
		else if (!strcmp(command, "mem.new_net"))
		{
			struct vi_net_t *net;

			net = vi_net_create(trace_line);
			hash_table_insert(vi_mem_system->net_table, net->name, net);
		}
	}
}


void vi_mem_system_done(void)
{
	struct vi_mod_t *mod;
	struct vi_net_t *net;
	struct vi_mod_access_t *access;

	char *mod_name;
	char *net_name;
	char *access_name;

	int i;

	/* Free modules */
	HASH_TABLE_FOR_EACH(vi_mem_system->mod_table, mod_name, mod)
		vi_mod_free(mod);
	hash_table_free(vi_mem_system->mod_table);

	/* Free networks */
	HASH_TABLE_FOR_EACH(vi_mem_system->net_table, net_name, net)
		vi_net_free(net);
	hash_table_free(vi_mem_system->net_table);

	/* Free accesses */
	HASH_TABLE_FOR_EACH(vi_mem_system->access_table, access_name, access)
		vi_mod_access_free(access);
	hash_table_free(vi_mem_system->access_table);

	/* Free levels */
	LIST_FOR_EACH(vi_mem_system->mod_level_list, i)
		list_free(list_get(vi_mem_system->mod_level_list, i));
	list_free(vi_mem_system->mod_level_list);

	/* Rest */
	free(vi_mem_system);
}
