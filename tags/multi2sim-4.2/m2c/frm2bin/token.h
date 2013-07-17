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

#ifndef M2C_FRM2BIN_TOKEN_H
#define M2C_FRM2BIN_TOKEN_H

enum frm_token_type_t
{
	frm_token_invalid = 0,

	/* 1st level token, see fermi/asm/asm.c "frm_inst_dump" func */
	frm_token_pred,
	frm_token_dst,
	frm_token_src1,
	frm_token_src1_neg,
	frm_token_src1_offs, /* for global-mem[] */
	frm_token_src2,
	frm_token_src2_neg,
	frm_token_src2_FFMA,
	frm_token_src3_FFMA,
	frm_token_src2_frm_sr,
	frm_token_tgt,
	frm_token_tgt_noinc,
	frm_token_tgt_u,
	frm_token_tgt_lmt,
	frm_token_imm32,
	frm_token_offs,
	frm_token_gen1_cmp,
	frm_token_gen1_logic,
	frm_token_gen1_logicftz,
	frm_token_R,
	/* 2nd level token */
	frm_token_mod0_A_ftz,
	frm_token_mod0_A_redarv,
	frm_token_mod0_A_u32,
	frm_token_mod0_A_w,
	frm_token_mod0_A_op,
	frm_token_mod0_A_s,
	frm_token_mod0_A_stype_s,
	frm_token_mod0_A_dtype_s,
	frm_token_mod0_B_u32,
	frm_token_mod0_B_brev,
	frm_token_mod0_B_cop,
	frm_token_mod0_B_type,
	frm_token_mod0_C_s,
	frm_token_mod0_C_shamt,
	frm_token_mod0_C_ccop,
	frm_token_mod0_D_ftzfmz,
	frm_token_mod0_D_sat,
	frm_token_mod0_D_op,
	frm_token_mod0_D_op67,
	frm_token_mod0_D_x,
	frm_token_gen0_mod1_B_rnd,
	frm_token_gen0_mod1_D_cmp,
	frm_token_gen0_mod1_D,
	frm_token_gen0_src1_dtype,
	frm_token_gen0_src1_dtype_n,
	frm_token_gen0_src1_stype_n,
	frm_token_gen0_dst_cc,
	frm_token_offs_op1_e,
	frm_token_offs_mod1_A_trig,
	frm_token_offs_mod1_A_op,
	frm_token_P,
	frm_token_Q,
	frm_token_src3,
	frm_token_FADD_sat,
	frm_token_MUFU_op,
	frm_token_NOP_op,
	frm_token_x,
	frm_token_IMAD_mod1,
	frm_token_IMAD_mod2,
	frm_token_IMAD_hi,
	frm_token_IMAD_sat,
	frm_token_stype_sn,
	frm_token_dtype_sn,

	/* temporialy, will be deleted later */
	frm_token_mod_data_width,
	frm_token_mod_logic,
	frm_token_mod_comparison,

	frm_token_count
};

extern struct str_map_t frm_token_map;

struct frm_token_t
{
	enum frm_token_type_t type;
};

struct frm_token_t *frm_token_create(enum frm_token_type_t type);
void frm_token_free(struct frm_token_t *token);

struct frm_arg_t;
int frm_token_is_arg_allowed(struct frm_token_t *token, struct frm_arg_t *arg);

#endif
