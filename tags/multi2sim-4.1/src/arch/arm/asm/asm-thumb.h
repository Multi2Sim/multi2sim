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
#include "asm.h"

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
	unsigned int reg_rs	: 3;	/* [5:3] */
	unsigned int h2		: 1;	/* [6] */
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
	unsigned int __reserved1: 4; 	/* [15:12] */
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

union arm_thumb16_inst_dword_t
{
	unsigned char bytes[2];


};

enum arm_thumb_inst_enum
{
	ARM_THUMB_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2, _arg3) \
	ARM_THUMB_INST_##_name,
#include "asm-thumb.dat"
#undef DEFINST

	/* Max */
	ARM_THUMB_INST_COUNT
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
	ARM_THUMB16_CAT_MISC_INS,	/* Miscellaneous Instructions */
	ARM_THUMB16_CAT_IF_THEN,	/* If Then Block instructions */
	ARM_THUMB16_CAT_LDM_STM,	/* Load/Store Multiple Instructions */
	ARM_THUMB16_CAT_CONDBR_SVC,	/* Conditional Branch, Software interrupt  */

	ARM_THUMB16_CAT_UNDEF,

	ARM_THUMB16_CAT_COUNT
};

enum arm_thumb32_cat_enum
{
	ARM_THUMB16_CAT_NONE = 0,

	ARM_THUMB16_CAT_UNDEF,

	ARM_THUMB16_CAT_COUNT
};

struct arm_thumb16_inst_info_t
{
	enum arm_thumb_inst_enum inst;
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
	enum arm_thumb_inst_enum inst;
	enum arm_thumb32_cat_enum cat32;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
};

struct arm_thumb16_inst_t
{
	unsigned int addr;
	union arm_inst_dword_t dword;
	struct arm_inst_info_t *info;
};
#endif /* ASM_THUMB_H_ */
