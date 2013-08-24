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

#ifndef ARCH_KEPLER_ASM_ASM_H
#define ARCH_KEPLER_ASM_ASM_H

#include <lib/class/class.h>

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inst.h"



/*
 * Class 'KplAsm'
 */

#define KPL_ASM_OPCODE_A            	0
#define KPL_ASM_OPCODE_B            	1
#define KPL_ASM_OPCODE_C            	2

#define KPL_ASM_OPCODE_A_A          	0

#define KPL_ASM_OPCODE_B_A          	0
#define KPL_ASM_OPCODE_B_B          	1
#define KPL_ASM_OPCODE_B_C          	2
#define KPL_ASM_OPCODE_B_D          	3

#define KPL_ASM_OPCODE_B_C_A        	0
#define KPL_ASM_OPCODE_B_C_B        	1
#define KPL_ASM_OPCODE_B_C_C        	2
#define KPL_ASM_OPCODE_B_C_D        	3

#define KPL_ASM_OPCODE_B_C_D_A      	1
#define KPL_ASM_OPCODE_B_C_D_B      	2
#define KPL_ASM_OPCODE_B_C_D_C      	3

#define KPL_ASM_OPCODE_B_C_D_C_A    	1
#define KPL_ASM_OPCODE_B_C_D_C_B    	2
#define KPL_ASM_OPCODE_B_C_D_C_C    	3

#define KPL_ASM_OPCODE_B_D_A        	0

#define KPL_ASM_OPCODE_C_A		0
#define KPL_ASM_OPCODE_C_B_0		1
#define KPL_ASM_OPCODE_C_B_1		2
#define KPL_ASM_OPCODE_C_B_2		3

#define KPL_ASM_OPCODE_C_A_A		2
#define KPL_ASM_OPCODE_C_A_B		3

#define KPL_ASM_OPCODE_C_B_A_0		1
#define KPL_ASM_OPCODE_C_B_A_1		9
#define KPL_ASM_OPCODE_C_B_B_0		2
#define KPL_ASM_OPCODE_C_B_B_1		10
#define KPL_ASM_OPCODE_C_B_C_0		3
#define KPL_ASM_OPCODE_C_B_C_1		11
#define KPL_ASM_OPCODE_C_B_D		12
#define KPL_ASM_OPCODE_C_B_E_0  	0
#define KPL_ASM_OPCODE_C_B_E_1      	4
#define KPL_ASM_OPCODE_C_B_E_2      	5
#define KPL_ASM_OPCODE_C_B_E_3      	6
#define KPL_ASM_OPCODE_C_B_E_4      	7
#define KPL_ASM_OPCODE_C_B_E_5      	13
#define KPL_ASM_OPCODE_C_B_E_6      	14
#define KPL_ASM_OPCODE_C_B_E_7          15

#define KPL_ASM_OPCODE_C_B_C_A			1
#define KPL_ASM_OPCODE_C_B_C_B			2
#define KPL_ASM_OPCODE_C_B_C_C			3

#define KPL_ASM_OPCODE_C_B_C_C_A		1
#define KPL_ASM_OPCODE_C_B_C_C_B		2
#define KPL_ASM_OPCODE_C_B_C_C_C		3

#define KPL_ASM_OPCODE_C_B_E_A			1
#define KPL_ASM_OPCODE_C_B_E_B			2
#define KPL_ASM_OPCODE_C_B_E_C			3

#define KPL_ASM_OPCODE_C_B_E_A_A		6
#define KPL_ASM_OPCODE_C_B_E_A_B		7

#define KPL_ASM_OPCODE_C_B_E_A_A_A		4
#define KPL_ASM_OPCODE_C_B_E_A_A_B		5
#define KPL_ASM_OPCODE_C_B_E_A_A_C		6
#define KPL_ASM_OPCODE_C_B_E_A_A_D		7

#define KPL_ASM_OPCODE_C_B_E_A_A_C_A		1

#define KPL_ASM_OPCODE_C_B_E_A_B_A		0
#define KPL_ASM_OPCODE_C_B_E_A_B_B		1

#define KPL_ASM_OPCODE_C_B_E_A_B_A_A		0
#define KPL_ASM_OPCODE_C_B_E_A_B_A_B		1
#define KPL_ASM_OPCODE_C_B_E_A_B_A_C		2

#define KPL_ASM_OPCODE_C_B_E_B_A		0
#define KPL_ASM_OPCODE_C_B_E_B_B		1

#define KPL_ASM_OPCODE_C_B_E_B_A_A		1


CLASS_BEGIN(KplAsm, Object)

	/* Instruction information */
	KplInstInfo inst_info[KplInstOpcodeCount];


	/* Decoding tables */

	KplInstTableEntry *dec_table;
	KplInstTableEntry *dec_table_a;
	KplInstTableEntry *dec_table_b;
	KplInstTableEntry *dec_table_c;

	KplInstTableEntry *dec_table_a_a;

	KplInstTableEntry *dec_table_b_a;
	KplInstTableEntry *dec_table_b_b;
	KplInstTableEntry *dec_table_b_c;
	KplInstTableEntry *dec_table_b_d;

	KplInstTableEntry *dec_table_b_c_a;
	KplInstTableEntry *dec_table_b_c_b;
	KplInstTableEntry *dec_table_b_c_c;
	KplInstTableEntry *dec_table_b_c_d;

	KplInstTableEntry *dec_table_b_c_d_a;
	KplInstTableEntry *dec_table_b_c_d_b;
	KplInstTableEntry *dec_table_b_c_d_c;

	KplInstTableEntry *dec_table_b_c_d_c_a;
	KplInstTableEntry *dec_table_b_c_d_c_b;
	KplInstTableEntry *dec_table_b_c_d_c_c;

	KplInstTableEntry *dec_table_b_d_a;

	KplInstTableEntry *dec_table_c_a;
	KplInstTableEntry *dec_table_c_b;

	KplInstTableEntry *dec_table_c_a_a;
	KplInstTableEntry *dec_table_c_a_b;

	KplInstTableEntry *dec_table_c_b_a;
	KplInstTableEntry *dec_table_c_b_b;
	KplInstTableEntry *dec_table_c_b_c;
	KplInstTableEntry *dec_table_c_b_d;
	KplInstTableEntry *dec_table_c_b_e;

	KplInstTableEntry *dec_table_c_b_c_a;
	KplInstTableEntry *dec_table_c_b_c_b;
	KplInstTableEntry *dec_table_c_b_c_c;

	KplInstTableEntry *dec_table_c_b_c_c_a;
	KplInstTableEntry *dec_table_c_b_c_c_b;
	KplInstTableEntry *dec_table_c_b_c_c_c;

	KplInstTableEntry *dec_table_c_b_e_a;
	KplInstTableEntry *dec_table_c_b_e_b;
	KplInstTableEntry *dec_table_c_b_e_c;

	KplInstTableEntry *dec_table_c_b_e_a_a;
	KplInstTableEntry *dec_table_c_b_e_a_b;

	KplInstTableEntry *dec_table_c_b_e_a_a_a;
	KplInstTableEntry *dec_table_c_b_e_a_a_b;
	KplInstTableEntry *dec_table_c_b_e_a_a_c;
	KplInstTableEntry *dec_table_c_b_e_a_a_d;

	KplInstTableEntry *dec_table_c_b_e_a_a_c_a;

	KplInstTableEntry *dec_table_c_b_e_a_b_a;
	KplInstTableEntry *dec_table_c_b_e_a_b_b;

	KplInstTableEntry *dec_table_c_b_e_a_b_a_a;
	KplInstTableEntry *dec_table_c_b_e_a_b_a_b;
	KplInstTableEntry *dec_table_c_b_e_a_b_a_c;

	KplInstTableEntry *dec_table_c_b_e_b_a;
	KplInstTableEntry *dec_table_c_b_e_b_b;

	KplInstTableEntry *dec_table_c_b_e_b_a_a;

CLASS_END(KplAsm)


void KplAsmCreate(KplAsm *self);
void KplAsmDestroy(KplAsm *self);

void KplAsmDisassembleBinary(KplAsm *self, char *path);


#endif

