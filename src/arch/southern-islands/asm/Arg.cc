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

#include <string>

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>

#include "Arg.h"
#include "Asm.h"


namespace SI
{

misc::StringMap arg_dimension_map =
{
	{ "2D", 2 },
	{ "3D", 3 }
};


misc::StringMap arg_access_type_map =
{
	{ "RO", ArgAccessTypeReadOnly },
	{ "WO", ArgAccessTypeWriteOnly },
	{ "RW", ArgAccessTypeReadWrite }
};


misc::StringMap arg_data_type_map =
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
	{ "opaque", ArgDataTypeOpaque }
};


misc::StringMap arg_scope_map =
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
	{ "hr", ArgScopeHwGDS }
};


/* FIXME: Still need to figure out reflection for i1 and u1 */
misc::StringMap arg_reflection_map =
{
	{ "char", ArgDataTypeInt8 },
	{ "short", ArgDataTypeInt16 },
	{ "int", ArgDataTypeInt32 },
	{ "long", ArgDataTypeInt64 },
	{ "uchar", ArgDataTypeUInt8 },
	{ "ushort", ArgDataTypeUInt16 },
	{ "uint", ArgDataTypeUInt32 },
	{ "ulong", ArgDataTypeUInt64 },
	{ "float", ArgDataTypeFloat },
	{ "double", ArgDataTypeDouble },
	{ "struct", ArgDataTypeStruct },
	{ "union", ArgDataTypeUnion },
	{ "event", ArgDataTypeEvent },
	{ "opaque", ArgDataTypeOpaque }
};


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

		throw misc::Panic(misc::fmt("Invalid data type (%d)",
				data_type));
	}
}


void ArgPointer::Dump(std::ostream &os)
{
	os << arg_data_type_map.MapValue(data_type);
	if (num_elems > 1)
		os << '[' << num_elems << ']';
	os << "* " << getName();
}

void ArgPointer::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	std::string data_type_str;
	std::string scope_str;
	std::string access_type_str;
	std::string line;
	std::string name_str;

	int data_size;


	data_type_str = arg_data_type_map.MapValue(getDataType());
	data_size = Arg::getDataSize(data_type) * num_elems;
	scope_str = arg_scope_map.MapValue(getScope());
	access_type_str = arg_access_type_map.MapValue(getAccessType());

	line =
		";pointer:" + 
		getName() + ":" + data_type_str + ":1:" +
		std::to_string(constant_buffer_num) + ":" +
		std::to_string(constant_offset) + ":" + scope_str + ":" +
		std::to_string(buffer_num) + ":" + std::to_string(data_size) + ":" +
		access_type_str + ":0:0\n";


	/* Check for 16 byte alignment */
	if (((constant_offset - offset) < 16) && index)
		throw Asm::Error(misc::fmt("16 byte alignment not maintained "
				"in argument: %s - Expected offset of %d or "
				"higher",
				getName().c_str(),
				offset + 16));

	offset = constant_offset;

	/* Mark which uav's are being used */
	if (scope == ArgScopeUAV
			&& !(uav[buffer_num]))
	{
		uav[buffer_num] = 1;
	}

	buffer->Write(line.c_str(), line.size());

	/* Include const_arg line only if pointer is marked with "const" */
	if (getConstArg())
	{
		line = ";constarg:" + std::to_string(index) + ":" + getName() + "\n";
		buffer->Write(line.c_str(), line.size());
	}

}

void ArgPointer::WriteReflection(ELFWriter::Buffer *buffer, unsigned int index)
{
	std::string line;
	std::string reflection;

	reflection = arg_reflection_map.MapValue(getDataType());

	if (num_elems == 1)
	{
		line =
			";reflection:" + std::to_string(index)
			+ ":" + reflection + "*\n";
	}
	else if (num_elems > 1)
	{
		line =
			";reflection:" + std::to_string(index)+
			":" + reflection + std::to_string(num_elems)
			+ "*\n";
	}
	else
	{
		throw Asm::Error("Invalid number of elements in argument " +
				getName());
	}

	buffer->Write(line.c_str(), line.size());

}

void ArgValue::Dump(std::ostream &os)
{
	os << arg_data_type_map.MapValue(data_type);
	if (num_elems > 1)
		os << '[' << num_elems << ']';
	os << ' ' << getName();
}


void ArgValue::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	
	std::string data_type_str;
	std::string line;

	data_type_str = arg_data_type_map.MapValue(getDataType());

	line = 
		";value:" + getName() + ":" +
		data_type_str + ":" + std::to_string(num_elems) + ":" +
		std::to_string(constant_buffer_num) + ":" +
		std::to_string(constant_offset) + "\n";
	
	/* Check for 16 byte alignment */
	if (((constant_offset - offset) < 16) && index)
		throw Asm::Error(misc::fmt("16 byte alignment not maintained "
				"in argument: %s - Expected offset of %d or "
				"higher",
				getName().c_str(),
				offset + 16));

	offset = constant_offset;

	buffer->Write(line.c_str(), line.size());

	/* Include const_arg line only if pointer is marked with "const" */
	if (getConstArg())
	{
		line = ";constarg:" + std::to_string(index) + ":" + getName() + "\n";

		buffer->Write(line.c_str(), line.size());
	}

}

void ArgValue::WriteReflection(ELFWriter::Buffer *buffer, unsigned int index)
{

	std::string line;
	std::string reflection;

	reflection = arg_reflection_map.MapValue(getDataType());

	if (num_elems == 1)
	{
		line =
			";reflection:" + std::to_string(index)
			+ ":" + reflection + "*\n";
	}
	else if (num_elems > 1)
	{
		line =
			";reflection:" + std::to_string(index)+
			":" + reflection + std::to_string(num_elems)
			+ "\n";
	}
	else
	{
		throw Asm::Error("Invalid number of elements in argument "
				+ getName());
	}

	buffer->Write(line.c_str(), line.size());

}


void ArgImage::Dump(std::ostream &os)
{
	os << arg_dimension_map.MapValue(dimension);
	if (dimension > 1)
		os << '[' << dimension << ']';
	os << ' ' << getName();
}


void ArgImage::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	throw misc::Panic("Not implemented");
}


void ArgImage::WriteReflection(ELFWriter::Buffer *buffer, unsigned int index)
{
	throw misc::Panic("Not implemented");
}


void ArgSampler::Dump(std::ostream &os)
{
	throw misc::Panic("Not implemented");
}


void ArgSampler::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	throw misc::Panic("Not implemented");
}


void ArgSampler::WriteReflection(ELFWriter::Buffer *buffer, unsigned int index)
{
	throw misc::Panic("Not implemented");
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
