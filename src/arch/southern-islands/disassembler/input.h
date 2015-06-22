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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_INPUT_H_OLD
#define ARCH_SOUTHERN_ISLANDS_ASM_INPUT_H_OLD

#include <lib/class/class.h>


/*
 * Class SIInput
 */

typedef enum 
{
	SIInputInvalid = 0,
	SIInputByte,
	SIInputUbyte,
	SIInputShort,
	SIInputUshort,
	SIInputInt,
	SIInputUint,
	SIInputHfloat,
	SIInputFloat,
	SIInputDouble,
	SIInputFixed,
	SIInputInt2101010Rev,
	SIInputUint2101010Rev
} SIInputDataType;

CLASS_BEGIN(SIInput, Object)

	unsigned int usage_index; /* The associated vertex attribute index */
	int set;  /* Set to true when it is assigned */

	SIInputDataType type;
	int num_elems;
	int size;
	unsigned int device_ptr; 

CLASS_END(SIInput)

void SIInputCreate(SIInput *self);
void SIInputDestroy(SIInput *input);


/*
 * Public
 */

void SIInputSetUsageIndex(SIInput *input, unsigned int usage_index);
unsigned int SIInputGetType(unsigned int gl_data_type);
int SIInputGetDataSize(SIInputDataType data_type);

#endif
