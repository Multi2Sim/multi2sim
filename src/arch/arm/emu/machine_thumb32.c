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


char *arm_th32_err_isa_note =
	"\tThe ARM instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define __ARM_TH32_NOT_IMPL__ \
	fatal("%s: Arm 32 instruction '%s' not implemented\n%s", \
		__FUNCTION__, ctx->inst_th_32.info->name, arm_th32_err_isa_note);



void arm_th32_isa_STREX_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDREX_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRD_imm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRD_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRD_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRD_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRD_imm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRD_imm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_lit1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_lit2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imma_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_immb_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_immc_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_immd_impl(struct arm_ctx_t *ctx)
{
	int offset;
	int value1;
	int value2;
	void *buf1;
	void *buf2;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf1 = &value1;
	buf2 = &value2;
	immd12 = ctx->inst_th_32.dword.ld_st_double.immd8;
	idx = ctx->inst_th_32.dword.ld_st_double.index;
	add = ctx->inst_th_32.dword.ld_st_double.add_sub;
	wback = ctx->inst_th_32.dword.ld_st_double.wback;

	if(ctx->inst_th_32.dword.ld_st_double.rn < 15)
	{
		arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ld_st_double.rn, &rn_val);
		if(add)
			offset = rn_val + (immd12 & 0x000000ff);
		else
			offset = rn_val - (immd12 & 0x000000ff);

		if(idx)
			addr = offset;
		else
			addr = rn_val;
		arm_isa_inst_debug("imm4  addr  = %d; (0x%x)\n", addr, addr);
		if(wback)
			arm_isa_reg_store(ctx,  ctx->inst_th_32.dword.ld_st_double.rn, offset);

		mem_read(ctx->mem, addr, 4, buf1);
		mem_read(ctx->mem, addr + 4, 4, buf2);
		value1 = value1 & (0xffffffff);
		value2 = value2 & (0xffffffff);

		if(ctx->inst_th_32.dword.ld_st_double.rt < 15)
			arm_isa_reg_store(ctx,  ctx->inst_th_32.dword.ld_st_double.rt, value1);
		else
		{
			if(value1 % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt, value1 - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt, value1 - 2);
		}

		if(ctx->inst_th_32.dword.ld_st_double.rt2 < 15)
			arm_isa_reg_store(ctx,  ctx->inst_th_32.dword.ld_st_double.rt2, value2);
		else
		{
			if(value1 % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt2, value2 - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt2, value2 - 2);
		}

	}

	else if (ctx->inst_th_32.dword.ldstr_imm.rn == 15)
	{
		arm_isa_inst_debug("  pc  = 0x%x; \n", ctx->regs->pc);
		if((ctx->regs->pc - 4) % 4 == 2)
			offset = (ctx->regs->pc - 4) + 2;
		else
			offset = ctx->regs->pc - 4;
		arm_isa_inst_debug("  offset  = 0x%x; \n", offset);
		if(ctx->inst_th_32.dword.ld_st_double.add_sub)
			addr = offset + immd12;
		else
			addr = offset - immd12;

		mem_read(ctx->mem, addr, 4, buf1);
		mem_read(ctx->mem, addr + 4, 4, buf2);
		value1 = value1 & (0xffffffff);
		value2 = value2 & (0xffffffff);

		if(ctx->inst_th_32.dword.ld_st_double.rt < 15)
			arm_isa_reg_store(ctx,  ctx->inst_th_32.dword.ld_st_double.rt, value1);
		else
		{
			if(value1 % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt, value1 - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt, value1 - 2);
		}

		if(ctx->inst_th_32.dword.ld_st_double.rt2 < 15)
			arm_isa_reg_store(ctx,  ctx->inst_th_32.dword.ld_st_double.rt2, value2);
		else
		{
			if(value1 % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt2, value2 - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ld_st_double.rt2, value2 - 2);
		}
	}
}

void arm_th32_isa_LDRD_imm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm20_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm21_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm22_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm23_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm24_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm25_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm26_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm27_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm28_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm29_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm210_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm211_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm212_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm213_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRD_imm214_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_TBB_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_TBH_impl(struct arm_ctx_t *ctx)
{
	int rn_val;
	int rm_val;
	int addr;
	unsigned int hfwrd;
	void *buf;

	hfwrd = 0;
	buf = &hfwrd;
	if(ctx->inst_th_32.dword.table_branch.rn == 15)
	{
		arm_isa_inst_debug("  PC = 0x%x\n", ctx->regs->pc);
		arm_isa_reg_load(ctx, ctx->inst_th_32.dword.table_branch.rn, &rn_val);
		rn_val = rn_val;
	}

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.table_branch.rm, &rm_val);

	rm_val = rm_val << 1;

	addr = rn_val + rm_val;
	arm_isa_inst_debug("  Addr = 0x%x\n", addr);

	mem_read(ctx->mem, addr, 2, buf);
	arm_isa_inst_debug("  HFwrd = 0x%x; Changed PC = 0x%x\n", hfwrd, (ctx->regs->pc - 2 + (2 * hfwrd)));
	ctx->regs->pc = ctx->regs->pc + (2 * hfwrd);
}

void arm_th32_isa_STM_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDM15_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_POP_impl(struct arm_ctx_t *ctx)
{
	struct arm_thumb32_inst_t *inst;
	enum arm_thumb32_cat_enum cat;
	unsigned int regs;
	unsigned int i;
	int wrt_val;
	int sp_val;
	int reg_val;
	void* buf;


	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	buf = &reg_val;

	if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
		regs = inst->dword.ld_st_mult.reglist;
	else if (cat == ARM_THUMB32_CAT_PUSH_POP)
		regs = inst->dword.push_pop.reglist;

	else
		fatal("%d: regs fmt not recognized", cat);
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

void arm_th32_isa_PUSH_impl(struct arm_ctx_t *ctx)
{
	struct arm_thumb32_inst_t *inst;
	enum arm_thumb32_cat_enum cat;
	unsigned int regs;
	unsigned int i;
	int wrt_val;
	int sp_val;
	int reg_val;
	void* buf;


	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	buf = &reg_val;

	if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
		regs = inst->dword.ld_st_mult.reglist;
	else if (cat == ARM_THUMB32_CAT_PUSH_POP)
		regs = inst->dword.push_pop.reglist;

	else
		fatal("%d: regs fmt not recognized", cat);
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

void arm_th32_isa_LDMDB_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STMDB15_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_TST_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg9_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	struct arm_regs_t *regs;
	//struct arm_regs_t prev_regs;
	enum arm_thumb32_cat_enum cat;
	int rm_val;
	int rn_val;
	int result;

	regs = ctx->regs;
	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	//prev_regs.cpsr = regs->cpsr;
	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);
	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rn, &rn_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	result = rn_val & rm_val;
	arm_isa_reg_store(ctx, inst->dword.data_proc_shftreg.rd, result);
	if(ctx->inst_th_32.dword.data_proc_immd.sign)
	{
		regs = ctx->regs;
		//prev_regs.cpsr = regs->cpsr;

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
		//regs->cpsr.C = prev_regs.cpsr.C;
		arm_isa_cpsr_print(ctx);
	}


}

void arm_th32_isa_AND_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BIC_reg_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	struct arm_regs_t *regs;
	//struct arm_regs_t prev_regs;
	enum arm_thumb32_cat_enum cat;
	int rm_val;
	int rn_val;
	int result;

	regs = ctx->regs;
	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	//prev_regs.cpsr = regs->cpsr;
	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);
	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rn, &rn_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	result = rn_val & (~(rm_val));
	arm_isa_reg_store(ctx, inst->dword.data_proc_shftreg.rd, result);
	if(ctx->inst_th_32.dword.data_proc_immd.sign)
	{
		regs = ctx->regs;
		//prev_regs.cpsr = regs->cpsr;

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
		//regs->cpsr.C = prev_regs.cpsr.C;
		arm_isa_cpsr_print(ctx);
	}

}

void arm_th32_isa_ORR_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg2_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	struct arm_regs_t *regs;
	//struct arm_regs_t prev_regs;
	enum arm_thumb32_cat_enum cat;
	int rm_val;
	int rn_val;
	int result;

	regs = ctx->regs;
	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	//prev_regs.cpsr = regs->cpsr;
	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);
	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rn, &rn_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	result = rn_val | rm_val;
	arm_isa_reg_store(ctx, inst->dword.data_proc_shftreg.rd, result);
	if(ctx->inst_th_32.dword.data_proc_immd.sign)
	{
		regs = ctx->regs;
		//prev_regs.cpsr = regs->cpsr;

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
		//regs->cpsr.C = prev_regs.cpsr.C;
		arm_isa_cpsr_print(ctx);
	}

}

void arm_th32_isa_ORR_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MVN_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_TST_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg0_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	enum arm_thumb32_cat_enum cat;
	int rm_val;

	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	arm_thumb_add_isa(ctx, inst->dword.data_proc_shftreg.rd,
		inst->dword.data_proc_shftreg.rn, rm_val, 0, 0);

}

void arm_th32_isa_ADD_reg1_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	enum arm_thumb32_cat_enum cat;
	int rm_val;

	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	arm_thumb_add_isa(ctx, inst->dword.data_proc_shftreg.rd,
		inst->dword.data_proc_shftreg.rn, rm_val, 0, 0);

}

void arm_th32_isa_ADD_reg2_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	enum arm_thumb32_cat_enum cat;
	int rm_val;

	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	arm_thumb_add_isa(ctx, inst->dword.data_proc_shftreg.rd,
		inst->dword.data_proc_shftreg.rn, rm_val, 0, 0);

}

void arm_th32_isa_ADD_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg4_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	enum arm_thumb32_cat_enum cat;
	int rm_val;

	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	arm_thumb_add_isa(ctx, inst->dword.data_proc_shftreg.rd,
		inst->dword.data_proc_shftreg.rn, rm_val, 0, 0);

}

void arm_th32_isa_ADD_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg9_impl(struct arm_ctx_t *ctx)
{
	unsigned int shift;
	unsigned int type;

	struct arm_thumb32_inst_t *inst;
	enum arm_thumb32_cat_enum cat;
	int rm_val;

	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

	arm_isa_reg_load(ctx, inst->dword.data_proc_shftreg.rm, &rm_val);

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			rm_val = rm_val << shift;
		break;

		case (ARM_OPTR_LSR):
			rm_val = rm_val >> shift;
		break;

		case (ARM_OPTR_ASR):
			rm_val = rm_val / (1 << shift);
		break;

		case (ARM_OPTR_ROR):
			rm_val = arm_rotr(rm_val, shift);
		break;
		}
	}

	arm_thumb_add_isa(ctx, inst->dword.data_proc_shftreg.rd,
		inst->dword.data_proc_shftreg.rn, rm_val, 0, 0);

}

void arm_th32_isa_ADD_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CMN_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADC_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SBC_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CMP_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_RSB_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MOVS_LSR_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MOVS_ASR_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MOVS_LSL_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_AND_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_TST_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BIC_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd;
	int result;
	int rn_val;

	struct arm_regs_t *regs;

	regs = ctx->regs;
	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
					| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
					| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	immd = arm_isa_thumb_immd_extend(immd);

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.data_proc_immd.rn, &rn_val);

	result = rn_val & (~(immd));
	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.data_proc_immd.rd, result);
	if(ctx->inst_th_32.dword.data_proc_immd.sign)
	{
		regs = ctx->regs;
		//prev_regs.cpsr = regs->cpsr;

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
		//regs->cpsr.C = prev_regs.cpsr.C;
		arm_isa_cpsr_print(ctx);
	}
}

void arm_th32_isa_ORR_imm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm4_impl(struct arm_ctx_t *ctx)
{
	unsigned int operand;
	int rn_val;
	int result;

	struct arm_regs_t *regs;
	struct arm_regs_t prev_regs;


	operand = arm_thumb32_isa_immd12(&ctx->inst_th_32, ctx->inst_th_32.info->cat32);
	arm_isa_inst_debug("  immd32 = 0x%x\n", operand);

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.data_proc_immd.rn, &rn_val);

	result = rn_val | operand;
	arm_isa_inst_debug("  result = 0x%x\n", result);

	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.data_proc_immd.rd, result);

	if(ctx->inst_th_32.dword.data_proc_immd.sign)
	{
		regs = ctx->regs;
		prev_regs.cpsr = regs->cpsr;

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if(operand == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(operand < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		regs->cpsr.C = prev_regs.cpsr.C;
		arm_isa_cpsr_print(ctx);
	}
}

void arm_th32_isa_ORR_imm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORR_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MOV_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned int operand;

	operand = arm_thumb32_isa_immd12(&ctx->inst_th_32, ctx->inst_th_32.info->cat32);
	arm_isa_inst_debug("  immd32 = %d\n", operand);
	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.data_proc_immd.rd, operand);
}

void arm_th32_isa_ORN_imm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ORN_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MVN_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_EOR_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_TEQ_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm0_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);

	immd = arm_isa_thumb_immd_extend(immd);

	arm_thumb_add_isa(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);

}

void arm_th32_isa_ADD_imm1_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	immd = arm_isa_thumb_immd_extend(immd);
	arm_thumb_add_isa(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);
}

void arm_th32_isa_ADD_imm2_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	immd = arm_isa_thumb_immd_extend(immd);
	arm_thumb_add_isa(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);

}

void arm_th32_isa_ADD_imm3_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	immd = arm_isa_thumb_immd_extend(immd);
	arm_thumb_add_isa(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);


}

void arm_th32_isa_ADD_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm6_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	immd = arm_isa_thumb_immd_extend(immd);
	arm_thumb_add_isa(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);

}

void arm_th32_isa_ADD_imm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm9_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	immd = arm_isa_thumb_immd_extend(immd);
	arm_thumb_add_isa(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);


}

void arm_th32_isa_ADD_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CMN_imm_impl(struct arm_ctx_t *ctx)
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

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.data_proc_immd.rn, &rn_val);
	operand2 = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);

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

void arm_th32_isa_SUB_imm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm1_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	arm_thumb_isa_subtract(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);
}

void arm_th32_isa_SUB_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm5_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	arm_thumb_isa_subtract(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);
}

void arm_th32_isa_SUB_imm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CMP_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned long flags;
	unsigned int immd;
	int rn_val;
	int rd_val;
	int result;
	int operand2;
	int op2;
	struct arm_regs_t *regs;

	regs = ctx->regs;

	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	immd = arm_isa_thumb_immd_extend(immd);
	operand2 = immd;
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.data_proc_immd.rn, &rn_val);
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

void arm_th32_isa_RSB_imm_impl(struct arm_ctx_t *ctx)
{
	unsigned int flags;
	unsigned int immd;
	flags = ctx->inst_th_32.dword.data_proc_immd.sign;

	immd = (ctx->inst_th_32.dword.data_proc_immd.i_flag << 11)
		| (ctx->inst_th_32.dword.data_proc_immd.immd3 << 8)
		| (ctx->inst_th_32.dword.data_proc_immd.immd8);
	arm_thumb_isa_rev_subtract(ctx, ctx->inst_th_32.dword.data_proc_immd.rd,
		ctx->inst_th_32.dword.data_proc_immd.rn, immd, 0, flags);
}

void arm_th32_isa_SBC_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADC_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ADD_binimm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SUB_binimm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFI_binimm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BFC_binimm_impl(struct arm_ctx_t *ctx)
{
	unsigned int msb;
	unsigned int immd2;
	unsigned int immd3;
	unsigned int lsb;
	int rd_val;
	struct arm_thumb32_inst_t *inst;
	int i_mask;
	int i;

	inst = &ctx->inst_th_32;

	msb = inst->dword.bit_field.msb;
	immd2 = inst->dword.bit_field.immd2;
	immd3 = inst->dword.bit_field.immd3;

	lsb = (immd3 << 2) | immd2;
	arm_isa_inst_debug("  msb = 0x%x, lsb = 0x%x\n", msb, lsb);
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.bit_field.rd, &rd_val);
	for (i = lsb; i <= msb; i++)
	{
		i_mask = 1 << i;
		rd_val = rd_val & (~(i_mask));
	}
	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.bit_field.rd, rd_val);
}

void arm_th32_isa_UBFX_binimm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SBFX_binimm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MOVT_binimm_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd16;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int immd4;
	int rd_val;

	enum arm_thumb32_cat_enum cat;
	struct arm_thumb32_inst_t *inst;
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.bit_field.rd, &rd_val);
	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

		if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
		{
			immd8 = inst->dword.data_proc_immd.immd8;
			immd3 = inst->dword.data_proc_immd.immd3;
			i = inst->dword.data_proc_immd.i_flag;
			immd4 = inst->dword.data_proc_immd.rn;
		}

		else
			fatal("%d: immd16 fmt not recognized", cat);

		immd16 = (immd4 << 12) | (i << 11) | (immd3 << 8) | immd8;

	arm_isa_inst_debug("  immd16 = 0x%x\n", immd16);
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.data_proc_immd.rd, &rd_val);
	arm_isa_inst_debug("  rd_val = 0x%x\n", rd_val);
	rd_val = (rd_val & 0x0000ffff) | (immd16 << 16);
	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.data_proc_immd.rd, rd_val);
}

void arm_th32_isa_MOVW_binimm_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd16;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int immd4;

	enum arm_thumb32_cat_enum cat;
	struct arm_thumb32_inst_t *inst;

	inst = &ctx->inst_th_32;
	cat = ctx->inst_th_32.info->cat32;

		if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
		{
			immd8 = inst->dword.data_proc_immd.immd8;
			immd3 = inst->dword.data_proc_immd.immd3;
			i = inst->dword.data_proc_immd.i_flag;
			immd4 = inst->dword.data_proc_immd.rn;
		}

		else
			fatal("%d: immd16 fmt not recognized", cat);

		immd16 = (immd4 << 12) | (i << 11) | (immd3 << 8) | immd8;

	arm_isa_inst_debug("  immd32 = %d\n", immd16);
	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.data_proc_immd.rd, immd16);

}

void arm_th32_isa_B_0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_01_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_02_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_03_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_04_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_05_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_06_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_2_impl(struct arm_ctx_t *ctx)
{
	arm_thumb32_isa_branch(ctx);
}

void arm_th32_isa_B_21_impl(struct arm_ctx_t *ctx)
{
	arm_thumb32_isa_branch(ctx);
}

void arm_th32_isa_B_22_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_23_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_24_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_25_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_26_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_B_3_impl(struct arm_ctx_t *ctx)
{
	arm_thumb32_isa_branch(ctx);
}

void arm_th32_isa_NOP_0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_NOP_1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BL_0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BL_1_impl(struct arm_ctx_t *ctx)
{
	arm_thumb32_isa_branch_link(ctx);
}

void arm_th32_isa_BLX_0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_BLX_1_impl(struct arm_ctx_t *ctx)
{
	arm_thumb32_isa_branch_link(ctx);
}

void arm_th32_isa_STRB_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_immd_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRBT_immd_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_immd_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_immd_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_immd2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRBT_immd2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd02_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_immd2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_immd2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_immd4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRBT_immd4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd04_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_immd4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_immd4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_immd66_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRBT_immd6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd06_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_immd6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRHT_immd6_impl(struct arm_ctx_t *ctx)
{
	int offset;
	int value;
	void *buf;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = ctx->inst_th_32.dword.ldstr_imm.immd12;
	idx = (immd12 & 0x00000400) >> 10;
	add = (immd12 & 0x00000200) >> 9;
	wback = (immd12 & 0x00000100) >> 8;

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, &rn_val);
	if(add)
		offset = rn_val + (immd12 & 0x000000ff);
	else
		offset = rn_val - (immd12 & 0x000000ff);

	if(idx)
		addr = offset;
	else
		addr = rn_val;
	arm_isa_inst_debug(" immd12 = %x; offset = %x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);
	if(wback)
		arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, offset);

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, &value);
	value = value & (0xffffffff);
	mem_write(ctx->mem, addr, 4, buf);
}

void arm_th32_isa_STR_immd6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_immda_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_immdb_impl(struct arm_ctx_t *ctx)
{
	int offset;
	int value;
	void *buf;
	int immd12;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = ctx->inst_th_32.dword.ldstr_imm.immd12;
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, &rn_val);
	offset = rn_val + (immd12);
	addr = offset;
	arm_isa_inst_debug(" immd12 = %x; offset = %x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, &value);
	value = value & (0x000000ff);
	mem_write(ctx->mem, addr, 1, buf);

}

void arm_th32_isa_STRB_immdc_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRB_immdd_impl(struct arm_ctx_t *ctx)
{
	int offset;
	int value;
	void *buf;
	int immd12;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = ctx->inst_th_32.dword.ldstr_imm.immd12;
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, &rn_val);
	offset = rn_val + (immd12);
	addr = offset;
	arm_isa_inst_debug(" immd12 = %x; offset = %x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, &value);
	value = value & (0x000000ff);
	mem_write(ctx->mem, addr, 1, buf);
}

void arm_th32_isa_STRH_immda_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_immdb_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_immdc_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STRH_immdd_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_immda_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_immdb_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_immdc_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STR_immdd_impl(struct arm_ctx_t *ctx)
{
	unsigned int immd12;
	int rn_val;
	int addr;
	int rd_val;
	void *buf;

	buf = &rd_val;
	immd12 = ctx->inst_th_32.dword.ldstr_imm.immd12;

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rn,
				&rn_val);

	addr = rn_val + immd12;

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rd,
					&rd_val);

	arm_isa_inst_debug(" r%d (0x%x) => [0x%x]\n",
		ctx->inst_th_32.dword.ldstr_imm.rd, rd_val, addr);

	mem_write(ctx->mem, addr, 4, buf);
}

void arm_th32_isa_LDRB_lit_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm40_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm30_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm50_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm70_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm41_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm31_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm51_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm71_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm42_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm32_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm52_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm72_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm43_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm33_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm53_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm73_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm44_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm34_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm54_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm74_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm45_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm15_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm35_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm55_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm75_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm46_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm16_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm36_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm56_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm76_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm47_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm17_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm37_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm57_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm77_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm48_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm18_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm38_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm58_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm78_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm49_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm19_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm39_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm59_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm79_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm410_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm110_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm310_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm510_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm710_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm411_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm111_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm311_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm511_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm711_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm412_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm112_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm312_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm512_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm712_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm413_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm113_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm313_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm513_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm713_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm414_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm114_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm314_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm514_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm714_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_imm715_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_lit_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm40_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm30_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm50_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm70_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm41_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm31_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm51_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm71_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm42_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm32_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm52_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm72_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm43_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm33_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm53_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm73_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm44_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm34_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm54_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm74_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm45_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm15_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm35_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm55_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm75_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm46_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm16_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm36_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm56_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm76_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm47_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm17_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm37_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm57_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm77_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm48_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm18_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm38_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm58_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm78_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm49_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm19_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm39_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm59_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm79_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm410_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm110_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm310_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm510_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm710_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm411_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm111_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm311_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm511_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm711_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm412_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm112_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm312_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm512_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm712_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm413_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm113_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm313_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm513_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm713_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm414_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm114_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm314_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm514_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_imm714_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA40_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA30_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA50_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA70_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA41_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA31_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA51_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA71_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA42_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA32_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA52_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA72_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA43_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA33_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA53_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA73_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA44_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA34_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA54_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA74_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA45_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA15_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA35_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA55_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA75_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA46_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA16_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA36_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA56_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA76_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA47_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA17_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA37_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA57_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA77_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA48_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA18_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA38_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA58_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA78_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA49_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA19_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA39_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA59_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA79_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA410_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA110_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA310_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA510_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA710_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA411_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA111_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA311_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA511_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA711_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA412_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA112_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA312_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA512_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA712_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA413_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA113_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA313_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA513_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA713_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_regA14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA414_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA114_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA314_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA514_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immA714_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA40_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA30_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA50_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA70_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA41_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA31_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA51_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA71_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA42_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA32_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA52_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA72_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA43_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA33_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA53_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA73_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA44_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA34_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA54_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA74_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA45_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA15_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA35_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA55_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA75_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA46_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA16_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA36_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA56_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA76_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA47_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA17_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA37_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA57_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA77_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA48_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA18_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA38_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA58_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA78_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA49_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA19_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA39_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA59_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA79_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA410_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA110_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA310_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA510_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA710_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA411_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA111_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA311_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA511_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA711_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA412_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA112_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA312_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA512_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA712_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA413_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA113_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA313_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA513_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA713_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_regA14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA414_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA114_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA314_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA514_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA714_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_litA_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRB_immA7140_impl(struct arm_ctx_t *ctx)
{
	int offset;
	int value;
	void *buf;
	int immd12;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = ctx->inst_th_32.dword.ldstr_imm.immd12;
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, &rn_val);
	offset = rn_val + (immd12);
	addr = offset;
	arm_isa_inst_debug(" immd12 = 0x%x; offset = 0x%x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);
	mem_read(ctx->mem, addr, 1, buf);
	value = value & (0x000000ff);
	if(ctx->inst_th_32.dword.ldstr_imm.rd < 15)
	{
		arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value);
	}
	else
	{
		if(value % 2)
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 3);
		else
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 2);
	}
}

void arm_th32_isa_LDRSB_litA_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immB1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSB_immB2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRH_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRH_imm0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSH_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSH_imm1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRH_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRH_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRH_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRH_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSH_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSH_imm2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSH_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDRSH_imm4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDR_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDR_imm_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDR_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDR_imm2_impl(struct arm_ctx_t *ctx)
{
	int offset;
	int value;
	void *buf;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = ctx->inst_th_32.dword.ldstr_imm.immd12;
	idx = (immd12 & 0x00000400) >> 10;
	add = (immd12 & 0x00000200) >> 9;
	wback = (immd12 & 0x00000100) >> 8;

	if(ctx->inst_th_32.dword.ldstr_imm.rn < 15)
	{
		arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, &rn_val);
		if(add)
			offset = rn_val + (immd12 & 0x000000ff);
		else
			offset = rn_val - (immd12 & 0x000000ff);

		if(idx)
			addr = offset;
		else
			addr = rn_val;
		arm_isa_inst_debug("imm2  addr  = %d; (0x%x)\n", addr, addr);
		if(wback)
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, offset);

		mem_read(ctx->mem, addr, 4, buf);
		value = value & (0xffffffff);

		if(ctx->inst_th_32.dword.ldstr_imm.rd < 15)
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 2);
		}
	}
	else if (ctx->inst_th_32.dword.ldstr_imm.rn == 15)
	{
		arm_isa_inst_debug("  pc  = 0x%x; \n", ctx->regs->pc);
		if((ctx->regs->pc - 4) % 4 == 2)
			offset = (ctx->regs->pc - 4) + 2;
		else
			offset = ctx->regs->pc - 4;
		arm_isa_inst_debug("  offset  = 0x%x; \n", offset);
		if(ctx->inst_th_32.dword.ldstr_imm.add)
			addr = offset + immd12;
		else
			addr = offset - immd12;

		mem_read(ctx->mem, addr, 4, buf);
		value = value & (0xffffffff);
		if(ctx->inst_th_32.dword.ldstr_imm.rd < 15)
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 2);
		}

	}
}

void arm_th32_isa_LDR_imm3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LDR_imm4_impl(struct arm_ctx_t *ctx)
{
	int offset;
	int value;
	void *buf;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = ctx->inst_th_32.dword.ldstr_imm.immd12;
	idx = (immd12 & 0x00000400) >> 10;
	add = (immd12 & 0x00000200) >> 9;
	wback = (immd12 & 0x00000100) >> 8;

	if(ctx->inst_th_32.dword.ldstr_imm.rn < 15)
	{
		arm_isa_reg_load(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, &rn_val);
		if(add)
			offset = rn_val + (immd12 & 0x000000ff);
		else
			offset = rn_val - (immd12 & 0x000000ff);

		if(idx)
			addr = offset;
		else
			addr = rn_val;
		arm_isa_inst_debug("imm4  addr  = %d; (0x%x)\n", addr, addr);
		if(wback)
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rn, offset);

		mem_read(ctx->mem, addr, 4, buf);
		value = value & (0xffffffff);

		if(ctx->inst_th_32.dword.ldstr_imm.rd < 15)
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 2);
		}
	}
	else if (ctx->inst_th_32.dword.ldstr_imm.rn == 15)
	{
		arm_isa_inst_debug("  pc  = 0x%x; \n", ctx->regs->pc);
		if((ctx->regs->pc - 4) % 4 == 2)
			offset = (ctx->regs->pc - 4) + 2;
		else
			offset = ctx->regs->pc - 4;
		arm_isa_inst_debug("  offset  = 0x%x; \n", offset);
		if(ctx->inst_th_32.dword.ldstr_imm.add)
			addr = offset + immd12;
		else
			addr = offset - immd12;

		mem_read(ctx->mem, addr, 4, buf);
		value = value & (0xffffffff);
		if(ctx->inst_th_32.dword.ldstr_imm.rd < 15)
			arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 3);
			else
				arm_isa_reg_store(ctx, ctx->inst_th_32.dword.ldstr_imm.rd, value - 2);
		}

	}

}

void arm_th32_isa_MUL_impl(struct arm_ctx_t *ctx)
{
	int rm_val;
	int rn_val;

	int result;

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.mult.rn, &rn_val);
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.mult.rm, &rm_val);

	result = rn_val * rm_val;

	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.mult.rd, result);

}

void arm_th32_isa_MLA0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA5_impl(struct arm_ctx_t *ctx)
{
	int rm_val;
	int rn_val;
	int ra_val;


	int result;

	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.mult.rn, &rn_val);
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.mult.rm, &rm_val);
	arm_isa_reg_load(ctx, ctx->inst_th_32.dword.mult.ra, &ra_val);

	result = (rn_val * rm_val) + ra_val;

	arm_isa_reg_store(ctx, ctx->inst_th_32.dword.mult.rd, result);

}

void arm_th32_isa_MLA6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLA14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MLS_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SMULL_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UMULL_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LSL_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LSL_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LSR_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_LSR_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ASR_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_ASR_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_SXTH_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTH_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg0_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg4_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg5_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg6_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg7_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg8_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg9_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg10_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg11_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg12_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg13_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTAH_reg14_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UXTB_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UADD8_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_REV_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CLZ_reg_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UADD8_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_REV_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CLZ_reg1_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UADD8_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_REV_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CLZ_reg2_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_UADD8_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_REV_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_CLZ_reg3_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_MRC_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}

void arm_th32_isa_STLDC_impl(struct arm_ctx_t *ctx)
{
	__ARM_TH32_NOT_IMPL__
}
