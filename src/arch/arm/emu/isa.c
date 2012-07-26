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

#include <mem-system.h>
#include <arm-emu.h>

/* Debug categories */
int arm_isa_call_debug_category;
int arm_isa_inst_debug_category;

/* FIXME */
char *arm_isa_inst_bytes;


/* Table including references to functions in machine.c
 * that implement machine instructions. */
/* Instruction execution table */


static arm_isa_inst_func_t arm_isa_inst_func[ARM_INST_COUNT] =
{
	NULL /* for op_none */
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) , arm_isa_##_name##_impl
#include <arm-asm.dat>
#undef DEFINST
};




/*
 * Instruction statistics
 */

static long long arm_inst_freq[ARM_INST_COUNT];




/*
 * Register Load/Store Operations
 */

unsigned int arm_isa_get_addr_amode2(struct arm_ctx_t *ctx)
{
	struct arm_inst_t *inst;
	unsigned int rn;
	unsigned int rm;
	unsigned int shift;
	unsigned int offset;
	int rn_val;
	unsigned int ret_addr;
	unsigned int shift_val;
	signed int rm_val;

	inst = &ctx->inst;

	offset = inst->dword.sdtr_ins.off;
	rn = inst->dword.sdtr_ins.base_rn;
	arm_isa_reg_load(ctx, rn, &rn_val);
	arm_isa_inst_debug("  rn = 0x%x\n", rn_val);
	if(inst->dword.sdtr_ins.imm == 1)
	{
		rm = (offset & (0x0000000f));
		arm_isa_reg_load(ctx, rm, &rm_val);
		shift = ((offset >> 4) & (0x000000ff));
		shift_val = ((shift >> 3) & 0x0000001f);

		if(inst->dword.sdtr_ins.up_dn)
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (LSL):
				ret_addr = rn_val + (rm_val << shift_val);
				break;

			case (LSR):
				ret_addr = rn_val + (rm_val >> shift_val);
				break;

			case (ASR):
				ret_addr = rn_val + (rm_val /(int)(pow(2,shift_val)));
				break;

			case (ROR):
				ret_addr = rn_val + arm_rotr(rm_val, shift_val);
				break;
			}
		}
		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (LSL):
				ret_addr = rn_val - (rm_val << shift_val);
				break;

			case (LSR):
				ret_addr = rn_val - (rm_val >> shift_val);
				break;

			case (ASR):
				ret_addr = rn_val - (rm_val / (int) (pow(
					2, shift_val)));
				break;

			case (ROR):
				ret_addr = rn_val - arm_rotr(rm_val, shift_val);
				break;
			}
		}
	}
	else
	{
		if(!offset)
		{
			ret_addr = rn_val + 0;
		}
		else
		{
			if(inst->dword.sdtr_ins.up_dn)
			{
				ret_addr = rn_val + offset;
			}
			else
			{
				ret_addr = rn_val - offset;
			}
		}

	}

	if(inst->dword.sdtr_ins.idx_typ)
	{
		if(inst->dword.sdtr_ins.wb)
			arm_isa_reg_store(ctx, rn, ret_addr);
	}

	arm_isa_inst_debug("  ls/st addr = 0x%x\n", ret_addr);
	return (ret_addr);
}

int arm_isa_op2_get(struct arm_ctx_t *ctx, unsigned int op2 , enum arm_isa_op2_cat_t cat)
{
	unsigned int imm;
	unsigned int rotate;
	int op_val;
	unsigned int shift;
	unsigned int rm;
	unsigned int rs;
	unsigned int shift_imm;
	signed int rm_val;

	if (cat == immd)
	{
		imm = (op2 & (0x000000ff));
		rotate = ((op2 >> 8) & 0x0000000f);
		op_val = (arm_rotr(imm, rotate));
	}

	else if (cat == reg)
	{
		rm = (op2 & (0x0000000f));
		arm_isa_reg_load(ctx, rm, &rm_val);
		shift = ((op2 >> 4) & (0x000000ff));
		shift_imm = ((shift >> 3) & 0x0000001f);

		if (shift & 0x00000001)
		{
			rs = (shift >> 4);
			switch ((shift >> 1) & 0x00000003)
			{
			case (LSL):
					op_val = rm_val << rs;
			break;

			case (LSR):
					op_val = rm_val >> rs;
			break;

			case (ASR):
					op_val = rm_val /(int)(pow(2,rs));
			break;

			case (ROR):
					op_val = arm_rotr(rm_val, rs);
			break;
			}
		}

		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (LSL):
					op_val = rm_val << shift_imm;
			break;

			case (LSR):
					op_val = rm_val >> shift_imm;
			break;

			case (ASR):
					op_val = rm_val /(int)(pow(2,shift_imm));
			break;

			case (ROR):
					op_val = arm_rotr(rm_val, shift_imm);
			break;
			}
		}
	}
	arm_isa_inst_debug("  op2 = %d; 0x%x\n", op_val, op_val);
	return (op_val);
}

int arm_isa_op2_carry(struct arm_ctx_t *ctx,  unsigned int op2 , enum arm_isa_op2_cat_t cat)
{
	unsigned int imm;
	unsigned int rotate;
	unsigned int shift;
	unsigned int rm;
	unsigned int rs;
	unsigned int shift_imm;
	signed int rm_val;
	unsigned int carry_ret;
	unsigned int cry_bit;
	unsigned int cry_mask;
	unsigned int rot_val;
	if (cat == immd)
	{
		imm = (op2 & (0x000000ff));
		rotate = ((op2 >> 8) & 0x0000000f);

		cry_bit = rotate;
		cry_mask = (unsigned int)(pow(2,cry_bit));

		if(cry_mask & imm)
			carry_ret = 1;
		else
			carry_ret = 0;

		arm_isa_inst_debug("  carry bit = %d, imm = 0x%x, rotate = %d\n",
				carry_ret, imm, rotate);
	}

	else if (cat == reg)
	{
		rm = (op2 & (0x0000000f));
		arm_isa_reg_load(ctx, rm, &rm_val);
		shift = ((op2 >> 4) & (0x000000ff));
		shift_imm = ((shift >> 3) & 0x0000001f);

		if (shift & 0x00000001)
		{
			rs = (shift >> 4);
			switch ((shift >> 1) & 0x00000003)
			{
			case (LSL):
				cry_bit = (32 - rs);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;

			case (LSR):
				cry_bit = (rs);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;

			case (ASR):
				cry_bit = (rs);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;

			case (ROR):
				cry_bit = (rs);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;
			}
			rot_val = rs;
		}

		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (LSL):
				cry_bit = (32 - shift_imm);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;

			case (LSR):
				cry_bit = (shift_imm);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;

			case (ASR):
				cry_bit = (shift_imm);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;

			case (ROR):
				cry_bit = (shift_imm);
				cry_mask = (unsigned int)(pow(2,cry_bit));
				break;
			}
			rot_val = shift_imm;
		}

		if(cry_mask & rm_val)
			carry_ret = 1;
		else
			carry_ret = 0;

		arm_isa_inst_debug("  carry bit = %d, rm_val = 0x%x, rotate = %d\n",
								carry_ret, rm_val, rot_val);
	}

	return (carry_ret);
}




void arm_isa_reg_store(struct arm_ctx_t *ctx, unsigned int reg_no, int value)
{
	arm_isa_inst_debug("  r%d <= %d; (0x%x)\n", reg_no, value, value);
	switch (reg_no)
	{
	case (r0):
		ctx->regs->r0 = value;
		break;
	case (r1):
		ctx->regs->r1 = value;
		break;
	case (r2):
		ctx->regs->r2 = value;
		break;
	case (r3):
		ctx->regs->r3 = value;
		break;
	case (r4):
		ctx->regs->r4 = value;
		break;
	case (r5):
		ctx->regs->r5 = value;
		break;
	case (r6):
		ctx->regs->r6 = value;
		break;
	case (r7):
		ctx->regs->r7 = value;
		break;
	case (r8):
		ctx->regs->r8 = value;
		break;
	case (r9):
		ctx->regs->r9 = value;
		break;
	case (r10):
		ctx->regs->sl = value;
		break;
	case (r11):
		ctx->regs->fp = value;
		break;
	case (r12):
		ctx->regs->ip = value;
		break;
	case (r13):
		ctx->regs->sp = value;
		break;
	case (r14):
		ctx->regs->lr = value;
		break;
	case (r15):
		ctx->regs->pc = value + 4;
		break;
	}
}

void arm_isa_reg_store_safe(struct arm_ctx_t *ctx, unsigned int reg_no, unsigned int value)
{
	arm_isa_inst_debug("  r%d <= %d; (0x%x); safe_store\n", reg_no, value, value);
	switch (reg_no)
	{
	case (r0):
		ctx->regs->r0 = value;
		break;
	case (r1):
		ctx->regs->r1 = value;
		break;
	case (r2):
		ctx->regs->r2 = value;
		break;
	case (r3):
		ctx->regs->r3 = value;
		break;
	case (r4):
		ctx->regs->r4 = value;
		break;
	case (r5):
		ctx->regs->r5 = value;
		break;
	case (r6):
		ctx->regs->r6 = value;
		break;
	case (r7):
		ctx->regs->r7 = value;
		break;
	case (r8):
		ctx->regs->r8 = value;
		break;
	case (r9):
		ctx->regs->r9 = value;
		break;
	case (r10):
		ctx->regs->sl = value;
		break;
	case (r11):
		ctx->regs->fp = value;
		break;
	case (r12):
		ctx->regs->ip = value;
		break;
	case (r13):
		ctx->regs->sp = value;
		break;
	case (r14):
		ctx->regs->lr = value;
		break;
	case (r15):
		ctx->regs->pc = value + 4;
		break;
	}
}


void arm_isa_reg_load(struct arm_ctx_t *ctx, unsigned int reg_no, int *value)
{

	switch (reg_no)
	{
	case (r0):
		(*value) = ctx->regs->r0;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r0);
		break;
	case (r1):
		(*value) = ctx->regs->r1;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r1);
		break;
	case (r2):
		(*value) = ctx->regs->r2;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r2);
		break;
	case (r3):
		(*value) = ctx->regs->r3;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r3);
		break;
	case (r4):
		(*value) = ctx->regs->r4;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r4);
		break;
	case (r5):
		(*value) = ctx->regs->r5;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r5);
		break;
	case (r6):
		(*value) = ctx->regs->r6;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r6);
		break;
	case (r7):
		(*value) = ctx->regs->r7;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r7);
		break;
	case (r8):
		(*value) = ctx->regs->r8;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r8);
		break;
	case (r9):
		(*value) = ctx->regs->r9;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->r9);
		break;
	case (r10):
		(*value) = ctx->regs->sl;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->sl);
		break;
	case (r11):
		(*value) = ctx->regs->fp;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->fp);
		break;
	case (r12):
		(*value) = ctx->regs->ip;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->ip);
		break;
	case (r13):
		(*value) = ctx->regs->sp;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->sp);
		break;
	case (r14):
		(*value) = ctx->regs->lr;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->lr);
		break;
	case (r15):
		(*value) = ctx->regs->pc;
		arm_isa_inst_debug("  r%d = 0x%x\n", reg_no, ctx->regs->pc);
		break;
	}
}




/*
 * Branching Functions
 */

void arm_isa_branch(struct arm_ctx_t *ctx)
{
	unsigned int offset;
	unsigned int br_add;
	int rm_val;

	if(ctx->inst.info->category == ARM_CAT_BRNCH)
	{
		offset = (ctx->inst.dword.brnch_ins.off << 2);
		br_add = offset + ctx->regs->pc;

		if(ctx->inst.dword.brnch_ins.link)
		{
			arm_isa_reg_store(ctx, 14, ctx->regs->pc - 4);
		}

		ctx->regs->pc = br_add + 4;
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %d\n", ctx->regs->pc - 4, ctx->regs->pc);
	}

	else if (ctx->inst.info->category == ARM_CAT_BAX)
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.bax_ins.op0_rn, &rm_val);
		ctx->regs->pc = rm_val + 4;
		arm_isa_inst_debug("  Branch addr = 0x%x, pc <= %d\n", rm_val, rm_val);
	}
}

int arm_isa_check_cond(struct arm_ctx_t *ctx)
{
	unsigned int cond;
	struct arm_regs_t *regs;
	unsigned int ret_val;
	regs = ctx->regs;
	cond = ctx->inst.dword.brnch_ins.cond;

	switch (cond)
	{
	case (EQ):
		ret_val = (regs->cpsr.z) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = EQ\n");
		break;

	case (NE):
		ret_val = (!(regs->cpsr.z)) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = NE\n");
		break;

	case (CS):
		ret_val = (regs->cpsr.C) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = CS\n");
		break;

	case (CC):
		ret_val = (!(regs->cpsr.C)) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = CC\n");
		break;

	case (MI):
		ret_val = (regs->cpsr.n) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = MI\n");
		break;

	case (PL):
		ret_val = (!(regs->cpsr.n)) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = PL\n");
		break;

	case (VS):
		ret_val = (regs->cpsr.v) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = VS\n");
		break;

	case (VC):
		ret_val = (!(regs->cpsr.v)) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = VC\n");
		break;

	case (HI):
		ret_val = (!(regs->cpsr.z) && (regs->cpsr.C)) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = HI\n");
		break;

	case (LS):
		ret_val = ((regs->cpsr.z) | !(regs->cpsr.C)) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = LS\n");
		break;

	case (GE):
		ret_val =
			(((regs->cpsr.n) & (regs->cpsr.v))
				| (!(regs->cpsr.n) & !(regs->cpsr.v))) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = GE\n");
		break;

	case (LT):
		ret_val =
			(((regs->cpsr.n) & !(regs->cpsr.v))
				| (!(regs->cpsr.n) && (regs->cpsr.v))) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = LT\n");
		break;

	case (GT):
		ret_val =
			(((regs->cpsr.n) & (regs->cpsr.v) & !(regs->cpsr.z))
				| (!(regs->cpsr.n) & !(regs->cpsr.v)
					& (regs->cpsr.z))) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = GT\n");
		break;

	case (LE):
		ret_val =
			(((regs->cpsr.z) | (!(regs->cpsr.n) && (regs->cpsr.v))
				| ((regs->cpsr.n) && !(regs->cpsr.v)))) ? 1 : 0;
		arm_isa_inst_debug(
			"  Cond = LE\n");
		break;

	case (AL):
		ret_val = 1;
		break;
	}

	return (ret_val);
}

void arm_isa_amode4s_str(struct arm_ctx_t *ctx)
{
	unsigned int reg_list;
	unsigned int wrt_val;
	void *buf;
	int rn_val;
	int copy_buf;
	int i;

	buf = &copy_buf;
	reg_list = ctx->inst.dword.bdtr_ins.reg_lst;
	arm_isa_reg_load(ctx, ctx->inst.dword.bdtr_ins.base_rn, &rn_val);

	if(ctx->inst.dword.bdtr_ins.idx_typ)	/* Pre indexed */
	{
		if (ctx->inst.dword.bdtr_ins.up_dn)
		{
			wrt_val = rn_val + 4;

			for (i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{

					arm_isa_reg_load(ctx, log_base2(i), &copy_buf);
					mem_write(ctx->mem, wrt_val, 4, buf);
					arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
					wrt_val += 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (wrt_val - 4));
		}

		else
		{
			wrt_val = rn_val - 4;

			for (i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					arm_isa_reg_load(ctx, log_base2(i), &copy_buf);
					mem_write(ctx->mem, wrt_val, 4, buf);
					arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
					wrt_val -= 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (wrt_val + 4));

		}
	}

	else	/* Post-Indexed */
	{
		if (ctx->inst.dword.bdtr_ins.up_dn)
		{
			wrt_val = rn_val;

			for (i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{

					arm_isa_reg_load(ctx, log_base2(i), &copy_buf);
					mem_write(ctx->mem, wrt_val, 4, buf);
					arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
					wrt_val += 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (wrt_val));
		}

		else
		{
			wrt_val = rn_val;

			for (i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					arm_isa_reg_load(ctx, log_base2(i), &copy_buf);
					mem_write(ctx->mem, wrt_val, 4, buf);
					arm_isa_inst_debug("  push r%d => 0x%x\n",log_base2(i),wrt_val);
					wrt_val -= 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (wrt_val));

		}
	}
}

void arm_isa_amode4s_ld(struct arm_ctx_t *ctx)
{
	unsigned int reg_list;
	unsigned int read_val;
	void *buf;
	int rn_val;
	int copy_buf;
	int i;

	buf = &copy_buf;
	reg_list = ctx->inst.dword.bdtr_ins.reg_lst;
	arm_isa_reg_load(ctx, ctx->inst.dword.bdtr_ins.base_rn, &rn_val);

	if(ctx->inst.dword.bdtr_ins.idx_typ)	/* Pre indexed */
	{
		if (ctx->inst.dword.bdtr_ins.up_dn)
		{
			read_val = rn_val + 4;

			for (i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{
					mem_read(ctx->mem, read_val, 4, buf);
					arm_isa_reg_store(ctx, log_base2(i), copy_buf);
					arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),read_val);
					read_val += 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (read_val - 4));
		}

		else
		{
			read_val = rn_val - 4;

			for (i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					mem_read(ctx->mem, read_val, 4, buf);
					arm_isa_reg_store(ctx, log_base2(i), copy_buf);
					arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),read_val);
					read_val -= 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (read_val + 4));

		}
	}

	else	/* Post-Indexed */
	{
		if (ctx->inst.dword.bdtr_ins.up_dn)
		{
			read_val = rn_val;

			for (i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{

					mem_read(ctx->mem, read_val, 4, buf);
					arm_isa_reg_store(ctx, log_base2(i), copy_buf);
					arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),read_val);
					read_val += 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (read_val));
		}

		else
		{
			read_val = rn_val;

			for (i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					mem_read(ctx->mem, read_val, 4, buf);
					arm_isa_reg_store(ctx, log_base2(i), copy_buf);
					arm_isa_inst_debug("  pop r%d <= 0x%x\n",log_base2(i),read_val);
					read_val -= 4;
				}
			}
			if(ctx->inst.dword.bdtr_ins.wb)
				arm_isa_reg_store(ctx, ctx->inst.dword.bdtr_ins.base_rn, (read_val));

		}
	}
}

void arm_isa_cpsr_print(struct arm_ctx_t *ctx)
{
	arm_isa_inst_debug("  CPSR update\n"
		"  n = %d\n"
		"  z = %d\n"
		"  c = %d\n"
		"  v = %d\n"
		"  q = %d\n"
		"  mode = 0x%x\n",ctx->regs->cpsr.n,ctx->regs->cpsr.z,ctx->regs->cpsr.C,
		ctx->regs->cpsr.v,ctx->regs->cpsr.q,ctx->regs->cpsr.mode);
}

void arm_isa_subtract(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3)
{
	int rd_val;
	int rn_val;
	unsigned int rd_val_safe;
	arm_isa_reg_load(ctx, rn, &rn_val);

	if(!(ctx->inst.dword.dpr_ins.s_cond))
	{
		if(rd == 15)
		{
			rd_val_safe = rn_val - op2 - op3;
			arm_isa_inst_debug("  r%d = r%d - %d\n", rd, rn, op2);
			arm_isa_reg_store_safe(ctx, rd, rd_val_safe);
		}
		else
		{
			rd_val = rn_val - op2 - op3;
			arm_isa_inst_debug("  r%d = r%d - %d\n", rd, rn, op2);
			arm_isa_reg_store(ctx, rd, rd_val);
		}
	}
	else
	{

	}
}

void arm_isa_subtract_rev(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3)
{
	int rd_val;
	int rn_val;

	arm_isa_reg_load(ctx, rn, &rn_val);

	if(!(ctx->inst.dword.dpr_ins.s_cond))
	{
		rd_val = op2 - rn_val - op3;
		arm_isa_inst_debug("  r%d = r%d - %d\n", rd, rn, op2);
		arm_isa_reg_store(ctx, rd, rd_val);
	}
	else
	{

	}
}


void arm_isa_add(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3)
{
	int rd_val;
	int rn_val;
	struct arm_regs_t *regs;
	unsigned int rd_val_safe;

	regs = ctx->regs;
	arm_isa_reg_load(ctx, rn, &rn_val);

	if(!(ctx->inst.dword.dpr_ins.s_cond))
	{
		rd_val = rn_val + op2 + op3;
		arm_isa_inst_debug("  r%d = r%d + %d\n", rd, rn, op2);
		arm_isa_reg_store(ctx, rd, rd_val);
	}
	else
	{
		rd_val = rn_val + op2 + op3;
		rd_val_safe = (unsigned int)rn_val + (unsigned int)op2 + (unsigned int)op3;
		arm_isa_inst_debug("  r%d = r%d + %d\n", rd, rn, op2);
		arm_isa_reg_store(ctx, rd, rd_val);

		regs->cpsr.z = 0;
		regs->cpsr.n = 0;
		regs->cpsr.C = 0;
		regs->cpsr.v = 0;

		if(rd_val == 0)
		{
			regs->cpsr.z = 1;
			regs->cpsr.n = 0;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if(rd_val < 0)
		{
			regs->cpsr.n = 1;
			regs->cpsr.C = 0;
			regs->cpsr.v = 0;
		}
		if (rd_val_safe > (0xffffffff))
		{
			regs->cpsr.C = 1;
			regs->cpsr.v = 0;
		}

		if (((rn_val >= 0 && rd_val < op2) || (rn_val < 0 && rd_val > op2)))
		{
			regs->cpsr.v = 1;
		}
		arm_isa_cpsr_print(ctx);
	}
}

void arm_isa_multiply(struct arm_ctx_t *ctx)
{
	int rm_val;
	int rs_val;
	int rd_val;
	int rn_val;

	if (!(ctx->inst.dword.mult_ins.m_acc))
	{
		arm_isa_reg_load(ctx, ctx->inst.dword.mult_ins.op1_rs, &rs_val);
		arm_isa_reg_load(ctx, ctx->inst.dword.mult_ins.op0_rm, &rm_val);

		rd_val = rm_val * rs_val;

		arm_isa_reg_store(ctx, ctx->inst.dword.mult_ins.dst_rd, rd_val);
	}
	else
	{

		arm_isa_reg_load(ctx, ctx->inst.dword.mult_ins.op1_rs, &rs_val);
		arm_isa_reg_load(ctx, ctx->inst.dword.mult_ins.op0_rm, &rm_val);
		arm_isa_reg_load(ctx, ctx->inst.dword.mult_ins.op2_rn, &rn_val);

		rd_val = (rm_val * rs_val) + rn_val;

		arm_isa_reg_store(ctx, ctx->inst.dword.mult_ins.dst_rd, rd_val);
	}
}

void arm_isa_syscall(struct arm_ctx_t *ctx)
{
	if(ctx->regs->r7 == ARM_set_tls)
	{
		ctx->regs->r7 = 330;
	}
	arm_sys_call(ctx);
	if(!ctx->regs->r0)
		arm_isa_inst_debug("  System call code = %d\n", ctx->regs->r7);
	else
		arm_isa_inst_debug("  System call code = %d\n", ctx->regs->r7);
}




/*
 * Public Functions
 */


/* Trace call debugging */
static void arm_isa_debug_call(struct arm_ctx_t *ctx)
{

	/* FIXME: Implement branch calling method for Arm */
	/*
	struct elf_symbol_t *from;
	struct elf_symbol_t *to;

	struct arm_regs_t *regs = ctx->regs;

	char *action;
	int i;

	 Do nothing on speculative mode
	if (ctx->status & arm_ctx_spec_mode)
		return;

	 Call or return. Otherwise, exit
	if (!strncmp(ctx->inst.info->fmt_str, "call", 4))
		action = "call";
	else if (!strncmp(ctx->inst.info->fmt_str, "ret", 3))
		action = "ret";
	else
		return;

	 Debug it
	for (i = 0; i < ctx->function_level; i++)
		arm_isa_call_debug("| ");
	from = elf_symbol_get_by_address(loader->elf_file, ctx->curr_eip, NULL);
	to = elf_symbol_get_by_address(loader->elf_file, regs->eip, NULL);
	if (from)
		arm_isa_call_debug("%s", from->name);
	else
		arm_isa_call_debug("0x%x", ctx->curr_eip);
	arm_isa_call_debug(" - %s to ", action);
	if (to)
		arm_isa_call_debug("%s", to->name);
	else
		arm_isa_call_debug("0x%x", regs->eip);
	arm_isa_call_debug("\n");

	 Change current level
	if (strncmp(ctx->inst.format, "call", 4))
		ctx->function_level--;
	else
		ctx->function_level++;
	*/
}

/* FIXME - merge with ctx_execute */
void arm_isa_execute_inst(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;


	/* Set last, current, and target instruction addresses */
	ctx->last_ip = ctx->curr_ip;
	ctx->curr_ip = (regs->pc - 4);
	ctx->target_ip = 0;

	/* Debug */
	if (debug_status(arm_isa_inst_debug_category))
	{
		arm_isa_inst_debug("%d %8lld %x: ", ctx->pid,
			arm_emu->inst_count, ctx->curr_ip);
		arm_inst_debug_dump(&ctx->inst, debug_file(arm_isa_inst_debug_category));
		arm_isa_inst_debug("  (%d bytes)", ctx->inst.info->size);
	}

	/* Call instruction emulation function */
	regs->pc = regs->pc + ctx->inst.info->size;
	if (ctx->inst.info->opcode)
		arm_isa_inst_func[ctx->inst.info->opcode](ctx);

	/* Statistics */
	arm_inst_freq[ctx->inst.info->opcode]++;

	/* Debug */
	arm_isa_inst_debug("\n");
	if (debug_status(arm_isa_call_debug_category))
		arm_isa_debug_call(ctx);
}


