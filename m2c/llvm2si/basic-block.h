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

#ifndef M2C_LLVM2SI_BASIC_BLOCK_H
#define M2C_LLVM2SI_BASIC_BLOCK_H

#include <stdio.h>

#include <llvm-c/Core.h>


/* Forward declarations */
struct linked_list_t;
struct llvm2si_function_t;
struct si2bin_inst_t;


struct llvm2si_basic_block_t
{
	char *name;

	/* Comment stored temporarily in the basic block to be attached to the
	 * next instruction added. */
	char *comment;

	/* Associated LLVM basic block. It can be NULL for those SI basic blocks
	 * that don't associate to an original LLVM block, such as the header
	 * basic block. */
	LLVMBasicBlockRef llbb;

	/* Predecessor and successor lists of basic blocks. Basic blocks are
	 * added to this lists by functions 'llvm2si_function_add_basic_block_
	 * after' and 'llvm2si_function_add_basic_block_before'. */
	struct linked_list_t *pred_list;
	struct linked_list_t *succ_list;

	/* Function where the basic block belongs. This field is populated
	 * automatically when function 'llvm2si_function_add' is called. */
	struct llvm2si_function_t *function;

	/* List list of instructions forming the basic block. Each element is of
	 * type 'struct si2bin_inst_t'. */
	struct linked_list_t *inst_list;
};


struct llvm2si_basic_block_t *llvm2si_basic_block_create(LLVMBasicBlockRef llbb);
void llvm2si_basic_block_free(struct llvm2si_basic_block_t *basic_block);
void llvm2si_basic_block_dump(struct llvm2si_basic_block_t *basic_block, FILE *f);

/* Add one SI instruction to the 'inst_list' field of the basic block. */
void llvm2si_basic_block_add_inst(struct llvm2si_basic_block_t *basic_block,
		struct si2bin_inst_t *inst);

/* Add a comment to a basic block. The comment will be attached to the next
 * instruction added with 'llvm2si_basic_block_add_inst'. If not other
 * instruction is added to the basic block, the comment won't have any effect. */
void llvm2si_basic_block_add_comment(struct llvm2si_basic_block_t *basic_block,
		char *comment);

/* Emit SI code for the basic block. The function reads the LLVM basic block
 * stored in field 'llbb', and emits SI instructions into field 'inst_list'. */
void llvm2si_basic_block_emit(struct llvm2si_basic_block_t *basic_block);

/* Connect 'basic_block' to 'basic_block_dest' in CFG. */
void llvm2si_basic_block_connect(struct llvm2si_basic_block_t *basic_block,
		struct llvm2si_basic_block_t *basic_block_dest);


#endif
