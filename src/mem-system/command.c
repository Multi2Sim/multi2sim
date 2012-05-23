/*
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


#include <mem-system.h>


int EV_MEM_SYSTEM_COMMAND;
int EV_MEM_SYSTEM_END_COMMAND;



/*
 * Private Functions
 */


static void mem_system_command_expect(struct list_t *token_list, char *command_line)
{
	if (!list_count(token_list))
		fatal("%s: unexpected end of line.\n\t> %s",
			__FUNCTION__, command_line);
}


static void mem_system_command_end(struct list_t *token_list, char *command_line)
{
	if (list_count(token_list))
		fatal("%s: %s: end of line expected.\n\t> %s",
			__FUNCTION__, str_token_list_first(token_list), command_line);
}


static unsigned int mem_system_command_get_hex(struct list_t *token_list,
	char *command_line)
{
	unsigned int value;

	/* Get value */
	mem_system_command_expect(token_list, command_line);
	if (sscanf(str_token_list_first(token_list), "0x%x", &value) != 1)
		fatal("%s: %s: invalid hex value.\n\t> %s",
			__FUNCTION__, str_token_list_first(token_list),
			command_line);
	str_token_list_shift(token_list);

	/* Return */
	return value;
}


static void mem_system_command_get_string(struct list_t *token_list,
	char *command_line, char *buf, int size)
{
	mem_system_command_expect(token_list, command_line);
	snprintf(buf, size, "%s", str_token_list_first(token_list));
	str_token_list_shift(token_list);
}


#if 0
static long long mem_system_command_get_long_long(struct list_t *token_list,
	char *command_line)
{
	long long value;

	/* Get value */
	mem_system_command_expect(token_list, command_line);
	if (sscanf(str_token_list_first(token_list), "%lld", &value) != 1)
		fatal("%s: %s: invalid integer value.\n\t> %s",
			__FUNCTION__, str_token_list_first(token_list),
			command_line);
	str_token_list_shift(token_list);

	/* Return */
	return value;
}
#endif


static struct mod_t *mem_system_command_get_mod(struct list_t *token_list,
	char *command_line)
{
	struct mod_t *mod;

	char *mod_name;

	/* Find module */
	mem_system_command_expect(token_list, command_line);
	mod_name = list_get(token_list, 0);
	mod = mem_system_get_mod(mod_name);
	if (!mod)
		fatal("%s: %s: invalid module name.\n\t> %s",
			__FUNCTION__, mod_name, command_line);

	/* Next token */
	str_token_list_shift(token_list);
	return mod;
}


static void mem_system_command_get_set_way(struct list_t *token_list,
	char *command_line, struct mod_t *mod, unsigned int *set_ptr,
	unsigned int *way_ptr)
{
	unsigned int set;
	unsigned int way;

	/* Get set */
	mem_system_command_expect(token_list, command_line);
	set = atoi(str_token_list_first(token_list));
	str_token_list_shift(token_list);

	/* Get way */
	mem_system_command_expect(token_list, command_line);
	way = atoi(str_token_list_first(token_list));
	str_token_list_shift(token_list);

	/* Check valid set/way */
	if (!IN_RANGE(set, 0, mod->cache->num_sets - 1))
		fatal("%s: %d: invalid set.\n\t> %s",
			__FUNCTION__, set, command_line);
	if (!IN_RANGE(way, 0, mod->cache->assoc - 1))
		fatal("%s: %d: invalid way.\n\t> %s",
			__FUNCTION__, set, command_line);

	/* Return */
	*set_ptr = set;
	*way_ptr = way;
}


static int mem_system_command_get_sub_block(struct list_t *token_list,
	char *command_line, struct mod_t *mod, unsigned int set,
	unsigned int way)
{
	int sub_block;

	/* Get sub-block */
	mem_system_command_expect(token_list, command_line);
	sub_block = atoi(str_token_list_first(token_list));
	str_token_list_shift(token_list);

	/* Check that module has a directory */
	if (!mod->dir)
		fatal("%s: %s: module lacks a directory.\n\t> %s",
			__FUNCTION__, mod->name, command_line);

	/* Check valid sub-block */
	if (!IN_RANGE(sub_block, 0, mod->dir->zsize - 1))
		fatal("%s: %d: invalid sub-block.\n\t> %s",
			__FUNCTION__, sub_block, command_line);

	/* Return */
	return sub_block;
}


static int mem_system_command_get_state(struct list_t *token_list,
	char *command_line)
{
	int state;

	/* Get state */
	mem_system_command_expect(token_list, command_line);
	state = map_string_case(&cache_block_state_map, str_token_list_first(token_list));
	if (!state && strcasecmp(str_token_list_first(token_list), "I"))
		fatal("%s: invalid state.\n\t> %s",
			__FUNCTION__, command_line);

	/* Return */
	str_token_list_shift(token_list);
	return state;
}


static enum mod_access_kind_t mem_system_command_get_mod_access(struct list_t *token_list,
	char *command_line)
{
	char mod_access_name[MAX_STRING_SIZE];
	int mod_access;

	/* Get access */
	mem_system_command_expect(token_list, command_line);
	snprintf(mod_access_name, sizeof mod_access_name, "%s", str_token_list_first(token_list));

	/* Decode access */
	mod_access = map_string_case(&mod_access_kind_map, mod_access_name);
	if (!mod_access)
		fatal("%s: %s: invalid access.\n\t> %s",
			__FUNCTION__, mod_access_name, command_line);
	
	/* Return */
	str_token_list_shift(token_list);
	return mod_access;
}




/*
 * Public Functions
 */


/* Event handler for EV_MEM_SYSTEM_COMMAND.
 * The event data is a string of type 'char *' that needs to be deallocated
 * after processing this event. */
void mem_system_command_handler(int event, void *data)
{
	struct list_t *token_list;

	char *command_line = data;
	char command[MAX_STRING_SIZE];

	/* Get command */
	str_token(command, sizeof command, command_line, 0, " ");
	if (!command[0])
		fatal("%s: invalid command syntax.\n\t> %s",
			__FUNCTION__, command_line);

	/* Commands that need to be processed at the end of the simulation
	 * are ignored here. */
	if (!strcasecmp(command, "CheckBlock"))
	{
		esim_schedule_end_event(EV_MEM_SYSTEM_END_COMMAND, data);
		return;
	}

	/* Split command in tokens, skip command */
	token_list = str_token_list_create(command_line, " ");
	assert(list_count(token_list));
	str_token_list_shift(token_list);

	/* Command 'SetBlock' */
	if (!strcasecmp(command, "SetBlock"))
	{
		struct mod_t *mod;

		unsigned int set;
		unsigned int way;
		unsigned int tag;

		unsigned int set_check;
		unsigned int tag_check;

		int state;

		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		tag = mem_system_command_get_hex(token_list, command_line);
		state = mem_system_command_get_state(token_list, command_line);
		mem_system_command_end(token_list, command_line);

		/* Check that module serves address */
		if (!mod_serves_address(mod, tag))
			fatal("%s: %s: module does not serve address 0x%x.\n\t> %s",
				__FUNCTION__, mod->name, tag, command_line);

		/* Check that tag goes to specified set */
		mod_find_block(mod, tag, &set_check, NULL, &tag_check, NULL);
		if (set != set_check)
			fatal("%s: %s: tag 0x%x belongs to set %d.\n\t> %s",
				__FUNCTION__, mod->name, tag, set_check, command_line);
		if (tag != tag_check)
			fatal("%s: %s: tag should be multiple of block size.\n\t> %s",
				__FUNCTION__, mod->name, command_line);

		/* Set tag */
		cache_set_block(mod->cache, set, way, tag, state);
	}

	/* Command 'SetOwner' */
	else if (!strcasecmp(command, "SetOwner"))
	{
		struct mod_t *mod;
		struct mod_t *owner;

		unsigned int set;
		unsigned int way;

		int sub_block;

		/* Get fields */
		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		sub_block = mem_system_command_get_sub_block(token_list, command_line, mod, set, way);
		owner = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_end(token_list, command_line);

		/* Check that owner is an immediate higher-level module */
		if (owner->low_net != mod->high_net || !owner->low_net)
			fatal("%s: %s is not a higher-level module of %s.\n\t> %s",
				__FUNCTION__, owner->name, mod->name, command_line);

		/* Set owner */
		dir_entry_set_owner(mod->dir, set, way, sub_block, owner->low_net_node->index);
	}

	/* Command 'SetSharers' */
	else if (!strcasecmp(command, "SetSharers"))
	{
		struct mod_t *mod;
		struct mod_t *sharer;

		unsigned int set;
		unsigned int way;

		int sub_block;

		/* Get first fields */
		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		sub_block = mem_system_command_get_sub_block(token_list, command_line, mod, set, way);

		/* Get sharers */
		mem_system_command_expect(token_list, command_line);
		while (list_count(token_list))
		{
			/* Get sharer */
			sharer = mem_system_command_get_mod(token_list, command_line);

			/* Check that sharer is an immediate higher-level module */
			if (sharer->low_net != mod->high_net || !sharer->low_net)
				fatal("%s: %s is not a higher-level module of %s.\n\t> %s",
					__FUNCTION__, sharer->name, mod->name, command_line);

			/* Set sharer */
			dir_entry_set_sharer(mod->dir, set, way, sub_block, sharer->low_net_node->index);
		}
	}

	/* Command 'Access' */
	else if (!strcasecmp(command, "Access"))
	{
		struct mod_t *mod;
		enum mod_access_kind_t access_kind;
		unsigned int addr;

		mod = mem_system_command_get_mod(token_list, command_line);
		access_kind = mem_system_command_get_mod_access(token_list, command_line);
		addr = mem_system_command_get_hex(token_list, command_line);

		/* Access module */
		mod_access(mod, access_kind, addr, NULL, NULL, NULL);
	}

	/* Command not supported */
	else
		fatal("%s: %s: invalid command.\n\t> %s",
			__FUNCTION__, command, command_line);

	/* Free command */
	free(command_line);
	str_token_list_free(token_list);
}


/* Event handler for EV_MEM_SYSTEM_END_COMMAND.
 * The event data is a string of type 'char *' that needs to be deallocated
 * after processing this event. */
void mem_system_end_command_handler(int event, void *data)
{
	char *command_line = data;
	char command[MAX_STRING_SIZE];

	struct list_t *token_list;

	/* Split command in tokens, skip command */
	token_list = str_token_list_create(command_line, " ");
	assert(list_count(token_list));

	/* Get command */
	mem_system_command_get_string(token_list, command_line, command, sizeof command);

	/* Command 'SetBlock' */
	if (!strcasecmp(command, "CheckBlock"))
	{
		struct mod_t *mod;

		unsigned int set;
		unsigned int way;
		unsigned int tag;
		unsigned int tag_check;

		int state;
		int state_check;

		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		tag = mem_system_command_get_hex(token_list, command_line);
		state = mem_system_command_get_state(token_list, command_line);
		mem_system_command_end(token_list, command_line);

		/* Check that module serves address */
		if (!mod_serves_address(mod, tag))
			fatal("%s: %s: module does not serve address 0x%x.\n\t> %s",
				__FUNCTION__, mod->name, tag, command_line);

		/* Check */
		cache_get_block(mod->cache, set, way, &tag_check, &state_check);
		if (tag != tag_check)
			fatal("%s: %s: set %d, way %d, tag mismatch",
				__FUNCTION__, mod->name, set, way);
		if (state != state_check)
			fatal("%s: %s: set %d, way %d, state mismatch",
				__FUNCTION__, mod->name, set, way);
	}

	/* Free command */
	free(command_line);
	str_token_list_free(token_list);
}
