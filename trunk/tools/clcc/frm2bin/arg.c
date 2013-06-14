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
#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "arg.h"
#include "frm2bin.h"
#include "token.h"


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

	return arg;
}

struct frm_arg_t *frm_arg_create_const_maddr(int bank_idx, int offset)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_const_maddr;
	arg->value.const_maddr.bank_idx = bank_idx;
	arg->value.const_maddr.offset = offset;

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

struct frm_arg_t *frm_arg_create_predicate_register(char *name)
{
	struct frm_arg_t *arg;

	arg = frm_arg_create();
	arg->type = frm_arg_predicate_register;
	arg->value.predicate_register.id = atoi(&name[1]);

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

	if (!strcmp(mod_name, "U32"))
	{
		mod->value.data_width = u32;
	}
	else if (!strcmp(mod_name, "S32"))
	{
		mod->value.data_width = s32;
	}
	else
	{
		printf("wrong mod_name inside frm_mod_create_data_width !\n");
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
		mod->value.comparison = lt;
	}
	else if (!strcmp(mod_name, "EQ"))
	{
		mod->value.comparison = eq;
	}
	else if (!strcmp(mod_name, "LE"))
	{
		mod->value.comparison = le;
	}
	else if (!strcmp(mod_name, "GT"))
	{
		mod->value.comparison = gt;
	}
	else if (!strcmp(mod_name, "NE"))
	{
		mod->value.comparison = ne;
	}
	else if (!strcmp(mod_name, "GE"))
	{
		mod->value.comparison = ge;
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

struct frm_mod_t *frm_mod_create_dst_cc(char *mod_name)
{
	struct frm_mod_t *mod;

	mod = frm_mod_create();

	mod->type = frm_token_gen0_dst_cc;
	mod->value.dst_cc = 1;

	return mod;
}

void frm_mod_free(struct frm_mod_t *mod)
{
	free(mod);
}

