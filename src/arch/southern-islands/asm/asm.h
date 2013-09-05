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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_H
#define ARCH_SOUTHERN_ISLANDS_ASM_H

#include <arch/common/asm.h>

#include "inst.h"



/*
 * Class 'SIAsm'
 */

#define SI_INST_INFO_SOPP_MAX_VALUE 22
#define SI_INST_INFO_SOPC_MAX_VALUE 16
#define SI_INST_INFO_SOP1_MAX_VALUE 53
#define SI_INST_INFO_SOPK_MAX_VALUE 21
#define SI_INST_INFO_SOP2_MAX_VALUE 44
#define SI_INST_INFO_SMRD_MAX_VALUE 31
#define SI_INST_INFO_VOP3_MAX_VALUE 452
#define SI_INST_INFO_VOPC_MAX_VALUE 247
#define SI_INST_INFO_VOP1_MAX_VALUE 68
#define SI_INST_INFO_VOP2_MAX_VALUE 49
#define SI_INST_INFO_VINTRP_MAX_VALUE 3
#define SI_INST_INFO_DS_MAX_VALUE 211
#define SI_INST_INFO_MTBUF_MAX_VALUE 7
#define SI_INST_INFO_MUBUF_MAX_VALUE 113
#define SI_INST_INFO_MIMG_MAX_VALUE 96
#define SI_INST_INFO_EXP_MAX_VALUE  0

CLASS_BEGIN(SIAsm, Asm)

	/* Array containing 'SIInstOpcodeCount' elements with information about
	 * the Southern Islands instructions. */
	SIInstInfo *inst_info;

	/* Pointers to elements in 'inst_info' */
	SIInstInfo *inst_info_sopp[SI_INST_INFO_SOPP_MAX_VALUE + 1];
	SIInstInfo *inst_info_sopc[SI_INST_INFO_SOPC_MAX_VALUE + 1];
	SIInstInfo *inst_info_sop1[SI_INST_INFO_SOP1_MAX_VALUE + 1];
	SIInstInfo *inst_info_sopk[SI_INST_INFO_SOPK_MAX_VALUE + 1];
	SIInstInfo *inst_info_sop2[SI_INST_INFO_SOP2_MAX_VALUE + 1];
	SIInstInfo *inst_info_smrd[SI_INST_INFO_SMRD_MAX_VALUE + 1];
	SIInstInfo *inst_info_vop3[SI_INST_INFO_VOP3_MAX_VALUE + 1];
	SIInstInfo *inst_info_vopc[SI_INST_INFO_VOPC_MAX_VALUE + 1];
	SIInstInfo *inst_info_vop1[SI_INST_INFO_VOP1_MAX_VALUE + 1];
	SIInstInfo *inst_info_vop2[SI_INST_INFO_VOP2_MAX_VALUE + 1];
	SIInstInfo *inst_info_vintrp[SI_INST_INFO_VINTRP_MAX_VALUE + 1];
	SIInstInfo *inst_info_ds[SI_INST_INFO_DS_MAX_VALUE + 1];
	SIInstInfo *inst_info_mtbuf[SI_INST_INFO_MTBUF_MAX_VALUE + 1];
	SIInstInfo *inst_info_mubuf[SI_INST_INFO_MUBUF_MAX_VALUE + 1];
	SIInstInfo *inst_info_mimg[SI_INST_INFO_MIMG_MAX_VALUE + 1];
	SIInstInfo *inst_info_exp[SI_INST_INFO_EXP_MAX_VALUE + 1];

CLASS_END(SIAsm)


void SIAsmCreate(SIAsm *self);
void SIAsmDestroy(SIAsm *self);

void SIAsmDisassembleBinary(SIAsm *self, char *path);
void SIAsmDisassembleOpenGLBinary(SIAsm *self, char *path, int shader_index);

#endif

