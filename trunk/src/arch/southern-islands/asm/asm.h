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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_H_OLD
#define ARCH_SOUTHERN_ISLANDS_ASM_H_OLD

#include <arch/common/asm.h>

#include "inst.h"



/*
 * Class 'SIAsm'
 */

#define si_inst_info_sopp_count 23
#define si_inst_info_sopc_count 17
#define si_inst_info_sop1_count 54
#define si_inst_info_sopk_count 22
#define si_inst_info_sop2_count 45
#define si_inst_info_smrd_count 32
#define si_inst_info_vop3_count 453
#define si_inst_info_vopc_count 248
#define si_inst_info_vop1_count 69
#define si_inst_info_vop2_count 50
#define si_inst_info_vintrp_count 4
#define si_inst_info_ds_count 212
#define si_inst_info_mtbuf_count 8
#define si_inst_info_mubuf_count 114
#define si_inst_info_mimg_count 97
#define si_inst_info_exp_count  1

CLASS_BEGIN(SIAsm, Asm)

	/* Array containing 'SIInstOpcodeCount' elements with information about
	 * the Southern Islands instructions. */
	SIInstInfo *inst_info;

	/* Pointers to elements in 'inst_info' */
	SIInstInfo *inst_info_sopp[si_inst_info_sopp_count];
	SIInstInfo *inst_info_sopc[si_inst_info_sopc_count];
	SIInstInfo *inst_info_sop1[si_inst_info_sop1_count];
	SIInstInfo *inst_info_sopk[si_inst_info_sopk_count];
	SIInstInfo *inst_info_sop2[si_inst_info_sop2_count];
	SIInstInfo *inst_info_smrd[si_inst_info_smrd_count];
	SIInstInfo *inst_info_vop3[si_inst_info_vop3_count];
	SIInstInfo *inst_info_vopc[si_inst_info_vopc_count];
	SIInstInfo *inst_info_vop1[si_inst_info_vop1_count];
	SIInstInfo *inst_info_vop2[si_inst_info_vop2_count];
	SIInstInfo *inst_info_vintrp[si_inst_info_vintrp_count];
	SIInstInfo *inst_info_ds[si_inst_info_ds_count];
	SIInstInfo *inst_info_mtbuf[si_inst_info_mtbuf_count];
	SIInstInfo *inst_info_mubuf[si_inst_info_mubuf_count];
	SIInstInfo *inst_info_mimg[si_inst_info_mimg_count];
	SIInstInfo *inst_info_exp[si_inst_info_exp_count];

CLASS_END(SIAsm)


void SIAsmCreate(SIAsm *self);
void SIAsmDestroy(SIAsm *self);

void SIAsmDisassembleBinary(SIAsm *self, char *path);
void SIAsmDisassembleOpenGLBinary(SIAsm *self, char *path, int shader_index);

#endif

