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

#ifndef ARM_ASM_H
#define ARM_ASM_H


/* Standard headers */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Project headers */
#include <list.h>
#include <misc.h>
#include <debug.h>
#include <elf_format.h>

/* Shift operators */
enum arm_op2_shift_t
{
	LSL = 0,
	LSR,
	ASR,
	ROR
};


/*
 * Structure of Instruction Format
 */

struct fmt_dpr_t
{
	unsigned long long op2 : 12; /* [11:0] */
	unsigned long long dst_reg : 4; /* [15:12] */
	unsigned long long op1_reg : 4; /* [19:16] */
	unsigned long long s_cond : 1; /* [20] */
	unsigned long long opc_dpr : 4; /* [24:21] */
	unsigned long long imm : 1; /* [25] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_dpr_sat_t
{
	unsigned long long op2 : 12; /* [11:0] */
	unsigned long long dst_reg : 4; /* [15:12] */
	unsigned long long op1_reg : 4; /* [19:16] */
	unsigned long long s_cond : 1; /* [20] */
	unsigned long long opc_dpr : 4; /* [24:21] */
	unsigned long long imm : 1; /* [25] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_psr_t
{
	unsigned long long op2 : 12; /* [11:0] */
	unsigned long long dst_reg : 4; /* [15:12] */
	unsigned long long psr_loc: 1; /* [22] */
	unsigned long long imm : 1; /* [25] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_mult_t
{
	unsigned long long op0_rm : 4; /* [3:0] */
	unsigned long long op1_rs : 4; /* [11:8] */
	unsigned long long op2_rn : 4; /* [15:12] */
	unsigned long long dst_rd : 4; /* [19:16] */
	unsigned long long s_cond : 1; /* [20] */
	unsigned long long m_acc : 1; /* [21] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_mul_ln_t
{
	unsigned long long op0_rm : 4; /* [3:0] */
	unsigned long long op1_rs : 4; /* [11:8] */
	unsigned long long dst_lo : 4; /* [15:12] */
	unsigned long long dst_hi : 4; /* [19:16] */
	unsigned long long s_cond : 1; /* [20] */
	unsigned long long m_acc : 1; /* [21] */
	unsigned long long sign : 1; /* [22] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_sngl_dswp_t
{
	unsigned long long op0_rm : 4; /* [3:0] */
	unsigned long long dst_rd : 4; /* [15:12] */
	unsigned long long base_rn : 4; /* [19:16] */
	unsigned long long d_type : 1; /* [22] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_bax_t
{
	unsigned long long ins_set : 1; /* [0] */
	unsigned long long op0_rn : 4; /* [3:0] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_hfwrd_tns_reg_t
{
	unsigned long long off_reg : 4; /* [3:0] */
	unsigned long long sh_comb : 2; /* [6:5] */
	unsigned long long dst_rd : 4; /* [15:12] */
	unsigned long long base_rn : 4; /* [19:16] */
	unsigned long long ld_st : 1; /* [20] */
	unsigned long long wb : 1; /* [21] */
	unsigned long long up_dn : 1; /* [23] */
	unsigned long long idx_typ : 1; /* [24] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_hfwrd_tns_imm_t
{
	unsigned long long imm_off_lo : 4; /* [3:0] */
	unsigned long long sh_comb : 2; /* [6:5] */
	unsigned long long imm_off_hi : 4; /* [11:8] */
	unsigned long long dst_rd : 4; /* [15:12] */
	unsigned long long base_rn : 4; /* [19:16] */
	unsigned long long ld_st : 1; /* [20] */
	unsigned long long wb : 1; /* [21] */
	unsigned long long up_dn : 1; /* [23] */
	unsigned long long idx_typ : 1; /* [24] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_sdtr_t
{
	unsigned long long off : 12; /* [11:0] */
	unsigned long long src_dst_rd : 4; /* [15:12] */
	unsigned long long base_rn : 4; /* [19:16] */
	unsigned long long ld_st : 1; /* [20] */
	unsigned long long wb : 1; /* [21] */
	unsigned long long d_type : 1; /* [22] */
	unsigned long long up_dn : 1; /* [23] */
	unsigned long long idx_typ : 1; /* [24] */
	unsigned long long imm : 1; /* [25] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_bdtr_t
{
	unsigned long long reg_lst : 16; /* [15:0] */
	unsigned long long base_rn : 4; /* [19:16] */
	unsigned long long ld_st : 1; /* [20] */
	unsigned long long wb : 1; /* [21] */
	unsigned long long psr_frc : 1; /* [22] */
	unsigned long long up_dn : 1; /* [23] */
	unsigned long long idx_typ : 1; /* [24] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_brnch_t
{
	unsigned long long off : 24; /* [23:0] */
	unsigned long long link : 1; /* [24] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_cpr_dtr_t
{
	unsigned long long off : 8; /* [7:0] */
	unsigned long long cpr_num : 4; /* [11:8] */
	unsigned long long cpr_src_dst : 4; /* [15:12] */
	unsigned long long base_rn : 4; /* [19:16] */
	unsigned long long ld_st : 1; /* [20] */
	unsigned long long wb : 1; /* [21] */
	unsigned long long tr_len : 1; /* [22] */
	unsigned long long up_dn : 1; /* [23] */
	unsigned long long idx_typ : 1; /* [24] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_cpr_dop_t
{
	unsigned long long cpr_op_rm : 4; /* [3:0] */
	unsigned long long cpr_info : 3; /* [7:5] */
	unsigned long long cpr_num : 4; /* [11:8] */
	unsigned long long cpr_dst : 4; /* [15:12] */
	unsigned long long cpr_rn : 4; /* [19:16] */
	unsigned long long cpr_opc : 4; /* [23:20] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_cpr_rtr_t
{
	unsigned long long cpr_op_rm : 4; /* [3:0] */
	unsigned long long cpr_info : 3; /* [7:5] */
	unsigned long long cpr_num : 4; /* [11:8] */
	unsigned long long rd : 4; /* [15:12] */
	unsigned long long cpr_rn : 4; /* [19:16] */
	unsigned long long ld_st : 1; /* [20] */
	unsigned long long cpr_opm : 1; /* [23:21] */
	unsigned long long cond : 4; /* [31:28] */
};

struct fmt_swi_svc_t
{
	unsigned long long cmnt : 24; /* [23:0] */
	unsigned long long cond : 4; /* [31:28] */
};

union arm_inst_dword_t
{
	unsigned char bytes[4];

	struct fmt_dpr_t dpr_ins;
	struct fmt_dpr_sat_t dpr_sat_ins;
	struct fmt_psr_t psr_ins;
	struct fmt_mult_t mult_ins;
	struct fmt_mul_ln_t mul_ln_ins;
	struct fmt_sngl_dswp_t sngl_dswp_ins;
	struct fmt_bax_t bax_ins;
	struct fmt_hfwrd_tns_reg_t hfwrd_reg_ins;
	struct fmt_hfwrd_tns_imm_t hfwrd_imm_ins;
	struct fmt_sdtr_t sdtr_ins;
	struct fmt_bdtr_t bdtr_ins;
	struct fmt_brnch_t brnch_ins;
	struct fmt_cpr_dtr_t cpr_dtr_ins;
	struct fmt_cpr_dop_t cpr_dop_ins;
	struct fmt_cpr_rtr_t cpr_rtr_ins;
	struct fmt_swi_svc_t swi_svc_ins;

};


enum arm_inst_enum
{
	ARM_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, _category, _opcode) \
	ARM_INST_##_name,
#include "arm-asm.dat"
#undef DEFINST

	/* Max */
	ARM_INST_COUNT
};

enum arm_cat_enum
{
	ARM_CAT_NONE = 0,

	ARM_CAT_DPR_REG, /* Data Processing Register instructions */
	ARM_CAT_DPR_IMM, /* Data Processing Immediate instructions */
	ARM_CAT_DPR_SAT, /* Data Processing saturation instructions */
	ARM_CAT_PSR, /* Processor Status Register (PSR) instructions */
	ARM_CAT_MULT, /* Multiplication instructions */
	ARM_CAT_MULT_SIGN, /* Multiplication instructions */
	ARM_CAT_MULT_LN, /* Long Multiplication instructions */
	ARM_CAT_MULT_LN_SIGN, /* Long Multiplication instructions */
	ARM_CAT_SDSWP, /* Single Data Swap instructions */
	ARM_CAT_BAX, /* Branch and Exchange instructions */
	ARM_CAT_HFWRD_REG, /* Halfword data transfer Register offset */
	ARM_CAT_HFWRD_IMM, /* Halfword data transfer Immediate offset */
	ARM_CAT_SDTR, /* Single Data Transfer instructions */
	ARM_CAT_BDTR, /* Block Data Transfer instructions */
	ARM_CAT_BRNCH, /* Branch instructions */
	ARM_CAT_CPR_DTR, /* Coprocessor Data Transfer instructions */
	ARM_CAT_CPR_DOP, /* Coprocessor Data Operation instructions */
	ARM_CAT_CPR_RTR, /* Coprocessor Register Transfer instructions */
	ARM_CAT_SWI_SVC, /* Software Interrupt / SVC Angel trap instructions */

	ARM_CAT_UNDEF,

	ARM_CAT_COUNT
};

struct arm_inst_info_t
{
	enum arm_inst_enum inst;
	enum arm_cat_enum category;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
};

struct arm_inst_t
{
	unsigned int addr;
	union arm_inst_dword_t dword;
	struct arm_inst_info_t *info;
};

struct arm_inst_info_t arm_inst_info[(256*16)];

//!TODO Define functions for the ARM_INST



#endif

