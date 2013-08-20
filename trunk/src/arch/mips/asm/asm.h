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
 * Public
 */

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



/*
 * Class 'MIPSAsm'
 */

CLASS_BEGIN(MIPSAsm, Asm)

	/* Decoding tables */
	struct mips_inst_info_t *dec_table;
	struct mips_inst_info_t *dec_table_special;
	struct mips_inst_info_t *dec_table_special_movci;
	struct mips_inst_info_t *dec_table_special_srl;
	struct mips_inst_info_t *dec_table_special_srlv;

	struct mips_inst_info_t *dec_table_regimm;

	struct mips_inst_info_t *dec_table_cop0;
	struct mips_inst_info_t *dec_table_cop0_c0;
	struct mips_inst_info_t *dec_table_cop0_notc0;
	struct mips_inst_info_t *dec_table_cop0_notc0_mfmc0;

	struct mips_inst_info_t *dec_table_cop1;
	struct mips_inst_info_t *dec_table_cop1_bc1;
	struct mips_inst_info_t *dec_table_cop1_s;
	struct mips_inst_info_t *dec_table_cop1_s_movcf;
	struct mips_inst_info_t *dec_table_cop1_d;
	struct mips_inst_info_t *dec_table_cop1_d_movcf;
	struct mips_inst_info_t *dec_table_cop1_w;
	struct mips_inst_info_t *dec_table_cop1_l;
	struct mips_inst_info_t *dec_table_cop1_ps;

	struct mips_inst_info_t *dec_table_cop2;
	struct mips_inst_info_t *dec_table_cop2_bc2;

	struct mips_inst_info_t *dec_table_special2;

	struct mips_inst_info_t *dec_table_special3;
	struct mips_inst_info_t *dec_table_special3_bshfl;

CLASS_END(MIPSAsm)

void MIPSAsmCreate(MIPSAsm *self);
void MIPSAsmDestroy(MIPSAsm *self);

void MIPSAsmDisassembleBinary(MIPSAsm *self, char *path);


#endif

