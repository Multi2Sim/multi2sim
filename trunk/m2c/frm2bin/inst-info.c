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

#include <arch/fermi/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "inst-info.h"
#include "token.h"
#include "arg.h"


/* 
 * Global Functions
 */

/* Hash table indexed by an instruction name, returning the associated entry
 * in 'frm_inst_info' of type 'frm_inst_info_t'. The name of the instruction
 * is extracted from the first token of the format string. */
struct hash_table_t *frm2bin_inst_info_table;


void frm2bin_inst_info_init(void)
{
	struct frm2bin_inst_info_t *info;
	struct frm2bin_inst_info_t *prev_info;
	struct frm_inst_info_t *inst_info;

	int i;

	/* Initialize hash table with instruction names. */
	frm2bin_inst_info_table = hash_table_create(FRM_INST_COUNT, 1);
	for (i = 0; i < FRM_INST_COUNT; i++)
	{
		/* Instruction info from disassembler */
		inst_info = &frm_inst_info[i];
		if (!inst_info->name || !inst_info->fmt_str)
			continue;

		/* Create instruction info object */
		info = frm2bin_inst_info_create(inst_info);

		/* Insert instruction info structure into hash table. There
		 * could be already an instruction encoding with the same
		 * name. They all formed a linked list. */
		prev_info =
			hash_table_get(frm2bin_inst_info_table, info->name);
		if (prev_info)
		{
			info->next = prev_info;
			hash_table_set(frm2bin_inst_info_table, info->name,
				info);
		}
		else
		{
			hash_table_insert(frm2bin_inst_info_table, info->name,
				info);
		}
	}
}

void frm2bin_inst_info_done(void)
{
	struct frm2bin_inst_info_t *info;
	struct frm2bin_inst_info_t *next_info;

	char *name;

	HASH_TABLE_FOR_EACH(frm2bin_inst_info_table, name, info)
	{
		while (info)
		{
			next_info = info->next;
			frm2bin_inst_info_free(info);
			info = next_info;
		}
	}
	hash_table_free(frm2bin_inst_info_table);
}

/* Object 'frm2bin_inst_info_t' */
struct frm2bin_inst_info_t *frm2bin_inst_info_create(struct frm_inst_info_t *inst_info)
{
	struct frm2bin_inst_info_t *info;
	struct frm_token_t *token;
	enum frm_token_type_t token_type;
	char *name_str;
	struct list_t *mod_list;
	char *mod_name;
	struct frm_mod_t *mod;

	int index;
	char *str_token;

	/* Initialize */
	info = xcalloc(1, sizeof(struct frm2bin_inst_info_t));
	info->inst_info = inst_info;

	/* Create list of tokens from format string */
	info->str_token_list =
		str_token_list_create(inst_info->fmt_str, ", []");
	assert(info->str_token_list->count);

	/* for Fermi, name should be the 2nd token, 1st token is predicate in 
	 * the name token, only the string before 1st % is name, others are
	 * modifier it's different from southern-island extract inst-name
	 * and inst-mod token list from name_str */
	name_str = list_get(info->str_token_list, 1);
	info->str_mod_list = str_token_list_create(name_str, "%");

	/* extract inst name from 1st element of str_mod_list, str_mod_list
	 * contains only modifier now */
	info->name = list_remove_at(info->str_mod_list, 0);

	/* create the mod_list */
	mod_list = list_create();
	if (info->str_mod_list->count > 0)
	{
		/* create one mod obj for each token in str_mod_list */
		LIST_FOR_EACH(info->str_mod_list, index)
		{
			mod_name = list_get(info->str_mod_list, index);
			mod = frm_mod_create_with_name(mod_name);
			list_add(mod_list, mod);
		}
	}
	info->mod_list = mod_list;

	/* Create list of formal arguments */
	info->token_list = list_create_with_size(5);

	/* 1st argument starts from index-2, because index 0 is predicate,
	 * index 1 is instruction name it's different from southern-island */
	for (index = 2; index < info->str_token_list->count; index++)
	{
		/* Get token from format string */
		str_token = list_get(info->str_token_list, index);
		token_type = str_map_string_case(&frm_token_map, str_token);
		if (!token_type)
			warning("%s: unrecognized token: %s",
				__FUNCTION__, str_token);

		/* Add formal argument */
		token = frm_token_create(token_type);
		list_add(info->token_list, token);
	}

	/* Return */
	return info;
}


void frm2bin_inst_info_free(struct frm2bin_inst_info_t *info)
{
	struct frm_token_t *token;
	struct frm_mod_t *mod;
	int index;

	/* Argument Tokens */
	str_token_list_free(info->str_token_list);
	LIST_FOR_EACH(info->token_list, index)
	{
		token = list_get(info->token_list, index);
		frm_token_free(token);
	}
	list_free(info->token_list);

	/* Modifier Tokens */
	str_token_list_free(info->str_mod_list);
	LIST_FOR_EACH(info->mod_list, index)
	{
		mod = list_get(info->mod_list, index);
		frm_mod_free(mod);
	}
	list_free(info->mod_list);

	/* free the info name */
	free(info->name);
	/* Free */
	free(info);
}

