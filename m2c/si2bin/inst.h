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

#ifndef M2C_SI2BIN_INST_H
#define M2C_SI2BIN_INST_H

#include <stdio.h>
#include <arch/southern-islands/asm/asm.h>
#include <lib/util/class.h>


struct si2bin_inst_t
{
	enum si_inst_opcode_t opcode;
	struct si2bin_inst_info_t *info;

	/* List of arguments. Each element in the list is of type
	 * 'struct si2bin_arg_t'. */
	struct list_t *arg_list;

	/* Instruction bytes generated */
	union si_inst_microcode_t inst_bytes;
	int size;  /* Number of bytes */

	/* For LLVM-to-SI back-end: basic block that the instruction
	 * belongs to. */
	Llvm2siBasicBlock *basic_block;

	/* Comment attached to the instruction, which will be dumped together
	 * with it. */
	char *comment;
};


/* Create a new instruction with an opcode of type 'enum si_inst_opcode_t', as
 * defined in the Southern Islands disassembler. The argument list in 'arg_list'
 * is composed of objects of type 'si2bin_arg_t'. All these objects, as well as
 * the argument list itself, will be freed internally when calling
 * 'si2bin_inst_free'. */
struct si2bin_inst_t *si2bin_inst_create(int opcode, struct list_t *arg_list);

/* Create a new instruction with the opcode corresponding to the first instruction
 * named 'name' that matches the number and type of arguments passed in 'arg_list'.
 * The list of arguments in 'arg_list' and 'arg_list' itself will be freed
 * internally during the call to 'si2bin_inst_free'. */
struct si2bin_inst_t *si2bin_inst_create_with_name(char *name,
		struct list_t *arg_list);

void si2bin_inst_free(struct si2bin_inst_t *inst);
void si2bin_inst_dump(struct si2bin_inst_t *inst, FILE *f);
void si2bin_inst_dump_assembly(struct si2bin_inst_t *inst, FILE *f);

/* Attach a comment to the instruction */
void si2bin_inst_add_comment(struct si2bin_inst_t *inst, char *comment);

/* Populate fields 'inst_bytes' and 'size' based on the instruction and the value
 * of its arguments. */
void si2bin_inst_gen(struct si2bin_inst_t *inst);

#endif

