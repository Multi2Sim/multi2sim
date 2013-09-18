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
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "inst.h"


/*
 * Class 'ARMInst'
 */

void ARMInstCreate(ARMInst *self, ARMAsm *as)
{
	/* Initialize */
	self->as = as;
}


void ARMInstDestroy(ARMInst *self)
{
}


unsigned int arm_rotl(unsigned int value, unsigned int shift)
{
	shift = shift * 2;
	if ((shift &= sizeof(value) * 8 - 1) == 0)
		return value;

	return (value << shift) | (value >> (sizeof(value) * 8 - shift));
}

unsigned int arm_rotr(unsigned int value, unsigned int shift)
{

	// Rotating 32 bits on a 32-bit integer is the same as rotating 0 bits; 33 bits -> 1 bit; etc.
	if (shift >= 32 || shift <= -32) {
		shift = shift % 32;
	}

	unsigned int temp = value;

	// Rotate input to the right
	value = value >> shift;

	// Build mask for carried over bits
	temp = temp << (32 - shift);

	return value | temp;
	/*
	shift = shift * 2;
	if ((shift &= sizeof(value) * 8 - 1) == 0)
		return value;

	return (value >> shift) | (value << (sizeof(value) * 8 - shift));
	*/
}

void arm_amode2_disasm(char **inst_str_ptr, int *inst_str_size,
	ARMInst *inst, ARMInstCategory cat)
{
	unsigned int rn;
	unsigned int rm;
	unsigned int shift;
	unsigned int offset;

	offset = inst->dword.sdtr.off;
	rn = inst->dword.sdtr.base_rn;

	if(inst->dword.sdtr.imm == 1)
	{
		rm = (offset & (0x0000000f));
		shift = ((offset >> 4) & (0x000000ff));

		if(inst->dword.sdtr.up_dn)
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (ARM_OPTR_LSL):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, lsl #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_LSR):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, lsr #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_ASR):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, asr #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_ROR):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, ror #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;
			}
		}
		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (ARM_OPTR_LSL):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, lsl #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_LSR):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, lsr #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_ASR):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, asr #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_ROR):
				str_printf(inst_str_ptr, inst_str_size, "[r%d, r%d, ror #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;
			}
		}
	}
	else if (inst->dword.sdtr.imm == 0)
	{
		if(!offset)
		{
			str_printf(inst_str_ptr, inst_str_size, "[r%d]",
				rn);
		}
		else
		{
			if(inst->dword.sdtr.up_dn)
				str_printf(inst_str_ptr, inst_str_size, "[r%d, #%d]",
					rn, offset);
			else
				str_printf(inst_str_ptr, inst_str_size, "[r%d, #-%d]",
					rn, offset);
		}
	}
	else
		fatal("%d: amode 2 fmt not recognized", cat);

}

void arm_amode3_disasm(char **inst_str_ptr, int *inst_str_size,
	ARMInst *inst, ARMInstCategory cat)
{
	unsigned int rn;
	unsigned int rm;
	unsigned int offset;
	if ( cat == ARMInstCategoryHfwrdReg)
	{
		rn = inst->dword.hfwrd_reg.base_rn;
		rm = inst->dword.hfwrd_reg.off_reg;
		if (rm)
			str_printf(inst_str_ptr, inst_str_size, "[r%d, #%d]", rn, rm);
		else
			str_printf(inst_str_ptr, inst_str_size, "[r%d]", rn);
	}
	else if (cat == ARMInstCategoryHfwrdImm)
	{
		rn = inst->dword.hfwrd_imm.base_rn;
		offset = (inst->dword.hfwrd_imm.imm_off_hi << 4)
			| (inst->dword.hfwrd_imm.imm_off_lo);
		if(offset)
			str_printf(inst_str_ptr, inst_str_size, "[r%d, #%d]", rn, offset);
		else
			str_printf(inst_str_ptr, inst_str_size, "[r%d]", rn);
	}
	else
		fatal("%d: amode 3 disasm fmt not recognized", cat);
}


static void ARMInstDumpBufRd(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int rd;

	if (cat == ARMInstCategoryDprReg)
		rd = self->dword.dpr.dst_reg;
	else if (cat == ARMInstCategoryDprImm)
		rd = self->dword.dpr.dst_reg;
	else if (cat == ARMInstCategoryDprSat)
		rd = self->dword.dpr_sat.dst_reg;
	else if (cat == ARMInstCategoryPsr)
		rd = self->dword.psr.dst_reg;
	else if (cat == ARMInstCategoryMult)
		rd = self->dword.mult.dst_rd;
	else if (cat == ARMInstCategoryMultSign)
		rd = self->dword.mult.dst_rd;
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: rd fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: rd fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		rd = self->dword.hfwrd_reg.dst_rd;
	else if (cat == ARMInstCategoryHfwrdImm)
		rd = self->dword.hfwrd_imm.dst_rd;
	else if (cat == ARMInstCategoryBax)
		fatal("%d: rd fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		rd = self->dword.sdtr.src_dst_rd;
	else if (cat == ARMInstCategorySdswp)
		rd = self->dword.sngl_dswp.dst_rd;
	else if (cat == ARMInstCategoryCprRtr)
		rd = self->dword.cpr_rtr.rd;
	else if (cat == ARMInstCategoryCprDtr)
		rd = self->dword.cpr_dtr.cpr_sr_dst;
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: rd fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: rd fmt not recognized", cat);

	switch (rd)
	{
	case (r13):
		str_printf(buf_ptr, size_ptr, "sp");
	break;

	case (r14):
		str_printf(buf_ptr, size_ptr, "lr");
	break;
	case (r15):

		str_printf(buf_ptr, size_ptr, "pc");
	break;

	default:
		str_printf(buf_ptr, size_ptr, "r%d", rd);
		break;
	}


}


static void ARMInstDumpBufRn(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int rn;

	if (cat == ARMInstCategoryDprReg)
		rn = self->dword.dpr.op1_reg;
	else if (cat == ARMInstCategoryDprImm)
		rn = self->dword.dpr.op1_reg;
	else if (cat == ARMInstCategoryDprSat)
		rn = self->dword.dpr_sat.op1_reg;
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: rn  fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		rn = self->dword.mult.op2_rn;
	else if (cat == ARMInstCategoryMultSign)
		rn = self->dword.mult.op2_rn;
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: rn  fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: rn fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		rn = self->dword.hfwrd_reg.base_rn;
	else if (cat == ARMInstCategoryHfwrdImm)
		rn = self->dword.hfwrd_imm.base_rn;
	else if (cat == ARMInstCategoryBax)
		rn = self->dword.bax.op0_rn;
	else if (cat == ARMInstCategorySdtr)
		rn = self->dword.sdtr.base_rn;
	else if (cat == ARMInstCategoryBdtr)
		rn = self->dword.bdtr.base_rn;
	else if (cat == ARMInstCategorySdswp)
		rn = self->dword.sngl_dswp.base_rn;
	else if (cat == ARMInstCategoryCprRtr)
		rn = self->dword.cpr_rtr.cpr_rn;
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: rn fmt not recognized", cat);
	else if (cat == ARMInstCategoryVfp)
		rn = self->dword.vfp_mv.vfp_rn;

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: rn fmt not recognized", cat);

	switch (rn)
	{
	case (r12):
		if(cat == ARMInstCategoryVfp)
		{
			if(self->dword.vfp_mv.w)
				str_printf(buf_ptr, size_ptr, "ip!");
			else
				str_printf(buf_ptr, size_ptr, "ip");
		}
		else
			str_printf(buf_ptr, size_ptr, "ip");
		break;
	case (r13):
		if(cat != ARMInstCategoryBdtr)
		{
		str_printf(buf_ptr, size_ptr, "sp");
		}
		else if (cat == ARMInstCategoryBdtr)
		{
		str_printf(buf_ptr, size_ptr, " ");
		}
		break;
	case (r14):
		str_printf(buf_ptr, size_ptr, "lr");
		break;
	case (r15):
		str_printf(buf_ptr, size_ptr, "pc");
		break;
	default:
		str_printf(buf_ptr, size_ptr, "r%d", rn);
		break;
	}
}


static void ARMInstDumpBufRm(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int rm;

	rm = 0;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		rm = self->dword.mult.op0_rm;
	else if (cat == ARMInstCategoryMultSign)
		rm = self->dword.mult.op0_rm;
	else if (cat == ARMInstCategoryMultLn)
		rm = self->dword.mult_ln.op0_rm;
	else if (cat == ARMInstCategoryMultLnSign)
		rm = self->dword.mult_ln.op0_rm;
	else if (cat == ARMInstCategoryHfwrdReg)
		rm = self->dword.hfwrd_reg.off_reg;
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		rm = self->dword.sngl_dswp.op0_rm;
	else if (cat == ARMInstCategoryCprRtr)
		rm = self->dword.cpr_rtr.cpr_op_rm;
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: rm fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: rm fmt not recognized", cat);

	switch (rm)
	{
	case (r13):
		str_printf(buf_ptr, size_ptr, "sp");
		break;
	case (r14):
		str_printf(buf_ptr, size_ptr, "lr");
		break;
	case (r15):
		str_printf(buf_ptr, size_ptr, "pc");
		break;
	default:
		str_printf(buf_ptr, size_ptr, "r%d", rm);
		break;
	}


}

static void ARMInstDumpBufRs(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int rs;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		rs = self->dword.mult.op1_rs;
	else if (cat == ARMInstCategoryMultSign)
		rs = self->dword.mult.op1_rs;
	else if (cat == ARMInstCategoryMultLn)
		rs = self->dword.mult_ln.op1_rs;
	else if (cat == ARMInstCategoryMultLnSign)
		rs = self->dword.mult_ln.op1_rs;
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: rs fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: rs fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: rs fmt not recognized", cat);

	switch (rs)
	{
	case (r13):
		str_printf(buf_ptr, size_ptr, "sp");
		break;
	case (r14):
		str_printf(buf_ptr, size_ptr, "lr");
		break;
	case (r15):
		str_printf(buf_ptr, size_ptr, "pc");
		break;
	default:
		str_printf(buf_ptr, size_ptr, "r%d", rs);
		break;
	}

}

static void ARMInstDumpBufOp2(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;

	unsigned int op2;
	unsigned int rm;
	unsigned int rs;
	unsigned int shift;
	unsigned int imm;
	unsigned int rotate;
	unsigned int imm_8r;

	if (cat == ARMInstCategoryDprReg)
		op2 = self->dword.dpr.op2;
	else if (cat == ARMInstCategoryDprImm)
		op2 = self->dword.dpr.op2;
	else if (cat == ARMInstCategoryDprSat)
		op2 = self->dword.dpr_sat.op2;
	else if (cat == ARMInstCategoryPsr)
		op2 = self->dword.psr.op2;
	else if (cat == ARMInstCategoryMult)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: op2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: op2 fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: op2 fmt not recognized", cat);

	if (!self->dword.dpr.imm)
	{
		rm = (op2 & (0x0000000f));
		shift = ((op2 >> 4) & (0x000000ff));

		if (shift & 0x00000001)
		{
			rs = (shift >> 4);
			switch ((shift >> 1) & 0x00000003)
			{
			case (ARM_OPTR_LSL):
				str_printf(buf_ptr, size_ptr, "r%d , lsl r%d", rm, rs);
			break;

			case (ARM_OPTR_LSR):
				str_printf(buf_ptr, size_ptr, "r%d , lsr r%d", rm, rs);
			break;

			case (ARM_OPTR_ASR):
				str_printf(buf_ptr, size_ptr, "r%d , asr r%d", rm, rs);
			break;

			case (ARM_OPTR_ROR):
				str_printf(buf_ptr, size_ptr, "r%d , ror r%d", rm, rs);
			break;
			}
		}

		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (ARM_OPTR_LSL):
				str_printf(buf_ptr, size_ptr, "r%d , LSL #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_LSR):
				str_printf(buf_ptr, size_ptr, "r%d , LSR #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_ASR):
				str_printf(buf_ptr, size_ptr, "r%d , ASR #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
			break;

			case (ARM_OPTR_ROR):
				str_printf(buf_ptr, size_ptr, "r%d , ROR #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
			break;
			}
		}
	}
	else
	{
		imm = (op2 & (0x000000ff));
		rotate = ((op2 >> 8) & 0x0000000f);
		imm_8r = arm_rotr( imm , rotate);
		str_printf(buf_ptr, size_ptr, "#%d   ;0x%x", imm_8r, imm_8r);
	}

}

static void ARMInstDumpBufCond(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int cond;

	if (cat == ARMInstCategoryDprReg)
		cond = self->dword.dpr.cond;
	else if (cat == ARMInstCategoryDprImm)
		cond = self->dword.dpr.cond;
	else if (cat == ARMInstCategoryDprSat)
		cond = self->dword.dpr_sat.cond;
	else if (cat == ARMInstCategoryPsr)
		cond = self->dword.psr.cond;
	else if (cat == ARMInstCategoryMult)
		cond = self->dword.mult.cond;
	else if (cat == ARMInstCategoryMultSign)
		cond = self->dword.mult.cond;
	else if (cat == ARMInstCategoryMultLn)
		cond = self->dword.mult_ln.cond;
	else if (cat == ARMInstCategoryMultLnSign)
		cond = self->dword.mult_ln.cond;
	else if (cat == ARMInstCategoryHfwrdReg)
		cond = self->dword.hfwrd_reg.cond;
	else if (cat == ARMInstCategoryHfwrdImm)
		cond = self->dword.hfwrd_imm.cond;
	else if (cat == ARMInstCategoryBax)
		cond = self->dword.bax.cond;
	else if (cat == ARMInstCategorySdtr)
		cond = self->dword.sdtr.cond;
	else if (cat == ARMInstCategoryBdtr)
		cond = self->dword.bdtr.cond;
	else if (cat == ARMInstCategorySdswp)
		cond = self->dword.sngl_dswp.cond;
	else if (cat == ARMInstCategoryCprRtr)
		cond = self->dword.cpr_rtr.cond;
	else if (cat == ARMInstCategoryCprDtr)
		cond = self->dword.cpr_dtr.cond;
	else if (cat == ARMInstCategoryBrnch)
		cond = self->dword.brnch.cond;
	else if (cat == ARMInstCategorySwiSvc)
		cond = self->dword.swi_svc.cond;
	else if (cat == ARMInstCategoryVfp)
		cond = self->dword.vfp_mv.cond;

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: cond fmt not recognized", cat);

	switch (cond)
	{
	case (EQ):
	str_printf(buf_ptr, size_ptr, "eq");
	break;

	case (NE):
	str_printf(buf_ptr, size_ptr, "ne");
	break;

	case (CS):
	str_printf(buf_ptr, size_ptr, "cs");
	break;

	case (CC):
	str_printf(buf_ptr, size_ptr, "cc");
	break;

	case (MI):
	str_printf(buf_ptr, size_ptr, "mi");
	break;

	case (PL):
	str_printf(buf_ptr, size_ptr, "pl");
	break;

	case (VS):
	str_printf(buf_ptr, size_ptr, "vs");
	break;

	case (VC):
	str_printf(buf_ptr, size_ptr, "vc");
	break;

	case (HI):
	str_printf(buf_ptr, size_ptr, "hi");
	break;

	case (LS):
	str_printf(buf_ptr, size_ptr, "ls");
	break;

	case (GE):
	str_printf(buf_ptr, size_ptr, "ge");
	break;

	case (LT):
	str_printf(buf_ptr, size_ptr, "lt");
	break;

	case (GT):
	str_printf(buf_ptr, size_ptr, "gt");
	break;

	case (LE):
	str_printf(buf_ptr, size_ptr, "le");
	break;

	case (AL):
	str_printf(buf_ptr, size_ptr, " ");
	break;

	}
}

static void ARMInstDumpBufRdlo(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int rdlo;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		rdlo = self->dword.mult_ln.dst_lo;
	else if (cat == ARMInstCategoryMultLnSign)
		rdlo = self->dword.mult_ln.dst_lo;
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: rdlo fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: rdlo fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: rdlo fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "r%d", rdlo);

}


static void ARMInstDumpBufRdhi(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int rdhi;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		rdhi = self->dword.mult_ln.dst_hi;
	else if (cat == ARMInstCategoryMultLnSign)
		rdhi = self->dword.mult_ln.dst_hi;
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: rdhi fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: rdhi fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: rdhi fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "r%d", rdhi);

}


static void ARMInstDumpBufPsr(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int psr;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		psr = self->dword.psr.psr_loc;
	else if (cat == ARMInstCategoryMult)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: psr fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: psr fmt not recognized", cat);

	switch (psr)
	{
	case (CPSR):
	str_printf(buf_ptr, size_ptr, "CPSR");
	break;

	case (SPSR):
	str_printf(buf_ptr, size_ptr, "SPSR");
	break;
	}
}


static void ARMInstDumpBufOp2Psr(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int op2_psr;
	unsigned int rotate;
	unsigned int immd_8r;
	unsigned int rm;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		op2_psr = self->dword.psr.op2;
	else if (cat == ARMInstCategoryMult)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: op2 psr fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: op2 psr fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: psr fmt not recognized", cat);

	if (self->dword.psr.imm)
	{
		rotate = ((op2_psr & 0x00000f00) >> 8);
		immd_8r = arm_rotr( op2_psr , rotate);
		str_printf(buf_ptr, size_ptr, "#%d   ;0x%x", immd_8r, immd_8r);
	}
	else
	{
		rm = (op2_psr & 0x0000000f);
		switch (rm)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
			break;
		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
			break;
		case (r15):
			str_printf(buf_ptr, size_ptr, "pc");
			break;
		default:
			str_printf(buf_ptr, size_ptr, "r%d", rm);
			break;
		}
	}
}


static void ARMInstDumpBufAMode3(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		arm_amode3_disasm(buf_ptr, size_ptr, self, cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		arm_amode3_disasm(buf_ptr, size_ptr, self, cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: amode 3 fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: amode 3 fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: amode 3 fmt not recognized", cat);
}


static void ARMInstDumpBufAMode2(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		arm_amode2_disasm(buf_ptr, size_ptr, self, cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: amode 2 fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: amode 2 fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: amode 2 fmt not recognized", cat);
}


static void ARMInstDumpBufIdx(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int idx;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		idx = self->dword.sdtr.idx_typ;
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: idx fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: idx fmt not recognized", cat);

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: idx fmt not recognized", cat);

	if (idx == 1)
	{
		if(self->dword.sdtr.off)
		str_printf(buf_ptr, size_ptr, "!");
	}
	else
		str_printf(buf_ptr, size_ptr, " ");
}


static void ARMInstDumpBufBaddr(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	signed int offset;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: brnch fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		offset = (self->dword.brnch.off << 2);
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: brnch fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "%x", self->addr + offset + 8);
}


static void ARMInstDumpBufRegs(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int reg_list;
	int i;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryBdtr)
		reg_list = self->dword.bdtr.reg_lst;
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: regs fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: regs fmt not recognized", cat);
	/* TODO: destinations for CDTR CDO*/
	else
		fatal("%d: regs fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "{");
	for (i = 1; i < 65536; i *= 2)
	{
		if(reg_list & (i))
		{
			str_printf(buf_ptr, size_ptr, "r%d ", log_base2(i));
		}
	}

	str_printf(buf_ptr, size_ptr, "}");
}


static void ARMInstDumpBufImmd24(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int immd24;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: swi_svc fmt not recognized", cat);
	else if (cat == ARMInstCategorySwiSvc)
		immd24 = self->dword.swi_svc.cmnt;
	/* TODO: destinations for CDTR CDO*/
	else
		fatal("%d: swi_svc fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "0x%x",immd24);
}


static void ARMInstDumpBufImmd16(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int immd16;

	if (cat == ARMInstCategoryDprReg)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryDprImm)
		immd16 = ((self->dword.dpr.op1_reg << 12)
			| self->dword.dpr.op2);
	else if (cat == ARMInstCategoryDprSat)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultSign)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprRtr)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: movt_movw fmt not recognized", cat);
	else if (cat == ARMInstCategorySwiSvc)
		fatal("%d: movt_movw fmt not recognized", cat);
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: movt_movw fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "#%d  ; 0x%x",immd16, immd16);
}


static void ARMInstDumpBufCopr(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int copr;

	if (cat == ARMInstCategoryCprRtr)
		copr = self->dword.cpr_rtr.cpr_num;
	else if (cat == ARMInstCategoryCprDtr)
		copr = self->dword.cpr_dtr.cpr_num;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: copr num fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "%d", copr);
}


static void ARMInstDumpBufAMode5(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int offset;
	unsigned int rn;

	if (cat == ARMInstCategoryCprRtr)
		fatal("%d: copr num fmt not recognized", cat);
	else if (cat == ARMInstCategoryCprDtr)
		offset = self->dword.cpr_dtr.off;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: amode5 fmt not recognized", cat);

	rn = self->dword.cpr_dtr.base_rn;
	if(offset)
	{
	str_printf(buf_ptr, size_ptr, "[r%d], #%d", rn, offset*4);
	}
	else
		str_printf(buf_ptr, size_ptr, "[r%d]", rn);
}


static void ARMInstDumpBufVfp1stm(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int vfp1;

	if (cat == ARMInstCategoryVfp)
		vfp1 = self->dword.vfp_mv.immd8;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: vfp1 stm fmt not recognized", cat);

	if (vfp1 % 2)
		str_printf(buf_ptr, size_ptr, "FSTMIAX");
	else
		str_printf(buf_ptr, size_ptr, "VSTMIA");
}


static void ARMInstDumpVfp1ldm(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int vfp1;

	if (cat == ARMInstCategoryVfp)
		vfp1 = self->dword.vfp_mv.immd8;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: vfp1 ldm fmt not recognized", cat);

	if(vfp1 % 2)
		str_printf(buf_ptr, size_ptr, "FLDMIAX");
	else
		str_printf(buf_ptr, size_ptr, "VLDMIA");
}


static void ARMInstDumpBufVfpRegs(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int immd8;
	unsigned int reg_start;

	if (cat == ARMInstCategoryVfp)
	{
		immd8 = self->dword.vfp_mv.immd8;
		reg_start = ((self->dword.vfp_mv.d << 4)
			| (self->dword.vfp_mv.vd)) & (0x0000001f);
	}
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: vfp regs fmt not recognized", cat);

	str_printf(buf_ptr, size_ptr, "{d%d-d%d}", reg_start,
		(reg_start + immd8/2 - 1));
}


static void ARMInstDumpBufFreg(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int freg;


	if (cat == ARMInstCategoryCprDtr)
		freg = self->dword.cpr_dtr.cpr_sr_dst;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: freg fmt not recognized", cat);

	if(freg > 7)
		str_printf(buf_ptr, size_ptr, "f%d", (freg-8));
	else
		str_printf(buf_ptr, size_ptr, "f%d", freg);
}


static void ARMInstDumpBufFp(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int freg;

	if (cat == ARMInstCategoryCprDtr)
		freg = self->dword.cpr_dtr.cpr_sr_dst;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: FP fmt not recognized", cat);

	if(freg > 7)
		str_printf(buf_ptr, size_ptr, "P");
	else
		str_printf(buf_ptr, size_ptr, "E");
}

static void ARMInstDumpBufRt(ARMInst *self, char **buf_ptr, int *size_ptr)
{
	ARMInstCategory cat = self->info->category;
	unsigned int rt;

	if (cat == ARMInstCategoryVfp)
		rt = self->dword.vfp_strreg_tr.vfp_rt;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: vfp rt fmt not recognized", cat);

	switch (rt)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
			break;
		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
			break;
		case (r15):
			str_printf(buf_ptr, size_ptr, "pc");
			break;
		default:
			str_printf(buf_ptr, size_ptr, "r%d", rt);
			break;
		}
}


void ARMInstDumpBuf(ARMInst *self, char *buf, int size)
{
	char *orig_buf = buf;
	char *fmt_str;
	int token_len;

	/* Nothing for empty format string */
	fmt_str = self->info ? self->info->fmt_str : NULL;
	if (!fmt_str || !*fmt_str)
	{
		str_printf(&buf, &size, "???\n");
		return;
	}

	/* Follow format string */
	while (*fmt_str)
	{
		if (*fmt_str != '%')
		{
			if (*fmt_str != ' ' || buf != orig_buf)
				str_printf(&buf, &size, "%c",
						*fmt_str);
			++fmt_str;
			continue;
		}

		++fmt_str;
		if (asm_is_token(fmt_str, "rd", &token_len))
			ARMInstDumpBufRd(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rn", &token_len))
			ARMInstDumpBufRn(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rm", &token_len))
			ARMInstDumpBufRm(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rs", &token_len))
			ARMInstDumpBufRs(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rt", &token_len))
			ARMInstDumpBufRt(self, &buf, &size);
		else if (asm_is_token(fmt_str, "op2", &token_len))
			ARMInstDumpBufOp2(self, &buf, &size);
		else if (asm_is_token(fmt_str, "cond", &token_len))
			ARMInstDumpBufCond(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rdlo", &token_len))
			ARMInstDumpBufRdlo(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rdhi", &token_len))
			ARMInstDumpBufRdhi(self, &buf, &size);
		else if (asm_is_token(fmt_str, "psr", &token_len))
			ARMInstDumpBufPsr(self, &buf, &size);
		else if (asm_is_token(fmt_str, "op2psr", &token_len))
			ARMInstDumpBufOp2Psr(self, &buf, &size);
		else if (asm_is_token(fmt_str, "amode3", &token_len))
			ARMInstDumpBufAMode3(self, &buf, &size);
		else if (asm_is_token(fmt_str, "amode2", &token_len))
			ARMInstDumpBufAMode2(self, &buf, &size);
		else if (asm_is_token(fmt_str, "idx", &token_len))
			ARMInstDumpBufIdx(self, &buf, &size);
		else if (asm_is_token(fmt_str, "baddr", &token_len))
			ARMInstDumpBufBaddr(self, &buf, &size);
		else if (asm_is_token(fmt_str, "regs", &token_len))
			ARMInstDumpBufRegs(self, &buf, &size);
		else if (asm_is_token(fmt_str, "immd24", &token_len))
			ARMInstDumpBufImmd24(self, &buf, &size);
		else if (asm_is_token(fmt_str, "immd16", &token_len))
			ARMInstDumpBufImmd16(self, &buf, &size);
		else if (asm_is_token(fmt_str, "copr", &token_len))
			ARMInstDumpBufCopr(self, &buf, &size);
		else if (asm_is_token(fmt_str, "amode5", &token_len))
			ARMInstDumpBufAMode5(self, &buf, &size);
		else if (asm_is_token(fmt_str, "vfp1stmia", &token_len))
			ARMInstDumpBufVfp1stm(self, &buf, &size);
		else if (asm_is_token(fmt_str, "vfp1ldmia", &token_len))
			ARMInstDumpVfp1ldm(self, &buf, &size);
		else if (asm_is_token(fmt_str, "vfpregs", &token_len))
			ARMInstDumpBufVfpRegs(self, &buf, &size);
		else if (asm_is_token(fmt_str, "freg", &token_len))
			ARMInstDumpBufFreg(self, &buf, &size);
		else if (asm_is_token(fmt_str, "fp", &token_len))
			ARMInstDumpBufFp(self, &buf, &size);

		else
			fatal("%s: token not recognized\n", fmt_str);

		fmt_str += token_len;
	}
	str_printf(&buf, &size, "\n");
}

void ARMInstThumb16DumpBufRD(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;

	unsigned int rd;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			rd = inst->dword_16.movshift_reg_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rd = inst->dword_16.addsub_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			rd = inst->dword_16.immd_oprs_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			rd = inst->dword_16.dpr_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			rd = ((inst->dword_16.high_oprs_ins.h1 << 3) |  inst->dword_16.high_oprs_ins.reg_rd);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			rd = inst->dword_16.pcldr_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rd = inst->dword_16.ldstr_reg_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rd = inst->dword_16.ldstr_exts_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			rd = inst->dword_16.ldstr_immd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			rd = inst->dword_16.ldstr_hfwrd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			rd = inst->dword_16.sp_immd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			rd = inst->dword_16.addsp_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_MISC_REV)
			rd = inst->dword_16.rev_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_CMP_T2)
			rd = (inst->dword_16.cmp_t2.N << 3 | inst->dword_16.cmp_t2.reg_rn);

		else
			fatal("%d: rd fmt not recognized", cat);

		switch (rd)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rd);
			break;
		}
}

void ARMInstThumb16DumpBufRM(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
	unsigned int rm;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			rm = inst->dword_16.movshift_reg_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rm = inst->dword_16.addsub_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			rm = inst->dword_16.dpr_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			rm = inst->dword_16.high_oprs_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rm = inst->dword_16.ldstr_reg_ins.reg_ro;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rm = inst->dword_16.ldstr_exts_ins.reg_rb;
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
		else if (cat == ARM_THUMB16_CAT_MISC_REV)
			rm = inst->dword_16.rev_ins.reg_rm;
		else if (cat == ARM_THUMB16_CAT_CMP_T2)
			rm = inst->dword_16.cmp_t2.reg_rm;
		else
			fatal("%d: rm fmt not recognized", cat);

		switch (rm)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rm);
			break;
		}
}

void ARMInstThumb16DumpBufRN(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
	unsigned int rn;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rn = inst->dword_16.addsub_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			rn = inst->dword_16.immd_oprs_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rn = inst->dword_16.ldstr_reg_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rn = inst->dword_16.ldstr_exts_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			rn = inst->dword_16.ldstr_immd_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			rn = inst->dword_16.ldstr_hfwrd_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			rn = inst->dword_16.ldm_stm_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_CBNZ)
			rn = inst->dword_16.cbnz_ins.reg_rn;
		else
			fatal("%d: rn fmt not recognized", cat);

		switch (rn)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rn);
			break;
		}
}

void ARMInstThumb16DumpBufIMMD8(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
	unsigned int inst_addr = inst->addr;
	unsigned int immd8;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			immd8 = inst->dword_16.immd_oprs_ins.offset8;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			immd8 =(inst->dword_16.pcldr_ins.immd_8 << 2);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			immd8 = 4 * inst->dword_16.sp_immd_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			immd8 = 4 * inst->dword_16.addsp_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			immd8 = 4 * inst->dword_16.sub_sp_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			immd8 = inst->dword_16.cond_br_ins.s_offset;
		else if (cat == ARM_THUMB16_CAT_MISC_UCBR)
			immd8 = inst->dword_16.br_ins.immd11;
		else if (cat == ARM_THUMB16_CAT_MISC_SVC_INS)
			immd8 = inst->dword_16.svc_ins.value;
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
			str_printf(buf_ptr, size_ptr, "%x",immd8);
		}
		else if(cat == ARM_THUMB16_CAT_MISC_UCBR)
		{
			immd8 = immd8 << 1;
			immd8 = SEXT32(immd8, 12);

			immd8 = inst_addr + 4 + immd8;
			str_printf(buf_ptr, size_ptr, "%x",immd8);
		}

		else
			str_printf(buf_ptr, size_ptr, "#%d",immd8);

}

void ARMInstThumb16DumpBufIMMD3(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
	unsigned int immd3;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			immd3 = inst->dword_16.addsub_ins.rn_imm;
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



		str_printf(buf_ptr, size_ptr, "#%d",immd3);

}

void ARMInstThumb16DumpBufIMMD5(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
	unsigned int inst_addr = inst->addr;
	unsigned int immd5;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			immd5 = inst->dword_16.movshift_reg_ins.offset;
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
			immd5 = inst->dword_16.ldstr_immd_ins.offset << 2;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			immd5 = inst->dword_16.ldstr_hfwrd_ins.offset;
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
			immd5 = inst->dword_16.cbnz_ins.immd_5;
		else
			fatal("%d: immd5 fmt not recognized", cat);

		if(cat == ARM_THUMB16_CAT_MISC_CBNZ)
		{
			if((inst_addr + 2) % 4)
				immd5 = (inst_addr + 4) + (immd5 << 1);
			else
				immd5 = (inst_addr + 2) + (immd5 << 1);

			str_printf(buf_ptr, size_ptr, "%x",immd5);
		}
		else
			str_printf(buf_ptr, size_ptr, "#%d",immd5);

}

void ARMInstThumb16DumpBufCOND(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
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
			cond = inst->dword_16.if_eq_ins.first_cond;
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			cond = inst->dword_16.cond_br_ins.cond;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			cond = inst->dword_16.if_eq_ins.first_cond;

		else
			fatal("%d: rm fmt not recognized", cat);

		switch (cond)
			{
			case (EQ):
			str_printf(buf_ptr, size_ptr, "eq");
			break;

			case (NE):
			str_printf(buf_ptr, size_ptr, "ne");
			break;

			case (CS):
			str_printf(buf_ptr, size_ptr, "cs");
			break;

			case (CC):
			str_printf(buf_ptr, size_ptr, "cc");
			break;

			case (MI):
			str_printf(buf_ptr, size_ptr, "mi");
			break;

			case (PL):
			str_printf(buf_ptr, size_ptr, "pl");
			break;

			case (VS):
			str_printf(buf_ptr, size_ptr, "vs");
			break;

			case (VC):
			str_printf(buf_ptr, size_ptr, "vc");
			break;

			case (HI):
			str_printf(buf_ptr, size_ptr, "hi");
			break;

			case (LS):
			str_printf(buf_ptr, size_ptr, "ls");
			break;

			case (GE):
			str_printf(buf_ptr, size_ptr, "ge");
			break;

			case (LT):
			str_printf(buf_ptr, size_ptr, "lt");
			break;

			case (GT):
			str_printf(buf_ptr, size_ptr, "gt");
			break;

			case (LE):
			str_printf(buf_ptr, size_ptr, "le");
			break;

			case (AL):
			str_printf(buf_ptr, size_ptr, " ");
			break;
			}

}

void ARMInstThumb16DumpBufREGS(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
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
			regs = inst->dword_16.ldm_stm_ins.reg_list;
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_PUSH_POP)
			regs = inst->dword_16.push_pop_ins.reg_list;
		else
			fatal("%d: regs fmt not recognized", cat);

		regs = (inst->dword_16.push_pop_ins.m_ext << 14) | regs;
		str_printf(buf_ptr, size_ptr, "{");
		for (i = 1; i < 65536; i *= 2)
		{
			if(regs & (i))
			{
				str_printf(buf_ptr, size_ptr, "r%d ", log_base2(i));
			}
		}

		str_printf(buf_ptr, size_ptr, "}");


}

void ARMInstThumb16DumpBufItEqX(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb16InstCategory cat = inst->info_16->cat16;
	unsigned int first_cond;
	unsigned int mask;

	if (cat == ARM_THUMB16_CAT_IF_THEN)
	{
		mask = inst->dword_16.if_eq_ins.mask;
		first_cond = inst->dword_16.if_eq_ins.first_cond;
	}
	else
		fatal("%d: x fmt not recognized", cat);

	if((mask != 0x8))
	{
		if((mask >> 3) ^ (first_cond & 1))
			str_printf(buf_ptr, size_ptr, "e");
		else
			str_printf(buf_ptr, size_ptr, "t");
	}
}

void ARMInstThumb16DumpBuf(ARMInst *self, char *buf, int size)
{

	char *orig_buf = buf;
		char *fmt_str;
		int token_len;

		/* Nothing for empty format string */
		fmt_str = self->info_16 ? self->info_16->fmt_str : NULL;
		if (!fmt_str || !*fmt_str)
		{
			str_printf(&buf, &size, "???\n");
			return;
		}

		/* Follow format string */
		while (*fmt_str)
		{
			if (*fmt_str != '%')
			{
				if (*fmt_str != ' ' || buf != orig_buf)
					str_printf(&buf, &size, "%c",
							*fmt_str);
				++fmt_str;
				continue;
			}

			++fmt_str;
			if (asm_is_token(fmt_str, "rd", &token_len))
				ARMInstThumb16DumpBufRD(self, &buf, &size);
			else if (asm_is_token(fmt_str, "rm", &token_len))
				ARMInstThumb16DumpBufRM(self, &buf, &size);
			else if (asm_is_token(fmt_str, "rn", &token_len))
				ARMInstThumb16DumpBufRN(self, &buf, &size);
			else if (asm_is_token(fmt_str, "immd8", &token_len))
				ARMInstThumb16DumpBufIMMD8(self, &buf, &size);
			else if (asm_is_token(fmt_str, "immd5", &token_len))
				ARMInstThumb16DumpBufIMMD5(self, &buf, &size);
			else if (asm_is_token(fmt_str, "immd3", &token_len))
				ARMInstThumb16DumpBufIMMD3(self, &buf, &size);
			else if (asm_is_token(fmt_str, "cond", &token_len))
				ARMInstThumb16DumpBufCOND(self, &buf, &size);
			else if (asm_is_token(fmt_str, "regs", &token_len))
				ARMInstThumb16DumpBufREGS(self, &buf, &size);
			else if (asm_is_token(fmt_str, "x", &token_len))
				ARMInstThumb16DumpBufItEqX(self, &buf, &size);
			else
				fatal("%s: token not recognized\n", fmt_str);

			fmt_str += token_len;
		}
		str_printf(&buf, &size, "\n");
}

void ARMInstThumb32DumpRD(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int rd;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			rd = inst->dword_32.data_proc_shftreg.rd;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			rd = inst->dword_32.data_proc_immd.rd;
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			rd = inst->dword_32.data_proc_immd.rd;
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			rd = inst->dword_32.ldstr_reg.rd;
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			rd = inst->dword_32.ldstr_reg.rd;
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			rd = inst->dword_32.ldstr_imm.rd;
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			rd = inst->dword_32.dproc_reg.rd;
		else if (cat == ARM_THUMB32_CAT_MULT)
			rd = inst->dword_32.mult.rd;
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			rd = inst->dword_32.bit_field.rd;

		else
			fatal("%d: rd fmt not recognized", cat);

		switch (rd)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rd);
			break;
		}

}

void ARMInstThumb32DumpRN(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int rn;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			rn = inst->dword_32.ld_st_mult.rn;
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			rn = inst->dword_32.ld_st_double.rn;
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			rn = inst->dword_32.table_branch.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			rn = inst->dword_32.data_proc_shftreg.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			rn = inst->dword_32.data_proc_immd.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			rn = inst->dword_32.data_proc_immd.rn;
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			rn = inst->dword_32.ldstr_reg.rn;
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			rn = inst->dword_32.ldstr_reg.rn;
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			rn = inst->dword_32.ldstr_imm.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			rn = inst->dword_32.dproc_reg.rn;
		else if (cat == ARM_THUMB32_CAT_MULT)
			rn = inst->dword_32.mult.rn;
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rn = inst->dword_32.mult_long.rn;
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			rn = inst->dword_32.bit_field.rn;

		else
			fatal("%d: rn fmt not recognized", cat);

		switch (rn)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rn);
			break;
		}

}

void ARMInstThumb32DumpRM(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int rm;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			rm = inst->dword_32.table_branch.rm;
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			rm = inst->dword_32.data_proc_shftreg.rm;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			rm = inst->dword_32.ldstr_reg.rm;
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			rm = inst->dword_32.ldstr_reg.rm;
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			rm = inst->dword_32.dproc_reg.rm;
		else if (cat == ARM_THUMB32_CAT_MULT)
			rm = inst->dword_32.mult.rm;
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rm = inst->dword_32.mult_long.rm;
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			fatal("%d: rm fmt not recognized", cat);

		else
			fatal("%d: rm fmt not recognized", cat);

		switch (rm)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rm);
			break;
		}

}

void ARMInstThumb32DumpRT(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int rt;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rt fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			rt = inst->dword_32.ld_st_double.rt;
		else
			fatal("%d: rt fmt not recognized", cat);

		switch (rt)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rt);
			break;
		}
}

void ARMInstThumb32DumpRT2(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int rt2;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rt fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			rt2 = inst->dword_32.ld_st_double.rt2;
		else
			fatal("%d: rt2 fmt not recognized", cat);

		switch (rt2)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rt2);
			break;
		}
}

void ARMInstThumb32DumpRA(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int ra;


		if (cat == ARM_THUMB32_CAT_MULT)
			ra = inst->dword_32.mult.ra;
		else
			fatal("%d: ra fmt not recognized", cat);

		switch (ra)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", ra);
			break;
		}
}

void ARMInstThumb32DumpRDLO(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int rdlo;


		if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rdlo = inst->dword_32.mult_long.rdlo;
		else
			fatal("%d: rdlo fmt not recognized", cat);

		switch (rdlo)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rdlo);
			break;
		}
}

void ARMInstThumb32DumpRDHI(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int rdhi;


		if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rdhi = inst->dword_32.mult_long.rdhi;
		else
			fatal("%d: rdhi fmt not recognized", cat);

		switch (rdhi)
		{
		case (r13):
			str_printf(buf_ptr, size_ptr, "sp");
		break;

		case (r14):
			str_printf(buf_ptr, size_ptr, "lr");
		break;
		case (r15):

			str_printf(buf_ptr, size_ptr, "pc");
		break;

		default:
			str_printf(buf_ptr, size_ptr, "r%d", rdhi);
			break;
		}
}

void ARMInstThumb32DumpS(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int sign;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			sign = inst->dword_32.data_proc_shftreg.sign;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			sign = inst->dword_32.data_proc_immd.sign;
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			sign = inst->dword_32.data_proc_immd.sign;
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			sign = inst->dword_32.branch.sign;
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			sign = inst->dword_32.dproc_reg.sign;
		else if (cat == ARM_THUMB32_CAT_MULT)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			fatal("%d: sign fmt not recognized", cat);

		else
			fatal("%d: sign fmt not recognized", cat);
		if(sign)
			str_printf(buf_ptr, size_ptr, "s");
}

void ARMInstThumb32dumpREGS(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int regs;
	unsigned int i;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			regs = inst->dword_32.ld_st_mult.reglist;
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			regs = inst->dword_32.push_pop.reglist;

		else
			fatal("%d: regs fmt not recognized", cat);

		str_printf(buf_ptr, size_ptr, "{");
		for (i = 1; i < 65536; i *= 2)
		{
			if(regs & (i))
			{
				str_printf(buf_ptr, size_ptr, "r%d ", log_base2(i));
			}
		}

		str_printf(buf_ptr, size_ptr, "}");

}

void ARMInstThumb32DumpSHFTREG(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int shift;
	unsigned int type;

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword_32.data_proc_shftreg.type;
		shift = (inst->dword_32.data_proc_shftreg.imm3 << 2) | (inst->dword_32.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: shft fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			str_printf(buf_ptr, size_ptr, "{lsl #%d}", shift);
		break;

		case (ARM_OPTR_LSR):
			str_printf(buf_ptr, size_ptr, "{lsr #%d}", shift);
		break;

		case (ARM_OPTR_ASR):
			str_printf(buf_ptr, size_ptr, "{asr #%d}", shift);
		break;

		case (ARM_OPTR_ROR):
			str_printf(buf_ptr, size_ptr, "{ror #%d}", shift);
		break;
		}
	}

}

void ARMInstThumb32DumpIMM12(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;

		if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			immd12 = inst->dword_32.ldstr_imm.immd12;
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			immd12 = inst->dword_32.ldstr_imm.immd12;
		else
			fatal("%d: imm12 fmt not recognized", cat);

		if(inst->dword_32.ldstr_imm.add)
			str_printf(buf_ptr, size_ptr, "#%d",immd12);
		else
		{
			idx = (immd12 & 0x00000400) >> 10;
			add = (immd12 & 0x00000200) >> 9;
			wback = (immd12 & 0x00000100) >> 8;
			if(add)
			{
				if(idx == 1 && wback == 0)
					str_printf(buf_ptr, size_ptr, "[#%d]",(immd12 & 0x000000ff));
				else if (idx == 1 && wback == 1)
					str_printf(buf_ptr, size_ptr, "[#%d]!",(immd12 & 0x000000ff));
				else if (idx == 0 && wback == 1)
					str_printf(buf_ptr, size_ptr, "#%d",(immd12 & 0x000000ff));
			}
			else
			{
				if(idx == 1 && wback == 0)
					str_printf(buf_ptr, size_ptr, "[#-%d]",(immd12 & 0x000000ff));
				else if (idx == 1 && wback == 1)
					str_printf(buf_ptr, size_ptr, "[#-%d]!",(immd12 & 0x000000ff));
				else if (idx == 0 && wback == 1)
					str_printf(buf_ptr, size_ptr, "#-%d",(immd12 & 0x000000ff));

			}
		}

}

void ARMInstThumb32DumpIMMD12(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int imm4;
	unsigned int imm5;
	unsigned int shft;
	unsigned int const_val;

		if (cat == ARM_THUMB32_CAT_DPR_IMM)
		{
			immd8 = inst->dword_32.data_proc_immd.immd8;
			immd3 = inst->dword_32.data_proc_immd.immd3;
			i = inst->dword_32.data_proc_immd.i_flag;
		}
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
		{
			immd8 = inst->dword_32.data_proc_immd.immd8;
			immd3 = inst->dword_32.data_proc_immd.immd3;
			i = inst->dword_32.data_proc_immd.i_flag;
		}
		else
			fatal("%d: immd12 fmt not recognized", cat);

		imm4 = (i << 3) | (immd3);

		if(imm4 < 4)
		{
			switch(imm4)
			{
			case(0) :
				const_val =  immd8;
			break;

			case(1) :
				const_val = (immd8 << 16) | immd8;
			break;

			case(2) :
				const_val = (immd8 << 24) | (immd8 << 8);
			break;

			case(3) :
				const_val = (immd8 << 24) | (immd8 << 16) | (immd8 << 8) | immd8;
			break;

			}
		}
		else
		{
			imm5 = (imm4 << 1) | ((0x00000008 & immd8) >> 8);

			const_val = (immd8 << 24) | 0x10000000;
			shft = (imm5 - 8);

			const_val = (const_val >> shft);
		}

		str_printf(buf_ptr, size_ptr, "#%d", const_val);
}

void ARMInstThumb32DumpIMMD8(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int immd8;

		if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			immd8 = (inst->dword_32.ld_st_double.immd8 << 2);
		else
			fatal("%d: immd12 fmt not recognized", cat);


		if(immd8)
		{
			if(inst->dword_32.ld_st_double.add_sub)
			{
				if(inst->dword_32.ld_st_double.index == 1 && inst->dword_32.ld_st_double.wback == 0)
					str_printf(buf_ptr, size_ptr, "#%d",(immd8));
				else if (inst->dword_32.ld_st_double.index == 1 && inst->dword_32.ld_st_double.wback == 1)
					str_printf(buf_ptr, size_ptr, "#%d!",(immd8));
				else if (inst->dword_32.ld_st_double.index == 0 && inst->dword_32.ld_st_double.wback == 0)
					str_printf(buf_ptr, size_ptr, "#%d",(immd8));
			}
			else
			{
				if(inst->dword_32.ld_st_double.index == 1 && inst->dword_32.ld_st_double.wback == 0)
					str_printf(buf_ptr, size_ptr, "#-%d",(immd8));
				else if (inst->dword_32.ld_st_double.index == 1 && inst->dword_32.ld_st_double.wback == 1)
					str_printf(buf_ptr, size_ptr, "#-%d!",(immd8));
				else if (inst->dword_32.ld_st_double.index == 0 && inst->dword_32.ld_st_double.wback == 1)
					str_printf(buf_ptr, size_ptr, "#-%d",(immd8));

			}
		}

}

void ARMInstThumb32DumpIMM2(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int immd2;


		if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
		{
			immd2 = inst->dword_32.ldstr_reg.immd2;
		}
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
		{
			immd2 = inst->dword_32.ldstr_reg.immd2;
		}
		else
			fatal("%d: imm2 fmt not recognized", cat);

		str_printf(buf_ptr, size_ptr, "#%d", immd2);
}

void ARMInstThumb32DumpCOND(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int cond;

	if (cat == ARM_THUMB32_CAT_BRANCH_COND)
		{
			cond = inst->dword_32.branch.cond;
		}
		else
			fatal("%d: cond fmt not recognized", cat);

		switch (cond)
			{
			case (EQ):
			str_printf(buf_ptr, size_ptr, "eq");
			break;

			case (NE):
			str_printf(buf_ptr, size_ptr, "ne");
			break;

			case (CS):
			str_printf(buf_ptr, size_ptr, "cs");
			break;

			case (CC):
			str_printf(buf_ptr, size_ptr, "cc");
			break;

			case (MI):
			str_printf(buf_ptr, size_ptr, "mi");
			break;

			case (PL):
			str_printf(buf_ptr, size_ptr, "pl");
			break;

			case (VS):
			str_printf(buf_ptr, size_ptr, "vs");
			break;

			case (VC):
			str_printf(buf_ptr, size_ptr, "vc");
			break;

			case (HI):
			str_printf(buf_ptr, size_ptr, "hi");
			break;

			case (LS):
			str_printf(buf_ptr, size_ptr, "ls");
			break;

			case (GE):
			str_printf(buf_ptr, size_ptr, "ge");
			break;

			case (LT):
			str_printf(buf_ptr, size_ptr, "lt");
			break;

			case (GT):
			str_printf(buf_ptr, size_ptr, "gt");
			break;

			case (LE):
			str_printf(buf_ptr, size_ptr, "le");
			break;

			case (AL):
			str_printf(buf_ptr, size_ptr, " ");
			break;
			}

}



void ARMInstThumb32DumpLSB(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int immd2;
	unsigned int immd3;


		if (cat == ARM_THUMB32_CAT_BIT_FIELD)
		{
			immd2 = inst->dword_32.bit_field.immd2;
			immd3 = inst->dword_32.bit_field.immd3;
		}
		else
			fatal("%d: imm2 fmt not recognized", cat);


		str_printf(buf_ptr, size_ptr, "#%d", ((immd3 << 2) | immd2));
}

void ARMInstThumb32DumpWID(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int msb;
	unsigned int immd2;
	unsigned int immd3;
	unsigned int lsb;



		if (cat == ARM_THUMB32_CAT_BIT_FIELD)
		{
			msb = inst->dword_32.bit_field.msb;
			immd2 = inst->dword_32.bit_field.immd2;
			immd3 = inst->dword_32.bit_field.immd3;
		}
		else
			fatal("%d: imm2 fmt not recognized", cat);

		lsb = (immd3 << 2) | immd2;
		str_printf(buf_ptr, size_ptr, "#%d", (msb - lsb + 1));
}

void ARMInstThumb32DumpIMMD16(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int immd16;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int immd4;


		if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
		{
			immd8 = inst->dword_32.data_proc_immd.immd8;
			immd3 = inst->dword_32.data_proc_immd.immd3;
			i = inst->dword_32.data_proc_immd.i_flag;
			immd4 = inst->dword_32.data_proc_immd.rn;
		}

		else
			fatal("%d: immd16 fmt not recognized", cat);

		immd16 = (immd4 << 12) | (i << 11) | (immd3 << 8) | immd8;

		str_printf(buf_ptr, size_ptr, "#%d	; 0x%x", immd16, immd16);
}

void ARMInstThumb32DumpADDR(ARMInst *inst, char **buf_ptr, int *size_ptr)
{
	ARMThumb32InstCategory cat = inst->info_32->cat32;
	unsigned int addr;
	unsigned int inst_addr = inst->addr;
	addr = 0;
		if (cat == ARM_THUMB32_CAT_BRANCH)
		{
			addr = (inst->dword_32.branch_link.sign << 24)
			| ((!(inst->dword_32.branch.j1 ^ inst->dword_32.branch_link.sign)) << 23)
			| ((!(inst->dword_32.branch.j2 ^ inst->dword_32.branch_link.sign)) << 22)
			| (inst->dword_32.branch_link.immd10 << 12)
			| (inst->dword_32.branch_link.immd11 << 1);
			addr = SEXT32(addr,25);
		}
		else if (cat == ARM_THUMB32_CAT_BRANCH_LX)
		{
			addr = (inst->dword_32.branch_link.sign << 24)
			| ((!(inst->dword_32.branch.j1 ^ inst->dword_32.branch_link.sign)) << 23)
			| ((!(inst->dword_32.branch.j2 ^ inst->dword_32.branch_link.sign)) << 22)
			| (inst->dword_32.branch_link.immd10 << 12)
			| ((inst->dword_32.branch_link.immd11 & 0xfffffffe) << 1);
			addr = SEXT32(addr,25);
		}
		else if (cat == ARM_THUMB32_CAT_BRANCH_COND)
		{
			addr = (inst->dword_32.branch.sign << 20)
			| (((inst->dword_32.branch.j2)) << 19)
			| (((inst->dword_32.branch.j1)) << 18)
			| (inst->dword_32.branch.immd6 << 12)
			| (inst->dword_32.branch.immd11 << 1);
			addr = SEXT32(addr,21);
		}
		else
			fatal("%d: addr fmt not recognized", cat);

		/* FIXME : Changed from +4 to +2 */
		addr = (inst_addr + 2) + (addr);
		str_printf(buf_ptr, size_ptr, "#%d	; 0x%x", addr, addr);
}


void ARMInstThumb32DumpBuf(ARMInst *self, char *buf, int size)
{

	char *orig_buf = buf;
	char *fmt_str;
	int token_len;

	/* Nothing for empty format string */
	fmt_str = self->info_32 ? self->info_32->fmt_str : NULL;
	if (!fmt_str || !*fmt_str)
	{
		str_printf(&buf, &size, "???\n");
		return;
	}

	/* Follow format string */
	while (*fmt_str)
	{
		if (*fmt_str != '%')
		{
			if (*fmt_str != ' ' || buf != orig_buf)
				str_printf(&buf, &size, "%c",
					*fmt_str);
			++fmt_str;
			continue;
		}


		++fmt_str;
		if (asm_is_token(fmt_str, "rd", &token_len))
			ARMInstThumb32DumpRD(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rn", &token_len))
			ARMInstThumb32DumpRN(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rm", &token_len))
			ARMInstThumb32DumpRM(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rt", &token_len))
			ARMInstThumb32DumpRT(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rt2", &token_len))
			ARMInstThumb32DumpRT2(self, &buf, &size);
		else if (asm_is_token(fmt_str, "ra", &token_len))
			ARMInstThumb32DumpRA(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rdlo", &token_len))
			ARMInstThumb32DumpRDLO(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rdhi", &token_len))
			ARMInstThumb32DumpRDHI(self, &buf, &size);
		else if (asm_is_token(fmt_str, "imm12", &token_len))
			ARMInstThumb32DumpIMM12(self, &buf, &size);
		else if (asm_is_token(fmt_str, "imm8", &token_len))
			ARMInstThumb32DumpIMM12(self, &buf, &size);
		else if (asm_is_token(fmt_str, "imm2", &token_len))
			ARMInstThumb32DumpIMM2(self, &buf, &size);
		else if (asm_is_token(fmt_str, "immd8", &token_len))
			ARMInstThumb32DumpIMMD8(self, &buf, &size);
		else if (asm_is_token(fmt_str, "immd12", &token_len))
			ARMInstThumb32DumpIMMD12(self, &buf, &size);
		else if (asm_is_token(fmt_str, "immd16", &token_len))
			ARMInstThumb32DumpIMMD16(self, &buf, &size);
		else if (asm_is_token(fmt_str, "addr", &token_len))
			ARMInstThumb32DumpADDR(self, &buf, &size);
		else if (asm_is_token(fmt_str, "regs", &token_len))
			ARMInstThumb32dumpREGS(self, &buf, &size);
		else if (asm_is_token(fmt_str, "shft", &token_len))
			ARMInstThumb32DumpSHFTREG(self, &buf, &size);
		else if (asm_is_token(fmt_str, "S", &token_len))
			ARMInstThumb32DumpS(self, &buf, &size);
		else if (asm_is_token(fmt_str, "lsb", &token_len))
			ARMInstThumb32DumpLSB(self, &buf, &size);
		else if (asm_is_token(fmt_str, "wid", &token_len))
			ARMInstThumb32DumpWID(self, &buf, &size);
		else if (asm_is_token(fmt_str, "cond", &token_len))
			ARMInstThumb32DumpCOND(self, &buf, &size);



		else
			fatal("%s: token not recognized\n", fmt_str);

		fmt_str += token_len;
	}
	str_printf(&buf, &size, "\n");
}


void ARMInstDump(ARMInst *self, FILE *f)
{
	char buf[200];

	ARMInstDumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}

void ARMInstThumb16Dump(ARMInst *self, FILE *f)
{
	char buf[200];

	ARMInstThumb16DumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}

void ARMInstThumb32Dump(ARMInst *self, FILE *f)
{
	char buf[200];

	ARMInstThumb32DumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}

void ARMThumb32InstTableDecode(volatile ARMInst *inst, ARMAsm *as)
{
	struct arm_thumb32_inst_info_t *current_table;
	/* We initially start with the first table mips_asm_table, with the opcode field as argument */
	current_table = as->arm_thumb32_asm_table;
	int current_table_low = 27;
	int current_table_high = 28;
	unsigned int thumb32_table_arg;
	int loop_iteration = 0;

	thumb32_table_arg =  BITS32(*(unsigned int*)inst->dword_32.bytes, current_table_high, current_table_low);

	/* Find next tables if the instruction belongs to another table */
	while (1) {
		if (current_table[thumb32_table_arg].next_table && loop_iteration < 8) {
			current_table_high = current_table[thumb32_table_arg].next_table_high;
			current_table_low = current_table[thumb32_table_arg].next_table_low;
			current_table = current_table[thumb32_table_arg].next_table;
			thumb32_table_arg = BITS32(*(unsigned int*)inst->dword_32.bytes, current_table_high, current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 8) {
			fatal("Can not find the correct table containing the instruction\n");
		}
		else
			break;

	}

	inst->info_32 = &current_table[thumb32_table_arg];
}

void ARMThumb16InstTableDecode(volatile ARMInst *inst, ARMAsm *as)
{
	struct arm_thumb16_inst_info_t *current_table;
	/* We initially start with the first table mips_asm_table, with the opcode field as argument */
	current_table = as->arm_thumb16_asm_table;
	int current_table_low = 14;
	int current_table_high = 15;
	unsigned int thumb16_table_arg;
	int loop_iteration = 0;

	thumb16_table_arg =  BITS16(*(unsigned short*)inst->dword_16.bytes, current_table_high, current_table_low);

	/* Find next tables if the instruction belongs to another table */
	while (1) {
		if (current_table[thumb16_table_arg].next_table && loop_iteration < 6) {
			current_table_high = current_table[thumb16_table_arg].next_table_high;
			current_table_low = current_table[thumb16_table_arg].next_table_low;
			current_table = current_table[thumb16_table_arg].next_table;
			thumb16_table_arg = BITS16(*(unsigned short*)inst->dword_16.bytes, current_table_high, current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 6) {
			fatal("Can not find the correct table containing the instruction\n");
		}
		else

			break;

	}

	inst->info_16 = &current_table[thumb16_table_arg];
}

void ARMInstDecode(volatile ARMInst *self, unsigned int addr, void *buf)
{

	unsigned int arg1;
	unsigned int arg2;

	unsigned char *as_char = buf;
	unsigned int *as_word = buf;

	self->addr = addr;
	self->dword.word = *as_word;
	arg1 = ((as_char[3] & 0x0f) << 4) | ((as_char[2] & 0xf0) >> 4);
	arg2 = ((as_char[0] & 0xf0) >> 4);
	self->info = &arm_inst_info[arg1 * 16 + arg2];
}

void ARMInstThumb16Decode(void *buf, unsigned int ip, volatile ARMInst *inst)
{
	ARMAsm *as = inst->as;
	unsigned int byte_index;
	inst->addr = ip;
	for (byte_index = 0; byte_index < 2; ++byte_index)
		inst->dword_16.bytes[byte_index] = *(unsigned char *) (buf + byte_index);

	ARMThumb16InstTableDecode(inst, as);
}

void ARMInstThumb32Decode(void *buf, unsigned int ip, volatile ARMInst *inst)
{
	ARMAsm *as = inst->as;
	unsigned int byte_index;
	inst->addr = ip - 2;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst->dword_32.bytes[byte_index] = *(unsigned char *) (buf
			+ ((byte_index + 2) % 4));


	ARMThumb32InstTableDecode(inst, as);
}
