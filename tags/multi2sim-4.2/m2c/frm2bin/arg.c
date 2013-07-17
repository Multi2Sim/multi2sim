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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "arg.h"
#include "frm2bin.h"


struct str_map_t frm_arg_special_register_map =
{
	50,
	{
	        { "SR_LaneId", frm_arg_special_register_LaneId }, 
	        { "SR_VirtCfg", frm_arg_special_register_VirtCfg }, 
	        { "SR_VirtId", frm_arg_special_register_VirtId }, 
	        { "SR_PM0", frm_arg_special_register_PM0 }, 
	        { "SR_PM1", frm_arg_special_register_PM1 }, 
	        { "SR_PM2", frm_arg_special_register_PM2 }, 
	        { "SR_PM3", frm_arg_special_register_PM3 }, 
	        { "SR_PM4", frm_arg_special_register_PM4 }, 
	        { "SR_PM5", frm_arg_special_register_PM5 }, 
	        { "SR_PM6", frm_arg_special_register_PM6 }, 
	        { "SR_PM7", frm_arg_special_register_PM7 }, 
	        { "SR_PRIM_TYPE", frm_arg_special_register_PRIM_TYPE }, 
	        { "SR_INVOCATION_ID", frm_arg_special_register_INVOCATION_ID }, 
	        { "SR_Y_DIRECTION", frm_arg_special_register_Y_DIRECTION }, 
	        { "SR_MACHINE_ID_0", frm_arg_special_register_MACHINE_ID_0 }, 
	        { "SR_MACHINE_ID_1", frm_arg_special_register_MACHINE_ID_1 }, 
	        { "SR_MACHINE_ID_2", frm_arg_special_register_MACHINE_ID_2 }, 
	        { "SR_MACHINE_ID_3", frm_arg_special_register_MACHINE_ID_3 }, 
	        { "SR_AFFINITY", frm_arg_special_register_AFFINITY }, 
	        { "SR_Tid", frm_arg_special_register_Tid },  
	        { "SR_Tid_X", frm_arg_special_register_Tid_X }, 
	        { "SR_Tid_Y", frm_arg_special_register_Tid_Y }, 
	        { "SR_Tid_Z", frm_arg_special_register_Tid_Z }, 
	        { "SR_CTAParam", frm_arg_special_register_CTAParam }, 
	        { "SR_CTAid_X", frm_arg_special_register_CTAid_X }, 
	        { "SR_CTAid_Y", frm_arg_special_register_CTAid_Y }, 
	        { "SR_CTAid_Z", frm_arg_special_register_CTAid_Z }, 
	        { "SR_NTid", frm_arg_special_register_NTid }, 
	        { "SR_NTid_X", frm_arg_special_register_NTid_X }, 
	        { "SR_NTid_Y", frm_arg_special_register_NTid_Y }, 
	        { "SR_NTid_Z", frm_arg_special_register_NTid_Z }, 
	        { "SR_GridParam", frm_arg_special_register_GridParam }, 
	        { "SR_NCTAid_X", frm_arg_special_register_NCTAid_X }, 
	        { "SR_NCTAid_Y", frm_arg_special_register_NCTAid_Y }, 
	        { "SR_NCTAid_Z", frm_arg_special_register_NCTAid_Z }, 
	        { "SR_SWinLo", frm_arg_special_register_SWinLo }, 
	        { "SR_SWINSZ", frm_arg_special_register_SWINSZ }, 
	        { "SR_SMemSz", frm_arg_special_register_SMemSz }, 
	        { "SR_SMemBanks", frm_arg_special_register_SMemBanks }, 
	        { "SR_LWinLo", frm_arg_special_register_LWinLo }, 
	        { "SR_LWINSZ", frm_arg_special_register_LWINSZ }, 
	        { "SR_LMemLoSz", frm_arg_special_register_LMemLoSz }, 
	        { "SR_LMemHiOff", frm_arg_special_register_LMemHiOff }, 
	        { "SR_EqMask", frm_arg_special_register_EqMask }, 
	        { "SR_LtMask", frm_arg_special_register_LtMask }, 
	        { "SR_LeMask", frm_arg_special_register_LeMask }, 
	        { "SR_GtMask", frm_arg_special_register_GtMask }, 
	        { "SR_GeMask", frm_arg_special_register_GeMask }, 
	        { "SR_ClockLo", frm_arg_special_register_ClockLo }, 
	        { "SR_ClockHi", frm_arg_special_register_ClockHi }, 

	}
};

struct str_map_t frm_arg_ccop_map =
{
	32,
	{
		{ "F", frm_arg_ccop_f},
		{ "LT", frm_arg_ccop_lt},
		{ "EQ", frm_arg_ccop_eq},
		{ "LE", frm_arg_ccop_le},
		{ "GT", frm_arg_ccop_gt},
		{ "NE", frm_arg_ccop_ne},
		{ "GE", frm_arg_ccop_ge},
		{ "NUM", frm_arg_ccop_num},
		{ "NAN", frm_arg_ccop_nan},
		{ "LTU", frm_arg_ccop_ltu},
		{ "EQU", frm_arg_ccop_equ},
		{ "LEU", frm_arg_ccop_leu},
		{ "GTU", frm_arg_ccop_gtu},
		{ "NEU", frm_arg_ccop_neu},
		{ "GEU", frm_arg_ccop_geu},
		{ "T", frm_arg_ccop_t},
		{ "OFF", frm_arg_ccop_off},
		{ "LO", frm_arg_ccop_lo},
		{ "SFF", frm_arg_ccop_sff},
		{ "LS", frm_arg_ccop_ls},
		{ "HI", frm_arg_ccop_hi},
		{ "SFT", frm_arg_ccop_sft},
		{ "HS", frm_arg_ccop_hs},
		{ "OFT", frm_arg_ccop_oft},
		{ "CSM_TA", frm_arg_ccop_csm_ta},
		{ "CSM_TR", frm_arg_ccop_csm_tr},
		{ "CSM_MX", frm_arg_ccop_csm_mx},
		{ "FCSM_TA", frm_arg_ccop_fcsm_ta},
		{ "FCSM_TR", frm_arg_ccop_fcsm_tr},
		{ "FCSM_MX", frm_arg_ccop_fcsm_mx},
		{ "RLE", frm_arg_ccop_rle},
		{ "RGT", frm_arg_ccop_rgt},
	}
};

struct frm_arg_t *frm_arg_create(void)
{
	struct frm_arg_t *arg;

	/* Allocate */
	arg = xcalloc(1, sizeof(struct frm_arg_t));

	/* Return */
	return arg;

}


struct frm_arg_t *frm_arg_create_literal(int value)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_literal;
	arg->value.literal.val = value;
	if (value < 0)
		arg->neg = 1;

	return arg;
}

struct frm_arg_t *frm_arg_create_const_maddr(int bank_idx, int offset, int negative)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_const_maddr;
	arg->value.const_maddr.bank_idx = bank_idx;
	arg->value.const_maddr.offset = offset;
	if (negative == 0)
		arg->neg = 0;
	else if (negative == 1)
		arg->neg = 1;
	else
	{
		frm2bin_yyerror_fmt("Error negative value for const_maddr argument!\n");
	}

	return arg;
}

struct frm_arg_t *frm_arg_create_shared_maddr(int bank_idx, int offset)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_shared_maddr;
	arg->value.shared_maddr.bank_idx = bank_idx;
	arg->value.shared_maddr.offset = offset;

	return arg;
}

struct frm_arg_t *frm_arg_create_glob_maddr(int reg_idx, int offset)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_glob_maddr;
	arg->value.glob_maddr.reg_idx = reg_idx;
	arg->value.glob_maddr.offset = offset;

	return arg;
}

struct frm_arg_t *frm_arg_create_glob_maddr_reg(int reg_idx)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_scalar_register;
	arg->value.scalar_register.id = reg_idx;

	return arg;
}

struct frm_arg_t *frm_arg_create_glob_maddr_offset(int offset)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_literal;
	arg->value.literal.val = offset;

	return arg;
}

struct frm_arg_t *frm_arg_create_pt(char *name)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_pt;

	/* more cases will be added later */
	if (!strcmp(name, "pt"))
	{
		/* value range 0 ~ 7, 7 means predicate ture */
		/* if it's pt, predicate ture, P7 */
		arg->value.pt.idx = 7;
	}
	else
	{
		arg->value.pt.idx = 0;
	}

	return arg;
}

struct frm_arg_t *frm_arg_create_scalar_register(char *name)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_scalar_register;

	assert(name[0] == 'R');
	arg->value.scalar_register.id = atoi(name + 1);
	if (!IN_RANGE(arg->value.scalar_register.id, 0, 62))
		frm2bin_yyerror_fmt("register out of range: %s", name);

	return arg;
}

struct frm_arg_t *frm_arg_create_special_register(char *name)
{
	struct frm_arg_t *arg;
	int err;

	arg = frm_arg_create();
	arg->type = frm_arg_special_register;
	arg->value.special_register.type =
		str_map_string_err(&frm_arg_special_register_map, name, &err);
	if (err)
		frm2bin_yyerror_fmt("invalid special register: %s", name);

	return arg;
}

struct frm_arg_t *frm_arg_create_zero_register(char *name)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_zero_register;

	return arg;
}

struct frm_arg_t *frm_arg_create_predicate_register(char *name)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_predicate_register;
	arg->value.predicate_register.id = atoi(&name[1]);

	return arg;
}

struct frm_arg_t *frm_arg_create_ccop(char *name)
{
	struct frm_arg_t *arg;
	int idx;

	arg = frm_arg_create();
	arg->type = frm_arg_ccop;

	/* name: CC.op, count from the 4th element,
	 * go though the map to see which string matches */
	for (idx = 0; idx < (frm_arg_ccop_map.count - 1); idx ++)
	{
		if (!strcmp(&(name[3]), str_map_value(&frm_arg_ccop_map, idx)))
			arg->value.ccop.op = idx;
	}

	return arg;

}

void frm_arg_free(struct frm_arg_t *arg)
{
	switch (arg->type)
	{

	case frm_arg_maddr:

		free(arg->value.maddr.data_format);
		free(arg->value.maddr.num_format);
		frm_arg_free(arg->value.maddr.soffset);
		frm_arg_free(arg->value.maddr.qual);
		break;

	default:
		break;
	}

	free(arg);
}


int frm_arg_encode_operand(struct frm_arg_t *arg)
{
	switch (arg->type)
	{

	case frm_arg_literal:
	{
		int value;

		value = arg->value.literal.val;
		/* may need improvement later */
		return value;

		/*
		if (IN_RANGE(value, 0, 64))
			return value + 128;
		if (IN_RANGE(value, -16, -1))
			return 192 - value;
		*/

		frm2bin_yyerror_fmt("invalid integer constant: %d", value);
		break;
	}

	case frm_arg_maddr:
	{
	  break;
	}

	/*
	case frm_arg_literal_float:
	{
		float value;

		value = arg->value.literal_float.val;
		if (value == 0.5)
			return 240;
		if (value == -0.5)
			return 241;
		if (value == 1.0)
			return 242;
		if (value == -1.0)
			return 243;
		if (value == 2.0)
			return 244;
		if (value == -2.0)
			return 245;
		if (value == 4.0)
			return 246;
		if (value == -4.0)
			return 247;

		frm2bin_yyerror_fmt("invalid float constant: %g", value);
		break;
	}
	*/

	case frm_arg_scalar_register:
	{
		int id;

		id = arg->value.scalar_register.id;
		if (IN_RANGE(id, 0, 103))
			return id;

		frm2bin_yyerror_fmt("invalid scalar register: s%d", id);
		break;
	}


		/* Special register */
	case frm_arg_special_register:
	{
		switch (arg->value.special_register.type)
		{
			/* not implemented yet */

		default:
			frm2bin_yyerror_fmt
				("%s: unsupported special register (code=%d)",
				__FUNCTION__,
				arg->value.special_register.type);
		}
		break;
	}

	default:
		frm2bin_yyerror_fmt("invalid operand (code %d)", arg->type);
		break;
	}

	/* Unreachable */
	return 0;
}


void frm_arg_dump(struct frm_arg_t *arg, FILE *f)
{
	switch (arg->type)
	{

	case frm_arg_invalid:

		fprintf(f, "<invalid>");
		break;

	case frm_arg_scalar_register:

		fprintf(f, "<sreg> s%d", arg->value.scalar_register.id);
		break;

	case frm_arg_literal:
	{
		int value;

		value = arg->value.literal.val;
		fprintf(f, "<const> %d", value);
		if (value)
			fprintf(f, " (0x%x)", value);
		break;
	}

	case frm_arg_special_register:
	{
		fprintf(f, "<special_reg> %s",
			str_map_value(&frm_arg_special_register_map,
				arg->value.special_register.type));
		break;
	}

	case frm_arg_maddr:
	{
		fprintf(f, "<maddr>");

		fprintf(f, " soffs={");
		frm_arg_dump(arg->value.maddr.soffset, f);
		fprintf(f, "}");

		fprintf(f, " qual={");
		frm_arg_dump(arg->value.maddr.qual, f);
		fprintf(f, "}");

		fprintf(f, " dfmt=%s", arg->value.maddr.data_format);
		fprintf(f, " nfmt=%s", arg->value.maddr.num_format);

		break;
	}


	/*
	case frm_arg_label:
		fprintf(f, "<label>");
		break;
	*/
		
	default:
		panic("%s: invalid argument type", __FUNCTION__);
		break;
	}
}

struct frm_mod_t *frm_mod_create_with_name(char *name)
{
	struct frm_mod_t *mod;
	enum frm_token_type_t token_type = frm_token_invalid;
	char long_name[40];

	/* % is removed from the mod in the previous processing but the
	 * frm_token_map has % in each token, so it's added back. This will
	 * be changed later */
	strcpy(long_name, "%");
	strcat(long_name, name);

	/* Allocate */
	mod = xcalloc(1, sizeof(struct frm_mod_t));

	/* create mod obj according to the mod type */
	token_type = str_map_string_case(&frm_token_map, long_name);
	if (token_type != frm_token_invalid)
	{
		mod->type = token_type;
	}
	else
	{
		mod->type = token_type;
		printf("invalid modifier type! [%s]\n", name);
	}

	/* Return */
	return mod;
}

struct frm_mod_t *frm_mod_create(void)
{
	struct frm_mod_t *mod;

	/* Allocate */
	mod = xcalloc(1, sizeof(struct frm_mod_t));

	/* Return */
	return mod;
}

struct frm_mod_t *frm_mod_create_data_width(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();
	mod->type = frm_token_mod_data_width;

	if (!strcmp(mod_name, "U16"))
	{
		mod->value.data_width = u16;
	}
	else if (!strcmp(mod_name, "S16"))
	{
		mod->value.data_width = s16;
	}
	else if (!strcmp(mod_name, "U32"))
	{
		mod->value.data_width = u32;
	}
	else if (!strcmp(mod_name, "S32"))
	{
		mod->value.data_width = s32;
	}
	else if (!strcmp(mod_name, "U64"))
	{
		mod->value.data_width = u64;
	}
	else if (!strcmp(mod_name, "S64"))
	{
		mod->value.data_width = s64;
	}
	else
	{
		printf("wrong mod_name inside frm_mod_create_data_width !\n");
	}

	return mod;
}

/* modifier specifically for IMAD instruction */
struct frm_mod_t *frm_mod_create_IMAD_mod(char *mod_name)
{
	struct frm_mod_t *mod;
	/* use count to tell create IMAD_mod1 or IMAD_mod2 */
	static int count = 0;

	mod = frm_mod_create();
	if (count == 0)
	{
		mod->type = frm_token_IMAD_mod1;
		count ++;
	}
	else
	{
		mod->type = frm_token_IMAD_mod2;
		count --;
	}

	if (!strcmp(mod_name, "U32"))
	{
		mod->value.IMAD_mod = 0;
	}
	else if (!strcmp(mod_name, "S32"))
	{
		mod->value.IMAD_mod = 1;
	}
	else
	{
		printf("wrong IMAD_mod !\n");
	}

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_B_type(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();
	mod->type = frm_token_mod0_B_type;

	if (!strcmp(mod_name, "U8"))
	{
		mod->value.mod0_B_type = 0;
	}
	else if (!strcmp(mod_name, "S8"))
	{
		mod->value.mod0_B_type = 1;
	}
	else if (!strcmp(mod_name, "U16"))
	{
		mod->value.mod0_B_type = 2;
	}
	else if (!strcmp(mod_name, "S16"))
	{
		mod->value.mod0_B_type = 3;
	}
	else if (!strcmp(mod_name, "64"))
	{
		mod->value.mod0_B_type = 5;
	}
	else if (!strcmp(mod_name, "128"))
	{
		mod->value.mod0_B_type = 6;
	}
	else
	{
		/* default: u32 */
		mod->value.mod0_B_type = 4;
	}

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_B_cop(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();
	mod->type = frm_token_mod0_B_cop;

	if (!strcmp(mod_name, "CA"))
	{
		mod->value.mod0_B_cop = 0;
	}
	else if (!strcmp(mod_name, "CG"))
	{
		mod->value.mod0_B_cop = 1;
	}
	else if (!strcmp(mod_name, "LU"))
	{
		mod->value.mod0_B_cop = 2;
	}
	else if (!strcmp(mod_name, "CV"))
	{
		mod->value.mod0_B_cop = 3;
	}
	else
	{
		printf("wrong mod0_b_cop!\n");
	}

	return mod;
}

struct frm_mod_t *frm_mod_create_logic(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();
	mod->type = frm_token_mod_logic;

	if (!strcmp(mod_name, "AND"))
	{
		mod->value.logic = logic_and;
	}
	else if (!strcmp(mod_name, "OR"))
	{
		mod->value.logic = logic_or;
	}
	else if (!strcmp(mod_name, "XOR"))
	{
		mod->value.logic = logic_xor;
	}
	else
	{
		printf("wrong mod_name inside frm_mod_create_logic !\n");
	}

	return mod;
}

struct frm_mod_t *frm_mod_create_comparison(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();
	mod->type = frm_token_mod_comparison;

	if (!strcmp(mod_name, "LT"))
	{
		mod->value.comparison = frm_lt;
	}
	else if (!strcmp(mod_name, "EQ"))
	{
		mod->value.comparison = frm_eq;
	}
	else if (!strcmp(mod_name, "LE"))
	{
		mod->value.comparison = frm_le;
	}
	else if (!strcmp(mod_name, "GT"))
	{
		mod->value.comparison = frm_gt;
	}
	else if (!strcmp(mod_name, "NE"))
	{
		mod->value.comparison = frm_ne;
	}
	else if (!strcmp(mod_name, "GE"))
	{
		mod->value.comparison = frm_ge;
	}
	else if (!strcmp(mod_name, "NUM"))
	{
		mod->value.comparison = frm_num;
	}
	else if (!strcmp(mod_name, "NAN"))
	{
		mod->value.comparison = frm_nan;
	}
	else if (!strcmp(mod_name, "LTU"))
	{
		mod->value.comparison = frm_ltu;
	}
	else if (!strcmp(mod_name, "EQU"))
	{
		mod->value.comparison = frm_equ;
	}
	else if (!strcmp(mod_name, "LEU"))
	{
		mod->value.comparison = frm_leu;
	}
	else if (!strcmp(mod_name, "GTU"))
	{
		mod->value.comparison = frm_gtu;
	}
	else if (!strcmp(mod_name, "GEU"))
	{
		mod->value.comparison = frm_geu;
	}
	else if (!strcmp(mod_name, "NEU"))
	{
		mod->value.comparison = frm_neu;
	}
	else
	{
		printf("wrong mod_name inside frm_mod_create_comparison !\n");
	}

	return mod;
}

struct frm_mod_t *frm_mod_create_brev(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_B_brev;
	mod->value.brev = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_gen0_src1_dtype(char* mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();
	mod->type = frm_token_gen0_src1_dtype;

	if (!strcmp(mod_name, "F16"))
	{
		mod->value.gen0_src1_dtype = 1;
	}
	else if (!strcmp(mod_name, "F32"))
	{
		mod->value.gen0_src1_dtype = 2;
	}
	else if (!strcmp(mod_name, "F64"))
	{
		mod->value.gen0_src1_dtype = 3;
	}
	else
	{
		mod->value.gen0_src1_dtype = 0;
	}

	return mod;
}

struct frm_mod_t *frm_mod_create_gen0_dst_cc(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_gen0_dst_cc;
	mod->value.dst_cc = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_tgt_u(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_tgt_u;
	mod->value.tgt_u = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_tgt_lmt(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_tgt_lmt;
	mod->value.tgt_lmt = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_A_w(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_A_w;
	mod->value.mod0_A_w = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_A_redarv(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_A_redarv;
	if (!strcmp(mod_name, "RED"))
		mod->value.mod0_A_redarv = 0;
	else
		mod->value.mod0_A_redarv = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_A_op(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_A_op;
	if (!strcmp(mod_name, "POPC"))
		mod->value.mod0_A_op = 0;
	else if (!strcmp(mod_name, "AND"))
		mod->value.mod0_A_op = 1;
	else if (!strcmp(mod_name, "OR"))
		mod->value.mod0_A_op = 2;
	else
		mod->value.mod0_A_op = 3;

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_C_s(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_C_s;
	mod->value.mod0_C_s = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_D_ftzfmz(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_D_ftzfmz;
	if (!strcmp(mod_name, "FTZ"))
		mod->value.mod0_D_ftzfmz = 1;
	else if (!strcmp(mod_name, "FMZ"))
		mod->value.mod0_D_ftzfmz = 2;
	else
		mod->value.mod0_D_ftzfmz = 3;

	return mod;
}

struct frm_mod_t *frm_mod_create_gen0_mod1_B_rnd(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_gen0_mod1_B_rnd;
	if (!strcmp(mod_name, "RN"))
		mod->value.gen0_mod1_B_rnd = 0;
	else if (!strcmp(mod_name, "RM"))
		mod->value.gen0_mod1_B_rnd = 1;
	else if (!strcmp(mod_name, "RP"))
		mod->value.gen0_mod1_B_rnd = 2;
	else
		mod->value.gen0_mod1_B_rnd = 3;

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_D_sat(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_D_sat;
	mod->value.mod0_D_sat = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_mod0_D_x(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_mod0_D_x;
	mod->value.mod0_D_x = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_offs_mod1_A_trig(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_offs_mod1_A_trig;
	mod->value.offs_mod1_A_trig = 1;

	return mod;
}

struct frm_mod_t *frm_mod_create_offs_mod1_A_op(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_offs_mod1_A_op;
	if (!strcmp(mod_name, "FMA64"))
		mod->value.offs_mod1_A_op = 1;
	else if (!strcmp(mod_name, "FMA32"))
		mod->value.offs_mod1_A_op = 2;
	else if (!strcmp(mod_name, "XLU"))
		mod->value.offs_mod1_A_op = 3;
	else if (!strcmp(mod_name, "ALU"))
		mod->value.offs_mod1_A_op = 4;
	else if (!strcmp(mod_name, "AGU"))
		mod->value.offs_mod1_A_op = 5;
	else if (!strcmp(mod_name, "SU"))
		mod->value.offs_mod1_A_op = 6;
	else if (!strcmp(mod_name, "FU"))
		mod->value.offs_mod1_A_op = 7;
	else if (!strcmp(mod_name, "FMUL"))
		mod->value.offs_mod1_A_op = 8;
	else
		mod->value.offs_mod1_A_op = 0;

	return mod;
}

void frm_mod_free(struct frm_mod_t *mod)
{
	free(mod);
}

