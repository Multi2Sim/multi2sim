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

#ifndef M2C_FRM2BIN_INST_H
#define M2C_FRM2BIN_INST_H

#include <stdio.h>

#include <arch/fermi/asm/asm.h>


struct frm2bin_inst_t
{
	/* si use: si_inst_opcode_t, frm use: frm_inst_enum, see asm.h */
	enum frm_inst_enum opcode;
	struct frm2bin_inst_info_t *info;
	struct list_t *arg_list;
	/* modifier list */
	struct list_t *mod_list;
	int pred_num;		/* predicate number */

	/* Instruction bytes generated */
	union frm_inst_dword_t inst_bytes;

	/* address of current instruction */
	long long int addr;

	int size;		/* Number of bytes */
};

/* def for predicate type */
struct frm2bin_pred_t
{
	/* which predicate register to use */
	unsigned int number;
};

/* Returns a newly created frm2bin_inst_t object with the op-code
 * corresponding to the 'inst_str' instruction. */
struct frm2bin_inst_t *frm2bin_inst_create(struct frm2bin_pred_t *pred,
	char *name, struct list_t *arg_list);

void frm2bin_inst_free(struct frm2bin_inst_t *inst);

void frm2bin_inst_dump(struct frm2bin_inst_t *inst, FILE *f);

void frm2bin_inst_gen(struct frm2bin_inst_t *inst);

struct frm2bin_pred_t *frm2bin_pred_create(int number);

void frm2bin_pred_free(struct frm2bin_pred_t *pred);

#endif

