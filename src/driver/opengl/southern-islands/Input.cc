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

#include <src/lib/cpp/Misc.h>

#include "Input.h"

using namespace misc;

namespace SI
{

Input::Input(unsigned index, unsigned device_buffer_size, unsigned device_buffer_ptr, 
	SIInputDataType data_type, unsigned num_elems, bool isNormalized, 
	unsigned stride, unsigned device_buffer_offset)
{
	this->index = index;

	// Info from glBufferData
	this->device_buffer_size = device_buffer_size;
	this->device_buffer_ptr = device_buffer_ptr;
	
	// Info from glVertexAttribPointer
	this->data_type = data_type;
	this->num_elems = num_elems;
	this->isNormalized = isNormalized;
	this->stride = stride;
	this->device_buffer_offset = device_buffer_offset;
}

unsigned Input::getDataSize(SIInputDataType data_type) 
{
	switch (data_type)
	{
	case SIInputByte:
	case SIInputUbyte:

		return 1;

	case SIInputShort:
	case SIInputUshort:

		return 2;

	case SIInputInt:
	case SIInputUint:
	case SIInputHfloat:
	case SIInputFloat:
	case SIInputFixed:
	case SIInputInt2101010Rev:
	case SIInputUint2101010Rev:

		return 4;

	case SIInputDouble:

		return 8;

	default:
		panic("%s: invalid data type (%d)",
				__FUNCTION__, data_type);
		return 0;
	}
}

}  // namespace SI