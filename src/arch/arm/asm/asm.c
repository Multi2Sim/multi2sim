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

#include <arm-asm.h>

void arm_disasm_init()
{
	struct arm_inst_info_t *info;
	int i;
	/* Form the Instruction table */
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	switch (ARM_CAT_##_category) { \
	case ARM_CAT_DPR_REG: \
		arm_inst_info[_arg1 * 16 + 0].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 1].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 3].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 4].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 5].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 6].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 7].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 8].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 10].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 12].opcode = ARM_INST_##_name; \
		arm_inst_info[_arg1 * 16 + 14].opcode = ARM_INST_##_name; \
		break; \
	case ARM_CAT_DPR_IMM: \
		for (i = 0; i < 16; i++ )\
		arm_inst_info[_arg1 * 16 + i].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_DPR_SAT:\
		arm_inst_info[_arg1 * 16 + 5].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_PSR:\
		arm_inst_info[_arg1 * 16 + 0].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_MULT:\
		arm_inst_info[_arg1 * 16 + 9].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_MULT_SIGN:\
		arm_inst_info[_arg1 * 16 + 8].opcode = ARM_INST_##_name;\
		arm_inst_info[_arg1 * 16 + 10].opcode = ARM_INST_##_name;\
		arm_inst_info[_arg1 * 16 + 12].opcode = ARM_INST_##_name;\
		arm_inst_info[_arg1 * 16 + 14].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_MULT_LN:\
		arm_inst_info[_arg1 * 16 + 9].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_MULT_LN_SIGN:\
		arm_inst_info[_arg1 * 16 + 8].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_SDSWP:\
		arm_inst_info[_arg1 * 16 + 9].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_BAX:\
		arm_inst_info[_arg1 * 16 + _arg2].opcode = ARM_INST_##_name;\
		break;\
	case ARM_CAT_HFWRD_REG:\
		arm_inst_info[_arg1 * 16 + _arg2].opcode = ARM_INST_##_name;\
		arm_inst_info[(_arg1 + 2) * 16 + _arg2].opcode = ARM_INST_##_name;\
		break;\
	}
#include "arm-asm.dat"
#undef DEFINST

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	info = &arm_inst_info[_arg1 * 16 + _arg2 ]; \
	info->inst = ARM_INST_##_name; \
	info->category = ARM_CAT_##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->size = 32;
	/*info->opcode = _opcode; \*/
#include "arm-asm.dat"
#undef DEFINST
}

static int arm_token_comp(char *fmt_str, char *token_str, int *token_len)
{
	*token_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_len) &&
		!isalnum(fmt_str[*token_len]);
}

void arm_inst_decode(struct arm_inst_t *inst)
{
	unsigned int arg1;
	unsigned int arg2;

	arg1 = ((inst->dword.bytes[3] & 0x0f) << 4) | ((inst->dword.bytes[2] & 0xf0) >> 4);

	arg2 = ((inst->dword.bytes[0] & 0xf0) >> 4);

	if(!(inst->info->fmt_str))
	{
		printf("%d: Instruction not supported\n", inst->addr);
		inst = 0;
	}
	else
	{
		inst->info = &arm_inst_info[arg1 * 16 + arg2];
	}
}

void arm_inst_hex_dump(FILE *f, unsigned char *section_buf, int section_buf_size)
{
	int inst_index;
	int byte_index;

	for (inst_index = 0; inst_index < section_buf_size / 4; ++inst_index)
	{
		printf("Instruction Hex: ");
		/* Print most significant byte first */
		for (byte_index = 3; byte_index >= 0; --byte_index)
		{
			printf(
			                "%02x",
			                *(section_buf + 4 * inst_index
			                                + byte_index));
		}
		printf("\n");
	}
	printf("\n\n");
}

void arm_inst_dump(FILE *f, char *str, int inst_str_size, unsigned char *buf,
                int inst_index)
{
	struct arm_inst_t inst;
	int byte_index;
	char *inst_str;
	char **inst_str_ptr;
	char *fmt_str;
	int token_len;

	inst.addr = 4 * inst_index;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst.dword.bytes[byte_index] = *(buf + 4 * inst_index
		                + byte_index);

	arm_inst_decode(&inst);

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

			else
				fatal("%c: token not recognized\n", *fmt_str);

			fmt_str += token_len;
		}
	}
	fprintf(f, "%s\n", inst_str);
}

unsigned int arm_rotl(unsigned int value, unsigned int shift)
{
	if ((shift &= sizeof(value) * 8 - 1) == 0)
	return value;

	return (value << shift) | (value >> (sizeof(value) * 8 - shift));
}

unsigned int arm_rotr(unsigned int value, unsigned int shift)
{
	if ((shift &= sizeof(value) * 8 - 1) == 0)
	return value;

	return (value >> shift) | (value << (sizeof(value) * 8 - shift));
}

void arm_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int rd;

	if (cat == ARM_CAT_DPR_REG)
		rd = inst->dword.dpr_ins.dst_reg;
	else if (cat == ARM_CAT_DPR_IMM)
		rd = inst->dword.dpr_ins.dst_reg;
	else if (cat == ARM_CAT_DPR_SAT)
		rd = inst->dword.dpr_sat_ins.dst_reg;
	else if (cat == ARM_CAT_PSR)
		rd = inst->dword.psr_ins.dst_reg;
	else if (cat == ARM_CAT_MULT)
		rd = inst->dword.mult_ins.dst_rd;
	else if (cat == ARM_CAT_MULT_SIGN)
		rd = inst->dword.mult_ins.dst_rd;
	else if (cat == ARM_CAT_MULT_LN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_REG)
		rd = inst->dword.hfwrd_reg_ins.dst_rd;
	else if (cat == ARM_CAT_HFWRD_IMM)
		rd = inst->dword.hfwrd_imm_ins.dst_rd;
	else if (cat == ARM_CAT_SDTR)
		rd = inst->dword.sdtr_ins.src_dst_rd;
	else if (cat == ARM_CAT_SDSWP)
		rd = inst->dword.sngl_dswp_ins.dst_rd;
	else if (cat == ARM_CAT_CPR_RTR)
		rd = inst->dword.cpr_rtr_ins.rd;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	str_printf(inst_str_ptr, inst_str_size, "r%d", rd);

}

void arm_inst_dump_RN(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int rn;

	if (cat == ARM_CAT_DPR_REG)
		rn = inst->dword.dpr_ins.op1_reg;
	else if (cat == ARM_CAT_DPR_IMM)
		rn = inst->dword.dpr_ins.op1_reg;
	else if (cat == ARM_CAT_DPR_SAT)
		rn = inst->dword.dpr_sat_ins.op1_reg;
	else if (cat == ARM_CAT_PSR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT)
		rn = inst->dword.mult_ins.op2_rn;
	else if (cat == ARM_CAT_MULT_SIGN)
		rn = inst->dword.mult_ins.op2_rn;
	else if (cat == ARM_CAT_MULT_LN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_REG)
		rn = inst->dword.hfwrd_reg_ins.base_rn;
	else if (cat == ARM_CAT_HFWRD_IMM)
		rn = inst->dword.hfwrd_imm_ins.base_rn;
	else if (cat == ARM_CAT_SDTR)
		rn = inst->dword.sdtr_ins.base_rn;
	else if (cat == ARM_CAT_SDSWP)
		rn = inst->dword.sngl_dswp_ins.base_rn;
	else if (cat == ARM_CAT_CPR_RTR)
		rn = inst->dword.cpr_rtr_ins.cpr_rn;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	str_printf(inst_str_ptr, inst_str_size, "r%d", rn);


}

void arm_inst_dump_RM(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int rm;

	if (cat == ARM_CAT_DPR_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_SAT)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_PSR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT)
		rm = inst->dword.mult_ins.op0_rm;
	else if (cat == ARM_CAT_MULT_SIGN)
		rm = inst->dword.mult_ins.op0_rm;
	else if (cat == ARM_CAT_MULT_LN)
		rm = inst->dword.mul_ln_ins.op0_rm;
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		rm = inst->dword.mul_ln_ins.op0_rm;
	else if (cat == ARM_CAT_HFWRD_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDTR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDSWP)
		rm = inst->dword.sngl_dswp_ins.op0_rm;
	else if (cat == ARM_CAT_CPR_RTR)
		rm = inst->dword.cpr_rtr_ins.cpr_op_rm;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	str_printf(inst_str_ptr, inst_str_size, "r%d", rm);

}

void arm_inst_dump_RS(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int rs;

	if (cat == ARM_CAT_DPR_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_SAT)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_PSR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT)
		rs = inst->dword.mult_ins.op1_rs;
	else if (cat == ARM_CAT_MULT_SIGN)
		rs = inst->dword.mult_ins.op1_rs;
	else if (cat == ARM_CAT_MULT_LN)
		rs = inst->dword.mul_ln_ins.op1_rs;
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		rs = inst->dword.mul_ln_ins.op1_rs;
	else if (cat == ARM_CAT_HFWRD_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDTR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDSWP)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_CPR_RTR)
		fatal("%d: fmt not recognized", cat);
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	str_printf(inst_str_ptr, inst_str_size, "r%d", rs);
}

void arm_inst_dump_OP2(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int op2;
	unsigned int rm;
	unsigned int rs;
	unsigned int shift;
	unsigned int imm;
	unsigned int rotate;
	unsigned int imm_8r;

	if (cat == ARM_CAT_DPR_REG)
		op2 = inst->dword.dpr_ins.op2;
	else if (cat == ARM_CAT_DPR_IMM)
		op2 = inst->dword.dpr_ins.op2;
	else if (cat == ARM_CAT_DPR_SAT)
		op2 = inst->dword.dpr_sat_ins.op2;
	else if (cat == ARM_CAT_PSR)
		op2 = inst->dword.psr_ins.op2;
	else if (cat == ARM_CAT_MULT)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_SIGN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_LN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDTR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDSWP)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_CPR_RTR)
		fatal("%d: fmt not recognized", cat);
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	if (!inst->dword.dpr_ins.imm)
	{
		rm = (op2 & (0x0000000f));
		shift = ((op2 >> 4) & (0x000000ff));

		if (shift & 0x00000001)
		{
			switch ((shift >> 1) & 0x00000003)
			{
				case (LSL):
				str_printf(inst_str_ptr, inst_str_size, "r%d , LSL #<%d>", rm, ((shift >> 4) & 0x0000000f));
				break;

				case (LSR):
				str_printf(inst_str_ptr, inst_str_size, "r%d , LSR #<%d>", rm, ((shift >> 4) & 0x0000000f));
				break;

				case (ASR):
				str_printf(inst_str_ptr, inst_str_size, "r%d , ASR #<%d>", rm, ((shift >> 4) & 0x0000000f));
				break;

				case (ROR):
				str_printf(inst_str_ptr, inst_str_size, "r%d , ROR #<%d>", rm, ((shift >> 4) & 0x0000000f));
				break;
			}

		}

		else
		{
			rs = (shift >> 4);
			switch ((shift >> 1) & 0x00000003)
			{
				case (LSL):
				str_printf(inst_str_ptr, inst_str_size, "r%d , LSL r%d", rm, rs);
				break;

				case (LSR):
				str_printf(inst_str_ptr, inst_str_size, "r%d , LSR r%d", rm, rs);
				break;

				case (ASR):
				str_printf(inst_str_ptr, inst_str_size, "r%d , ASR r%d", rm, rs);
				break;

				case (ROR):
				str_printf(inst_str_ptr, inst_str_size, "r%d , ROR r%d", rm, rs);
				break;
			}
		}
	}
	else
	{
		imm = (op2 & (0x000000ff));
		rotate = ((op2 >> 8) & 0x0000000f);
		imm_8r = arm_rotr( imm , rotate);
		str_printf(inst_str_ptr, inst_str_size, "#<%d>", imm_8r);
	}

}

void arm_inst_dump_COND(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int cond;

	if (cat == ARM_CAT_DPR_REG)
		cond = inst->dword.dpr_ins.cond;
	else if (cat == ARM_CAT_DPR_IMM)
		cond = inst->dword.dpr_ins.cond;
	else if (cat == ARM_CAT_DPR_SAT)
		cond = inst->dword.dpr_sat_ins.cond;
	else if (cat == ARM_CAT_PSR)
		cond = inst->dword.psr_ins.cond;
	else if (cat == ARM_CAT_MULT)
		cond = inst->dword.mult_ins.cond;
	else if (cat == ARM_CAT_MULT_SIGN)
		cond = inst->dword.mult_ins.cond;
	else if (cat == ARM_CAT_MULT_LN)
		cond = inst->dword.mul_ln_ins.cond;
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		cond = inst->dword.mul_ln_ins.cond;
	else if (cat == ARM_CAT_HFWRD_REG)
		cond = inst->dword.hfwrd_reg_ins.cond;
	else if (cat == ARM_CAT_HFWRD_IMM)
		cond = inst->dword.hfwrd_imm_ins.cond;
	else if (cat == ARM_CAT_SDTR)
		cond = inst->dword.sdtr_ins.cond;
	else if (cat == ARM_CAT_SDSWP)
		cond = inst->dword.sngl_dswp_ins.cond;
	else if (cat == ARM_CAT_CPR_RTR)
		cond = inst->dword.cpr_rtr_ins.cond;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	switch (cond)
	{
		case (EQ):
		str_printf(inst_str_ptr, inst_str_size, "EQ");
		break;

		case (NE):
		str_printf(inst_str_ptr, inst_str_size, "NE");
		break;

		case (CS):
		str_printf(inst_str_ptr, inst_str_size, "CS");
		break;

		case (CC):
		str_printf(inst_str_ptr, inst_str_size, "CC");
		break;

		case (MI):
		str_printf(inst_str_ptr, inst_str_size, "MI");
		break;

		case (PL):
		str_printf(inst_str_ptr, inst_str_size, "PL");
		break;

		case (VS):
		str_printf(inst_str_ptr, inst_str_size, "VS");
		break;

		case (VC):
		str_printf(inst_str_ptr, inst_str_size, "VC");
		break;

		case (HI):
		str_printf(inst_str_ptr, inst_str_size, "HI");
		break;

		case (LS):
		str_printf(inst_str_ptr, inst_str_size, "LS");
		break;

		case (GE):
		str_printf(inst_str_ptr, inst_str_size, "GE");
		break;

		case (LT):
		str_printf(inst_str_ptr, inst_str_size, "LT");
		break;

		case (GT):
		str_printf(inst_str_ptr, inst_str_size, "GT");
		break;

		case (LE):
		str_printf(inst_str_ptr, inst_str_size, "LE");
		break;

		case (AL):
		str_printf(inst_str_ptr, inst_str_size, " ");
		break;

	}
}

void arm_inst_dump_RDLO(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int rdlo;

	if (cat == ARM_CAT_DPR_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_SAT)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_PSR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_SIGN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_LN)
		rdlo = inst->dword.mul_ln_ins.dst_lo;
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		rdlo = inst->dword.mul_ln_ins.dst_lo;
	else if (cat == ARM_CAT_HFWRD_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDTR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDSWP)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_CPR_RTR)
		fatal("%d: fmt not recognized", cat);
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	str_printf(inst_str_ptr, inst_str_size, "r%d", rdlo);

}

void arm_inst_dump_RDHI(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat)
{
	unsigned int rdhi;

	if (cat == ARM_CAT_DPR_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_DPR_SAT)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_PSR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_SIGN)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_MULT_LN)
		rdhi = inst->dword.mul_ln_ins.dst_hi;
	else if (cat == ARM_CAT_MULT_LN_SIGN)
		rdhi = inst->dword.mul_ln_ins.dst_hi;
	else if (cat == ARM_CAT_HFWRD_REG)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_HFWRD_IMM)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDTR)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_SDSWP)
		fatal("%d: fmt not recognized", cat);
	else if (cat == ARM_CAT_CPR_RTR)
		fatal("%d: fmt not recognized", cat);
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: fmt not recognized", cat);

	str_printf(inst_str_ptr, inst_str_size, "r%d", rdhi);

}
