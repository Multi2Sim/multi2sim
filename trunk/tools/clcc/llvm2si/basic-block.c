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

#include <clcc/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>

#include "basic-block.h"


/*
 * Public Functions
 */

struct llvm2si_basic_block_t *llvm2si_basic_block_create(const char *name)
{
	struct llvm2si_basic_block_t *basic_block;

	/* Initialize */
	basic_block = xcalloc(1, sizeof(struct llvm2si_basic_block_t));
	basic_block->name = xstrdup(name);
	basic_block->inst_list = linked_list_create();

	/* Return */
	return basic_block;
}


void llvm2si_basic_block_free(struct llvm2si_basic_block_t *basic_block)
{
	/* Free list of instructions */
	LINKED_LIST_FOR_EACH(basic_block->inst_list)
		si2bin_inst_free(linked_list_get(basic_block->inst_list));
	linked_list_free(basic_block->inst_list);
	
	/* Rest */
	free(basic_block->name);
	free(basic_block);
}


void llvm2si_basic_block_dump(struct llvm2si_basic_block_t *basic_block, FILE *f)
{
	struct si2bin_inst_t *inst;

	/* Basic block name */
	fprintf(f, "== Basic block '%s' ==\n", basic_block->name);

	/* Print list of instructions */
	LINKED_LIST_FOR_EACH(basic_block->inst_list)
	{
		inst = linked_list_get(basic_block->inst_list);
		si2bin_inst_dump(inst, f);
	}

	/* End */
	fprintf(f, "\n");
}


void llvm2si_basic_block_add(struct llvm2si_basic_block_t *basic_block,
		struct si2bin_inst_t *inst)
{
	/* Check that the instruction does not belong to any other basic
	 * block already. */
	if (inst->basic_block)
		panic("%s: instruction already added to basic block",
				__FUNCTION__);

	/* Add instruction */
	linked_list_add(basic_block->inst_list, inst);
	inst->basic_block = basic_block;
}
