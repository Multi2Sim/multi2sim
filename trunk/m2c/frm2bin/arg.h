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

#ifndef M2C_FRM2BIN_ARG_H
#define M2C_FRM2BIN_ARG_H

#include <stdio.h>

#include "token.h"

/* Forward declarations */
struct frm_symbol_t;


enum frm_arg_type_t 
{
	frm_arg_invalid = 0,

	frm_arg_scalar_register,
	frm_arg_special_register,
	frm_arg_predicate_register,
	frm_arg_zero_register,
	frm_arg_literal,
	/*frm_arg_label,*/
	frm_arg_maddr,
	frm_arg_const_maddr,
	frm_arg_shared_maddr,
	frm_arg_glob_maddr,
	frm_arg_pt,
	frm_arg_ccop,
};

/* NOTE: modify string map 'frm_arg_special_register_map' in asm.c together
 * with this enumeration. */
enum frm_arg_special_register_type_t
{
	/* Do we still need the invalid ?? */
	//frm_arg_special_register_invalid = 0,

	/* More to be added */
	frm_arg_special_register_LaneId = 0, 
	frm_arg_special_register_VirtCfg = 2, 
	frm_arg_special_register_VirtId, 
	frm_arg_special_register_PM0, 
	frm_arg_special_register_PM1, 
	frm_arg_special_register_PM2, 
	frm_arg_special_register_PM3, 
	frm_arg_special_register_PM4, 
	frm_arg_special_register_PM5, 
	frm_arg_special_register_PM6, 
	frm_arg_special_register_PM7, 
	frm_arg_special_register_PRIM_TYPE = 16, 
	frm_arg_special_register_INVOCATION_ID, 
	frm_arg_special_register_Y_DIRECTION, 
	frm_arg_special_register_MACHINE_ID_0 = 24, 
	frm_arg_special_register_MACHINE_ID_1, 
	frm_arg_special_register_MACHINE_ID_2, 
	frm_arg_special_register_MACHINE_ID_3, 
	frm_arg_special_register_AFFINITY, 
	frm_arg_special_register_Tid = 32,  
	frm_arg_special_register_Tid_X, 
	frm_arg_special_register_Tid_Y, 
	frm_arg_special_register_Tid_Z, 
	frm_arg_special_register_CTAParam, 
	frm_arg_special_register_CTAid_X, 
	frm_arg_special_register_CTAid_Y, 
	frm_arg_special_register_CTAid_Z, 
	frm_arg_special_register_NTid, 
	frm_arg_special_register_NTid_X, 
	frm_arg_special_register_NTid_Y, 
	frm_arg_special_register_NTid_Z, 
	frm_arg_special_register_GridParam, 
	frm_arg_special_register_NCTAid_X, 
	frm_arg_special_register_NCTAid_Y, 
	frm_arg_special_register_NCTAid_Z, 
	frm_arg_special_register_SWinLo, 
	frm_arg_special_register_SWINSZ, 
	frm_arg_special_register_SMemSz, 
	frm_arg_special_register_SMemBanks, 
	frm_arg_special_register_LWinLo, 
	frm_arg_special_register_LWINSZ, 
	frm_arg_special_register_LMemLoSz, 
	frm_arg_special_register_LMemHiOff, 
	frm_arg_special_register_EqMask, 
	frm_arg_special_register_LtMask, 
	frm_arg_special_register_LeMask, 
	frm_arg_special_register_GtMask, 
	frm_arg_special_register_GeMask, 
	frm_arg_special_register_ClockLo = 80, 
	frm_arg_special_register_ClockHi, 

};

enum frm_arg_ccop_type_t
{
	frm_arg_ccop_f = 0,
	frm_arg_ccop_lt,
	frm_arg_ccop_eq,
	frm_arg_ccop_le,
	frm_arg_ccop_gt,
	frm_arg_ccop_ne,
	frm_arg_ccop_ge,
	frm_arg_ccop_num,
	frm_arg_ccop_nan,
	frm_arg_ccop_ltu,
	frm_arg_ccop_equ,
	frm_arg_ccop_leu,
	frm_arg_ccop_gtu,
	frm_arg_ccop_neu,
	frm_arg_ccop_geu,
	frm_arg_ccop_t,
	frm_arg_ccop_off,
	frm_arg_ccop_lo,
	frm_arg_ccop_sff,
	frm_arg_ccop_ls,
	frm_arg_ccop_hi,
	frm_arg_ccop_sft,
	frm_arg_ccop_hs,
	frm_arg_ccop_oft,
	frm_arg_ccop_csm_ta,
	frm_arg_ccop_csm_tr,
	frm_arg_ccop_csm_mx,
	frm_arg_ccop_fcsm_ta,
	frm_arg_ccop_fcsm_tr,
	frm_arg_ccop_fcsm_mx,
	frm_arg_ccop_rle,
	frm_arg_ccop_rgt,
};

/* logic type */
enum frm_mod_logic_type_t
{
	logic_and,
	logic_or,
	logic_xor,
};

/* mod data width */
enum frm_mod_data_width_t
{
	u16,
	s16,
	u32,
	s32,
	u64,
	s64,
};

/* mod comparison type */
enum frm_mod_comp_t
{
	frm_lt,
	frm_eq,
	frm_le,
	frm_gt,
	frm_ne,
	frm_ge,
	frm_num,
	frm_nan,
	frm_ltu,
	frm_equ,
	frm_leu,
	frm_gtu,
	frm_geu,
	frm_neu,
};

/* type def for modifier */
struct frm_mod_t
{
	enum frm_token_type_t type;
	union
	{
		enum frm_mod_data_width_t data_width;
		enum frm_mod_logic_type_t logic; 
		enum frm_mod_comp_t comparison;
		int brev;	/* bit reverse; 0-false, 1-true */
		int dst_cc;	/* dst condition code? 0-false, 1-ture */
		int tgt_u;	/* tgt_u , 0-false, 1-true */
		int tgt_lmt;	/* tgt_lmt, 0-false, 1-ture */
		int mod0_A_w;	/* 0-default, 1-w */
		int mod0_A_redarv;	/* 0-red, 1-arv */
		int mod0_A_op;	/* 0-popc, 1-and, 2-or, 3-invalid */
		int mod0_B_cop;	/* 0-ca, 1-cg, 2-lu, 3-cv */
		int mod0_B_type;	/* 0-u8, 1-s8, 2-u16, 3-s16, 4-default, 5-64, 6-128 */
		int mod0_C_s;	/* 0-default, 1-s */
		int mod0_D_sat;	/* 0-default, 1-sat */
		int mod0_D_x;	/* 0-default, 1-x */
		int mod0_D_ftzfmz;	/* 0-default, 1-ftz, 2-fmz, 3-invalid */
		int gen0_mod1_B_rnd;	/* 0-default, 1-rm, 2-rp, 3-rz */
		int gen0_src1_dtype;	/* 0-default, 1-f16, 2-f32, 3-f64 */
		int offs_mod1_A_trig;	/* 0-default, 1-trig */
		int offs_mod1_A_op;	/* 0-default, fma64, fma32, xlu, alu, agu, su, fu, fmul */
		int IMAD_mod;	/* for both IMAD_mod1 and IMAD_mod2, 0-u32, 1-s32 */
	} value;
};

struct frm_arg_t 
{
	enum frm_arg_type_t type;
	int abs;  /* Absolute value */
	int neg;  /* Negation */

	union
	{
	
		struct
		{
			unsigned int id;
		} scalar_register;

		struct
		{
			int low;
			int high;
		} scalar_register_series;

		struct
		{
			int low;
			int high;
		} vector_register_series;
		
		struct
		{
			int id;
		} 
		vector_register;
		
		struct
		{
			int val;
		} literal;
		
		struct
		{
			float val;
		} literal_float;

		struct
		{
			int vmcnt_active;
			int vmcnt_value;

			int lgkmcnt_active;
			int lgkmcnt_value;

			int expcnt_active;
			int expcnt_value;
		} wait_cnt;
		
		struct
		{
			int id;
		} mem_register;
		
		struct
		{
			/* Sub-argument of type 'vector', 'scalar', 'literal',
			 * or 'literal_float'. */
			struct frm_arg_t *soffset;

			/* Sub-argument of type 'maddr_qual'
			 * (memory address qualifier) */
			struct frm_arg_t *qual;

			char *data_format;
			char *num_format;
		} maddr;

		struct
		{
			int offen;
			int idxen;
			int offset;
		} maddr_qual;

		struct
		{
			unsigned int bank_idx;
			/* offset within the bank */
			unsigned int offset;
		} const_maddr;

		struct
		{
			unsigned int bank_idx;
			/* offset within the bank */
			unsigned int offset;
		} shared_maddr;


		struct
		{
			/* [reg + offset] */
			/* index of the register used */
			unsigned int reg_idx;
			unsigned int offset;
		} glob_maddr;

		struct
		{
			/* whether it's pt or not*/
			unsigned int idx;
		} pt;
		
		struct
		{
			int op;
		} ccop;

		struct
		{
			enum frm_arg_special_register_type_t type;
		} special_register;
		
		struct
		{
			/* P1: 1, !P1: -1*/
			int id;
		} predicate_register;

		struct
		{
			struct frm_symbol_t *symbol;
		} label;
		
	} value;
};



struct frm_arg_t *frm_arg_create(void);
void frm_arg_free(struct frm_arg_t *inst_arg);

struct frm_arg_t *frm_arg_create_literal(int value);
struct frm_arg_t *frm_arg_create_scalar_register(char* name);
struct frm_arg_t *frm_arg_create_special_register(char *name);
struct frm_arg_t *frm_arg_create_predicate_register(char *name);
struct frm_arg_t *frm_arg_create_zero_register(char *name);
struct frm_arg_t *frm_arg_create_const_maddr(int bank_idx, int offset, int negative);
struct frm_arg_t *frm_arg_create_shared_maddr(int bank_idx, int offset);
struct frm_arg_t *frm_arg_create_glob_maddr(int reg_idx, int offset);
struct frm_arg_t *frm_arg_create_glob_maddr_reg(int reg_idx);
struct frm_arg_t *frm_arg_create_glob_maddr_offset(int offset);
struct frm_arg_t *frm_arg_create_pt(char* name);
struct frm_arg_t *frm_arg_create_ccop(char* name);
struct frm_arg_t *frm_arg_create_maddr(struct frm_arg_t *soffset,
	struct frm_arg_t *qual, char *data_format, char *num_format);
/*
struct frm_arg_t *frm_arg_create_label(struct frm_symbol_t *symbol);
*/

int frm_arg_encode_operand(struct frm_arg_t *arg);

void frm_arg_dump(struct frm_arg_t *inst_arg, FILE *f);
void frm_mod_free(struct frm_mod_t *mod);

struct frm_mod_t *frm_mod_create_data_width(char *mod_name);
struct frm_mod_t *frm_mod_create_IMAD_mod(char *mod_name);
struct frm_mod_t *frm_mod_create_logic(char *mod_name);
struct frm_mod_t *frm_mod_create_comparison(char *mod_name);
struct frm_mod_t *frm_mod_create_brev(char *mod_name);
struct frm_mod_t *frm_mod_create_gen0_dst_cc(char *mod_name);
struct frm_mod_t *frm_mod_create_tgt_u(char *mod_name);
struct frm_mod_t *frm_mod_create_tgt_lmt(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_A_w(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_A_redarv(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_A_op(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_B_cop(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_B_type(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_C_s(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_D_sat(char* mod_name);
struct frm_mod_t *frm_mod_create_mod0_D_x(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_D_ftzfmz(char* mod_name);
struct frm_mod_t *frm_mod_create_gen0_src1_dtype(char* mod_name);
struct frm_mod_t *frm_mod_create_gen0_mod1_B_rnd(char* mod_name);
struct frm_mod_t *frm_mod_create_offs_mod1_A_trig(char* mod_name);
struct frm_mod_t *frm_mod_create_offs_mod1_A_op(char* mod_name);
struct frm_mod_t *frm_mod_create_with_name(char *name);


#endif

