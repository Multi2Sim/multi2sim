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
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>
#include "asm-thumb.h"


/* Hard-coded instructions */


void arm_thumb16_disasm_init()
{
	//int op[6];
	int i = 0;
	arm_thumb16_asm_table 			= xcalloc(8, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_shft_ins_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_shft_ins_lv2_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));

	arm_thumb16_asm_lv1_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv2_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv3_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv4_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv5_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv6_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv7_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv8_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv9_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));


	arm_thumb16_data_proc_table		= xcalloc(32, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_spcl_data_brex_table	= xcalloc(32, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_spcl_data_brex_lv1_table	= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));

	arm_thumb16_ld_st_table			= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_ld_st_lv1_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_ld_st_lv2_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));


	arm_thumb16_misc_table			= xcalloc(128, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_it_table			= xcalloc(256, sizeof(struct arm_thumb16_inst_info_t));


	/* Directing to Shift Instructions */
	arm_thumb16_asm_table[0].next_table 	= arm_thumb16_shft_ins_table;
	arm_thumb16_asm_table[0].next_table_high= 13;
	arm_thumb16_asm_table[0].next_table_low	= 11;

	arm_thumb16_shft_ins_table[3].next_table = arm_thumb16_shft_ins_lv2_table;
	arm_thumb16_shft_ins_table[3].next_table_high = 10;
	arm_thumb16_shft_ins_table[3].next_table_low = 9;

	/* Directing to Data Processing Instructions */
	arm_thumb16_asm_table[1].next_table 	= arm_thumb16_asm_lv1_table;
	arm_thumb16_asm_table[1].next_table_high= 13;
	arm_thumb16_asm_table[1].next_table_low	= 13;

	arm_thumb16_asm_lv1_table[0].next_table 	= arm_thumb16_asm_lv2_table;
	arm_thumb16_asm_lv1_table[0].next_table_high	= 12;
	arm_thumb16_asm_lv1_table[0].next_table_low	= 12;

	arm_thumb16_asm_lv2_table[0].next_table 	= arm_thumb16_asm_lv3_table;
	arm_thumb16_asm_lv2_table[0].next_table_high	= 11;
	arm_thumb16_asm_lv2_table[0].next_table_low	= 10;

	arm_thumb16_asm_lv3_table[0].next_table 	= arm_thumb16_data_proc_table;
	arm_thumb16_asm_lv3_table[0].next_table_high	= 9;
	arm_thumb16_asm_lv3_table[0].next_table_low	= 6;

	/* Directing to LD/ST Instructions */
	arm_thumb16_asm_lv1_table[1].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv1_table[1].next_table_high	= 15;
	arm_thumb16_asm_lv1_table[1].next_table_low	= 11;

	arm_thumb16_asm_lv2_table[1].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv2_table[1].next_table_high	= 15;
	arm_thumb16_asm_lv2_table[1].next_table_low	= 11;

	arm_thumb16_asm_lv4_table[0].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv4_table[0].next_table_high	= 15;
	arm_thumb16_asm_lv4_table[0].next_table_low	= 11;

	arm_thumb16_ld_st_table[10].next_table		= arm_thumb16_ld_st_lv1_table;
	arm_thumb16_ld_st_table[10].next_table_high 	= 10;
	arm_thumb16_ld_st_table[10].next_table_low 	= 9;

	arm_thumb16_ld_st_table[11].next_table 		= arm_thumb16_ld_st_lv2_table;
	arm_thumb16_ld_st_table[11].next_table_high 	= 10;
	arm_thumb16_ld_st_table[11].next_table_low 	= 9;

	/* Directing to Special data Instructions and B&EX instructions*/
	arm_thumb16_asm_lv3_table[1].next_table 	= arm_thumb16_spcl_data_brex_table;
	arm_thumb16_asm_lv3_table[1].next_table_high	= 9;
	arm_thumb16_asm_lv3_table[1].next_table_low	= 7;

	arm_thumb16_spcl_data_brex_table[0].next_table 	= arm_thumb16_spcl_data_brex_lv1_table;
	arm_thumb16_spcl_data_brex_table[0].next_table_high	= 6;
	arm_thumb16_spcl_data_brex_table[0].next_table_low	= 6;

	/* Directing to Misellaneous 16 bit thumb2 instructions */
	arm_thumb16_asm_table[2].next_table = arm_thumb16_asm_lv4_table;
	arm_thumb16_asm_table[2].next_table_high = 13;
	arm_thumb16_asm_table[2].next_table_low = 13;

	arm_thumb16_asm_lv4_table[1].next_table 	= arm_thumb16_asm_lv5_table;
	arm_thumb16_asm_lv4_table[1].next_table_high	= 12;
	arm_thumb16_asm_lv4_table[1].next_table_low	= 12;

	arm_thumb16_asm_lv5_table[1].next_table 	= arm_thumb16_misc_table;
	arm_thumb16_asm_lv5_table[1].next_table_high	= 11;
	arm_thumb16_asm_lv5_table[1].next_table_low	= 5;

	for(i = 0; i < 8; i++)
	{
		arm_thumb16_misc_table[(0x78 + i)].next_table 		= arm_thumb16_it_table;
		arm_thumb16_misc_table[(0x78 + i)].next_table_high 	= 3;
		arm_thumb16_misc_table[(0x78 + i)].next_table_low 	= 0;
	}

	/* Directing to PC and SP relative instructions */
	arm_thumb16_asm_lv5_table[0].next_table 	= arm_thumb16_asm_lv6_table;
	arm_thumb16_asm_lv5_table[0].next_table_high	= 11;
	arm_thumb16_asm_lv5_table[0].next_table_low	= 11;

	arm_thumb16_asm_table[3].next_table = arm_thumb16_asm_lv7_table;
	arm_thumb16_asm_table[3].next_table_high = 13;
	arm_thumb16_asm_table[3].next_table_low = 12;

	/* Directing to Software interrupt instructions */
	arm_thumb16_asm_lv7_table[0].next_table 	= arm_thumb16_asm_lv8_table;
	arm_thumb16_asm_lv7_table[0].next_table_high	= 11;
	arm_thumb16_asm_lv7_table[0].next_table_low	= 11;

	/* Directing to unconditional branch instructions */
	arm_thumb16_asm_lv7_table[1].next_table 	= arm_thumb16_asm_lv9_table; // entries [0 to 0xe] of lv9 are inst B //
	arm_thumb16_asm_lv7_table[1].next_table_high	= 11;
	arm_thumb16_asm_lv7_table[1].next_table_low	= 8;



#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
	arm_thumb16_setup_table(#_name, _fmt_str, ARM_THUMB16_CAT_##_cat, _op1, _op2,\
	_op3, _op4, _op5, _op6, ARM_THUMB16_INST_##_name);
#include "asm-thumb.dat"
#undef DEFINST
}

void arm_thumb16_setup_table(char* name , char* fmt_str ,
	enum arm_thumb16_cat_enum cat16 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, enum arm_thumb16_inst_enum inst_name)
{
	struct arm_thumb16_inst_info_t *current_table;
	/* We initially start with the first table arm_asm_table, with the opcode field as argument */
	current_table = arm_thumb16_asm_table;
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
			current_table[op[i]].inst = inst_name;

			break;
		}
	}
	printf(" %s \n", fmt_str);// Remove this YU
}


static int arm_token_comp(char *fmt_str, char *token_str, int *token_len)
{
	*token_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_len) &&
		!isalnum(fmt_str[*token_len]);
}


void arm_thumb16_inst_dump(FILE *f , char *str , int inst_str_size , void *inst_ptr ,
	unsigned int inst_index, unsigned int inst_addr)
{
	struct arm_thumb16_inst_t inst;
	int byte_index;
	char *inst_str;
	char **inst_str_ptr;
	char *fmt_str;
	int token_len;

	inst.addr = inst_index;
	for (byte_index = 0; byte_index < 2; ++byte_index)
		inst.dword.bytes[byte_index] = *(unsigned char *) (inst_ptr
			+ byte_index);

	arm_thumb16_inst_decode(&inst); // Change to thumb2
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
				arm_thumb16_inst_dump_RD(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat16);
			else if (arm_token_comp(fmt_str, "rm", &token_len))
				arm_thumb16_inst_dump_RM(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat16);
			else if (arm_token_comp(fmt_str, "rn", &token_len))
				arm_thumb16_inst_dump_RN(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat16);
			else if (arm_token_comp(fmt_str, "immd8", &token_len))
				arm_thumb16_inst_dump_IMMD8(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16, inst_addr);
			else if (arm_token_comp(fmt_str, "immd5", &token_len))
				arm_thumb16_inst_dump_IMMD5(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16, inst_addr);
			else if (arm_token_comp(fmt_str, "immd3", &token_len))
				arm_thumb16_inst_dump_IMMD3(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16);
			else if (arm_token_comp(fmt_str, "cond", &token_len))
				arm_thumb16_inst_dump_COND(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16);
			else if (arm_token_comp(fmt_str, "regs", &token_len))
				arm_thumb16_inst_dump_REGS(inst_str_ptr, &inst_str_size, &inst,
											inst.info->cat16);
			else if (arm_token_comp(fmt_str, "x", &token_len))
				arm_thumb16_inst_dump_it_eq_x(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16);
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

void arm_thumb16_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int rd;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			rd = inst->dword.movshift_reg_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rd = inst->dword.addsub_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			rd = inst->dword.immd_oprs_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			rd = inst->dword.dpr_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			rd = ((inst->dword.high_oprs_ins.h1 << 3) |  inst->dword.high_oprs_ins.reg_rd);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			rd = inst->dword.pcldr_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rd = inst->dword.ldstr_reg_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rd = inst->dword.ldstr_exts_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			rd = inst->dword.ldstr_immd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			rd = inst->dword.ldstr_hfwrd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			rd = inst->dword.sp_immd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			rd = inst->dword.addsp_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_CMP_T2)
			rd = (inst->dword.cmp_t2.N << 3 | inst->dword.cmp_t2.reg_rn);

		else
			fatal("%d: rd fmt not recognized", cat);

		switch (rd)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rd);
			break;
		}
}

void arm_thumb16_inst_dump_RM(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int rm;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			rm = inst->dword.movshift_reg_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rm = inst->dword.addsub_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			rm = inst->dword.dpr_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			rm = inst->dword.high_oprs_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rm = inst->dword.ldstr_reg_ins.reg_ro;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rm = inst->dword.ldstr_exts_ins.reg_ro;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_CMP_T2)
			rm = inst->dword.cmp_t2.reg_rm;
		else
			fatal("%d: rm fmt not recognized", cat);

		switch (rm)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rm);
			break;
		}
}

void arm_thumb16_inst_dump_RN(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int rn;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rn = inst->dword.addsub_ins.rn_imm;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			rn = inst->dword.immd_oprs_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rn = inst->dword.ldstr_reg_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rn = inst->dword.ldstr_exts_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			rn = inst->dword.ldstr_immd_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			rn = inst->dword.ldstr_hfwrd_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			rn = inst->dword.ldm_stm_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_CBNZ)
			rn = inst->dword.cbnz_ins.reg_rn;
		else
			fatal("%d: rn fmt not recognized", cat);

		switch (rn)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rn);
			break;
		}
}

void arm_thumb16_inst_dump_IMMD8(char **inst_str_ptr , int *inst_str_size ,
	struct arm_thumb16_inst_t *inst , enum arm_thumb16_cat_enum cat ,
	unsigned int inst_addr)
{
	unsigned int immd8;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			immd8 = inst->dword.immd_oprs_ins.offset8;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			immd8 =(inst->dword.pcldr_ins.immd_8 << 2);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			immd8 = 4 * inst->dword.sp_immd_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			immd8 = 4 * inst->dword.addsp_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			immd8 = 4 * inst->dword.sub_sp_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			immd8 = inst->dword.cond_br_ins.s_offset;
		else if (cat == ARM_THUMB16_CAT_MISC_SVC_INS)
			immd8 = inst->dword.svc_ins.value;
		else
			fatal("%d: immd8 fmt not recognized", cat);

		if(cat == ARM_THUMB16_CAT_MISC_BR)
		{
			if((immd8 >> 7))
			{
				immd8 = ((inst_addr + 4) + ((immd8 << 1) | 0xffffff00));
			}
			else
			{
				immd8 = (inst_addr + 4) + (immd8 << 1);
			}
			str_printf(inst_str_ptr, inst_str_size, "%x",immd8);
		}
		else
			str_printf(inst_str_ptr, inst_str_size, "#%d",immd8);

}

void arm_thumb16_inst_dump_IMMD3(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int immd3;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			immd3 = inst->dword.addsub_ins.rn_imm;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			fatal("%d: immd3 fmt not recognized", cat);

		else
			fatal("%d: immd3 fmt not recognized", cat);



		str_printf(inst_str_ptr, inst_str_size, "#%d",immd3);

}

void arm_thumb16_inst_dump_IMMD5(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat,
	unsigned int inst_addr)
{
	unsigned int immd5;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			immd5 = inst->dword.movshift_reg_ins.offset;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			immd5 = inst->dword.ldstr_immd_ins.offset;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			immd5 = inst->dword.ldstr_hfwrd_ins.offset;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_CBNZ)
			immd5 = inst->dword.cbnz_ins.immd_5;
		else
			fatal("%d: immd5 fmt not recognized", cat);

		if(cat == ARM_THUMB16_CAT_MISC_CBNZ)
		{
			immd5 = (inst_addr + 4) + (immd5 << 1);
			str_printf(inst_str_ptr, inst_str_size, "%x",immd5);
		}
		else
			str_printf(inst_str_ptr, inst_str_size, "#%d",immd5);

}

void arm_thumb16_inst_dump_COND(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int cond;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			cond = inst->dword.if_eq_ins.first_cond;
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			cond = inst->dword.cond_br_ins.cond;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			cond = inst->dword.if_eq_ins.first_cond;

		else
			fatal("%d: rm fmt not recognized", cat);

		switch (cond)
			{
			case (EQ):
			str_printf(inst_str_ptr, inst_str_size, "eq");
			break;

			case (NE):
			str_printf(inst_str_ptr, inst_str_size, "ne");
			break;

			case (CS):
			str_printf(inst_str_ptr, inst_str_size, "cs");
			break;

			case (CC):
			str_printf(inst_str_ptr, inst_str_size, "cc");
			break;

			case (MI):
			str_printf(inst_str_ptr, inst_str_size, "mi");
			break;

			case (PL):
			str_printf(inst_str_ptr, inst_str_size, "pl");
			break;

			case (VS):
			str_printf(inst_str_ptr, inst_str_size, "vs");
			break;

			case (VC):
			str_printf(inst_str_ptr, inst_str_size, "vc");
			break;

			case (HI):
			str_printf(inst_str_ptr, inst_str_size, "hi");
			break;

			case (LS):
			str_printf(inst_str_ptr, inst_str_size, "ls");
			break;

			case (GE):
			str_printf(inst_str_ptr, inst_str_size, "ge");
			break;

			case (LT):
			str_printf(inst_str_ptr, inst_str_size, "lt");
			break;

			case (GT):
			str_printf(inst_str_ptr, inst_str_size, "gt");
			break;

			case (LE):
			str_printf(inst_str_ptr, inst_str_size, "le");
			break;

			case (AL):
			str_printf(inst_str_ptr, inst_str_size, " ");
			break;
			}

}

void arm_thumb16_inst_dump_REGS(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int regs;
	unsigned int i;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			regs = inst->dword.ldm_stm_ins.reg_list;
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_PUSH_POP)
			regs = inst->dword.push_pop_ins.reg_list;
		else
			fatal("%d: regs fmt not recognized", cat);

		str_printf(inst_str_ptr, inst_str_size, "{");
		for (i = 1; i < 65536; i *= 2)
		{
			if(regs & (i))
			{
				str_printf(inst_str_ptr, inst_str_size, "r%d ", log_base2(i));
			}
		}

		str_printf(inst_str_ptr, inst_str_size, "}");


}

void arm_thumb16_inst_dump_it_eq_x(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int first_cond;
	unsigned int mask;

	if (cat == ARM_THUMB16_CAT_IF_THEN)
	{
		mask = inst->dword.if_eq_ins.mask;
		first_cond = inst->dword.if_eq_ins.first_cond;
	}
	else
		fatal("%d: x fmt not recognized", cat);

	if((mask != 0x8))
	{
		if((mask >> 3) ^ (first_cond & 1))
			str_printf(inst_str_ptr, inst_str_size, "e");
		else
			str_printf(inst_str_ptr, inst_str_size, "t");
	}
}

void arm_thumb32_inst_dump(FILE *f , char *str , int inst_str_size , void *inst_ptr ,
	unsigned int inst_index, unsigned int inst_addr)
{
	printf("???\n");
}

void arm_thumb16_inst_decode(struct arm_thumb16_inst_t *inst)
{
	struct arm_thumb16_inst_info_t *current_table;
		/* We initially start with the first table mips_asm_table, with the opcode field as argument */
		current_table = arm_thumb16_asm_table;
		int current_table_low = 14;
		int current_table_high = 15;
		unsigned int thumb16_table_arg;
		int loop_iteration = 0;

		thumb16_table_arg =  BITS16(*(unsigned short*)inst->dword.bytes, current_table_high, current_table_low);

		/* Find next tables if the instruction belongs to another table */
		while (1) {
			if (current_table[thumb16_table_arg].next_table && loop_iteration < 6) {
				current_table_high = current_table[thumb16_table_arg].next_table_high;
				current_table_low = current_table[thumb16_table_arg].next_table_low;
				current_table = current_table[thumb16_table_arg].next_table;
	//			thumb16_table_arg = BITS16(*(unsigned short*)inst->dword.bytes, current_table_high, current_table_low);
	//			loop_iteration++;
			}
			else if (loop_iteration > 6) {
				fatal("Can not find the correct table containing the instruction\n");
			}
			else
				break;

		}

		inst->info = &current_table[thumb16_table_arg];
}

void arm_disasm_done()
{

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

}
