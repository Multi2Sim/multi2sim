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

#include "Arg.h"


StringMap SI::arg_dimension_map =
{
	{ "2D", 2 },
	{ "3D", 3 },
	{ 0, 0 }
};


StringMap SI::arg_access_type_map =
{
	{ "RO", ArgReadOnly },
	{ "WO", ArgWriteOnly },
	{ "RW", ArgReadWrite },
	{ 0, 0 }
};


StringMap SI::arg_data_type_map =
{
	{ "i1", ArgInt1 },
	{ "i8", ArgInt8 },
	{ "i16", ArgInt16 },
	{ "i32", ArgInt32 },
	{ "i64", ArgInt64 },
	{ "u1", ArgUInt1 },
	{ "u8", ArgUInt8 },
	{ "u16", ArgUInt16 },
	{ "u32", ArgUInt32 },
	{ "u64", ArgUInt64 },
	{ "float", ArgFloat },
	{ "double", ArgDouble },
	{ "struct", ArgStruct },
	{ "union", ArgUnion },
	{ "event", ArgEvent },
	{ "opaque", ArgOpaque },
	{ 0, 0 }
};


SringMap SI::arg_scope_map =
{
	{ "g", ArgGlobal },
	{ "p", ArgEmuPrivate },
	{ "l", ArgEmuLocal },
	{ "uav", ArgUAV },
	{ "c", ArgEmuConstant },
	{ "r", ArgEmuGDS },
	{ "hl", ArgHwLocal },
	{ "hp", ArgHwPrivate },
	{ "hc", ArgHwConstant },
	{ "hr", ArgHwGDS },
	{ 0, 0 }
};


/* FIXME: Still need to figure out reflection for i1 and u1 */
StringMap SI::arg_reflection_map =
{
	{ "char", ArgInt8 },
	{ "short", ArgInt16 },
	{ "int", ArgInt32 },
	{ "long", ArgInt64 },
	{ "uchar", ArgUInt8 },
	{ "ushort", ArgUInt16 },
	{ "uint", ArgUInt32 },
	{ "ulong", ArgUInt64 },
	{ "float", ArgFloat },
	{ "double", ArgDouble },
	{ "struct", ArgStruct },
	{ "union", ArgUnion },
	{ "event", ArgEvent },
	{ "opaque", ArgOpaque },
	{ 0, 0 }
};


SI::Arg::Arg(ArgType type, string name)
{
	/* Initialize */
	this->type = type;
	this->name = name;
}


/* Infer argument size from its data type */
SI::Arg::GetDataSize(ArgDataType data_type)
{
	switch (data_type)
	{

	case ArgInt8:
	case ArgUInt8:
	case ArgStruct:
	case ArgUnion:
	case ArgEvent:
	case ArgOpaque:

		return 1;

	case ArgInt16:
	case ArgUInt16:

		return 2;

	case ArgInt32:
	case ArgUInt32:
	case ArgFloat:

		return 4;

	case ArgInt64:
	case ArgUInt64:
	case ArgDouble:

		return 8;

	default:

		panic("%s: invalid data type (%d)",
				__FUNCTION__, data_type);
		return 0;
	}
}




#ifdef 0
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
#endif
