/*
 *  Multi2Sim
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

#include <arch/southern-islands/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "inst-info.h"
#include "token.h"


/*
 * Global Functions
 */

/* List indexed by an instruction opcode (enum si_inst_opcode_t). Each element
 * of the list if of type 'si2bin_inst_info_t'. */
struct list_t *si2bin_inst_info_list;

/* Hash table indexed by an instruction name, returning the associated entry in
 * of type 'si2bin_inst_info_t'. The name of the instruction is
 * extracted from the first token of the format string. */
struct hash_table_t *si2bin_inst_info_table;


void si2bin_inst_info_init(void)
{
	struct si2bin_inst_info_t *info;
	struct si2bin_inst_info_t *prev_info;
	struct si_inst_info_t *inst_info;

	int i;

	/* Initialize hash table and list */
	si2bin_inst_info_list = list_create_with_size(SI_INST_COUNT);
	si2bin_inst_info_table = hash_table_create(SI_INST_COUNT, 1);

	/* Populate them */
	for (i = 0; i < SI_INST_COUNT; i++)
	{
		/* Instruction info from disassembler */
		inst_info = &si_inst_info[i];
		if (!inst_info->name || !inst_info->fmt_str)
		{
			list_add(si2bin_inst_info_list, NULL);
			continue;
		}

		/* Create instruction info object */
		info = si2bin_inst_info_create(inst_info);

		/* Insert to list */
		list_add(si2bin_inst_info_list, info);

		/* Insert instruction info structure into hash table. There could
		 * be already an instruction encoding with the same name but a
		 * different encoding. They all form a linked list. */
		prev_info = hash_table_get(si2bin_inst_info_table, info->name);
		if (prev_info)
		{
			/* non vop3 instructions are added first into list. Add vop3 version to end of list */
			prev_info->next = info;

			/* non vop3 instructions are added first but vop3 version is added to the front of list */
			//info->next = prev_info;
			//hash_table_set(si2bin_inst_info_table, info->name, info);
		}
		else
		{
			hash_table_insert(si2bin_inst_info_table, info->name, info);
		}
	}
}


void si2bin_inst_info_done(void)
{
	struct si2bin_inst_info_t *info;
	int index;

	/* Free elements 'si2bin_inst_info_t' */
	LIST_FOR_EACH(si2bin_inst_info_list, index)
	{
		info = list_get(si2bin_inst_info_list, index);
		if (info)
			si2bin_inst_info_free(info);
	}

	/* Free list and hash table */
	list_free(si2bin_inst_info_list);
	hash_table_free(si2bin_inst_info_table);
}




/*
 * Instruction Information Object
 */

struct si2bin_inst_info_t *si2bin_inst_info_create(struct si_inst_info_t *inst_info)
{
	struct si2bin_inst_info_t *info;
	struct si2bin_token_t *token;
	enum si2bin_token_type_t token_type;

	int index;
	char *str_token;

	/* Initialize */
	info = xcalloc(1, sizeof(struct si2bin_inst_info_t));
	info->inst_info = inst_info;

	/* Create list of tokens from format string */
	info->str_token_list = str_token_list_create(inst_info->fmt_str, ", ");
	assert(info->str_token_list->count);
	info->name = list_get(info->str_token_list, 0);

	/* Create list of formal arguments */
	info->token_list = list_create_with_size(5);
	for (index = 1; index < info->str_token_list->count; index++)
	{
		/* Get token from format string */
		str_token = list_get(info->str_token_list, index);
		token_type = str_map_string_case(&si2bin_token_map, str_token);

		/* Add formal argument */
		token = si2bin_token_create(token_type);
		list_add(info->token_list, token);
	}

	/* Return */
	return info;
}


void si2bin_inst_info_free(struct si2bin_inst_info_t *info)
{
	struct si2bin_token_t *token;
	int index;

	/* Tokens */
	str_token_list_free(info->str_token_list);
	LIST_FOR_EACH(info->token_list, index)
	{
		token = list_get(info->token_list, index);
		si2bin_token_free(token);
	}
	list_free(info->token_list);

	/* Free */
	free(info);
}
