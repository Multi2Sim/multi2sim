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

#include <lib/class/class.h>


/*
 * Class 'Si2binToken'
 */

typedef enum
{
	Si2binTokenInvalid = 0,

	Si2binToken64Sdst,
	Si2binToken64Ssrc0,
	Si2binToken64Ssrc1,
	Si2binToken64Src0,
	Si2binToken64Src1,
	Si2binToken64Src2,
	Si2binToken64Svdst,
	Si2binToken64Vdst,
	Si2binTokenLabel,
	Si2binTokenMtMaddr,
	Si2binTokenMtSeriesVdata,
	Si2binTokenOffset,
	Si2binTokenSdst,
	Si2binTokenSeriesSbase,
	Si2binTokenSeriesSdst,
	Si2binTokenSeriesSrsrc,
	Si2binTokenSimm16,
	Si2binTokenSmrdSdst,
	Si2binTokenSrc0,
	Si2binTokenSrc1,
	Si2binTokenSrc2,
	Si2binTokenSsrc0,
	Si2binTokenSsrc1,
	Si2binTokenVaddr,
	Si2binTokenVcc,
	Si2binTokenVdst,
	Si2binTokenSvdst,
	Si2binTokenVop364Svdst,
	Si2binTokenVop3Src0,
	Si2binTokenVop3Src1,
	Si2binTokenVop3Src2,
	Si2binTokenVop364Src0,
	Si2binTokenVop364Src1,
	Si2binTokenVop364Src2,
	Si2binTokenVop364Sdst,
	Si2binTokenVop3Vdst,
	Si2binTokenVop364Vdst,
	Si2binTokenVsrc0,
	Si2binTokenVsrc1,
	Si2binTokenWaitCnt,
	Si2binTokenTokenAddr,
	Si2binTokenData0,
	Si2binTokenDsVdst,

	Si2binTokenCount
} Si2binTokenType;


CLASS_BEGIN(Si2binToken, Object)

	Si2binTokenType type;

CLASS_END(Si2binToken)


void Si2binTokenCreate(Si2binToken *self, Si2binTokenType type);
void Si2binTokenDestroy(Si2binToken *self);

int Si2binTokenIsArgAllowed(Si2binToken *token, Si2binArg *arg);




/*
 * Public
 */

extern struct str_map_t si2bin_token_map;


#endif

