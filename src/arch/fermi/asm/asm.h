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



/*
 * Class 'FrmAsm'
 */

#define FRM_ASM_DEC_TABLE_SIZE  1024

CLASS_BEGIN(FrmAsm, Asm)

	/* Instruction information table, indexed with an instruction opcode
	 * enumeration. */
	FrmInstInfo inst_info[FrmInstOpcodeCount];

	/* Decoding table. This table is indexed by the opcode bits of the
	 * instruction bytes. */
	FrmInstInfo *dec_table[FRM_ASM_DEC_TABLE_SIZE];

CLASS_END(FrmAsm)


void FrmAsmCreate(FrmAsm *self);
void FrmAsmDestroy(FrmAsm *self);

void FrmAsmDisassembleBinary(FrmAsm *self, char *path);
void FrmAsmDisassembleBuffer(FrmAsm *self, void *ptr, int size);


#endif
