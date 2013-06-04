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

#ifndef TOOLS_CLCC_LLVM2SI_BASIC_BLOCK_H
#define TOOLS_CLCC_LLVM2SI_BASIC_BLOCK_H

#include <stdio.h>


/* Forward declarations */
struct linked_list_t;
struct llvm2si_function_t;
struct si2bin_inst_t;


struct llvm2si_basic_block_t
{
	char *name;

	/* Function where the basic block belongs. This field is populated
	 * automatically when function 'llvm2si_function_add' is called. */
	struct llvm2si_function_t *function;

	/* List list of instructions forming the basic block. Each element is of
	 * type 'struct si2bin_inst_t'. */
	struct linked_list_t *inst_list;
};


struct llvm2si_basic_block_t *llvm2si_basic_block_create(const char *name);
void llvm2si_basic_block_free(struct llvm2si_basic_block_t *basic_block);
void llvm2si_basic_block_dump(struct llvm2si_basic_block_t *basic_block, FILE *f);

/* Add an instruction to the basic block. */
void llvm2si_basic_block_add_inst(struct llvm2si_basic_block_t *basic_block,
		struct si2bin_inst_t *inst);


#endif
