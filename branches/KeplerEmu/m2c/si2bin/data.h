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

#ifndef M2C_SI2BIN_DATA_H_OLD
#define M2C_SI2BIN_DATA_H_OLD

#include <lib/class/class.h>


/*
 * Class 'Si2binData'
 */

typedef enum
{
	Si2binDataTypeInvalid = 0,

	Si2binDataFloat,
	Si2binDataWord,
	Si2binDataHalf,
	Si2binDataByte
} Si2binDataType;


CLASS_BEGIN(Si2binData, Object)

	Si2binDataType type;
	
	union
	{
		float float_value;
		unsigned int word_value;
		unsigned short half_value;
		unsigned char byte_value;
	};

CLASS_END(Si2binData)

void Si2binDataCreate(Si2binData *self, Si2binDataType type);
void Si2binDataDestroy(Si2binData *self);


#endif
