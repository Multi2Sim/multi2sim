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
#include <lib/util/string.h>

#include "arg.h"
#include "main.h"
#include "token.h"


struct str_map_t si_token_map =
{
	si_token_count,
	{
		{ "<invalid>", si_token_invalid },

		{ "\%64_sdst", si_token_64_sdst },
		{ "\%64_ssrc0", si_token_64_ssrc0 },
		{ "\%64_ssrc1", si_token_64_ssrc1 },
		{ "\%64_src0", si_token_64_src0 },
		{ "\%64_src1", si_token_64_src1 },
		{ "\%64_src2", si_token_64_src2 },
		{ "\%64_svdst", si_token_64_svdst },
		{ "\%64_vdst", si_token_64_vdst },
		{ "\%label", si_token_label },
		{ "\%mt_maddr", si_token_mt_maddr },
		{ "\%mt_series_vdata", si_token_mt_series_vdata },
		{ "\%offset", si_token_offset },
		{ "\%sdst", si_token_sdst },
		{ "\%series_sbase", si_token_series_sbase },
		{"\%series_sdst", si_token_series_sdst },
		{ "\%series_srsrc", si_token_series_srsrc },
		{ "\%simm16", si_token_simm16 },
		{ "\%smrd_sdst", si_token_smrd_sdst },
		{ "\%src0", si_token_src0 },
		{ "\%src1", si_token_src1 },
		{ "\%src2", si_token_src2 },
		{ "\%ssrc0", si_token_ssrc0 },
		{ "\%ssrc1", si_token_ssrc1 },
		{ "\%vaddr", si_token_vaddr },
		{ "\%vdst", si_token_vdst },
		{ "\%vop3_64_svdst", si_token_vop3_64_svdst },
		{ "\%vop3_src0", si_token_vop3_src0 },
		{ "\%vop3_src1", si_token_vop3_src1 },
		{ "\%vop3_src2", si_token_vop3_src2 },
		{ "\%vop3_vdst", si_token_vop3_vdst },
		{ "\%vsrc0", si_token_vsrc0 },
		{ "\%vsrc1", si_token_vsrc1 },
		{ "\%wait_cnt", si_token_wait_cnt },
		{ "vcc", si_token_vcc }
	}
};


struct si_token_t *si_token_create(enum si_token_type_t type)
{
	struct si_token_t *token;

	token = xcalloc(1, sizeof(struct si_token_t));
	token->type = type;
	return token;
}


void si_token_free(struct si_token_t *token)
{
	free(token);
}


int si_token_is_arg_allowed(struct si_token_t *token, struct si_arg_t *arg)
{
	switch (token->type)
	{

	case si_token_simm16:
		return arg->type == si_arg_literal;

	case si_token_64_sdst:
		return arg->type == si_arg_scalar_register_series ||
				arg->type == si_arg_special_register;

	case si_token_64_ssrc0:
	case si_token_64_ssrc1:
		return arg->type == si_arg_scalar_register_series ||
				arg->type == si_arg_literal ||
				arg->type == si_arg_special_register;
	
	case si_token_label:
		return arg->type == si_arg_label;

	case si_token_mt_maddr:
		return arg->type == si_arg_maddr;

	case si_token_mt_series_vdata:
		return arg->type == si_arg_vector_register ||
			arg->type == si_arg_vector_register_series;

	case si_token_offset:
		return arg->type == si_arg_literal ||
			arg->type == si_arg_scalar_register;

	case si_token_ssrc0:
	case si_token_ssrc1:
		return arg->type == si_arg_literal ||
				arg->type == si_arg_literal_float ||
				arg->type == si_arg_scalar_register ||
				arg->type == si_arg_special_register;

	case si_token_series_sdst:
	case si_token_series_sbase:
	case si_token_series_srsrc:
		return arg->type == si_arg_scalar_register_series;

	case si_token_sdst:
	case si_token_smrd_sdst:
		return arg->type == si_arg_scalar_register;

	case si_token_src0:

		/* Token 'src' does not accept 'abs' function */
		if (arg->abs)
			return 0;

		return arg->type == si_arg_literal ||
			arg->type == si_arg_literal_float ||
			arg->type == si_arg_vector_register ||
			arg->type == si_arg_scalar_register;

	case si_token_vaddr:
		return arg->type == si_arg_vector_register_series ||
			arg->type == si_arg_vector_register;

	case si_token_vdst:
	case si_token_vop3_vdst:
	case si_token_vsrc0:
		return arg->type == si_arg_vector_register;

	case si_token_vsrc1:
		return arg->type == si_arg_vector_register ||
			arg->type == si_arg_literal ||
			arg->type == si_arg_literal_float;

	case si_token_vop3_64_svdst:
		return arg->type == si_arg_scalar_register_series ||
			arg->type == si_arg_special_register;

	case si_token_vop3_src0:
	case si_token_vop3_src1:
	case si_token_vop3_src2:
		return arg->type == si_arg_literal ||
			arg->type == si_arg_literal_float ||
			arg->type == si_arg_vector_register ||
			arg->type == si_arg_scalar_register;
	
	case si_token_vcc:
		return arg->type == si_arg_special_register &&
			arg->value.special_register.type == si_arg_special_register_vcc;
	
	case si_token_wait_cnt:
		return arg->type == si_arg_waitcnt;
	
	default:
		yyerror_fmt("%s: unsupported token (code = %d)",
				__FUNCTION__, token->type);
		return 0;
	}
}

