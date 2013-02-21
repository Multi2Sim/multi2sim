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


struct si_arg_t *si_arg_create_mt_addr(struct si_arg_t *offset,
		int offen, char *data_format, char *num_format)
{
	struct si_arg_t *arg;

	arg = si_arg_create();
	arg->type = si_arg_mt_addr;
	arg->value.mt_addr.offset = offset;
	arg->value.mt_addr.offen = offen;
	arg->value.mt_addr.data_format = xstrdup(data_format);
	arg->value.mt_addr.num_format = xstrdup(num_format);

	return arg;
}


void si_arg_free(struct si_arg_t *arg)
{
	switch (arg->type)
	{

	case si_arg_mt_addr:

		free(arg->value.mt_addr.data_format);
		free(arg->value.mt_addr.num_format);
		si_arg_free(arg->value.mt_addr.offset);
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

		fprintf(f, "\t\tinvalid\n");
		break;
		
	case si_arg_scalar_register:

		fprintf(f, "\t\tscalar register\n");
		fprintf(f, "\t\ts%d\n", arg->value.scalar_register.id);
		break;
		
	case si_arg_vector_register:

		fprintf(f, "\t\tvector register\n");
		fprintf(f, "\t\tv%d\n", arg->value.vector_register.id);
		break;
		
	case si_arg_scalar_register_series:

		fprintf(f, "\t\tscalar register series\n");
		fprintf(f, "\t\ts[%d:%d]\n", arg->value.scalar_register_series.low,
			arg->value.scalar_register_series.high);
		break;
			
	case si_arg_vector_register_series:

		fprintf(f, "\t\tvector register series\n");
		fprintf(f, "\t\tv[%d:%d]\n", arg->value.vector_register_series.low,
			arg->value.vector_register_series.high);
		break;
			
	case si_arg_literal:

		fprintf(f, "\t\tliteral constant\n");
		fprintf(f, "\t\t0x%x (%d)\n", arg->value.literal.val,
			arg->value.literal.val);
		break;
		
	case si_arg_literal_float:

		fprintf(f, "\t\tliteral float\n");
		fprintf(f, "\t\t%g\n", arg->value.literal_float.val);
		break;

	case si_arg_waitcnt:

		fprintf(f, "\t\twaitcnt\n");
		fprintf(f, "\t\tvmcnt: active=%d, value=%d\n", arg->value.wait_cnt.vmcnt_active,
				arg->value.wait_cnt.vmcnt_value);
		fprintf(f, "\t\texpcnt: active=%d, value=%d\n", arg->value.wait_cnt.expcnt_active,
				arg->value.wait_cnt.expcnt_value);
		fprintf(f, "\t\tlgkmcnt: active=%d, value=%d\n", arg->value.wait_cnt.lgkmcnt_active,
				arg->value.wait_cnt.lgkmcnt_value);
		break;

	case si_arg_special_register:

		fprintf(f, "\t\tspecial register\n");
		if (arg->value.special_register.type == si_arg_special_register_vcc)
			fprintf(f, "\t\tvcc\n");
		else if (arg->value.special_register.type == si_arg_special_register_scc)
			fprintf(f, "\t\tscc\n");
		break;
	
	case si_arg_mtype_register:

		fprintf(f, "\t\tm-type register\n");
		fprintf(f, "\t\tm%d\n", arg->value.mtype_register.id);
		break;
	
	case si_arg_mt_addr:

		fprintf(f, "\t\tmt_addr\n");
		//fprintf(f, "\t\toffset: %d\n", inst_arg->value.format.offset);
		fprintf(f, "\t\toffen: %c\n", arg->value.mt_addr.offen ? 't' : 'f');
		fprintf(f, "\t\tdata format: %s\n", arg->value.mt_addr.data_format);
		fprintf(f, "\t\tnum format: %s\n", arg->value.mt_addr.num_format);
		break;

	case si_arg_label:
		fprintf(f, "\tlabel\n");
		break;
		
	default:
		panic("%s: invalid argument type", __FUNCTION__);
		break;
	}
}

