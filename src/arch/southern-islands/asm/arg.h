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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_ARG_H_OLD
#define ARCH_SOUTHERN_ISLANDS_ASM_ARG_H_OLD

#include <lib/class/class.h>


/*
 * Class 'SIArg'
 */

typedef enum
{
	SIArgTypeInvalid = 0,
	SIArgTypeValue,
	SIArgTypePointer,
	SIArgTypeImage,
	SIArgTypeSampler
} SIArgType;

typedef enum
{
	SIArgAccessTypeInvalid = 0,
	SIArgReadOnly,
	SIArgWriteOnly,
	SIArgReadWrite
} SIArgAccessType;

typedef enum
{
	SIArgScopeInavlid = 0,
	SIArgGlobal,
	SIArgEmuPrivate,
	SIArgEmuLocal,
	SIArgUAV,
	SIArgEmuConstant,
	SIArgEmuGDS,
	SIArgHwLocal,
	SIArgHwPrivate,
	SIArgHwConstant,
	SIArgHwGDS
} SIArgScope;

typedef enum
{
	SIArgDataTypeInvalid = 0,
	SIArgInt1,
	SIArgInt8,
	SIArgInt16,
	SIArgInt32,
	SIArgInt64,
	SIArgUInt1,
	SIArgUInt8,
	SIArgUInt16,
	SIArgUInt32,
	SIArgUInt64,
	SIArgFloat,
	SIArgDouble,
	SIArgStruct,
	SIArgUnion,
	SIArgEvent,
	SIArgOpaque
} SIArgDataType;

typedef struct
{
	/* Metadata info */
	SIArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;

	/* Value set by user */
	void *value_ptr;
} SIArgValue;

typedef struct
{
	/* Metadata info */
	SIArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	SIArgScope scope;
	int buffer_num;
	int alignment;
	SIArgAccessType access_type;

	/* Value set by user */
	unsigned int device_ptr;
} SIArgPointer;

typedef struct
{
	int dimension;  /* 2 or 3 */
	SIArgAccessType access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;
} SIArgImage;

typedef struct
{
	int id;
	unsigned int location;
	int value;
} SIArgSampler;


CLASS_BEGIN(SIArg, Object)

	SIArgType type;
	String *name;

	int set;  /* Set to true when it is assigned */
	int size; /* Inferred from metadata or user calls */
	int constarg; /*Set to true when argument is constant */

	union
	{
		SIArgValue value;
		SIArgPointer pointer;
		SIArgImage image;
		SIArgSampler sampler;
	};

CLASS_END(SIArg)


void SIArgCreate(SIArg *self, SIArgType type, char *name);
void SIArgDestroy(SIArg *self);

void SIArgSetName(SIArg *self, char *name);



/*
 * Public
 */

extern struct str_map_t si_arg_dimension_map;
extern struct str_map_t si_arg_access_type_map;
extern struct str_map_t si_arg_data_type_map;
extern struct str_map_t si_arg_scope_map;
extern struct str_map_t si_arg_reflection_map;

int SIArgGetDataSize(SIArgDataType data_type);
SIArg * SIArgCopy(SIArg *original);

#endif
