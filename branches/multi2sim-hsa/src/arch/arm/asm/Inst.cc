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

#include <fstream>
#include <iomanip>
#include <iostream>

#include <arch/common/Asm.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Asm.h"
#include "Inst.h"


namespace ARM
{

Inst::Inst()
{
	// Initialize
	this->as = Asm::getInstance();
	addr = 0;
	info = NULL;
	info_16 = NULL;
	info_32 = NULL;
}


unsigned int Inst::Rotl(unsigned int value, int shift)
{
	shift = shift * 2;
	if ((shift &= sizeof(value) * 8 - 1) == 0)
		return value;
	return (value << shift) | (value >> (sizeof(value) * 8 - shift));
}


unsigned int Inst::Rotr(unsigned int value, int shift)
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
}


void Inst::Amode2Disasm(std::ostream &os, InstCategory cat)
{
	unsigned int rn;
	unsigned int rm;
	unsigned int shift;
	unsigned int offset;

	offset = this->dword.sdtr.off;
	rn = this->dword.sdtr.base_rn;

	if (this->dword.sdtr.imm == 1)
	{
		rm = (offset & (0x0000000f));
		shift = ((offset >> 4) & (0x000000ff));

		if (this->dword.sdtr.up_dn)
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (AsmShiftOperatorLsl):
				os << misc::fmt("[r%d, r%d, lsl #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (AsmShiftOperatorLsr):
				os << misc::fmt("[r%d, r%d, lsr #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (AsmShiftOperatorAsr):
				os << misc::fmt("[r%d, r%d, asr #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (AsmShiftOperatorRor):
				os << misc::fmt("[r%d, r%d, ror #%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;
			}
		}
		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (AsmShiftOperatorLsl):
				os << misc::fmt("[r%d, r%d, lsl #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (AsmShiftOperatorLsr):
				os << misc::fmt("[r%d, r%d, lsr #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (AsmShiftOperatorAsr):
				os << misc::fmt("[r%d, r%d, asr #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;

			case (AsmShiftOperatorRor):
				os << misc::fmt("[r%d, r%d, ror #-%d]",
					rn, rm, ((shift >> 3) & 0x0000001f));
			break;
			}
		}
	}
	else if (this->dword.sdtr.imm == 0)
	{
		if (!offset)
		{
			os << misc::fmt("[r%d]",
				rn);
		}
		else
		{
			if (this->dword.sdtr.up_dn)
				os << misc::fmt("[r%d, #%d]", rn, offset);
			else
				os << misc::fmt("[r%d, #-%d]", rn, offset);
		}
	}
	else
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));

}


void Inst::Amode3Disasm(std::ostream &os, InstCategory cat)
{
	unsigned int rn;
	unsigned int rm;
	unsigned int offset;
	if (cat == InstCategoryHfwrdReg)
	{
		rn = this->dword.hfwrd_reg.base_rn;
		rm = this->dword.hfwrd_reg.off_reg;
		if (rm)
			os << misc::fmt("[r%d, #%d]", rn, rm);
		else
			os << misc::fmt("[r%d]", rn);
	}
	else if (cat == InstCategoryHfwrdImm)
	{
		rn = this->dword.hfwrd_imm.base_rn;
		offset = (this->dword.hfwrd_imm.imm_off_hi << 4)
			| (this->dword.hfwrd_imm.imm_off_lo);
		if (offset)
			os << misc::fmt("[r%d, #%d]", rn, offset);
		else
			os << misc::fmt("[r%d]", rn);
	}
	else
		throw misc::Panic(misc::fmt("%d: amode 3 disasm fmt not recognized", cat));
}


void Inst::DumpRd(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int rd;

	if (cat == InstCategoryDprReg)
		rd = this->dword.dpr.dst_reg;
	else if (cat == InstCategoryDprImm)
		rd = this->dword.dpr.dst_reg;
	else if (cat == InstCategoryDprSat)
		rd = this->dword.dpr_sat.dst_reg;
	else if (cat == InstCategoryPsr)
		rd = this->dword.psr.dst_reg;
	else if (cat == InstCategoryMult)
		rd = this->dword.mult.dst_rd;
	else if (cat == InstCategoryMultSign)
		rd = this->dword.mult.dst_rd;
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		rd = this->dword.hfwrd_reg.dst_rd;
	else if (cat == InstCategoryHfwrdImm)
		rd = this->dword.hfwrd_imm.dst_rd;
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		rd = this->dword.sdtr.src_dst_rd;
	else if (cat == InstCategorySdswp)
		rd = this->dword.sngl_dswp.dst_rd;
	else if (cat == InstCategoryCprRtr)
		rd = this->dword.cpr_rtr.rd;
	else if (cat == InstCategoryCprDtr)
		rd = this->dword.cpr_dtr.cpr_sr_dst;
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));

	switch (rd)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;
	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rd);
		break;
	}


}


void Inst::DumpRn(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int rn;

	if (cat == InstCategoryDprReg)
		rn = this->dword.dpr.op1_reg;
	else if (cat == InstCategoryDprImm)
		rn = this->dword.dpr.op1_reg;
	else if (cat == InstCategoryDprSat)
		rn = this->dword.dpr_sat.op1_reg;
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: rn  fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		rn = this->dword.mult.op2_rn;
	else if (cat == InstCategoryMultSign)
		rn = this->dword.mult.op2_rn;
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: rn  fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		rn = this->dword.hfwrd_reg.base_rn;
	else if (cat == InstCategoryHfwrdImm)
		rn = this->dword.hfwrd_imm.base_rn;
	else if (cat == InstCategoryBax)
		rn = this->dword.bax.op0_rn;
	else if (cat == InstCategorySdtr)
		rn = this->dword.sdtr.base_rn;
	else if (cat == InstCategoryBdtr)
		rn = this->dword.bdtr.base_rn;
	else if (cat == InstCategorySdswp)
		rn = this->dword.sngl_dswp.base_rn;
	else if (cat == InstCategoryCprRtr)
		rn = this->dword.cpr_rtr.cpr_rn;
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstCategoryVfp)
		rn = this->dword.vfp_mv.vfp_rn;

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));

	switch (rn)
	{
	case (AsmUserRegistersR12):

		if (cat == InstCategoryVfp)
		{
			if(this->dword.vfp_mv.w)
				os << misc::fmt("ip!");
			else
				os << misc::fmt("ip");
		}
		else
			os << misc::fmt("ip");
		break;

	case (AsmUserRegistersR13):

		if (cat != InstCategoryBdtr)
		{
			os << misc::fmt("sp");
		}
		else if (cat == InstCategoryBdtr)
		{
			os << misc::fmt(" ");
		}
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rn);
		break;
	}
}


void Inst::DumpRm(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int rm = 0;
	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		rm = this->dword.mult.op0_rm;
	else if (cat == InstCategoryMultSign)
		rm = this->dword.mult.op0_rm;
	else if (cat == InstCategoryMultLn)
		rm = this->dword.mult_ln.op0_rm;
	else if (cat == InstCategoryMultLnSign)
		rm = this->dword.mult_ln.op0_rm;
	else if (cat == InstCategoryHfwrdReg)
		rm = this->dword.hfwrd_reg.off_reg;
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		rm = this->dword.sngl_dswp.op0_rm;
	else if (cat == InstCategoryCprRtr)
		rm = this->dword.cpr_rtr.cpr_op_rm;
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));

	switch (rm)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rm);
		break;
	}
}

void Inst::DumpRs(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int rs;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		rs = this->dword.mult.op1_rs;
	else if (cat == InstCategoryMultSign)
		rs = this->dword.mult.op1_rs;
	else if (cat == InstCategoryMultLn)
		rs = this->dword.mult_ln.op1_rs;
	else if (cat == InstCategoryMultLnSign)
		rs = this->dword.mult_ln.op1_rs;
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: rs fmt not recognized", cat));

	switch (rs)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rs);
		break;
	}
}


void Inst::DumpOp2(std::ostream &os)
{
	InstCategory cat = this->info->category;

	unsigned int op2;
	unsigned int rm;
	unsigned int rs;
	unsigned int shift;
	unsigned int imm;
	unsigned int rotate;
	unsigned int imm_8r;

	if (cat == InstCategoryDprReg)
		op2 = this->dword.dpr.op2;
	else if (cat == InstCategoryDprImm)
		op2 = this->dword.dpr.op2;
	else if (cat == InstCategoryDprSat)
		op2 = this->dword.dpr_sat.op2;
	else if (cat == InstCategoryPsr)
		op2 = this->dword.psr.op2;
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: op2 fmt not recognized", cat));

	if (!this->dword.dpr.imm)
	{
		rm = (op2 & (0x0000000f));
		shift = ((op2 >> 4) & (0x000000ff));

		if (shift & 0x00000001)
		{
			rs = (shift >> 4);
			switch ((shift >> 1) & 0x00000003)
			{
			case (AsmShiftOperatorLsl):

				os << misc::fmt("r%d , lsl r%d", rm, rs);
				break;

			case (AsmShiftOperatorLsr):

				os << misc::fmt("r%d , lsr r%d", rm, rs);
				break;

			case (AsmShiftOperatorAsr):

				os << misc::fmt("r%d , asr r%d", rm, rs);
				break;

			case (AsmShiftOperatorRor):

				os << misc::fmt("r%d , ror r%d", rm, rs);
				break;
			}
		}

		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case (AsmShiftOperatorLsl):

				os << misc::fmt("r%d , LSL #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
				break;

			case (AsmShiftOperatorLsr):

				os << misc::fmt("r%d , LSR #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
				break;

			case (AsmShiftOperatorAsr):

				os << misc::fmt("r%d , ASR #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
				break;

			case (AsmShiftOperatorRor):

				os << misc::fmt("r%d , ROR #%d   ;0x%x",
					rm, ((shift >> 3) & 0x0000001f),((shift >> 3) & 0x0000001f));
				break;
			}
		}
	}
	else
	{
		imm = (op2 & (0x000000ff));
		rotate = ((op2 >> 8) & 0x0000000f);
		imm_8r = Rotr( imm , rotate);
		os << misc::fmt("#%d   ;0x%x", imm_8r, imm_8r);
	}

}


void Inst::DumpCond(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int cond;

	if (cat == InstCategoryDprReg)
		cond = this->dword.dpr.cond;
	else if (cat == InstCategoryDprImm)
		cond = this->dword.dpr.cond;
	else if (cat == InstCategoryDprSat)
		cond = this->dword.dpr_sat.cond;
	else if (cat == InstCategoryPsr)
		cond = this->dword.psr.cond;
	else if (cat == InstCategoryMult)
		cond = this->dword.mult.cond;
	else if (cat == InstCategoryMultSign)
		cond = this->dword.mult.cond;
	else if (cat == InstCategoryMultLn)
		cond = this->dword.mult_ln.cond;
	else if (cat == InstCategoryMultLnSign)
		cond = this->dword.mult_ln.cond;
	else if (cat == InstCategoryHfwrdReg)
		cond = this->dword.hfwrd_reg.cond;
	else if (cat == InstCategoryHfwrdImm)
		cond = this->dword.hfwrd_imm.cond;
	else if (cat == InstCategoryBax)
		cond = this->dword.bax.cond;
	else if (cat == InstCategorySdtr)
		cond = this->dword.sdtr.cond;
	else if (cat == InstCategoryBdtr)
		cond = this->dword.bdtr.cond;
	else if (cat == InstCategorySdswp)
		cond = this->dword.sngl_dswp.cond;
	else if (cat == InstCategoryCprRtr)
		cond = this->dword.cpr_rtr.cond;
	else if (cat == InstCategoryCprDtr)
		cond = this->dword.cpr_dtr.cond;
	else if (cat == InstCategoryBrnch)
		cond = this->dword.brnch.cond;
	else if (cat == InstCategorySwiSvc)
		cond = this->dword.swi_svc.cond;
	else if (cat == InstCategoryVfp)
		cond = this->dword.vfp_mv.cond;

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));

	switch (cond)
	{
	case (AsmConditionCodesEQ):

		os << misc::fmt("eq");
		break;

	case (AsmConditionCodesNE):

		os << misc::fmt("ne");
		break;

	case (AsmConditionCodesCS):

		os << misc::fmt("cs");
		break;

	case (AsmConditionCodesCC):

		os << misc::fmt("cc");
		break;

	case (AsmConditionCodesMI):

		os << misc::fmt("mi");
		break;

	case (AsmConditionCodesPL):

		os << misc::fmt("pl");
		break;

	case (AsmConditionCodesVS):

		os << misc::fmt("vs");
		break;

	case (AsmConditionCodesVC):

		os << misc::fmt("vc");
		break;

	case (AsmConditionCodesHI):

		os << misc::fmt("hi");
		break;

	case (AsmConditionCodesLS):

		os << misc::fmt("ls");
		break;

	case (AsmConditionCodesGE):

		os << misc::fmt("ge");
		break;

	case (AsmConditionCodesLT):

		os << misc::fmt("lt");
		break;

	case (AsmConditionCodesGT):

		os << misc::fmt("gt");
		break;

	case (AsmConditionCodesLE):

		os << misc::fmt("le");
		break;

	case (AsmConditionCodesAL):

		os << misc::fmt(" ");
		break;

	}
}


void Inst::DumpRdlo(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int rdlo;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		rdlo = this->dword.mult_ln.dst_lo;
	else if (cat == InstCategoryMultLnSign)
		rdlo = this->dword.mult_ln.dst_lo;
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));

	os << misc::fmt("r%d", rdlo);

}


void Inst::DumpRdhi(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int rdhi;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		rdhi = this->dword.mult_ln.dst_hi;
	else if (cat == InstCategoryMultLnSign)
		rdhi = this->dword.mult_ln.dst_hi;
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));

	os << misc::fmt("r%d", rdhi);

}


void Inst::DumpPsr(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int psr;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		psr = this->dword.psr.psr_loc;
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));

	switch (psr)
	{
	case (AsmPsrRegistersCPSR):

		os << misc::fmt("CPSR");
		break;

	case (AsmPsrRegistersSPSR):

		os << misc::fmt("SPSR");
		break;
	}
}


void Inst::DumpOp2Psr(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int op2_psr;
	unsigned int rotate;
	unsigned int immd_8r;
	unsigned int rm;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		op2_psr = this->dword.psr.op2;
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: op2 psr fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: psr fmt not recognized", cat));

	if (this->dword.psr.imm)
	{
		rotate = ((op2_psr & 0x00000f00) >> 8);
		immd_8r = Rotr( op2_psr , rotate);
		os << misc::fmt("#%d   ;0x%x", immd_8r, immd_8r);
	}
	else
	{
		rm = (op2_psr & 0x0000000f);
		switch (rm)
		{
		case (AsmUserRegistersR13):

			os << misc::fmt("sp");
			break;

		case (AsmUserRegistersR14):

			os << misc::fmt("lr");
			break;

		case (AsmUserRegistersR15):

			os << misc::fmt("pc");
			break;

		default:

			os << misc::fmt("r%d", rm);
			break;
		}
	}
}


void Inst::DumpAMode3(std::ostream &os)
{
	InstCategory cat = this->info->category;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		Amode3Disasm(os, cat);
	else if (cat == InstCategoryHfwrdImm)
		Amode3Disasm(os, cat);
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: amode 3 fmt not recognized", cat));
}


void Inst::DumpAMode2(std::ostream &os)
{
	InstCategory cat = this->info->category;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		Amode2Disasm(os, cat);
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: amode 2 fmt not recognized", cat));
}


void Inst::DumpIdx(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int idx;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		idx = this->dword.sdtr.idx_typ;
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));

	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: idx fmt not recognized", cat));

	if (idx == 1)
	{
		if(this->dword.sdtr.off)
			os << misc::fmt("!");
	}
	else
		os << misc::fmt(" ");
}


void Inst::DumpBaddr(std::ostream &os)
{
	InstCategory cat = this->info->category;
	signed int offset;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		offset = (this->dword.brnch.off << 2);
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: brnch fmt not recognized", cat));

	os << misc::fmt("%x", this->addr + offset + 8);
}


void Inst::DumpRegs(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int reg_list;
	int i;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryBdtr)
		reg_list = this->dword.bdtr.reg_lst;
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	// TODO: destinations for CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));

	os << misc::fmt("{");
	for (i = 1; i < 65536; i *= 2)
	{
		if(reg_list & (i))
		{
			os << misc::fmt("r%d ", misc::LogBase2(i));
		}
	}

	os << misc::fmt("}");
}


void Inst::DumpImmd24(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int immd24;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));
	else if (cat == InstCategorySwiSvc)
		immd24 = this->dword.swi_svc.cmnt;
	// TODO: destinations for CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: swi_svc fmt not recognized", cat));

	os << misc::fmt("0x%x",immd24);
}


void Inst::DumpImmd16(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int immd16;

	if (cat == InstCategoryDprReg)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryDprImm)
		immd16 = ((this->dword.dpr.op1_reg << 12)
			| this->dword.dpr.op2);
	else if (cat == InstCategoryDprSat)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryPsr)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryMult)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryMultSign)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryMultLn)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryMultLnSign)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdReg)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryHfwrdImm)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryBax)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategorySdtr)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategorySdswp)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategoryBrnch)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	else if (cat == InstCategorySwiSvc)
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: movt_movw fmt not recognized", cat));

	os << misc::fmt("#%d  ; 0x%x",immd16, immd16);
}


void Inst::DumpCopr(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int copr;

	if (cat == InstCategoryCprRtr)
		copr = this->dword.cpr_rtr.cpr_num;
	else if (cat == InstCategoryCprDtr)
		copr = this->dword.cpr_dtr.cpr_num;
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: copr num fmt not recognized", cat));

	os << misc::fmt("%d", copr);
}


void Inst::DumpAMode5(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int offset;
	unsigned int rn;

	if (cat == InstCategoryCprRtr)
		throw misc::Panic(misc::fmt("%d: copr num fmt not recognized", cat));
	else if (cat == InstCategoryCprDtr)
		offset = this->dword.cpr_dtr.off;
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: amode5 fmt not recognized", cat));

	rn = this->dword.cpr_dtr.base_rn;
	if(offset)
	{
	os << misc::fmt("[r%d], #%d", rn, offset*4);
	}
	else
		os << misc::fmt("[r%d]", rn);
}


void Inst::DumpVfp1stm(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int vfp1;

	if (cat == InstCategoryVfp)
		vfp1 = this->dword.vfp_mv.immd8;
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: vfp1 stm fmt not recognized", cat));

	if (vfp1 % 2)
		os << misc::fmt("FSTMIAX");
	else
		os << misc::fmt("VSTMIA");
}


void Inst::DumpVfp1ldm(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int vfp1;

	if (cat == InstCategoryVfp)
		vfp1 = this->dword.vfp_mv.immd8;
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: vfp1 ldm fmt not recognized", cat));

	if(vfp1 % 2)
		os << misc::fmt("FLDMIAX");
	else
		os << misc::fmt("VLDMIA");
}


void Inst::DumpVfpRegs(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int immd8;
	unsigned int reg_start;

	if (cat == InstCategoryVfp)
	{
		immd8 = this->dword.vfp_mv.immd8;
		reg_start = ((this->dword.vfp_mv.d << 4)
			| (this->dword.vfp_mv.vd)) & (0x0000001f);
	}
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: vfp regs fmt not recognized", cat));

	os << misc::fmt("{d%d-d%d}", reg_start,
		(reg_start + immd8/2 - 1));
}


void Inst::DumpFreg(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int freg;


	if (cat == InstCategoryCprDtr)
		freg = this->dword.cpr_dtr.cpr_sr_dst;
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: freg fmt not recognized", cat));

	if(freg > 7)
		os << misc::fmt("f%d", (freg-8));
	else
		os << misc::fmt("f%d", freg);
}


void Inst::DumpFp(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int freg;

	if (cat == InstCategoryCprDtr)
		freg = this->dword.cpr_dtr.cpr_sr_dst;
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: FP fmt not recognized", cat));

	if(freg > 7)
		os << misc::fmt("P");
	else
		os << misc::fmt("E");
}


void Inst::DumpRt(std::ostream &os)
{
	InstCategory cat = this->info->category;
	unsigned int rt;

	if (cat == InstCategoryVfp)
		rt = this->dword.vfp_strreg_tr.vfp_rt;
	// TODO: destinations for BDTR CDTR CDO
	else
		throw misc::Panic(misc::fmt("%d: vfp rt fmt not recognized", cat));

	switch (rt)
		{
		case (AsmUserRegistersR13):

			os << misc::fmt("sp");
			break;

		case (AsmUserRegistersR14):

			os << misc::fmt("lr");
			break;

		case (AsmUserRegistersR15):

			os << misc::fmt("pc");
			break;

		default:

			os << misc::fmt("r%d", rt);
			break;
		}
}


void Inst::DumpHex(std::ostream &os, unsigned int *inst_ptr, unsigned int inst_addr)
{
	os << misc::fmt("%8x:\t%08x\t",inst_addr, *inst_ptr);
}


void Inst::Dump(std::ostream &os)
{
	const char *fmt_str;
	int token_len;

	// Nothing for empty format string 
	fmt_str = this->info ? this->info->fmt_str : NULL;
	if (!fmt_str || !*fmt_str)
	{
		os << misc::fmt("???\n");
		return;
	}

	// Follow format string 
	while (*fmt_str)
	{
		if (*fmt_str != '%')
		{
			os << misc::fmt("%c",
					*fmt_str);
			++fmt_str;
			continue;
		}

		++fmt_str;
		if (comm::Asm::isToken(fmt_str, "rd", token_len))
			DumpRd(os);
		else if (comm::Asm::isToken(fmt_str, "rn", token_len))
			DumpRn(os);
		else if (comm::Asm::isToken(fmt_str, "rm", token_len))
			DumpRm(os);
		else if (comm::Asm::isToken(fmt_str, "rs", token_len))
			DumpRs(os);
		else if (comm::Asm::isToken(fmt_str, "rt", token_len))
			DumpRt(os);
		else if (comm::Asm::isToken(fmt_str, "op2", token_len))
			DumpOp2(os);
		else if (comm::Asm::isToken(fmt_str, "cond", token_len))
			DumpCond(os);
		else if (comm::Asm::isToken(fmt_str, "rdlo", token_len))
			DumpRdlo(os);
		else if (comm::Asm::isToken(fmt_str, "rdhi", token_len))
			DumpRdhi(os);
		else if (comm::Asm::isToken(fmt_str, "psr", token_len))
			DumpPsr(os);
		else if (comm::Asm::isToken(fmt_str, "op2psr", token_len))
			DumpOp2Psr(os);
		else if (comm::Asm::isToken(fmt_str, "amode3", token_len))
			DumpAMode3(os);
		else if (comm::Asm::isToken(fmt_str, "amode2", token_len))
			DumpAMode2(os);
		else if (comm::Asm::isToken(fmt_str, "idx", token_len))
			DumpIdx(os);
		else if (comm::Asm::isToken(fmt_str, "baddr", token_len))
			DumpBaddr(os);
		else if (comm::Asm::isToken(fmt_str, "regs", token_len))
			DumpRegs(os);
		else if (comm::Asm::isToken(fmt_str, "immd24", token_len))
			DumpImmd24(os);
		else if (comm::Asm::isToken(fmt_str, "immd16", token_len))
			DumpImmd16(os);
		else if (comm::Asm::isToken(fmt_str, "copr", token_len))
			DumpCopr(os);
		else if (comm::Asm::isToken(fmt_str, "amode5", token_len))
			DumpAMode5(os);
		else if (comm::Asm::isToken(fmt_str, "vfp1stmia", token_len))
			DumpVfp1stm(os);
		else if (comm::Asm::isToken(fmt_str, "vfp1ldmia", token_len))
			DumpVfp1ldm(os);
		else if (comm::Asm::isToken(fmt_str, "vfpregs", token_len))
			DumpVfpRegs(os);
		else if (comm::Asm::isToken(fmt_str, "freg", token_len))
			DumpFreg(os);
		else if (comm::Asm::isToken(fmt_str, "fp", token_len))
			DumpFp(os);

		else
			throw misc::Panic(misc::fmt("%s: token not recognized\n", fmt_str));

		fmt_str += token_len;
	}
	os << misc::fmt("\n");
}


void Inst::Thumb16DumpRD(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;

	unsigned int rd;

	if (cat == InstThumb16CategoryMovshiftReg)
		rd = this->dword_16.movshift_reg_ins.reg_rd;
	else if (cat == InstThumb16CategoryAddsub)
		rd = this->dword_16.addsub_ins.reg_rd;
	else if (cat == InstThumb16CategoryImmdOprs)
		rd = this->dword_16.immd_oprs_ins.reg_rd;
	else if (cat == InstThumb16CategoryDprIns)
		rd = this->dword_16.dpr_ins.reg_rd;
	else if (cat == InstThumb16CategoryHiRegOprs)
		rd = ((this->dword_16.high_oprs_ins.h1 << 3) |  this->dword_16.high_oprs_ins.reg_rd);
	else if (cat == InstThumb16CategoryPcLdr)
		rd = this->dword_16.pcldr_ins.reg_rd;
	else if (cat == InstThumb16CategoryLdstrReg)
		rd = this->dword_16.ldstr_reg_ins.reg_rd;
	else if (cat == InstThumb16CategoryLdstrExts)
		rd = this->dword_16.ldstr_exts_ins.reg_rd;
	else if (cat == InstThumb16CategoryLdstrImmd)
		rd = this->dword_16.ldstr_immd_ins.reg_rd;
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		rd = this->dword_16.ldstr_hfwrd_ins.reg_rd;
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		rd = this->dword_16.sp_immd_ins.reg_rd;
	else if (cat == InstThumb16CategoryIfThen)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdmStm)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscAddspIns)
		rd = this->dword_16.addsp_ins.reg_rd;
	else if (cat == InstThumb16CategoryMiscRev)
		rd = this->dword_16.rev_ins.reg_rd;
	else if (cat == InstThumb16CategoryCmpT2)
		rd = (this->dword_16.cmp_t2.N << 3 | this->dword_16.cmp_t2.reg_rn);

	else
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));

	switch (rd)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rd);
		break;
	}
}


void Inst::Thumb16DumpRM(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int rm;

	if (cat == InstThumb16CategoryMovshiftReg)
		rm = this->dword_16.movshift_reg_ins.reg_rs;
	else if (cat == InstThumb16CategoryAddsub)
		rm = this->dword_16.addsub_ins.reg_rs;
	else if (cat == InstThumb16CategoryImmdOprs)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryDprIns)
		rm = this->dword_16.dpr_ins.reg_rs;
	else if (cat == InstThumb16CategoryHiRegOprs)
		rm = this->dword_16.high_oprs_ins.reg_rs;
	else if (cat == InstThumb16CategoryPcLdr)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrReg)
		rm = this->dword_16.ldstr_reg_ins.reg_ro;
	else if (cat == InstThumb16CategoryLdstrExts)
		rm = this->dword_16.ldstr_exts_ins.reg_rb;
	else if (cat == InstThumb16CategoryLdstrImmd)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryIfThen)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdmStm)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscAddspIns)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscRev)
		rm = this->dword_16.rev_ins.reg_rm;
	else if (cat == InstThumb16CategoryCmpT2)
		rm = this->dword_16.cmp_t2.reg_rm;
	else
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));

	switch (rm)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rm);
		break;
	}
}


void Inst::Thumb16DumpRN(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int rn;

	if (cat == InstThumb16CategoryMovshiftReg)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb16CategoryAddsub)
		rn = this->dword_16.addsub_ins.reg_rs;
	else if (cat == InstThumb16CategoryImmdOprs)
		rn = this->dword_16.immd_oprs_ins.reg_rd;
	else if (cat == InstThumb16CategoryDprIns)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb16CategoryHiRegOprs)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb16CategoryPcLdr)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrReg)
		rn = this->dword_16.ldstr_reg_ins.reg_rb;
	else if (cat == InstThumb16CategoryLdstrExts)
		rn = this->dword_16.ldstr_exts_ins.reg_rb;
	else if (cat == InstThumb16CategoryLdstrImmd)
		rn = this->dword_16.ldstr_immd_ins.reg_rb;
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		rn = this->dword_16.ldstr_hfwrd_ins.reg_rb;
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb16CategoryIfThen)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdmStm)
		rn = this->dword_16.ldm_stm_ins.reg_rb;
	else if (cat == InstThumb16CategoryMiscAddspIns)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscCbnz)
		rn = this->dword_16.cbnz_ins.reg_rn;
	else
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));

	switch (rn)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;
	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rn);
		break;
	}
}


void Inst::Thumb16DumpIMMD8(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int inst_addr = this->addr;
	unsigned int immd8;

	if (cat == InstThumb16CategoryMovshiftReg)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryAddsub)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryImmdOprs)
		immd8 = this->dword_16.immd_oprs_ins.offset8;
	else if (cat == InstThumb16CategoryDprIns)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryHiRegOprs)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryPcLdr)
		immd8 =(this->dword_16.pcldr_ins.immd_8 << 2);
	else if (cat == InstThumb16CategoryLdstrReg)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrExts)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrImmd)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		immd8 = 4 * this->dword_16.sp_immd_ins.immd_8;
	else if (cat == InstThumb16CategoryIfThen)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdmStm)
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscAddspIns)
		immd8 = 4 * this->dword_16.addsp_ins.immd_8;
	else if (cat == InstThumb16CategoryMiscSubspIns)
		immd8 = 4 * this->dword_16.sub_sp_ins.immd_8;
	else if (cat == InstThumb16CategoryMiscBr)
		immd8 = this->dword_16.cond_br_ins.s_offset;
	else if (cat == InstThumb16CategoryMiscUcbr)
		immd8 = this->dword_16.br_ins.immd11;
	else if (cat == InstThumb16CategoryMiscSvcIns)
		immd8 = this->dword_16.svc_ins.value;
	else
		throw misc::Panic(misc::fmt("%d: immd8 fmt not recognized", cat));

	if(cat == InstThumb16CategoryMiscBr)
	{
		if((immd8 >> 7))
		{
			immd8 = ((inst_addr + 4) + ((immd8 << 1) | 0xffffff00));
		}
		else
		{
			immd8 = (inst_addr + 4) + (immd8 << 1);
		}
		os << misc::fmt("%x",immd8);
	}
	else if(cat == InstThumb16CategoryMiscUcbr)
	{
		immd8 = immd8 << 1;
		immd8 = misc::SignExtend32(immd8, 12);

		immd8 = inst_addr + 4 + immd8;
		os << misc::fmt("%x",immd8);
	}

	else
		os << misc::fmt("#%d",immd8);

}


void Inst::Thumb16DumpIMMD3(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int immd3;

	if (cat == InstThumb16CategoryMovshiftReg)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryAddsub)
		immd3 = this->dword_16.addsub_ins.rn_imm;
	else if (cat == InstThumb16CategoryImmdOprs)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryDprIns)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryHiRegOprs)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryPcLdr)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrReg)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrExts)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrImmd)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryIfThen)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdmStm)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscAddspIns)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscSubspIns)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscBr)
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));

	else
		throw misc::Panic(misc::fmt("%d: immd3 fmt not recognized", cat));



	os << misc::fmt("#%d",immd3);

}


void Inst::Thumb16DumpIMMD5(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int inst_addr = this->addr;
	unsigned int immd5;

	if (cat == InstThumb16CategoryMovshiftReg)
		immd5 = this->dword_16.movshift_reg_ins.offset;
	else if (cat == InstThumb16CategoryAddsub)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryImmdOprs)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryDprIns)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryHiRegOprs)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryPcLdr)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrReg)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrExts)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrImmd)
		immd5 = this->dword_16.ldstr_immd_ins.offset << 2;
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		immd5 = this->dword_16.ldstr_hfwrd_ins.offset;
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryIfThen)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdmStm)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscAddspIns)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscSubspIns)
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscCbnz)
		immd5 = this->dword_16.cbnz_ins.immd_5;
	else
		throw misc::Panic(misc::fmt("%d: immd5 fmt not recognized", cat));

	if(cat == InstThumb16CategoryMiscCbnz)
	{
		if((inst_addr + 2) % 4)
			immd5 = (inst_addr + 4) + (immd5 << 1);
		else
			immd5 = (inst_addr + 2) + (immd5 << 1);

		os << misc::fmt("%x",immd5);
	}
	else
		os << misc::fmt("#%d",immd5);

}


void Inst::Thumb16DumpCOND(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int cond;

	if (cat == InstThumb16CategoryMovshiftReg)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryAddsub)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryImmdOprs)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryDprIns)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryHiRegOprs)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryPcLdr)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrReg)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrExts)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrImmd)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryIfThen)
		cond = this->dword_16.if_eq_ins.first_cond;
	else if (cat == InstThumb16CategoryLdmStm)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscAddspIns)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscSubspIns)
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscBr)
		cond = this->dword_16.cond_br_ins.cond;
	else if (cat == InstThumb16CategoryIfThen)
		cond = this->dword_16.if_eq_ins.first_cond;

	else
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));

	switch (cond)
	{
	case (AsmConditionCodesEQ):

		os << misc::fmt("eq");
		break;

	case (AsmConditionCodesNE):

		os << misc::fmt("ne");
		break;

	case (AsmConditionCodesCS):

		os << misc::fmt("cs");
		break;

	case (AsmConditionCodesCC):

		os << misc::fmt("cc");
		break;

	case (AsmConditionCodesMI):

		os << misc::fmt("mi");
		break;

	case (AsmConditionCodesPL):

		os << misc::fmt("pl");
		break;

	case (AsmConditionCodesVS):

		os << misc::fmt("vs");
		break;

	case (AsmConditionCodesVC):

		os << misc::fmt("vc");
		break;

	case (AsmConditionCodesHI):

		os << misc::fmt("hi");
		break;

	case (AsmConditionCodesLS):

		os << misc::fmt("ls");
		break;

	case (AsmConditionCodesGE):

		os << misc::fmt("ge");
		break;

	case (AsmConditionCodesLT):

		os << misc::fmt("lt");
		break;

	case (AsmConditionCodesGT):

		os << misc::fmt("gt");
		break;

	case (AsmConditionCodesLE):

		os << misc::fmt("le");
		break;

	case (AsmConditionCodesAL):

		os << misc::fmt(" ");
		break;
	}

}


void Inst::Thumb16DumpREGS(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int regs;
	unsigned int i;

	if (cat == InstThumb16CategoryMovshiftReg)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryAddsub)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryImmdOprs)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryDprIns)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryHiRegOprs)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryPcLdr)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrReg)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrExts)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrImmd)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrHfwrd)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdstrSpImmd)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryIfThen)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryLdmStm)
		regs = this->dword_16.ldm_stm_ins.reg_list;
	else if (cat == InstThumb16CategoryMiscAddspIns)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscSubspIns)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscBr)
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	else if (cat == InstThumb16CategoryMiscPushPop)
		regs = this->dword_16.push_pop_ins.reg_list;
	else
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));

	regs = (this->dword_16.push_pop_ins.m_ext << 14) | regs;
	os << misc::fmt("{");
	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			os << misc::fmt("r%d ", misc::LogBase2(i));
		}
	}
	os << misc::fmt("}");
}


void Inst::Thumb16DumpItEqX(std::ostream &os)
{
	InstThumb16Category cat = this->info_16->cat16;
	unsigned int first_cond;
	unsigned int mask;

	if (cat == InstThumb16CategoryIfThen)
	{
		mask = this->dword_16.if_eq_ins.mask;
		first_cond = this->dword_16.if_eq_ins.first_cond;
	}
	else
		throw misc::Panic(misc::fmt("%d: x fmt not recognized", cat));

	if((mask != 0x8))
	{
		if((mask >> 3) ^ (first_cond & 1))
			os << misc::fmt("e");
		else
			os << misc::fmt("t");
	}
}


void Inst::Thumb16DumpHex(std::ostream &os, unsigned int *inst_ptr , unsigned int inst_addr)
{
	os << misc::fmt("%8x:	%04x		", inst_addr, *inst_ptr);
}


void Inst::Thumb16Dump(std::ostream &os)
{

	const char *fmt_str;
	int token_len;

	// Nothing for empty format string 
	fmt_str = this->info_16 ? this->info_16->fmt_str : NULL;
	if (!fmt_str || !*fmt_str)
	{
		os << misc::fmt("???\n");
		return;
	}

	// Follow format string 
	while (*fmt_str)
	{
		if (*fmt_str != '%')
		{
			if (*fmt_str != ' ')
				os << misc::fmt("%c",
						*fmt_str);
			++fmt_str;
			continue;
		}

		++fmt_str;
		if (comm::Asm::isToken(fmt_str, "rd", token_len))
			Thumb16DumpRD(os);
		else if (comm::Asm::isToken(fmt_str, "rm", token_len))
			Thumb16DumpRM(os);
		else if (comm::Asm::isToken(fmt_str, "rn", token_len))
			Thumb16DumpRN(os);
		else if (comm::Asm::isToken(fmt_str, "immd8", token_len))
			Thumb16DumpIMMD8(os);
		else if (comm::Asm::isToken(fmt_str, "immd5", token_len))
			Thumb16DumpIMMD5(os);
		else if (comm::Asm::isToken(fmt_str, "immd3", token_len))
			Thumb16DumpIMMD3(os);
		else if (comm::Asm::isToken(fmt_str, "cond", token_len))
			Thumb16DumpCOND(os);
		else if (comm::Asm::isToken(fmt_str, "regs", token_len))
			Thumb16DumpREGS(os);
		else if (comm::Asm::isToken(fmt_str, "x", token_len))
			Thumb16DumpItEqX(os);
		else
			throw misc::Panic(misc::fmt("%s: token not recognized\n", fmt_str));

		fmt_str += token_len;
	}
	os << misc::fmt("\n");
}


void Inst::Thumb32DumpRD(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int rd;

	if (cat == InstThumb32CategoryLdStMult)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdStDouble)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb32CategoryPushPop)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb32CategoryTableBrnch)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb32CategoryDprShftreg)
		rd = this->dword_32.data_proc_shftreg.rd;
	else if (cat == InstThumb32CategoryDprImm)
		rd = this->dword_32.data_proc_immd.rd;
	else if (cat == InstThumb32CategoryDprBinImm)
		rd = this->dword_32.data_proc_immd.rd;
	else if (cat == InstThumb32CategoryBranch)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdstrByte)
		rd = this->dword_32.ldstr_reg.rd;
	else if (cat == InstThumb32CategoryLdstrReg)
		rd = this->dword_32.ldstr_reg.rd;
	else if (cat == InstThumb32CategoryLdstrImmd)
		rd = this->dword_32.ldstr_imm.rd;
	else if (cat == InstThumb32CategoryDprReg)
		rd = this->dword_32.dproc_reg.rd;
	else if (cat == InstThumb32CategoryMult)
		rd = this->dword_32.mult.rd;
	else if (cat == InstThumb32CategoryMultLong)
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));
	else if (cat == InstThumb32CategoryBitField)
		rd = this->dword_32.bit_field.rd;

	else
		throw misc::Panic(misc::fmt("%d: rd fmt not recognized", cat));

	switch (rd)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;
	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rd);
		break;
	}

}


void Inst::Thumb32DumpRN(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int rn;

	if (cat == InstThumb32CategoryLdStMult)
		rn = this->dword_32.ld_st_mult.rn;
	else if (cat == InstThumb32CategoryLdStDouble)
		rn = this->dword_32.ld_st_double.rn;
	else if (cat == InstThumb32CategoryPushPop)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb32CategoryTableBrnch)
		rn = this->dword_32.table_branch.rn;
	else if (cat == InstThumb32CategoryDprShftreg)
		rn = this->dword_32.data_proc_shftreg.rn;
	else if (cat == InstThumb32CategoryDprImm)
		rn = this->dword_32.data_proc_immd.rn;
	else if (cat == InstThumb32CategoryDprBinImm)
		rn = this->dword_32.data_proc_immd.rn;
	else if (cat == InstThumb32CategoryBranch)
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdstrByte)
		rn = this->dword_32.ldstr_reg.rn;
	else if (cat == InstThumb32CategoryLdstrReg)
		rn = this->dword_32.ldstr_reg.rn;
	else if (cat == InstThumb32CategoryLdstrImmd)
		rn = this->dword_32.ldstr_imm.rn;
	else if (cat == InstThumb32CategoryDprReg)
		rn = this->dword_32.dproc_reg.rn;
	else if (cat == InstThumb32CategoryMult)
		rn = this->dword_32.mult.rn;
	else if (cat == InstThumb32CategoryMultLong)
		rn = this->dword_32.mult_long.rn;
	else if (cat == InstThumb32CategoryBitField)
		rn = this->dword_32.bit_field.rn;

	else
		throw misc::Panic(misc::fmt("%d: rn fmt not recognized", cat));

	switch (rn)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rn);
		break;
	}

}


void Inst::Thumb32DumpRM(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int rm;

	if (cat == InstThumb32CategoryLdStMult)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdStDouble)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb32CategoryPushPop)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb32CategoryTableBrnch)
		rm = this->dword_32.table_branch.rm;
	else if (cat == InstThumb32CategoryDprShftreg)
		rm = this->dword_32.data_proc_shftreg.rm;
	else if (cat == InstThumb32CategoryDprImm)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb32CategoryDprBinImm)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb32CategoryBranch)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdstrByte)
		rm = this->dword_32.ldstr_reg.rm;
	else if (cat == InstThumb32CategoryLdstrReg)
		rm = this->dword_32.ldstr_reg.rm;
	else if (cat == InstThumb32CategoryLdstrImmd)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));
	else if (cat == InstThumb32CategoryDprReg)
		rm = this->dword_32.dproc_reg.rm;
	else if (cat == InstThumb32CategoryMult)
		rm = this->dword_32.mult.rm;
	else if (cat == InstThumb32CategoryMultLong)
		rm = this->dword_32.mult_long.rm;
	else if (cat == InstThumb32CategoryBitField)
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));

	else
		throw misc::Panic(misc::fmt("%d: rm fmt not recognized", cat));

	switch (rm)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rm);
		break;
	}

}


void Inst::Thumb32DumpRT(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int rt;

	if (cat == InstThumb32CategoryLdStMult)
		throw misc::Panic(misc::fmt("%d: rt fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdStDouble)
		rt = this->dword_32.ld_st_double.rt;
	else
		throw misc::Panic(misc::fmt("%d: rt fmt not recognized", cat));

	switch (rt)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rt);
		break;
	}
}


void Inst::Thumb32DumpRT2(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int rt2;

	if (cat == InstThumb32CategoryLdStMult)
		throw misc::Panic(misc::fmt("%d: rt fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdStDouble)
		rt2 = this->dword_32.ld_st_double.rt2;
	else
		throw misc::Panic(misc::fmt("%d: rt2 fmt not recognized", cat));

	switch (rt2)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rt2);
		break;
	}
}


void Inst::Thumb32DumpRA(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int ra;


	if (cat == InstThumb32CategoryMult)
		ra = this->dword_32.mult.ra;
	else
		throw misc::Panic(misc::fmt("%d: ra fmt not recognized", cat));

	switch (ra)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", ra);
		break;
	}
}


void Inst::Thumb32DumpRDLO(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int rdlo;


	if (cat == InstThumb32CategoryMultLong)
		rdlo = this->dword_32.mult_long.rdlo;
	else
		throw misc::Panic(misc::fmt("%d: rdlo fmt not recognized", cat));

	switch (rdlo)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rdlo);
		break;
	}
}


void Inst::Thumb32DumpRDHI(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int rdhi;


	if (cat == InstThumb32CategoryMultLong)
		rdhi = this->dword_32.mult_long.rdhi;
	else
		throw misc::Panic(misc::fmt("%d: rdhi fmt not recognized", cat));

	switch (rdhi)
	{
	case (AsmUserRegistersR13):

		os << misc::fmt("sp");
		break;

	case (AsmUserRegistersR14):

		os << misc::fmt("lr");
		break;

	case (AsmUserRegistersR15):

		os << misc::fmt("pc");
		break;

	default:

		os << misc::fmt("r%d", rdhi);
		break;
	}
}


void Inst::Thumb32DumpS(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int sign;

	if (cat == InstThumb32CategoryLdStMult)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdStDouble)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryPushPop)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryTableBrnch)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryDprShftreg)
		sign = this->dword_32.data_proc_shftreg.sign;
	else if (cat == InstThumb32CategoryDprImm)
		sign = this->dword_32.data_proc_immd.sign;
	else if (cat == InstThumb32CategoryDprBinImm)
		sign = this->dword_32.data_proc_immd.sign;
	else if (cat == InstThumb32CategoryBranch)
		sign = this->dword_32.branch.sign;
	else if (cat == InstThumb32CategoryLdstrByte)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdstrReg)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryLdstrImmd)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryDprReg)
		sign = this->dword_32.dproc_reg.sign;
	else if (cat == InstThumb32CategoryMult)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryMultLong)
		throw misc::Panic(misc::fmt("%d: S fmt not recognized", cat));
	else if (cat == InstThumb32CategoryBitField)
		throw misc::Panic(misc::fmt("%d: sign fmt not recognized", cat));

	else
		throw misc::Panic(misc::fmt("%d: sign fmt not recognized", cat));
	if (sign)
		os << misc::fmt("s");
}


void Inst::Thumb32dumpREGS(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int regs;

	if (cat == InstThumb32CategoryLdStMult)
		regs = this->dword_32.ld_st_mult.reglist;
	else if (cat == InstThumb32CategoryPushPop)
		regs = this->dword_32.push_pop.reglist;

	else
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));

	os << misc::fmt("{");
	for (unsigned int i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			os << misc::fmt("r%d ", misc::LogBase2(i));
		}
	}

	os << misc::fmt("}");

}


void Inst::Thumb32DumpSHFTREG(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int shift;
	unsigned int type;

	if (cat == InstThumb32CategoryDprShftreg)
	{
		type = this->dword_32.data_proc_shftreg.type;
		shift = (this->dword_32.data_proc_shftreg.imm3 << 2) | (this->dword_32.data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: shft fmt not recognized", cat));

	if (shift)
	{
		switch(type)
		{
		case (AsmShiftOperatorLsl):

			os << misc::fmt("{lsl #%d}", shift);
			break;

		case (AsmShiftOperatorLsr):

			os << misc::fmt("{lsr #%d}", shift);
			break;

		case (AsmShiftOperatorAsr):

			os << misc::fmt("{asr #%d}", shift);
			break;

		case (AsmShiftOperatorRor):

			os << misc::fmt("{ror #%d}", shift);
			break;
		}
	}

}


void Inst::Thumb32DumpIMM12(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;

	if (cat == InstThumb32CategoryLdstrImmd)
		immd12 = this->dword_32.ldstr_imm.immd12;
	else if (cat == InstThumb32CategoryLdstrByte)
		immd12 = this->dword_32.ldstr_imm.immd12;
	else
		throw misc::Panic(misc::fmt("%d: imm12 fmt not recognized", cat));

	if (this->dword_32.ldstr_imm.add)
	{
		os << misc::fmt("#%d",immd12);
	}
	else
	{
		idx = (immd12 & 0x00000400) >> 10;
		add = (immd12 & 0x00000200) >> 9;
		wback = (immd12 & 0x00000100) >> 8;
		if (add)
		{
			if(idx == 1 && wback == 0)
				os << misc::fmt("[#%d]",(immd12 & 0x000000ff));
			else if (idx == 1 && wback == 1)
				os << misc::fmt("[#%d]!",(immd12 & 0x000000ff));
			else if (idx == 0 && wback == 1)
				os << misc::fmt("#%d",(immd12 & 0x000000ff));
		}
		else
		{
			if(idx == 1 && wback == 0)
				os << misc::fmt("[#-%d]",(immd12 & 0x000000ff));
			else if (idx == 1 && wback == 1)
				os << misc::fmt("[#-%d]!",(immd12 & 0x000000ff));
			else if (idx == 0 && wback == 1)
				os << misc::fmt("#-%d",(immd12 & 0x000000ff));

		}
	}

}


void Inst::Thumb32DumpIMMD12(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int imm4;
	unsigned int imm5;
	unsigned int shft;
	unsigned int const_val;

	if (cat == InstThumb32CategoryDprImm)
	{
		immd8 = this->dword_32.data_proc_immd.immd8;
		immd3 = this->dword_32.data_proc_immd.immd3;
		i = this->dword_32.data_proc_immd.i_flag;
	}
	else if (cat == InstThumb32CategoryDprBinImm)
	{
		immd8 = this->dword_32.data_proc_immd.immd8;
		immd3 = this->dword_32.data_proc_immd.immd3;
		i = this->dword_32.data_proc_immd.i_flag;
	}
	else
		throw misc::Panic(misc::fmt("%d: immd12 fmt not recognized", cat));

	imm4 = (i << 3) | (immd3);

	if (imm4 < 4)
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

	os << misc::fmt("#%d", const_val);
}


void Inst::Thumb32DumpIMMD8(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int immd8;

	if (cat == InstThumb32CategoryLdStDouble)
		immd8 = (this->dword_32.ld_st_double.immd8 << 2);
	else
		throw misc::Panic(misc::fmt("%d: immd12 fmt not recognized", cat));


	if (immd8)
	{
		if(this->dword_32.ld_st_double.add_sub)
		{
			if(this->dword_32.ld_st_double.index == 1 && this->dword_32.ld_st_double.wback == 0)
				os << misc::fmt("#%d",(immd8));
			else if (this->dword_32.ld_st_double.index == 1 && this->dword_32.ld_st_double.wback == 1)
				os << misc::fmt("#%d!",(immd8));
			else if (this->dword_32.ld_st_double.index == 0 && this->dword_32.ld_st_double.wback == 0)
				os << misc::fmt("#%d",(immd8));
		}
		else
		{
			if(this->dword_32.ld_st_double.index == 1 && this->dword_32.ld_st_double.wback == 0)
				os << misc::fmt("#-%d",(immd8));
			else if (this->dword_32.ld_st_double.index == 1 && this->dword_32.ld_st_double.wback == 1)
				os << misc::fmt("#-%d!",(immd8));
			else if (this->dword_32.ld_st_double.index == 0 && this->dword_32.ld_st_double.wback == 1)
				os << misc::fmt("#-%d",(immd8));

		}
	}

}


void Inst::Thumb32DumpIMM2(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int immd2;

	if (cat == InstThumb32CategoryLdstrByte)
	{
		immd2 = this->dword_32.ldstr_reg.immd2;
	}
	else if (cat == InstThumb32CategoryLdstrReg)
	{
		immd2 = this->dword_32.ldstr_reg.immd2;
	}
	else
		throw misc::Panic(misc::fmt("%d: imm2 fmt not recognized", cat));

	os << misc::fmt("#%d", immd2);
}


void Inst::Thumb32DumpCOND(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int cond;

	if (cat == InstThumb32CategoryBranchCond)
	{
		cond = this->dword_32.branch.cond;
	}
	else
		throw misc::Panic(misc::fmt("%d: cond fmt not recognized", cat));

	switch (cond)
	{
	case (AsmConditionCodesEQ):

		os << misc::fmt("eq");
		break;

	case (AsmConditionCodesNE):

		os << misc::fmt("ne");
		break;

	case (AsmConditionCodesCS):

		os << misc::fmt("cs");
		break;

	case (AsmConditionCodesCC):

		os << misc::fmt("cc");
		break;

	case (AsmConditionCodesMI):

		os << misc::fmt("mi");
		break;

	case (AsmConditionCodesPL):

		os << misc::fmt("pl");
		break;

	case (AsmConditionCodesVS):

		os << misc::fmt("vs");
		break;

	case (AsmConditionCodesVC):

		os << misc::fmt("vc");
		break;

	case (AsmConditionCodesHI):

		os << misc::fmt("hi");
		break;

	case (AsmConditionCodesLS):

		os << misc::fmt("ls");
		break;

	case (AsmConditionCodesGE):

		os << misc::fmt("ge");
		break;

	case (AsmConditionCodesLT):

		os << misc::fmt("lt");
		break;

	case (AsmConditionCodesGT):

		os << misc::fmt("gt");
		break;

	case (AsmConditionCodesLE):

		os << misc::fmt("le");
		break;

	case (AsmConditionCodesAL):

		os << misc::fmt(" ");
		break;
	}

}


void Inst::Thumb32DumpLSB(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int immd2;
	unsigned int immd3;

	if (cat == InstThumb32CategoryBitField)
	{
		immd2 = this->dword_32.bit_field.immd2;
		immd3 = this->dword_32.bit_field.immd3;
	}
	else
		throw misc::Panic(misc::fmt("%d: imm2 fmt not recognized", cat));

	os << misc::fmt("#%d", ((immd3 << 2) | immd2));
}


void Inst::Thumb32DumpWID(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int msb;
	unsigned int immd2;
	unsigned int immd3;
	unsigned int lsb;

	if (cat == InstThumb32CategoryBitField)
	{
		msb = this->dword_32.bit_field.msb;
		immd2 = this->dword_32.bit_field.immd2;
		immd3 = this->dword_32.bit_field.immd3;
	}
	else
		throw misc::Panic(misc::fmt("%d: imm2 fmt not recognized", cat));

	lsb = (immd3 << 2) | immd2;
	os << misc::fmt("#%d", (msb - lsb + 1));
}


void Inst::Thumb32DumpIMMD16(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int immd16;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int immd4;


	if (cat == InstThumb32CategoryDprBinImm)
	{
		immd8 = this->dword_32.data_proc_immd.immd8;
		immd3 = this->dword_32.data_proc_immd.immd3;
		i = this->dword_32.data_proc_immd.i_flag;
		immd4 = this->dword_32.data_proc_immd.rn;
	}

	else
		throw misc::Panic(misc::fmt("%d: immd16 fmt not recognized", cat));

	immd16 = (immd4 << 12) | (i << 11) | (immd3 << 8) | immd8;

	os << misc::fmt("#%d	; 0x%x", immd16, immd16);
}


void Inst::Thumb32DumpADDR(std::ostream &os)
{
	InstThumb32Category cat = this->info_32->cat32;
	unsigned int addr;
	unsigned int inst_addr = this->addr;
	addr = 0;
	if (cat == InstThumb32CategoryBranch)
	{
		addr = (this->dword_32.branch_link.sign << 24)
					| ((!(this->dword_32.branch.j1 ^ this->dword_32.branch_link.sign)) << 23)
					| ((!(this->dword_32.branch.j2 ^ this->dword_32.branch_link.sign)) << 22)
					| (this->dword_32.branch_link.immd10 << 12)
					| (this->dword_32.branch_link.immd11 << 1);
		addr = misc::SignExtend32(addr,25);
	}
	else if (cat == InstThumb32CategoryBranchLx)
	{
		addr = (this->dword_32.branch_link.sign << 24)
					| ((!(this->dword_32.branch.j1 ^ this->dword_32.branch_link.sign)) << 23)
					| ((!(this->dword_32.branch.j2 ^ this->dword_32.branch_link.sign)) << 22)
					| (this->dword_32.branch_link.immd10 << 12)
					| ((this->dword_32.branch_link.immd11 & 0xfffffffe) << 1);
		addr = misc::SignExtend32(addr,25);
	}
	else if (cat == InstThumb32CategoryBranchCond)
	{
		addr = (this->dword_32.branch.sign << 20)
					| (((this->dword_32.branch.j2)) << 19)
					| (((this->dword_32.branch.j1)) << 18)
					| (this->dword_32.branch.immd6 << 12)
					| (this->dword_32.branch.immd11 << 1);
		addr = misc::SignExtend32(addr,21);
	}
	else
		throw misc::Panic(misc::fmt("%d: addr fmt not recognized", cat));

	// FIXME : Changed from +4 to +2 
	addr = (inst_addr + 2) + (addr);
	os << misc::fmt("#%d	; 0x%x", addr, addr);
}


void Inst::Thumb32DumpHex(std::ostream &os, unsigned int *inst_ptr , unsigned int inst_addr)
{
	int thumb_32;
	thumb_32 = *inst_ptr;
	os << misc::fmt("%8x:	%04x %04x	", inst_addr, (thumb_32 & 0x0000ffff),
			((thumb_32) & 0xffff0000) >> 16);
}


void Inst::Thumb32Dump(std::ostream &os)
{

	const char *fmt_str;
	int token_len;

	// Nothing for empty format string 
	fmt_str = this->info_32 ? this->info_32->fmt_str : NULL;
	if (!fmt_str || !*fmt_str)
	{
		os << misc::fmt("???\n");
		return;
	}

	// Follow format string 
	while (*fmt_str)
	{
		if (*fmt_str != '%')
		{
			if (*fmt_str != ' ' )
				os << misc::fmt("%c",
					*fmt_str);
			++fmt_str;
			continue;
		}


		++fmt_str;
		if (comm::Asm::isToken(fmt_str, "rd", token_len))
			Thumb32DumpRD(os);
		else if (comm::Asm::isToken(fmt_str, "rn", token_len))
			Thumb32DumpRN(os);
		else if (comm::Asm::isToken(fmt_str, "rm", token_len))
			Thumb32DumpRM(os);
		else if (comm::Asm::isToken(fmt_str, "rt", token_len))
			Thumb32DumpRT(os);
		else if (comm::Asm::isToken(fmt_str, "rt2", token_len))
			Thumb32DumpRT2(os);
		else if (comm::Asm::isToken(fmt_str, "ra", token_len))
			Thumb32DumpRA(os);
		else if (comm::Asm::isToken(fmt_str, "rdlo", token_len))
			Thumb32DumpRDLO(os);
		else if (comm::Asm::isToken(fmt_str, "rdhi", token_len))
			Thumb32DumpRDHI(os);
		else if (comm::Asm::isToken(fmt_str, "imm12", token_len))
			Thumb32DumpIMM12(os);
		else if (comm::Asm::isToken(fmt_str, "imm8", token_len))
			Thumb32DumpIMM12(os);
		else if (comm::Asm::isToken(fmt_str, "imm2", token_len))
			Thumb32DumpIMM2(os);
		else if (comm::Asm::isToken(fmt_str, "immd8", token_len))
			Thumb32DumpIMMD8(os);
		else if (comm::Asm::isToken(fmt_str, "immd12", token_len))
			Thumb32DumpIMMD12(os);
		else if (comm::Asm::isToken(fmt_str, "immd16", token_len))
			Thumb32DumpIMMD16(os);
		else if (comm::Asm::isToken(fmt_str, "addr", token_len))
			Thumb32DumpADDR(os);
		else if (comm::Asm::isToken(fmt_str, "regs", token_len))
			Thumb32dumpREGS(os);
		else if (comm::Asm::isToken(fmt_str, "shft", token_len))
			Thumb32DumpSHFTREG(os);
		else if (comm::Asm::isToken(fmt_str, "S", token_len))
			Thumb32DumpS(os);
		else if (comm::Asm::isToken(fmt_str, "lsb", token_len))
			Thumb32DumpLSB(os);
		else if (comm::Asm::isToken(fmt_str, "wid", token_len))
			Thumb32DumpWID(os);
		else if (comm::Asm::isToken(fmt_str, "cond", token_len))
			Thumb32DumpCOND(os);
		else
			throw misc::Panic(misc::fmt("%s: token not recognized\n", fmt_str));

		fmt_str += token_len;
	}
	os << misc::fmt("\n");
}


void Inst::Thumb32InstTableDecode()
{
	struct InstThumb32Info *current_table;

	// We initially start with the first table mips_asm_table, with the opcode field as argument 
	current_table = as->dec_table_thumb32_asm;
	int current_table_low = 27;
	int current_table_high = 28;
	unsigned int thumb32_table_arg;
	int loop_iteration = 0;

	thumb32_table_arg =  misc::getBits32(*(unsigned int*)this->dword_32.bytes, current_table_high, current_table_low);

	// Find next tables if the instruction belongs to another table 
	while (1) {
		if (current_table[thumb32_table_arg].next_table && loop_iteration < 8) {
			current_table_high = current_table[thumb32_table_arg].next_table_high;
			current_table_low = current_table[thumb32_table_arg].next_table_low;
			current_table = current_table[thumb32_table_arg].next_table;
			thumb32_table_arg = misc::getBits32(*(unsigned int*)this->dword_32.bytes, current_table_high, current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 8) {
			throw misc::Panic(misc::fmt("Can not find the correct table containing the instruction\n"));
		}
		else
			break;

	}

	this->info_32 = &current_table[thumb32_table_arg];
}


void Inst::Thumb16InstTableDecode()
{
	struct InstThumb16Info *current_table;

	// We initially start with the first table mips_asm_table, with the opcode field as argument 
	current_table = as->dec_table_thumb16_asm;
	int current_table_low = 14;
	int current_table_high = 15;
	unsigned int thumb16_table_arg;
	int loop_iteration = 0;

	thumb16_table_arg =  misc::getBits16(*(unsigned short*)this->dword_16.bytes, current_table_high, current_table_low);

	// Find next tables if the instruction belongs to another table 
	while (1) {
		if (current_table[thumb16_table_arg].next_table && loop_iteration < 6) {
			current_table_high = current_table[thumb16_table_arg].next_table_high;
			current_table_low = current_table[thumb16_table_arg].next_table_low;
			current_table = current_table[thumb16_table_arg].next_table;
			thumb16_table_arg = misc::getBits16(*(unsigned short*)this->dword_16.bytes, current_table_high, current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 6) {
			throw misc::Panic(misc::fmt("Can not find the correct table containing the instruction\n"));
		}
		else

			break;

	}

	this->info_16 = &current_table[thumb16_table_arg];
}


void Inst::Decode(unsigned int addr, const char *buf)
{

	unsigned int arg1;
	unsigned int arg2;

	unsigned char *as_char = (unsigned char *)buf;
	unsigned int *as_word = (unsigned int *)buf;

	this->addr = addr;
	this->dword.word = *as_word;
	arg1 = ((as_char[3] & 0x0f) << 4) | ((as_char[2] & 0xf0) >> 4);
	arg2 = ((as_char[0] & 0xf0) >> 4);
	this->info = &as->inst_info[arg1 * 16 + arg2];
}


void Inst::Thumb16Decode(const char *buf, unsigned int ip)
{
	unsigned int byte_index;
	this->addr = ip;
	for (byte_index = 0; byte_index < 2; ++byte_index)
		this->dword_16.bytes[byte_index] = *(unsigned char *) (buf + byte_index);

	Thumb16InstTableDecode();
}


void Inst::Thumb32Decode(const char *buf, unsigned int ip)
{
	unsigned int byte_index;
	this->addr = ip;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		this->dword_32.bytes[byte_index] = *(unsigned char *) (buf
			+ ((byte_index + 2) % 4));

	Thumb32InstTableDecode();
}

} // namespace ARM



