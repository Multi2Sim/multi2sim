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
#include "frm2bin.h"
#include "token.h"


struct str_map_t frm_token_map =
{
	frm_token_count,
	{
		{ "<invalid>", frm_token_invalid },

		/* 1st level token */
		{ "\%pred", frm_token_pred },
		{ "\%dst", frm_token_dst },
		{ "\%src1", frm_token_src1 },
		{ "\%src1_offs", frm_token_src1_offs },
		{ "\%src1_neg", frm_token_src1_neg },
		{ "\%src2", frm_token_src2 },
		{ "\%src2_neg", frm_token_src2_neg },
		{ "\%src2_FFMA", frm_token_src2_FFMA },
		{ "\%src3_FFMA", frm_token_src3_FFMA },
		{ "\%src2_frm_sr", frm_token_src2_frm_sr },
		{ "\%tgt", frm_token_tgt },
		{ "\%tgt_noinc", frm_token_tgt_noinc },
		{ "\%tgt_u", frm_token_tgt_u },
		{ "\%tgt_lmt", frm_token_tgt_lmt },
		{ "\%imm32", frm_token_imm32 },
		{ "\%offs", frm_token_offs },
		{ "\%gen1_cmp", frm_token_gen1_cmp },
		{ "\%gen1_logic", frm_token_gen1_logic },
		{ "\%gen1_logicftz", frm_token_gen1_logicftz },
		{ "\%R", frm_token_R },

		/* 2nd level token */
		{ "\%mod0_A_ftz", frm_token_mod0_A_ftz },
		{ "\%mod0_A_redarv", frm_token_mod0_A_redarv },
		{ "\%mod0_A_u32", frm_token_mod0_A_u32 },
		{ "\%mod0_A_w", frm_token_mod0_A_w },
		{ "\%mod0_A_op", frm_token_mod0_A_op },
		{ "\%mod0_A_s", frm_token_mod0_A_s },
		{ "\%mod0_A_stype_s", frm_token_mod0_A_stype_s },
		{ "\%mod0_A_dtype_s", frm_token_mod0_A_dtype_s },
		{ "\%mod0_B_u32", frm_token_mod0_B_u32 },
		{ "\%mod0_B_brev", frm_token_mod0_B_brev },
		{ "\%mod0_B_cop", frm_token_mod0_B_cop },
		{ "\%mod0_B_type", frm_token_mod0_B_type },
		{ "\%mod0_C_s", frm_token_mod0_C_s },
		{ "\%mod0_C_shamt", frm_token_mod0_C_shamt },
		{ "CC\%mod0_C_ccop", frm_token_mod0_C_ccop },
		{ "\%mod0_D_ftzfmz", frm_token_mod0_D_ftzfmz },
		{ "\%mod0_D_sat", frm_token_mod0_D_sat },
		{ "\%mod0_D_op", frm_token_mod0_D_op },
		{ "\%mod0_D_op67", frm_token_mod0_D_op67 },
		{ "\%mod0_D_x", frm_token_mod0_D_x },
		{ "\%gen0_mod1_B_rnd", frm_token_gen0_mod1_B_rnd },
		{ "\%gen0_mod1_D_cmp", frm_token_gen0_mod1_D_cmp },
		{ "\%gen0_mod1_D", frm_token_gen0_mod1_D },
		{ "\%gen0_src1_dtype", frm_token_gen0_src1_dtype },
		{ "\%gen0_src1_dtype_n", frm_token_gen0_src1_dtype_n },
		{ "\%gen0_src1_stype_n", frm_token_gen0_src1_stype_n },
		{ "\%offs_op1_e", frm_token_offs_op1_e },
		{ "\%P", frm_token_P },
		{ "\%Q", frm_token_Q },
		{ "\%src3", frm_token_src3 },
		{ "\%FADD_sat", frm_token_FADD_sat },
		{ "\%MUFU_op", frm_token_MUFU_op },
		{ "\%NOP_op", frm_token_NOP_op },
		{ "\%x", frm_token_x },
		{ "\%IMAD_mod1", frm_token_IMAD_mod1 },
		{ "\%IMAD_mod2", frm_token_IMAD_mod2 },
		{ "\%IMAD_hi", frm_token_IMAD_hi },
		{ "\%IMAD_sat", frm_token_IMAD_sat },
		{"\%stype_sn", frm_token_stype_sn},
		{"\%dtype_sn", frm_token_dtype_sn},

		/* temporially, may be deleted later */
		{ "\%gen0_mod_data_width", frm_token_mod_data_width },
		{ "\%gen0_mod_logic", frm_token_mod_logic },
		{ "\%gen0_mod_comparison", frm_token_mod_comparison },
		{ "\%src1+\%offs", frm_token_src1_offs },
	}
};


struct frm_token_t *frm_token_create(enum frm_token_type_t type)
{
	struct frm_token_t *token;

	token = xcalloc(1, sizeof(struct frm_token_t));
	token->type = type;
	return token;
}


void frm_token_free(struct frm_token_t *token)
{
	free(token);
}


int frm_token_is_arg_allowed(struct frm_token_t *token, struct frm_arg_t *arg)
{
	/* always return right. This should be modified later */
  	return 1;
	//switch (token->type)
	//{

	//case frm_token_simm16:
	//	return arg->type == frm_arg_literal;

	//case frm_token_64_sdst:
	//	return arg->type == frm_arg_scalar_register_series ||
	//			arg->type == frm_arg_special_register;

	//case frm_token_64_ssrc0:
	//case frm_token_64_ssrc1:
	//	return arg->type == frm_arg_scalar_register_series ||
	//			arg->type == frm_arg_literal ||
	//			arg->type == frm_arg_special_register;
	//
	//case frm_token_label:
	//	return arg->type == frm_arg_label;

	//case frm_token_mt_maddr:
	//	return arg->type == frm_arg_maddr;

	//case frm_token_mt_series_vdata:
	//	return arg->type == frm_arg_vector_register ||
	//		arg->type == frm_arg_vector_register_series;

	//case frm_token_offset:
	//	return arg->type == frm_arg_literal ||
	//		arg->type == frm_arg_scalar_register;

	//case frm_token_ssrc0:
	//case frm_token_ssrc1:
	//	return arg->type == frm_arg_literal ||
	//			arg->type == frm_arg_literal_float ||
	//			arg->type == frm_arg_scalar_register ||
	//			arg->type == frm_arg_special_register;

	//case frm_token_series_sdst:
	//case frm_token_series_sbase:
	//case frm_token_series_srsrc:
	//	return arg->type == frm_arg_scalar_register_series;

	//case frm_token_sdst:
	//case frm_token_smrd_sdst:
	//	return arg->type == frm_arg_scalar_register;

	//case frm_token_src0:

	//	/* Token 'src' does not accept 'abs' function */
	//	if (arg->abs)
	//		return 0;

	//	return arg->type == frm_arg_literal ||
	//		arg->type == frm_arg_literal_float ||
	//		arg->type == frm_arg_vector_register ||
	//		arg->type == frm_arg_scalar_register;

	//case frm_token_vaddr:
	//	return arg->type == frm_arg_vector_register_series ||
	//		arg->type == frm_arg_vector_register;

	//case frm_token_vdst:
	//case frm_token_vop3_vdst:
	//case frm_token_vsrc0:
	//	return arg->type == frm_arg_vector_register;

	//case frm_token_vsrc1:
	//	return arg->type == frm_arg_vector_register ||
	//		arg->type == frm_arg_literal ||
	//		arg->type == frm_arg_literal_float;

	//case frm_token_vop3_64_svdst:
	//	return arg->type == frm_arg_scalar_register_series ||
	//		arg->type == frm_arg_special_register;

	//case frm_token_vop3_src0:
	//case frm_token_vop3_src1:
	//case frm_token_vop3_src2:
	//	return arg->type == frm_arg_literal ||
	//		arg->type == frm_arg_literal_float ||
	//		arg->type == frm_arg_vector_register ||
	//		arg->type == frm_arg_scalar_register;
	//
	//case frm_token_vcc:
	//	return arg->type == frm_arg_special_register &&
	//		arg->value.special_register.type == frm_arg_special_register_vcc;
	//
	//case frm_token_wait_cnt:
	//	return arg->type == frm_arg_waitcnt;
	//
	//default:
	//	frm2bin_yyerror_fmt("%s: unsupported token (code = %d)",
	//			__FUNCTION__, token->type);
	//	return 0;
	//}
}

