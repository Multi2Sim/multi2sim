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
}


void ARMAsmDestroy(ARMAsm *self)
{
	/* Thumb 16 tables */
	free(arm_thumb16_asm_table);

	free(arm_thumb16_shft_ins_table);
	free(arm_thumb16_shft_ins_lv2_table);

	free(arm_thumb16_asm_lv1_table);
	free(arm_thumb16_asm_lv2_table);
	free(arm_thumb16_asm_lv3_table);
	free(arm_thumb16_asm_lv4_table);
	free(arm_thumb16_asm_lv5_table);
	free(arm_thumb16_asm_lv6_table);
	free(arm_thumb16_asm_lv7_table);
	free(arm_thumb16_asm_lv8_table);
	free(arm_thumb16_asm_lv9_table);

	free(arm_thumb16_data_proc_table);
	free(arm_thumb16_spcl_data_brex_table);
	free(arm_thumb16_spcl_data_brex_lv1_table);
	free(arm_thumb16_ld_st_table);
	free(arm_thumb16_ld_st_lv1_table);
	free(arm_thumb16_ld_st_lv2_table);
	free(arm_thumb16_misc_table);
	free(arm_thumb16_it_table);

	/* Thumb 32 tables */
	free(arm_thumb32_asm_table);
	free(arm_thumb32_asm_lv1_table);
	free(arm_thumb32_asm_lv2_table);
	free(arm_thumb32_asm_lv3_table);
	free(arm_thumb32_asm_lv4_table);
	free(arm_thumb32_asm_lv5_table);
	free(arm_thumb32_asm_lv6_table);
	free(arm_thumb32_asm_lv7_table);
	free(arm_thumb32_asm_lv8_table);
	free(arm_thumb32_asm_lv9_table);
	free(arm_thumb32_asm_lv10_table);
	free(arm_thumb32_asm_lv11_table);
	free(arm_thumb32_asm_lv12_table);
	free(arm_thumb32_asm_lv13_table);
	free(arm_thumb32_asm_lv14_table);
	free(arm_thumb32_asm_lv15_table);


	free(arm_thumb32_asm_ldst_mul_table);
	free(arm_thumb32_asm_ldst_mul1_table);
	free(arm_thumb32_asm_ldst_mul2_table);
	free(arm_thumb32_asm_ldst_mul3_table);
	free(arm_thumb32_asm_ldst_mul4_table);
	free(arm_thumb32_asm_ldst_mul5_table);
	free(arm_thumb32_asm_ldst_mul6_table);

	free(arm_thumb32_asm_ldst_dual_table);
	free(arm_thumb32_asm_ldst1_dual_table);
	free(arm_thumb32_asm_ldst2_dual_table);
	free(arm_thumb32_asm_ldst3_dual_table);

	free(arm_thumb32_dproc_shft_reg_table);
	free(arm_thumb32_dproc_shft_reg1_table);
	free(arm_thumb32_dproc_shft_reg2_table);
	free(arm_thumb32_dproc_shft_reg3_table);
	free(arm_thumb32_dproc_shft_reg4_table);
	free(arm_thumb32_dproc_shft_reg5_table);
	free(arm_thumb32_dproc_shft_reg6_table);

	free(arm_thumb32_dproc_imm_table);
	free(arm_thumb32_dproc_imm1_table);
	free(arm_thumb32_dproc_imm2_table);
	free(arm_thumb32_dproc_imm3_table);
	free(arm_thumb32_dproc_imm4_table);
	free(arm_thumb32_dproc_imm5_table);
	free(arm_thumb32_dproc_imm6_table);

	free(arm_thumb32_dproc_reg_table);
	free(arm_thumb32_dproc_reg1_table);
	free(arm_thumb32_dproc_reg2_table);
	free(arm_thumb32_dproc_reg3_table);
	free(arm_thumb32_dproc_reg4_table);
	free(arm_thumb32_dproc_reg5_table);
	free(arm_thumb32_dproc_reg6_table);
	free(arm_thumb32_dproc_reg7_table);

	free(arm_thumb32_dproc_misc_table);
	free(arm_thumb32_dproc_misc1_table);

	free(arm_thumb32_st_single_table);
	free(arm_thumb32_st_single1_table);
	free(arm_thumb32_st_single2_table);
	free(arm_thumb32_st_single3_table);
	free(arm_thumb32_st_single4_table);
	free(arm_thumb32_st_single5_table);
	free(arm_thumb32_st_single6_table);

	free(arm_thumb32_ld_byte_table);
	free(arm_thumb32_ld_byte1_table);
	free(arm_thumb32_ld_byte2_table);
	free(arm_thumb32_ld_byte3_table);
	free(arm_thumb32_ld_byte4_table);
	free(arm_thumb32_ld_byte5_table);
	free(arm_thumb32_ld_byte6_table);

	free(arm_thumb32_ld_hfword_table);
	free(arm_thumb32_ld_hfword1_table);
	free(arm_thumb32_ld_hfword2_table);

	free(arm_thumb32_ld_word_table);
	free(arm_thumb32_ld_word1_table);

	free(arm_thumb32_mult_table);
	free(arm_thumb32_mult1_table);

	free(arm_thumb32_dproc_bin_imm_table);
	free(arm_thumb32_dproc_bin_imm1_table);
	free(arm_thumb32_dproc_bin_imm2_table);
	free(arm_thumb32_dproc_bin_imm3_table);

	free(arm_thumb32_mult_long_table);
	free(arm_thumb32_mov_table);
	free(arm_thumb32_mov1_table);

	free(arm_thumb32_brnch_ctrl_table);
	free(arm_thumb32_brnch_ctrl1_table);
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
		inst.bytes.bytes[byte_index] = *(unsigned char *) (inst_ptr
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

int arm_test_thumb32(void *inst_ptr)
{
	unsigned int byte_index;
	unsigned int arg1;
	ARMInst inst;
	for (byte_index = 0; byte_index < 4; ++byte_index)
			inst.bytes.bytes[byte_index] = *(unsigned char *) (inst_ptr
				+ byte_index);

	arg1 = ((inst.bytes.bytes[1] & 0xf8) >> 3);

	if((arg1 == 0x1d) || (arg1 == 0x1e) || (arg1 == 0x1f))
	{
		return (1);
	}
	else
	{
		return(0);
	}
}

void arm_emu_disasm(char *path)
{
	ARMAsm *as;

	struct elf_file_t *elf_file;
	struct elf_section_t *section;

	struct list_t *thumb_symbol_list;

	static int disasm_mode;

	char inst_str[MAX_STRING_SIZE];
	int i;
	unsigned int inst_index;
	unsigned int prev_symbol;
	void *inst_ptr;

	/* Initialization */
	as = new(ARMAsm);
	
	arm_thumb16_disasm_init();
	arm_thumb32_disasm_init();
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

			if(arm_test_thumb32(inst_ptr))
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


					arm_thumb32_inst_dump(stdout, inst_str,	MAX_STRING_SIZE, inst_ptr,
						inst_index, (section->header->sh_addr + inst_index));

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
					arm_thumb16_inst_dump(stdout, inst_str,	MAX_STRING_SIZE, inst_ptr,
						inst_index, (section->header->sh_addr + inst_index));
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
void arm_disasm(void *buf, unsigned int ip, volatile ARMInst *inst)
{
	unsigned int byte_index;
	unsigned int arg1;
	unsigned int arg2;

	inst->addr = ip;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst->bytes.bytes[byte_index] = *(unsigned char *) (buf + byte_index);

	arg1 = ((inst->bytes.bytes[3] & 0x0f) << 4) | ((inst->bytes.bytes[2] & 0xf0) >> 4);

	arg2 = ((inst->bytes.bytes[0] & 0xf0) >> 4);

	inst->info = &arm_inst_info[arg1 * 16 + arg2];

}

void thumb16_disasm(void *buf, unsigned int ip, volatile struct arm_thumb16_inst_t *inst)
{
	unsigned int byte_index;
	inst->addr = ip;
	for (byte_index = 0; byte_index < 2; ++byte_index)
		inst->dword.bytes[byte_index] = *(unsigned char *) (buf + byte_index);

	arm_thumb16_inst_decode((struct arm_thumb16_inst_t*) inst);
}

void thumb32_disasm(void *buf, unsigned int ip, volatile struct arm_thumb32_inst_t *inst)
{
	unsigned int byte_index;
	inst->addr = ip - 2;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst->dword.bytes[byte_index] = *(unsigned char *) (buf
			+ ((byte_index + 2) % 4));


	arm_thumb32_inst_decode((struct arm_thumb32_inst_t*) inst);

}


void arm_th16_inst_debug_dump(struct arm_thumb16_inst_t *inst, FILE *f )
{

	char inst_str[MAX_STRING_SIZE];
	void *inst_ptr;

	inst_ptr = &inst->dword.bytes;
	arm_thumb16_inst_dump(f, inst_str, MAX_STRING_SIZE, inst_ptr, inst->addr, inst->addr);
}

void arm_th32_inst_debug_dump(struct arm_thumb32_inst_t *inst, FILE *f )
{

	char inst_str[MAX_STRING_SIZE];
	struct arm_thumb32_inst_t inst_ptr;
	unsigned int byte_index;

	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst_ptr.dword.bytes[byte_index] = inst->dword.bytes[((byte_index
			+ 2) % 4)];

	arm_thumb32_inst_dump(f, inst_str, MAX_STRING_SIZE, &inst_ptr.dword.bytes, inst->addr, inst->addr);
}
