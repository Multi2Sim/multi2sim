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

#ifndef FERMI_ASM_ASM_H
#define FERMI_ASM_ASM_H

#include <arch/common/asm.h>

#include "inst.h"



void frm_disasm_init(void);
void frm_disasm_done(void);

void frm_inst_dump(char *str, int inst_str_size, void *buf, int inst_index);
void frm_inst_hex_dump(FILE *f, unsigned char *buf, int inst_index);




/* Copy instruction */
void frm_inst_copy(FrmInst *dest, FrmInst *src);

/* Obtaining source operand fields for ALU instructions */
void frm_inst_get_op_src(FrmInst *inst, int src_idx,
        int *sel, int *rel, int *chan, int *neg, int *abs);

void frm_inst_decode(FrmInst *inst);

void frm_disasm(char *path);

/* a temporial implementation to disassmbler only text_section */
struct elf_enc_buffer_t;
void frm_disasm_text_section_buffer(struct elf_enc_buffer_t *buffer);



/*
 * Class 'FrmAsm'
 */

CLASS_BEGIN(FrmAsm, Asm)

CLASS_END(FrmAsm)


void FrmAsmCreate(FrmAsm *self);
void FrmAsmDestroy(FrmAsm *self);



/*
 * Public
 */


struct frm_inst_info_t
{
	FrmInstOpcode opcode;
	FrmInstCategory category;

	char *name;
	char *fmt_str;

	/* Field of the instruction containing the instruction opcode
	 * identifier. */
	unsigned int op;

	int size;
};

/* Table containing information for all instructions, filled out with the
 * fields found in 'asm.dat'. */
extern struct frm_inst_info_t frm_inst_info[FrmInstOpcodeCount];


#endif
