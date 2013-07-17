/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

struct str_map_t si_arg_dimension_map =
{
	2,
	{
		{ "2D", 2 },
		{ "3D", 3 }
	}
};


struct str_map_t si_arg_access_type_map =
{
	3,
	{
		{ "RO", si_arg_read_only },
		{ "WO", si_arg_write_only },
		{ "RW", si_arg_read_write }
	}
};


struct str_map_t si_arg_data_type_map =
{
	16,
	{
		{ "i1", si_arg_i1 },
		{ "i8", si_arg_i8 },
		{ "i16", si_arg_i16 },
		{ "i32", si_arg_i32 },
		{ "i64", si_arg_i64 },
		{ "u1", si_arg_u1 },
		{ "u8", si_arg_u8 },
		{ "u16", si_arg_u16 },
		{ "u32", si_arg_u32 },
		{ "u64", si_arg_u64 },
		{ "float", si_arg_float },
		{ "double", si_arg_double },
		{ "struct", si_arg_struct },
		{ "union", si_arg_union },
		{ "event", si_arg_event },
		{ "opaque", si_arg_opaque }
	}
};


struct str_map_t si_arg_scope_map =
{
	10,
	{
		{ "g", si_arg_global },
		{ "p", si_arg_emu_private },
		{ "l", si_arg_emu_local },
		{ "uav", si_arg_uav },
		{ "c", si_arg_emu_constant },
		{ "r", si_arg_emu_gds },
		{ "hl", si_arg_hw_local },
		{ "hp", si_arg_hw_private },
		{ "hc", si_arg_hw_constant },
		{ "hr", si_arg_hw_gds }
	}
};


/* FIXME: Still need to figure out reflection for i1 and u1 */
struct str_map_t si_arg_reflection_map =
{
	14,
	{
		{ "char", si_arg_i8 },
		{ "short", si_arg_i16 },
		{ "int", si_arg_i32 },
		{ "long", si_arg_i64 },
		{ "uchar", si_arg_u8 },
		{ "ushort", si_arg_u16 },
		{ "uint", si_arg_u32 },
		{ "ulong", si_arg_u64 },
		{ "float", si_arg_float },
		{ "double", si_arg_double },
		{ "struct", si_arg_struct },
		{ "union", si_arg_union },
		{ "event", si_arg_event },
		{ "opaque", si_arg_opaque }
	}
};


struct si_arg_t *si_arg_create(enum si_arg_type_t type,
		char *name)
{
	struct si_arg_t *arg;

	/* Initialize */
	arg = xcalloc(1, sizeof(struct si_arg_t));
	arg->type = type;
	arg->name = xstrdup(name);

	/* Return */
	return arg;
}


void si_arg_free(struct si_arg_t *arg)
{
	/* Specific fields per type */
	switch (arg->type)
	{
	case si_arg_value:

		if (arg->value.value_ptr)
			free(arg->value.value_ptr);
		break;

	default:
		break;
	}

	/* Rest */
	free(arg->name);
	free(arg);
}

/* Set name if incorrect name is used in constructor */
void si_arg_name_set(struct si_arg_t *arg, char *name)
{
	free(arg->name);
	arg->name = xstrdup(name);
}


/* Infer argument size from its data type */
int si_arg_get_data_size(enum si_arg_data_type_t data_type)
{
	switch (data_type)
	{

	case si_arg_i8:
	case si_arg_u8:
	case si_arg_struct:
	case si_arg_union:
	case si_arg_event:
	case si_arg_opaque:

		return 1;

	case si_arg_i16:
	case si_arg_u16:

		return 2;

	case si_arg_i32:
	case si_arg_u32:
	case si_arg_float:

		return 4;

	case si_arg_i64:
	case si_arg_u64:
	case si_arg_double:

		return 8;

	default:

		panic("%s: invalid data type (%d)",
				__FUNCTION__, data_type);
		return 0;
	}
}



