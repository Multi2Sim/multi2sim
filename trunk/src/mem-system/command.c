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



/*
 * Private Functions
 */


static struct mod_t *mem_system_command_get_mod(struct list_t *token_list,
	char *command_line)
{
	struct mod_t *mod;

	char *mod_name;

	/* No more tokens found */
	if (!list_count(token_list))
		fatal("%s: module name expected.\n\t> %s",
			__FUNCTION__, command_line);

	/* Find module */
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

	/* Not enough tokens */
	if (list_count(token_list) < 2)
		fatal("%s: <set> and <way> tokens expected.\n\t> %s",
			__FUNCTION__, command_line);
	
	/* Decode */
	set = atoi(str_token_list_first(token_list));
	str_token_list_shift(token_list);
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


static unsigned int mem_system_command_get_tag(struct list_t *token_list,
	char *command_line)
{
	unsigned int tag;

	/* No more tokens found */
	if (!list_count(token_list))
		fatal("%s: token <tag> expected.\n\t> %s",
			__FUNCTION__, command_line);

	/* Get tag */
	if (sscanf(str_token_list_first(token_list), "0x%x", &tag) != 1)
		fatal("%s: token <tag> must be a valid hex value.\n\t> %s",
			__FUNCTION__, command_line);
	str_token_list_shift(token_list);

	/* Return */
	return tag;
}


static int mem_system_command_get_state(struct list_t *token_list,
	char *command_line)
{
	int state;

	/* No more tokens found */
	if (!list_count(token_list))
		fatal("%s: token <state> expected.\n\t> %s",
			__FUNCTION__, command_line);

	/* Get state */
	state = map_string(&cache_block_state_map, str_token_list_first(token_list));
	if (!state)
		fatal("%s: invalid value for <state>.\n\t> %s",
			__FUNCTION__, command_line);
	str_token_list_shift(token_list);

	/* Return */
	return state;
}


static void mem_system_command_end(struct list_t *token_list, char *command_line)
{
	if (list_count(token_list))
		fatal("%s: %s: end of line expected.\n\t> %s",
			__FUNCTION__, str_token_list_first(token_list), command_line);
}




/*
 * Public Functions
 */


void mem_system_command_handler(int event, void *data)
{
	struct list_t *token_list;

	char *command_line = data;
	char command[MAX_STRING_SIZE];

	/* Split command in tokens */
	token_list = str_token_list_create(command_line, " ");
	if (!list_count(token_list))
		fatal("%s: invalid command syntax.\n\t> %s",
			__FUNCTION__, command_line);

	/* Get command */
	snprintf(command, sizeof command, "%s", str_token_list_first(token_list));
	str_token_list_shift(token_list);

	/* Command 'SetBlock' */
	if (!strcasecmp(command, "SetBlock"))
	{
		struct mod_t *mod;

		unsigned int set;
		unsigned int way;
		unsigned int tag;

		int state;

		mod = mem_system_command_get_mod(token_list, command_line);
		mem_system_command_get_set_way(token_list, command_line, mod, &set, &way);
		tag = mem_system_command_get_tag(token_list, command_line);
		state = mem_system_command_get_state(token_list, command_line);
		mem_system_command_end(token_list, command_line);

		/* Set tag */
		cache_set_block(mod->cache, set, way, tag, state);
	}

	/* Command not supported */
	else
		fatal("%s: %s: invalid command.\n\t> %s",
			__FUNCTION__, command, command_line);

	/* Free command */
	free(command_line);
	str_token_list_free(token_list);
}

