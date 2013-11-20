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

#include <lib/cpp/Misc.h>

#include "Arg.h"


using namespace misc;

namespace SI
{

StringMap arg_dimension_map =
{
	{ "2D", 2 },
	{ "3D", 3 },
	{ 0, 0 }
};


StringMap arg_access_type_map =
{
	{ "RO", ArgAccessTypeReadOnly },
	{ "WO", ArgAccessTypeWriteOnly },
	{ "RW", ArgAccessTypeReadWrite },
	{ 0, 0 }
};


StringMap arg_data_type_map =
{
	{ "i1", ArgDataTypeInt1 },
	{ "i8", ArgDataTypeInt8 },
	{ "i16", ArgDataTypeInt16 },
	{ "i32", ArgDataTypeInt32 },
	{ "i64", ArgDataTypeInt64 },
	{ "u1", ArgDataTypeUInt1 },
	{ "u8", ArgDataTypeUInt8 },
	{ "u16", ArgDataTypeUInt16 },
	{ "u32", ArgDataTypeUInt32 },
	{ "u64", ArgDataTypeUInt64 },
	{ "float", ArgDataTypeFloat },
	{ "double", ArgDataTypeDouble },
	{ "struct", ArgDataTypeStruct },
	{ "union", ArgDataTypeUnion },
	{ "event", ArgDataTypeEvent },
	{ "opaque", ArgDataTypeOpaque },
	{ 0, 0 }
};


StringMap arg_scope_map =
{
	{ "g", ArgScopeGlobal },
	{ "p", ArgScopeEmuPrivate },
	{ "l", ArgScopeEmuLocal },
	{ "uav", ArgScopeUAV },
	{ "c", ArgScopeEmuConstant },
	{ "r", ArgScopeEmuGDS },
	{ "hl", ArgScopeHwLocal },
	{ "hp", ArgScopeHwPrivate },
	{ "hc", ArgScopeHwConstant },
	{ "hr", ArgScopeHwGDS },
	{ 0, 0 }
};


/* FIXME: Still need to figure out reflection for i1 and u1 */
/*StringMap arg_reflection_map =
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
};*/


/* Infer argument size from its data type */
int Arg::getDataSize(ArgDataType data_type)
{
	switch (data_type)
	{

	case ArgDataTypeInt8:
	case ArgDataTypeUInt8:
	case ArgDataTypeStruct:
	case ArgDataTypeUnion:
	case ArgDataTypeEvent:
	case ArgDataTypeOpaque:

		return 1;

	case ArgDataTypeInt16:
	case ArgDataTypeUInt16:

		return 2;

	case ArgDataTypeInt32:
	case ArgDataTypeUInt32:
	case ArgDataTypeFloat:

		return 4;

	case ArgDataTypeInt64:
	case ArgDataTypeUInt64:
	case ArgDataTypeDouble:

		return 8;

	default:

		panic("%s: invalid data type (%d)",
				__FUNCTION__, data_type);
		return 0;
	}
}


void ArgPointer::Dump(std::ostream &os)
{
	os << StringMapValue(arg_data_type_map, data_type);
	if (num_elems > 1)
		os << '[' << num_elems << ']';
	os << "* " << getName();
}


void ArgValue::Dump(std::ostream &os)
{
	os << StringMapValue(arg_data_type_map, data_type);
	if (num_elems > 1)
		os << '[' << num_elems << ']';
	os << ' ' << getName();
}


#if 0
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

}  /* namespace SI */
