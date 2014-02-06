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
#include <lib/util/string.h>

#include "arg.h"
#include "PVars.h"
#include "si2bin.h"
#include "token.h"


struct str_map_t si2bin_token_map =
{
	Si2binTokenCount,
	{
			{ "<invalid>", Si2binTokenInvalid },

			{ "\%64_sdst", Si2binToken64Sdst },
			{ "\%64_ssrc0", Si2binToken64Ssrc0 },
			{ "\%64_ssrc1", Si2binToken64Ssrc1 },
			{ "\%64_src0", Si2binToken64Src0 },
			{ "\%64_src1", Si2binToken64Src1 },
			{ "\%64_src2", Si2binToken64Src2 },
			{ "\%64_vsrc0", Si2binToken64Vsrc0 },
			{ "\%64_vsrc1", Si2binToken64Vsrc1 },
			{ "\%64_svdst", Si2binToken64Svdst },
			{ "\%64_vdst", Si2binToken64Vdst },
			{ "\%label", Si2binTokenLabel },
			{ "\%mt_maddr", Si2binTokenMtMaddr },
			{ "\%offset", Si2binTokenOffset },
			{ "\%sdst", Si2binTokenSdst },
			{ "\%series_sbase", Si2binTokenSeriesSbase },
			{ "\%series_sdst", Si2binTokenSeriesSdst },
			{ "\%series_srsrc", Si2binTokenSeriesSrsrc },
			{ "\%simm16", Si2binTokenSimm16 },
			{ "\%smrd_sdst", Si2binTokenSmrdSdst },
			{ "\%src0", Si2binTokenSrc0 },
			{ "\%src1", Si2binTokenSrc1 },
			{ "\%src2", Si2binTokenSrc2 },
			{ "\%ssrc0", Si2binTokenSsrc0 },
			{ "\%ssrc1", Si2binTokenSsrc1 },
			{ "\%vaddr", Si2binTokenVaddr },
			{ "\%svdst", Si2binTokenSvdst },
			{ "\%vdst", Si2binTokenVdst },
			{ "\%vop3_64_svdst", Si2binTokenVop364Svdst },
			{ "\%vop3_src0", Si2binTokenVop3Src0 },
			{ "\%vop3_src1", Si2binTokenVop3Src1 },
			{ "\%vop3_src2", Si2binTokenVop3Src2 },
			{ "\%vop3_64_src0", Si2binTokenVop364Src0 },
			{ "\%vop3_64_src1", Si2binTokenVop364Src1 },
			{ "\%vop3_64_src2", Si2binTokenVop364Src2 },
			{ "\%vop3_64_sdst", Si2binTokenVop364Sdst },
			{ "\%vop3_vdst", Si2binTokenVop3Vdst },
			{ "\%vop3_64_vdst", Si2binTokenVop364Vdst },
			{ "\%vsrc0", Si2binTokenVsrc0 },
			{ "\%vsrc1", Si2binTokenVsrc1 },
			{ "\%wait_cnt", Si2binTokenWaitCnt },
			{ "\%addr", Si2binTokenAddr },
			{ "\%data0", Si2binTokenData0 },
			{ "\%data1", Si2binTokenData1 },
			{ "\%ds_vdst", Si2binTokenDsVdst },
			{ "vcc", Si2binTokenVcc },
			{ "fixme", Si2binTokenFixme },
			{ "\%vop2_lit", Si2binTokenVop2Lit },
			{ "\%vintrp_vdst", Si2binTokenVIntrpVdst },
			{ "\%vsrc_i_j", Si2binTokenVSrcIJ },
			{ "\%attr", Si2binTokenAttr },
			{ "\%attrchan", Si2binTokenAttrChan },
			{ "\%offset0", Si2binTokenOffset0 },
			{ "\%offset1", Si2binTokenOffset1 },
			{ "\%ds_series_vdst", Si2binTokenDsSeriesVdst },
			{ "\%mu_series_vdata_dst", Si2binTokenMuSeriesVdataDst },
			{ "\%mu_series_vdata_src", Si2binTokenMuSeriesVdataSrc },
			{ "\%mu_maddr", Si2binTokenMuMaddr },
			{ "\%mu_glc", Si2binTokenMuGlc },
			{ "\%mt_series_vdata_dst", Si2binTokenMtSeriesVdataDst },
			{ "\%mt_series_vdata_src", Si2binTokenMtSeriesVdataSrc },
			{ "\%mimg_vaddr", Si2binTokenMimgVaddr },
			{ "\%mimg_series_srsrc", Si2binTokenMimgSeriesSrsrc },
			{ "\%mimg_dug_series_srsrc", Si2binTokenMimgDugSeriesSrsrc },
			{ "\%mimg_dug_series_ssamp", Si2binTokenMimgDugSeriesSsamp },
			{ "\%mimg_series_vdata_dst", Si2binTokenMimgSeriesVdataDst },
			{ "\%mimg_series_vdata_src", Si2binTokenMimgSeriesVdataSrc },
			{ "\%tgt", Si2binTokenTgt },
			{ "\%exp_vsrcs", Si2binTokenExpVSrcs }
	}
};


void Si2binTokenCreate(Si2binToken *self, Si2binTokenType type)
{
	/* Initialize */
	self->type = type;

	/* Set direction */
	switch (type)
	{

	case Si2binToken64Sdst:
	case Si2binToken64Svdst:
	case Si2binToken64Vdst:
	case Si2binTokenSdst:
	case Si2binTokenSeriesSdst:
	case Si2binTokenSmrdSdst:
	case Si2binTokenSvdst:
	case Si2binTokenVdst:
	case Si2binTokenVop364Svdst:
	case Si2binTokenVop364Sdst:
	case Si2binTokenVop3Vdst:
	case Si2binTokenVop364Vdst:
	case Si2binTokenDsVdst:
	case Si2binTokenDsSeriesVdst:
	case Si2binTokenMuSeriesVdataDst:
	case Si2binTokenMtSeriesVdataDst:
	case Si2binTokenMimgSeriesVdataDst:
	case Si2binTokenVIntrpVdst:
		self->direction = Si2binTokenDirectionDst;
		break;

	case Si2binToken64Ssrc0:
	case Si2binToken64Ssrc1:
	case Si2binToken64Src0:
	case Si2binToken64Src1:
	case Si2binToken64Src2:
	case Si2binTokenSeriesSrsrc:
	case Si2binTokenSrc0:
	case Si2binTokenSrc1:
	case Si2binTokenSrc2:
	case Si2binTokenSsrc0:
	case Si2binTokenSsrc1:
	case Si2binTokenVop3Src0:
	case Si2binTokenVop3Src1:
	case Si2binTokenVop3Src2:
	case Si2binTokenVop364Src0:
	case Si2binTokenVop364Src1:
	case Si2binTokenVop364Src2:
	case Si2binTokenVsrc0:
	case Si2binTokenVsrc1:
	case Si2binTokenMtMaddr:
	case Si2binTokenMuMaddr:
	case Si2binToken64Vsrc0:
	case Si2binToken64Vsrc1:
	case Si2binTokenMimgSeriesSrsrc:
	case Si2binTokenMimgDugSeriesSrsrc:
	case Si2binTokenMimgDugSeriesSsamp:
	case Si2binTokenExpVSrcs:
	case Si2binTokenMtSeriesVdataSrc:
	case Si2binTokenMuSeriesVdataSrc:
	case Si2binTokenMimgSeriesVdataSrc:
	case Si2binTokenMimgVaddr:
	case Si2binTokenAddr:
	case Si2binTokenVaddr:
	case Si2binTokenSeriesSbase:
	case Si2binTokenVSrcIJ:
	case Si2binTokenData0:
	case Si2binTokenData1:
		self->direction = Si2binTokenDirectionSrc;
		break;

	case Si2binTokenLabel:
	case Si2binTokenOffset:
	case Si2binTokenOffset0:
	case Si2binTokenOffset1:
	case Si2binTokenSimm16:
	case Si2binTokenWaitCnt:
	case Si2binTokenVcc:
	case Si2binTokenVop2Lit:
	case Si2binTokenFixme:
	case Si2binTokenAttr:
	case Si2binTokenAttrChan:
	case Si2binTokenMuGlc:
	case Si2binTokenTgt:
		self->direction = Si2binTokenDirectionOther;
		break;

	default:
		self->direction = Si2binTokenDirectionInvalid;
		//panic("%s: token type not supported", __FUNCTION__);
	}

}


void Si2binTokenDestroy(Si2binToken *self)
{
}


int Si2binTokenIsArgAllowed(Si2binToken *self, Si2binArg *arg)
{
	/* FIXME
	 * Some tokens that currently allow for literal and literal_float
	 * actually should change to literal_reduced and literal_float_reduced.
	 * Some others should extend it to literal_reduced and
	 * literal_float_reduced (such as src0) */
	switch (self->type)
	{

	case Si2binTokenSimm16:
		return arg->type == Si2binArgLiteral ||
			arg->type == Si2binArgLiteralReduced ||
			arg->type == Si2binArgLiteralFloat ||
			arg->type == Si2binArgLiteralFloatReduced;
		/*TODO - Check if this is correct */

	case Si2binToken64Sdst:
		return arg->type == Si2binArgScalarRegisterSeries ||
				arg->type == Si2binArgSpecialRegister;

	case Si2binToken64Ssrc0:
	case Si2binToken64Ssrc1:
		return arg->type == Si2binArgScalarRegisterSeries ||
				arg->type == Si2binArgLiteral ||
				arg->type == Si2binArgLiteralReduced ||
				arg->type == Si2binArgLiteralFloat ||
				arg->type == Si2binArgLiteralFloatReduced ||
				arg->type == Si2binArgSpecialRegister;
	
	case Si2binTokenLabel:
		return arg->type == Si2binArgLabel;

	case Si2binTokenMtMaddr:
		return arg->type == Si2binArgMaddr;

	case Si2binTokenMtSeriesVdataSrc:
	case Si2binTokenMtSeriesVdataDst:
		return arg->type == Si2binArgVectorRegister ||
			arg->type == Si2binArgVectorRegisterSeries;

	case Si2binTokenOffset:
		return arg->type == Si2binArgLiteral ||
			arg->type == Si2binArgLiteralReduced ||
			arg->type == Si2binArgScalarRegister;

	case Si2binTokenSsrc0:
	case Si2binTokenSsrc1:
		return arg->type == Si2binArgLiteral ||
				arg->type == Si2binArgLiteralReduced ||
				arg->type == Si2binArgLiteralFloat ||
				arg->type == Si2binArgLiteralFloatReduced ||
				arg->type == Si2binArgScalarRegister ||
				arg->type == Si2binArgSpecialRegister;

	case Si2binTokenSeriesSdst:
	case Si2binTokenSeriesSbase:
	case Si2binTokenSeriesSrsrc:
		return arg->type == Si2binArgScalarRegisterSeries;

	case Si2binTokenSdst:
	case Si2binTokenSmrdSdst:
		return arg->type == Si2binArgScalarRegister ||
			arg->type == Si2binArgMemRegister;

	case Si2binTokenSrc0:

		/* Token 'src' does not accept 'abs' of 'neg' function */
		if (arg->abs)
			return 0;
		if (arg->neg)
			return 0;

		return arg->type == Si2binArgLiteral ||
			arg->type == Si2binArgLiteralReduced ||
			arg->type == Si2binArgLiteralFloat ||
			arg->type == Si2binArgLiteralFloatReduced ||
			arg->type == Si2binArgVectorRegister ||
			arg->type == Si2binArgScalarRegister;

	case Si2binTokenVaddr:
		return arg->type == Si2binArgVectorRegisterSeries ||
			arg->type == Si2binArgVectorRegister;

	case Si2binTokenVdst:
	case Si2binTokenVop3Vdst:
	case Si2binTokenVsrc0:
		return arg->type == Si2binArgVectorRegister;
	
	case Si2binToken64Src0:
		return arg->type == Si2binArgVectorRegisterSeries ||
			arg->type == Si2binArgScalarRegisterSeries;

	case Si2binToken64Vdst:
		return arg->type == Si2binArgVectorRegisterSeries;

	case Si2binTokenSvdst:
		return arg->type == Si2binArgScalarRegister;

	case Si2binTokenVsrc1:
		
		/* Token 'src' does not accept 'abs' of 'neg' function */
		if (arg->abs)
			return 0;
		if (arg->neg)
			return 0;
		
		return arg->type == Si2binArgVectorRegister;

	case Si2binTokenVop364Svdst:
		return arg->type == Si2binArgScalarRegisterSeries ||
			arg->type == Si2binArgSpecialRegister;

	case Si2binTokenVop3Src0:
	case Si2binTokenVop3Src1:
	case Si2binTokenVop3Src2:
		return arg->type == Si2binArgLiteralReduced ||
			arg->type == Si2binArgLiteralFloatReduced ||
			arg->type == Si2binArgVectorRegister ||
			arg->type == Si2binArgScalarRegister;
	
	case Si2binTokenVop364Src0:
	case Si2binTokenVop364Src1:
	case Si2binTokenVop364Src2:
		return arg->type == Si2binArgScalarRegisterSeries ||
			arg->type == Si2binArgVectorRegisterSeries ||
			arg->type == Si2binArgSpecialRegister;
	
	case Si2binTokenVop364Vdst:
		return arg->type == Si2binArgVectorRegisterSeries;
	
	case Si2binTokenVop364Sdst:
		return arg->type == Si2binArgSpecialRegister;
	
	case Si2binTokenVcc:
		return arg->type == Si2binArgSpecialRegister &&
			arg->value.special_register.reg == SIInstSpecialRegVcc;
	
	case Si2binTokenWaitCnt:
		return arg->type == Si2binArgWaitcnt;
	
	case Si2binTokenAddr:
	case Si2binTokenData0:
	case Si2binTokenDsVdst:
		return arg->type == Si2binArgVectorRegister;

//	case Si2binTokenMtSeriesVdataSrc:
//	case Si2binTokenMtSeriesVdataDst:
//		return arg->type == Si2binArgVectorRegister;
	
	// New tokens
	case Si2binTokenVop2Lit:
	case Si2binToken64Vsrc0:
	case Si2binToken64Vsrc1:
	case Si2binTokenFixme:
	case Si2binTokenVIntrpVdst:
	case Si2binTokenVSrcIJ:
	case Si2binTokenAttr:
	case Si2binTokenAttrChan:
	case Si2binTokenData1:
	case Si2binTokenOffset0:
	case Si2binTokenOffset1:
	case Si2binTokenDsSeriesVdst:
	case Si2binTokenMuSeriesVdataDst:
	case Si2binTokenMuSeriesVdataSrc:
	case Si2binTokenMuMaddr:
	case Si2binTokenMuGlc:
	case Si2binTokenMimgSeriesVdataSrc:
	case Si2binTokenMimgSeriesVdataDst:
	case Si2binTokenMimgVaddr:
	case Si2binTokenMimgSeriesSrsrc:
	case Si2binTokenMimgDugSeriesSrsrc:
	case Si2binTokenMimgDugSeriesSsamp:
	case Si2binTokenTgt:
	case Si2binTokenExpVSrcs:
		return 1;
	

	default:
		si2bin_yyerror_fmt("%s: unsupported token (code = %d)",
				__FUNCTION__, self->type);
		return 0;
	}
}
