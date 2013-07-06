/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>

#include "basic-block.h"


struct basic_block_t *basic_block_create(char *name)
{
	struct basic_block_t *basic_block;

	/* Initialize */
	basic_block = xcalloc(1, sizeof(struct basic_block_t));
	basic_block->pred_list = linked_list_create();
	basic_block->succ_list = linked_list_create();
	basic_block->name = str_set(basic_block->name, name);

	/* Class information */
	CLASS_INIT(basic_block, BASIC_BLOCK_TYPE, NULL);

	/* Return */
	return basic_block;
}


void basic_block_free(struct basic_block_t *basic_block)
{
	linked_list_free(basic_block->pred_list);
	linked_list_free(basic_block->succ_list);
	str_free(basic_block->name);
	free(basic_block);
}


void basic_block_set_name(struct basic_block_t *basic_block, char *name)
{
	basic_block->name = str_set(basic_block->name, name);
}


void basic_block_connect(struct basic_block_t *basic_block,
		struct basic_block_t *basic_block_dest)
{
	/* Make sure that connection does not exist */
	linked_list_find(basic_block->succ_list, basic_block_dest);
	linked_list_find(basic_block_dest->pred_list, basic_block);
	if (!basic_block->succ_list->error_code ||
			!basic_block_dest->pred_list->error_code)
		panic("%s: redundant connection between basic blocks",
				__FUNCTION__);

	/* Make connection */
	linked_list_add(basic_block->succ_list, basic_block_dest);
	linked_list_add(basic_block_dest->pred_list, basic_block);
}
