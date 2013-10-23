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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_ARG_H
#define ARCH_SOUTHERN_ISLANDS_ASM_ARG_H


namespace SI
{

enum ArgType
{
	ArgTypeInvalid = 0,
	ArgTypeValue,
	ArgTypePointer,
	ArgTypeImage,
	ArgTypeSampler
};

enum ArgAccessType
{
	ArgAccessTypeInvalid = 0,
	ArgReadOnly,
	ArgWriteOnly,
	ArgReadWrite
};

enum ArgScope
{
	ArgScopeInavlid = 0,
	ArgGlobal,
	ArgEmuPrivate,
	ArgEmuLocal,
	ArgUAV,
	ArgEmuConstant,
	ArgEmuGDS,
	ArgHwLocal,
	ArgHwPrivate,
	ArgHwConstant,
	ArgHwGDS
};

enum ArgDataType
{
	ArgDataTypeInvalid = 0,
	ArgInt1,
	ArgInt8,
	ArgInt16,
	ArgInt32,
	ArgInt64,
	ArgUInt1,
	ArgUInt8,
	ArgUInt16,
	ArgUInt32,
	ArgUInt64,
	ArgFloat,
	ArgDouble,
	ArgStruct,
	ArgUnion,
	ArgEvent,
	ArgOpaque
};

struct ArgValue
{
	/* Metadata info */
	ArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;

	/* Value set by user */
	void *value_ptr;
};

struct ArgPointer
{
	/* Metadata info */
	ArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	ArgScope scope;
	int buffer_num;
	int alignment;
	ArgAccessType access_type;

	/* Value set by user */
	unsigned int device_ptr;
};

struct ArgImage
{
	int dimension;  /* 2 or 3 */
	ArgAccessType access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;
};

struct ArgSampler
{
	int id;
	unsigned int location;
	int value;
};


class Arg
{

	ArgType type;
	std::string name;

	int set;  /* Set to true when it is assigned */
	int size; /* Inferred from metadata or user calls */
	int constarg; /*Set to true when argument is constant */

	union
	{
		ArgValue value;
		ArgPointer pointer;
		ArgImage image;
		ArgSampler sampler;
	};

public:
	Arg(ArgType type, const std::string &name);

	ArgType GetType() { return type; }
	std::string GetName() { return name; }
	
	static int GetDataSize(ArgDataType data_type);
};


/*
 * Public
 */

/*
struct StringMapWrap;
extern struct StringMapWrap *si_arg_dimension_map;
extern struct StringMapWrap *si_arg_access_type_map;
extern struct StringMapWrap *si_arg_data_type_map;
extern struct StringMapWrap *si_arg_scope_map;
extern struct StringMapWrap *si_arg_reflection_map;
*/

}  /* namespace SI */

#endif
