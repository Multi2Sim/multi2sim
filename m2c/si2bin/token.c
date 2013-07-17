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
#include "si2bin.h"
#include "token.h"


struct str_map_t si2bin_token_map =
{
	si2bin_token_count,
	{
		{ "<invalid>", si2bin_token_invalid },

		{ "\%64_sdst", si2bin_token_64_sdst },
		{ "\%64_ssrc0", si2bin_token_64_ssrc0 },
		{ "\%64_ssrc1", si2bin_token_64_ssrc1 },
		{ "\%64_src0", si2bin_token_64_src0 },
		{ "\%64_src1", si2bin_token_64_src1 },
		{ "\%64_src2", si2bin_token_64_src2 },
		{ "\%64_svdst", si2bin_token_64_svdst },
		{ "\%64_vdst", si2bin_token_64_vdst },
		{ "\%label", si2bin_token_label },
		{ "\%mt_maddr", si2bin_token_mt_maddr },
		{ "\%mt_series_vdata", si2bin_token_mt_series_vdata },
		{ "\%offset", si2bin_token_offset },
		{ "\%sdst", si2bin_token_sdst },
		{ "\%series_sbase", si2bin_token_series_sbase },
		{"\%series_sdst", si2bin_token_series_sdst },
		{ "\%series_srsrc", si2bin_token_series_srsrc },
		{ "\%simm16", si2bin_token_simm16 },
		{ "\%smrd_sdst", si2bin_token_smrd_sdst },
		{ "\%src0", si2bin_token_src0 },
		{ "\%src1", si2bin_token_src1 },
		{ "\%src2", si2bin_token_src2 },
		{ "\%ssrc0", si2bin_token_ssrc0 },
		{ "\%ssrc1", si2bin_token_ssrc1 },
		{ "\%vaddr", si2bin_token_vaddr },
		{ "\%svdst", si2bin_token_svdst },
		{ "\%vdst", si2bin_token_vdst },
		{ "\%vop3_64_svdst", si2bin_token_vop3_64_svdst },
		{ "\%vop3_src0", si2bin_token_vop3_src0 },
		{ "\%vop3_src1", si2bin_token_vop3_src1 },
		{ "\%vop3_src2", si2bin_token_vop3_src2 },
		{ "\%vop3_64_src0", si2bin_token_vop3_64_src0 },
		{ "\%vop3_64_src1", si2bin_token_vop3_64_src1 },
		{ "\%vop3_64_src2", si2bin_token_vop3_64_src2 },
		{ "\%vop3_64_sdst", si2bin_token_vop3_64_sdst },
		{ "\%vop3_vdst", si2bin_token_vop3_vdst },
		{ "\%vop3_64_vdst", si2bin_token_vop3_64_vdst },
		{ "\%vsrc0", si2bin_token_vsrc0 },
		{ "\%vsrc1", si2bin_token_vsrc1 },
		{ "\%wait_cnt", si2bin_token_wait_cnt },
		{ "\%addr", si2bin_token_addr },
		{ "\%data0", si2bin_token_data0 },
		{ "\%ds_vdst", si2bin_token_ds_vdst },
		{ "vcc", si2bin_token_vcc }
	}
};


struct si2bin_token_t *si2bin_token_create(enum si2bin_token_type_t type)
{
	struct si2bin_token_t *token;

	token = xcalloc(1, sizeof(struct si2bin_token_t));
	token->type = type;
	return token;
}


void si2bin_token_free(struct si2bin_token_t *token)
{
	free(token);
}


int si2bin_token_is_arg_allowed(struct si2bin_token_t *token, struct si2bin_arg_t *arg)
{
	/* FIXME
	 * Some tokens that currently allow for literal and literal_float
	 * actually should change to literal_reduced and literal_float_reduced.
	 * Some others should extend it to literal_reduced and
	 * literal_float_reduced (such as src0) */
	switch (token->type)
	{

	case si2bin_token_simm16:
		return arg->type == si2bin_arg_literal ||
			arg->type == si2bin_arg_literal_reduced ||
			arg->type == si2bin_arg_literal_float ||
			arg->type == si2bin_arg_literal_float_reduced;
		/*TODO - Check if this is correct */

	case si2bin_token_64_sdst:
		return arg->type == si2bin_arg_scalar_register_series ||
				arg->type == si2bin_arg_special_register;

	case si2bin_token_64_ssrc0:
	case si2bin_token_64_ssrc1:
		return arg->type == si2bin_arg_scalar_register_series ||
				arg->type == si2bin_arg_literal ||
				arg->type == si2bin_arg_literal_reduced ||
				arg->type == si2bin_arg_literal_float ||
				arg->type == si2bin_arg_literal_float_reduced ||
				arg->type == si2bin_arg_special_register;
	
	case si2bin_token_label:
		return arg->type == si2bin_arg_label;

	case si2bin_token_mt_maddr:
		return arg->type == si2bin_arg_maddr;

	case si2bin_token_mt_series_vdata:
		return arg->type == si2bin_arg_vector_register ||
			arg->type == si2bin_arg_vector_register_series;

	case si2bin_token_offset:
		return arg->type == si2bin_arg_literal ||
			arg->type == si2bin_arg_literal_reduced ||
			arg->type == si2bin_arg_scalar_register;

	case si2bin_token_ssrc0:
	case si2bin_token_ssrc1:
		return arg->type == si2bin_arg_literal ||
				arg->type == si2bin_arg_literal_reduced ||
				arg->type == si2bin_arg_literal_float ||
				arg->type == si2bin_arg_literal_float_reduced ||
				arg->type == si2bin_arg_scalar_register ||
				arg->type == si2bin_arg_special_register;

	case si2bin_token_series_sdst:
	case si2bin_token_series_sbase:
	case si2bin_token_series_srsrc:
		return arg->type == si2bin_arg_scalar_register_series;

	case si2bin_token_sdst:
	case si2bin_token_smrd_sdst:
		return arg->type == si2bin_arg_scalar_register ||
			arg->type == si2bin_arg_mem_register;

	case si2bin_token_src0:

		/* Token 'src' does not accept 'abs' of 'neg' function */
		if (arg->abs)
			return 0;
		if (arg->neg)
			return 0;

		return arg->type == si2bin_arg_literal ||
			arg->type == si2bin_arg_literal_reduced ||
			arg->type == si2bin_arg_literal_float ||
			arg->type == si2bin_arg_literal_float_reduced ||
			arg->type == si2bin_arg_vector_register ||
			arg->type == si2bin_arg_scalar_register;

	case si2bin_token_vaddr:
		return arg->type == si2bin_arg_vector_register_series ||
			arg->type == si2bin_arg_vector_register;

	case si2bin_token_vdst:
	case si2bin_token_vop3_vdst:
	case si2bin_token_vsrc0:
		return arg->type == si2bin_arg_vector_register;
	
	case si2bin_token_64_src0:
		return arg->type == si2bin_arg_vector_register_series ||
			arg->type == si2bin_arg_scalar_register_series;

	case si2bin_token_64_vdst:
		return arg->type == si2bin_arg_vector_register_series;

	case si2bin_token_svdst:
		return arg->type == si2bin_arg_scalar_register;

	case si2bin_token_vsrc1:
		
		/* Token 'src' does not accept 'abs' of 'neg' function */
		if (arg->abs)
			return 0;
		if (arg->neg)
			return 0;
		
		return arg->type == si2bin_arg_vector_register;

	case si2bin_token_vop3_64_svdst:
		return arg->type == si2bin_arg_scalar_register_series ||
			arg->type == si2bin_arg_special_register;

	case si2bin_token_vop3_src0:
	case si2bin_token_vop3_src1:
	case si2bin_token_vop3_src2:
		return arg->type == si2bin_arg_literal_reduced ||
			arg->type == si2bin_arg_literal_float_reduced ||
			arg->type == si2bin_arg_vector_register ||
			arg->type == si2bin_arg_scalar_register;
	
	case si2bin_token_vop3_64_src0:
	case si2bin_token_vop3_64_src1:
	case si2bin_token_vop3_64_src2:
		return arg->type == si2bin_arg_scalar_register_series ||
			arg->type == si2bin_arg_vector_register_series ||
			arg->type == si2bin_arg_special_register;
	
	case si2bin_token_vop3_64_vdst:
		return arg->type == si2bin_arg_vector_register_series;
	
	case si2bin_token_vop3_64_sdst:
		return arg->type == si2bin_arg_special_register;
	
	case si2bin_token_vcc:
		return arg->type == si2bin_arg_special_register &&
			arg->value.special_register.reg == si_inst_special_reg_vcc;
	
	case si2bin_token_wait_cnt:
		return arg->type == si2bin_arg_waitcnt;
	
	case si2bin_token_addr:
	case si2bin_token_data0:
	case si2bin_token_ds_vdst:
		return arg->type == si2bin_arg_vector_register;
	

	default:
		si2bin_yyerror_fmt("%s: unsupported token (code = %d)",
				__FUNCTION__, token->type);
		return 0;
	}
}

