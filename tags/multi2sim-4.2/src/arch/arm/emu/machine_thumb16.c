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
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "context.h"
#include "isa.h"
#include "regs.h"


char *arm_th16_err_isa_note =
	"\tThe ARM instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define __ARM_TH16_NOT_IMPL__ \
	fatal("%s: Arm 16 instruction '%s' not implemented\n%s", \
		__FUNCTION__, ctx->inst_th_16.info->name, arm_th16_err_isa_note);




void arm_th16_isa_LSL_imm_impl(struct arm_ctx_t *ctx)
{
	int rm_val;
	unsigned int immd;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	immd = ctx->inst_th_16.dword.movshift_reg_ins.offset;
	prev_regs.cpsr = regs->cpsr;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.movshift_reg_ins.reg_rs, &rm_val);

	rm_val = rm_val << immd;

	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.movshift_reg_ins.reg_rd, rm_val);

	regs->cpsr.z = 0;
	regs->cpsr.n = 0;
	regs->cpsr.C = 0;
	regs->cpsr.v = 0;

	if(rm_val == 0)
	{
		regs->cpsr.z = 1;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}
	if(rm_val < 0)
	{
		regs->cpsr.n = 1;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}

	regs->cpsr.C = prev_regs.cpsr.C;
}

void arm_th16_isa_LSR_imm_impl(struct arm_ctx_t *ctx)
{
	int rm_val;
	unsigned int immd;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	immd = ctx->inst_th_16.dword.movshift_reg_ins.offset;
	prev_regs.cpsr = regs->cpsr;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.movshift_reg_ins.reg_rs, &rm_val);

	rm_val = rm_val >> immd;

	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.movshift_reg_ins.reg_rd, rm_val);

	regs->cpsr.z = 0;
	regs->cpsr.n = 0;
	regs->cpsr.C = 0;
	regs->cpsr.v = 0;

	if(rm_val == 0)
	{
		regs->cpsr.z = 1;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}
	if(rm_val < 0)
	{
		regs->cpsr.n = 1;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}

	regs->cpsr.C = prev_regs.cpsr.C;

}

void arm_th16_isa_ASR_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MOV_imm_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs;
	int operand2;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;
		operand2 = ctx->inst_th_16.dword.immd_oprs_ins.offset8;
		arm_isa_reg_store(ctx, ctx->inst_th_16.dword.immd_oprs_ins.reg_rd, operand2);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if(operand2 == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(operand2 < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}

		regs->cpsr.C = prev_regs.cpsr.C;

	arm_isa_cpsr_print(ctx);
}

void arm_th16_isa_CMP_imm_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs;
	int rn_val;
	int operand2;
	int result;
	int op2;
	int rd_val;
	unsigned long flags;


	flags = 0;
	regs = ctx->regs;
	operand2 = ctx->inst_th_16.dword.immd_oprs_ins.offset8;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.immd_oprs_ins.reg_rd, &rn_val);
	result = rn_val - operand2;
	arm_isa_inst_debug("  result = %d ; 0x%x\n", result, result);

	op2 = (-1 * operand2);

	regs->cpsr.z = 0;
	regs->cpsr.n = 0;
	regs->cpsr.C = 0;
	regs->cpsr.v = 0;

	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"add %%eax, %3\n\t"
		"mov %3, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (op2)
		  : "m" (op2), "m" (rn_val), "m" (rd_val), "g" (flags)
		    : "eax"
	);

	arm_isa_inst_debug("  flags = 0x%lx\n", flags);
	if(flags & 0x00000001)
	{
		regs->cpsr.C = 1;
	}
	if(flags & 0x00008000)
	{
		regs->cpsr.v = 1;
	}
	if(flags & 0x00000040)
	{
		regs->cpsr.z = 1;
	}
	if(flags & 0x00000080)
	{
		regs->cpsr.n = 1;
	}

	if((operand2 == 0) && (rn_val == 0))
	{
		regs->cpsr.C = 1;
	}

	if(operand2 == 0)
	{
		regs->cpsr.C = 1;
	}


	arm_isa_cpsr_print(ctx);

}

void arm_th16_isa_ADD_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd;

	immd = ctx->inst_th_16.dword.immd_oprs_ins.offset8;
	arm_thumb_add_isa(ctx, ctx->inst_th_16.dword.immd_oprs_ins.reg_rd,
		ctx->inst_th_16.dword.immd_oprs_ins.reg_rd, immd, 0, 1);

}

void arm_th16_isa_SUB_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd;

	immd = ctx->inst_th_16.dword.immd_oprs_ins.offset8;
	arm_thumb_isa_subtract(ctx, ctx->inst_th_16.dword.immd_oprs_ins.reg_rd,
		ctx->inst_th_16.dword.immd_oprs_ins.reg_rd, immd, 0, 1);
}

void arm_th16_isa_ADD_reg_impl(struct arm_ctx_t *ctx)
{
	int rn_val;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.addsub_ins.rn_imm, &rn_val);
	arm_thumb_add_isa(ctx, ctx->inst_th_16.dword.addsub_ins.reg_rd,
		ctx->inst_th_16.dword.addsub_ins.reg_rs, rn_val, 0, 1);

}

void arm_th16_isa_SUB_reg_impl(struct arm_ctx_t *ctx)
{
	int rn_val;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.addsub_ins.rn_imm, &rn_val);
	arm_thumb_isa_subtract(ctx, ctx->inst_th_16.dword.addsub_ins.reg_rd,
		ctx->inst_th_16.dword.addsub_ins.reg_rs, rn_val, 0, 1);
}

void arm_th16_isa_ADD_immd3_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd;


	immd = ctx->inst_th_16.dword.addsub_ins.rn_imm;

	arm_thumb_add_isa(ctx, ctx->inst_th_16.dword.addsub_ins.reg_rd,
		ctx->inst_th_16.dword.addsub_ins.reg_rs, immd, 0, 1);
}

void arm_th16_isa_SUB_immd3_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd;

	immd = ctx->inst_th_16.dword.addsub_ins.rn_imm;

	arm_thumb_isa_subtract(ctx, ctx->inst_th_16.dword.addsub_ins.reg_rd,
		ctx->inst_th_16.dword.addsub_ins.reg_rs, immd, 0, 1);
}

void arm_th16_isa_AND_reg_impl(struct arm_ctx_t *ctx)
{
	int rn_val;
	int rm_val;
	unsigned int result;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rs, &rm_val);
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rd, &rn_val);

	result = rm_val & rn_val;
	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rd, result);

	regs->cpsr.z = 0;
	regs->cpsr.n = 0;
	regs->cpsr.C = 0;
	regs->cpsr.v = 0;

	if(result == 0)
	{
		regs->cpsr.z = 1;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}
	if(result < 0)
	{
		regs->cpsr.n = 1;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}

	regs->cpsr.C = prev_regs.cpsr.C;

	arm_isa_cpsr_print(ctx);
}

void arm_th16_isa_EOR_reg_impl(struct arm_ctx_t *ctx)
{
	int rn_val;
	int rm_val;
	unsigned int result;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rs, &rm_val);
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rd, &rn_val);

	result = rm_val ^ rn_val;
	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rd, result);

	regs->cpsr.z = 0;
	regs->cpsr.n = 0;
	regs->cpsr.C = 0;
	regs->cpsr.v = 0;

	if(result == 0)
	{
		regs->cpsr.z = 1;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}
	if(result < 0)
	{
		regs->cpsr.n = 1;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}

	regs->cpsr.C = prev_regs.cpsr.C;

	arm_isa_cpsr_print(ctx);
}

void arm_th16_isa_LSL_reg_impl(struct arm_ctx_t *ctx)
{
	int rn_val;
	int rm_val;
	unsigned int result;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;
	int shift;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rs, &rm_val);
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rd, &rn_val);

	shift = rm_val & 0x000000ff;
	result = rn_val << shift;

	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rd, result);

	regs->cpsr.z = 0;
	regs->cpsr.n = 0;
	regs->cpsr.C = 0;
	regs->cpsr.v = 0;

	if(result == 0)
	{
		regs->cpsr.z = 1;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}
	if(result < 0)
	{
		regs->cpsr.n = 1;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;
	}

	regs->cpsr.C = prev_regs.cpsr.C;

	arm_isa_cpsr_print(ctx);
}

void arm_th16_isa_LSR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ASR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADC_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SBC_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ROR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_TST_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_RSB_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMP_reg1_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs;
	int rn_val;
	int result;
	int op2;
	int rm_val;
	int rd_val;
	unsigned long flags;


	flags = 0;
	regs = ctx->regs;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rd, &rn_val);
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.dpr_ins.reg_rs, &rm_val);

	result = rn_val - rm_val;
	arm_isa_inst_debug("  result = %d ; 0x%x\n", result, result);

	op2 = (-1 * rm_val);

	regs->cpsr.z = 0;
	regs->cpsr.n = 0;
	regs->cpsr.C = 0;
	regs->cpsr.v = 0;

	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"add %%eax, %3\n\t"
		"mov %3, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (op2)
		  : "m" (op2), "m" (rn_val), "m" (rd_val), "g" (flags)
		    : "eax"
	);

	arm_isa_inst_debug("  flags = 0x%lx\n", flags);
	if(flags & 0x00000001)
	{
		regs->cpsr.C = 1;
	}
	if(flags & 0x00008000)
	{
		regs->cpsr.v = 1;
	}
	if(flags & 0x00000040)
	{
		regs->cpsr.z = 1;
	}
	if(flags & 0x00000080)
	{
		regs->cpsr.n = 1;
	}

	if((rm_val == 0) && (rn_val == 0))
	{
		regs->cpsr.C = 1;
	}

	if(rm_val == 0)
	{
		regs->cpsr.C = 1;
	}


	arm_isa_cpsr_print(ctx);

}

void arm_th16_isa_CMN_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ORR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MUL_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_BIC_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MVN_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_reg_lo_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_reg_hi1_impl(struct arm_ctx_t *ctx)
{
	int rs_val;
	int rd_val;
	int rn;
	int result;

	rn = (ctx->inst_th_16.dword.high_oprs_ins.h1 << 3) | (ctx->inst_th_16.dword.high_oprs_ins.reg_rd);

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.high_oprs_ins.reg_rs, &rs_val);
	arm_isa_reg_load(ctx, rn, &rd_val);

	result = rd_val + rs_val;
	arm_isa_reg_store(ctx, rn, result);
}

void arm_th16_isa_ADD_reg_hi2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMP_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMP_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MOV_reg1_impl(struct arm_ctx_t *ctx)
{
	int rs_val;
	int rd;

	rd = (ctx->inst_th_16.dword.high_oprs_ins.h1 << 3) | (ctx->inst_th_16.dword.high_oprs_ins.reg_rd);

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.high_oprs_ins.reg_rs, &rs_val);
	if(rd < 15)
	{
		arm_isa_reg_store(ctx, rd, rs_val);
	}
}

void arm_th16_isa_MOV_reg2_impl(struct arm_ctx_t *ctx)
{
	int rs_val;
	int rd;

	rd = (ctx->inst_th_16.dword.high_oprs_ins.h1 << 3) | (ctx->inst_th_16.dword.high_oprs_ins.reg_rd);

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.high_oprs_ins.reg_rs, &rs_val);
	if(rd < 15)
	{
		arm_isa_reg_store(ctx, rd, rs_val);
	}
}

void arm_th16_isa_BX_impl(struct arm_ctx_t *ctx)
{
	int rs_val;
	int addr;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.high_oprs_ins.reg_rs, &rs_val);

	if(rs_val % 2)
		addr = rs_val - 1;
	else
		addr = rs_val;

	ctx->regs->pc = addr + 2;
}

void arm_th16_isa_BLX_impl(struct arm_ctx_t *ctx)
{
	int rs_val;
	int addr;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.high_oprs_ins.reg_rs, &rs_val);

	if(rs_val % 2)
		addr = rs_val - 1;
	else
		addr = rs_val;

	ctx->regs->lr = ctx->regs->pc;
	ctx->regs->pc = addr + 2;
}

void arm_th16_isa_LDR_lit1_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd32;
	unsigned int offset;
	unsigned int addr;
	int value;
	void *buf;

	buf = &value;
	immd32 = ctx->inst_th_16.dword.pcldr_ins.immd_8 << 2;

	arm_isa_inst_debug("  pc  = 0x%x; \n", ctx->regs->pc);
	if((ctx->regs->pc - 2) % 4 == 2)
		offset = (ctx->regs->pc - 2) + 2;
	else
		offset = ctx->regs->pc - 2;
	arm_isa_inst_debug("  offset  = 0x%x; \n", offset);

	addr = offset + (immd32);
	mem_read(ctx->mem, addr, 4, buf);
	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.pcldr_ins.reg_rd, value);
}

void arm_th16_isa_LDR_lit2_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd32;
	unsigned int offset;
	unsigned int addr;
	int value;
	void *buf;

	buf = &value;
	immd32 = ctx->inst_th_16.dword.pcldr_ins.immd_8 << 2;

	arm_isa_inst_debug("  pc  = 0x%x; \n", ctx->regs->pc);
	if((ctx->regs->pc - 2) % 4 == 2)
		offset = (ctx->regs->pc - 2) + 2;
	else
		offset = ctx->regs->pc - 2;
	arm_isa_inst_debug("  offset  = 0x%x; \n", offset);

	addr = offset + (immd32);
	mem_read(ctx->mem, addr, 4, buf);
	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.pcldr_ins.reg_rd, value);

}

void arm_th16_isa_STR_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSB_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSH_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSB_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSH_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_imm4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSB_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSH_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_immd5_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;
	int rd_val;
	int addr;
	void *buf;

	buf = &rd_val;
	immd5 = ctx->inst_th_16.dword.ldstr_immd_ins.offset << 2;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.ldstr_immd_ins.reg_rb, &rn_val);

	addr = rn_val + immd5;

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.ldstr_immd_ins.reg_rd,
			&rd_val);

	arm_isa_inst_debug(" r%d (0x%x) => [0x%x]\n",
		ctx->inst_th_16.dword.ldstr_immd_ins.reg_rd, rd_val, addr);
	mem_write(ctx->mem, addr, 4, buf);
}

void arm_th16_isa_LDR_immd5_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;
	int rd_val;
	int addr;
	void *buf;

	buf = &rd_val;
	immd5 = ctx->inst_th_16.dword.ldstr_immd_ins.offset << 2;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.ldstr_immd_ins.reg_rb, &rn_val);

	addr = rn_val + immd5;

	mem_read(ctx->mem, addr, 4, buf);
	arm_isa_inst_debug(" r%d (0x%x) <= [0x%x]\n",
		ctx->inst_th_16.dword.ldstr_immd_ins.reg_rd, rd_val, addr);

	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.ldstr_immd_ins.reg_rd, rd_val);
}

void arm_th16_isa_STRB_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_immd3_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;
	int rd_val;
	int addr;
	void *buf;

	buf = &rd_val;
	immd5 = ctx->inst_th_16.dword.ldstr_immd_ins.offset << 2;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.ldstr_immd_ins.reg_rb, &rn_val);

	addr = rn_val + immd5;

	mem_read(ctx->mem, addr, 1, buf);
	rd_val = rd_val & 0x000000ff;
	arm_isa_inst_debug(" r%d (0x%x) <= [0x%x]\n",
		ctx->inst_th_16.dword.ldstr_immd_ins.reg_rd, rd_val, addr);

	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.ldstr_immd_ins.reg_rd, rd_val);

}

void arm_th16_isa_STRH_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm6_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd8;
	unsigned int addr;
	int sp_val, rn_val;
	void *buf;

	buf = &rn_val;
	immd8 = 4 * ctx->inst_th_16.dword.sp_immd_ins.immd_8;

	arm_isa_reg_load(ctx, 13, &sp_val);
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.sp_immd_ins.reg_rd,
			&rn_val);

	addr = sp_val + immd8;

	arm_isa_inst_debug(" r%d (0x%x) => [0x%x]\n",
		ctx->inst_th_16.dword.sp_immd_ins.reg_rd, rn_val, addr);
	mem_write(ctx->mem, addr, 4, buf);
}

void arm_th16_isa_LDR_imm6_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd8;
	unsigned int addr;
	int sp_val, rn_val;
	void *buf;

	buf = &rn_val;
	immd8 = 4 * ctx->inst_th_16.dword.sp_immd_ins.immd_8;

	arm_isa_reg_load(ctx, 13, &sp_val);
	addr = sp_val + immd8;

	arm_isa_inst_debug(" r%d (0x%x) <= [0x%x]\n",
		ctx->inst_th_16.dword.sp_immd_ins.reg_rd, rn_val, addr);

	mem_read(ctx->mem, addr, 4, buf);

	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.sp_immd_ins.reg_rd,
			rn_val);
}

void arm_th16_isa_ADR_PC_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP_impl(struct arm_ctx_t *ctx)
{
	int sp_val;
	int immd;
	int result;

	arm_isa_reg_load(ctx, 13, &sp_val);
	immd = ctx->inst_th_16.dword.addsp_ins.immd_8 * 4;

	result = sp_val + immd;

	if(ctx->inst_th_16.dword.addsp_ins.reg_rd < 15)
	{
		arm_isa_reg_store(ctx, ctx->inst_th_16.dword.addsp_ins.reg_rd, result);
	}
	else
	{
		if(result % 2)
		{
			arm_isa_reg_store(ctx, ctx->inst_th_16.dword.addsp_ins.reg_rd, result - 3);
		}
		else
		{
			arm_isa_reg_store(ctx, ctx->inst_th_16.dword.addsp_ins.reg_rd, result - 2);
		}
	}

}

void arm_th16_isa_STM_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDM_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SVC_impl(struct arm_ctx_t *ctx)
{
	arm_isa_syscall(ctx);
}

void arm_th16_isa_B_0_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;
	int cond;

	cond = ctx->inst_th_16.dword.cond_br_ins.cond;
	immd = ctx->inst_th_16.dword.cond_br_ins.s_offset << 1;
	immd = SEXT32(immd, 9);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);
	addr = ctx->regs->pc + immd;

	if(arm_isa_thumb_check_cond(ctx, cond))
	{
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
		ctx->regs->pc = addr + 2;
	}
}

void arm_th16_isa_B_1_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;
	int cond;

	cond = ctx->inst_th_16.dword.cond_br_ins.cond;
	immd = ctx->inst_th_16.dword.cond_br_ins.s_offset << 1;
	immd = SEXT32(immd, 9);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);
	addr = ctx->regs->pc + immd;

	if(arm_isa_thumb_check_cond(ctx, cond))
	{
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
		ctx->regs->pc = addr + 2;
	}
}

void arm_th16_isa_B_2_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;
	int cond;

	cond = ctx->inst_th_16.dword.cond_br_ins.cond;
	immd = ctx->inst_th_16.dword.cond_br_ins.s_offset << 1;
	immd = SEXT32(immd, 9);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);
	addr = ctx->regs->pc + immd;

	if(arm_isa_thumb_check_cond(ctx, cond))
	{
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
		ctx->regs->pc = addr + 2;
	}

}

void arm_th16_isa_B_3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_7_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_8_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;
	int cond;

	cond = ctx->inst_th_16.dword.cond_br_ins.cond;
	immd = ctx->inst_th_16.dword.cond_br_ins.s_offset << 1;
	immd = SEXT32(immd, 9);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);
	addr = ctx->regs->pc + immd;

	if(arm_isa_thumb_check_cond(ctx, cond))
	{
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
		ctx->regs->pc = addr + 2;
	}

}

void arm_th16_isa_B_9_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;
	int cond;

	cond = ctx->inst_th_16.dword.cond_br_ins.cond;
	immd = ctx->inst_th_16.dword.cond_br_ins.s_offset << 1;
	immd = SEXT32(immd, 9);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);
	addr = ctx->regs->pc + immd;

	if(arm_isa_thumb_check_cond(ctx, cond))
	{
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
		ctx->regs->pc = addr + 2;
	}

}

void arm_th16_isa_B_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_11_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;
	int cond;

	cond = ctx->inst_th_16.dword.cond_br_ins.cond;
	immd = ctx->inst_th_16.dword.cond_br_ins.s_offset << 1;
	immd = SEXT32(immd, 9);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);
	addr = ctx->regs->pc + immd;

	if(arm_isa_thumb_check_cond(ctx, cond))
	{
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
		ctx->regs->pc = addr + 2;
	}

}

void arm_th16_isa_B_12_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_13_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;
	int cond;

	cond = ctx->inst_th_16.dword.cond_br_ins.cond;
	immd = ctx->inst_th_16.dword.cond_br_ins.s_offset << 1;
	immd = SEXT32(immd, 9);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);
	addr = ctx->regs->pc + immd;

	if(arm_isa_thumb_check_cond(ctx, cond))
	{
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
		ctx->regs->pc = addr + 2;
	}

}

void arm_th16_isa_B_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_15_impl(struct arm_ctx_t *ctx)
{
	int immd;
	int addr;


	immd = ctx->inst_th_16.dword.br_ins.immd11 << 1;
	immd = SEXT32(immd, 12);
	arm_isa_inst_debug("  Offset = %x (%d)\n", immd, immd);

	addr = ctx->regs->pc + immd;

	arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %x\n", addr, ctx->regs->pc);
	ctx->regs->pc = addr + 2;

}

void arm_th16_isa_ADD_SP1_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd7;
	int sp_val;

	arm_isa_reg_load(ctx, 13, &sp_val);
	immd7 = 4 * ctx->inst_th_16.dword.sub_sp_ins.immd_8;

	sp_val = sp_val + immd7;

	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_ADD_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP4_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd7;
	int sp_val;

	arm_isa_reg_load(ctx, 13, &sp_val);
	immd7 = 4 * ctx->inst_th_16.dword.sub_sp_ins.immd_8;

	sp_val = sp_val + immd7;

	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_SUB_SP1_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd7;
	int sp_val;

	arm_isa_reg_load(ctx, 13, &sp_val);
	immd7 = 4 * ctx->inst_th_16.dword.sub_sp_ins.immd_8;

	sp_val = sp_val - immd7;

	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_SUB_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_SP3_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd7;
	int sp_val;

	arm_isa_reg_load(ctx, 13, &sp_val);
	immd7 = 4 * ctx->inst_th_16.dword.sub_sp_ins.immd_8;

	sp_val = sp_val - immd7;

	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_SUB_SP4_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd7;
	int sp_val;

	arm_isa_reg_load(ctx, 13, &sp_val);
	immd7 = 4 * ctx->inst_th_16.dword.sub_sp_ins.immd_8;

	sp_val = sp_val - immd7;

	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_SXTH_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SXTH_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SXTB_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SXTB_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_UXTH_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_UXTH_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_UXTB_SP1_impl(struct arm_ctx_t *ctx)
{
	int rm_val;
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.ldstr_exts_ins.reg_rb, &rm_val);
	rm_val = rm_val & 0x000000ff;

	arm_isa_reg_store(ctx, ctx->inst_th_16.dword.ldstr_exts_ins.reg_rd, rm_val);
}

void arm_th16_isa_UXTB_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_0_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val - 4*(arm_isa_bit_count(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			arm_isa_reg_load(ctx,log_base2(i) , &reg_val);
			mem_write(ctx->mem, wrt_val, 4, buf);
			arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_PUSH_1_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val - 4*(arm_isa_bit_count(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			arm_isa_reg_load(ctx,log_base2(i) , &reg_val);
			mem_write(ctx->mem, wrt_val, 4, buf);
			arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);

}

void arm_th16_isa_PUSH_2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_7_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_8_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val - 4*(arm_isa_bit_count(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			arm_isa_reg_load(ctx,log_base2(i) , &reg_val);
			mem_write(ctx->mem, wrt_val, 4, buf);
			arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_PUSH_9_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_11_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val - 4*(arm_isa_bit_count(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			arm_isa_reg_load(ctx,log_base2(i) , &reg_val);
			mem_write(ctx->mem, wrt_val, 4, buf);
			arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);

}

void arm_th16_isa_PUSH_12_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val - 4*(arm_isa_bit_count(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			arm_isa_reg_load(ctx,log_base2(i) , &reg_val);
			mem_write(ctx->mem, wrt_val, 4, buf);
			arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_PUSH_13_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_15_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val - 4*(arm_isa_bit_count(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			arm_isa_reg_load(ctx,log_base2(i) , &reg_val);
			mem_write(ctx->mem, wrt_val, 4, buf);
			arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_POP_0_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			if(log_base2(i) < 15)
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				arm_isa_reg_store(ctx, log_base2(i), reg_val);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				if(reg_val % 2)
					reg_val = reg_val - 1;

				arm_isa_reg_store(ctx, log_base2(i), reg_val - 2);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;

			}

		}
	}

	sp_val = sp_val + 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);

}

void arm_th16_isa_POP_1_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			if(log_base2(i) < 15)
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				arm_isa_reg_store(ctx, log_base2(i), reg_val);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				if(reg_val % 2)
					reg_val = reg_val - 1;

				arm_isa_reg_store(ctx, log_base2(i), reg_val - 2);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;

			}

		}
	}

	sp_val = sp_val + 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);

}

void arm_th16_isa_POP_2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_7_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_8_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_9_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_11_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			if(log_base2(i) < 15)
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				arm_isa_reg_store(ctx, log_base2(i), reg_val);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				if(reg_val % 2)
					reg_val = reg_val - 1;

				arm_isa_reg_store(ctx, log_base2(i), reg_val - 2);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;

			}

		}
	}

	sp_val = sp_val + 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_POP_12_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			if(log_base2(i) < 15)
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				arm_isa_reg_store(ctx, log_base2(i), reg_val);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				if(reg_val % 2)
					reg_val = reg_val - 1;

				arm_isa_reg_store(ctx, log_base2(i), reg_val - 2);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;

			}

		}
	}

	sp_val = sp_val + 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);

}

void arm_th16_isa_POP_13_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_15_impl(struct arm_ctx_t *ctx)
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = ctx->inst_th_16.dword.push_pop_ins.reg_list;
	regs = (ctx->inst_th_16.dword.push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	arm_isa_reg_load(ctx, 13, &sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			if(log_base2(i) < 15)
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				arm_isa_reg_store(ctx, log_base2(i), reg_val);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				mem_read(ctx->mem, wrt_val, 4, buf);
				if(reg_val % 2)
					reg_val = reg_val - 1;

				arm_isa_reg_store(ctx, log_base2(i), reg_val - 2);
				arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),wrt_val);
				wrt_val += 4;

			}

		}
	}

	sp_val = sp_val + 4*(arm_isa_bit_count(regs));
	arm_isa_reg_store(ctx, 13, sp_val);
}

void arm_th16_isa_CBZ_8_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	arm_isa_inst_debug(" PC : 0x%x\n", ctx->regs->pc);
	immd5 = ctx->inst_th_16.dword.cbnz_ins.immd_5;
	inst_addr = ctx->inst_th_16.addr;
	arm_isa_inst_debug("  Inst addr <= 0x%x\n", inst_addr);
	if((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if((ctx->regs->pc - 2) % 4)
		immd5 = (ctx->regs->pc) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (ctx->regs->pc - 2) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	arm_isa_inst_debug("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, ctx->regs->pc);

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.cbnz_ins.reg_rn, &rn_val);
	if(rn_val == 0)
	{
//		if(immd5 % 4)
//			ctx->regs->pc = immd5 + 4;
//		else
			ctx->regs->pc = immd5 + 2;
		arm_isa_inst_debug("  After Branch pc <= 0x%x\n", ctx->regs->pc);
	}
}

void arm_th16_isa_CBZ_9_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	arm_isa_inst_debug(" PC : 0x%x\n", ctx->regs->pc);
	immd5 = ctx->inst_th_16.dword.cbnz_ins.immd_5;
	inst_addr = ctx->inst_th_16.addr;
	arm_isa_inst_debug("  Inst addr <= 0x%x\n", inst_addr);
	if((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if((ctx->regs->pc - 2) % 4)
		immd5 = (ctx->regs->pc) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (ctx->regs->pc - 2) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	arm_isa_inst_debug("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, ctx->regs->pc);

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.cbnz_ins.reg_rn, &rn_val);
	if(rn_val == 0)
	{
//		if(immd5 % 4)
//			ctx->regs->pc = immd5 + 4;
//		else
			ctx->regs->pc = immd5 + 2;
		arm_isa_inst_debug("  After Branch pc <= 0x%x\n", ctx->regs->pc);
	}

}

void arm_th16_isa_CBZ_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_11_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_12_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	arm_isa_inst_debug(" PC : 0x%x\n", ctx->regs->pc);
	immd5 = ctx->inst_th_16.dword.cbnz_ins.immd_5;
	inst_addr = ctx->inst_th_16.addr;
	arm_isa_inst_debug("  Inst addr <= 0x%x\n", inst_addr);
	if((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if((ctx->regs->pc - 2) % 4)
		immd5 = (ctx->regs->pc) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (ctx->regs->pc - 2) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	arm_isa_inst_debug("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, ctx->regs->pc);

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.cbnz_ins.reg_rn, &rn_val);
	if(rn_val == 0)
	{
//		if(immd5 % 4)
//			ctx->regs->pc = immd5 + 4;
//		else
			ctx->regs->pc = immd5 + 2;
		arm_isa_inst_debug("  After Branch pc <= 0x%x\n", ctx->regs->pc);
	}

}

void arm_th16_isa_CBZ_13_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	arm_isa_inst_debug(" PC : 0x%x\n", ctx->regs->pc);
	immd5 = ctx->inst_th_16.dword.cbnz_ins.immd_5;
	inst_addr = ctx->inst_th_16.addr;
	arm_isa_inst_debug("  Inst addr <= 0x%x\n", inst_addr);
	if((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if((ctx->regs->pc - 2) % 4)
		immd5 = (ctx->regs->pc) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (ctx->regs->pc - 2) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	arm_isa_inst_debug("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, ctx->regs->pc);

	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.cbnz_ins.reg_rn, &rn_val);
	if(rn_val == 0)
	{
//		if(immd5 % 4)
//			ctx->regs->pc = immd5 + 4;
//		else
			ctx->regs->pc = immd5 + 2;
		arm_isa_inst_debug("  After Branch pc <= 0x%x\n", ctx->regs->pc);
	}

}

void arm_th16_isa_CBZ_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_15_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_18_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_19_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_110_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_111_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_112_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_113_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_114_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_115_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_58_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_59_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_510_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_511_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_512_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_513_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_514_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_515_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_48_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_49_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_410_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_411_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_412_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd5;
	int rn_val;


	arm_isa_inst_debug(" PC : 0x%x\n", ctx->regs->pc);
	immd5 = ctx->inst_th_16.dword.cbnz_ins.immd_5;
	if((ctx->regs->pc - 2) % 4)
		immd5 = (ctx->regs->pc) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (ctx->regs->pc - 2) + ((ctx->inst_th_16.dword.cbnz_ins.i_ext << 6) | (immd5 << 1));

	arm_isa_inst_debug("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, ctx->regs->pc);
	arm_isa_reg_load(ctx, ctx->inst_th_16.dword.cbnz_ins.reg_rn, &rn_val);
	if(rn_val != 0)
	{
		ctx->regs->pc = immd5 + 2;
		arm_isa_inst_debug("  After Branch pc <= 0x%x\n", ctx->regs->pc);
	}
}

void arm_th16_isa_CBNZ_413_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_414_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_415_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_NOP_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_41_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_42_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_43_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_44_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_45_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_46_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_47_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_48_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_49_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_410_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_411_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_412_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_413_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_414_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_415_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_51_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_52_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_53_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_54_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_55_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_56_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_57_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_58_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_59_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_510_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_511_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_512_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_513_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_514_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_515_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_61_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_62_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_63_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_64_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_65_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_66_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_67_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_68_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_69_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_610_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_611_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_612_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_613_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_614_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_615_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_71_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_72_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_73_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_74_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_75_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_76_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_77_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_78_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_79_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_710_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_711_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_712_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_713_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_714_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_715_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_81_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_82_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_83_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_84_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_85_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_86_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_87_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_88_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_89_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_810_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_811_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_812_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_813_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_814_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_815_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_91_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_92_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_93_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_94_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_95_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_96_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_97_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_98_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_99_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_910_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_911_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_912_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_913_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_914_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_915_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_101_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_102_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_103_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_104_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_105_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_106_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_107_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_108_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_109_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1010_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1011_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1012_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1013_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1014_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1015_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_111_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_112_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_113_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_114_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_115_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_116_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_117_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_118_impl(struct arm_ctx_t *ctx)
{
 	arm_thumb_isa_iteq(ctx);
}

void arm_th16_isa_IT_119_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1110_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1111_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1112_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1113_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1114_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1115_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_REV_0_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_REV_1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}
