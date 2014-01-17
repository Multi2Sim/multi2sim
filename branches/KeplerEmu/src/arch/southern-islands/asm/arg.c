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

#include <lib/class/string.h>
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
		{ "RO", SIArgReadOnly },
		{ "WO", SIArgWriteOnly },
		{ "RW", SIArgReadWrite }
	}
};


struct str_map_t si_arg_data_type_map =
{
	16,
	{
		{ "i1", SIArgInt1 },
		{ "i8", SIArgInt8 },
		{ "i16", SIArgInt16 },
		{ "i32", SIArgInt32 },
		{ "i64", SIArgInt64 },
		{ "u1", SIArgUInt1 },
		{ "u8", SIArgUInt8 },
		{ "u16", SIArgUInt16 },
		{ "u32", SIArgUInt32 },
		{ "u64", SIArgUInt64 },
		{ "float", SIArgFloat },
		{ "double", SIArgDouble },
		{ "struct", SIArgStruct },
		{ "union", SIArgUnion },
		{ "event", SIArgEvent },
		{ "opaque", SIArgOpaque }
	}
};


struct str_map_t si_arg_scope_map =
{
	10,
	{
		{ "g", SIArgGlobal },
		{ "p", SIArgEmuPrivate },
		{ "l", SIArgEmuLocal },
		{ "uav", SIArgUAV },
		{ "c", SIArgEmuConstant },
		{ "r", SIArgEmuGDS },
		{ "hl", SIArgHwLocal },
		{ "hp", SIArgHwPrivate },
		{ "hc", SIArgHwConstant },
		{ "hr", SIArgHwGDS }
	}
};


/* FIXME: Still need to figure out reflection for i1 and u1 */
struct str_map_t si_arg_reflection_map =
{
	14,
	{
		{ "char", SIArgInt8 },
		{ "short", SIArgInt16 },
		{ "int", SIArgInt32 },
		{ "long", SIArgInt64 },
		{ "uchar", SIArgUInt8 },
		{ "ushort", SIArgUInt16 },
		{ "uint", SIArgUInt32 },
		{ "ulong", SIArgUInt64 },
		{ "float", SIArgFloat },
		{ "double", SIArgDouble },
		{ "struct", SIArgStruct },
		{ "union", SIArgUnion },
		{ "event", SIArgEvent },
		{ "opaque", SIArgOpaque }
	}
};


void SIArgCreate(SIArg *self, SIArgType type, char *name)
{
	/* Initialize */
	self->type = type;
	self->name = new(String, name);
}


void SIArgDestroy(SIArg *self)
{
	/* Specific fields per type */
	switch (self->type)
	{
	case SIArgTypeValue:

		if (self->value.value_ptr)
			free(self->value.value_ptr);
		break;

	default:
		break;
	}

	/* Rest */
	delete(self->name);
}

/* Set name if incorrect name is used in constructor */
void SIArgSetName(SIArg *self, char *name)
{
	StringSet(self->name, "%s", name);
}


/* Infer argument size from its data type */
int SIArgGetDataSize(SIArgDataType data_type)
{
	switch (data_type)
	{

	case SIArgInt8:
	case SIArgUInt8:
	case SIArgStruct:
	case SIArgUnion:
	case SIArgEvent:
	case SIArgOpaque:

		return 1;

	case SIArgInt16:
	case SIArgUInt16:

		return 2;

	case SIArgInt32:
	case SIArgUInt32:
	case SIArgFloat:

		return 4;

	case SIArgInt64:
	case SIArgUInt64:
	case SIArgDouble:

		return 8;

	default:

		panic("%s: invalid data type (%d)",
				__FUNCTION__, data_type);
		return 0;
	}
}


SIArg * SIArgCopy(SIArg *original)
{
	SIArg *copy;

	copy = new(SIArg, original->type, original->name->text);
	delete(copy->name);

	memcpy(copy, original, sizeof(*original));

	/* Copy name */
	copy->name = asString(StringClone(asObject(original->name)));

	/* Copy specific fields per type */ 
	switch (copy->type)
	{
	case SIArgTypeValue:

		if (copy->value.value_ptr)
		{
			copy->value.value_ptr = xcalloc(1, copy->size); 
			memcpy(copy->value.value_ptr, original->value.value_ptr,
				copy->size);
		}
		break;

	default:
		break;
	}

	return copy;
}

