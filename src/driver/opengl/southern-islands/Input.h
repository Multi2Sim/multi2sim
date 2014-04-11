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

#ifndef DRIVER_OPENGL_SI_INPUT_H
#define DRIVER_OPENGL_SI_INPUT_H

namespace SI
{

enum SIInputDataType
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
};

class Input
{
	unsigned index;

	// Info from glBufferData
	unsigned device_buffer_size;
	unsigned device_buffer_ptr;
	
	// Info from glVertexAttribPointer
	SIInputDataType data_type;
	unsigned num_elems;
	unsigned isNormalized;
	unsigned stride;
	unsigned device_buffer_offset;

public:
	Input(unsigned index, unsigned device_buffer_size, unsigned device_buffer_ptr, 
		SIInputDataType data_type, unsigned num_elems, bool isNormalized, 
		unsigned stride, unsigned device_buffer_offset);

	/// Getters
	///
	/// Get data size based on data type
	static unsigned getDataSize(SIInputDataType data_type);
};


} // namespace SI

#endif
