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

#include <llvm-c/Core.h>
#include <stdio.h>

#include <lib/util/class.h>


/* Forward declarations */
struct linked_list_t;
struct llvm2si_function_t;
struct llvm2si_node_t;
struct si2bin_inst_t;
struct basic_block_t;


/* Class-related macros */
#define LLVM2SI_BASIC_BLOCK_TYPE 0xbf85a191
#define LLVM2SI_BASIC_BLOCK(p) CLASS_REINTERPRET_CAST((p), \
		LLVM2SI_BASIC_BLOCK_TYPE, struct llvm2si_basic_block_t)
#define LLVM2SI_BASIC_BLOCK_CLASS_OF(p) CLASS_OF((p), LLVM2SI_BASIC_BLOCK_TYPE)


/* Class: llvm2si_basic_block_t
 * Inherits: basic_block_tm
 */
struct llvm2si_basic_block_t
{
	/* Class information
	 * WARNING - must be the first field */
	struct class_t class_info;

	/* Comment stored temporarily in the basic block to be attached to the
	 * next instruction added. */
	char *comment;

	/* Associated LLVM basic block. It can be NULL for those SI basic blocks
	 * that don't associate to an original LLVM block, such as the header
	 * basic block. */
	LLVMBasicBlockRef llbb;

	/* Function where the basic block belongs. This field is populated
	 * automatically when function 'llvm2si_function_add' is called. */
	struct llvm2si_function_t *function;

	/* Leaf node in control flow tree associated with the basic block. This
	 * value is initialized by 'cnode_create_leaf' when the
	 * basic block is passed. */
	struct cnode_t *node;

	/* List list of instructions forming the basic block. Each element is of
	 * type 'struct si2bin_inst_t'. */
	struct linked_list_t *inst_list;
};


struct llvm2si_basic_block_t *llvm2si_basic_block_create(LLVMBasicBlockRef llbb);
struct llvm2si_basic_block_t *llvm2si_basic_block_create_with_name(char *name);

/* Destructor.
 * Virtual function. */
void llvm2si_basic_block_free(struct basic_block_t *basic_block);

/* Dump the basic block into a file descriptor.
 * Virtual function. */
void llvm2si_basic_block_dump(struct basic_block_t *basic_block, FILE *f);

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


#endif
