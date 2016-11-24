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

#include "Argument.h"
#include "Disassembler.h"


namespace SI
{

// String map of the argument's access type
const misc::StringMap Argument::access_type_map = 
{
	{ "RO", AccessTypeReadOnly },
	{ "WO", AccessTypeWriteOnly },
	{ "RW", AccessTypeReadWrite }
};

// String map of the argument's scope
const misc::StringMap Argument::scope_map = 
{
	{ "g", ScopeGlobal },
	{ "p", ScopeEmuPrivate },
	{ "l", ScopeEmuLocal },
	{ "uav", ScopeUAV },
	{ "c", ScopeEmuConstant },
	{ "r", ScopeEmuGDS },
	{ "hl", ScopeHwLocal },
	{ "hp", ScopeHwPrivate },
	{ "hc", ScopeHwConstant },
	{ "hr", ScopeHwGDS }
};

// String map of the argument's data type
const misc::StringMap Argument::data_type_map = 
{
	{ "i1", DataTypeInt1 },
	{ "i8", DataTypeInt8 },
	{ "i16", DataTypeInt16 },
	{ "i32", DataTypeInt32 },
	{ "i64", DataTypeInt64 },
	{ "u1", DataTypeUInt1 },
	{ "u8", DataTypeUInt8 },
	{ "u16", DataTypeUInt16 },
	{ "u32", DataTypeUInt32 },
	{ "u64", DataTypeUInt64 },
	{ "float", DataTypeFloat },
	{ "double", DataTypeDouble },
	{ "struct", DataTypeStruct },
	{ "union", DataTypeUnion },
	{ "event", DataTypeEvent },
	{ "opaque", DataTypeOpaque }
};

// String map of the argument's dimension (2D or 3D)
const misc::StringMap Argument::dimension_map =
{
	{ "2D", 2 },
	{ "3D", 3 }
};

// String map of the argument's reflection for each data type
// FIXME: Still need to figure out reflection for i1 and u1
const misc::StringMap Argument::reflection_map =
{
	{ "char", DataTypeInt8 },
	{ "short", DataTypeInt16 },
	{ "int", DataTypeInt32 },
	{ "long", DataTypeInt64 },
	{ "uchar", DataTypeUInt8 },
	{ "ushort", DataTypeUInt16 },
	{ "uint", DataTypeUInt32 },
	{ "ulong", DataTypeUInt64 },
	{ "float", DataTypeFloat },
	{ "double", DataTypeDouble },
	{ "struct", DataTypeStruct },
	{ "union", DataTypeUnion },
	{ "event", DataTypeEvent },
	{ "opaque", DataTypeOpaque }
};


/* Infer argument size from its data type */
int Argument::getDataSize(DataType data_type)
{
	switch (data_type)
	{

	case DataTypeInt8:
	case DataTypeUInt8:
	case DataTypeStruct:
	case DataTypeUnion:
	case DataTypeEvent:
	case DataTypeOpaque:

		return 1;

	case DataTypeInt16:
	case DataTypeUInt16:

		return 2;

	case DataTypeInt32:
	case DataTypeUInt32:
	case DataTypeFloat:

		return 4;

	case DataTypeInt64:
	case DataTypeUInt64:
	case DataTypeDouble:

		return 8;

	default:

		throw misc::Panic(misc::fmt("Invalid data type (%d)",
				data_type));
	}
}


void PointerArgument::Dump(std::ostream &os)
{
	os << data_type_map.MapValue(data_type);
	if (num_elems > 1)
		os << '[' << num_elems << ']';
	os << "* " << name;
}

void PointerArgument::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	std::string data_type_str;
	std::string scope_str;
	std::string access_type_str;
	std::string line;
	std::string name_str;

	int data_size;


	data_type_str = data_type_map.MapValue(getDataType());
	data_size = Argument::getDataSize(data_type) * num_elems;
	scope_str = scope_map.MapValue(getScope());
	access_type_str = access_type_map.MapValue(getAccessType());

	line =
		";pointer:" + 
		name + ":" + data_type_str + ":1:" +
		std::to_string(constant_buffer_num) + ":" +
		std::to_string(constant_offset) + ":" + scope_str + ":" +
		std::to_string(buffer_num) + ":" + std::to_string(data_size) + ":" +
		access_type_str + ":0:0\n";


	/* Check for 16 byte alignment */
	if (((constant_offset - offset) < 16) && index)
		throw Disassembler::Error(misc::fmt("16 byte alignment not maintained "
				"in argument: %s - Expected offset of %d or "
				"higher",
				name.c_str(),
				offset + 16));

	offset = constant_offset;

	/* Mark which uav's are being used */
	if (scope == ScopeUAV
			&& !(uav[buffer_num]))
	{
		uav[buffer_num] = 1;
	}

	buffer->Write(line.c_str(), line.size());

	/* Include const_arg line only if pointer is marked with "const" */
	if (constarg)
	{
		line = ";constarg:" + std::to_string(index) + ":" + name + "\n";
		buffer->Write(line.c_str(), line.size());
	}

}

void PointerArgument::WriteReflection(ELFWriter::Buffer *buffer, 
		unsigned int index)
{
	std::string line;
	std::string reflection;

	reflection = reflection_map.MapValue(getDataType());

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
		throw Disassembler::Error("Invalid number of elements in argument " +
				name);
	}

	buffer->Write(line.c_str(), line.size());

}

void ValueArgument::Dump(std::ostream &os)
{
	os << data_type_map.MapValue(data_type);
	if (num_elems > 1)
		os << '[' << num_elems << ']';
	os << ' ' << name;
}


void ValueArgument::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	
	std::string data_type_str;
	std::string line;

	data_type_str = data_type_map.MapValue(getDataType());

	line = 
		";value:" + name + ":" +
		data_type_str + ":" + std::to_string(num_elems) + ":" +
		std::to_string(constant_buffer_num) + ":" +
		std::to_string(constant_offset) + "\n";
	
	/* Check for 16 byte alignment */
	if (((constant_offset - offset) < 16) && index)
		throw Disassembler::Error(misc::fmt("16 byte alignment not maintained "
				"in argument: %s - Expected offset of %d or "
				"higher",
				name.c_str(),
				offset + 16));

	offset = constant_offset;

	buffer->Write(line.c_str(), line.size());

	/* Include const_arg line only if pointer is marked with "const" */
	if (constarg)
	{
		line = ";constarg:" + std::to_string(index) + ":" + name + "\n";

		buffer->Write(line.c_str(), line.size());
	}

}

void ValueArgument::WriteReflection(ELFWriter::Buffer *buffer, unsigned int index)
{

	std::string line;
	std::string reflection;

	reflection = reflection_map.MapValue(getDataType());

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
		throw Disassembler::Error("Invalid number of elements in argument "
				+ name);
	}

	buffer->Write(line.c_str(), line.size());

}


void ImageArgument::Dump(std::ostream &os)
{
	os << dimension_map.MapValue(dimension);
	if (dimension > 1)
		os << '[' << dimension << ']';
	os << ' ' << name;
}


void ImageArgument::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	throw misc::Panic("Not implemented");
}


void ImageArgument::WriteReflection(ELFWriter::Buffer *buffer, unsigned int index)
{
	throw misc::Panic("Not implemented");
}


void SamplerArgument::Dump(std::ostream &os)
{
	throw misc::Panic("Not implemented");
}


void SamplerArgument::WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
		unsigned int &offset, int *uav)
{
	throw misc::Panic("Not implemented");
}


void SamplerArgument::WriteReflection(ELFWriter::Buffer *buffer, unsigned int index)
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

}  // namespace SI
