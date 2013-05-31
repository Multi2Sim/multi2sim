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

#ifndef TOOLS_CLCC_SI2BIN_INST_H
#define TOOLS_CLCC_SI2BIN_INST_H

#include <stdio.h>
#include <arch/southern-islands/asm/asm.h>

/* Forward Declarations */
//struct elf_enc_buffer_t;

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
};


/* Create a new instruction with the opcode corresponding to the first instruction
 * named 'name' that matches the number and type of arguments passed in 'arg_list'.
 * The list of arguments in 'arg_list' and 'arg_list' itself will be freed
 * internally during the call to 'si2bin_inst_free'. */
struct si2bin_inst_t *si2bin_inst_create(char *name, struct list_t *arg_list);

void si2bin_inst_free(struct si2bin_inst_t *inst);
void si2bin_inst_dump(struct si2bin_inst_t *inst, FILE *f);

/* Populate fields 'inst_bytes' and 'size' based on the instruction and the value
 * of its arguments. */
void si2bin_inst_gen(struct si2bin_inst_t *inst);

#endif

