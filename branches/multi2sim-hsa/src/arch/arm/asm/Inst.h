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

#ifndef ARCH_ARM_ASM_INST_H
#define ARCH_ARM_ASM_INST_H

#include <iostream>

namespace ARM
{

// Forward declarations
class Asm;


struct InstBytesDpr
{
	unsigned int op2 	: 12;  // [11:0]
	unsigned int dst_reg 	: 4;   // [15:12]
	unsigned int op1_reg 	: 4;   // [19:16]
	unsigned int s_cond 	: 1;   // [20]
	unsigned int opc_dpr 	: 4;   // [24:21]
	unsigned int imm 	: 1;   // [25]
	unsigned int __reserved0: 2;   // [27:26]
	unsigned int cond 	: 4;   // [31:28]
};


struct InstBytesDprSat
{
	unsigned int op2 	: 12;  // [11:0]
	unsigned int dst_reg 	: 4;   // [15:12]
	unsigned int op1_reg 	: 4;   // [19:16]
	unsigned int s_cond 	: 1;   // [20]
	unsigned int opc_dpr 	: 4;   // [24:21]
	unsigned int imm 	: 1;   // [25]
	unsigned int __reserved0: 2;   // [27:26]
	unsigned int cond 	: 4;   // [31:28]
};


struct InstBytesPsr
{
	unsigned int op2 	: 12;  // [11:0]
	unsigned int dst_reg 	: 4;   // [15:12]
	unsigned int __reserved0: 6;   // [21:16]
	unsigned int psr_loc	: 1;   // [22]
	unsigned int __reserved1: 2;   // [24:23]
	unsigned int imm 	: 1;   // [25]
	unsigned int __reserved2: 2;   // [27:26]
	unsigned int cond 	: 4;   // [31:28]
};


struct InstBytesMult
{
	unsigned int op0_rm 	: 4;  // [3:0]
	unsigned int __reserved0: 4;  // [7:4]
	unsigned int op1_rs 	: 4;  // [11:8]
	unsigned int op2_rn 	: 4;  // [15:12]
	unsigned int dst_rd 	: 4;  // [19:16]
	unsigned int s_cond 	: 1;  // [20]
	unsigned int m_acc 	: 1;  // [21]
	unsigned int __reserved1: 6;  // [27:22]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesMultLn
{
	unsigned int op0_rm 	: 4;  // [3:0]
	unsigned int __reserved0: 4;  // [7:4]
	unsigned int op1_rs 	: 4;  // [11:8]
	unsigned int dst_lo 	: 4;  // [15:12]
	unsigned int dst_hi 	: 4;  // [19:16]
	unsigned int s_cond 	: 1;  // [20]
	unsigned int m_acc 	: 1;  // [21]
	unsigned int sign 	: 1;  // [22]
	unsigned int __reserved1: 5;  // [27:23]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesSnglDswp
{
	unsigned int op0_rm 	: 4;  // [3:0]
	unsigned int __reserved0: 8;  // [11:4]
	unsigned int dst_rd 	: 4;  // [15:12]
	unsigned int base_rn 	: 4;  // [19:16]
	unsigned int __reserved1: 2;  // [21:20]
	unsigned int d_type 	: 1;  // [22]
	unsigned int __reserved2: 5;  // [27:23]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesBax
{
	unsigned int op0_rn 	: 4;  // [3:0]
	unsigned int __reserved0: 24;  // [27:4]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesHfwrdTnsReg
{
	unsigned int off_reg 	: 4;  // [3:0]
	unsigned int __reserved0: 1;  // [4]
	unsigned int sh_comb 	: 2;  // [6:5]
	unsigned int __reserved1: 5;  // [11:7]
	unsigned int dst_rd 	: 4;  // [15:12]
	unsigned int base_rn 	: 4;  // [19:16]
	unsigned int ld_st 	: 1;  // [20]
	unsigned int wb 	: 1;  // [21]
	unsigned int __reserved2: 1;  // [22]
	unsigned int up_dn 	: 1;  // [23]
	unsigned int idx_typ 	: 1;  // [24]
	unsigned int __reserved3: 3;  // [27:25]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesHfwrdTnsImm
{
	unsigned int imm_off_lo : 4;  // [3:0]
	unsigned int __reserved0: 1;  // [4]
	unsigned int sh_comb 	: 2;  // [6:5]
	unsigned int __reserved1: 1;  // [7]
	unsigned int imm_off_hi : 4;  // [11:8]
	unsigned int dst_rd 	: 4;  // [15:12]
	unsigned int base_rn 	: 4;  // [19:16]
	unsigned int ld_st 	: 1;  // [20]
	unsigned int wb 	: 1;  // [21]
	unsigned int __reserved2: 1;  // [22]
	unsigned int up_dn 	: 1;  // [23]
	unsigned int idx_typ 	: 1;  // [24]
	unsigned int __reserved3: 3;  // [27:25]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesSdtr
{
	unsigned int off 	: 12;  // [11:0]
	unsigned int src_dst_rd : 4;  // [15:12]
	unsigned int base_rn 	: 4;  // [19:16]
	unsigned int ld_st 	: 1;  // [20]
	unsigned int wb 	: 1;  // [21]
	unsigned int d_type 	: 1;  // [22]
	unsigned int up_dn 	: 1;  // [23]
	unsigned int idx_typ 	: 1;  // [24]
	unsigned int imm 	: 1;  // [25]
	unsigned int __reserved0: 2;  // [27:26]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesBdtr
{
	unsigned int reg_lst 	: 16;  // [15:0]
	unsigned int base_rn 	: 4;  // [19:16]
	unsigned int ld_st 	: 1;  // [20]
	unsigned int wb 	: 1;  // [21]
	unsigned int psr_frc 	: 1;  // [22]
	unsigned int up_dn 	: 1;  // [23]
	unsigned int idx_typ 	: 1;  // [24]
	unsigned int __reserved0: 3;  // [27:25]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesBrnch
{
	signed int off 		: 24;  // [23:0]
	unsigned int link 	: 1;  // [24]
	unsigned int __reserved0: 3;  // [27:25]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesCprDtr
{
	unsigned int off 	: 8;  // [7:0]
	unsigned int cpr_num 	: 4;  // [11:8]
	unsigned int cpr_sr_dst : 4;  // [15:12]
	unsigned int base_rn 	: 4;  // [19:16]
	unsigned int ld_st 	: 1;  // [20]
	unsigned int wb 	: 1;  // [21]
	unsigned int tr_len 	: 1;  // [22]
	unsigned int up_dn 	: 1;  // [23]
	unsigned int idx_typ	: 1;  // [24]
	unsigned int __reserved0: 3;  // [27:25]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesCprDop
{
	unsigned int cpr_op_rm 	: 4;  // [3:0]
	unsigned int __reserved0: 1;  // [4]
	unsigned int cpr_info 	: 3;  // [7:5]
	unsigned int cpr_num 	: 4;  // [11:8]
	unsigned int cpr_dst 	: 4;  // [15:12]
	unsigned int cpr_rn 	: 4;  // [19:16]
	unsigned int cpr_opc 	: 4;  // [23:20]
	unsigned int __reserved1: 4;  // [27:24]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesCprRtr
{
	unsigned int cpr_op_rm 	: 4;  // [3:0]
	unsigned int __reserved0: 1;  // [4]
	unsigned int cpr_info 	: 3;  // [7:5]
	unsigned int cpr_num 	: 4;  // [11:8]
	unsigned int rd 	: 4;  // [15:12]
	unsigned int cpr_rn 	: 4;  // [19:16]
	unsigned int ld_st 	: 1;  // [20]
	unsigned int cpr_opm 	: 1;  // [23:21]
	unsigned int __reserved1: 2;  // [27:24]
	unsigned int cond 	: 4;  // [31:28]
};


struct InstBytesSwiSvc
{
	unsigned int cmnt 	: 24;  // [23:0]
	unsigned int __reserved0: 4;   // [27:24]
	unsigned int cond 	: 4;   // [31:28]
};


struct InstBytesVfpMv
{
	unsigned int immd8	: 8;  // [7:0]
	unsigned int __reserved0: 4;  // [11:8]
	unsigned int vd		: 4;  // [15:12]
	unsigned int vfp_rn	: 4;  // [19:16]
	unsigned int __reserved1: 1;  // [20]
	unsigned int w		: 1;  // [21]
	unsigned int d		: 1;  // [22]
	unsigned int u		: 1;  // [23]
	unsigned int p		: 1;  // [24]
	unsigned int __reserved2: 3;  // [27:25]
	unsigned int cond	: 4;  // [31:28]
};


struct InstBytesVfpStregTr
{
	unsigned int __reserved0:12;  // [11:0]
	unsigned int vfp_rt	: 4;  // [15:12]
	unsigned int __reserved1:12;  // [27:16]
	unsigned int cond	: 4;  // [31:28]
};


/// Structure of Instruction Format (Thumb2-32bit)
struct InstThumb32BytesLdStMult
{
	unsigned int reglist	: 16;	// [15:0]
	unsigned int rn		: 4;	// [19:16]
	unsigned int __reserved0: 1; 	// [20]
	unsigned int wback	: 1;	// [21]
	unsigned int __reserved1: 10; 	// [31:22]
};


struct InstThumb32BytesPushPop
{
	unsigned int reglist	: 16;	// [15:0]
	unsigned int __reserved0: 16; 	// [31:16]
};


struct InstThumb32BytesLdStDouble
{
	unsigned int immd8	: 8;	// [7:0]
	unsigned int rt2	: 4;	// [11:8]
	unsigned int rt		: 4; 	// [15:12]
	unsigned int rn		: 4; 	// [19:16]
	unsigned int __reserved0: 1; 	// [20]
	unsigned int wback	: 1;	// [21]
	unsigned int __reserved1: 1; 	// [22]
	unsigned int add_sub	: 1;	// [23]
	unsigned int index	: 1;	// [24]
	unsigned int __reserved2: 7; 	// [31:25]
};


struct InstThumb32BytesTableBranch
{
	unsigned int rm		: 4; 	// [3:0]
	unsigned int h		: 1; 	// [4]
	unsigned int __reserved0: 11; 	// [15:5]
	unsigned int rn		: 4; 	// [19:16]
	unsigned int __reserved1: 12; 	// [31:20]
};


struct InstThumb32BytesDataProcShftreg
{
	unsigned int rm		: 4; 	// [3:0]
	unsigned int type	: 2; 	// [5:4]
	unsigned int imm2	: 2; 	// [7:6]
	unsigned int rd		: 4; 	// [11:8]
	unsigned int imm3	: 3; 	// [14:12]
	unsigned int __reserved0: 1; 	// [15]
	unsigned int rn		: 4; 	// [19:16]
	unsigned int sign	: 1; 	// [20]
	unsigned int __reserved1: 11; 	// [31:21]
};


struct InstThumb32BytesDataProcImmd
{
	unsigned int immd8	: 8; 	// [7:0]
	unsigned int rd		: 4; 	// [11:8]
	unsigned int immd3	: 3; 	// [14:12]
	unsigned int __reserved0: 1; 	// [15]
	unsigned int rn		: 4; 	// [19:16]
	unsigned int sign	: 1; 	// [20]
	unsigned int __reserved1: 5; 	// [25:21]
	unsigned int i_flag	: 1; 	// [26]
	unsigned int __reserved2: 5; 	// [31:27]
};


struct InstThumb32BytesBranch
{
	unsigned int immd11	: 11; 	// [10:0]
	unsigned int j2		: 1; 	// [11]
	unsigned int __reserved0: 1; 	// [12]
	unsigned int j1		: 1; 	// [13]
	unsigned int __reserved1: 2; 	// [15:14]
	unsigned int immd6	: 6; 	// [21:16]
	unsigned int cond	: 4; 	// [25:22]
	unsigned int sign	: 1; 	// [26]
	unsigned int __reserved2: 5; 	// [31:27]
};


struct InstThumb32BytesBranchLink
{
	unsigned int immd11	: 11; 	// [10:0]
	unsigned int j2		: 1; 	// [11]
	unsigned int __reserved0: 1; 	// [12]
	unsigned int j1		: 1; 	// [13]
	unsigned int __reserved1: 2; 	// [15:14]
	unsigned int immd10	: 10; 	// [25:16]
	unsigned int sign	: 1; 	// [26]
	unsigned int __reserved2: 5; 	// [31:27]
};


struct InstThumb32BytesLdStrReg
{
	unsigned int rm		: 4;	// [3:0]
	unsigned int immd2	: 2;	// [5:4]
	unsigned int __reserved0: 6; 	// [11:6]
	unsigned int rd		: 4;	// [15:12]
	unsigned int rn		: 4;	// [19:16]
	unsigned int __reserved1: 12; 	// [31:20]
};


struct InstThumb32BytesLdStrImm
{
	unsigned int immd12	: 12;	// [11:0]
	unsigned int rd		: 4;	// [15:12]
	unsigned int rn		: 4;	// [19:16]
	unsigned int __reserved0: 3; 	// [22:20]
	unsigned int add	: 1;	// [23]
	unsigned int __reserved1: 8; 	// [31:24]
};


struct InstThumb32BytesLdStrtImm
{
	unsigned int immd8	: 8;	// [7:0]
	unsigned int __reserved0: 4; 	// [11:8]
	unsigned int rd		: 4;	// [15:12]
	unsigned int rn		: 4;	// [19:16]
	unsigned int __reserved1: 12; 	// [31:20]
};


struct InstThumb32BytesDprocReg
{
	unsigned int rm		: 4;	// [3:0]
	unsigned int rot	: 2;	// [5:4]
	unsigned int __reserved0: 2; 	// [7:6]
	unsigned int rd		: 4;	// [11:8]
	unsigned int __reserved1: 4; 	// [15:12]
	unsigned int rn		: 4;	// [19:16]
	unsigned int sign	: 1;	// [20]
	unsigned int __reserved2: 11; 	// [31:21]
};


struct InstThumb32BytesMult
{
	unsigned int rm		: 4;	// [3:0]
	unsigned int __reserved0: 4; 	// [7:4]
	unsigned int rd		: 4; 	// [11:8]
	unsigned int ra		: 4; 	// [15:12]
	unsigned int rn		: 4; 	// [19:16]
	unsigned int __reserved1: 12; 	// [31:20]
};


struct InstThumb32BytesMultLong
{
	unsigned int rm		: 4;	// [3:0]
	unsigned int __reserved0: 4; 	// [7:4]
	unsigned int rdhi	: 4; 	// [11:8]
	unsigned int rdlo	: 4; 	// [15:12]
	unsigned int rn		: 4; 	// [19:16]
	unsigned int __reserved1: 12; 	// [31:20]
};


struct InstThumb32BytesBitField
{
	unsigned int msb	: 5;	// [4:0]
	unsigned int __reserved0: 1; 	// [5]
	unsigned int immd2	: 2; 	// [7:6]
	unsigned int rd		: 4; 	// [11:8]
	unsigned int immd3	: 3; 	// [14:9]
	unsigned int __reserved1: 1; 	// [15]
	unsigned int rn		: 4; 	// [19:16]
	unsigned int __reserved2: 12; 	// [31:20]
};


/// Structure of typedef Instruction Format (Thumb2-16bit)
struct InstThumb16BytesMovshiftReg
{
	unsigned int reg_rd	: 3;	// [2:0]
	unsigned int reg_rs	: 3;	// [5:3]
	unsigned int offset	: 5;	// [10:6]
	unsigned int op_int	: 2; 	// [12:11]
	unsigned int __reserved0: 3; 	// [15:13]

};


struct InstThumb16BytesAddSub
{
	unsigned int reg_rd	: 3;	// [2:0]
	unsigned int reg_rs	: 3;	// [5:3]
	unsigned int rn_imm	: 3;	// [8:6]
	unsigned int op_int	: 1;	// [9]
	unsigned int immd	: 1;	// [10]
	unsigned int __reserved0: 5; 	// [15:11]
};


struct InstThumb16BytesImmdOprs
{
	unsigned int offset8	: 8;	// [7:0]
	unsigned int reg_rd	: 3;	// [10:8]
	unsigned int op_int	: 2;	// [12:11]
	unsigned int __reserved0: 3; 	// [15:13]
};


struct InstThumb16BytesCmpT2
{
	unsigned int reg_rn	: 3;	// [2:0]
	unsigned int reg_rm	: 4;	// [6:3]
	unsigned int N		: 1;	// [7]
	unsigned int __reserved0: 8; 	// [15:8]
};


struct InstThumb16BytesDpr
{
	unsigned int reg_rd	: 3;	// [2:0]
	unsigned int reg_rs	: 3;	// [5:3]
	unsigned int op_int	: 4;	// [9:6]
	unsigned int __reserved0: 6; 	// [15:10]
};


struct InstThumb16BytesHighRegOprs
{
	unsigned int reg_rd	: 3;	// [2:0]
	unsigned int reg_rs	: 4;	// [6:3]
	unsigned int h1		: 1;	// [7]
	unsigned int op_int	: 2;	// [9:8]
	unsigned int __reserved0: 6; 	// [15:10]
};


struct InstThumb16BytesPcLdr
{
	unsigned int immd_8	: 8;	// [7:0]
	unsigned int reg_rd	: 3; 	// [10:8]
	unsigned int __reserved0: 5; 	// [15:11]
};


struct InstThumb16BytesLdStrReg
{

	unsigned int reg_rd	: 3; 	// [2:0]
	unsigned int reg_rb	: 3; 	// [5:3]
	unsigned int reg_ro	: 3; 	// [8:6]
	unsigned int __reserved0: 1; 	// [9]
	unsigned int byte_wrd	: 1; 	// [10]
	unsigned int ld_st	: 1; 	// [11]
	unsigned int __reserved1: 4; 	// [15:12]
};


struct InstThumb16BytesLdStrExts
{

	unsigned int reg_rd	: 3; 	// [2:0]
	unsigned int reg_rb	: 3; 	// [5:3]
	unsigned int reg_ro	: 3; 	// [8:6]
	unsigned int __reserved0: 1; 	// [9]
	unsigned int sign_ext	: 1; 	// [10]
	unsigned int h_flag	: 1; 	// [11]
	unsigned int __reserved1: 4; 	// [15:12]
};


struct InstThumb16BytesLdStrImmd
{
	unsigned int reg_rd	: 3; 	// [2:0]
	unsigned int reg_rb	: 3; 	// [5:3]
	unsigned int offset	: 5;	// [10:6]
	unsigned int ld_st	: 1; 	// [11]
	unsigned int byte_wrd	: 1; 	// [12]
	unsigned int __reserved0: 3; 	// [15:13]
};


struct InstThumb16BytesLdStrHfwrd
{
	unsigned int reg_rd	: 3; 	// [2:0]
	unsigned int reg_rb	: 3; 	// [5:3]
	unsigned int offset	: 5;	// [10:6]
	unsigned int ld_st	: 1; 	// [11]
	unsigned int __reserved0: 4; 	// [15:12]
};


struct InstThumb16BytesLdStrSpImmd
{
	unsigned int immd_8	: 8;	// [7:0]
	unsigned int reg_rd	: 3; 	// [10:8]
	unsigned int ld_st	: 1; 	// [11]
	unsigned int __reserved0: 4; 	// [15:12]
};


struct InstThumb16BytesMiscAddsp
{
	unsigned int immd_8	: 8;	// [7:0]
	unsigned int reg_rd	: 3; 	// [10:8]
	unsigned int __reserved0: 5; 	// [15:11]
};


struct InstThumb16BytesMiscSubsp
{
	unsigned int immd_8	: 7;	// [7:0]
	unsigned int __reserved0: 9; 	// [15:8]
};


struct InstThumb16BytesMiscCbnz
{
	unsigned int reg_rn	: 3;	// [2:0]
	unsigned int immd_5	: 5;	// [7:3]
	unsigned int __reserved0: 1; 	// [8]
	unsigned int i_ext	: 1;	// [9]
	unsigned int __reserved1: 1; 	// [10]
	unsigned int op_int	: 1;	// [11]
	unsigned int __reserved2: 4; 	// [15:12]
};


struct InstThumb16BytesMiscExtnd
{
	unsigned int reg_rd	: 3;	// [2:0]
	unsigned int reg_rm	: 3;	// [5:3]
	unsigned int __reserved0: 10; 	// [15:6]
};


struct InstThumb16BytesMiscPushPop
{
	unsigned int reg_list	: 8;	// [7:0]
	unsigned int m_ext	: 1;	// [8]
	unsigned int __reserved0: 7; 	// [15:9]
};


struct InstThumb16BytesMiscRev
{
	unsigned int reg_rd	: 3;	// [2:0]
	unsigned int reg_rm	: 3;	// [5:3]
	unsigned int __reserved0: 10; 	// [15:6]
};


struct InstThumb16BytesIfThen
{
	unsigned int mask	: 4;	// [3:0]
	unsigned int first_cond	: 4;	// [7:4]
	unsigned int __reserved0: 8;	// [15:8]
};


struct InstThumb16BytesLdmStm
{
	unsigned int reg_list	: 8;	// [7:0]
	unsigned int reg_rb	: 3;	// [10:8]
	unsigned int ld_st	: 1;	// [11]
	unsigned int __reserved0: 4;	// [15:12]
};


struct InstThumb16BytesSvc
{
	unsigned int value	: 8;	// [7:0]
	unsigned int __reserved0: 8;	// [15:8]
};


struct InstThumb16BytesCondBr
{
	unsigned int s_offset	: 8;	// [7:0]
	unsigned int cond	: 4;	// [11:8]
	unsigned int __reserved0: 4;	// [15:12]
};


struct InstThumb16BytesBr
{
	unsigned int immd11	: 11;	// [10:0]
	unsigned int __reserved0: 5;	// [15:11]
};


union InstBytes
{
	// Indicate the instruction bit width
	unsigned char bytes[4];
	unsigned int word;

	InstBytesDpr dpr;
	InstBytesDprSat dpr_sat;
	InstBytesPsr psr;
	InstBytesMult mult;
	InstBytesMultLn mult_ln;
	InstBytesSnglDswp sngl_dswp;
	InstBytesBax bax;
	InstBytesHfwrdTnsReg hfwrd_reg;
	InstBytesHfwrdTnsImm hfwrd_imm;
	InstBytesSdtr sdtr;
	InstBytesBdtr bdtr;
	InstBytesBrnch brnch;
	InstBytesCprDtr cpr_dtr;
	InstBytesCprDop cpr_dop;
	InstBytesCprRtr cpr_rtr;
	InstBytesSwiSvc swi_svc;
	InstBytesVfpMv vfp_mv;
	InstBytesVfpStregTr vfp_strreg_tr;
};


union InstThumb32Bytes
{
	// Indicate the instruction bit width
	unsigned char bytes[4];
	unsigned int word;

	InstThumb32BytesLdStMult ld_st_mult;
	InstThumb32BytesPushPop push_pop;
	InstThumb32BytesLdStDouble ld_st_double;
	InstThumb32BytesTableBranch table_branch;
	InstThumb32BytesDataProcShftreg data_proc_shftreg;
	InstThumb32BytesDataProcImmd data_proc_immd;
	InstThumb32BytesBranch branch;
	InstThumb32BytesBranchLink branch_link;
	InstThumb32BytesLdStrReg ldstr_reg;
	InstThumb32BytesLdStrImm ldstr_imm;
	InstThumb32BytesLdStrtImm ldstrt_imm;
	InstThumb32BytesDprocReg dproc_reg;
	InstThumb32BytesMult mult;
	InstThumb32BytesMultLong mult_long;
	InstThumb32BytesBitField bit_field;
	InstThumb16BytesMiscAddsp add_sp;
};


union InstThumb16Bytes
{
	// Indicate the instruction bit width
	unsigned char bytes[2];

	InstThumb16BytesMovshiftReg movshift_reg_ins;
	InstThumb16BytesAddSub addsub_ins;
	InstThumb16BytesImmdOprs immd_oprs_ins;
	InstThumb16BytesDpr dpr_ins;
	InstThumb16BytesHighRegOprs high_oprs_ins;
	InstThumb16BytesPcLdr pcldr_ins;
	InstThumb16BytesLdStrReg ldstr_reg_ins;
	InstThumb16BytesLdStrExts ldstr_exts_ins;
	InstThumb16BytesLdStrImmd ldstr_immd_ins;
	InstThumb16BytesLdStrHfwrd ldstr_hfwrd_ins;
	InstThumb16BytesLdStrSpImmd sp_immd_ins;
	InstThumb16BytesMiscAddsp addsp_ins;
	InstThumb16BytesMiscSubsp sub_sp_ins;
	InstThumb16BytesMiscCbnz cbnz_ins;
	InstThumb16BytesMiscExtnd misc_extnd_ins;
	InstThumb16BytesMiscPushPop push_pop_ins;
	InstThumb16BytesMiscRev rev_ins;
	InstThumb16BytesIfThen if_eq_ins;
	InstThumb16BytesLdmStm ldm_stm_ins;
	InstThumb16BytesSvc svc_ins;
	InstThumb16BytesCondBr cond_br_ins;
	InstThumb16BytesCmpT2 cmp_t2;
	InstThumb16BytesBr br_ins;
};


enum InstOpcode
{
	InstOpcodeInvalid = 0,
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	InstOpcode##_name,
#include "Inst.def"
#undef DEFINST
	// Max
	InstOpcodeCount
};


enum InstThumb16Opcode
{
	InstThumb16OpcodeInvalid = 0,
#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
	InstThumb16Opcode##_name,
#include "InstThumb.def"
#undef DEFINST
	// Max
	InstThumb16OpcodeCount
};



enum InstThumb32Opcode
{
	InstThumb32OpcodeInvalid = 0,
#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
	InstThumb32Opcode##_name,
#include "InstThumb32.def"
#undef DEFINST
	// Max
	InstThumb32OpcodeCount
};


enum InstCategory
{
	InstCatInvalid = 0,
	InstCategoryDprReg, 	 // Data Processing Register instructions
	InstCategoryDprImm,	 // Data Processing Immediate instructions
	InstCategoryDprSat, 	 // Data Processing saturation instructions
	InstCategoryPsr,   // Processor Status Register (PSR) instructions
	InstCategoryMult,   // Multiplication instructions
	InstCategoryMultSign,   // Multiplication instructions
	InstCategoryMultLn,   // Long Multiplication instructions
	InstCategoryMultLnSign,   // Long Multiplication instructions
	InstCategorySdswp,   // Single Data Swap instructions
	InstCategoryBax,   // Branch and Exchange instructions
	InstCategoryHfwrdReg,   // Halfword data transfer Register offset
	InstCategoryHfwrdImm,   // Halfword data transfer Immediate offset
	InstCategorySdtr,   // Single Data Transfer instructions
	InstCategoryBdtr,   // Block Data Transfer instructions
	InstCategoryBrnch,   // Branch instructions
	InstCategoryCprDtr,   // Coprocessor Data Transfer instructions
	InstCategoryCprDop,   // Coprocessor Data Operation instructions
	InstCategoryCprRtr,   // Coprocessor Register Transfer instructions
	InstCategorySwiSvc,   // Software Interrupt / SVC Angel trap instructions
	InstCategoryVfp,   // Vector Floating Point Instructions
	InstCategoryUndef,
	InstCategoryCount
};


enum InstThumb16Category
{
	InstThumb16CategoryNone = 0,
	InstThumb16CategoryMovshiftReg,	 // Move Shift Instructions Register Based
	InstThumb16CategoryAddsub,		 // Addition and Subtraction Instructions
	InstThumb16CategoryImmdOprs,		 // Operations with Immediate Operations
	InstThumb16CategoryDprIns,		 // Data Processing Operations
	InstThumb16CategoryHiRegOprs,	 // High Register Operations
	InstThumb16CategoryPcLdr,		 // LDR operation over PC
	InstThumb16CategoryLdstrReg,		 // Load Store with Register Offset
	InstThumb16CategoryLdstrExts,	 // Sign Extended Load Store
	InstThumb16CategoryLdstrImmd,	 // Load Store with Immediate Offset
	InstThumb16CategoryLdstrHfwrd,	 // Load Store Half Word
	InstThumb16CategoryLdstrSpImmd,	 // Load Store SP Related with Immediate Offset
	InstThumb16CategoryIfThen,		 // If Then Block Instructions
	InstThumb16CategoryLdmStm,		 // Load/Store Multiple Instructions
	InstThumb16CategoryMiscAddspIns,	 // Miscellaneous Instructions ADD SP relative
	InstThumb16CategoryMiscSvcIns,	 // Miscellaneous Instructions SVC Instructions
	InstThumb16CategoryMiscBr,		 // Miscellaneous Instructions Conditional Branch
	InstThumb16CategoryMiscUcbr,		 // Miscellaneous Instructions Conditional Branch
	InstThumb16CategoryMiscRev,		 // Miscellaneous Reverse Instructions
	InstThumb16CategoryMiscSubspIns,	 // Miscellaneous Instructions SUB SP relative
	InstThumb16CategoryMiscPushPop,	 // Miscellaneous Instructions PUSH and POP
	InstThumb16CategoryMiscCbnz,		 // Miscellaneous Instructions CB{N}Z
	InstThumb16CategoryCmpT2,		 // Miscellaneous Instructions CB{N}Z
	InstThumb16CategoryUndef,
	InstThumb16CategoryCount
};


enum InstThumb32Category
{
	InstThumb32CategoryNone = 0,
	InstThumb32CategoryLdStMult,	// Load Store Multiple
	InstThumb32CategoryLdStDouble,	// Load Store Double Exclusive
	InstThumb32CategoryPushPop,	// Push Pop Multiple
	InstThumb32CategoryTableBrnch,	// Table Branch Byte
	InstThumb32CategoryDprShftreg,	// Data processing Shifted register
	InstThumb32CategoryDprImm,	// Data processing immediate
	InstThumb32CategoryDprBinImm,	// Data processing binary immediate
	InstThumb32CategoryBranch,	// Branch
	InstThumb32CategoryBranchLx,	// Branch with Link exchange
	InstThumb32CategoryBranchCond,	// Branch Conditional
	InstThumb32CategoryLdstrByte,	// Load Store Register Byte/Halfword
	InstThumb32CategoryLdstrReg,	// Load Store Register
	InstThumb32CategoryLdstrImmd,	// Load Store Immediate
	InstThumb32CategoryDprReg,	// Data Processing Register
	InstThumb32CategoryMult,	// Multiply
	InstThumb32CategoryMultLong,	// Multiply Long
	InstThumb32CategoryBitField,	// Multiply Long
	InstThumb32CategoryMovImmd,	// Immediate Move
	InstThumb32CategoryUndef,
	InstThumb32CategoryCount
};


struct InstInfo
{
	InstOpcode inst;
	InstCategory category;
	const char* name;
	const char* fmt_str;
	InstOpcode opcode;
	int size;
};


struct InstThumb16Info
{
	InstThumb16Opcode inst_16;
	InstThumb16Category cat16;
	const char* name;
	const char* fmt_str;
	InstThumb16Opcode opcode;
	int size;
	InstThumb16Info *next_table;
	int next_table_low;
	int next_table_high;
};


struct InstThumb32Info
{
	InstThumb32Opcode inst_32;
	InstThumb32Category cat32;
	const char* name;
	const char* fmt_str;
	InstThumb32Opcode opcode;
	int size;
	InstThumb32Info *next_table;
	int next_table_low;
	int next_table_high;
};


/// ARM instruction
class Inst
{
	// Disassembler
	Asm *as; 

	// Instruction address
	unsigned int addr;

	// Instruction content
	InstBytes dword;
	InstThumb16Bytes dword_16;
	InstThumb32Bytes dword_32;

	// Information after decoded
	InstInfo *info;
	InstThumb16Info *info_16;
	InstThumb32Info *info_32;

public:

	/// constructor
	Inst();

	/// FIXME comment
	unsigned int Rotl(unsigned int value, int shift);
	unsigned int Rotr(unsigned int value, int shift);

	/// FIXME comment
	void Amode2Disasm(std::ostream &os, InstCategory cat);
	void Amode3Disasm(std::ostream &os, InstCategory cat);

	/// Functions that dump the ARM instruction related information
	void DumpRd(std::ostream &os);
	void DumpRn(std::ostream &os);
	void DumpRm(std::ostream &os);
	void DumpRs(std::ostream &os);
	void DumpOp2(std::ostream &os);
	void DumpCond(std::ostream &os);
	void DumpRdlo(std::ostream &os);
	void DumpRdhi(std::ostream &os);
	void DumpPsr(std::ostream &os);
	void DumpOp2Psr(std::ostream &os);
	void DumpAMode3(std::ostream &os);
	void DumpAMode2(std::ostream &os);
	void DumpIdx(std::ostream &os);
	void DumpBaddr(std::ostream &os);
	void DumpRegs(std::ostream &os);
	void DumpImmd24(std::ostream &os);
	void DumpImmd16(std::ostream &os);
	void DumpCopr(std::ostream &os);
	void DumpAMode5(std::ostream &os);
	void DumpVfp1stm(std::ostream &os);
	void DumpVfp1ldm(std::ostream &os);
	void DumpVfpRegs(std::ostream &os);
	void DumpFreg(std::ostream &os);
	void DumpFp(std::ostream &os);
	void DumpRt(std::ostream &os);
	void DumpHex(std::ostream &os, unsigned int *inst_ptr, unsigned int inst_addr);
	void Dump(std::ostream &os);

	/// Functions that dump the Thumb 16 instruction related information
	void Thumb16DumpRD(std::ostream &os);
	void Thumb16DumpRM(std::ostream &os);
	void Thumb16DumpRN(std::ostream &os);
	void Thumb16DumpIMMD8(std::ostream &os);
	void Thumb16DumpIMMD3(std::ostream &os);
	void Thumb16DumpIMMD5(std::ostream &os);
	void Thumb16DumpCOND(std::ostream &os);
	void Thumb16DumpREGS(std::ostream &os);
	void Thumb16DumpItEqX(std::ostream &os);
	void Thumb16DumpHex(std::ostream &os, unsigned int *inst_ptr , unsigned int inst_addr);
	void Thumb16Dump(std::ostream &os);

	/// Functions that dump the Thumb 32 instruction related information
	void Thumb32DumpRD(std::ostream &os);
	void Thumb32DumpRN(std::ostream &os);
	void Thumb32DumpRM(std::ostream &os);
	void Thumb32DumpRT(std::ostream &os);
	void Thumb32DumpRT2(std::ostream &os);
	void Thumb32DumpRA(std::ostream &os);
	void Thumb32DumpRDLO(std::ostream &os);
	void Thumb32DumpRDHI(std::ostream &os);
	void Thumb32DumpS(std::ostream &os);
	void Thumb32dumpREGS(std::ostream &os);
	void Thumb32DumpSHFTREG(std::ostream &os);
	void Thumb32DumpIMM12(std::ostream &os);
	void Thumb32DumpIMMD12(std::ostream &os);
	void Thumb32DumpIMMD8(std::ostream &os);
	void Thumb32DumpIMM2(std::ostream &os);
	void Thumb32DumpCOND(std::ostream &os);
	void Thumb32DumpLSB(std::ostream &os);
	void Thumb32DumpWID(std::ostream &os);
	void Thumb32DumpIMMD16(std::ostream &os);
	void Thumb32DumpADDR(std::ostream &os);
	void Thumb32DumpHex(std::ostream &os, unsigned int *inst_ptr, unsigned int inst_addr);
	void Thumb32Dump(std::ostream &os);

	/// Populate the 'info' field of an instruction by decoding the instruction
	/// in 'buf'. The first 4 bytes of 'buf' are copied to field 'bytes'. The value
	/// given in 'addr' is the instruction's virtual address, used for branch
	/// decoding purposes.
	void Decode(unsigned int addr, const char *buf);
	void Thumb16Decode(const char *buf, unsigned int ip);
	void Thumb32Decode(const char *buf, unsigned int ip);
	void Thumb16InstTableDecode();
	void Thumb32InstTableDecode();

	///Getters
	unsigned int getAddress() { return addr; }
	InstBytes *getBytes() { return &dword; }
	InstThumb16Bytes *getThumb16Bytes() { return &dword_16; }
	InstThumb32Bytes *getThumb32Bytes() { return &dword_32; }
	InstOpcode getOpcode() { return info ? info->opcode : InstOpcodeInvalid; }
	InstThumb16Opcode getThumb16Opcode() { return info_16 ? info_16->opcode : InstThumb16OpcodeInvalid; }
	InstThumb32Opcode getThumb32Opcode() { return info_32 ? info_32->opcode : InstThumb32OpcodeInvalid; }
	InstInfo *getInstInfo() { return info; }
	InstThumb16Info *getInstThumb16Info() { return info_16; }
	InstThumb32Info *getInstThumb32Info() { return info_32; }

};

}  // namespace ARM

#endif // ARCH_ARM_ASM_INST_H
