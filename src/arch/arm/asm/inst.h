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

#ifndef ARM_ASM_INST_H
#define ARM_ASM_INST_H

#include <lib/class/class.h>


/*
 * Class 'ARMInst'
 */
typedef struct
{
	unsigned int op2 	: 12; /* [11:0] */
	unsigned int dst_reg 	: 4; /* [15:12] */
	unsigned int op1_reg 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int opc_dpr 	: 4; /* [24:21] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved0: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesDpr;


typedef struct
{
	unsigned int op2 	: 12; /* [11:0] */
	unsigned int dst_reg 	: 4; /* [15:12] */
	unsigned int op1_reg 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int opc_dpr 	: 4; /* [24:21] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved0: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesDprSat;


typedef struct
{
	unsigned int op2 	: 12; /* [11:0] */
	unsigned int dst_reg 	: 4; /* [15:12] */
	unsigned int __reserved0: 6; /* [21:16] */
	unsigned int psr_loc	: 1; /* [22] */
	unsigned int __reserved1: 2; /* [24:23] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved2: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesPsr;


typedef struct
{
	unsigned int op0_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 4; /* [7:4] */
	unsigned int op1_rs 	: 4; /* [11:8] */
	unsigned int op2_rn 	: 4; /* [15:12] */
	unsigned int dst_rd 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int m_acc 	: 1; /* [21] */
	unsigned int __reserved1: 6; /* [27:22] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesMult;


typedef struct
{
	unsigned int op0_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 4; /* [7:4] */
	unsigned int op1_rs 	: 4; /* [11:8] */
	unsigned int dst_lo 	: 4; /* [15:12] */
	unsigned int dst_hi 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int m_acc 	: 1; /* [21] */
	unsigned int sign 	: 1; /* [22] */
	unsigned int __reserved1: 5; /* [27:23] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesMultLn;


typedef struct
{
	unsigned int op0_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 8; /* [11:4] */
	unsigned int dst_rd 	: 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int __reserved1: 2; /* [21:20] */
	unsigned int d_type 	: 1; /* [22] */
	unsigned int __reserved2: 5; /* [27:23] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesSnglDswp;


typedef struct
{
	unsigned int op0_rn 	: 4; /* [3:0] */
	unsigned int __reserved0: 24; /* [27:4] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesBax;


typedef struct
{
	unsigned int off_reg 	: 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int sh_comb 	: 2; /* [6:5] */
	unsigned int __reserved1: 5; /* [11:7] */
	unsigned int dst_rd 	: 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int __reserved2: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int __reserved3: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesHfwrdTnsReg;


typedef struct
{
	unsigned int imm_off_lo : 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int sh_comb 	: 2; /* [6:5] */
	unsigned int __reserved1: 1; /* [7] */
	unsigned int imm_off_hi : 4; /* [11:8] */
	unsigned int dst_rd 	: 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int __reserved2: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int __reserved3: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesHfwrdTnsImm;


typedef struct
{
	unsigned int off 	: 12; /* [11:0] */
	unsigned int src_dst_rd : 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int d_type 	: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved0: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesSdtr;


typedef struct
{
	unsigned int reg_lst 	: 16; /* [15:0] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int psr_frc 	: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int __reserved0: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesBdtr;


typedef struct
{
	signed int off 		: 24; /* [23:0] */
	unsigned int link 	: 1; /* [24] */
	unsigned int __reserved0: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesBrnch;


typedef struct
{
	unsigned int off 	: 8; /* [7:0] */
	unsigned int cpr_num 	: 4; /* [11:8] */
	unsigned int cpr_sr_dst : 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int tr_len 	: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ	: 1; /* [24] */
	unsigned int __reserved0: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesCprDtr;


typedef struct
{
	unsigned int cpr_op_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int cpr_info 	: 3; /* [7:5] */
	unsigned int cpr_num 	: 4; /* [11:8] */
	unsigned int cpr_dst 	: 4; /* [15:12] */
	unsigned int cpr_rn 	: 4; /* [19:16] */
	unsigned int cpr_opc 	: 4; /* [23:20] */
	unsigned int __reserved1: 4; /* [27:24] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesCprDop;


typedef struct
{
	unsigned int cpr_op_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int cpr_info 	: 3; /* [7:5] */
	unsigned int cpr_num 	: 4; /* [11:8] */
	unsigned int rd 	: 4; /* [15:12] */
	unsigned int cpr_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int cpr_opm 	: 1; /* [23:21] */
	unsigned int __reserved1: 2; /* [27:24] */
	unsigned int cond 	: 4; /* [31:28] */
} ARMInstBytesCprRtr;


typedef struct
{
	unsigned int cmnt 	: 24; /* [23:0] */
	unsigned int __reserved0: 4;  /* [27:24] */
	unsigned int cond 	: 4;  /* [31:28] */
} ARMInstBytesSwiSvc;


typedef struct
{
	unsigned int immd8	: 8; /* [7:0] */
	unsigned int __reserved0: 4; /* [11:8] */
	unsigned int vd		: 4; /* [15:12] */
	unsigned int vfp_rn	: 4; /* [19:16] */
	unsigned int __reserved1: 1; /* [20] */
	unsigned int w		: 1; /* [21] */
	unsigned int d		: 1; /* [22] */
	unsigned int u		: 1; /* [23] */
	unsigned int p		: 1; /* [24] */
	unsigned int __reserved2: 3; /* [27:25] */
	unsigned int cond	: 4; /* [31:28] */
} ARMInstBytesVfpMv;


typedef struct
{
	unsigned int __reserved0:12; /* [11:0] */
	unsigned int vfp_rt	: 4; /* [15:12] */
	unsigned int __reserved1:12; /* [27:16] */
	unsigned int cond	: 4; /* [31:28] */
} ARMInstBytesVfpStregTr;


typedef union
{
	unsigned char bytes[4];
	unsigned int word;

	ARMInstBytesDpr dpr;
	ARMInstBytesDprSat dpr_sat;
	ARMInstBytesPsr psr;
	ARMInstBytesMult mult;
	ARMInstBytesMultLn mult_ln;
	ARMInstBytesSnglDswp sngl_dswp;
	ARMInstBytesBax bax;
	ARMInstBytesHfwrdTnsReg hfwrd_reg;
	ARMInstBytesHfwrdTnsImm hfwrd_imm;
	ARMInstBytesSdtr sdtr;
	ARMInstBytesBdtr bdtr;
	ARMInstBytesBrnch brnch;
	ARMInstBytesCprDtr cpr_dtr;
	ARMInstBytesCprDop cpr_dop;
	ARMInstBytesCprRtr cpr_rtr;
	ARMInstBytesSwiSvc swi_svc;
	ARMInstBytesVfpMv vfp_mv;
	ARMInstBytesVfpStregTr vfp_strreg_tr;
} ARMInstBytes;


typedef enum
{
	ARMInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	ARM_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	ARMInstOpcodeCount

} ARMInstOpcode;

typedef enum
{
	ARMInstCategoryInvalid = 0,

	ARMInstCategoryDprReg, 	/* Data Processing Register instructions */
	ARMInstCategoryDprImm,	/* Data Processing Immediate instructions */
	ARMInstCategoryDprSat, 	/* Data Processing saturation instructions */
	ARMInstCategoryPsr,  /* Processor Status Register (PSR) instructions */
	ARMInstCategoryMult,  /* Multiplication instructions */
	ARMInstCategoryMultSign,  /* Multiplication instructions */
	ARMInstCategoryMultLn,  /* Long Multiplication instructions */
	ARMInstCategoryMultLnSign,  /* Long Multiplication instructions */
	ARMInstCategorySdswp,  /* Single Data Swap instructions */
	ARMInstCategoryBax,  /* Branch and Exchange instructions */
	ARMInstCategoryHfwrdReg,  /* Halfword data transfer Register offset */
	ARMInstCategoryHfwrdImm,  /* Halfword data transfer Immediate offset */
	ARMInstCategorySdtr,  /* Single Data Transfer instructions */
	ARMInstCategoryBdtr,  /* Block Data Transfer instructions */
	ARMInstCategoryBrnch,  /* Branch instructions */
	ARMInstCategoryCprDtr,  /* Coprocessor Data Transfer instructions */
	ARMInstCategoryCprDop,  /* Coprocessor Data Operation instructions */
	ARMInstCategoryCprRtr,  /* Coprocessor Register Transfer instructions */
	ARMInstCategorySwiSvc,  /* Software Interrupt / SVC Angel trap instructions */
	ARMInstCategoryVfp,  /* Vector Floating Point Instructions */

	ARMInstCategoryUndef,

	ARMInstCategoryCount

} ARMInstCategory;


CLASS_BEGIN(ARMInst, Object)

	/* Disassembler */
	ARMAsm *as;

	unsigned int addr;
	ARMInstBytes bytes;

	/* Information after decoded */
	struct arm_inst_info_t *info;

CLASS_END(ARMInst)

void ARMInstCreate(ARMInst *self, ARMAsm *as);
void ARMInstDestroy(ARMInst *self);

void ARMInstDump(ARMInst *self, FILE *f);
void ARMInstDumpBuf(ARMInst *self, char *buf, int size);

/* Populate the 'info' field of an instruction by decoding the instruction
 * in 'buf'. The first 4 bytes of 'buf' are copied to field 'bytes'. The value
 * given in 'addr' is the instruction's virtual address, used for branch
 * decoding purposes. */
void ARMInstDecode(ARMInst *self, unsigned int addr, void *buf);

#endif

