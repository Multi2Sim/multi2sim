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
#include "main.h"


struct str_map_t si_arg_special_register_map =
{
	2,
	{
		{ "vcc", si_arg_special_register_vcc },
		{ "scc", si_arg_special_register_scc }
	}
};


struct si_arg_t *si_arg_create(void)
{
	struct si_arg_t *arg;
	
	/* Allocate */
	arg = xcalloc(1, sizeof(struct si_arg_t));
	
	/* Return */
	return arg;
	
}


struct si_arg_t *si_arg_create_literal(int value)
{
	struct si_arg_t *arg;

	arg = si_arg_create();
	arg->type = si_arg_literal;
	arg->value.literal.val = value;

	return arg;
}

struct si_arg_t *si_arg_create_literal_float(float value)
{
	struct si_arg_t *arg;

	arg = si_arg_create();
	arg->type = si_arg_literal_float;
	arg->value.literal_float.val = value;

	return arg;
}


struct si_arg_t *si_arg_create_scalar_register(char *name)
{
	struct si_arg_t *arg;

	arg = si_arg_create();
	arg->type = si_arg_scalar_register;

	assert(name[0] == 's');
	arg->value.scalar_register.id = atoi(name + 1);
	if (!IN_RANGE(arg->value.scalar_register.id, 0, 255))
		yyerror_fmt("scalar register out of range: %s", name);

	return arg;
}


struct si_arg_t *si_arg_create_vector_register(char *name)
{
	struct si_arg_t *arg;

	arg = si_arg_create();
	arg->type = si_arg_vector_register;

	assert(name[0] == 'v');
	arg->value.vector_register.id = atoi(name + 1);
	if (!IN_RANGE(arg->value.vector_register.id, 0, 255))
		yyerror_fmt("vector register out of range: %s", name);

	return arg;
}

struct si_arg_t *si_arg_create_special_register(char *name)
{
	struct si_arg_t *arg;
	int err;

	arg = si_arg_create();
	arg->type = si_arg_special_register;
	arg->value.special_register.type =
			str_map_string_err(&si_arg_special_register_map,
			name, &err);
	if (err)
		yyerror_fmt("invalid special register: %s", name);

	return arg;
}


struct si_arg_t *si_arg_create_maddr(struct si_arg_t *soffset,
		struct si_arg_t *qual, char *data_format, char *num_format)
{
	struct si_arg_t *arg;

	arg = si_arg_create();
	arg->type = si_arg_maddr;
	arg->value.maddr.soffset = soffset;
	arg->value.maddr.qual = qual;
	arg->value.maddr.data_format = xstrdup(data_format);
	arg->value.maddr.num_format = xstrdup(num_format);

	return arg;
}


struct si_arg_t *si_arg_create_maddr_qual(void)
{
	struct si_arg_t *arg;

	arg = si_arg_create();
	arg->type = si_arg_maddr_qual;

	return arg;
}


void si_arg_free(struct si_arg_t *arg)
{
	switch (arg->type)
	{

	case si_arg_maddr:

		free(arg->value.maddr.data_format);
		free(arg->value.maddr.num_format);
		si_arg_free(arg->value.maddr.soffset);
		si_arg_free(arg->value.maddr.qual);
		break;

	default:
		break;
	}

	free(arg);
}


int si_arg_encode_operand(struct si_arg_t *arg)
{
	switch (arg->type)
	{

	case si_arg_literal:
	{
		int value;

		value = arg->value.literal.val;
		if (IN_RANGE(value, 0, 64))
			return value + 128;
		if (IN_RANGE(value, -16, -1))
			return 192 - value;
		yyerror_fmt("invalid integer constant: %d", value);
		break;
	}

	case si_arg_literal_float:
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

		yyerror_fmt("invalid float constant: %g", value);
		break;
	}

	case si_arg_scalar_register:
	{
		int id;

		id = arg->value.scalar_register.id;
		if (IN_RANGE(id, 0, 103))
			return id;

		yyerror_fmt("invalid scalar register: s%d", id);
		break;
	}

	case si_arg_vector_register:
	{
		int id;

		id = arg->value.vector_register.id;
		if (IN_RANGE(id, 0, 255))
			return id;

		yyerror_fmt("invalid vector register: v%d", id);
		break;
	}

	/* FIXME - special registers missing here */

	default:
		yyerror("invalid operand");
		break;
	}

	/* Unreachable */
	return 0;
}


void si_arg_dump(struct si_arg_t *arg, FILE *f)
{
	switch (arg->type)
	{
	
	case si_arg_invalid:

		fprintf(f, "<invalid>");
		break;
		
	case si_arg_scalar_register:

		fprintf(f, "<sreg> s%d", arg->value.scalar_register.id);
		break;
		
	case si_arg_vector_register:

		fprintf(f, "<vreg> v%d", arg->value.vector_register.id);
		break;
		
	case si_arg_scalar_register_series:

		fprintf(f, "<sreg_series> s[%d:%d]",
			arg->value.scalar_register_series.low,
			arg->value.scalar_register_series.high);
		break;
			
	case si_arg_vector_register_series:

		fprintf(f, "<vreg_series> v[%d:%d]",
			arg->value.vector_register_series.low,
			arg->value.vector_register_series.high);
		break;
			
	case si_arg_literal:
	{
		int value;

		value = arg->value.literal.val;
		fprintf(f, "<const> %d", value);
		if (value)
			fprintf(f, " (0x%x)", value);
		break;
	}
		
	case si_arg_literal_float:

		fprintf(f, "<const_float> %g", arg->value.literal_float.val);
		break;

	case si_arg_waitcnt:
	{
		char buf[MAX_STRING_SIZE];

		fprintf(f, "<waitcnt>");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.vmcnt_value);
		fprintf(f, " vmcnt=%s", arg->value.wait_cnt.vmcnt_active ? buf : "x");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.expcnt_value);
		fprintf(f, " expcnt=%s", arg->value.wait_cnt.expcnt_active ? buf : "x");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.lgkmcnt_value);
		fprintf(f, " lgkmcnt=%s", arg->value.wait_cnt.lgkmcnt_active ? buf : "x");

		break;
	}

	case si_arg_special_register:

		fprintf(f, "<special_reg> %s", str_map_value(&si_arg_special_register_map,
				arg->value.special_register.type));
		break;
	
	case si_arg_mem_register:

		fprintf(f, "<mreg> m%d", arg->value.mem_register.id);
		break;
	
	case si_arg_maddr:

		fprintf(f, "<maddr>");

		fprintf(f, " soffs={");
		si_arg_dump(arg->value.maddr.soffset, f);
		fprintf(f, "}");

		fprintf(f, " qual={");
		si_arg_dump(arg->value.maddr.qual, f);
		fprintf(f, "}");

		fprintf(f, " dfmt=%s", arg->value.maddr.data_format);
		fprintf(f, " nfmt=%s", arg->value.maddr.num_format);

		break;

	case si_arg_maddr_qual:

		fprintf(f, "offen=%c", arg->value.maddr_qual.offen ? 't' : 'f');
		fprintf(f, " idxen=%c", arg->value.maddr_qual.idxen ? 't' : 'f');
		fprintf(f, " offset=%d", arg->value.maddr_qual.offset);
		break;

	case si_arg_label:
		fprintf(f, "<label>");
		break;
		
	default:
		panic("%s: invalid argument type", __FUNCTION__);
		break;
	}
}

