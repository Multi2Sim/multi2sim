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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_INPUT_H
#define ARCH_SOUTHERN_ISLANDS_ASM_INPUT_H

namespace SI
{

enum InputDataType
{
	InputDataTypeInvalid = 0,
	InputDataTypeByte,
	InputDataTypeUbyte,
	InputDataTypeShort,
	InputDataTypeUshort,
	InputDataTypeInt,
	InputDataTypeUint,
	InputDataTypeHfloat,
	InputDataTypeFloat,
	InputDataTypeDouble,
	InputDataTypeFixed,
	InputDataTypeInt2101010Rev,
	InputDataTypeUint2101010Rev
};

// Input is created by runtime API calls glVertexAttribPointer
// The .input section in GL shader binary overrules runtime 
// created inputs. They are 'matched' in fetch shader.
class Input
{
	unsigned vertex_attrib_index;

	InputDataType data_type;
	unsigned num_elems;
	unsigned data_size;
	bool normalized;
	
	unsigned device_ptr;

public:
	Input();

	// Getters
	unsigned getDataType(InputDataType data_type);
	
};

} // namespace SI

#endif
