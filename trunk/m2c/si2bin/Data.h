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

#ifndef M2C_SI2BIN_DATA_H
#define M2C_SI2BIN_DATA_H

namespace si2bin
{

/*
 * Class 'Data'
 */

enum DataType
{
	DataTypeInvalid = 0,

	DataTypeFloat,
	DataTypeWord,
	DataTypeHalf,
	DataTypeByte
};

class Data
{

protected:
	DataType type;
	
public:
	
	/* Constructor */
	Data(DataType type) { this->type = type; }
	
	/* Getter */
	DataType GetType() { return type; }

};

class DataFloat : public Data
{
	float value;

public:
	DataFloat(DataType type, float value) : 
		Data(type), value(value){}
	float GetValue() { return value; }

};

class DataWord : public Data
{
	unsigned int value;

public:
	DataWord(DataType type, unsigned int value) :
		Data(type), value(value){}
	unsigned int GetValue() { return value; }

};

class DataHalf : public Data
{
	unsigned short value;

public:
	DataHalf(DataType type, unsigned short value) :
		Data(type), value(value){}
	unsigned short GetValue() { return value; }

};

class DataByte : public Data
{
	unsigned char value;

public:
	DataByte(DataType type, unsigned char value) :
		Data(type), value(value){}
	unsigned char GetValue() { return value; }

};


} /* Namespace si2bin */

#endif
