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

#ifndef FERMI_ASM_H
#define FERMI_ASM_H

#include <stdio.h>



/* Microcode Formats */
enum frm_fmt_enum
{
	FRM_FMT_NONE = 0, 

#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
	FRM_FMT_##_fmt,
#include "asm.dat"
#undef DEFINST

	/* Max */
	FRM_FMT_COUNT
};


/* Special Registers */
enum frm_sr_enum
{
	FRM_SR_Laneld = 0, 
	FRM_SR_VirtCfg = 2, 
	FRM_SR_VirtId, 
	FRM_SR_PM0, 
	FRM_SR_PM1, 
	FRM_SR_PM2, 
	FRM_SR_PM3, 
	FRM_SR_PM4, 
	FRM_SR_PM5, 
	FRM_SR_PM6, 
	FRM_SR_PM7, 
	FRM_SR_PRIM_TYPE = 16, 
	FRM_SR_INVOCATION_ID, 
	FRM_SR_Y_DIRECTION, 
	FRM_SR_MACHINE_ID_0 = 24, 
	FRM_SR_MACHINE_ID_1, 
	FRM_SR_MACHINE_ID_2, 
	FRM_SR_MACHINE_ID_3, 
	FRM_SR_AFFINITY, 
	FRM_SR_Tid = 32,  
	FRM_SR_Tid_X, 
	FRM_SR_Tid_Y, 
	FRM_SR_Tid_Z, 
	FRM_SR_CTAParam, 
	FRM_SR_CTAid_X, 
	FRM_SR_CTAid_Y, 
	FRM_SR_CTAid_Z, 
	FRM_SR_NTid, 
	FRM_SR_NTid_X, 
	FRM_SR_NTid_Y, 
	FRM_SR_NTid_Z, 
	FRM_SR_GridParam, 
	FRM_SR_NCTAid_X, 
	FRM_SR_NCTAid_Y, 
	FRM_SR_NCTAid_Z, 
	FRM_SR_SWinLo, 
	FRM_SR_SWINSZ, 
	FRM_SR_SMemSz, 
	FRM_SR_SMemBanks, 
	FRM_SR_LWinLo, 
	FRM_SR_LWINSZ, 
	FRM_SR_LMemLoSz, 
	FRM_SR_LMemHiOff, 
	FRM_SR_EqMask, 
	FRM_SR_LtMask, 
	FRM_SR_LeMask, 
	FRM_SR_GtMask, 
	FRM_SR_GeMask, 
	FRM_SR_ClockLo = 80, 
	FRM_SR_ClockHi, 

	/* Max */
	FRM_SR_COUNT
};




/* 
 * Structure of Microcode Format
 */

struct frm_fmt_fp_ffma_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int ftzfmz : 2; /* [7:6] */
	unsigned long long int neg_src3 : 1; /* [8] */
	unsigned long long int neg_src2 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 1; /* [48] */
	unsigned long long int src3 : 6; /* [54:49] */
	unsigned long long int rnd : 2; /* [56:55] */
	unsigned long long int __reserved2 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_fadd_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int ftz : 1; /* [5] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int abs_src1 : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int neg_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 1; /* [48] */
	unsigned long long int sat : 1; /* [49] */
	unsigned long long int __reserved2 : 5; /* [54:50] */
	unsigned long long int rnd : 2; /* [56:55] */
	unsigned long long int fma : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_fadd32i_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int ftz : 1; /* [5] */
	unsigned long long int __reserved1 : 1; /* [6] */
	unsigned long long int abs_src1 : 1; /* [7] */
	unsigned long long int __reserved2 : 1; /* [8] */
	unsigned long long int neg_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int imm32 : 32; /* [57:26] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_fcmp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int ftz : 1; /* [5] */
	unsigned long long int __reserved1 : 4; /* [9:6] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved2 : 1; /* [48] */
	unsigned long long int src3 : 6; /* [54:49] */
	unsigned long long int cmp : 4; /* [58:55] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_fp_fmul_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int ftzfmz : 2; /* [7:6] */
	unsigned long long int fma_mode : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 7; /* [54:48] */
	unsigned long long int rnd : 2; /* [56:55] */
	unsigned long long int neg_src2 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_fmul32i_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int ftzfmz : 2; /* [7:6] */
	unsigned long long int fma_mode : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int imm32 : 32; /* [57:26] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_fsetp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 2; /* [5:4] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int abs_src1 : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int neg_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int Q : 3; /* [16:14] */
	unsigned long long int P : 3; /* [19:17] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 1; /* [48] */
	unsigned long long int R : 4; /* [52:49] */
	unsigned long long int logic : 2; /* [54:53] */
	unsigned long long int cmp : 4; /* [58:55] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_fp_mufu_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int __reserved1 : 1; /* [6] */
	unsigned long long int abs_src1 : 1; /* [7] */
	unsigned long long int __reserved2 : 1; /* [8] */
	unsigned long long int neg_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int op : 3; /* [28:26] */
	unsigned long long int __reserved3 : 29; /* [57:29] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_dfma_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 4; /* [7:4] */
	unsigned long long int neg_src3 : 1; /* [8] */
	unsigned long long int neg_src2 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 1; /* [48] */
	unsigned long long int src3 : 6; /* [54:49] */
	unsigned long long int __reserved2 : 3; /* [57:55] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_dadd_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 2; /* [5:4] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int abs_src1 : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int neg_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 7; /* [54:48] */
	unsigned long long int rnd : 2; /* [56:55] */
	unsigned long long int __reserved2 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_dmul_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 5; /* [8:4] */
	unsigned long long int neg_src2 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 7; /* [54:48] */
	unsigned long long int rnd : 2; /* [56:55] */
	unsigned long long int __reserved2 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_fp_dsetp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 2; /* [5:4] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int abs_src1 : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int neg_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int Q : 3; /* [16:14] */
	unsigned long long int P : 3; /* [19:17] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 1; /* [48] */
	unsigned long long int R : 4; /* [52:49] */
	unsigned long long int logic : 2; /* [54:53] */
	unsigned long long int cmp : 4; /* [58:55] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_int_imad_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int mod2 : 1; /* [5] */
	unsigned long long int hi : 1; /* [6] */
	unsigned long long int mod1 : 1; /* [7] */
	unsigned long long int src_mod : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int src3 : 6; /* [54:49] */
	unsigned long long int __reserved1 : 1; /* [55] */
	unsigned long long int sat : 1; /* [56] */
	unsigned long long int __reserved2 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_imul_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int mod2 : 1; /* [5] */
	unsigned long long int hi : 1; /* [6] */
	unsigned long long int mod1 : 1; /* [7] */
	unsigned long long int src_mod : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved1 : 9; /* [57:49] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_iadd_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int x : 1; /* [6] */
	unsigned long long int __reserved1 : 1; /* [7] */
	unsigned long long int src_mod : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved2 : 9; /* [57:49] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_iadd32i_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int x : 1; /* [6] */
	unsigned long long int __reserved1 : 3; /* [9:7] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int imm32 : 32; /* [57:26] */
	unsigned long long int dst_cc : 1; /* [58] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_int_iscadd_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int shamt : 5; /* [9:5] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved1 : 6; /* [54:49] */
	unsigned long long int src_mod : 2; /* [56:55] */
	unsigned long long int __reserved2 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_bfe_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int u32 : 1; /* [5] */
	unsigned long long int __reserved1 : 2; /* [7:6] */
	unsigned long long int brev : 1; /* [8] */
	unsigned long long int __reserved2 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved3 : 9; /* [57:49] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_bfi_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 1; /* [48] */
	unsigned long long int src3 : 6; /* [54:49] */
	unsigned long long int __reserved2 : 2; /* [56:55] */
	unsigned long long int dst_cc : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_shr_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int s : 1; /* [4] */
	unsigned long long int u32 : 1; /* [5] */
	unsigned long long int __reserved1 : 3; /* [8:6] */
	unsigned long long int w : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved2 : 9; /* [57:49] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_shl_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int u32 : 1; /* [5] */
	unsigned long long int __reserved1 : 3; /* [8:6] */
	unsigned long long int w : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved2 : 9; /* [57:49] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_lop_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 2; /* [5:4] */
	unsigned long long int op : 2; /* [7:6] */
	unsigned long long int not_src2 : 1; /* [8] */
	unsigned long long int not_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved1 : 9; /* [57:49] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_lop32i_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 2; /* [5:4] */
	unsigned long long int op : 2; /* [7:6] */
	unsigned long long int __reserved1 : 1; /* [8] */
	unsigned long long int not_src1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int imm32 : 32; /* [57:26] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_flo_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_int_isetp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int u32 : 1; /* [5] */
	unsigned long long int __reserved1 : 4; /* [9:6] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int Q : 3; /* [16:14] */
	unsigned long long int P : 3; /* [19:17] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved2 : 1; /* [48] */
	unsigned long long int R : 4; /* [52:49] */
	unsigned long long int logic : 2; /* [54:53] */
	unsigned long long int cmp : 4; /* [58:55] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_int_icmp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int u32 : 1; /* [5] */
	unsigned long long int __reserved1 : 4; /* [9:6] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved2 : 1; /* [48] */
	unsigned long long int src3 : 6; /* [54:49] */
	unsigned long long int cmp : 3; /* [57:55] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_conv_f2f_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int s : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int pass_round : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int __reserved1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int dtype : 2; /* [21:20] */
	unsigned long long int __reserved2 : 1; /* [22] */
	unsigned long long int stype : 2; /* [24:23] */
	unsigned long long int __reserved3 : 1; /* [25] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved4 : 1; /* [48] */
	unsigned long long int rnd : 2; /* [50:49] */
	unsigned long long int __reserved5 : 4; /* [54:51] */
	unsigned long long int ftz : 1; /* [55] */
	unsigned long long int __reserved6 : 2; /* [57:56] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_conv_f2i_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 2; /* [4:5] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int dtype_s : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int __reserved1 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int dtype_n : 2; /* [21:20] */
	unsigned long long int __reserved2 : 1; /* [22] */
	unsigned long long int stype : 2; /* [24:23] */
	unsigned long long int __reserved3 : 1; /* [25] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved4 : 1; /* [48] */
	unsigned long long int rnd : 2; /* [50:49] */
	unsigned long long int __reserved5 : 4; /* [54:51] */
	unsigned long long int ftz : 1; /* [55] */
	unsigned long long int __reserved6 : 2; /* [57:56] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_conv_i2f_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 2; /* [5:4] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int __reserved1 : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int stype_s : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int dtype : 2; /* [21:20] */
	unsigned long long int __reserved2 : 1; /* [22] */
	unsigned long long int stype_n : 2; /* [24:23] */
	unsigned long long int __reserved3 : 1; /* [25] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved4 : 1; /* [48] */
	unsigned long long int rnd : 2; /* [50:49] */
	unsigned long long int __reserved5 : 7; /* [57:51] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_conv_i2i_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int sat : 1; /* [5] */
	unsigned long long int abs_src2 : 1; /* [6] */
	unsigned long long int dtype_s : 1; /* [7] */
	unsigned long long int neg_src2 : 1; /* [8] */
	unsigned long long int stype_s : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int dtype_n : 2; /* [21:20] */
	unsigned long long int __reserved2 : 1; /* [22] */
	unsigned long long int stype_n : 2; /* [24:23] */
	unsigned long long int __reserved3 : 1; /* [25] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int dst_cc : 1; /* [48] */
	unsigned long long int __reserved5 : 6; /* [54:49] */
	unsigned long long int b_src2 : 2; /* [56:55] */
	unsigned long long int __reserved6 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_mov_mov_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int s : 1; /* [4] */
	unsigned long long int __reserved0 : 5; /* [9:5] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int __reserved1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved2 : 10; /* [57:48] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_mov_mov32i_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int s : 1; /* [4] */
	unsigned long long int __reserved0 : 5; /* [9:5] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int __reserved1 : 6; /* [25:20] */
	unsigned long long int imm32 : 32; /* [57:26] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_mov_sel_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int src2_mod : 2; /* [47:46] */
	unsigned long long int __reserved1 : 1; /* [48] */
	unsigned long long int R : 4; /* [52:49] */
	unsigned long long int __reserved2 : 5; /* [57:53] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_ldc_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int __reserved2 : 6; /* [25:20] */
	unsigned long long int src2 : 20; /* [45:26] */
	unsigned long long int __reserved3 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_ld_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int cop : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved1 : 16; /* [57:42] */
	unsigned long long int e : 1; /* [58] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_ldst_ldu_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved2 : 16; /* [57:42] */
	unsigned long long int e : 1; /* [58] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_ldst_ldl_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int cop : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved1 : 16; /* [57:42] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_lds_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved2 : 16; /* [57:42] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_ldlk_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int R_1_0 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved1 : 16; /* [57:42] */
	unsigned long long int R_2 : 1; /* [58] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_ldst_ldslk_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved2 : 8; /* [49:42] */
	unsigned long long int R : 3; /* [52:50] */
	unsigned long long int __reserved3 : 5; /* [57:53] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_st_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int cop : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved1 : 16; /* [57:42] */
	unsigned long long int e : 1; /* [58] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_ldst_stl_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int cop : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved1 : 16; /* [57:42] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_stul_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved2 : 16; /* [57:42] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_sts_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved2 : 16; /* [57:42] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_stsul_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved2 : 16; /* [57:42] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_atom_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 4; /* [8:5] */
	unsigned long long int op_0 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset20_16_0 : 17; /* [42:26] */
	unsigned long long int src3 : 6; /* [48:43] */
	unsigned long long int src4 : 6; /* [54:49] */
	unsigned long long int offset20_19_17 : 3; /* [57:55] */
	unsigned long long int e : 1; /* [58] */
	unsigned long long int op_3_1 : 3; /* [61:59] */
	unsigned long long int op1 : 2; /* [63:62] */
};

struct frm_fmt_ldst_red_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int type : 4; /* [8:5] */
	unsigned long long int op_0 : 1; /* [9] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset20_16_0 : 17; /* [42:26] */
	unsigned long long int src3 : 6; /* [48:43] */
	unsigned long long int src4 : 6; /* [54:49] */
	unsigned long long int offset20_19_17 : 3; /* [57:55] */
	unsigned long long int e : 1; /* [58] */
	unsigned long long int op_3_1 : 3; /* [61:59] */
	unsigned long long int op1 : 2; /* [63:62] */
};

struct frm_fmt_ldst_cctl_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int opb : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int opa : 2; /* [27:26] */
	unsigned long long int offset16 : 16; /* [43:28] */
	unsigned long long int __reserved2 : 14; /* [57:44] */
	unsigned long long int e : 1; /* [58] */
	unsigned long long int op1 : 5; /* [63:59] */
};

struct frm_fmt_ldst_cctll_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int op : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int src1 : 6; /* [25:20] */
	unsigned long long int offset16 : 16; /* [41:26] */
	unsigned long long int __reserved2 : 16; /* [57:42] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ldst_membar_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int s : 1; /* [4] */
	unsigned long long int lvl : 2; /* [6:5] */
	unsigned long long int __reserved0 : 3; /* [9:7] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int __reserved1 : 44; /* [57:14] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_bra_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int u : 1; /* [15] */
	unsigned long long int lmt : 1; /* [16] */
	unsigned long long int __reserved1 : 9; /* [25:17] */
	unsigned long long int tgt : 24; /* [49:26] */
	unsigned long long int __reserved2 : 8; /* [57:50] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_jmp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int u : 1; /* [15] */
	unsigned long long int lmt : 1; /* [16] */
	unsigned long long int __reserved1 : 9; /* [25:17] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved2 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_cal_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int __reserved1 : 1; /* [15] */
	unsigned long long int noinc : 1; /* [16] */
	unsigned long long int __reserved2 : 9; /* [25:17] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved3 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_jcal_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int __reserved1 : 1; /* [15] */
	unsigned long long int noinc : 1; /* [16] */
	unsigned long long int __reserved2 : 9; /* [25:17] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved3 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_ret_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int __reserved1 : 44; /* [57:14] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_brk_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int __reserved1 : 44; /* [57:14] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_cont_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int __reserved1 : 44; /* [57:14] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_longjmp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int __reserved1 : 44; /* [57:14] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_ssy_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int __reserved1 : 11; /* [25:15] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved2 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_pbk_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int __reserved1 : 11; /* [25:15] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved2 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_pcnt_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int __reserved1 : 11; /* [25:15] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved2 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_pret_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int noinc : 1; /* [15] */
	unsigned long long int __reserved1 : 10; /* [25:16] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved2 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_plongjmp_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int tgt_mod : 1; /* [14] */
	unsigned long long int __reserved1 : 11; /* [25:15] */
	unsigned long long int tgt : 20; /* [45:26] */
	unsigned long long int __reserved2 : 12; /* [57:46] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_ctrl_exit_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int __reserved1 : 44; /* [57:14] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_misc_nop_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int s : 1; /* [4] */
	unsigned long long int ccop : 5; /* [9:5] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int __reserved0 : 12; /* [25:14] */
	unsigned long long int imm16 : 16; /* [41:26] */
	unsigned long long int __reserved1 : 8; /* [49:42] */
	unsigned long long int trig : 1; /* [50] */
	unsigned long long int op : 4; /* [54:51] */
	unsigned long long int __reserved2 : 3; /* [57:55] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_misc_s2r_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int __reserved1 : 6; /* [25:20] */
	unsigned long long int sreg : 8; /* [33:26] */
	unsigned long long int __reserved2 : 24; /* [57:34] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_misc_b2r_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 4; /* [7:4] */
	unsigned long long int op : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int imm6 : 6; /* [25:20] */
	unsigned long long int __reserved1 : 32; /* [57:26] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_misc_lepc_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 6; /* [9:4] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int __reserved1 : 38; /* [57:20] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_misc_bar_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int op : 2; /* [6:5] */
	unsigned long long int mod : 1; /* [7] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int bar : 6; /* [25:20] */
	unsigned long long int tcount : 12; /* [37:26] */
	unsigned long long int __reserved2 : 8; /* [45:38] */
	unsigned long long int tcount_mod : 1; /* [46] */
	unsigned long long int bar_mod : 1; /* [47] */
	unsigned long long int __reserved3 : 1; /* [48] */
	unsigned long long int Q : 4; /* [52:49] */
	unsigned long long int P : 3; /* [55:53] */
	unsigned long long int __reserved4 : 2; /* [57:56] */
	unsigned long long int op1 : 6; /* [63:58] */
};

struct frm_fmt_misc_vote_t
{
	unsigned long long int op0 : 4; /* [3:0] */
	unsigned long long int __reserved0 : 1; /* [4] */
	unsigned long long int mod : 3; /* [7:5] */
	unsigned long long int __reserved1 : 2; /* [9:8] */
	unsigned long long int pred : 4; /* [13:10] */
	unsigned long long int dst : 6; /* [19:14] */
	unsigned long long int Q : 4; /* [23:20] */
	unsigned long long int __reserved2 : 30; /* [53:24] */
	unsigned long long int P : 3; /* [56:54] */
	unsigned long long int __reserved3 : 1; /* [57] */
	unsigned long long int op1 : 6; /* [63:58] */
};


enum frm_inst_category_enum
{
	FRM_INST_CAT_NONE = 0,

	FRM_INST_CAT_FP,  /* Floating point instructions */
	FRM_INST_CAT_INT,  /* Integer instructions */
	FRM_INST_CAT_CONV,  /* Conversion instructions */
	FRM_INST_CAT_MOV,  /* Movement instructions */
	FRM_INST_CAT_PRED,  /* Predicate/CC instructions */
	FRM_INST_CAT_TEX,  /* Texture instructions */
	FRM_INST_CAT_LDST, /* Compute load/store instructions */
	FRM_INST_CAT_SURF, /* Surface memory instructions */
	FRM_INST_CAT_CTRL, /* Control instructions */
	FRM_INST_CAT_MISC, /* Miscellaneous instructions */

	FRM_INST_CAT_COUNT
};


enum frm_inst_enum
{
	FRM_INST_NONE = 0, 

#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
	FRM_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	FRM_INST_COUNT
};


union frm_inst_dword_t
{
	unsigned char bytes[8];

	struct frm_fmt_fp_ffma_t fp_ffma;
	struct frm_fmt_fp_fadd_t fp_fadd;
	struct frm_fmt_fp_fadd32i_t fp_fadd32i;
	struct frm_fmt_fp_fcmp_t fp_fcmp;
	struct frm_fmt_fp_fmul_t fp_fmul;
	struct frm_fmt_fp_fmul32i_t fp_fmul32i;
	struct frm_fmt_fp_fsetp_t fp_fsetp;
	struct frm_fmt_fp_mufu_t fp_mufu;
	struct frm_fmt_fp_dfma_t fp_dfma;
	struct frm_fmt_fp_dadd_t fp_dadd;
	struct frm_fmt_fp_dmul_t fp_dmul;
	struct frm_fmt_fp_dsetp_t fp_dsetp;
	struct frm_fmt_int_imad_t int_imad;
	struct frm_fmt_int_imul_t int_imul;
	struct frm_fmt_int_iadd_t int_iadd;
	struct frm_fmt_int_iadd32i_t int_iadd32i;
	struct frm_fmt_int_iscadd_t int_iscadd;
	struct frm_fmt_int_bfe_t int_bfe;
	struct frm_fmt_int_bfi_t int_bfi;
	struct frm_fmt_int_shr_t int_shr;
	struct frm_fmt_int_shl_t int_shl;
	struct frm_fmt_int_lop_t int_lop;
	struct frm_fmt_int_lop32i_t int_lop32i;
	struct frm_fmt_int_isetp_t int_isetp;
	struct frm_fmt_int_icmp_t int_icmp;
	struct frm_fmt_int_flo_t int_flo;
	struct frm_fmt_conv_f2f_t conv_f2f;
	struct frm_fmt_conv_f2i_t conv_f2i;
	struct frm_fmt_conv_i2f_t conv_i2f;
	struct frm_fmt_conv_i2i_t conv_i2i;
	struct frm_fmt_mov_mov_t mov_mov;
	struct frm_fmt_mov_mov32i_t mov_mov32i;
	struct frm_fmt_mov_sel_t mov_sel;
	struct frm_fmt_ldst_ldc_t ldst_ldc;
	struct frm_fmt_ldst_ld_t ldst_ld;
	struct frm_fmt_ldst_ldu_t ldst_ldu;
	struct frm_fmt_ldst_ldl_t ldst_ldl;
	struct frm_fmt_ldst_lds_t ldst_lds;
	struct frm_fmt_ldst_ldlk_t ldst_ldlk;
	struct frm_fmt_ldst_ldslk_t ldst_ldslk;
	struct frm_fmt_ldst_st_t ldst_st;
	struct frm_fmt_ldst_stl_t ldst_stl;
	struct frm_fmt_ldst_stul_t ldst_stul;
	struct frm_fmt_ldst_sts_t ldst_sts;
	struct frm_fmt_ldst_stsul_t ldst_stsul;
	struct frm_fmt_ldst_atom_t ldst_atom;
	struct frm_fmt_ldst_red_t ldst_red;
	struct frm_fmt_ldst_cctl_t ldst_cctl;
	struct frm_fmt_ldst_cctll_t ldst_cctll;
	struct frm_fmt_ldst_membar_t ldst_membar;
	struct frm_fmt_ctrl_bra_t ctrl_bra;
	struct frm_fmt_ctrl_jmp_t ctrl_jmp;
	struct frm_fmt_ctrl_cal_t ctrl_cal;
	struct frm_fmt_ctrl_jcal_t ctrl_jcal;
	struct frm_fmt_ctrl_ret_t ctrl_ret;
	struct frm_fmt_ctrl_brk_t ctrl_brk;
	struct frm_fmt_ctrl_cont_t ctrl_cont;
	struct frm_fmt_ctrl_longjmp_t ctrl_longjmp;
	struct frm_fmt_ctrl_ssy_t ctrl_ssy;
	struct frm_fmt_ctrl_pbk_t ctrl_pbk;
	struct frm_fmt_ctrl_pcnt_t ctrl_pcnt;
	struct frm_fmt_ctrl_pret_t ctrl_pret;
	struct frm_fmt_ctrl_plongjmp_t ctrl_plongjmp;
	struct frm_fmt_ctrl_exit_t ctrl_exit;
	struct frm_fmt_misc_nop_t misc_nop;
	struct frm_fmt_misc_s2r_t misc_s2r;
	struct frm_fmt_misc_b2r_t misc_b2r;
	struct frm_fmt_misc_lepc_t misc_lepc;
	struct frm_fmt_misc_bar_t misc_bar;
	struct frm_fmt_misc_vote_t misc_vote;
};


struct frm_inst_info_t
{
	enum frm_inst_enum inst;
	enum frm_inst_category_enum category;
	char *name;
	char *fmt_str;
	enum frm_fmt_enum fmt;
	unsigned int opcode;
	int size;
};


struct frm_inst_t
{
	unsigned int addr;
	union frm_inst_dword_t dword;
	struct frm_inst_info_t *info;
};


void frm_disasm_init(void);
void frm_disasm_done(void);

void frm_inst_dump(FILE *f, char *str, int inst_str_size, unsigned char *buf, int inst_index);
void frm_inst_hex_dump(FILE *f, unsigned char *buf, int inst_index);

void frm_inst_dump_pred(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_ext(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_dst(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_P(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_Q(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_src1(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_src2(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_src3(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_imm(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_R(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_offset(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_shamt(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_target(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);
void frm_inst_dump_ccop(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt);

/* Copy instruction */
void frm_inst_copy(struct frm_inst_t *dest, struct frm_inst_t *src);

/* Obtaining source operand fields for ALU instructions */
void frm_inst_get_op_src(struct frm_inst_t *inst, int src_idx,
        int *sel, int *rel, int *chan, int *neg, int *abs);

void frm_inst_decode(struct frm_inst_t *inst);

void frm_disasm(char *path);

#endif

