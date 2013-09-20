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

#include <arch/common/Asm.h>

#include "Inst.h"



/*
 * Class 'MIPSAsm'
 */

class MIPSAsm : public Asm
{
public:
	/* Decoding tables */
	MIPSInstInfo *dec_table;
	MIPSInstInfo *dec_table_special;
	MIPSInstInfo *dec_table_special_movci;
	MIPSInstInfo *dec_table_special_srl;
	MIPSInstInfo *dec_table_special_srlv;

	MIPSInstInfo *dec_table_regimm;

	MIPSInstInfo *dec_table_cop0;
	MIPSInstInfo *dec_table_cop0_c0;
	MIPSInstInfo *dec_table_cop0_notc0;
	MIPSInstInfo *dec_table_cop0_notc0_mfmc0;

	MIPSInstInfo *dec_table_cop1;
	MIPSInstInfo *dec_table_cop1_bc1;
	MIPSInstInfo *dec_table_cop1_s;
	MIPSInstInfo *dec_table_cop1_s_movcf;
	MIPSInstInfo *dec_table_cop1_d;
	MIPSInstInfo *dec_table_cop1_d_movcf;
	MIPSInstInfo *dec_table_cop1_w;
	MIPSInstInfo *dec_table_cop1_l;
	MIPSInstInfo *dec_table_cop1_ps;

	MIPSInstInfo *dec_table_cop2;
	MIPSInstInfo *dec_table_cop2_bc2;

	MIPSInstInfo *dec_table_special2;

	MIPSInstInfo *dec_table_special3;
	MIPSInstInfo *dec_table_special3_bshfl;

};

void MIPSAsmCreate(MIPSAsm *self);
void MIPSAsmDestroy(MIPSAsm *self);

void MIPSAsmDisassembleBinary(MIPSAsm *self, char *path);


#endif

