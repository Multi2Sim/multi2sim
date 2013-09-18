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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "inst.h"



/*
 * Class 'ARMAsm'
 */

void ARMAsmCreate(ARMAsm *self)
{
	struct arm_inst_info_t *info;
	int i;
	int j;

	/* Form the Instruction table and read Information from table*/
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	switch (ARMInstCategory##_category) { \
	\
	case ARMInstCategoryDprReg: \
	arm_inst_info[_arg1 * 16 + 0].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 0 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 1].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 1 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 2].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 2 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 3].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 3 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 4].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 4 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 5].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 5 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 6].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 6 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 7].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 7 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 8].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 8 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 10].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 10 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 12].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 12 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 14].opcode = ARM_INST_##_name; \
	info = &arm_inst_info[_arg1 * 16 + 14 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break; \
	\
	case ARMInstCategoryDprImm: \
	for (i = 0; i < 16; i++ ){\
		arm_inst_info[_arg1 * 16 + i].opcode = ARM_INST_##_name;\
		info = &arm_inst_info[_arg1 * 16 + i ]; \
		info->inst = ARM_INST_##_name; \
		info->category = ARMInstCategory##_category; \
		info->name = #_name; \
		info->fmt_str = _fmt_str; \
		info->size = 4;\
		info->opcode = ARM_INST_##_name; \
	}\
	break;\
	\
	case ARMInstCategoryDprSat:\
	arm_inst_info[_arg1 * 16 + 5].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 5 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryPsr:\
	arm_inst_info[_arg1 * 16 + 0].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 0 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryMult:\
	arm_inst_info[_arg1 * 16 + 9].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 9 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryMultSign:\
	arm_inst_info[_arg1 * 16 + 8].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 8 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 10].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 10 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 12].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 12 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	arm_inst_info[_arg1 * 16 + 14].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 14 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryMultLn:\
	arm_inst_info[_arg1 * 16 + 9].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 9 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryMultLnSign:\
	arm_inst_info[_arg1 * 16 + 8].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 8 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategorySdswp:\
	arm_inst_info[_arg1 * 16 + 9].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + 9 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryBax:\
	arm_inst_info[_arg1 * 16 + _arg2].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + _arg2 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryHfwrdReg:\
	arm_inst_info[_arg1 * 16 + _arg2].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + _arg2 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryHfwrdImm:\
	arm_inst_info[_arg1 * 16 + _arg2].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + _arg2 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategorySdtr:\
	if (_arg2 == 0xff){\
		for (i = 0; i < 16; i++){\
			arm_inst_info[_arg1 * 16 + i].opcode = ARM_INST_##_name;\
			info = &arm_inst_info[_arg1 * 16 + i ]; \
			info->inst = ARM_INST_##_name; \
			info->category = ARMInstCategory##_category; \
			info->name = #_name; \
			info->fmt_str = _fmt_str; \
			info->size = 4;\
			info->opcode = ARM_INST_##_name; \
		}\
	}\
	else{\
		arm_inst_info[_arg1 * 16 + _arg2].opcode = ARM_INST_##_name;\
		info = &arm_inst_info[_arg1 * 16 + _arg2 ]; \
		info->inst = ARM_INST_##_name; \
		info->category = ARMInstCategory##_category; \
		info->name = #_name; \
		info->fmt_str = _fmt_str; \
		info->size = 4;\
		info->opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + (_arg2 + 8)].opcode = ARM_INST_##_name;\
		info = &arm_inst_info[_arg1 * 16 + (_arg2 + 8)]; \
		info->inst = ARM_INST_##_name; \
		info->category = ARMInstCategory##_category; \
		info->name = #_name; \
		info->fmt_str = _fmt_str; \
		info->size = 4;\
		info->opcode = ARM_INST_##_name; \
	}\
	break;\
	\
	case ARMInstCategoryBrnch:\
	for (i = 0 ; i < 16; i++){\
		for (j = 0 ; j < 16; j++){\
			arm_inst_info[(_arg1 + i ) * 16 + j].opcode = ARM_INST_##_name;\
			info = &arm_inst_info[(_arg1 + i) * 16 + j ]; \
			info->inst = ARM_INST_##_name; \
			info->category = ARMInstCategory##_category; \
			info->name = #_name; \
			info->fmt_str = _fmt_str; \
			info->size = 4;\
			info->opcode = ARM_INST_##_name; \
		}\
	}\
	break;\
	\
	case ARMInstCategoryBdtr:\
	for (i = 0; i < 16; i++){\
		arm_inst_info[_arg1 * 16 + i].opcode = ARM_INST_##_name;\
		info = &arm_inst_info[_arg1 * 16 + i ]; \
		info->inst = ARM_INST_##_name; \
		info->category = ARMInstCategory##_category; \
		info->name = #_name; \
		info->fmt_str = _fmt_str; \
		info->size = 4;\
		info->opcode = ARM_INST_##_name; \
	}\
	break;\
	\
	case ARMInstCategorySwiSvc:\
	for (i = 0 ; i < 16; i++){\
		for (j = 0 ; j < 16; j++){\
			arm_inst_info[(_arg1 + i ) * 16 + j].opcode = ARM_INST_##_name;\
			info = &arm_inst_info[(_arg1 + i) * 16 + j ]; \
			info->inst = ARM_INST_##_name; \
			info->category = ARMInstCategory##_category; \
			info->name = #_name; \
			info->fmt_str = _fmt_str; \
			info->size = 4;\
			info->opcode = ARM_INST_##_name; \
		}\
	}\
	break;\
	\
	case ARMInstCategoryCprDtr:\
	arm_inst_info[_arg1 * 16 + _arg2].opcode = ARM_INST_##_name;\
	info = &arm_inst_info[_arg1 * 16 + _arg2 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARMInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 4;\
	info->opcode = ARM_INST_##_name; \
	break;\
	\
	case ARMInstCategoryVfp:\
	for (i = 0; i < 16; i++){\
		arm_inst_info[_arg1 * 16 + i].opcode = ARM_INST_##_name;\
		info = &arm_inst_info[_arg1 * 16 + i ]; \
		info->inst = ARM_INST_##_name; \
		info->category = ARMInstCategory##_category; \
		info->name = #_name; \
		info->fmt_str = _fmt_str; \
		info->size = 4;\
		info->opcode = ARM_INST_##_name; \
	}\
	break;\
	}
#include "asm.dat"
#undef DEFINST


	/* Allocate Memory */
	self->arm_thumb32_asm_table			= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv2_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv3_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv4_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv5_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv6_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv7_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv8_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv9_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv10_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv11_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv12_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv13_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv14_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_lv15_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_asm_ldst_mul_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst_mul1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst_mul2_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst_mul3_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst_mul4_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst_mul5_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst_mul6_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_asm_ldst_dual_table		= xcalloc(32, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst1_dual_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst2_dual_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_asm_ldst3_dual_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_dproc_shft_reg_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_shft_reg1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_shft_reg2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_shft_reg3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_shft_reg4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_shft_reg5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_shft_reg6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_dproc_imm_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_imm1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_imm2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_imm3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_imm4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_imm5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_imm6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_dproc_reg_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_reg1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_reg2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_reg3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_reg4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_reg5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_reg6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_reg7_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_dproc_misc_table	= xcalloc(8, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_misc1_table	= xcalloc(8, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_dproc_bin_imm_table		= xcalloc(32, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_bin_imm1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_bin_imm2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_dproc_bin_imm3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_brnch_ctrl_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_brnch_ctrl1_table 	= xcalloc(8, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_st_single_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_st_single1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_st_single2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_st_single3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_st_single4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_st_single5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_st_single6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_ld_byte_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_byte1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_byte2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_byte3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_byte4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_byte5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_byte6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_ld_hfword_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_hfword1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_hfword2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_ld_word_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_ld_word1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_mult_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_mult1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_mult_long_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	self->arm_thumb32_mov_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	self->arm_thumb32_mov1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));


	/* Load store Multiple tables */
	self->arm_thumb32_asm_table[1].next_table 		=  self->arm_thumb32_asm_lv1_table;
	self->arm_thumb32_asm_table[1].next_table_high 	= 26;
	self->arm_thumb32_asm_table[1].next_table_low		= 26;

	self->arm_thumb32_asm_lv1_table[0].next_table		= self->arm_thumb32_asm_lv2_table;
	self->arm_thumb32_asm_lv1_table[0].next_table_high	= 25;
	self->arm_thumb32_asm_lv1_table[0].next_table_low	= 25;

	self->arm_thumb32_asm_lv2_table[0].next_table		= self->arm_thumb32_asm_lv3_table;
	self->arm_thumb32_asm_lv2_table[0].next_table_high	= 22;
	self->arm_thumb32_asm_lv2_table[0].next_table_low	= 22;

	self->arm_thumb32_asm_lv3_table[0].next_table		= self->arm_thumb32_asm_ldst_mul_table;
	self->arm_thumb32_asm_lv3_table[0].next_table_high	= 24;
	self->arm_thumb32_asm_lv3_table[0].next_table_low	= 23;

	self->arm_thumb32_asm_ldst_mul_table[1].next_table 		= self->arm_thumb32_asm_ldst_mul1_table;
	self->arm_thumb32_asm_ldst_mul_table[1].next_table_high 	= 20;
	self->arm_thumb32_asm_ldst_mul_table[1].next_table_low 	= 20;

	self->arm_thumb32_asm_ldst_mul1_table[1].next_table 		= self->arm_thumb32_asm_ldst_mul2_table;
	self->arm_thumb32_asm_ldst_mul1_table[1].next_table_high 	= 21;
	self->arm_thumb32_asm_ldst_mul1_table[1].next_table_low 	= 21;

	self->arm_thumb32_asm_ldst_mul2_table[1].next_table 		= self->arm_thumb32_asm_ldst_mul3_table;
	self->arm_thumb32_asm_ldst_mul2_table[1].next_table_high 	= 19;
	self->arm_thumb32_asm_ldst_mul2_table[1].next_table_low 	= 16;

	self->arm_thumb32_asm_ldst_mul_table[2].next_table 		= self->arm_thumb32_asm_ldst_mul4_table;
	self->arm_thumb32_asm_ldst_mul_table[2].next_table_high 	= 20;
	self->arm_thumb32_asm_ldst_mul_table[2].next_table_low 	= 20;

	self->arm_thumb32_asm_ldst_mul4_table[0].next_table 		= self->arm_thumb32_asm_ldst_mul5_table;
	self->arm_thumb32_asm_ldst_mul4_table[0].next_table_high 	= 21;
	self->arm_thumb32_asm_ldst_mul4_table[0].next_table_low 	= 21;

	self->arm_thumb32_asm_ldst_mul5_table[1].next_table 		= self->arm_thumb32_asm_ldst_mul6_table;
	self->arm_thumb32_asm_ldst_mul5_table[1].next_table_high 	= 19;
	self->arm_thumb32_asm_ldst_mul5_table[1].next_table_low 	= 16;

	/* Load store Dual tables */
	self->arm_thumb32_asm_lv3_table[1].next_table		= self->arm_thumb32_asm_ldst_dual_table;
	self->arm_thumb32_asm_lv3_table[1].next_table_high	= 24;
	self->arm_thumb32_asm_lv3_table[1].next_table_low	= 20;

	self->arm_thumb32_asm_ldst_dual_table[(0x07)].next_table	 = self->arm_thumb32_asm_ldst1_dual_table;
	self->arm_thumb32_asm_ldst_dual_table[(0x07)].next_table_high	 = 19;
	self->arm_thumb32_asm_ldst_dual_table[(0x07)].next_table_low	 = 16;

	self->arm_thumb32_asm_ldst_dual_table[(0x08)].next_table	 = self->arm_thumb32_asm_ldst2_dual_table;
	self->arm_thumb32_asm_ldst_dual_table[(0x08)].next_table_high	 = 19;
	self->arm_thumb32_asm_ldst_dual_table[(0x08)].next_table_low	 = 16;

	self->arm_thumb32_asm_ldst_dual_table[(0x0d)].next_table	 = self->arm_thumb32_asm_ldst3_dual_table;
	self->arm_thumb32_asm_ldst_dual_table[(0x0d)].next_table_high	 = 7;
	self->arm_thumb32_asm_ldst_dual_table[(0x0d)].next_table_low	 = 4;

	/* Data Processing Shifted Reg Tables */
	self->arm_thumb32_asm_lv2_table[1].next_table 	= self->arm_thumb32_dproc_shft_reg_table;
	self->arm_thumb32_asm_lv2_table[1].next_table_high	= 24;
	self->arm_thumb32_asm_lv2_table[1].next_table_low 	= 21;

	self->arm_thumb32_dproc_shft_reg_table[0].next_table  	= self->arm_thumb32_dproc_shft_reg1_table;
	self->arm_thumb32_dproc_shft_reg_table[0].next_table_high 	= 11;
	self->arm_thumb32_dproc_shft_reg_table[0].next_table_low 	= 8;

	self->arm_thumb32_dproc_shft_reg_table[2].next_table  	= self->arm_thumb32_dproc_shft_reg2_table;
	self->arm_thumb32_dproc_shft_reg_table[2].next_table_high 	= 19;
	self->arm_thumb32_dproc_shft_reg_table[2].next_table_low 	= 16;

	self->arm_thumb32_dproc_shft_reg_table[3].next_table  	= self->arm_thumb32_dproc_shft_reg3_table;
	self->arm_thumb32_dproc_shft_reg_table[3].next_table_high 	= 19;
	self->arm_thumb32_dproc_shft_reg_table[3].next_table_low 	= 16;

	self->arm_thumb32_dproc_shft_reg_table[4].next_table  	= self->arm_thumb32_dproc_shft_reg4_table;
	self->arm_thumb32_dproc_shft_reg_table[4].next_table_high 	= 11;
	self->arm_thumb32_dproc_shft_reg_table[4].next_table_low 	= 8;

	self->arm_thumb32_dproc_shft_reg_table[8].next_table  	= self->arm_thumb32_dproc_shft_reg5_table;
	self->arm_thumb32_dproc_shft_reg_table[8].next_table_high 	= 11;
	self->arm_thumb32_dproc_shft_reg_table[8].next_table_low 	= 8;

	self->arm_thumb32_dproc_shft_reg_table[(0xd)].next_table  		= self->arm_thumb32_dproc_shft_reg6_table;
	self->arm_thumb32_dproc_shft_reg_table[(0xd)].next_table_high 	= 11;
	self->arm_thumb32_dproc_shft_reg_table[(0xd)].next_table_low 		= 8;

	self->arm_thumb32_dproc_shft_reg2_table[(0xf)].next_table	  	= self->arm_thumb32_mov_table;
	self->arm_thumb32_dproc_shft_reg2_table[(0xf)].next_table_high 	= 5;
	self->arm_thumb32_dproc_shft_reg2_table[(0xf)].next_table_low 	= 4;


	/* Data Processing Immediate Tables */
	self->arm_thumb32_asm_table[2].next_table 		= self->arm_thumb32_asm_lv4_table;
	self->arm_thumb32_asm_table[2].next_table_high 	= 15;
	self->arm_thumb32_asm_table[2].next_table_low 	= 15;

	self->arm_thumb32_asm_lv4_table[0].next_table 	= self->arm_thumb32_asm_lv5_table;
	self->arm_thumb32_asm_lv4_table[0].next_table_high 	= 25;
	self->arm_thumb32_asm_lv4_table[0].next_table_low 	= 25;

	self->arm_thumb32_asm_lv5_table[0].next_table 	= self->arm_thumb32_dproc_imm_table;
	self->arm_thumb32_asm_lv5_table[0].next_table_high 	= 24;
	self->arm_thumb32_asm_lv5_table[0].next_table_low 	= 21;

	self->arm_thumb32_dproc_imm_table[0].next_table  	= self->arm_thumb32_dproc_imm1_table;
	self->arm_thumb32_dproc_imm_table[0].next_table_high 	= 11;
	self->arm_thumb32_dproc_imm_table[0].next_table_low 	= 8;

	self->arm_thumb32_dproc_imm_table[2].next_table  	= self->arm_thumb32_dproc_imm2_table;
	self->arm_thumb32_dproc_imm_table[2].next_table_high 	= 19;
	self->arm_thumb32_dproc_imm_table[2].next_table_low 	= 16;

	self->arm_thumb32_dproc_imm_table[3].next_table  	= self->arm_thumb32_dproc_imm3_table;
	self->arm_thumb32_dproc_imm_table[3].next_table_high 	= 19;
	self->arm_thumb32_dproc_imm_table[3].next_table_low 	= 16;

	self->arm_thumb32_dproc_imm_table[4].next_table  	= self->arm_thumb32_dproc_imm4_table;
	self->arm_thumb32_dproc_imm_table[4].next_table_high 	= 11;
	self->arm_thumb32_dproc_imm_table[4].next_table_low 	= 8;

	self->arm_thumb32_dproc_imm_table[8].next_table  	= self->arm_thumb32_dproc_imm5_table;
	self->arm_thumb32_dproc_imm_table[8].next_table_high 	= 11;
	self->arm_thumb32_dproc_imm_table[8].next_table_low 	= 8;

	self->arm_thumb32_dproc_imm_table[(0xd)].next_table  		= self->arm_thumb32_dproc_imm6_table;
	self->arm_thumb32_dproc_imm_table[(0xd)].next_table_high 	= 11;
	self->arm_thumb32_dproc_imm_table[(0xd)].next_table_low 	= 8;


	/* Data Processing Plain Binary Immediate Tables */
	self->arm_thumb32_asm_lv5_table[1].next_table 	= self->arm_thumb32_dproc_bin_imm_table;
	self->arm_thumb32_asm_lv5_table[1].next_table_high 	= 24;
	self->arm_thumb32_asm_lv5_table[1].next_table_low 	= 20;

	self->arm_thumb32_dproc_bin_imm_table[0].next_table 		= self->arm_thumb32_dproc_bin_imm1_table;
	self->arm_thumb32_dproc_bin_imm_table[0].next_table_high 	= 19;
	self->arm_thumb32_dproc_bin_imm_table[0].next_table_low 	= 16;

	self->arm_thumb32_dproc_bin_imm_table[(0x0a)].next_table 	= self->arm_thumb32_dproc_bin_imm2_table;
	self->arm_thumb32_dproc_bin_imm_table[(0x0a)].next_table_high = 19;
	self->arm_thumb32_dproc_bin_imm_table[(0x0a)].next_table_low 	= 16;

	self->arm_thumb32_dproc_bin_imm_table[(0x16)].next_table 	= self->arm_thumb32_dproc_bin_imm3_table;
	self->arm_thumb32_dproc_bin_imm_table[(0x16)].next_table_high = 19;
	self->arm_thumb32_dproc_bin_imm_table[(0x16)].next_table_low 	= 16;

	/* Branch_control table */
	self->arm_thumb32_asm_lv4_table[1].next_table 	= self->arm_thumb32_brnch_ctrl_table;
	self->arm_thumb32_asm_lv4_table[1].next_table_high 	= 14;
	self->arm_thumb32_asm_lv4_table[1].next_table_low 	= 12;

	self->arm_thumb32_brnch_ctrl_table[0].next_table 	= self->arm_thumb32_brnch_ctrl1_table;
	self->arm_thumb32_brnch_ctrl_table[0].next_table_high = 25;
	self->arm_thumb32_brnch_ctrl_table[0].next_table_low 	= 25;

	self->arm_thumb32_brnch_ctrl_table[2].next_table 	= self->arm_thumb32_brnch_ctrl1_table;
	self->arm_thumb32_brnch_ctrl_table[2].next_table_high = 25;
	self->arm_thumb32_brnch_ctrl_table[2].next_table_low 	= 23;

	/* Single Data table */
	self->arm_thumb32_asm_table[3].next_table 		= self->arm_thumb32_asm_lv6_table;
	self->arm_thumb32_asm_table[3].next_table_high 	= 26;
	self->arm_thumb32_asm_table[3].next_table_low 	= 26;

	self->arm_thumb32_asm_lv6_table[0].next_table 	= self->arm_thumb32_asm_lv7_table;
	self->arm_thumb32_asm_lv6_table[0].next_table_high 	= 25;
	self->arm_thumb32_asm_lv6_table[0].next_table_low 	= 24;

	self->arm_thumb32_asm_lv7_table[0].next_table 	= self->arm_thumb32_asm_lv8_table;
	self->arm_thumb32_asm_lv7_table[0].next_table_high 	= 22;
	self->arm_thumb32_asm_lv7_table[0].next_table_low 	= 20;

	for(i = 0; i < 8; i++)
	{
		if(!(i % 2))
		{
			self->arm_thumb32_asm_lv8_table[i].next_table 	= self->arm_thumb32_st_single_table;
			self->arm_thumb32_asm_lv8_table[i].next_table_high 	= 23;
			self->arm_thumb32_asm_lv8_table[i].next_table_low 	= 21;
		}
	}

	self->arm_thumb32_st_single_table[0].next_table 	= self->arm_thumb32_st_single1_table;
	self->arm_thumb32_st_single_table[0].next_table_high 	= 11;
	self->arm_thumb32_st_single_table[0].next_table_low 	= 11;

	self->arm_thumb32_st_single1_table[1].next_table 	= self->arm_thumb32_st_single2_table;
	self->arm_thumb32_st_single1_table[1].next_table_high	= 9;
	self->arm_thumb32_st_single1_table[1].next_table_low 	= 9;

	self->arm_thumb32_st_single_table[1].next_table 	= self->arm_thumb32_st_single3_table;
	self->arm_thumb32_st_single_table[1].next_table_high 	= 11;
	self->arm_thumb32_st_single_table[1].next_table_low 	= 11;

	self->arm_thumb32_st_single3_table[1].next_table 	= self->arm_thumb32_st_single4_table;
	self->arm_thumb32_st_single3_table[1].next_table_high	= 9;
	self->arm_thumb32_st_single3_table[1].next_table_low 	= 9;

	self->arm_thumb32_st_single_table[2].next_table 	= self->arm_thumb32_st_single5_table;
	self->arm_thumb32_st_single_table[2].next_table_high 	= 11;
	self->arm_thumb32_st_single_table[2].next_table_low 	= 11;

	self->arm_thumb32_st_single5_table[1].next_table 	= self->arm_thumb32_st_single6_table;
	self->arm_thumb32_st_single5_table[1].next_table_high	= 9;
	self->arm_thumb32_st_single5_table[1].next_table_low 	= 9;

	/* Load Byte Table */
	self->arm_thumb32_asm_lv7_table[1].next_table 	= self->arm_thumb32_asm_lv9_table;
	self->arm_thumb32_asm_lv7_table[1].next_table_high 	= 22;
	self->arm_thumb32_asm_lv7_table[1].next_table_low 	= 20;

	self->arm_thumb32_asm_lv9_table[1].next_table 	= self->arm_thumb32_ld_byte_table;
	self->arm_thumb32_asm_lv9_table[1].next_table_high 	= 24;
	self->arm_thumb32_asm_lv9_table[1].next_table_low 	= 23;

	self->arm_thumb32_asm_lv8_table[1].next_table 	= self->arm_thumb32_ld_byte_table;
	self->arm_thumb32_asm_lv8_table[1].next_table_high 	= 24;
	self->arm_thumb32_asm_lv8_table[1].next_table_low 	= 23;

	self->arm_thumb32_ld_byte_table[0].next_table 	= self->arm_thumb32_ld_byte1_table;
	self->arm_thumb32_ld_byte_table[0].next_table_high 	= 19;
	self->arm_thumb32_ld_byte_table[0].next_table_low 	= 16;

	for(i = 0; i < 15; i++)
	{
		self->arm_thumb32_ld_byte1_table[i].next_table 	= self->arm_thumb32_ld_byte2_table;
		self->arm_thumb32_ld_byte1_table[i].next_table_high 	= 11;
		self->arm_thumb32_ld_byte1_table[i].next_table_low 	= 11;
	}

	self->arm_thumb32_ld_byte2_table[1].next_table 	= self->arm_thumb32_ld_byte3_table;
	self->arm_thumb32_ld_byte2_table[1].next_table_high 	= 10;
	self->arm_thumb32_ld_byte2_table[1].next_table_low 	= 8;

	self->arm_thumb32_ld_byte_table[2].next_table 	= self->arm_thumb32_ld_byte4_table;
	self->arm_thumb32_ld_byte_table[2].next_table_high 	= 19;
	self->arm_thumb32_ld_byte_table[2].next_table_low 	= 16;

	for(i = 0; i < 15; i++)
	{
		self->arm_thumb32_ld_byte4_table[i].next_table 	= self->arm_thumb32_ld_byte5_table;
		self->arm_thumb32_ld_byte4_table[i].next_table_high 	= 11;
		self->arm_thumb32_ld_byte4_table[i].next_table_low 	= 11;
	}

	self->arm_thumb32_ld_byte5_table[1].next_table 	= self->arm_thumb32_ld_byte6_table;
	self->arm_thumb32_ld_byte5_table[1].next_table_high 	= 10;
	self->arm_thumb32_ld_byte5_table[1].next_table_low 	= 8;

	/* Load Halfword Table */
	self->arm_thumb32_asm_lv7_table[1].next_table 	= self->arm_thumb32_asm_lv9_table;
	self->arm_thumb32_asm_lv7_table[1].next_table_high 	= 22;
	self->arm_thumb32_asm_lv7_table[1].next_table_low 	= 20;

	self->arm_thumb32_asm_lv9_table[3].next_table 	= self->arm_thumb32_ld_hfword_table;
	self->arm_thumb32_asm_lv9_table[3].next_table_high 	= 24;
	self->arm_thumb32_asm_lv9_table[3].next_table_low 	= 23;

	self->arm_thumb32_asm_lv8_table[3].next_table 	= self->arm_thumb32_ld_hfword_table;
	self->arm_thumb32_asm_lv8_table[3].next_table_high 	= 24;
	self->arm_thumb32_asm_lv8_table[3].next_table_low 	= 23;

	self->arm_thumb32_ld_hfword_table[0].next_table	= self->arm_thumb32_ld_hfword1_table;
	self->arm_thumb32_ld_hfword_table[0].next_table_high 	= 11;
	self->arm_thumb32_ld_hfword_table[0].next_table_low	= 11;

	self->arm_thumb32_ld_hfword_table[2].next_table	= self->arm_thumb32_ld_hfword2_table;
	self->arm_thumb32_ld_hfword_table[2].next_table_high 	= 11;
	self->arm_thumb32_ld_hfword_table[2].next_table_low	= 11;

	/* Load Word Table */
	self->arm_thumb32_asm_lv7_table[1].next_table 	= self->arm_thumb32_asm_lv9_table;
	self->arm_thumb32_asm_lv7_table[1].next_table_high 	= 22;
	self->arm_thumb32_asm_lv7_table[1].next_table_low 	= 20;

	self->arm_thumb32_asm_lv9_table[5].next_table 	= self->arm_thumb32_ld_word_table;
	self->arm_thumb32_asm_lv9_table[5].next_table_high 	= 24;
	self->arm_thumb32_asm_lv9_table[5].next_table_low 	= 23;

	self->arm_thumb32_asm_lv8_table[5].next_table 	= self->arm_thumb32_ld_word_table;
	self->arm_thumb32_asm_lv8_table[5].next_table_high 	= 24;
	self->arm_thumb32_asm_lv8_table[5].next_table_low 	= 23;

	self->arm_thumb32_ld_word_table[0].next_table		= self->arm_thumb32_ld_word1_table;
	self->arm_thumb32_ld_word_table[0].next_table_high 	= 11;
	self->arm_thumb32_ld_word_table[0].next_table_low	= 11;

	/* Data Processing Register Based Table */
	self->arm_thumb32_asm_lv7_table[2].next_table 	= self->arm_thumb32_dproc_reg_table;
	self->arm_thumb32_asm_lv7_table[2].next_table_high 	= 23;
	self->arm_thumb32_asm_lv7_table[2].next_table_low 	= 20;

	self->arm_thumb32_dproc_reg_table[0].next_table	= self->arm_thumb32_dproc_reg1_table;
	self->arm_thumb32_dproc_reg_table[0].next_table_high	= 7;
	self->arm_thumb32_dproc_reg_table[0].next_table_low	= 7;

	self->arm_thumb32_dproc_reg_table[1].next_table	= self->arm_thumb32_dproc_reg2_table;
	self->arm_thumb32_dproc_reg_table[1].next_table_high	= 7;
	self->arm_thumb32_dproc_reg_table[1].next_table_low	= 7;

	self->arm_thumb32_dproc_reg2_table[1].next_table	= self->arm_thumb32_dproc_reg3_table;
	self->arm_thumb32_dproc_reg2_table[1].next_table_high	= 19;
	self->arm_thumb32_dproc_reg2_table[1].next_table_low	= 16;

	self->arm_thumb32_dproc_reg_table[2].next_table	= self->arm_thumb32_dproc_reg4_table;
	self->arm_thumb32_dproc_reg_table[2].next_table_high	= 7;
	self->arm_thumb32_dproc_reg_table[2].next_table_low	= 7;

	self->arm_thumb32_dproc_reg_table[3].next_table	= self->arm_thumb32_dproc_reg5_table;
	self->arm_thumb32_dproc_reg_table[3].next_table_high	= 7;
	self->arm_thumb32_dproc_reg_table[3].next_table_low	= 7;

	self->arm_thumb32_dproc_reg_table[4].next_table	= self->arm_thumb32_dproc_reg6_table;
	self->arm_thumb32_dproc_reg_table[4].next_table_high	= 7;
	self->arm_thumb32_dproc_reg_table[4].next_table_low	= 7;

	self->arm_thumb32_dproc_reg_table[5].next_table	= self->arm_thumb32_dproc_reg7_table;
	self->arm_thumb32_dproc_reg_table[5].next_table_high	= 7;
	self->arm_thumb32_dproc_reg_table[5].next_table_low	= 7;

	self->arm_thumb32_dproc_reg_table[8].next_table	= self->arm_thumb32_dproc_misc_table;
	self->arm_thumb32_dproc_reg_table[8].next_table_high	= 7;
	self->arm_thumb32_dproc_reg_table[8].next_table_low	= 6;

	self->arm_thumb32_dproc_misc_table[2].next_table	= self->arm_thumb32_dproc_misc1_table;
	self->arm_thumb32_dproc_misc_table[2].next_table_high	= 21;
	self->arm_thumb32_dproc_misc_table[2].next_table_low	= 20;


	/* Multiply Tables */
	self->arm_thumb32_asm_lv7_table[3].next_table 	= self->arm_thumb32_asm_lv10_table;
	self->arm_thumb32_asm_lv7_table[3].next_table_high 	= 23;
	self->arm_thumb32_asm_lv7_table[3].next_table_low 	= 23;

	self->arm_thumb32_asm_lv10_table[0].next_table 	= self->arm_thumb32_mult_table;
	self->arm_thumb32_asm_lv10_table[0].next_table_high 	= 5;
	self->arm_thumb32_asm_lv10_table[0].next_table_low 	= 4;

	self->arm_thumb32_mult_table[0].next_table 		= self->arm_thumb32_mult1_table;
	self->arm_thumb32_mult_table[0].next_table_high 	= 15;
	self->arm_thumb32_mult_table[0].next_table_low 	= 12;

	/* Multiply Long Tables */
	self->arm_thumb32_asm_lv10_table[1].next_table 	= self->arm_thumb32_mult_long_table;
	self->arm_thumb32_asm_lv10_table[1].next_table_high 	= 22;
	self->arm_thumb32_asm_lv10_table[1].next_table_low 	= 20;

#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
	ARMThumb32SetupTable(#_name, _fmt_str, ARM_THUMB32_CAT_##_cat, _op1, _op2,\
		_op3, _op4, _op5, _op6, _op7, _op8, ARM_THUMB32_INST_##_name, self);
#include "asm-thumb32.dat"
#undef DEFINST


	self->arm_thumb16_asm_table 			= xcalloc(8, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_shft_ins_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_shft_ins_lv2_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));

	self->arm_thumb16_asm_lv1_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv2_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv3_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv4_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv5_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv6_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv7_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv8_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_asm_lv9_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));


	self->arm_thumb16_data_proc_table		= xcalloc(32, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_spcl_data_brex_table	= xcalloc(32, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_spcl_data_brex_lv1_table	= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));

	self->arm_thumb16_ld_st_table			= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_ld_st_lv1_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_ld_st_lv2_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));


	self->arm_thumb16_misc_table			= xcalloc(128, sizeof(struct arm_thumb16_inst_info_t));
	self->arm_thumb16_it_table			= xcalloc(256, sizeof(struct arm_thumb16_inst_info_t));


	/* Directing to Shift Instructions */
	self->arm_thumb16_asm_table[0].next_table 	= self->arm_thumb16_shft_ins_table;
	self->arm_thumb16_asm_table[0].next_table_high= 13;
	self->arm_thumb16_asm_table[0].next_table_low	= 11;

	self->arm_thumb16_shft_ins_table[3].next_table = self->arm_thumb16_shft_ins_lv2_table;
	self->arm_thumb16_shft_ins_table[3].next_table_high = 10;
	self->arm_thumb16_shft_ins_table[3].next_table_low = 9;

	/* Directing to Data Processing Instructions */
	self->arm_thumb16_asm_table[1].next_table 	= self->arm_thumb16_asm_lv1_table;
	self->arm_thumb16_asm_table[1].next_table_high= 13;
	self->arm_thumb16_asm_table[1].next_table_low	= 13;

	self->arm_thumb16_asm_lv1_table[0].next_table 	= self->arm_thumb16_asm_lv2_table;
	self->arm_thumb16_asm_lv1_table[0].next_table_high	= 12;
	self->arm_thumb16_asm_lv1_table[0].next_table_low	= 12;

	self->arm_thumb16_asm_lv2_table[0].next_table 	= self->arm_thumb16_asm_lv3_table;
	self->arm_thumb16_asm_lv2_table[0].next_table_high	= 11;
	self->arm_thumb16_asm_lv2_table[0].next_table_low	= 10;

	self->arm_thumb16_asm_lv3_table[0].next_table 	= self->arm_thumb16_data_proc_table;
	self->arm_thumb16_asm_lv3_table[0].next_table_high	= 9;
	self->arm_thumb16_asm_lv3_table[0].next_table_low	= 6;

	/* Directing to LD/ST Instructions */
	self->arm_thumb16_asm_lv1_table[1].next_table 	= self->arm_thumb16_ld_st_table;
	self->arm_thumb16_asm_lv1_table[1].next_table_high	= 15;
	self->arm_thumb16_asm_lv1_table[1].next_table_low	= 11;

	self->arm_thumb16_asm_lv2_table[1].next_table 	= self->arm_thumb16_ld_st_table;
	self->arm_thumb16_asm_lv2_table[1].next_table_high	= 15;
	self->arm_thumb16_asm_lv2_table[1].next_table_low	= 11;

	self->arm_thumb16_asm_lv4_table[0].next_table 	= self->arm_thumb16_ld_st_table;
	self->arm_thumb16_asm_lv4_table[0].next_table_high	= 15;
	self->arm_thumb16_asm_lv4_table[0].next_table_low	= 11;

	self->arm_thumb16_ld_st_table[10].next_table		= self->arm_thumb16_ld_st_lv1_table;
	self->arm_thumb16_ld_st_table[10].next_table_high 	= 10;
	self->arm_thumb16_ld_st_table[10].next_table_low 	= 9;

	self->arm_thumb16_ld_st_table[11].next_table 		= self->arm_thumb16_ld_st_lv2_table;
	self->arm_thumb16_ld_st_table[11].next_table_high 	= 10;
	self->arm_thumb16_ld_st_table[11].next_table_low 	= 9;

	/* Directing to Special data Instructions and B&EX instructions*/
	self->arm_thumb16_asm_lv3_table[1].next_table 	= self->arm_thumb16_spcl_data_brex_table;
	self->arm_thumb16_asm_lv3_table[1].next_table_high	= 9;
	self->arm_thumb16_asm_lv3_table[1].next_table_low	= 7;

	self->arm_thumb16_spcl_data_brex_table[0].next_table 	= self->arm_thumb16_spcl_data_brex_lv1_table;
	self->arm_thumb16_spcl_data_brex_table[0].next_table_high	= 6;
	self->arm_thumb16_spcl_data_brex_table[0].next_table_low	= 6;

	/* Directing to Misellaneous 16 bit thumb2 instructions */
	self->arm_thumb16_asm_table[2].next_table = self->arm_thumb16_asm_lv4_table;
	self->arm_thumb16_asm_table[2].next_table_high = 13;
	self->arm_thumb16_asm_table[2].next_table_low = 13;

	self->arm_thumb16_asm_lv4_table[1].next_table 	= self->arm_thumb16_asm_lv5_table;
	self->arm_thumb16_asm_lv4_table[1].next_table_high	= 12;
	self->arm_thumb16_asm_lv4_table[1].next_table_low	= 12;

	self->arm_thumb16_asm_lv5_table[1].next_table 	= self->arm_thumb16_misc_table;
	self->arm_thumb16_asm_lv5_table[1].next_table_high	= 11;
	self->arm_thumb16_asm_lv5_table[1].next_table_low	= 5;

	for(i = 0; i < 8; i++)
	{
		self->arm_thumb16_misc_table[(0x78 + i)].next_table 		= self->arm_thumb16_it_table;
		self->arm_thumb16_misc_table[(0x78 + i)].next_table_high 	= 3;
		self->arm_thumb16_misc_table[(0x78 + i)].next_table_low 	= 0;
	}

	/* Directing to PC and SP relative instructions */
	self->arm_thumb16_asm_lv5_table[0].next_table 	= self->arm_thumb16_asm_lv6_table;
	self->arm_thumb16_asm_lv5_table[0].next_table_high	= 11;
	self->arm_thumb16_asm_lv5_table[0].next_table_low	= 11;

	self->arm_thumb16_asm_table[3].next_table = self->arm_thumb16_asm_lv7_table;
	self->arm_thumb16_asm_table[3].next_table_high = 13;
	self->arm_thumb16_asm_table[3].next_table_low = 12;

	/* Directing to Software interrupt instructions */
	self->arm_thumb16_asm_lv7_table[0].next_table 	= self->arm_thumb16_asm_lv8_table;
	self->arm_thumb16_asm_lv7_table[0].next_table_high	= 11;
	self->arm_thumb16_asm_lv7_table[0].next_table_low	= 11;

	/* Directing to unconditional branch instructions */
	self->arm_thumb16_asm_lv7_table[1].next_table 	= self->arm_thumb16_asm_lv9_table; // entries [0 to 0xe] of lv9 are inst B //
	self->arm_thumb16_asm_lv7_table[1].next_table_high	= 11;
	self->arm_thumb16_asm_lv7_table[1].next_table_low	= 8;



#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
	ARMThumb16SetupTable(#_name, _fmt_str, ARM_THUMB16_CAT_##_cat, _op1, _op2,\
		_op3, _op4, _op5, _op6, ARM_THUMB16_INST_##_name, self);
#include "asm-thumb.dat"
#undef DEFINST

}



void ARMThumb32SetupTable(char* name , char* fmt_str ,
	ARMThumb32InstCategory cat32 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, int op7, int op8, ARMInstThumb32Opcode inst_name, ARMAsm *as)
{
	struct arm_thumb32_inst_info_t *current_table;
	/* We initially start with the first table arm_asm_table, with the opcode field as argument */
	current_table = as->arm_thumb32_asm_table;
	int op[8];
	int i;

	op[0] = op1;
	op[1] = op2;
	op[2] = op3;
	op[3] = op4;
	op[4] = op5;
	op[5] = op6;
	op[6] = op7;
	op[7] = op8;

	i = 0;
	while(1)
	{
		if(current_table[op[i]].next_table && (op[i] >= 0))
		{
			current_table = current_table[op[i]].next_table;
			i++;
		}
		else
		{
			current_table[op[i]].name = name;
			current_table[op[i]].fmt_str = fmt_str;
			current_table[op[i]].cat32 = cat32;
			current_table[op[i]].size = 4;
			current_table[op[i]].inst_32 = inst_name;

			break;
		}
	}
}


void ARMThumb16SetupTable(char* name , char* fmt_str ,
	ARMThumb16InstCategory cat16 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, ARMInstThumb16Opcode inst_name, ARMAsm *as)
{
	struct arm_thumb16_inst_info_t *current_table;
	/* We initially start with the first table arm_asm_table, with the opcode field as argument */
	current_table = as->arm_thumb16_asm_table;
	int op[6];
	int i;

	op[0] = op1;
	op[1] = op2;
	op[2] = op3;
	op[3] = op4;
	op[4] = op5;
	op[5] = op6;

	i = 0;
	while(1)
	{
		if(current_table[op[i]].next_table && (op[i] >= 0))
		{
			current_table = current_table[op[i]].next_table;
			i++;
		}
		else
		{
			current_table[op[i]].name = name;
			current_table[op[i]].fmt_str = fmt_str;
			current_table[op[i]].cat16 = cat16;
			current_table[op[i]].size = 2;
			current_table[op[i]].inst_16 = inst_name;

			break;
		}
	}
}


void ARMAsmDestroy(ARMAsm *self)
{
	/* Thumb 16 tables */
	free(self->arm_thumb16_asm_table);

	free(self->arm_thumb16_shft_ins_table);
	free(self->arm_thumb16_shft_ins_lv2_table);

	free(self->arm_thumb16_asm_lv1_table);
	free(self->arm_thumb16_asm_lv2_table);
	free(self->arm_thumb16_asm_lv3_table);
	free(self->arm_thumb16_asm_lv4_table);
	free(self->arm_thumb16_asm_lv5_table);
	free(self->arm_thumb16_asm_lv6_table);
	free(self->arm_thumb16_asm_lv7_table);
	free(self->arm_thumb16_asm_lv8_table);
	free(self->arm_thumb16_asm_lv9_table);

	free(self->arm_thumb16_data_proc_table);
	free(self->arm_thumb16_spcl_data_brex_table);
	free(self->arm_thumb16_spcl_data_brex_lv1_table);
	free(self->arm_thumb16_ld_st_table);
	free(self->arm_thumb16_ld_st_lv1_table);
	free(self->arm_thumb16_ld_st_lv2_table);
	free(self->arm_thumb16_misc_table);
	free(self->arm_thumb16_it_table);

	/* Thumb 32 tables */
	free(self->arm_thumb32_asm_table);
	free(self->arm_thumb32_asm_lv1_table);
	free(self->arm_thumb32_asm_lv2_table);
	free(self->arm_thumb32_asm_lv3_table);
	free(self->arm_thumb32_asm_lv4_table);
	free(self->arm_thumb32_asm_lv5_table);
	free(self->arm_thumb32_asm_lv6_table);
	free(self->arm_thumb32_asm_lv7_table);
	free(self->arm_thumb32_asm_lv8_table);
	free(self->arm_thumb32_asm_lv9_table);
	free(self->arm_thumb32_asm_lv10_table);
	free(self->arm_thumb32_asm_lv11_table);
	free(self->arm_thumb32_asm_lv12_table);
	free(self->arm_thumb32_asm_lv13_table);
	free(self->arm_thumb32_asm_lv14_table);
	free(self->arm_thumb32_asm_lv15_table);


	free(self->arm_thumb32_asm_ldst_mul_table);
	free(self->arm_thumb32_asm_ldst_mul1_table);
	free(self->arm_thumb32_asm_ldst_mul2_table);
	free(self->arm_thumb32_asm_ldst_mul3_table);
	free(self->arm_thumb32_asm_ldst_mul4_table);
	free(self->arm_thumb32_asm_ldst_mul5_table);
	free(self->arm_thumb32_asm_ldst_mul6_table);

	free(self->arm_thumb32_asm_ldst_dual_table);
	free(self->arm_thumb32_asm_ldst1_dual_table);
	free(self->arm_thumb32_asm_ldst2_dual_table);
	free(self->arm_thumb32_asm_ldst3_dual_table);

	free(self->arm_thumb32_dproc_shft_reg_table);
	free(self->arm_thumb32_dproc_shft_reg1_table);
	free(self->arm_thumb32_dproc_shft_reg2_table);
	free(self->arm_thumb32_dproc_shft_reg3_table);
	free(self->arm_thumb32_dproc_shft_reg4_table);
	free(self->arm_thumb32_dproc_shft_reg5_table);
	free(self->arm_thumb32_dproc_shft_reg6_table);

	free(self->arm_thumb32_dproc_imm_table);
	free(self->arm_thumb32_dproc_imm1_table);
	free(self->arm_thumb32_dproc_imm2_table);
	free(self->arm_thumb32_dproc_imm3_table);
	free(self->arm_thumb32_dproc_imm4_table);
	free(self->arm_thumb32_dproc_imm5_table);
	free(self->arm_thumb32_dproc_imm6_table);

	free(self->arm_thumb32_dproc_reg_table);
	free(self->arm_thumb32_dproc_reg1_table);
	free(self->arm_thumb32_dproc_reg2_table);
	free(self->arm_thumb32_dproc_reg3_table);
	free(self->arm_thumb32_dproc_reg4_table);
	free(self->arm_thumb32_dproc_reg5_table);
	free(self->arm_thumb32_dproc_reg6_table);
	free(self->arm_thumb32_dproc_reg7_table);

	free(self->arm_thumb32_dproc_misc_table);
	free(self->arm_thumb32_dproc_misc1_table);

	free(self->arm_thumb32_st_single_table);
	free(self->arm_thumb32_st_single1_table);
	free(self->arm_thumb32_st_single2_table);
	free(self->arm_thumb32_st_single3_table);
	free(self->arm_thumb32_st_single4_table);
	free(self->arm_thumb32_st_single5_table);
	free(self->arm_thumb32_st_single6_table);

	free(self->arm_thumb32_ld_byte_table);
	free(self->arm_thumb32_ld_byte1_table);
	free(self->arm_thumb32_ld_byte2_table);
	free(self->arm_thumb32_ld_byte3_table);
	free(self->arm_thumb32_ld_byte4_table);
	free(self->arm_thumb32_ld_byte5_table);
	free(self->arm_thumb32_ld_byte6_table);

	free(self->arm_thumb32_ld_hfword_table);
	free(self->arm_thumb32_ld_hfword1_table);
	free(self->arm_thumb32_ld_hfword2_table);

	free(self->arm_thumb32_ld_word_table);
	free(self->arm_thumb32_ld_word1_table);

	free(self->arm_thumb32_mult_table);
	free(self->arm_thumb32_mult1_table);

	free(self->arm_thumb32_dproc_bin_imm_table);
	free(self->arm_thumb32_dproc_bin_imm1_table);
	free(self->arm_thumb32_dproc_bin_imm2_table);
	free(self->arm_thumb32_dproc_bin_imm3_table);

	free(self->arm_thumb32_mult_long_table);
	free(self->arm_thumb32_mov_table);
	free(self->arm_thumb32_mov1_table);

	free(self->arm_thumb32_brnch_ctrl_table);
	free(self->arm_thumb32_brnch_ctrl1_table);
}




/*
 * Public
 */

void arm_inst_hex_dump(FILE *f , void *inst_ptr , unsigned int inst_addr)
{
	printf("%8x:	%08x	", inst_addr, *(unsigned int *) inst_ptr);
}


#if 0
void arm_inst_dump(FILE *f, char *str, int inst_str_size, void *inst_ptr,
	unsigned int inst_index, unsigned int inst_addr)
{
	ARMInst inst;
	int byte_index;
	char *inst_str;
	char **inst_str_ptr;
	char *fmt_str;
	int token_len;



	inst.addr = inst_index;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst.dword.dword[byte_index] = *(unsigned char *) (inst_ptr
			+ byte_index);

	ARMInstDecode(&inst, inst_addr, inst_ptr);

	inst_str = str;
	inst_str_ptr = &str;
	fmt_str = inst.info->fmt_str;
	if (fmt_str)
	{
		while (*fmt_str)
		{
			if (*fmt_str != '%')
			{
				if (!(*fmt_str == ' ' && *inst_str_ptr == inst_str))
					str_printf(inst_str_ptr, &inst_str_size, "%c",
						*fmt_str);
				++fmt_str;
				continue;
			}

			++fmt_str;
			if (arm_token_comp(fmt_str, "rd", &token_len))
				arm_inst_dump_RD(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "rn", &token_len))
				arm_inst_dump_RN(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "rm", &token_len))
				arm_inst_dump_RM(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "rs", &token_len))
				arm_inst_dump_RS(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "rt", &token_len))
				arm_inst_dump_RT(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "op2", &token_len))
				arm_inst_dump_OP2(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "cond", &token_len))
				arm_inst_dump_COND(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "rdlo", &token_len))
				arm_inst_dump_RDLO(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "rdhi", &token_len))
				arm_inst_dump_RDHI(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "psr", &token_len))
				arm_inst_dump_PSR(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "op2psr", &token_len))
				arm_inst_dump_OP2_PSR(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "amode3", &token_len))
				arm_inst_dump_AMODE_3(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "amode2", &token_len))
				arm_inst_dump_AMODE_2(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "idx", &token_len))
				arm_inst_dump_IDX(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "baddr", &token_len))
				arm_inst_dump_BADDR(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category, inst_addr);
			else if (arm_token_comp(fmt_str, "regs", &token_len))
				arm_inst_dump_REGS(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "immd24", &token_len))
				arm_inst_dump_IMMD24(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "immd16", &token_len))
				arm_inst_dump_IMMD16(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "copr", &token_len))
				arm_inst_dump_COPR(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "amode5", &token_len))
				arm_inst_dump_AMODE_5(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "vfp1stmia", &token_len))
				arm_inst_dump_VFP1STM(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "vfp1ldmia", &token_len))
				arm_inst_dump_VFP1LDM(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "vfpregs", &token_len))
				arm_inst_dump_VFP_REGS(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "freg", &token_len))
				arm_inst_dump_FREG(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);
			else if (arm_token_comp(fmt_str, "fp", &token_len))
				arm_inst_dump_FP(inst_str_ptr, &inst_str_size, &inst,
					inst.info->category);

			else
				fatal("%s: token not recognized\n", fmt_str);

			fmt_str += token_len;
		}
		fprintf(f, "%s\n", inst_str);

	}
	else
	{
		fprintf (f,"???\n");
	}

}
#endif


/*
 * Arm disassembler
 */

unsigned int arm_elf_function_symbol(struct elf_file_t *elf_file ,
	unsigned int inst_addr , unsigned int prev_symbol ,
	enum arm_disassembly_mode_t disasm_mode)
{
	unsigned int i;
	struct elf_symbol_t *symbol;
	if (disasm_mode == ARM_DISASM)
	{
		for ( i = 0; i < list_count(elf_file->symbol_table); i++)
		{
			symbol = (struct elf_symbol_t* )list_get(elf_file->symbol_table, i);
			if(symbol->value == inst_addr)
			{
				if((!strncmp(symbol->name, "$",1)))
				{
					continue;
				}
				else
				{
					//if(prev_symbol != symbol->value)
					{
						printf ("\n%08x <%s>\n", symbol->value, symbol->name);
						prev_symbol = symbol->value;
					}
					break;
				}
			}
		}
	}
	else if (disasm_mode == THUMB_DISASM)
	{
		for ( i = 0; i < list_count(elf_file->symbol_table); i++)
		{
			symbol = (struct elf_symbol_t* )list_get(elf_file->symbol_table, i);
			if(symbol->value == (inst_addr + 1))
			{
				if((!strncmp(symbol->name, "$",1)))
				{
					continue;
				}
				else
				{
					//if(prev_symbol != symbol->value)
					{
						printf ("\n%08x <%s>\n", (symbol->value -1), symbol->name);
						prev_symbol = symbol->value;
					}
					break;
				}
			}
		}
	}
	return (prev_symbol);
}

unsigned int arm_dump_word_symbol(struct elf_file_t *elf_file, unsigned int inst_addr, void *inst_ptr)
{
	struct elf_symbol_t *symbol;
	unsigned int word_flag;
	symbol = elf_symbol_get_by_address(elf_file, inst_addr,	NULL);

	if((!strncmp(symbol->name, "$d",2)))
	{
		printf (".word   0x%08x\n", *(unsigned int *)inst_ptr);
		word_flag = 1;
	}
	else
	{
		word_flag = 0;
	}

	return (word_flag);
}

unsigned int thumb_dump_word_symbol(struct elf_file_t *elf_file, unsigned int inst_addr, void *inst_ptr)
{
	struct elf_symbol_t *symbol;
	unsigned int word_flag;
	symbol = elf_symbol_get_by_address(elf_file, inst_addr,	NULL);

	if((!strncmp(symbol->name, "$d",2)))
	{
		printf (".word   0x%08x\n", *(unsigned int *)inst_ptr);
		word_flag = 1;
	}
	else
	{
		word_flag = 0;
	}

	return (word_flag);
}

int comp (const void *arg1,const void *arg2)
{
	struct elf_symbol_t *tmp1;
	struct elf_symbol_t *tmp2;

	tmp1 = (struct elf_symbol_t*)arg1;
	tmp2 = (struct elf_symbol_t*)arg2;

	return (tmp1->value - tmp2->value);
}

void arm_thumb_symbol_list_sort(struct list_t * thumb_symbol_list, struct elf_file_t *elf_file)
{
	struct elf_symbol_t *symbol;
	unsigned int i;

	for ( i = 0; i < list_count(elf_file->symbol_table); i++)
	{
		symbol = (struct elf_symbol_t* )list_get(elf_file->symbol_table, i);
		if((!strncmp(symbol->name, "$t",2)) || (!strncmp(symbol->name, "$a",2)))
		{
			list_add(thumb_symbol_list, symbol);
		}
	}

	list_sort(thumb_symbol_list, comp);

}

int arm_dissassembly_mode_tag(struct list_t * thumb_symbol_list, unsigned int addr)
{
	struct elf_symbol_t *symbol;

	int disasm_mode;

	unsigned int tag_index;
	//unsigned int i;

	tag_index = 0;
	symbol = NULL;

	// Binary search
	int lo;
	int mid;
	int hi;

	lo = 0;
	hi = list_count(thumb_symbol_list) - 1;

	while(lo <= hi)
	{
		mid = (lo + hi) / 2;
		symbol = list_get(thumb_symbol_list, mid);

		if(addr < symbol->value)
			hi = mid - 1;
		else if(addr > symbol->value)
			lo = mid + 1;
		else
			lo = mid + 1;
	}

	/* Linear Search
	 * Deprecated
	 */
	/*
	for (i = 0; i < list_count(thumb_symbol_list); ++i)
	{
		symbol = list_get(thumb_symbol_list, i);
		if (symbol->value > addr)
		{
			tag_index = i - 1;
			break;
		}
	}
	 */

	tag_index = mid;
	symbol = (struct elf_symbol_t *) list_get(thumb_symbol_list, tag_index);
	if (!symbol)
		return ARM_DISASM;

	/* Possible symbols */
	if (!strcmp(symbol->name, "$a"))
		disasm_mode = ARM_DISASM;
	else if (!strcmp(symbol->name, "$t"))
		disasm_mode = THUMB_DISASM;

	/* Return */
	return disasm_mode;
}

void arm_thumb16_inst_hex_dump(FILE *f , void *inst_ptr , unsigned int inst_addr)
{
	printf("%8x:	%04x		", inst_addr, *(unsigned short *) inst_ptr);
}

void arm_thumb32_inst_hex_dump(FILE *f , void *inst_ptr , unsigned int inst_addr)
{
	int thumb_32;
	thumb_32 = *(unsigned int *)inst_ptr;
	printf("%8x:	%04x %04x	", inst_addr, (thumb_32 & 0x0000ffff), ((thumb_32) & 0xffff0000) >> 16);
}

int ARMTestThumb32(void *inst_ptr)
{
	unsigned int byte_index;
	unsigned int arg1;
	ARMInst inst;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst.dword.bytes[byte_index] = *(unsigned char *) (inst_ptr
			+ byte_index);

	arg1 = ((inst.dword.bytes[1] & 0xf8) >> 3);

	if((arg1 == 0x1d) || (arg1 == 0x1e) || (arg1 == 0x1f))
	{
		return (1);
	}
	else
	{
		return(0);
	}
}

void ARMAsmDisassembleBinary(ARMAsm *self, char *path)
{
	ARMAsm *as;

	struct elf_file_t *elf_file;
	struct elf_section_t *section;

	struct list_t *thumb_symbol_list;

	static int disasm_mode;


	int i;
	unsigned int inst_index;
	unsigned int prev_symbol;
	void *inst_ptr;

	/* Initialization */
	as = new(ARMAsm);

	inst_index = 0;
	/* Find .text section which saves instruction bits */
	elf_file = elf_file_create_from_path(path);

	for (i = 0; i < list_count(elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *) list_get(
			elf_file->section_list, i);
		if (!strncmp(section->name, ".text", 5))
			break;
	}
	if (i == list_count(elf_file->section_list))
		fatal(".text section not found!\n");

	thumb_symbol_list = list_create();
	arm_thumb_symbol_list_sort(thumb_symbol_list, elf_file);

	/* Decode and dump instructions */
	for (inst_ptr = section->buffer.ptr; inst_ptr < section->buffer.ptr +
	section->buffer.size; )
	{

		disasm_mode = arm_dissassembly_mode_tag(
			thumb_symbol_list,
			(section->header->sh_addr + inst_index));

		if (disasm_mode == ARM_DISASM)
		{
			prev_symbol = arm_elf_function_symbol(
				elf_file,
				(section->header->sh_addr + inst_index),
				prev_symbol, disasm_mode);

			arm_inst_hex_dump(
				stdout, inst_ptr,
				(section->header->sh_addr + inst_index));

			if (!arm_dump_word_symbol(elf_file, (section->header->sh_addr + inst_index),
				inst_ptr))
			{
				ARMInst inst;

				new_static(&inst, ARMInst, as);
				ARMInstDecode(&inst, section->header->sh_addr + inst_index, inst_ptr);
				ARMInstDump(&inst, stdout);
				delete_static(&inst);
				/*arm_inst_dump(stdout, inst_str,	MAX_STRING_SIZE, inst_ptr,
					inst_index, (section->header->sh_addr + inst_index));*/
			}
			/* Increment instruction buffer index by 4 for ARM mode */
			inst_index += 4;
			inst_ptr += 4;
		}

		else if(disasm_mode == THUMB_DISASM)
		{

			if(ARMTestThumb32(inst_ptr))
			{


				prev_symbol = arm_elf_function_symbol(
					elf_file,
					(section->header->sh_addr + inst_index),
					prev_symbol, disasm_mode);

				arm_thumb32_inst_hex_dump(
					stdout, inst_ptr,
					(section->header->sh_addr + inst_index));

				if (!arm_dump_word_symbol(elf_file, (section->header->sh_addr + inst_index),
					inst_ptr))
				{

					ARMInst inst;
					new_static(&inst, ARMInst, as);
					ARMInstThumb32Decode(&inst, section->header->sh_addr + inst_index, inst_ptr);
					ARMInstThumb32Dump(&inst, stdout);
					delete_static(&inst);
					/* Increment instruction buffer index by 4 for Thumb32 mode */
					inst_index += 4;
					inst_ptr += 4;
				}
				else
				{
					inst_index += 4;
					inst_ptr += 4;
				}
			}

			else
			{

				prev_symbol = arm_elf_function_symbol(
					elf_file,
					(section->header->sh_addr + inst_index),
					prev_symbol, disasm_mode);

				arm_thumb16_inst_hex_dump(
					stdout, inst_ptr,
					(section->header->sh_addr + inst_index));
				if (!arm_dump_word_symbol(elf_file, (section->header->sh_addr + inst_index),
					inst_ptr))
				{
					ARMInst inst;
					new_static(&inst, ARMInst, as);
					ARMInstThumb16Decode(&inst, section->header->sh_addr + inst_index, inst_ptr);
					ARMInstThumb16Dump(&inst, stdout);
					delete_static(&inst);

					/* Increment instruction buffer index by 2 for Thumb16 mode */
					inst_index += 2;
					inst_ptr += 2;
				}
				else
				{
					inst_index += 2;
					inst_ptr += 2;
				}
			}
		}
	}
	list_free(thumb_symbol_list);
	delete(as);
	/* Free external ELF */
	elf_file_free(elf_file);
}

/* Pointer to 'inst' is declared volatile to avoid optimizations when calling 'memset' */


void ARMThumb16InstDebugDump(ARMInst *inst, FILE *f )
{
	ARMInstThumb16Dump(inst, f);
}

void ARMThumb32InstDebugDump(ARMInst *inst, FILE *f )
{



	ARMInstThumb32Dump(inst, f);
}
