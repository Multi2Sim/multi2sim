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
#include "cnode.h"


struct basic_block_t *basic_block_create(struct cnode_t *node)
{
	struct basic_block_t *basic_block;

	/* Check that 'node' doesn't have a basic block */
	if (node->basic_block)
		fatal("%s: node '%s' already contains a basic block",
				__FUNCTION__, node->name);

	/* Initialize */
	basic_block = xcalloc(1, sizeof(struct basic_block_t));
	basic_block->node = node;
	node->basic_block = basic_block;

	/* Class information */
	CLASS_INIT(basic_block, BASIC_BLOCK_TYPE, NULL);

	/* Return */
	return basic_block;
}


void basic_block_free(struct basic_block_t *basic_block)
{
	free(basic_block);
}
