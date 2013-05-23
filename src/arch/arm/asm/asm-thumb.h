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

#ifndef ASM_THUMB_H_
#define ASM_THUMB_H_

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "asm.h"



/*
 * Structure of Instruction Format (Thumb2-32bit)
 */

struct arm_thumb32_ld_st_mult_t
{
	unsigned int reglist	: 16;	/* [15:0] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved0: 1; 	/* [20] */
	unsigned int wback	: 1;	/* [21] */
	unsigned int __reserved1: 10; 	/* [31:22] */
};

struct arm_thumb32_push_pop_t
{
	unsigned int reglist	: 16;	/* [15:0] */
	unsigned int __reserved0: 16; 	/* [31:16] */
};

struct arm_thumb32_ld_st_double_t
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
};

struct arm_thumb32_table_branch_t
{
	unsigned int rm		: 4; 	/* [3:0] */
	unsigned int h		: 1; 	/* [4] */
	unsigned int __reserved0: 11; 	/* [15:5] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
};

struct arm_thumb32_data_proc_shftreg_t
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
};

struct arm_thumb32_data_proc_immd_t
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
};

struct arm_thumb32_branch_t
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
};

struct arm_thumb32_branch_link_t
{
	unsigned int immd11	: 11; 	/* [10:0] */
	unsigned int j2		: 1; 	/* [11] */
	unsigned int __reserved0: 1; 	/* [12] */
	unsigned int j1		: 1; 	/* [13] */
	unsigned int __reserved1: 2; 	/* [15:14] */
	unsigned int immd10	: 10; 	/* [25:16] */
	unsigned int sign	: 1; 	/* [26] */
	unsigned int __reserved2: 5; 	/* [31:27] */
};


struct arm_thumb32_ldstr_reg_t
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int immd2	: 2;	/* [5:4] */
	unsigned int __reserved0: 6; 	/* [11:6] */
	unsigned int rd		: 4;	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
};

struct arm_thumb32_ldstr_imm_t
{
	unsigned int immd12	: 12;	/* [11:0] */
	unsigned int rd		: 4;	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved0: 3; 	/* [22:20] */
	unsigned int add	: 1;	/* [23] */
	unsigned int __reserved1: 8; 	/* [31:24] */
};

struct arm_thumb32_ldstrt_imm_t
{
	unsigned int immd8	: 8;	/* [7:0] */
	unsigned int __reserved0: 4; 	/* [11:8] */
	unsigned int rd		: 4;	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
};

struct arm_thumb32_dproc_reg_t
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int rot	: 2;	/* [5:4] */
	unsigned int __reserved0: 2; 	/* [7:6] */
	unsigned int rd		: 4;	/* [11:8] */
	unsigned int __reserved1: 4; 	/* [15:12] */
	unsigned int rn		: 4;	/* [19:16] */
	unsigned int sign	: 1;	/* [20] */
	unsigned int __reserved2: 11; 	/* [31:21] */
};

struct arm_thumb32_mult_t
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int __reserved0: 4; 	/* [7:4] */
	unsigned int rd		: 4; 	/* [11:8] */
	unsigned int ra		: 4; 	/* [15:12] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
};

struct arm_thumb32_mult_long_t
{
	unsigned int rm		: 4;	/* [3:0] */
	unsigned int __reserved0: 4; 	/* [7:4] */
	unsigned int rdhi	: 4; 	/* [11:8] */
	unsigned int rdlo	: 4; 	/* [15:12] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved1: 12; 	/* [31:20] */
};

struct arm_thumb32_bit_field_t
{
	unsigned int msb	: 5;	/* [4:0] */
	unsigned int __reserved0: 1; 	/* [5] */
	unsigned int immd2	: 2; 	/* [7:6] */
	unsigned int rd		: 4; 	/* [11:8] */
	unsigned int immd3	: 3; 	/* [14:9] */
	unsigned int __reserved1: 1; 	/* [15] */
	unsigned int rn		: 4; 	/* [19:16] */
	unsigned int __reserved2: 12; 	/* [31:20] */
};


/*
 * Structure of Instruction Format (Thumb2-16bit)
 */

struct arm_thumb16_movshift_reg_t
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 3;	/* [5:3] */
	unsigned int offset	: 5;	/* [10:6] */
	unsigned int op_int	: 2; 	/* [12:11] */
	unsigned int __reserved0: 3; 	/* [15:13] */

};

struct arm_thumb16_addsub_t
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 3;	/* [5:3] */
	unsigned int rn_imm	: 3;	/* [8:6] */
	unsigned int op_int	: 1;	/* [9] */
	unsigned int immd	: 1;	/* [10] */
	unsigned int __reserved0: 5; 	/* [15:11] */
};

struct arm_thumb16_immd_oprs_t
{
	unsigned int offset8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3;	/* [10:8] */
	unsigned int op_int	: 2;	/* [12:11] */
	unsigned int __reserved0: 3; 	/* [15:13] */
};

struct arm_thumb16_cmp_t2_t
{
	unsigned int reg_rn	: 3;	/* [2:0] */
	unsigned int reg_rm	: 4;	/* [6:3] */
	unsigned int N		: 1;	/* [7] */
	unsigned int __reserved0: 8; 	/* [15:8] */
};

struct arm_thumb16_dpr_t
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 3;	/* [5:3] */
	unsigned int op_int	: 4;	/* [9:6] */
	unsigned int __reserved0: 6; 	/* [15:10] */
};

struct arm_thumb16_highreg_oprs_t
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rs	: 4;	/* [6:3] */
	unsigned int h1		: 1;	/* [7] */
	unsigned int op_int	: 2;	/* [9:8] */
	unsigned int __reserved0: 6; 	/* [15:10] */
};

struct arm_thumb16_pcldr_t
{
	unsigned int immd_8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3; 	/* [10:8] */
	unsigned int __reserved0: 5; 	/* [15:11] */
};

struct arm_thumb16_ldstr_reg_t
{

	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int reg_ro	: 3; 	/* [8:6] */
	unsigned int __reserved0: 1; 	/* [9] */
	unsigned int byte_wrd	: 1; 	/* [10] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int __reserved1: 4; 	/* [15:12] */
};

struct arm_thumb16_ldstr_exts_t
{

	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int reg_ro	: 3; 	/* [8:6] */
	unsigned int __reserved0: 1; 	/* [9] */
	unsigned int sign_ext	: 1; 	/* [10] */
	unsigned int h_flag	: 1; 	/* [11] */
	unsigned int __reserved1: 4; 	/* [15:12] */
};

struct arm_thumb16_ldstr_immd_t
{
	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int offset	: 5;	/* [10:6] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int byte_wrd	: 1; 	/* [12] */
	unsigned int __reserved0: 3; 	/* [15:13] */
};

struct arm_thumb16_ldstr_hfwrd_t
{
	unsigned int reg_rd	: 3; 	/* [2:0] */
	unsigned int reg_rb	: 3; 	/* [5:3] */
	unsigned int offset	: 5;	/* [10:6] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int __reserved0: 4; 	/* [15:12] */
};

struct arm_thumb16_ldstr_sp_immd_t
{
	unsigned int immd_8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3; 	/* [10:8] */
	unsigned int ld_st	: 1; 	/* [11] */
	unsigned int __reserved0: 4; 	/* [15:12] */
};

struct arm_thumb16_misc_addsp_t
{
	unsigned int immd_8	: 8;	/* [7:0] */
	unsigned int reg_rd	: 3; 	/* [10:8] */
	unsigned int __reserved0: 5; 	/* [15:11] */
};

struct arm_thumb16_misc_subsp_t
{
	unsigned int immd_8	: 7;	/* [7:0] */
	unsigned int __reserved0: 9; 	/* [15:8] */
};

struct arm_thumb16_misc_cbnz_t
{
	unsigned int reg_rn	: 3;	/* [2:0] */
	unsigned int immd_5	: 5;	/* [7:3] */
	unsigned int __reserved0: 1; 	/* [8] */
	unsigned int i_ext	: 1;	/* [9] */
	unsigned int __reserved1: 1; 	/* [10] */
	unsigned int op_int	: 1;	/* [11] */
	unsigned int __reserved2: 4; 	/* [15:12] */
};

struct arm_thumb16_misc_extnd_t
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rm	: 3;	/* [5:3] */
	unsigned int __reserved0: 10; 	/* [15:6] */
};

struct arm_thumb16_misc_push_pop_t
{
	unsigned int reg_list	: 8;	/* [7:0] */
	unsigned int m_ext	: 1;	/* [8] */
	unsigned int __reserved0: 7; 	/* [15:9] */
};

struct arm_thumb16_misc_rev_t
{
	unsigned int reg_rd	: 3;	/* [2:0] */
	unsigned int reg_rm	: 3;	/* [5:3] */
	unsigned int __reserved0: 10; 	/* [15:6] */
};

struct arm_thumb16_if_then_t
{
	unsigned int mask	: 4;	/* [3:0] */
	unsigned int first_cond	: 4;	/* [7:4] */
	unsigned int __reserved0: 8;	/* [15:8] */
};

struct arm_thumb16_ldm_stm_t
{
	unsigned int reg_list	: 8;	/* [7:0] */
	unsigned int reg_rb	: 3;	/* [10:8] */
	unsigned int ld_st	: 1;	/* [11] */
	unsigned int __reserved0: 4;	/* [15:12] */
};

struct arm_thumb16_svc_t
{
	unsigned int value	: 8;	/* [7:0] */
	unsigned int __reserved0: 8;	/* [15:8] */
};

struct arm_thumb16_cond_br_t
{
	unsigned int s_offset	: 8;	/* [7:0] */
	unsigned int cond	: 4;	/* [11:8] */
	unsigned int __reserved0: 4;	/* [15:12] */
};

struct arm_thumb16_br_t
{
	unsigned int immd11	: 11;	/* [10:0] */
	unsigned int __reserved0: 5;	/* [15:11] */
};

union arm_thumb16_inst_dword_t
{
	unsigned char bytes[2];

	struct arm_thumb16_movshift_reg_t movshift_reg_ins;
	struct arm_thumb16_addsub_t addsub_ins;
	struct arm_thumb16_immd_oprs_t immd_oprs_ins;
	struct arm_thumb16_dpr_t dpr_ins;
	struct arm_thumb16_highreg_oprs_t high_oprs_ins;
	struct arm_thumb16_pcldr_t pcldr_ins;
	struct arm_thumb16_ldstr_reg_t ldstr_reg_ins;
	struct arm_thumb16_ldstr_exts_t ldstr_exts_ins;
	struct arm_thumb16_ldstr_immd_t ldstr_immd_ins;
	struct arm_thumb16_ldstr_hfwrd_t ldstr_hfwrd_ins;
	struct arm_thumb16_ldstr_sp_immd_t sp_immd_ins;
	struct arm_thumb16_misc_addsp_t addsp_ins;
	struct arm_thumb16_misc_subsp_t sub_sp_ins;
	struct arm_thumb16_misc_cbnz_t cbnz_ins;
	struct arm_thumb16_misc_extnd_t misc_extnd_ins;
	struct arm_thumb16_misc_push_pop_t push_pop_ins;
	struct arm_thumb16_misc_rev_t rev_ins;
	struct arm_thumb16_if_then_t if_eq_ins;
	struct arm_thumb16_ldm_stm_t ldm_stm_ins;
	struct arm_thumb16_svc_t svc_ins;
	struct arm_thumb16_cond_br_t cond_br_ins;
	struct arm_thumb16_cmp_t2_t cmp_t2;
	struct arm_thumb16_br_t br_ins;
};

union arm_thumb32_inst_dword_t
{
	unsigned char bytes[4];

	struct arm_thumb32_ld_st_mult_t ld_st_mult;
	struct arm_thumb32_push_pop_t push_pop;
	struct arm_thumb32_ld_st_double_t ld_st_double;
	struct arm_thumb32_table_branch_t table_branch;
	struct arm_thumb32_data_proc_shftreg_t data_proc_shftreg;
	struct arm_thumb32_data_proc_immd_t data_proc_immd;
	struct arm_thumb32_branch_t branch;
	struct arm_thumb32_branch_link_t branch_link;
	struct arm_thumb32_ldstr_reg_t ldstr_reg;
	struct arm_thumb32_ldstr_imm_t ldstr_imm;
	struct arm_thumb32_ldstrt_imm_t ldstrt_imm;
	struct arm_thumb32_dproc_reg_t dproc_reg;
	struct arm_thumb32_mult_t mult;
	struct arm_thumb32_mult_long_t mult_long;
	struct arm_thumb32_bit_field_t bit_field;
	struct arm_thumb16_misc_addsp_t add_sp;

};

enum arm_thumb16_inst_enum
{
	ARM_THUMB16_INST_NONE = 0,

#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
	ARM_THUMB16_INST_##_name,
#include "asm-thumb.dat"
#undef DEFINST

	/* Max */
	ARM_THUMB16_INST_COUNT
};

enum arm_thumb32_inst_enum
{
	ARM_THUMB32_INST_NONE = 0,
#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
	ARM_THUMB32_INST_##_name,
#include "asm-thumb32.dat"
#undef DEFINST
	/* Max */
	ARM_THUMB32_INST_COUNT
};

enum arm_thumb16_cat_enum
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
};

enum arm_thumb32_cat_enum
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
};

struct arm_thumb16_inst_info_t
{
	enum arm_thumb16_inst_enum inst_16;
	enum arm_thumb16_cat_enum cat16;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
	struct arm_thumb16_inst_info_t *next_table;
	int next_table_low;
	int next_table_high;
};

struct arm_thumb32_inst_info_t
{
	enum arm_thumb32_inst_enum inst_32;
	enum arm_thumb32_cat_enum cat32;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
	struct arm_thumb32_inst_info_t *next_table;
	int next_table_low;
	int next_table_high;
};

struct arm_thumb16_inst_t
{
	unsigned int addr;
	union arm_thumb16_inst_dword_t dword;
	struct arm_thumb16_inst_info_t *info;
};

struct arm_thumb32_inst_t
{
	unsigned int addr;
	unsigned int decode;
	union arm_thumb32_inst_dword_t dword;
	struct arm_thumb32_inst_info_t *info;
};


/* Pointers to the tables of instructions Thumb16*/

struct arm_thumb16_inst_info_t *arm_thumb16_asm_table;
struct arm_thumb16_inst_info_t *arm_thumb16_shft_ins_table;
struct arm_thumb16_inst_info_t *arm_thumb16_shft_ins_lv2_table;

struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv1_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv2_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv3_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv4_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv5_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv6_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv7_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv8_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv9_table;


struct arm_thumb16_inst_info_t *arm_thumb16_data_proc_table;
struct arm_thumb16_inst_info_t *arm_thumb16_spcl_data_brex_table;
struct arm_thumb16_inst_info_t *arm_thumb16_spcl_data_brex_lv1_table;

struct arm_thumb16_inst_info_t *arm_thumb16_ld_st_table;
struct arm_thumb16_inst_info_t *arm_thumb16_ld_st_lv1_table;
struct arm_thumb16_inst_info_t *arm_thumb16_ld_st_lv2_table;

struct arm_thumb16_inst_info_t *arm_thumb16_misc_table;
struct arm_thumb16_inst_info_t *arm_thumb16_it_table;
struct arm_thumb16_inst_info_t *current_table;



/* Pointers to the tables of instructions Thumb32 */

struct arm_thumb32_inst_info_t *arm_thumb32_asm_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv6_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv7_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv8_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv9_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv10_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv11_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv12_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv13_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv14_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv15_table;


struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_dual_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst1_dual_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst2_dual_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst3_dual_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg6_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg7_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_misc_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_misc1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_st_single_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_ld_hfword_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_hfword1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_hfword2_table;

struct arm_thumb32_inst_info_t *arm_thumb32_ld_word_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_word1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_mult_table;
struct arm_thumb32_inst_info_t *arm_thumb32_mult1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm3_table;

struct arm_thumb32_inst_info_t *arm_thumb32_mult_long_table;

struct arm_thumb32_inst_info_t *arm_thumb32_brnch_ctrl_table;
struct arm_thumb32_inst_info_t *arm_thumb32_brnch_ctrl1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_mov_table;
struct arm_thumb32_inst_info_t *arm_thumb32_mov1_table;


/* Thumb Disassembler Functions */

void arm_thumb16_disasm_init();
void arm_thumb16_setup_table(char* name , char* fmt_str ,
	enum arm_thumb16_cat_enum cat16 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, enum arm_thumb16_inst_enum inst_name);
void arm_thumb16_inst_decode(struct arm_thumb16_inst_t *inst);
void arm_thumb16_inst_dump(FILE *f , char *str , int inst_str_size , void *inst_ptr ,
	unsigned int inst_index, unsigned int inst_addr);
void arm_thumb32_inst_dump(FILE *f , char *str , int inst_str_size , void *inst_ptr ,
	unsigned int inst_index, unsigned int inst_addr);

void arm_thumb16_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat);
void arm_thumb16_inst_dump_RM(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat);
void arm_thumb16_inst_dump_RN(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat);
void arm_thumb16_inst_dump_IMMD8(char **inst_str_ptr , int *inst_str_size ,
	struct arm_thumb16_inst_t *inst , enum arm_thumb16_cat_enum cat ,
	unsigned int inst_addr);
void arm_thumb16_inst_dump_IMMD5(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat,
	unsigned int inst_addr);
void arm_thumb16_inst_dump_IMMD3(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat);
void arm_thumb16_inst_dump_COND(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat);
void arm_thumb16_inst_dump_REGS(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat);
void arm_thumb16_inst_dump_it_eq_x(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat);

void arm_thumb32_disasm_init();
void arm_thumb32_setup_table(char* name , char* fmt_str ,
	enum arm_thumb32_cat_enum cat32 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, int op7, int op8, enum arm_thumb32_inst_enum inst_name);
void arm_thumb32_inst_decode(struct arm_thumb32_inst_t *inst);
void arm_thumb32_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_REGS(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_RN(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_RM(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_RT(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_RT2(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_IMM12(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_S(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_SHFT_REG(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_IMMD12(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_IMM2(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_IMMD16(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_IMMD8(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_WID(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_LSB(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_RA(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_RDHI(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_RDLO(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_inst_dump_ADDR(char **inst_str_ptr, int *inst_str_size,
		struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat,
		unsigned int inst_addr);
void arm_thumb32_inst_dump_COND(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);

void thumb16_disasm(void *buf, unsigned int ip, volatile struct arm_thumb16_inst_t *inst);
void thumb32_disasm(void *buf, unsigned int ip, volatile struct arm_thumb32_inst_t *inst);
void arm_th16_inst_debug_dump(struct arm_thumb16_inst_t *inst, FILE *f );
void arm_th32_inst_debug_dump(struct arm_thumb32_inst_t *inst, FILE *f );



int arm_test_thumb32(void *inst_ptr);

#endif /* ASM_THUMB_H_ */
