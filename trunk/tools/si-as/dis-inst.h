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

#include <stdio.h>
#include <arch/southern-islands/asm/asm.h>


/*
 * Object 'si_dis_inst_t'
 */

struct si_dis_inst_t
{
	enum si_inst_opcode_t opcode;
	struct si_dis_inst_info_t *info;
	struct list_t *arg_list;
};


/* Returns a newly created si_dis_inst_t object
 * with the op-code corresponding to the
 * 'inst_str' instruction.                 */
struct si_dis_inst_t *si_dis_inst_create(char *name, struct list_t *arg_list);

void si_dis_inst_free(struct si_dis_inst_t *inst);
void si_dis_inst_dump(struct si_dis_inst_t *inst, FILE *f);

/* This function generates the assembly code for instruction 'inst' into
 * buffer 'inst_bytes. The returned value is the number of
 *  bytes of the generated instruction, or 0 if the inst
 * could not be decoded */
int si_dis_inst_code_gen(struct si_dis_inst_t *inst, unsigned long long *inst_bytes);




/*
 * Object 'si_dis_inst_info_t'
 */

/* Forward declaration */
struct si_inst_info_t;

/* Element of hash table 'si_dis_inst_info_table', index by an instruction
 * name. */
struct si_dis_inst_info_t
{
	/* There can be multiple instruction encodings for the same instruction
	 * name. This points to the next one in the list. */
	struct si_dis_inst_info_t *next;

	/* Associated info structure in disassembler */
	struct si_inst_info_t *inst_info;

	/* List of tokens in format string */
	struct list_t *tokens;
	char *name;  /* Token 0 of this list */

	/* Formal arguments. Each element of the list is of type
	 * 'si_formal_arg_t'. */
	struct list_t *formal_arg_list;
};

struct si_dis_inst_info_t *si_dis_inst_info_create(struct si_inst_info_t *inst_info);
void si_dis_inst_info_free(struct si_dis_inst_info_t *info);






/*
 * Other global functions
 */

/* Initialization of encoding tables */
void si_dis_inst_init(void);
void si_dis_inst_done(void);

