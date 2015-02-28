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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <runtime/include/GL/gl.h>

#include "input.h"

/*
 * Public Functions
 */

void SIInputCreate(SIInput *self)
{
	/* Initialize */
}

void SIInputDestroy(SIInput *input)
{
}

void SIInputSetUsageIndex(SIInput *input, unsigned int usage_index)
{
	if (input)
		input->usage_index = usage_index;
}

unsigned int SIInputGetType(unsigned int gl_data_type)
{
	switch(gl_data_type)
	{

	case GL_BYTE:
		return SIInputByte;
	case GL_UNSIGNED_BYTE:
		return SIInputUbyte;
	case GL_SHORT:
		return SIInputShort;
	case GL_UNSIGNED_SHORT:
		return SIInputUshort;
	case GL_INT:
		return SIInputInt;
	case GL_UNSIGNED_INT:
		return SIInputUint;
	case GL_HALF_FLOAT:
		return SIInputHfloat;
	case GL_FLOAT:
		return SIInputFloat;
	case GL_DOUBLE:
		return SIInputDouble;
	case GL_FIXED:
		return SIInputFixed;
	case GL_INT_2_10_10_10_REV:
		return SIInputInt2101010Rev;
	case GL_UNSIGNED_INT_2_10_10_10_REV:
		return SIInputUint2101010Rev;
	default:
		fatal("Unrecognized GLenum type=%x", gl_data_type);
		break;
	}

	/* Return */
	return 0;

}

/* Infer inputs size from its data type */
int SIInputGetDataSize(SIInputDataType data_type)
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
