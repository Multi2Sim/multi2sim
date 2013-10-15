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

#include "Data.h"

namespace si2bin
{

/*
 * Class 'Data'
 */

Data::Data(DataType type)
{
	this->type = type;
}

DataFloat::DataFloat(DataType type, float value) : Data(type)
{
	this->value = value;
}

DataWord::DataWord(DataType type, unsigned int value) : Data(type)
{
	this->value = value;
}

/*DataHalf::DataHalf(Datatype type, unsigned short value) : Data(type)
{
	this->value = value;
}*/

DataByte::DataByte(DataType type, unsigned char value) : Data(type)
{
	this->value = value;
}

} /* namespace si2bin */
