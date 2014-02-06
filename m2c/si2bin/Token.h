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

#ifndef M2C_SI2BIN_TOKEN_H
#define M2C_SI2BIN_TOKEN_H

#include <lib/cpp/Misc.h>


namespace si2bin
{

/* Forward declarations */
class Arg;


extern misc::StringMap token_type_map;
enum TokenType
{
	TokenInvalid = 0,

	Token64Sdst,
	Token64Ssrc0,
	Token64Ssrc1,
	Token64Src0,
	Token64Src1,
	Token64Src2,
	Token64Svdst,
	Token64Vdst,
	TokenLabel,
	TokenMtMaddr,
	TokenOffset,
	TokenSdst,
	TokenSeriesSbase,
	TokenSeriesSdst,
	TokenSeriesSrsrc,
	TokenSimm16,
	TokenSmrdSdst,
	TokenSrc0,
	TokenSrc1,
	TokenSrc2,
	TokenSsrc0,
	TokenSsrc1,
	TokenVaddr,
	TokenVcc,
	TokenVdst,
	TokenSvdst,
	TokenVop364Svdst,
	TokenVop3Src0,
	TokenVop3Src1,
	TokenVop3Src2,
	TokenVop364Src0,
	TokenVop364Src1,
	TokenVop364Src2,
	TokenVop364Sdst,
	TokenVop3Vdst,
	TokenVop364Vdst,
	TokenVsrc0,
	TokenVsrc1,
	TokenWaitCnt,
	TokenAddr,
	TokenData0,
	TokenDsVdst,
	TokenVop2Lit,
	Token64Vsrc0,
	Token64Vsrc1,
	TokenFixme,
	TokenVIntrpVdst,
	TokenVSrcIJ,
	TokenAttr,
	TokenAttrChan,
	TokenData1,
	TokenOffset0,
	TokenOffset1,
	TokenDsSeriesVdst,
	TokenMuSeriesVdataDst,
	TokenMuSeriesVdataSrc,
	TokenMuMaddr,
	TokenMuGlc,
	TokenMtSeriesVdataDst,
	TokenMtSeriesVdataSrc,
	TokenMimgVaddr,
	TokenMimgSeriesSrsrc,
	TokenMimgDugSeriesSrsrc,
	TokenMimgDugSeriesSsamp,
	TokenMimgSeriesVdataDst,
	TokenMimgSeriesVdataSrc,
	TokenTgt,
	TokenExpVSrcs,

	TokenCount
};


enum TokenDirection
{
	TokenDirectionInvalid = 0,

	TokenDirectionDst,
	TokenDirectionSrc,
	TokenDirectionOther,

	TokenDirectionCount
};


class Token
{
	TokenType type;
	TokenDirection direction;
public:
	Token(TokenType type);

	/* Getters */
	TokenType GetType() { return type; }
	TokenDirection GetDirection() { return direction; }

	/* Return true if this token accepts the Southern Islands argument
	 * passed in the function. */
	bool IsArgAllowed(Arg *arg);
};


}  /* namespace si2bin */

#endif
