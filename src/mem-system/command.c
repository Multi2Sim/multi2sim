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


static struct mod_t *mem_system_command_get_mod(struct list_t *token_list,
	char *command_line)
{
	struct mod_t *mod;

	char *mod_name;

	/* Get module name */
	mem_system_command_expect(token_list, command_line);
	mod_name = str_token_list_first(token_list);

	/* Find module */
	if (!strcasecmp(mod_name, "None"))
		mod = NULL;
	else
	{
		mod = mem_system_get_mod(mod_name);
		if (!mod)
			fatal("%s: %s: invalid module name.\n\t> %s",
				__FUNCTION__, mod_name, command_line);
	}

	/* Return module */
	str_token_list_shift(token_list);
	return mod;
}


static void mem_system_command_get_set_way(struct list_t *token_list,
	char *command_line, struct mod_t *mod, unsigned int *set_ptr,
	unsigned int *way_ptr)
{
	unsigned int set;
	unsigned int way;

	/* Check valid module */
	if (!mod)
		fatal("%s: invalid module.\n\t> %s",
			__FUNCTION__, command_line);

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
	 * are ignored here. These are command prefixed with 'CheckXXX'. */
	if (!strncasecmp(command, "Check", 5))
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
		int owner_index;

		/* Get fields */
		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		sub_block = mem_system_command_get_sub_block(token_list, command_line, mod, set, way);
		owner = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_end(token_list, command_line);

		/* Check that owner is an immediate higher-level module */
		if (owner)
		{
			if (owner->low_net != mod->high_net || !owner->low_net)
				fatal("%s: %s is not a higher-level module of %s.\n\t> %s",
					__FUNCTION__, owner->name, mod->name, command_line);
		}

		/* Set owner */
		owner_index = owner ? owner->low_net_node->index : -1;
		dir_entry_set_owner(mod->dir, set, way, sub_block, owner_index);
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

	char msg[MAX_STRING_SIZE];
	char msg_detail[MAX_STRING_SIZE];

	char *msg_str = msg;
	int msg_size = sizeof msg;

	char *msg_detail_str = msg_detail;
	int msg_detail_size = sizeof msg_detail;

	int test_failed;

	struct list_t *token_list;

	/* Split command in tokens, skip command */
	token_list = str_token_list_create(command_line, " ");
	assert(list_count(token_list));

	/* Get command */
	mem_system_command_get_string(token_list, command_line, command, sizeof command);

	/* Messages */
	test_failed = 0;
	*msg_str = '\0';
	*msg_detail_str = '\0';

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

		/* Output */
		str_printf(&msg_str, &msg_size,
			"check module %s, set %d, way %d - state %s, tag 0x%x",
			mod->name, set, way, map_value(&cache_block_state_map, state), tag);

		/* Check */
		cache_get_block(mod->cache, set, way, &tag_check, &state_check);
		if (tag != tag_check)
		{
			test_failed = 1;
			str_printf(&msg_detail_str, &msg_detail_size,
				"\ttag 0x%x found, but 0x%x expected\n",
				tag_check, tag);
		}
		if (state != state_check)
		{
			test_failed = 1;
			str_printf(&msg_detail_str, &msg_detail_size,
				"\tstate %s found, but %s expected\n",
				map_value(&cache_block_state_map, state_check),
				map_value(&cache_block_state_map, state));
		}
	}

	/* Command 'CheckOwner' */
	else if (!strcasecmp(command, "CheckOwner"))
	{
		struct mod_t *mod;
		struct mod_t *owner;
		struct mod_t *owner_check = NULL;

		struct net_node_t *net_node;

		struct dir_entry_t *dir_entry;

		unsigned int set;
		unsigned int way;

		int sub_block;

		/* Read fields */
		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		sub_block = mem_system_command_get_sub_block(token_list, command_line, mod, set, way);
		owner = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_end(token_list, command_line);

		/* Get actual owner */
		owner_check = NULL;
		if (mod->dir)
		{
			dir_entry = dir_entry_get(mod->dir, set, way, sub_block);
			if (dir_entry->owner >= 0)
			{
				assert(mod->high_net);
				net_node = list_get(mod->high_net->node_list, dir_entry->owner);
				owner_check = net_node->user_data;
			}
		}

		/* Message */
		str_printf(&msg_str, &msg_size,
			"check owner at module %s, set %d, way %d, subblock %d - %s",
			mod->name, set, way, sub_block, owner ? owner->name : "None");

		/* Check match */
		if (owner != owner_check)
		{
			test_failed = 1;
			str_printf(&msg_detail_str, &msg_detail_size,
				"\towner %s found, but %s expected\n",
				owner_check ? owner_check->name : "None",
				owner ? owner->name : "None");
		}
	}

	/* Command 'CheckSharers' */
	else if (!strcasecmp(command, "CheckSharers"))
	{
		struct mod_t *mod;
		struct mod_t *sharer;

		struct net_node_t *node;

		unsigned int set;
		unsigned int way;

		int sub_block;
		int node_index;

		struct linked_list_t *sharers_list;
		struct linked_list_t *sharers_check_list;

		/* Read fields */
		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		sub_block = mem_system_command_get_sub_block(token_list, command_line, mod, set, way);
		mem_system_command_expect(token_list, command_line);

		/* Construct list of expected sharers */
		sharers_list = linked_list_create();
		while (list_count(token_list))
		{
			sharer = mem_system_command_get_mod(token_list, command_line);
			linked_list_add(sharers_list, sharer);
		}

		/* Output */
		str_printf(&msg_str, &msg_size,
			"check sharers at module %s, set %d, way %d, subblock %d - { ",
			mod->name, set, way, sub_block);
		LINKED_LIST_FOR_EACH(sharers_list)
		{
			sharer = linked_list_get(sharers_list);
			str_printf(&msg_str, &msg_size, "%s ", sharer->name);
		}
		str_printf(&msg_str, &msg_size, "}");

		/* Construct list of actual sharers */
		sharers_check_list = linked_list_create();
		assert(mod->high_net);
		for (node_index = 0; node_index < mod->high_net->node_count; node_index++)
		{
			if (!dir_entry_is_sharer(mod->dir, set, way, sub_block, node_index))
				continue;
			node = list_get(mod->high_net->node_list, node_index);
			sharer = node->user_data;
			linked_list_add(sharers_check_list, sharer);
		}

		/* Remove in actual sharers everything from expected sharers */
		LINKED_LIST_FOR_EACH(sharers_list)
		{
			/* Get expected sharer */
			sharer = linked_list_get(sharers_list);

			/* Check that it's an actual sharer */
			linked_list_find(sharers_check_list, sharer);
			if (sharers_check_list->error_code)
			{
				test_failed = 1;
				str_printf(&msg_detail_str, &msg_detail_size,
					"\tsharer %s expected, but not found\n",
					sharer->name);
			}

			/* Remove from actual sharers */
			linked_list_remove(sharers_check_list);
		}

		/* Check that there is no actual sharer left */
		LINKED_LIST_FOR_EACH(sharers_check_list)
		{
			sharer = linked_list_get(sharers_check_list);
			test_failed = 1;
			str_printf(&msg_detail_str, &msg_detail_size,
				"\tsharer %s found, but not expected\n",
				sharer->name);
		}

		/* Free lists */
		linked_list_free(sharers_list);
		linked_list_free(sharers_check_list);
	}

	/* Invalid command */
	else
		fatal("%s: %s: invalid command.\n\t> %s",
			__FUNCTION__, command, command_line);

	/* Output */
	fprintf(stderr, ">>> %s - %s\n", msg, test_failed ? "failed" : "passed");
	fprintf(stderr, "%s", msg_detail);

	/* Free command */
	free(command_line);
	str_token_list_free(token_list);
}
