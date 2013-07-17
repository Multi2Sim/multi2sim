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

#include <math.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>
#include <lib/util/misc.h>

#include "context.h"
#include "isa.h"
#include "machine.h"
#include "regs.h"


char *mips_err_note =
	"\tThe MIPS instruction is currently in active development in Multi2Sim.\n"
	"\tIf you encounter an unimplemented instruction, please send an  email to\n"
	"\tdevelopment@multi2sim.org to request support for it\n";

#define __MIPS_NOT_IMPL__								  \
	fatal("%s, Mips instruction '%s' not implemented.\nAddress: 0x%x\n", \
		__FUNCTION__, ctx->inst.info->name, ctx->regs->pc);


void mips_isa_J_impl(struct mips_ctx_t *ctx)
{
	unsigned int dest;
	dest = BITS32(ctx->regs->pc + 4, 31, 28) << 28 | (TARGET << 2);
	mips_isa_branch(ctx,dest);
}
void mips_isa_JAL_impl(struct mips_ctx_t *ctx)
{
	unsigned int reg_no = 31;
	unsigned int value = ctx->regs->pc + 8;
	unsigned int dest = (BITS32(ctx->regs->pc + 4, 31, 28) << 28) | (TARGET << 2);
//	mips_gpr_set_value(ctx,31, ctx->regs->pc + 8);
	mips_gpr_set_value(ctx,reg_no, value);
	mips_isa_branch(ctx,dest);
}
void mips_isa_BEQ_impl(struct mips_ctx_t *ctx)
{
	if (mips_gpr_get_value(ctx,RS) == mips_gpr_get_value(ctx,RT))
		mips_isa_rel_branch(ctx,SEXT32(IMM << 2,16));
		//mips_isa_rel_branch(ctx,SEXT32(SEXT32(IMM,16) << 2, 18));
	mips_isa_inst_debug("  regRS = %d and regRT = %d", mips_gpr_get_value(ctx,RS), mips_gpr_get_value(ctx,RT));
}
void mips_isa_BNE_impl(struct mips_ctx_t *ctx)
{
	if (mips_gpr_get_value(ctx,RS) != mips_gpr_get_value(ctx,RT))
	{
		mips_isa_rel_branch(ctx,SEXT32(IMM << 2, 16));
		mips_isa_inst_debug("  Branch taken");
	}
	else
		mips_isa_inst_debug("  Branch not taken");

}
void mips_isa_BLEZ_impl(struct mips_ctx_t *ctx)
{
	if ((int)(mips_gpr_get_value(ctx,RS)) <= 0)
		mips_isa_rel_branch(ctx,SEXT32(IMM << 2,16));
		//mips_isa_rel_branch(ctx,IMM << 2);
}
void mips_isa_BGTZ_impl(struct mips_ctx_t *ctx)
{
	if ((int)(mips_gpr_get_value(ctx,RS)) > 0)
		mips_isa_rel_branch(ctx,IMM << 2);
}
void mips_isa_ADDI_impl(struct mips_ctx_t *ctx)
{
	int temp;
	temp = (int)mips_gpr_get_value(ctx,RS) + (int)(OFFSET);
	mips_gpr_set_value(ctx,RT, temp);
}
void mips_isa_ADDIU_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RT, mips_gpr_get_value(ctx,RS) + SEXT32((signed)IMM, 16));
	mips_isa_inst_debug("  r%d -> r%d+0x%x", RT, RS, SEXT32(IMM, 16));
}
void mips_isa_SLTI_impl(struct mips_ctx_t *ctx)
{
	if ((int)(mips_gpr_get_value(ctx,RS)) < IMM)
		mips_gpr_set_value(ctx,RT, 1);
	else
		mips_gpr_set_value(ctx,RT, 0);
}
void mips_isa_SLTIU_impl(struct mips_ctx_t *ctx)
{
	if ((unsigned int)mips_gpr_get_value(ctx,RS) < (unsigned int)SEXT32(IMM,16))
		mips_gpr_set_value(ctx,RT, 1);
	else
		mips_gpr_set_value(ctx,RT, 0);
}
void mips_isa_ANDI_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RT, mips_gpr_get_value(ctx,RS) & (unsigned int)IMM);
}
void mips_isa_ORI_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RT, mips_gpr_get_value(ctx,RS) | ((unsigned int) (IMM) & ((1U << (16)) - 1)));
		     // (unsigned int) IMM);
	mips_isa_inst_debug(" r%d=0x%x", RT, mips_gpr_get_value(ctx,RT));
}
void mips_isa_XORI_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RT, mips_gpr_get_value(ctx,RS) ^ (unsigned int) IMM);
}
void mips_isa_LUI_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RT, (int)(IMM << 16));
	mips_isa_inst_debug("  r%d: $0x%x", RT, mips_gpr_get_value(ctx,RT));

}
void mips_isa_BEQL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BNEL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BLEZL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BGTZL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_LB_impl(struct mips_ctx_t *ctx)
{
	unsigned char temp;
	unsigned int addr = mips_gpr_get_value(ctx,RS) + SEXT32(IMM,16);
	mem_read(ctx->mem, addr, sizeof(unsigned char), &temp);
	mips_gpr_set_value(ctx,RT, SEXT32(temp, 8));
}
void mips_isa_LH_impl(struct mips_ctx_t *ctx)
{
	unsigned short int temp;
	unsigned int addr = mips_gpr_get_value(ctx,RS) + IMM;
	mem_read(ctx->mem, addr, sizeof(unsigned short int), &temp);
	mips_gpr_set_value(ctx,RT, SEXT32(temp, 16));
}
void mips_isa_LWL_impl(struct mips_ctx_t *ctx)
{
	unsigned char src[4];
	unsigned int rt_value = mips_gpr_get_value(ctx,RT);
	unsigned char *dst = (unsigned char *)& rt_value;
	unsigned int addr = mips_gpr_get_value(ctx,RS) + SEXT32(IMM,16);
	int i, size = 4 - (addr & 3);
	mem_read(ctx->mem, addr, size, src);
	for	(i = 0; i < size; i++)
		dst[3 - i] = src[i];
}
void mips_isa_LW_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp;
	unsigned int addr = mips_gpr_get_value(ctx,RS) + SEXT32((signed)IMM,16);
	if ((BITS32(addr, 1, 0) | 0) == 1 )
		fatal("LW: address error, effective address must be naturallty-aligned\n");
	mem_read(ctx->mem, addr, 4, &temp);
	mips_gpr_set_value(ctx,RT, temp);
	mips_isa_inst_debug("  $0x%x=>tmp0, tmp0+r%d=>tmp0, tmp0=>r%d\n", SEXT32(IMM,16), RS, RT);
	mips_isa_inst_debug("  value loaded: %x", temp);
}
void mips_isa_LBU_impl(struct mips_ctx_t *ctx)
{
	unsigned char temp;
	unsigned int addr = mips_gpr_get_value(ctx,RS) + SEXT32(IMM,16);
	mem_read(ctx->mem, addr, sizeof(unsigned char), &temp);
	mips_gpr_set_value(ctx,RT, (unsigned)temp);
	mips_isa_inst_debug(" r%d=0x%x", RT, mips_gpr_get_value(ctx,RT));
}
void mips_isa_LHU_impl(struct mips_ctx_t *ctx)
{
	unsigned short int temp;
	unsigned int addr = mips_gpr_get_value(ctx,RS) + IMM;
	mem_read(ctx->mem, addr, sizeof(unsigned short int), &temp);
	mips_gpr_set_value(ctx,RT, temp);
}
void mips_isa_LWR_impl(struct mips_ctx_t *ctx)
{
	unsigned char src[4];
	unsigned int rt_value = mips_gpr_get_value(ctx,RT);
	unsigned char *dst = (unsigned char *) & rt_value;
	unsigned int addr = mips_gpr_get_value(ctx,RS) + SEXT32(IMM,16);
	int i, size = 1 + (addr & 3);
	mem_read(ctx->mem, addr - size+ 1, size, src);
	for (i = 0; i < size; i++)
		dst[size - i -1] = src[i];
}
void mips_isa_SB_impl(struct mips_ctx_t *ctx)
{
	unsigned char temp = mips_gpr_get_value(ctx,RT);
	unsigned int addr = mips_gpr_get_value(ctx,RS) + IMM;
	mem_write(ctx->mem, addr, sizeof(unsigned char), &temp);
	mips_isa_inst_debug("byte written: %x",temp);
}
void mips_isa_SH_impl(struct mips_ctx_t *ctx)
{
	unsigned short int temp = mips_gpr_get_value(ctx,RT);
	unsigned int addr = mips_gpr_get_value(ctx,RS) + IMM;
	mem_write(ctx->mem, addr, sizeof(unsigned short int), &temp);
}
void mips_isa_SWL_impl(struct mips_ctx_t *ctx)
{
	unsigned int rt_value = mips_gpr_get_value(ctx,RT);
	unsigned char *src = (unsigned char *) & rt_value;
	unsigned char dst[4];
	unsigned int addr = mips_gpr_get_value(ctx,RS) + IMM;
	int size = 4 - (addr & 3);
	int i;
	for(i = 0; i < size; i++)
		dst[i] = src[3 -i];
	mem_write(ctx->mem, addr, size, dst);
}
void mips_isa_SW_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp = mips_gpr_get_value(ctx,RT);
	unsigned int addr = mips_gpr_get_value(ctx,RS) + SEXT32(IMM,16);

	mem_write(ctx->mem, addr, 4, &temp);
	mips_isa_inst_debug(" value stored: 0x%x", temp);
}


void mips_isa_SWR_impl(struct mips_ctx_t *ctx)
{
	unsigned int rt_value = mips_gpr_get_value(ctx,RT);
	unsigned char *src = (unsigned char *) & rt_value;
	unsigned char dst[4];
	unsigned int addr = mips_gpr_get_value(ctx,RS) + IMM;
	int size = 1 + (addr & 3);
	int i;
	for(i =0; i < size; i++)
		dst[i] = src[size - i -1];
	mem_write(ctx->mem, addr - size + 1, size, dst);
}


void mips_isa_CACHE_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}



void mips_isa_LL_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp;
	mem_read(ctx->mem, mips_gpr_get_value(ctx,RS) + IMM, sizeof(unsigned int), &temp);
	mips_gpr_set_value(ctx,RT, temp);
	// FIXME: add details from m2s-1.3/src/kernel/machine.def
}
void mips_isa_LWC1_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp;
	float f;
	mem_read(ctx->mem, mips_gpr_get_value(ctx,RS) + IMM, sizeof(unsigned int), &temp);
	f = *(float *) &temp;
	MIPS_FPR_S_SET(FT, f);
}
void mips_isa_LWC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_PREF_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_LDC1_impl(struct mips_ctx_t *ctx)
{
	unsigned long int temp;
	mem_read(ctx->mem, mips_gpr_get_value(ctx,RS) + IMM, sizeof(unsigned int), &temp);
	MIPS_FPR_D_SET(FT, * (double *) &temp);
}
void mips_isa_LDC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SC_impl(struct mips_ctx_t *ctx)
{      
	unsigned int temp = mips_gpr_get_value(ctx,RT);
	//	if()
	mem_write(ctx->mem, mips_gpr_get_value(ctx,RS) + IMM, 4, &temp);
	mips_gpr_set_value(ctx,RT, 1);
	// FIXME: revisit details in m2s-1.3/srs/kernel/machine.def
}
void mips_isa_SWC1_impl(struct mips_ctx_t *ctx)
{
	float f = MIPS_FPR_S_GET(FT);
	unsigned int temp = * (unsigned int *) &f;
	mem_write(ctx->mem, mips_gpr_get_value(ctx,RS) + IMM, sizeof(unsigned int), &temp);
}
void mips_isa_SWC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SDC1_impl(struct mips_ctx_t *ctx)
{
	double dbl = MIPS_FPR_D_GET(FT);
	unsigned long int temp = * (unsigned long int *) &dbl;
	mem_write(ctx->mem, mips_gpr_get_value(ctx,RS) + IMM, sizeof(unsigned int), &temp);
}
void mips_isa_SDC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}

/* End of the standard instructions */
	
/* Instructions with opcode SPECIAL */

void mips_isa_SLL_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, (mips_gpr_get_value(ctx,RT) << SA));
	mips_isa_inst_debug("  %x=%x<<%x", mips_gpr_get_value(ctx,RD), mips_gpr_get_value(ctx,RT), SA);
}
void mips_isa_MOVF_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVT_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SRL_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RT) >> SA);
}
void mips_isa_ROR_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SRA_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, (int)mips_gpr_get_value(ctx,RT) >> SA);
}
void mips_isa_SLLV_impl(struct mips_ctx_t *ctx)
{
	unsigned int s;
	s = BITS32(mips_gpr_get_value(ctx,RS), 4, 0);
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RT) << s);
}
void mips_isa_SRLV_impl(struct mips_ctx_t *ctx)
{
	unsigned int s = BITS32(mips_gpr_get_value(ctx,RS), 4, 0);
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RT) >> s);
}
void mips_isa_ROTRV_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SRAV_impl(struct mips_ctx_t *ctx)
{
	unsigned int s = BITS32(mips_gpr_get_value(ctx,RS), 4, 0);
	mips_gpr_set_value(ctx,RD, (int)mips_gpr_get_value(ctx,RT) >> s);
}
void mips_isa_JR_impl(struct mips_ctx_t *ctx)
{
	mips_isa_branch(ctx,mips_gpr_get_value(ctx,RS));
	mips_isa_inst_debug(" target address=0x%x", mips_gpr_get_value(ctx,RS));
}
void mips_isa_JALR_impl(struct mips_ctx_t *ctx)
{
	mips_isa_branch(ctx,mips_gpr_get_value(ctx,RS));
	mips_gpr_set_value(ctx,RD, ctx->regs->pc + 8);
	mips_isa_inst_debug("  reg %d = pc + 8 = %x",RD, mips_gpr_get_value(ctx,RD));
}
void mips_isa_MOVZ_impl(struct mips_ctx_t *ctx)
{
	if(mips_gpr_get_value(ctx,RT) == 0)
		mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RS));
}
void mips_isa_MOVN_impl(struct mips_ctx_t *ctx)
{
	if(mips_gpr_get_value(ctx,RT) != 0)
		mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RS));
}
void mips_isa_SYSCALL_impl(struct mips_ctx_t *ctx)
{
	mips_sys_call(ctx);
}
void mips_isa_BREAK_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SYNC_impl(struct mips_ctx_t *ctx)
{
	//__MIPS_NOT_IMPL__
}
void mips_isa_MFHI_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, MIPS_REG_HI);
}
void mips_isa_MTHI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MFLO_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, MIPS_REG_LO);
}
void mips_isa_MTLO_impl(struct mips_ctx_t *ctx)
{
	ctx->regs->regs_LO = mips_gpr_get_value(ctx,RS);
}
void mips_isa_MULT_impl(struct mips_ctx_t *ctx)
{
	unsigned long long int temp = (long long int) mips_gpr_get_value(ctx,(int)RS) *
			(long long int) mips_gpr_get_value(ctx,(int)RT);
	ctx->regs->regs_LO = BITS64(temp, 31, 0);
	ctx->regs->regs_HI = BITS64(temp, 63, 32);
}
void mips_isa_MULTU_impl(struct mips_ctx_t *ctx)
{
	unsigned long long int temp = (unsigned long long int) (mips_gpr_get_value(ctx,RS)) *
		(unsigned long long int) (mips_gpr_get_value(ctx,RT));
	ctx->regs->regs_LO = BITS64(temp, 31, 0);
	ctx->regs->regs_HI = BITS64(temp, 63, 32);
	mips_isa_inst_debug(" r%d=%x, r%d=%x, prod=%llu\n",RS,mips_gpr_get_value(ctx,RS),RT, mips_gpr_get_value(ctx,RT), temp);
	mips_isa_inst_debug(" \t\t\tregHI=%x and regLO=%x",MIPS_REG_HI, MIPS_REG_LO);
}
void mips_isa_DIV_impl(struct mips_ctx_t *ctx)
{
	if(mips_gpr_get_value(ctx,RT))
	{
		ctx->regs->regs_LO = (int)mips_gpr_get_value(ctx,RS) /
								(int)mips_gpr_get_value(ctx,RT);
		ctx->regs->regs_HI = (int)mips_gpr_get_value(ctx,RS) %
								(int)mips_gpr_get_value(ctx,RT);
	}
}
void mips_isa_DIVU_impl(struct mips_ctx_t *ctx)
{
	if(mips_gpr_get_value(ctx,RT))
	{
		ctx->regs->regs_LO = mips_gpr_get_value(ctx,RS) /
								mips_gpr_get_value(ctx,RT);
		ctx->regs->regs_HI = mips_gpr_get_value(ctx,RS) %
								mips_gpr_get_value(ctx,RT);
	}
}
void mips_isa_ADD_impl(struct mips_ctx_t *ctx)
{
	int temp = (int)mips_gpr_get_value(ctx,RS) +
				(int)mips_gpr_get_value(ctx,RT);
	mips_gpr_set_value(ctx,RD, temp);
}
void mips_isa_ADDU_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RS) + mips_gpr_get_value(ctx,RT));
	mips_isa_inst_debug("  %x = %x + %x", mips_gpr_get_value(ctx,RD), mips_gpr_get_value(ctx,RS), mips_gpr_get_value(ctx,RT));
}
void mips_isa_SUB_impl(struct mips_ctx_t *ctx)
{

	__MIPS_NOT_IMPL__
}
void mips_isa_SUBU_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RS) - mips_gpr_get_value(ctx,RT));
}
void mips_isa_AND_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RS) & mips_gpr_get_value(ctx,RT));
}
void mips_isa_OR_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RS) | mips_gpr_get_value(ctx,RT));
}
void mips_isa_XOR_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, mips_gpr_get_value(ctx,RS) ^ mips_gpr_get_value(ctx,RT));
}
void mips_isa_NOR_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp = mips_gpr_get_value(ctx,RS) | mips_gpr_get_value(ctx,RT);
	mips_gpr_set_value(ctx,RD, ~temp);
}
void mips_isa_SLT_impl(struct mips_ctx_t *ctx)
{
	if ((int)mips_gpr_get_value(ctx,RS) < (int)mips_gpr_get_value(ctx,RT))
		mips_gpr_set_value(ctx,RD, 1);
	else
		mips_gpr_set_value(ctx,RD, 0);
}
void mips_isa_SLTU_impl(struct mips_ctx_t *ctx)
{
	if (mips_gpr_get_value(ctx,RS) < mips_gpr_get_value(ctx,RT))
		mips_gpr_set_value(ctx,RD, 1);
	else
		mips_gpr_set_value(ctx,RD, 0);
}
void mips_isa_TGE_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TGEU_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLT_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLTU_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TEQ_impl(struct mips_ctx_t *ctx)
{
	if ((int)mips_gpr_get_value(ctx,RS) == (int)mips_gpr_get_value(ctx,RT))
	{
		//FIXME:	TRAP, this is supposed to trap
	}
}
void mips_isa_TNE_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}

/* End of instructions with opcode SPECIAL */
	
/* Instructions with opcode REGIMM = 0x01 arg2 is the rt field
 * (bits [20:16]) */

void mips_isa_BLTZ_impl(struct mips_ctx_t *ctx)
{
	if ((int)mips_gpr_get_value(ctx,RS) < 0)
		mips_isa_rel_branch(ctx,IMM << 2);
}
void mips_isa_BGEZ_impl(struct mips_ctx_t *ctx)
{
	if ((int)mips_gpr_get_value(ctx,RS) >= 0)
		mips_isa_rel_branch(ctx,SEXT32(IMM << 2,16));
}
void mips_isa_BLTZL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BGEZL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TGEI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TGEIU_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLTI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLTIU_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TEQI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TNEI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BLTZAL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BGEZAL_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,31, ctx->regs->pc + 8);
	if ((int)mips_gpr_get_value(ctx,RS) >= 0)
		mips_isa_rel_branch(ctx,SEXT32(IMM,16) << 2);
	mips_isa_inst_debug("  BGEZAL: PC=%8x, r31=%8x",
			ctx->regs->pc, ctx->regs->regs_R[31]);
}
void mips_isa_BLTZALL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BGEZALL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SYNCI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}

/* End of REGIMM instructions */
	
/* Instructions with opcode SPECIAL2 = 1c arg2 is the function field 
 * (bits [5:0]) */

void mips_isa_MADD_impl(struct mips_ctx_t *ctx)
{/*
	long int temp, temp1, temp2, temp3;
	temp1 = (int)mips_gpr_get_value(ctx,RS);
	temp2 = (int)mips_gpr_get_value(ctx,RT);
	temp3 = ((unsigned long int) ctx->regs->regs_HI << 32) |
			((unsigned long int) ctx->regs->regs_LO);
	temp = temp1 * temp2 + temp3;
	ctx->regs->regs_HI = BITS64(temp, 63, 32);
	ctx->regs->regs_LO = BITS64(temp, 31, 0);*/
}
void mips_isa_MADDU_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MUL_impl(struct mips_ctx_t *ctx)
{
	long int temp = (long int) mips_gpr_get_value(ctx,(int)RS) *
					(long int) mips_gpr_get_value(ctx,(int)RT);
	mips_gpr_set_value(ctx,RD, temp);
}
void mips_isa_MSUB_impl(struct mips_ctx_t *ctx)
{
	/*	long int temp, temp1, temp2, temp3;
	temp1 = mips_gpr_get_value(ctx,(int)RS);
	temp2 = mips_gpr_get_value(ctx,(int)RT);
	temp3 = ((unsigned long int) MIPS_REG_HI << 32) |
			((unsigned long int) MIPS_REG_LO);
	temp = temp3 - temp1 * temp2 + temp3;
	ctx->regs->regs_HI = BITS64(temp, 63, 32);
	ctx->regs->regs_LO = BITS64(temp, 31, 0);*/
}
void mips_isa_MSUBU_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CLZ_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CLO_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}

//void mips_isa_ST(SDBBP_impl(struct mips_ctx_t *ctx)
//{
//}

/* Ednd of SPECIAL2 instructions */

/* Instructions with opcode SPECIAL3=0x1f */

void mips_isa_EXT_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}

//#define MASK_BITS(X, HI, LO) (unsigned int)((X) & ((((1ull)<<(HI-LO+1))-1) << LO))
void mips_isa_INS_impl(struct mips_ctx_t *ctx)
{
	if (SA > RD)
		mips_isa_inst_debug(" INS: LSB>MSB, unpredictable behavior\n");
	unsigned int value, new_rt;
	value = BITS32(mips_gpr_get_value(ctx,RS),(mips_gpr_get_value(ctx,RD)-mips_gpr_get_value(ctx,SA)),0);
	new_rt = SET_BITS_32(mips_gpr_get_value(ctx,RT), mips_gpr_get_value(ctx,RD), mips_gpr_get_value(ctx,SA), value);
	mips_gpr_set_value(ctx,RT, new_rt);
	mips_isa_inst_debug(" r%d=$0x%x", RT, mips_gpr_get_value(ctx,RT));
	//__MIPS_NOT_IMPL__
}
void mips_isa_WSBH_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SEB_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, SEXT32(BITS32(mips_gpr_get_value(ctx,RT), 7, 0),16));
}
void mips_isa_SEH_impl(struct mips_ctx_t *ctx)
{
	mips_gpr_set_value(ctx,RD, SEXT32((unsigned short int)mips_gpr_get_value(ctx,RT), 16));
}
void mips_isa_RDHWR_impl(struct mips_ctx_t *ctx)
{
	if (RD == 29)
	{
		mips_gpr_set_value(ctx,RT, ctx->regs->regs_cop0[RD]);
		mips_isa_inst_debug("  value in HW reg is 0x%x", MIPS_COP0_GET(RD));
	}
	//FIXME: This is implemented only for the case where RD is 29
//	__MIPS_NOT_IMPL__
}

/* End of SPECIAL3 Instructions */

/* Instructions with opcode COP0=0x10 */

void mips_isa_MFCO_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MTCO_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_RDPGPR_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_DI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_EI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_WRPGPR_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLBR_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLBWI_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLBWR_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TLBP_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_ERET_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_DERET_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_WAIT_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}

/* End of COP0 instructions */
	
/* Instructions with opcode COP1=0x11 */

void mips_isa_MFC1_impl(struct mips_ctx_t *ctx)
{
	float temp = MIPS_FPR_S_GET((int)FS);
	unsigned int wrd = * (unsigned int *) &temp;
	mips_gpr_set_value(ctx,RT, wrd);
}
void mips_isa_CFC1_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp = 0;
	switch(FS)
	{
	case 0:
		temp = MIPS_REG_C_FPC_FIR;
		break;
	case 25:
		temp = (BITS32(MIPS_REG_C_FPC_FCSR, 31, 25) << 1) |
				BITS32(MIPS_REG_C_FPC_FCSR, 23, 23);
		break;
	case 26:
		temp = (BITS32(MIPS_REG_C_FPC_FCSR, 17, 12) << 12) |
				(BITS32(MIPS_REG_C_FPC_FCSR, 6, 2) << 2);
		break;
	case 28:
		temp = (BITS32(MIPS_REG_C_FPC_FCSR, 11, 7) << 7) |
				BITS32(MIPS_REG_C_FPC_FCSR, 1, 0);
		break;
	case 31:
		temp = MIPS_REG_C_FPC_FCSR;
		break;
	default:
		fatal("machine.c: CFC1: unknown value for fs");
	}
	mips_gpr_set_value(ctx,RT, temp);
}
void mips_isa_MFHC1_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MTC1_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp = mips_gpr_get_value(ctx,RT);
	float f = * (float *) &temp;
	MIPS_FPR_S_SET(FS, f);
}
void mips_isa_CTC1_impl(struct mips_ctx_t *ctx)
{
	unsigned int temp = mips_gpr_get_value(ctx,RT);
	switch(FS)
	{
	case 25:
		MIPS_REG_C_FPC_FCSR = (BITS32(temp, 7, 1) << 25) |
							(BITS32(MIPS_REG_C_FPC_FCSR, 24, 24) << 24) |
							(BITS32(temp, 0, 0) << 23) |
							BITS32(MIPS_REG_C_FPC_FCSR, 1, 0);
		break;
	case 26:
		MIPS_REG_C_FPC_FCSR = (BITS32(MIPS_REG_C_FPC_FCSR, 31, 18) << 18) |
								(BITS32(temp, 17, 12) << 12) |
								(BITS32(MIPS_REG_C_FPC_FCSR, 11, 7) << 7) |
								(BITS32(temp, 6, 2) << 2) |
								BITS32(MIPS_REG_C_FPC_FCSR, 1, 0);
		break;
	case 28:
		MIPS_REG_C_FPC_FCSR = (BITS32(MIPS_REG_C_FPC_FCSR, 31, 25) << 25) |
								(BITS32(temp, 2, 2) << 24) |
								(BITS32(MIPS_REG_C_FPC_FCSR, 23, 12) << 12) |
								(BITS32(temp, 11, 7) << 7) |
								(BITS32(MIPS_REG_C_FPC_FCSR, 6, 2) << 2) |
								BITS32(temp, 1, 0);
		break;
	case 31:
		MIPS_REG_C_FPC_FCSR = temp;
		break;
	default:
		fatal("machine.c: CTC1: unknown value for fs");
	}
}
void mips_isa_MTHC1_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC1F_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC1FL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC1T_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC1TL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_ADD_S_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_S_SET(FD, MIPS_FPR_S_GET(FS) + MIPS_FPR_S_GET(FT));
}
void mips_isa_SUB_S_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_S_SET(FD, MIPS_FPR_S_GET(FS) - MIPS_FPR_S_GET(FT));
}
void mips_isa_MUL_S_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_S_SET(FD, MIPS_FPR_S_GET(FS) * MIPS_FPR_S_GET(FT));
}
void mips_isa_DIV_S_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_S_SET(FD, MIPS_FPR_S_GET(FS) / MIPS_FPR_S_GET(FT));
}
void mips_isa_SQRT_S_impl(struct mips_ctx_t *ctx)
{
	float temp = sqrt(MIPS_FPR_S_GET(FS));
	MIPS_FPR_S_SET(FD, temp);
}
void mips_isa_ABS_S_impl(struct mips_ctx_t *ctx)
{
	float temp = MIPS_FPR_S_GET(FS) < 0.0 ? -MIPS_FPR_S_GET(FS) : MIPS_FPR_S_GET(FS);
	MIPS_FPR_S_SET(FD, temp);
}
void mips_isa_MOV_S_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_S_SET(FD, MIPS_FPR_S_GET(FS));
}
void mips_isa_NEG_S_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_S_SET(FD, -MIPS_FPR_S_GET(FS));
}
void mips_isa_ROUND_L_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TRUNC_L_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CEIL_L_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_FLOOR_L_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_ROUND_W_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TRUNC_W_S_impl(struct mips_ctx_t *ctx)
{
	float temp = MIPS_FPR_S_GET(FS);
	int itmp = (int) temp;
	MIPS_FPR_S_SET(FD, * (float *) &itmp);
}
void mips_isa_CEIL_W_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_FLOOR_W_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVF_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVT_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVZ_S_impl(struct mips_ctx_t *ctx)
{
	if (!mips_gpr_get_value(ctx,RT))
		MIPS_FPR_S_SET(FD, MIPS_FPR_S_GET(FS));
}
void mips_isa_MOVN_S_impl(struct mips_ctx_t *ctx)
{
	if (mips_gpr_get_value(ctx,RT))
		MIPS_FPR_S_SET(FD, MIPS_FPR_S_GET(FS));
}
void mips_isa_RECIP_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_RSQRT_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CVT_D_S_impl(struct mips_ctx_t *ctx)
{
	float temp = (float) MIPS_FPR_D_GET(FS);
	MIPS_FPR_S_SET(FD, temp);
}
void mips_isa_CVT_W_S_impl(struct mips_ctx_t *ctx)
{
	float temp = (float) MIPS_FPR_S_GET(FS);
	int wrd = * (int *) &temp;
	MIPS_FPR_S_SET(FD, (float) wrd);
}
void mips_isa_CVT_L_S_impl(struct mips_ctx_t *ctx)
{
	double temp = MIPS_FPR_D_GET(FS);
	long int dwrd = * (long int *) &temp;
	MIPS_FPR_S_SET(FD, (double)dwrd);
}
void mips_isa_CVT_PS_S_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_ADD_D_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_D_SET(FD, MIPS_FPR_D_GET(FS) + MIPS_FPR_D_GET(FT));
}
void mips_isa_SUB_D_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_D_SET(FD, MIPS_FPR_D_GET(FS) - MIPS_FPR_D_GET(FT));
}
void mips_isa_MUL_D_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_D_SET(FD, MIPS_FPR_D_GET(FS) * MIPS_FPR_D_GET(FT));
}
void mips_isa_DIV_D_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_D_SET(FD, MIPS_FPR_D_GET(FS) / MIPS_FPR_D_GET(FT));
}
void mips_isa_SQRT_D_impl(struct mips_ctx_t *ctx)
{
	double temp = sqrt(MIPS_FPR_D_GET(FS));
	MIPS_FPR_D_SET(FD, temp);
}
void mips_isa_ABS_D_impl(struct mips_ctx_t *ctx)
{
	double temp = MIPS_FPR_D_GET(FS) < 0.0 ? -MIPS_FPR_D_GET(FS) : MIPS_FPR_D_GET(FS);
	MIPS_FPR_D_SET(FD, temp);
}
void mips_isa_MOV_D_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_D_SET(FD, MIPS_FPR_D_GET(FS));
}
void mips_isa_NEG_D_impl(struct mips_ctx_t *ctx)
{
	MIPS_FPR_D_SET(FD, -MIPS_FPR_D_GET(FS));
}
void mips_isa_ROUND_L_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TRUNC_L_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CEIL_L_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_FLOOR_L_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_ROUND_W_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_TRUNC_W_D_impl(struct mips_ctx_t *ctx)
{
	double temp = MIPS_FPR_D_GET(FS);
	int itmp = (int) temp;
	MIPS_FPR_D_SET(FD, * (double *) &itmp);
}
void mips_isa_CEIL_W_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_FLOOR_W_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVF_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVT_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVZ_D_impl(struct mips_ctx_t *ctx)
{
	if (!mips_gpr_get_value(ctx,RT))
		MIPS_FPR_D_SET(FD, MIPS_FPR_D_GET(FS));
}
void mips_isa_MOVN_D_impl(struct mips_ctx_t *ctx)
{
	if (mips_gpr_get_value(ctx,RT))
		MIPS_FPR_D_SET(FD, MIPS_FPR_D_GET(FS));
}
void mips_isa_RECIP_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_RSQRT_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CVT_S_D_impl(struct mips_ctx_t *ctx)
{
	double temp = MIPS_FPR_S_GET(FS);
	MIPS_FPR_D_SET(FD, temp);
}
void mips_isa_CVT_W_D_impl(struct mips_ctx_t *ctx)
{
	float temp = MIPS_FPR_S_GET(FS);
	int wrd = * (int *) &temp;
	MIPS_FPR_D_SET(FD, (double) wrd);
}
void mips_isa_CVT_L_D_impl(struct mips_ctx_t *ctx)
{
	double temp = MIPS_FPR_D_GET(FS);
	long int dwrd = * (long int *) &temp;
	MIPS_FPR_D_SET(FD, (double) dwrd);
}
void mips_isa_C_F_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_UN_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_EQ_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_UEQ_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_OLT_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_ULT_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_OLE_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_ULE_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_SF_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_NGLE_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_SEQ_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_NGL_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_LT_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_NGE_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_LE_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_C_NGT_D_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CVT_S_W_impl(struct mips_ctx_t *ctx)
{
	int wrd = (int) MIPS_FPR_S_GET(FS);
	float temp = * (float *) &wrd;
	MIPS_FPR_S_SET(FD, temp);
}
void mips_isa_CVT_D_W_impl(struct mips_ctx_t *ctx)
{
	int wrd = (int) MIPS_FPR_D_GET(FS);
	float temp = * (float *) &wrd;
	MIPS_FPR_S_SET(FD, temp);
}
void mips_isa_ADD_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_SUB_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MUL_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_ABS_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOV_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_NEG_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVF_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVT_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVZ_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MOVN_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CVT_S_PU_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CVT_S_PL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_PLL_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_PLU_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_PUL_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_PUU_PS_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}

/* Instructions with opcode COP2=0x12 arg2 is the rs field (bits
 * [25:21]) arg3, is used for the bits nd and tf (17 and 16) */

void mips_isa_MFC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CFC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MFHC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MTC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_CTC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_MTHC2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC2F_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC2FL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC2T_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_BC2TL_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
void mips_isa_COP2_impl(struct mips_ctx_t *ctx)
{
	__MIPS_NOT_IMPL__
}
