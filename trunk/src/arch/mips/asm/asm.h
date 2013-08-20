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

#ifndef MIPS_ASM_ASM_H
#define MIPS_ASM_ASM_H

#include <arch/common/asm.h>
#include <lib/class/class.h>

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inst.h"


/* 
 * Mips Disassembler
 */

void mips_asm_init();
void mips_asm_done();
void mips_emu_disasm(char *path);

/* Pointers to the tables of instructions */
extern struct mips_inst_info_t *mips_asm_table;
extern struct mips_inst_info_t *mips_asm_table_special;
extern struct mips_inst_info_t *mips_asm_table_special_movci;
extern struct mips_inst_info_t *mips_asm_table_special_srl;
extern struct mips_inst_info_t *mips_asm_table_special_srlv;

extern struct mips_inst_info_t *mips_asm_table_regimm;

extern struct mips_inst_info_t *mips_asm_table_cop0;
extern struct mips_inst_info_t *mips_asm_table_cop0_c0;
extern struct mips_inst_info_t *mips_asm_table_cop0_notc0;
extern struct mips_inst_info_t *mips_asm_table_cop0_notc0_mfmc0;

extern struct mips_inst_info_t *mips_asm_table_cop1;
extern struct mips_inst_info_t *mips_asm_table_cop1_bc1;
extern struct mips_inst_info_t *mips_asm_table_cop1_s;
extern struct mips_inst_info_t *mips_asm_table_cop1_s_movcf;
extern struct mips_inst_info_t *mips_asm_table_cop1_d;
extern struct mips_inst_info_t *mips_asm_table_cop1_d_movcf;
extern struct mips_inst_info_t *mips_asm_table_cop1_w;
extern struct mips_inst_info_t *mips_asm_table_cop1_l;
extern struct mips_inst_info_t *mips_asm_table_cop1_ps;

extern struct mips_inst_info_t *mips_asm_table_cop2;
extern struct mips_inst_info_t *mips_asm_table_cop2_bc2;

extern struct mips_inst_info_t *mips_asm_table_special2;

extern struct mips_inst_info_t *mips_asm_table_special3;
extern struct mips_inst_info_t *mips_asm_table_special3_bshfl;


struct mips_inst_info_t
{
	MIPSInstOpcode opcode;
	char *name;
	char *fmt_str;
	int size;
	int next_table_low;
	int next_table_high;
	struct mips_inst_info_t *next_table;

};


void mips_disasm(unsigned int buf, unsigned int ip, volatile MIPSInst *inst);



/*
 * Class 'MIPSAsm'
 */

CLASS_BEGIN(MIPSAsm, Asm)

CLASS_END(MIPSAsm)

void MIPSAsmCreate(MIPSAsm *self);
void MIPSAsmDestroy(MIPSAsm *self);

#endif

