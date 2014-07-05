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

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Input.h"


namespace SI
{

unsigned Input::getDataType(InputDataType data_type)
{
	switch (data_type)
	{
	case InputDataTypeByte:
	case InputDataTypeUbyte:

		return 1;

	case InputDataTypeShort:
	case InputDataTypeUshort:

		return 2;

	case InputDataTypeInt:
	case InputDataTypeUint:
	case InputDataTypeHfloat:
	case InputDataTypeFloat:
	case InputDataTypeFixed:
	case InputDataTypeInt2101010Rev:
	case InputDataTypeUint2101010Rev:

		return 4;

	case InputDataTypeDouble:

		return 8;

	default:

		throw misc::Panic(misc::fmt("Invalid data type (%d)",
				data_type));
	}

}

	
}  // namespace SI
