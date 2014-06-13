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

#ifndef M2C_SI2BIN_OUTER_BIN_H
#define M2C_SI2BIN_OUTER_BIN_H

#include <memory>
#include <vector>

#include <src/lib/cpp/ELFWriter.h>

#include "InternalBinary.h"
#include "Metadata.h"

namespace si2bin
{

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

	friend class OuterBin;
	
	// Constructor
	Data(DataType type) { this->type = type; }
	
	virtual void Write(ELFWriter::Buffer *buffer) = 0;
	
	DataType type;

public:
	// Getter
	DataType getType() { return type; }

};

class DataFloat : public Data
{
	friend class OuterBin;
	
	float value;

	DataFloat(float value) :
	Data(DataTypeFloat), value(value) { }

	void Write(ELFWriter::Buffer *buffer) { buffer->Write((char *) &value, sizeof(float)); }
};

class DataWord : public Data
{
	friend class OuterBin;
	
	unsigned int value;

	DataWord(unsigned int value) :
		Data(DataTypeWord), value(value) { }

	void Write(ELFWriter::Buffer *buffer) { buffer->Write((char *) &value, 
			sizeof(unsigned int)); }
};


class DataHalf : public Data
{
	friend class OuterBin;
	
	unsigned short value;

	DataHalf(unsigned short value) :
		Data(DataTypeHalf), value(value) { }

	void Write(ELFWriter::Buffer *buffer) { buffer->Write((char *) &value, 
			sizeof(unsigned short)); }
};

class DataByte : public Data
{
	friend class OuterBin;
	
	unsigned char value;

	DataByte(unsigned char value) :
		Data(DataTypeByte), value(value) { }
	
	void Write(ELFWriter::Buffer *buffer) { buffer->Write((char *) &value, 
			sizeof(unsigned char)); }

};




enum OuterBinDevice
{
	OuterBinInvalid = 0,

	OuterBinCapeVerde,
	OuterBinPitcairn,
	OuterBinTahiti
};

/*
 * AMD External ELF Binary
 */

class OuterBin
{
	// Device Type
	OuterBinDevice device;

	std::vector<std::unique_ptr<Data>> data_list;

	/* ELF file create internally.
	 * Private field. */
	ELFWriter::File writer;

	std::vector<std::unique_ptr<InnerBin>> inner_bin_list;

	std::vector<std::unique_ptr<Metadata>> metadata_list;
	
public:
	
	// Getters

	///Returns the device type for the Outer Bin object
	OuterBinDevice getDevice() { return device; }

	///Returns a pointer to the Inner Binary object
	InnerBin *getInnerBin(unsigned int index)
	{
		return index < inner_bin_list.size() ? inner_bin_list[index].get()
				: nullptr;
	}

	///Returns a pointer to the Metadata object
	Metadata *getMetadata(unsigned int index)
	{
		return index < metadata_list.size() ? metadata_list[index].get()
				: nullptr;
	}
	
	///Returns a pointer to the data object at position <index> in the Data List
	Data *getData(unsigned int index)
	{
		return index < data_list.size() ? data_list[index].get()
				: nullptr;
	}

	///Returns the number of data objects in the Data List
	unsigned int getDataCount() { return data_list.size(); }

	// Setters
	///Set the device type for the Outer Bin object
	void setDevice(OuterBinDevice device) { this->device = device; }

	///Create a binary based on the provided assembly source file and output it to 
	///the file pointed by std::ostream &os
	void Generate(std::ostream& os);

	///Create a new Data object of type float and add it to the Data List
	void newDataFloat(float value)
	{
		data_list.push_back(std::unique_ptr<Data>(new DataFloat(value)));
	}

	///Create a new Data object of type word and add it to the Data List
	void newDataWord(unsigned int value)
	{
		data_list.push_back(std::unique_ptr<Data>(new DataWord(value)));
	}

	///Create a new Data object of type half and add it to the Data List
	void newDataHalf(unsigned short value)
	{
		data_list.push_back(std::unique_ptr<Data>(new DataHalf(value)));
	}

	///Create a new Data object of type byte and add it to the Data List
	void newDataByte(unsigned char value)
	{
		data_list.push_back(std::unique_ptr<Data>(new DataByte(value)));
	}

	///Create a new Inner Bin bject and add it to the Inner Bin List
	InnerBin *newInnerBin(const std::string &name)
	{ 
		inner_bin_list.push_back(std::unique_ptr<InnerBin>(new InnerBin(name)));
		return inner_bin_list.back().get();
	}
	
	///Create a new Metadata object and add it to the Metadata List
	Metadata *newMetadata()
	{ 
		metadata_list.push_back(std::unique_ptr<Metadata>(new Metadata()));
		return metadata_list.back().get();
	}

};

} // namespace si2bin

#endif

