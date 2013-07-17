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
#include <mem-system/memory.h>

#include "context.h"
#include "isa.h"
#include "machine.h"
#include "regs.h"


char *arm_err_isa_note =
	"\tThe ARM instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define __ARM_NOT_IMPL__ \
	fatal("%s: Arm instruction '%s' not implemented\n%s", \
		__FUNCTION__, ctx->inst.info->name, arm_err_isa_note);


void arm_isa_AND_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;

	if (arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 & rn_val));
	}
}

void arm_isa_ANDS_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 & rn_val));

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg) == -1)
		{
			regs->cpsr.C = prev_regs.cpsr.C;
		}

		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_EOR_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 ^ rn_val));
	}
}

void arm_isa_EORS_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SUB_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_subtract(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_SUBS_reg_impl(struct arm_ctx_t *ctx)
{

	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_subtract(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_RSB_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_subtract_rev(ctx, ctx->inst.dword.dpr_ins.dst_reg,
			ctx->inst.dword.dpr_ins.op1_reg, operand2, 0);
	}
}

void arm_isa_RSBS_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_ADD_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_add(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_ADDS_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_add(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_ADC_reg_impl(struct arm_ctx_t *ctx)
{

	int operand2;
	unsigned int carry;

	carry = ctx->regs->cpsr.C;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_add(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, carry);
	}
}

void arm_isa_ADCS_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SBC_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	unsigned int carry;

	carry = ctx->regs->cpsr.C;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_subtract(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, ~(carry));
	}
}

void arm_isa_SBCS_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	unsigned int carry;

	carry = ctx->regs->cpsr.C;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_subtract(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, ~(carry));
	}
}

void arm_isa_RSC_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_RSCS_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_TSTS_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;
	struct arm_regs_t *regs;

	regs = ctx->regs;
	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_TEQS_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;
	struct arm_regs_t *regs;

	regs = ctx->regs;
	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if ((operand2 ^ rn_val) == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if((operand2 ^ rn_val) < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_CMPS_reg_impl(struct arm_ctx_t *ctx)
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
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
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
}

void arm_isa_CMNS_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_ORR_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 | rn_val));
	}
}

void arm_isa_ORRS_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 | rn_val));

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if ((operand2 | rn_val) == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if((operand2 | rn_val) < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg) == -1)
		{
			regs->cpsr.C = prev_regs.cpsr.C;
		}

		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_MOV_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, operand2);
	}
}

void arm_isa_MOVS_reg_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs;
	int operand2;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, operand2);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if (operand2 == 0)
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
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg) == -1)
		{
			regs->cpsr.C = prev_regs.cpsr.C;
		}
	}
	arm_isa_cpsr_print(ctx);
}

void arm_isa_BIC_reg_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rd_val;
	int rn_val;


	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);

		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);

		if(!(ctx->inst.dword.dpr_ins.s_cond))
		{
			rd_val = (rn_val) & (~(operand2)) ;
			arm_isa_inst_debug("  r%d = r%d & (~%d)\n", ctx->inst.dword.dpr_ins.dst_reg,
				ctx->inst.dword.dpr_ins.op1_reg, operand2);
			arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, rd_val);
		}
		else
		{

		}
	}
}

void arm_isa_BICS_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MVN_reg_impl(struct arm_ctx_t *ctx)
{
	unsigned int operand2;
	unsigned int result;

	if (arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		result = (0xffffffff) ^ (operand2);
		arm_isa_reg_store_safe(ctx, ctx->inst.dword.dpr_ins.dst_reg, result);
	}
}

void arm_isa_MVNS_reg_impl(struct arm_ctx_t *ctx)
{
	unsigned int operand2;
	unsigned int result;
	struct arm_regs_t *regs = ctx->regs;
	struct arm_regs_t prev_regs;

	prev_regs.cpsr = regs->cpsr;
	if (arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, reg);
		result = (0xffffffff) ^ (operand2);
		arm_isa_reg_store_safe(ctx, ctx->inst.dword.dpr_ins.dst_reg, result);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if (result == 0)
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
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, reg) == -1)
		{
			regs->cpsr.C = prev_regs.cpsr.C;
		}
	}
	arm_isa_cpsr_print(ctx);
}


void arm_isa_AND_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 & rn_val));
	}
}

void arm_isa_ANDS_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;

	regs = ctx->regs;
	prev_regs.cpsr = regs->cpsr;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 & rn_val));

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, immd))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, immd) == -1)
		{
			regs->cpsr.C = prev_regs.cpsr.C;
		}

		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_EOR_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 ^ rn_val));
	}
}

void arm_isa_EORS_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SUB_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_subtract(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_SUBS_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_subtract(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_RSB_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_subtract_rev(ctx, ctx->inst.dword.dpr_ins.dst_reg,
			ctx->inst.dword.dpr_ins.op1_reg, operand2, 0);
	}
}

void arm_isa_RSBS_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_subtract_rev(ctx, ctx->inst.dword.dpr_ins.dst_reg,
			ctx->inst.dword.dpr_ins.op1_reg, operand2, 0);
	}
}

void arm_isa_ADD_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_add(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_ADDS_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_add(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, 0);
	}
}

void arm_isa_ADC_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	unsigned int carry;

	carry = ctx->regs->cpsr.C;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_add(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, carry);
	}
}

void arm_isa_ADCS_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	unsigned int carry;

	carry = ctx->regs->cpsr.C;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_add(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, carry);
	}
}

void arm_isa_SBC_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SBCS_imm_impl(struct arm_ctx_t *ctx)
{

	int operand2;
	unsigned int carry;

	carry = ctx->regs->cpsr.C;
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_subtract(ctx, ctx->inst.dword.dpr_ins.dst_reg, ctx->inst.dword.dpr_ins.op1_reg,
			operand2, ~(carry));
	}
}

void arm_isa_RSC_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_RSCS_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_TSTS_imm_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;
	int operand2;
	int rn_val;

	regs = ctx->regs;

	prev_regs.cpsr = regs->cpsr;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, immd))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, immd) == -1)
		{
			regs->cpsr.C = prev_regs.cpsr.C;
		}

	}
	arm_isa_cpsr_print(ctx);
}

void arm_isa_TEQS_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;
	struct arm_regs_t *regs;

	regs = ctx->regs;
	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if ((operand2 ^ rn_val) == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if((operand2 ^ rn_val) < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(arm_isa_op2_carry(ctx, ctx->inst.dword.dpr_ins.op2, immd))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}
		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_CMPS_imm_impl(struct arm_ctx_t *ctx)
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
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
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

	}
	arm_isa_cpsr_print(ctx);
}

void arm_isa_CMNS_imm_impl(struct arm_ctx_t *ctx)
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
	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);

		result = rn_val + operand2;
		arm_isa_inst_debug("  result = %d ; 0x%x\n", result, result);

		op2 = operand2;
		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		asm volatile (
			"push %5\n\t"
			"popf\n\t"
			"mov %2, %%eax\n\t"
			"add %3, %%eax\n\t"
			"mov %%eax, %4\n\t"
			"pushf\n\t"
			"pop %0\n\t"
			: "=g" (flags), "=m" (rn_val)
			  : "m" (rn_val), "m" (op2), "m" (rd_val), "g" (flags)
			    : "eax"
		);


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
		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_ORR_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rn_val;

	if(arm_isa_check_cond(ctx))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, (operand2 | rn_val));
	}
}

void arm_isa_ORRS_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MOV_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned int operand2;

	if (arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, operand2);
	}

}

void arm_isa_MOVS_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_BIC_imm_impl(struct arm_ctx_t *ctx)
{
	int operand2;
	int rd_val;
	int rn_val;


	if(arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);

		arm_isa_reg_load(ctx, ctx->inst.dword.dpr_ins.op1_reg, &rn_val);

		if(!(ctx->inst.dword.dpr_ins.s_cond))
		{
			rd_val = (rn_val) & (~(operand2)) ;
			arm_isa_inst_debug("  r%d = r%d & (~%d)\n", ctx->inst.dword.dpr_ins.dst_reg,
				ctx->inst.dword.dpr_ins.op1_reg, operand2);
			arm_isa_reg_store(ctx, ctx->inst.dword.dpr_ins.dst_reg, rd_val);
		}
		else
		{

		}
	}
}

void arm_isa_BICS_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MVN_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned int operand2;
	unsigned int result;

	if (arm_isa_check_cond(ctx))
	{
		operand2 = arm_isa_op2_get(ctx, ctx->inst.dword.dpr_ins.op2, immd);
		result = (0xffffffff) ^ (operand2);
		arm_isa_reg_store_safe(ctx, ctx->inst.dword.dpr_ins.dst_reg, result);
	}
}

void arm_isa_MVNS_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MOVT_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MOVW_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_QADD_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_QSUB_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_QDADD_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_QDSUB_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MRS_reg_impl(struct arm_ctx_t *ctx)
{
	unsigned int rd_val;
	if (arm_isa_check_cond(ctx))
	{
		if(!(ctx->inst.dword.psr_ins.psr_loc))
		{
			rd_val = arm_isa_ret_cpsr_val(ctx);
		}
	}

	arm_isa_reg_store(ctx, ctx->inst.dword.psr_ins.dst_reg, rd_val);
}

void arm_isa_MSR_reg_impl(struct arm_ctx_t *ctx)
{
	if (arm_isa_check_cond(ctx))
	{
		if(!(ctx->inst.dword.psr_ins.psr_loc))
		{
			arm_isa_set_cpsr_val(ctx,ctx->inst.dword.psr_ins.op2);
		}
	}

}

void arm_isa_MSR_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MUL_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
	{
		arm_isa_multiply(ctx);
	}
}

void arm_isa_MULS_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_MLA_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
	{
		arm_isa_multiply(ctx);
	}
}

void arm_isa_MLAS_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMLA_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMLAW_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMULW_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMUL_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_UMULL_impl(struct arm_ctx_t *ctx)
{
	unsigned long long result;

	int rm_val;
	int rs_val;
	unsigned int rdhi;
	unsigned int rdlo;
	unsigned int rs;
	unsigned int rm;

	arm_isa_reg_load(ctx, ctx->inst.dword.mul_ln_ins.op1_rs, &rs_val);
	arm_isa_reg_load(ctx, ctx->inst.dword.mult_ins.op0_rm, &rm_val);

	arm_isa_inst_debug("  rm_val: 0x%x, rs_val: 0x%x\n", rm_val,rs_val);

	rs =(unsigned int)rs_val;
	rm =(unsigned int)rm_val;

	result = (unsigned long long)rm*rs;
	arm_isa_inst_debug("  result = 0x%llx\n", result);

	rdhi = (0xffffffff00000000 & result) >> 32;
	rdlo = 0x00000000ffffffff & result;

	arm_isa_reg_store(ctx, ctx->inst.dword.mul_ln_ins.dst_hi, rdhi);
	arm_isa_reg_store(ctx, ctx->inst.dword.mul_ln_ins.dst_lo, rdlo);


}

void arm_isa_UMULLS_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_UMLAL_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_UMLALS_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMULL_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMULLS_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMLAL_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMLALS_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SMLALXX_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SWP_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_SWPB_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_BX_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_branch(ctx);
}

void arm_isa_BLX_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
			arm_isa_branch(ctx);
}

void arm_isa_LDRH_ptrm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ptrm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ptrp1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ptrp2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ofrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_prrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ofrp_impl(struct arm_ctx_t *ctx)
{
	int addr;
	int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		mem_read(ctx->mem, addr, 4, buf);
		value = value & (0x0000ffff);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd, value);
	}
}

void arm_isa_LDRH_prrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ptrm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ptrm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ptrp1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ptrp2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ofrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_prrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ofrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_prrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptrm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptrm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptrp1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptrp2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ofrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_prrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ofrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptrm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptrm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptrp1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptrp2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ofrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_prrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ofrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_prrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptrm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptrm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptrp1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptrp2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ofrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_prrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ofrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_prrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptrm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptrm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptrp1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptrp2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ofrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_prrm_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ofrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_prrp_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_CLZ_reg_impl(struct arm_ctx_t *ctx)
{
	int rm_val;
	unsigned int i;
	unsigned int zero_ct;

	zero_ct = 0;
	arm_isa_reg_load(ctx, ctx->inst.dword.hfwrd_reg_ins.off_reg, &rm_val);

	for (i = 2147483648; i >= 1; i /= 2)
	{
		if((rm_val & (i)) == 0)
		{
			zero_ct += 1;
		}
		else
		{
			break;
		}
	}
	arm_isa_inst_debug("  leading zero count = %d\n", zero_ct);
	arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_reg_ins.dst_rd, zero_ct);
}

void arm_isa_LDRH_ptim1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ptim2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ptip1_impl(struct arm_ctx_t *ctx)
{
	int addr;
	int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		mem_read(ctx->mem, addr, 4, buf);
		value = value & (0x0000ffff);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd, value);
	}
}

void arm_isa_LDRH_ptip2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ofim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_prim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRH_ofip_impl(struct arm_ctx_t *ctx)
{

	int addr;
	int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		mem_read(ctx->mem, addr, 4, buf);
		value = value & (0x0000ffff);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd, value);
	}
}

void arm_isa_LDRH_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ptim1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ptim2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ptip1_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		value = value & (0x0000ffff);
		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 2, buf);
	}
}

void arm_isa_STRH_ptip2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ofim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_prim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRH_ofip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		value = value & (0x0000ffff);
		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 2, buf);
	}
}

void arm_isa_STRH_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptim1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptim2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptip1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ptip2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ofim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_prim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSB_ofip_impl(struct arm_ctx_t *ctx)
{

	int addr;
	int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		mem_read(ctx->mem, addr, 1, buf);
		value = value & (0xffffffff);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd, value);
	}
}

void arm_isa_LDRSB_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptim1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptim2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptip1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ptip2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ofim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_prim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRSH_ofip_impl(struct arm_ctx_t *ctx)
{
	int addr;
	int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		mem_read(ctx->mem, addr, 2, buf);
		value = value & (0xffffffff);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd, value);
	}
}

void arm_isa_LDRSH_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptim1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptim2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptip1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ptip2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ofim_impl(struct arm_ctx_t *ctx)
{

	int addr;
	int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		mem_read(ctx->mem, addr, 4, buf);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd, value);

		mem_read(ctx->mem, addr + 4, 4, buf);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd + 1, value);
	}
}

void arm_isa_LDRD_prim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRD_ofip_impl(struct arm_ctx_t *ctx)
{
	int addr;
	int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		mem_read(ctx->mem, addr, 4, buf);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd, value);

		mem_read(ctx->mem, addr + 4, 4, buf);
		arm_isa_reg_store(ctx, ctx->inst.dword.hfwrd_imm_ins.dst_rd + 1, value);
	}
}

void arm_isa_LDRD_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptim1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptim2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptip1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ptip2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ofim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);

		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 4, buf);

		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd + 1, &value);

		if(!(arm_isa_invalid_addr_str(addr + 4, value, ctx)))
			mem_write(ctx->mem, addr + 4, 4, buf);
	}
}

void arm_isa_STRD_prim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRD_ofip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode3_imm(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);

		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 4, buf);

		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd + 1, &value);

		if(!(arm_isa_invalid_addr_str(addr + 4, value, ctx)))
			mem_write(ctx->mem, addr + 4, 4, buf);
	}
}

void arm_isa_STRD_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	unsigned int value;
	void *buf;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);

		if(!(arm_isa_invalid_addr_ldr(addr, buf, ctx)))
		{
			mem_read(ctx->mem, addr, 4, &value);
		}

		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, value);
	}
}

void arm_isa_LDR_ptip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);

		if(!(arm_isa_invalid_addr_ldr(addr, buf, ctx)))
		{
			mem_read(ctx->mem, addr, 4, &value);
		}

		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, value);
	}
}

void arm_isa_LDR_ofim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);

		if(!(arm_isa_invalid_addr_ldr(addr, buf, ctx)))
		{
			mem_read(ctx->mem, addr, 4, &value);
		}

		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, value);
	}
}

void arm_isa_LDR_prim_impl(struct arm_ctx_t *ctx)
{

	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);

		if(!(arm_isa_invalid_addr_ldr(addr, buf, ctx)))
		{
			mem_read(ctx->mem, addr, 4, &value);
		}

		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, value);
	}
}

void arm_isa_LDR_ofip_impl(struct arm_ctx_t *ctx)
{

	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);

		if(!(arm_isa_invalid_addr_ldr(addr, buf, ctx)))
		{
			mem_read(ctx->mem, addr, 4, &value);
		}

		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, value);
	}
}

void arm_isa_LDR_prip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);

		if(!(arm_isa_invalid_addr_ldr(addr, buf, ctx)))
		{
			mem_read(ctx->mem, addr, 4, &value);
		}

		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, value);
	}
}

void arm_isa_LDR_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ofrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ofrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ofrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ofrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ofrpll_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);

		if(!(arm_isa_invalid_addr_ldr(addr, buf, ctx)))
		{
			mem_read(ctx->mem, addr, 4, &value);
		}
		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, value);
	}
}

void arm_isa_LDR_ofrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ofrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_ofrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDR_prrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);

		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 4, buf);
	}
}

void arm_isa_STR_ofim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);

		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 4, buf);
	}
}

void arm_isa_STR_prim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 4, buf);
	}
}

void arm_isa_STR_ofip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);

		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 4, buf);
	}
}

void arm_isa_STR_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ofrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ofrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ofrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ofrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ofrpll_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);

		if(!(arm_isa_invalid_addr_str(addr, value, ctx)))
			mem_write(ctx->mem, addr, 4, buf);
	}
}

void arm_isa_STR_ofrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ofrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_ofrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STR_prrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		mem_read(ctx->mem, addr, 1, buf);
		value = value & (0x000000ff);
		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, (value));

	}
}

void arm_isa_LDRB_ofim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		mem_read(ctx->mem, addr, 1, buf);
		value = value & (0x000000ff);
		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, (value));

	}
}

void arm_isa_LDRB_prim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		mem_read(ctx->mem, addr, 1, buf);
		value = value & (0x000000ff);
		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, (value));

	}
}

void arm_isa_LDRB_ofip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		mem_read(ctx->mem, addr, 1, buf);
		value = value & (0x000000ff);
		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, (value));

	}
}

void arm_isa_LDRB_prip_impl(struct arm_ctx_t *ctx)
{

	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		mem_read(ctx->mem, addr, 1, buf);
		value = value & (0x000000ff);
		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, (value));

	}
}

void arm_isa_LDRB_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ofrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ofrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ofrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ofrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ofrpll_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;


	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		mem_read(ctx->mem, addr, 1, buf);
		value = value & (0x000000ff);
		arm_isa_reg_store(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, (value));
	}
}

void arm_isa_LDRB_ofrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ofrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_ofrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRB_prrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		value = value & (0x000000ff);
		mem_write(ctx->mem, addr, 1, buf);
	}
}

void arm_isa_STRB_ptip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		value = value & (0x000000ff);
		mem_write(ctx->mem, addr, 1, buf);
	}
}

void arm_isa_STRB_ofim_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		value = value & (0x000000ff);
		mem_write(ctx->mem, addr, 1, buf);
	}
}

void arm_isa_STRB_prim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofip_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		value = value & (0x000000ff);
		mem_write(ctx->mem, addr, 1, buf);
	}

}

void arm_isa_STRB_prip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofrpll_impl(struct arm_ctx_t *ctx)
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (arm_isa_check_cond(ctx))
	{
		buf = &value;
		addr = arm_isa_get_addr_amode2(ctx);
		arm_isa_reg_load(ctx, ctx->inst.dword.sdtr_ins.src_dst_rd, &value);
		value = value & (0x000000ff);
		mem_write(ctx->mem, addr, 1, buf);
	}
}

void arm_isa_STRB_ofrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_ofrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRB_prrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRT_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRT_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDRBT_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptim_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptip_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrmll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrmlr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrmar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrmrr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrpll_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrplr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrpar_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STRBT_ptrprr_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMDA_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMDA_w_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMDA_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMDA_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMIA_impl(struct arm_ctx_t *ctx)
{

	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_ld(ctx);
}

void arm_isa_LDMIA_w_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_ld(ctx);
}

void arm_isa_LDMIA_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMIA_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMDB_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMDB_w_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_ld(ctx);
}

void arm_isa_LDMDB_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMDB_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMIB_impl(struct arm_ctx_t *ctx)
{

	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_ld(ctx);
}

void arm_isa_LDMIB_w_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMIB_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDMIB_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMDA_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMDA_w_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMDA_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMDA_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMIA_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_str(ctx);
}

void arm_isa_STMIA_w_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_str(ctx);
}

void arm_isa_STMIA_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMIA_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMDB_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMDB_w_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_str(ctx);
}

void arm_isa_STMDB_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMDB_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMIB_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_amode4s_str(ctx);
}

void arm_isa_STMIB_w_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMIB_u_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STMIB_uw_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_BRNCH_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_branch(ctx);
}

void arm_isa_BRNCH_LINK_impl(struct arm_ctx_t *ctx)
{
	if(arm_isa_check_cond(ctx))
		arm_isa_branch(ctx);
}

void arm_isa_SWI_SVC_impl(struct arm_ctx_t *ctx)
{
	arm_isa_syscall(ctx);
}

void arm_isa_LDC2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STC2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_LDF_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_STF_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_LDMIA1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_LDMIA2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_LDMIA3_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_STMIA1_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_STMIA2_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_STMIA3_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_MSR_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}

void arm_isa_VFP_MRS_impl(struct arm_ctx_t *ctx)
{
	__ARM_NOT_IMPL__
}
