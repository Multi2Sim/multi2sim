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

#include <lib/cpp/Misc.h>

#include "Arg.h"
#include "Token.h"


using namespace misc;

namespace si2bin
{

StringMap token_type_map =
{
	{ "<invalid>", TokenInvalid },

	{ "\%64_sdst", Token64Sdst },
	{ "\%64_ssrc0", Token64Ssrc0 },
	{ "\%64_ssrc1", Token64Ssrc1 },
	{ "\%64_src0", Token64Src0 },
	{ "\%64_src1", Token64Src1 },
	{ "\%64_src2", Token64Src2 },
	{ "\%64_vsrc0", Token64Vsrc0 },
	{ "\%64_vsrc1", Token64Vsrc1 },
	{ "\%64_svdst", Token64Svdst },
	{ "\%64_vdst", Token64Vdst },
	{ "\%label", TokenLabel },
	{ "\%mt_maddr", TokenMtMaddr },
	{ "\%offset", TokenOffset },
	{ "\%sdst", TokenSdst },
	{ "\%series_sbase", TokenSeriesSbase },
	{ "\%series_sdst", TokenSeriesSdst },
	{ "\%series_srsrc", TokenSeriesSrsrc },
	{ "\%simm16", TokenSimm16 },
	{ "\%smrd_sdst", TokenSmrdSdst },
	{ "\%src0", TokenSrc0 },
	{ "\%src1", TokenSrc1 },
	{ "\%src2", TokenSrc2 },
	{ "\%ssrc0", TokenSsrc0 },
	{ "\%ssrc1", TokenSsrc1 },
	{ "\%vaddr", TokenVaddr },
	{ "\%svdst", TokenSvdst },
	{ "\%vdst", TokenVdst },
	{ "\%vop3_64_svdst", TokenVop364Svdst },
	{ "\%vop3_src0", TokenVop3Src0 },
	{ "\%vop3_src1", TokenVop3Src1 },
	{ "\%vop3_src2", TokenVop3Src2 },
	{ "\%vop3_64_src0", TokenVop364Src0 },
	{ "\%vop3_64_src1", TokenVop364Src1 },
	{ "\%vop3_64_src2", TokenVop364Src2 },
	{ "\%vop3_64_sdst", TokenVop364Sdst },
	{ "\%vop3_vdst", TokenVop3Vdst },
	{ "\%vop3_64_vdst", TokenVop364Vdst },
	{ "\%vsrc0", TokenVsrc0 },
	{ "\%vsrc1", TokenVsrc1 },
	{ "\%wait_cnt", TokenWaitCnt },
	{ "\%addr", TokenAddr },
	{ "\%data0", TokenData0 },
	{ "\%data1", TokenData1 },
	{ "\%ds_vdst", TokenDsVdst },
	{ "vcc", TokenVcc },
	{ "fixme", TokenFixme },
	{ "\%vop2_lit", TokenVop2Lit },
	{ "\%vintrp_vdst", TokenVIntrpVdst },
	{ "\%vsrc_i_j", TokenVSrcIJ },
	{ "\%attr", TokenAttr },
	{ "\%attrchan", TokenAttrChan },
	{ "\%offset0", TokenOffset0 },
	{ "\%offset1", TokenOffset1 },
	{ "\%ds_series_vdst", TokenDsSeriesVdst },
	{ "\%mu_series_vdata_dst", TokenMuSeriesVdataDst },
	{ "\%mu_series_vdata_src", TokenMuSeriesVdataSrc },
	{ "\%mu_maddr", TokenMuMaddr },
	{ "\%mu_glc", TokenMuGlc },
	{ "\%mt_series_vdata_dst", TokenMtSeriesVdataDst },
	{ "\%mt_series_vdata_src", TokenMtSeriesVdataSrc },
	{ "\%mimg_vaddr", TokenMimgVaddr },
	{ "\%mimg_series_srsrc", TokenMimgSeriesSrsrc },
	{ "\%mimg_dug_series_srsrc", TokenMimgDugSeriesSrsrc },
	{ "\%mimg_dug_series_ssamp", TokenMimgDugSeriesSsamp },
	{ "\%mimg_series_vdata_dst", TokenMimgSeriesVdataDst },
	{ "\%mimg_series_vdata_src", TokenMimgSeriesVdataSrc },
	{ "\%tgt", TokenTgt },
	{ "\%exp_vsrcs", TokenExpVSrcs }
};


Token::Token(TokenType type)
{
	/* Initialize */
	this->type = type;

	/* Set direction */
	switch (type)
	{

	case Token64Sdst:
	case Token64Svdst:
	case Token64Vdst:
	case TokenSdst:
	case TokenSeriesSdst:
	case TokenSmrdSdst:
	case TokenSvdst:
	case TokenVdst:
	case TokenVop364Svdst:
	case TokenVop364Sdst:
	case TokenVop3Vdst:
	case TokenVop364Vdst:
	case TokenDsVdst:
	case TokenDsSeriesVdst:
	case TokenMuSeriesVdataDst:
	case TokenMtSeriesVdataDst:
	case TokenMimgSeriesVdataDst:
	case TokenVIntrpVdst:
		this->direction = TokenDirectionDst;
		break;

	case Token64Ssrc0:
	case Token64Ssrc1:
	case Token64Src0:
	case Token64Src1:
	case Token64Src2:
	case TokenSeriesSrsrc:
	case TokenSrc0:
	case TokenSrc1:
	case TokenSrc2:
	case TokenSsrc0:
	case TokenSsrc1:
	case TokenVop3Src0:
	case TokenVop3Src1:
	case TokenVop3Src2:
	case TokenVop364Src0:
	case TokenVop364Src1:
	case TokenVop364Src2:
	case TokenVsrc0:
	case TokenVsrc1:
	case TokenMtMaddr:
	case TokenMuMaddr:
	case Token64Vsrc0:
	case Token64Vsrc1:
	case TokenMimgSeriesSrsrc:
	case TokenMimgDugSeriesSrsrc:
	case TokenMimgDugSeriesSsamp:
	case TokenExpVSrcs:
	case TokenMtSeriesVdataSrc:
	case TokenMuSeriesVdataSrc:
	case TokenMimgSeriesVdataSrc:
	case TokenMimgVaddr:
	case TokenAddr:
	case TokenVaddr:
	case TokenSeriesSbase:
	case TokenVSrcIJ:
	case TokenData0:
	case TokenData1:
		this->direction = TokenDirectionSrc;
		break;

	case TokenLabel:
	case TokenOffset:
	case TokenOffset0:
	case TokenOffset1:
	case TokenSimm16:
	case TokenWaitCnt:
	case TokenVcc:
	case TokenVop2Lit:
	case TokenFixme:
	case TokenAttr:
	case TokenAttrChan:
	case TokenMuGlc:
	case TokenTgt:
		this->direction = TokenDirectionOther;
		break;


	/* Need the rest, finish with Rafa */
	default:
		direction = TokenDirectionInvalid;
		//panic("%s: token type not supported", __FUNCTION__);
	}

}


bool Token::IsArgAllowed(Arg *arg)
{
	/* FIXME
	 * Some tokens that currently allow for literal and literal_float
	 * actually should change to literal_reduced and literal_float_reduced.
	 * Some others should extend it to literal_reduced and
	 * literal_float_reduced (such as src0) */
	ArgType t = arg->getType();
	switch (type)
	{

	case TokenSimm16:
		return t == ArgTypeLiteral ||
			t == ArgTypeLiteralReduced ||
			t == ArgTypeLiteralFloat ||
			t == ArgTypeLiteralFloatReduced;
		/*TODO - Check if this is correct */

	case Token64Sdst:
		return t == ArgTypeScalarRegisterSeries ||
				t == ArgTypeSpecialRegister;

	case Token64Ssrc0:
	case Token64Ssrc1:
		return t == ArgTypeScalarRegisterSeries ||
				t == ArgTypeLiteral ||
				t == ArgTypeLiteralReduced ||
				t == ArgTypeLiteralFloat ||
				t == ArgTypeLiteralFloatReduced ||
				t == ArgTypeSpecialRegister;
	
	case TokenLabel:
		return t == ArgTypeLabel;

	case TokenMtMaddr:
		return t == ArgTypeMaddr;

	case TokenMtSeriesVdataSrc:
	case TokenMtSeriesVdataDst:
		return t == ArgTypeVectorRegister ||
			t == ArgTypeVectorRegisterSeries;

	case TokenOffset:
		return t == ArgTypeLiteral ||
			t == ArgTypeLiteralReduced ||
			t == ArgTypeScalarRegister;

	case TokenSsrc0:
	case TokenSsrc1:
		return t == ArgTypeLiteral ||
				t == ArgTypeLiteralReduced ||
				t == ArgTypeLiteralFloat ||
				t == ArgTypeLiteralFloatReduced ||
				t == ArgTypeScalarRegister ||
				t == ArgTypeSpecialRegister;

	case TokenSeriesSdst:
	case TokenSeriesSbase:
	case TokenSeriesSrsrc:
		return t == ArgTypeScalarRegisterSeries;

	case TokenSdst:
	case TokenSmrdSdst:
		return t == ArgTypeScalarRegister ||
			t == ArgTypeMemRegister;

	case TokenSrc0:

		/* Token 'src' does not accept 'abs' of 'neg' function */
		if (arg->getAbs())
			return false;
		if (arg->getNeg())
			return false;

		return t == ArgTypeLiteral ||
			t == ArgTypeLiteralReduced ||
			t == ArgTypeLiteralFloat ||
			t == ArgTypeLiteralFloatReduced ||
			t == ArgTypeVectorRegister ||
			t == ArgTypeScalarRegister;

	case TokenVaddr:
		return t == ArgTypeVectorRegisterSeries ||
			t == ArgTypeVectorRegister;

	case TokenVdst:
	case TokenVsrc0:
		
		/* Token 'src' does not accept 'abs' of 'neg' function */
		if (arg->getAbs())
			return false;
		if (arg->getNeg())
			return false;

		return t == ArgTypeVectorRegister;
	
	case TokenVop3Vdst:
		return t == ArgTypeVectorRegister;
	
	case Token64Src0:
		return t == ArgTypeVectorRegisterSeries ||
			t == ArgTypeScalarRegisterSeries;

	case Token64Vdst:
		return t == ArgTypeVectorRegisterSeries;

	case TokenSvdst:
		return t == ArgTypeScalarRegister;

	case TokenVsrc1:
		
		/* Token 'src' does not accept 'abs' of 'neg' function */
		if (arg->getAbs())
			return false;
		if (arg->getNeg())
			return false;
		
		return t == ArgTypeVectorRegister;

	case TokenVop364Svdst:
		return t == ArgTypeScalarRegisterSeries ||
			t == ArgTypeSpecialRegister;

	case TokenVop3Src0:
	case TokenVop3Src1:
	case TokenVop3Src2:
		return t == ArgTypeLiteralReduced ||
			t == ArgTypeLiteralFloatReduced ||
			t == ArgTypeVectorRegister ||
			t == ArgTypeScalarRegister;
	
	case TokenVop364Src0:
	case TokenVop364Src1:
	case TokenVop364Src2:
		return t == ArgTypeScalarRegisterSeries ||
			t == ArgTypeVectorRegisterSeries ||
			t == ArgTypeSpecialRegister;
	
	case TokenVop364Vdst:
		return t == ArgTypeVectorRegisterSeries;
	
	case TokenVop364Sdst:
		return t == ArgTypeSpecialRegister;
	
	case TokenVcc:
	{
		ArgSpecialRegister *arg_special_register =
				dynamic_cast<ArgSpecialRegister *>(arg);
		return arg_special_register && arg_special_register->getReg() ==
				SI::InstSpecialRegVcc;
	}
	
	case TokenWaitCnt:
		return t == ArgTypeWaitCnt;
	
	case TokenAddr:
	case TokenData0:
	case TokenDsVdst:
		return t == ArgTypeVectorRegister;

	// New tokens
	case TokenVop2Lit:
	case Token64Vsrc0:
	case Token64Vsrc1:
	case TokenFixme:
	case TokenVIntrpVdst:
	case TokenVSrcIJ:
	case TokenAttr:
	case TokenAttrChan:
	case TokenData1:
	case TokenOffset0:
	case TokenOffset1:
	case TokenDsSeriesVdst:
	case TokenMuSeriesVdataDst:
	case TokenMuSeriesVdataSrc:
	case TokenMuMaddr:
	case TokenMuGlc:
	case TokenMimgVaddr:
	case TokenMimgSeriesSrsrc:
	case TokenMimgDugSeriesSrsrc:
	case TokenMimgDugSeriesSsamp:
	case TokenTgt:
	case TokenExpVSrcs:
		return 1;
	

	default:
		panic("%s: unsupported token (%s)",
				__FUNCTION__,
				token_type_map.MapValue(type));
		return 0;
	}
}


}  /* namespace si2bin */
