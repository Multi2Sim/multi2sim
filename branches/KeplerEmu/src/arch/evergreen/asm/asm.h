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

#ifndef EVERGREEN_ASM_ASM_H
#define EVERGREEN_ASM_ASM_H

#include <stdio.h>

#include <arch/common/asm.h>
#include <lib/util/elf-format.h>

#include "inst.h"


/*
 * Class 'EvgAsm'
 */

#define EVG_INST_INFO_CF_LONG_SIZE  256
#define EVG_INST_INFO_CF_SHORT_SIZE 16
#define EVG_INST_INFO_ALU_LONG_SIZE 256
#define EVG_INST_INFO_ALU_SHORT_SIZE 32
#define EVG_INST_INFO_TEX_SIZE 32

CLASS_BEGIN(EvgAsm, Asm)

	/* Table containing information of all instructions */
	EvgInstInfo inst_info[EvgInstOpcodeCount];

	/* Pointers to 'inst_info' table indexed by instruction opcode */
	EvgInstInfo *inst_info_cf_long[EVG_INST_INFO_CF_LONG_SIZE];  /* for 8-bit cf_inst */
	EvgInstInfo *inst_info_cf_short[EVG_INST_INFO_CF_SHORT_SIZE];  /* for 4-bit cf_inst */
	EvgInstInfo *inst_info_alu_long[EVG_INST_INFO_ALU_LONG_SIZE];  /* for ALU_OP2 */
	EvgInstInfo *inst_info_alu_short[EVG_INST_INFO_ALU_SHORT_SIZE];  /* for ALU_OP3 */
	EvgInstInfo *inst_info_tex[EVG_INST_INFO_TEX_SIZE];  /* For tex instructions */

CLASS_END(EvgAsm)

void EvgAsmCreate(EvgAsm *self);
void EvgAsmDestroy(EvgAsm *self);

void EvgAsmDisassembleBinary(EvgAsm *self, char *path);
void EvgAsmDisassembleOpenGLBinary(EvgAsm *self, char *path, int index);

#endif

