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

#ifndef M2C_FRM2BIN_INST_INFO_H
#define M2C_FRM2BIN_INST_INFO_H

#include <stdio.h>
#include <arch/fermi/asm/asm.h>


/* Object 'frm2bin_inst_info_t' */
/* Forward declaration */
struct frm_inst_info_t;

extern struct hash_table_t *frm2bin_inst_info_table;

/* Element of hash table 'frm2bin_inst_info_table', index by an instruction
 * name. */
struct frm2bin_inst_info_t
{
	/* There can be multiple instruction encodings for the same instruction
	 * name. This points to the next one in the list. */
	struct frm2bin_inst_info_t *next;

	/* Associated info structure in disassembler */
	struct frm_inst_info_t *inst_info;

	/* List of tokens in format string */
	struct list_t *str_token_list;  /* Element of type string */
	struct list_t *token_list;  /* Element of type 'frm_token_t' */
	char *name;  /* Not necessarily token 0 of this list */

	/* add the following to record modifier list */
	struct list_t *str_mod_list;
	struct list_t *mod_list;
};

struct frm2bin_inst_info_t *frm2bin_inst_info_create(struct frm_inst_info_t *inst_info);
void frm2bin_inst_info_free(struct frm2bin_inst_info_t *info);


/*
 * Global functions
 */

/* Initialization of encoding tables */
void frm2bin_inst_info_init(void);
void frm2bin_inst_info_done(void);

#endif

