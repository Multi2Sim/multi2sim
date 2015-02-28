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

#ifndef ARM_ASM_INST_H_
#define ARM_ASM_INST_H_

#include <lib/class/class.h>


/*
 * Public
 */



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



/*
 * Structure of Instruction Format (Thumb2-32bit)
 */

typedef struct
{
	unsigned int reglist	: 16;	/* [15:0] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved0: 1; 	/* [20] */
	unsigned int wback	: 1;	/* [21] */
	unsigned int __reserved1: 10; 	/* [31:22] */
}ARMInstThumb32BytesLdStMult;

typedef struct
{
	unsigned int reglist	: 16;	/* [15:0] */
	unsigned int __reserved0: 16; 	/* [31:16] */
}ARMInstThumb32BytesPushPop;

typedef struct
{
	unsigned int immd8	: 8;	/* [7:0] */
	unsigned int rt2	: 4;	/* [11:8] */
	unsigned int rt		: 4; 	/* [15:12] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved0: 1; 	/* [20] */
	unsigned int wback	: 1;	/* [21] */
	unsigned int __reserved1: 1; 	/* [22] */
	unsigned int add_sub	: 1;	/* [23] */
	unsigned int index	: 1;	/* [24] */
	unsigned int __reserved2: 7; 	/* [31:25] */
}ARMInstThumb32BytesLdStDouble;

typedef struct
{
	unsigned int rm		: 4; 	/* [3:0] */
	unsigned int h		: 1; 	/* [4] */
	unsigned int __reserved0: 11; 	/* [15:5] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
}ARMInstThumb32BytesTableBranch;

typedef struct
{
	unsigned int rm		: 4; 	/* [3:0] */
	unsigned int type	: 2; 	/* [5:4] */
	unsigned int imm2	: 2; 	/* [7:6] */
	unsigned int rd		: 4; 	/* [11:8] */
	unsigned int imm3	: 3; 	/* [14:12] */
	unsigned int __reserved0: 1; 	/* [15] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int sign	: 1; 	/* [20] */
	unsigned int __reserved1: 11; 	/* [31:21] */
}ARMInstThumb32BytesDataProcShftreg;

typedef struct
{
	unsigned int immd8	: 8; 	/* [7:0] */
	unsigned int rd		: 4; 	/* [11:8] */
	unsigned int immd3	: 3; 	/* [14:12] */
	unsigned int __reserved0: 1; 	/* [15] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int sign	: 1; 	/* [20] */
	unsigned int __reserved1: 5; 	/* [25:21] */
	unsigned int i_flag	: 1; 	/* [26] */
	unsigned int __reserved2: 5; 	/* [31:27] */
}ARMInstThumb32BytesDataProcImmd;

typedef struct
{
	unsigned int immd11	: 11; 	/* [10:0] */
	unsigned int j2		: 1; 	/* [11] */
	unsigned int __reserved0: 1; 	/* [12] */
	unsigned int j1		: 1; 	/* [13] */
	unsigned int __reserved1: 2; 	/* [15:14] */
	unsigned int immd6	: 6; 	/* [21:16] */
	unsigned int cond	: 4; 	/* [25:22] */
	unsigned int sign	: 1; 	/* [26] */
	unsigned int __reserved2: 5; 	/* [31:27] */
}ARMInstThumb32BytesBranch;

typedef struct
{
	unsigned int immd11	: 11; 	/* [10:0] */
	unsigned int j2		: 1; 	/* [11] */
	unsigned int __reserved0: 1; 	/* [12] */
	unsigned int j1		: 1; 	/* [13] */
	unsigned int __reserved1: 2; 	/* [15:14] */
	unsigned int immd10	: 10; 	/* [25:16] */
	unsigned int sign	: 1; 	/* [26] */
	unsigned int __reserved2: 5; 	/* [31:27] */
}ARMInstThumb32BytesBranchLink;


typedef struct
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int immd2	: 2;	/* [5:4] */
	unsigned int __reserved0: 6; 	/* [11:6] */
	unsigned int rd		: 4;	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
}ARMInstThumb32BytesLdStrReg;

typedef struct
{
	unsigned int immd12	: 12;	/* [11:0] */
	unsigned int rd		: 4;	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved0: 3; 	/* [22:20] */
	unsigned int add	: 1;	/* [23] */
	unsigned int __reserved1: 8; 	/* [31:24] */
}ARMInstThumb32BytesLdStrImm;

typedef struct
{
	unsigned int immd8	: 8;	/* [7:0] */
	unsigned int __reserved0: 4; 	/* [11:8] */
	unsigned int rd		: 4;	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
}ARMInstThumb32BytesLdStrtImm;

typedef struct
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int rot	: 2;	/* [5:4] */
	unsigned int __reserved0: 2; 	/* [7:6] */
	unsigned int rd		: 4;	/* [11:8] */
	unsigned int __reserved1: 4; 	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int sign	: 1;	/* [20] */
	unsigned int __reserved2: 11; 	/* [31:21] */
}ARMInstThumb32BytesDprocReg;

typedef struct
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int __reserved0: 4; 	/* [7:4] */
	unsigned int rd		: 4; 	/* [11:8] */
	unsigned int ra		: 4; 	/* [15:12] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
}ARMInstThumb32BytesMult;

typedef struct
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int __reserved0: 4; 	/* [7:4] */
	unsigned int rdhi	: 4; 	/* [11:8] */
	unsigned int rdlo	: 4; 	/* [15:12] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
}ARMInstThumb32BytesMultLong;

typedef struct
{
	unsigned int msb	: 5;	/* [4:0] */
	unsigned int __reserved0: 1; 	/* [5] */
	unsigned int immd2	: 2; 	/* [7:6] */
	unsigned int rd		: 4; 	/* [11:8] */
	unsigned int immd3	: 3; 	/* [14:9] */
	unsigned int __reserved1: 1; 	/* [15] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved2: 12; 	/* [31:20] */
}ARMInstThumb32BytesBitField;


/*
 * Structure of Intypedef struction Format (Thumb2-16bit)
 */

typedef struct
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 3;	/* [5:3] */
	unsigned int offset	: 5;	/* [10:6] */
	unsigned int op_int	: 2; 	/* [12:11] */
	unsigned int __reserved0: 3; 	/* [15:13] */

}ARMInstThumb16BytesMovshiftReg;

typedef struct
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 3;	/* [5:3] */
	unsigned int rn_imm	: 3;	/* [8:6] */
	unsigned int op_int	: 1;	/* [9] */
	unsigned int immd	: 1;	/* [10] */
	unsigned int __reserved0: 5; 	/* [15:11] */
}ARMInstThumb16BytesAddSub;

typedef struct
{
	unsigned int offset8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3;	/* [10:8] */
	unsigned int op_int	: 2;	/* [12:11] */
	unsigned int __reserved0: 3; 	/* [15:13] */
}ARMInstThumb16BytesImmdOprs;

typedef struct
{
	unsigned int reg_rn	: 3;	/* [2:0] */
	unsigned int reg_rm	: 4;	/* [6:3] */
	unsigned int N		: 1;	/* [7] */
	unsigned int __reserved0: 8; 	/* [15:8] */
}ARMInstThumb16BytesCmpT2;

typedef struct
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 3;	/* [5:3] */
	unsigned int op_int	: 4;	/* [9:6] */
	unsigned int __reserved0: 6; 	/* [15:10] */
}ARMInstThumb16BytesDpr;

typedef struct
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 4;	/* [6:3] */
	unsigned int h1		: 1;	/* [7] */
	unsigned int op_int	: 2;	/* [9:8] */
	unsigned int __reserved0: 6; 	/* [15:10] */
}ARMInstThumb16BytesHighRegOprs;

typedef struct
{
	unsigned int immd_8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3; 	/* [10:8] */
	unsigned int __reserved0: 5; 	/* [15:11] */
}ARMInstThumb16BytesPcLdr;

typedef struct
{

	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int reg_ro	: 3; 	/* [8:6] */
	unsigned int __reserved0: 1; 	/* [9] */
	unsigned int byte_wrd	: 1; 	/* [10] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int __reserved1: 4; 	/* [15:12] */
}ARMInstThumb16BytesLdStrReg;

typedef struct
{

	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int reg_ro	: 3; 	/* [8:6] */
	unsigned int __reserved0: 1; 	/* [9] */
	unsigned int sign_ext	: 1; 	/* [10] */
	unsigned int h_flag	: 1; 	/* [11] */
	unsigned int __reserved1: 4; 	/* [15:12] */
}ARMInstThumb16BytesLdStrExts;

typedef struct
{
	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int offset	: 5;	/* [10:6] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int byte_wrd	: 1; 	/* [12] */
	unsigned int __reserved0: 3; 	/* [15:13] */
}ARMInstThumb16BytesLdStrImmd;

typedef struct
{
	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int offset	: 5;	/* [10:6] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int __reserved0: 4; 	/* [15:12] */
}ARMInstThumb16BytesLdStrHfwrd;

typedef struct
{
	unsigned int immd_8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3; 	/* [10:8] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int __reserved0: 4; 	/* [15:12] */
}ARMInstThumb16BytesLdStrSpImmd;

typedef struct
{
	unsigned int immd_8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3; 	/* [10:8] */
	unsigned int __reserved0: 5; 	/* [15:11] */
}ARMInstThumb16BytesMiscAddsp;

typedef struct
{
	unsigned int immd_8	: 7;	/* [7:0] */
	unsigned int __reserved0: 9; 	/* [15:8] */
}ARMInstThumb16BytesMiscSubsp;

typedef struct
{
	unsigned int reg_rn	: 3;	/* [2:0] */
	unsigned int immd_5	: 5;	/* [7:3] */
	unsigned int __reserved0: 1; 	/* [8] */
	unsigned int i_ext	: 1;	/* [9] */
	unsigned int __reserved1: 1; 	/* [10] */
	unsigned int op_int	: 1;	/* [11] */
	unsigned int __reserved2: 4; 	/* [15:12] */
}ARMInstThumb16BytesMiscCbnz;

typedef struct
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rm	: 3;	/* [5:3] */
	unsigned int __reserved0: 10; 	/* [15:6] */
}ARMInstThumb16BytesMiscExtnd;

typedef struct
{
	unsigned int reg_list	: 8;	/* [7:0] */
	unsigned int m_ext	: 1;	/* [8] */
	unsigned int __reserved0: 7; 	/* [15:9] */
}ARMInstThumb16BytesMiscPushPop;

typedef struct
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rm	: 3;	/* [5:3] */
	unsigned int __reserved0: 10; 	/* [15:6] */
}ARMInstThumb16BytesMiscRev;

typedef struct
{
	unsigned int mask	: 4;	/* [3:0] */
	unsigned int first_cond	: 4;	/* [7:4] */
	unsigned int __reserved0: 8;	/* [15:8] */
}ARMInstThumb16BytesIfThen;

typedef struct
{
	unsigned int reg_list	: 8;	/* [7:0] */
	unsigned int reg_rb	: 3;	/* [10:8] */
	unsigned int ld_st	: 1;	/* [11] */
	unsigned int __reserved0: 4;	/* [15:12] */
}ARMInstThumb16BytesLdmStm;

typedef struct
{
	unsigned int value	: 8;	/* [7:0] */
	unsigned int __reserved0: 8;	/* [15:8] */
}ARMInstThumb16BytesSvc;

typedef struct
{
	unsigned int s_offset	: 8;	/* [7:0] */
	unsigned int cond	: 4;	/* [11:8] */
	unsigned int __reserved0: 4;	/* [15:12] */
}ARMInstThumb16BytesCondBr;

typedef struct
{
	unsigned int immd11	: 11;	/* [10:0] */
	unsigned int __reserved0: 5;	/* [15:11] */
}ARMInstThumb16BytesBr;



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

typedef union
{
	unsigned char bytes[4];
	unsigned int word;

	ARMInstThumb32BytesLdStMult ld_st_mult;
	ARMInstThumb32BytesPushPop push_pop;
	ARMInstThumb32BytesLdStDouble ld_st_double;
	ARMInstThumb32BytesTableBranch table_branch;
	ARMInstThumb32BytesDataProcShftreg data_proc_shftreg;
	ARMInstThumb32BytesDataProcImmd data_proc_immd;
	ARMInstThumb32BytesBranch branch;
	ARMInstThumb32BytesBranchLink branch_link;
	ARMInstThumb32BytesLdStrReg ldstr_reg;
	ARMInstThumb32BytesLdStrImm ldstr_imm;
	ARMInstThumb32BytesLdStrtImm ldstrt_imm;
	ARMInstThumb32BytesDprocReg dproc_reg;
	ARMInstThumb32BytesMult mult;
	ARMInstThumb32BytesMultLong mult_long;
	ARMInstThumb32BytesBitField bit_field;
	ARMInstThumb16BytesMiscAddsp add_sp;

} ARMInstThumb32Bytes;

typedef union
{
	unsigned char bytes[2];

	 ARMInstThumb16BytesMovshiftReg movshift_reg_ins;
	 ARMInstThumb16BytesAddSub addsub_ins;
	 ARMInstThumb16BytesImmdOprs immd_oprs_ins;
	 ARMInstThumb16BytesDpr dpr_ins;
	 ARMInstThumb16BytesHighRegOprs high_oprs_ins;
	 ARMInstThumb16BytesPcLdr pcldr_ins;
	 ARMInstThumb16BytesLdStrReg ldstr_reg_ins;
	 ARMInstThumb16BytesLdStrExts ldstr_exts_ins;
	 ARMInstThumb16BytesLdStrImmd ldstr_immd_ins;
	 ARMInstThumb16BytesLdStrHfwrd ldstr_hfwrd_ins;
	 ARMInstThumb16BytesLdStrSpImmd sp_immd_ins;
	 ARMInstThumb16BytesMiscAddsp addsp_ins;
	 ARMInstThumb16BytesMiscSubsp sub_sp_ins;
	 ARMInstThumb16BytesMiscCbnz cbnz_ins;
	 ARMInstThumb16BytesMiscExtnd misc_extnd_ins;
	 ARMInstThumb16BytesMiscPushPop push_pop_ins;
	 ARMInstThumb16BytesMiscRev rev_ins;
	 ARMInstThumb16BytesIfThen if_eq_ins;
	 ARMInstThumb16BytesLdmStm ldm_stm_ins;
	 ARMInstThumb16BytesSvc svc_ins;
	 ARMInstThumb16BytesCondBr cond_br_ins;
	 ARMInstThumb16BytesCmpT2 cmp_t2;
	 ARMInstThumb16BytesBr br_ins;

}ARMInstThumb16Bytes;


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
	ARMInstThumb16OpcodeInvalid = 0,

#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
	ARM_THUMB16_INST_##_name,
#include "asm-thumb.dat"
#undef DEFINST

	/* Max */
	ARMInstThumb16OpcodeCount
}ARMInstThumb16Opcode;



typedef enum
{
	ARMInstThumb32OpcodeInvalid = 0,
#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
	ARM_THUMB32_INST_##_name,
#include "asm-thumb32.dat"
#undef DEFINST
	/* Max */
	ARMInstThumb32OpcodeCount
}ARMInstThumb32Opcode;


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

typedef enum
{
	ARM_THUMB16_CAT_NONE = 0,

	ARM_THUMB16_CAT_MOVSHIFT_REG,	/* Move Shift Instructions Register Based */
	ARM_THUMB16_CAT_ADDSUB,		/* Addition and Subtraction Instructions */
	ARM_THUMB16_CAT_IMMD_OPRS,	/* Operations with Immediate Operations */
	ARM_THUMB16_CAT_DPR_INS,	/* Data Processing Operations */
	ARM_THUMB16_CAT_HI_REG_OPRS,	/* High Register Operations */
	ARM_THUMB16_CAT_PC_LDR,		/* LDR operation over PC */
	ARM_THUMB16_CAT_LDSTR_REG,	/* Load Store with Register Offset */
	ARM_THUMB16_CAT_LDSTR_EXTS,	/* Sign Extended Load Store */
	ARM_THUMB16_CAT_LDSTR_IMMD,	/* Load Store with Immediate Offset */
	ARM_THUMB16_CAT_LDSTR_HFWRD,	/* Load Store Half Word */
	ARM_THUMB16_CAT_LDSTR_SP_IMMD,	/* Load Store SP Related with Immediate Offset */
	ARM_THUMB16_CAT_IF_THEN,	/* If Then Block instructions */
	ARM_THUMB16_CAT_LDM_STM,	/* Load/Store Multiple Instructions */
	ARM_THUMB16_CAT_MISC_ADDSP_INS,	/* Miscellaneous Instructions ADD SP relative*/
	ARM_THUMB16_CAT_MISC_SVC_INS,	/* Miscellaneous Instructions SVC instructions */
	ARM_THUMB16_CAT_MISC_BR,	/* Miscellaneous Instructions Conditional Branch */
	ARM_THUMB16_CAT_MISC_UCBR,	/* Miscellaneous Instructions Conditional Branch */
	ARM_THUMB16_CAT_MISC_REV,	/* Miscellaneous Reverse instructions */
	ARM_THUMB16_CAT_MISC_SUBSP_INS,	/* Miscellaneous Instructions SUB SP relative*/
	ARM_THUMB16_CAT_MISC_PUSH_POP,	/* Miscellaneous Instructions PUSH and POP*/
	ARM_THUMB16_CAT_MISC_CBNZ,	/* Miscellaneous Instructions CB{N}Z*/
	ARM_THUMB16_CAT_CMP_T2,		/* Miscellaneous Instructions CB{N}Z*/
	ARM_THUMB16_CAT_UNDEF,

	ARM_THUMB16_CAT_COUNT

}ARMThumb16InstCategory;

typedef enum
{
	ARM_THUMB32_CAT_NONE = 0,

	ARM_THUMB32_CAT_LD_ST_MULT,	/* Load Store Multiple */
	ARM_THUMB32_CAT_LD_ST_DOUBLE,	/* Load Store Double Exclusive */
	ARM_THUMB32_CAT_PUSH_POP,	/* Push Pop Multiple */
	ARM_THUMB32_CAT_TABLE_BRNCH,	/* Table Branch Byte */
	ARM_THUMB32_CAT_DPR_SHFTREG,	/* Data processing Shifted register */
	ARM_THUMB32_CAT_DPR_IMM,	/* Data processing immediate */
	ARM_THUMB32_CAT_DPR_BIN_IMM,	/* Data processing binary immediate */
	ARM_THUMB32_CAT_BRANCH,		/* Branch */
	ARM_THUMB32_CAT_BRANCH_LX,		/* Branch with Link exchange*/
	ARM_THUMB32_CAT_BRANCH_COND,	/* Branch Conditional */
	ARM_THUMB32_CAT_LDSTR_BYTE,	/* Load Store Register Byte/Halfword */
	ARM_THUMB32_CAT_LDSTR_REG,	/* Load Store Register */
	ARM_THUMB32_CAT_LDSTR_IMMD,	/* Load Store Immediate */
	ARM_THUMB32_CAT_DPR_REG,	/* Data Processing Register */
	ARM_THUMB32_CAT_MULT,		/* Multiply */
	ARM_THUMB32_CAT_MULT_LONG,	/* Multiply Long*/
	ARM_THUMB32_CAT_BIT_FIELD,	/* Multiply Long*/
	ARM_THUMB32_CAT_MOV_IMMD,	/* Immediate Move*/
	ARM_THUMB32_CAT_UNDEF,

	ARM_THUMB32_CAT_COUNT

}ARMThumb32InstCategory;


CLASS_BEGIN(ARMInst, Object)

	/* Disassembler */
	ARMAsm *as;

	unsigned int addr;
	ARMInstBytes dword;
	ARMInstThumb16Bytes dword_16;
	ARMInstThumb32Bytes dword_32;
	/* Information after decoded */
	struct arm_inst_info_t *info;
	struct arm_thumb16_inst_info_t *info_16;
	struct arm_thumb32_inst_info_t *info_32;

CLASS_END(ARMInst)

void ARMInstCreate(ARMInst *self, ARMAsm *as);
void ARMInstDestroy(ARMInst *self);

void ARMInstDump(ARMInst *self, FILE *f);
void ARMInstDumpBuf(ARMInst *self, char *buf, int size);

void ARMInstThumb16Dump(ARMInst *self, FILE *f);
void ARMInstThumb16DumpBuf(ARMInst *self, char *buf, int size);

void ARMInstThumb32Dump(ARMInst *self, FILE *f);
void ARMInstThumb32DumpBuf(ARMInst *self, char *buf, int size);

/* Populate the 'info' field of an instruction by decoding the instruction
 * in 'buf'. The first 4 bytes of 'buf' are copied to field 'bytes'. The value
 * given in 'addr' is the instruction's virtual address, used for branch
 * decoding purposes. */
void ARMInstDecode(volatile ARMInst *self, unsigned int addr, void *buf);
void ARMInstThumb16Decode(void *buf, unsigned int ip, volatile ARMInst *inst);
void ARMInstThumb32Decode(void *buf, unsigned int ip, volatile ARMInst *inst);
void ARMThumb16InstTableDecode(volatile ARMInst *inst, ARMAsm *as);
void ARMThumb32InstTableDecode(volatile ARMInst *inst, ARMAsm *as);

#endif

