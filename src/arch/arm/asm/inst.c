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

	offset = inst->bytes.sdtr.off;
	rn = inst->bytes.sdtr.base_rn;

	if(inst->bytes.sdtr.imm == 1)
	{
		rm = (offset & (0x0000000f));
		shift = ((offset >> 4) & (0x000000ff));

		if(inst->bytes.sdtr.up_dn)
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
	else if (inst->bytes.sdtr.imm == 0)
	{
		if(!offset)
		{
			str_printf(inst_str_ptr, inst_str_size, "[r%d]",
				rn);
		}
		else
		{
			if(inst->bytes.sdtr.up_dn)
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
		rn = inst->bytes.hfwrd_reg.base_rn;
		rm = inst->bytes.hfwrd_reg.off_reg;
		if (rm)
			str_printf(inst_str_ptr, inst_str_size, "[r%d, #%d]", rn, rm);
		else
			str_printf(inst_str_ptr, inst_str_size, "[r%d]", rn);
	}
	else if (cat == ARMInstCategoryHfwrdImm)
	{
		rn = inst->bytes.hfwrd_imm.base_rn;
		offset = (inst->bytes.hfwrd_imm.imm_off_hi << 4)
			| (inst->bytes.hfwrd_imm.imm_off_lo);
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
		rd = self->bytes.dpr.dst_reg;
	else if (cat == ARMInstCategoryDprImm)
		rd = self->bytes.dpr.dst_reg;
	else if (cat == ARMInstCategoryDprSat)
		rd = self->bytes.dpr_sat.dst_reg;
	else if (cat == ARMInstCategoryPsr)
		rd = self->bytes.psr.dst_reg;
	else if (cat == ARMInstCategoryMult)
		rd = self->bytes.mult.dst_rd;
	else if (cat == ARMInstCategoryMultSign)
		rd = self->bytes.mult.dst_rd;
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: rd fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: rd fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		rd = self->bytes.hfwrd_reg.dst_rd;
	else if (cat == ARMInstCategoryHfwrdImm)
		rd = self->bytes.hfwrd_imm.dst_rd;
	else if (cat == ARMInstCategoryBax)
		fatal("%d: rd fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		rd = self->bytes.sdtr.src_dst_rd;
	else if (cat == ARMInstCategorySdswp)
		rd = self->bytes.sngl_dswp.dst_rd;
	else if (cat == ARMInstCategoryCprRtr)
		rd = self->bytes.cpr_rtr.rd;
	else if (cat == ARMInstCategoryCprDtr)
		rd = self->bytes.cpr_dtr.cpr_sr_dst;
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
		rn = self->bytes.dpr.op1_reg;
	else if (cat == ARMInstCategoryDprImm)
		rn = self->bytes.dpr.op1_reg;
	else if (cat == ARMInstCategoryDprSat)
		rn = self->bytes.dpr_sat.op1_reg;
	else if (cat == ARMInstCategoryPsr)
		fatal("%d: rn  fmt not recognized", cat);
	else if (cat == ARMInstCategoryMult)
		rn = self->bytes.mult.op2_rn;
	else if (cat == ARMInstCategoryMultSign)
		rn = self->bytes.mult.op2_rn;
	else if (cat == ARMInstCategoryMultLn)
		fatal("%d: rn  fmt not recognized", cat);
	else if (cat == ARMInstCategoryMultLnSign)
		fatal("%d: rn fmt not recognized", cat);
	else if (cat == ARMInstCategoryHfwrdReg)
		rn = self->bytes.hfwrd_reg.base_rn;
	else if (cat == ARMInstCategoryHfwrdImm)
		rn = self->bytes.hfwrd_imm.base_rn;
	else if (cat == ARMInstCategoryBax)
		rn = self->bytes.bax.op0_rn;
	else if (cat == ARMInstCategorySdtr)
		rn = self->bytes.sdtr.base_rn;
	else if (cat == ARMInstCategoryBdtr)
		rn = self->bytes.bdtr.base_rn;
	else if (cat == ARMInstCategorySdswp)
		rn = self->bytes.sngl_dswp.base_rn;
	else if (cat == ARMInstCategoryCprRtr)
		rn = self->bytes.cpr_rtr.cpr_rn;
	else if (cat == ARMInstCategoryBrnch)
		fatal("%d: rn fmt not recognized", cat);
	else if (cat == ARMInstCategoryVfp)
		rn = self->bytes.vfp_mv.vfp_rn;

	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: rn fmt not recognized", cat);

	switch (rn)
	{
	case (r12):
		if(cat == ARMInstCategoryVfp)
		{
			if(self->bytes.vfp_mv.w)
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
		rm = self->bytes.mult.op0_rm;
	else if (cat == ARMInstCategoryMultSign)
		rm = self->bytes.mult.op0_rm;
	else if (cat == ARMInstCategoryMultLn)
		rm = self->bytes.mult_ln.op0_rm;
	else if (cat == ARMInstCategoryMultLnSign)
		rm = self->bytes.mult_ln.op0_rm;
	else if (cat == ARMInstCategoryHfwrdReg)
		rm = self->bytes.hfwrd_reg.off_reg;
	else if (cat == ARMInstCategoryHfwrdImm)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategoryBax)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategorySdtr)
		fatal("%d: rm fmt not recognized", cat);
	else if (cat == ARMInstCategorySdswp)
		rm = self->bytes.sngl_dswp.op0_rm;
	else if (cat == ARMInstCategoryCprRtr)
		rm = self->bytes.cpr_rtr.cpr_op_rm;
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
		rs = self->bytes.mult.op1_rs;
	else if (cat == ARMInstCategoryMultSign)
		rs = self->bytes.mult.op1_rs;
	else if (cat == ARMInstCategoryMultLn)
		rs = self->bytes.mult_ln.op1_rs;
	else if (cat == ARMInstCategoryMultLnSign)
		rs = self->bytes.mult_ln.op1_rs;
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
		op2 = self->bytes.dpr.op2;
	else if (cat == ARMInstCategoryDprImm)
		op2 = self->bytes.dpr.op2;
	else if (cat == ARMInstCategoryDprSat)
		op2 = self->bytes.dpr_sat.op2;
	else if (cat == ARMInstCategoryPsr)
		op2 = self->bytes.psr.op2;
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

	if (!self->bytes.dpr.imm)
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
		cond = self->bytes.dpr.cond;
	else if (cat == ARMInstCategoryDprImm)
		cond = self->bytes.dpr.cond;
	else if (cat == ARMInstCategoryDprSat)
		cond = self->bytes.dpr_sat.cond;
	else if (cat == ARMInstCategoryPsr)
		cond = self->bytes.psr.cond;
	else if (cat == ARMInstCategoryMult)
		cond = self->bytes.mult.cond;
	else if (cat == ARMInstCategoryMultSign)
		cond = self->bytes.mult.cond;
	else if (cat == ARMInstCategoryMultLn)
		cond = self->bytes.mult_ln.cond;
	else if (cat == ARMInstCategoryMultLnSign)
		cond = self->bytes.mult_ln.cond;
	else if (cat == ARMInstCategoryHfwrdReg)
		cond = self->bytes.hfwrd_reg.cond;
	else if (cat == ARMInstCategoryHfwrdImm)
		cond = self->bytes.hfwrd_imm.cond;
	else if (cat == ARMInstCategoryBax)
		cond = self->bytes.bax.cond;
	else if (cat == ARMInstCategorySdtr)
		cond = self->bytes.sdtr.cond;
	else if (cat == ARMInstCategoryBdtr)
		cond = self->bytes.bdtr.cond;
	else if (cat == ARMInstCategorySdswp)
		cond = self->bytes.sngl_dswp.cond;
	else if (cat == ARMInstCategoryCprRtr)
		cond = self->bytes.cpr_rtr.cond;
	else if (cat == ARMInstCategoryCprDtr)
		cond = self->bytes.cpr_dtr.cond;
	else if (cat == ARMInstCategoryBrnch)
		cond = self->bytes.brnch.cond;
	else if (cat == ARMInstCategorySwiSvc)
		cond = self->bytes.swi_svc.cond;
	else if (cat == ARMInstCategoryVfp)
		cond = self->bytes.vfp_mv.cond;

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
		rdlo = self->bytes.mult_ln.dst_lo;
	else if (cat == ARMInstCategoryMultLnSign)
		rdlo = self->bytes.mult_ln.dst_lo;
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
		rdhi = self->bytes.mult_ln.dst_hi;
	else if (cat == ARMInstCategoryMultLnSign)
		rdhi = self->bytes.mult_ln.dst_hi;
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
		psr = self->bytes.psr.psr_loc;
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
		op2_psr = self->bytes.psr.op2;
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

	if (self->bytes.psr.imm)
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
		idx = self->bytes.sdtr.idx_typ;
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
		if(self->bytes.sdtr.off)
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
		offset = (self->bytes.brnch.off << 2);
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
		reg_list = self->bytes.bdtr.reg_lst;
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
		immd24 = self->bytes.swi_svc.cmnt;
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
		immd16 = ((self->bytes.dpr.op1_reg << 12)
			| self->bytes.dpr.op2);
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
		copr = self->bytes.cpr_rtr.cpr_num;
	else if (cat == ARMInstCategoryCprDtr)
		copr = self->bytes.cpr_dtr.cpr_num;
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
		offset = self->bytes.cpr_dtr.off;
	/* TODO: destinations for BDTR CDTR CDO*/
	else
		fatal("%d: amode5 fmt not recognized", cat);

	rn = self->bytes.cpr_dtr.base_rn;
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
		vfp1 = self->bytes.vfp_mv.immd8;
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
		vfp1 = self->bytes.vfp_mv.immd8;
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
		immd8 = self->bytes.vfp_mv.immd8;
		reg_start = ((self->bytes.vfp_mv.d << 4)
			| (self->bytes.vfp_mv.vd)) & (0x0000001f);
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
		freg = self->bytes.cpr_dtr.cpr_sr_dst;
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
		freg = self->bytes.cpr_dtr.cpr_sr_dst;
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
		rt = self->bytes.vfp_strreg_tr.vfp_rt;
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


void ARMInstDump(ARMInst *self, FILE *f)
{
	char buf[200];

	ARMInstDumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}


void ARMInstDecode(ARMInst *self, unsigned int addr, void *buf)
{
	unsigned int arg1;
	unsigned int arg2;

	unsigned char *as_char = buf;
	unsigned int *as_word = buf;

	self->addr = addr;
	self->bytes.word = *as_word;
	arg1 = ((as_char[3] & 0x0f) << 4) | ((as_char[2] & 0xf0) >> 4);
	arg2 = ((as_char[0] & 0xf0) >> 4);
	self->info = &arm_inst_info[arg1 * 16 + arg2];
}
