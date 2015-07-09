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

#ifndef ARCH_SOUTHERN_ISLANDS_DISASSEMBLER_ARGUMENT_H
#define ARCH_SOUTHERN_ISLANDS_DISASSEMBLER_ARGUMENT_H

#include <memory>

#include <lib/cpp/ELFWriter.h>
#include <lib/cpp/String.h>


namespace SI
{



class Argument
{
public: 

	/// Argument type
	enum Type
	{
		TypeInvalid = 0,
		TypeValue,
		TypePointer,
		TypeImage,
		TypeSampler
	};

	/// Type of access that the argument has
	enum AccessType
	{
		AccessTypeInvalid = 0,
		AccessTypeReadOnly,
		AccessTypeWriteOnly,
		AccessTypeReadWrite
	};
	
	/// String map of the argument's access type
	static const misc::StringMap access_type_map;
	
	/// Scope of the argument
	enum Scope
	{
		ScopeInavlid = 0,
		ScopeGlobal,
		ScopeEmuPrivate,
		ScopeEmuLocal,
		ScopeUAV,
		ScopeEmuConstant,
		ScopeEmuGDS,
		ScopeHwLocal,
		ScopeHwPrivate,
		ScopeHwConstant,
		ScopeHwGDS
	};
	
	/// String map of the argument's scope
	static const misc::StringMap scope_map;
	
	/// Data type of the argument
	enum DataType
	{
		DataTypeInvalid = 0,
		DataTypeInt1,
		DataTypeInt8,
		DataTypeInt16,
		DataTypeInt32,
		DataTypeInt64,
		DataTypeUInt1,
		DataTypeUInt8,
		DataTypeUInt16,
		DataTypeUInt32,
		DataTypeUInt64,
		DataTypeFloat,
		DataTypeDouble,
		DataTypeStruct,
		DataTypeUnion,
		DataTypeEvent,
		DataTypeOpaque
	};

	/// String map of the argument's data type
	static const misc::StringMap data_type_map;
	
	/// String map of the argument's dimension (2D or 3D)
	static const misc::StringMap dimension_map; 
	
	/// String map of the argument's reflection for each data type
	static const misc::StringMap reflection_map;

private:
	Type type;
	
public:
	/// Constructor
	Argument(Type type, const std::string &name) :
			type(type), name(name) { }
	
	/// Size of the argument - inferred from metadata or user calls
	int size;

	/// Name of the argument
	std::string name;

	// Set to true when it is assigned
	bool set;
	
	// Set to true when argument is constant
	bool constarg; 




	// 
	// Getters
	//
	
	/// Get the type of the argument
	Type getType() const { return type; }
	
	/// Dump
	virtual void Dump(std::ostream &os = std::cout) { os << name; }
	friend std::ostream &operator<<(std::ostream &os, Argument &argument) {
			argument.Dump(os); return os; }

	/// Static function to get the size of a data type
	///
	/// \param data_type Enum describing the data type to get the size of
	///
	/// \return The size of the data type
	static int getDataSize(DataType data_type);

	/// Write info from a UAV to a buffer
	///
	/// \param buffer Pointer to an ELFWriter buffer to write to.
	///
	/// \param index Index of the argument
	///
	/// \param offset Offset of the argument in the UAV. The constant
	/// offset is return in this argument.
	/// 
	/// \param uav Pointer to the uav array 
	virtual void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
			unsigned int &offset, int *uav) = 0;
	
	/// Create a reflection from a buffer
	///
	/// \param buffer Pointer to an ELFWriter buffer to write to
	///
	/// \param index Index of the argument
	virtual void WriteReflection(ELFWriter::Buffer *buffer, 
			unsigned int index) = 0;
};


class PointerArgument : public Argument
{
	// Metadata info
	DataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	Scope scope;
	int buffer_num;
	int alignment;
	AccessType access_type;

	// Value set by user
	unsigned int device_ptr;

public:
	PointerArgument(const std::string &name,
			DataType data_type,
			int num_elems,
			int constant_buffer_num,
			int constant_offset,
			Scope scope,
			int buffer_num,
			int alignment,
			AccessType access_type) :
				Argument(TypePointer, name),
				data_type(data_type),
				num_elems(num_elems),
				constant_buffer_num(constant_buffer_num),
				constant_offset(constant_offset),
				scope(scope),
				buffer_num(buffer_num),
				alignment(alignment),
				access_type(access_type) { };

	//
	// Getters
	//

	/// Get data type
	DataType getDataType() const { return data_type; }
	
	/// Get the number of elements in the argument
	int getNumElems() const { return num_elems; }

	/// Get the scope of the argument
	Scope getScope() const { return scope; }

	/// Get the access type of the argument
	AccessType getAccessType() const { return access_type; }

	/// Get the constant buffer number
	int getConstantBufferNum() const { return constant_buffer_num; }
	
	/// Get the constant offset
	int getConstantOffset() const { return constant_offset; }

	/// Get the buffer number
	int getBufferNum() const { return buffer_num; }

	/// Get the device pointer 
	unsigned int getDevicePtr() const { return device_ptr; }




	// 
	// Setters
	//

	/// Set the device pointer
	void setDevicePtr(unsigned int device_ptr) {
		this->device_ptr = device_ptr; }
	
	/// Set the data type of the argument
	void setDataType(DataType data_type) { this->data_type = data_type; }

	/// Set the number of elements in the argument
	void setNumElems(int num_elems) { this->num_elems = num_elems; }

	/// Set the constant buffer number
	void setConstantBufferNum(int constant_buffer_num) { this->constant_buffer_num = constant_buffer_num; }

	/// Set the constant offset
	void setConstantOffset(int constant_offset) { this->constant_offset = constant_offset; }

	/// Set the scope of the argument
	void setScope(Scope scope) { this->scope = scope; }

	/// Set the buffer number
	void setBufferNum(int buffer_num) { this->buffer_num = buffer_num; }

	/// Set the alignment of the argument
	void setAlignment(int alignment) { this->alignment = alignment; }
	
	/// Set the access type of the argument
	void setAccessType(AccessType access_type) { this->access_type = access_type; }

	/// Dump
	void Dump(std::ostream &os = std::cout);

	/// Pointer Argument implementation of virtual function WriteInfo
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);

	/// Pointer Argument implementation of virtual function WriteReflection
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);
};


class ValueArgument : public Argument
{
	// Metadata info
	DataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;

	// Value set by user
	std::unique_ptr<char[]> value;

public:
	ValueArgument(const std::string &name,
			DataType data_type,
			int num_elems,
			int constant_buffer_num,
			int constant_offset) :
				Argument(TypeValue, name),
				data_type(data_type),
				num_elems(num_elems),
				constant_buffer_num(constant_buffer_num),
				constant_offset(constant_offset) { }




	// 
	// Getters
	//

	/// Get the data type of the argument
	DataType getDataType() const { return data_type; }

	/// Get the number of elements in the argument
	int getNumElems() const { return num_elems; }
	
	/// Get the constant buffer number
	int getConstantBufferNum() const { return constant_buffer_num; }
	
	/// Get the constant offset
	int getConstantOffset() const { return constant_offset; }
	
	/// Get the value held in the argument
	///
	/// \return Returns a void pointer to the value
	void *getValue() const { return value.get(); }




	// 
	// Setters
	//

	/// Set the data type of the argument
	void setDataType(DataType data_type) { this->data_type = data_type; }

	/// Set the number of elements in the argument
	void setNumElems(int num_elems) { this->num_elems = num_elems; }

	/// Set the constant buffer number
	void setConstantBufferNum(int constant_buffer_num) { this->constant_buffer_num = constant_buffer_num; }

	/// Set the constant offset
	void setConstantOffset(int constant_offset) { this->constant_offset = constant_offset; }

	/// Set the value 
	///
	/// \param value A void pointer to the buffer holding the value
	void setValue(std::unique_ptr<char[]> &&value) { this->value = std::move(value); }

	/// Dump 
	void Dump(std::ostream &os = std::cout);
	
	/// Value Argument implementation of virtual function WriteInfo
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);
	
	/// Value Argument implementation of virtual function WriteReflection
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);
};


class ImageArgument : public Argument
{
	// Dimension is 2 or 3
	int dimension;
	AccessType access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;

public:

	ImageArgument(const std::string &name,
			int dimension,
			AccessType access_type,
			int uav,
			int constant_buffer_num,
			int constant_offset) :
				Argument(TypeImage, name),
				dimension(dimension),
				access_type(access_type),
				uav(uav),
				constant_buffer_num(constant_buffer_num),
				constant_offset(constant_offset) { }

	/// Dump 
	void Dump(std::ostream &os = std::cout);
	
	/// Image Argument implementation of virtual function WriteInfo
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);
	
	/// Image Argument implementation of virtual function WriteReflection
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);

};


class SamplerArgument : public Argument
{
	int id;
	unsigned int location;
	int value;

public:

	SamplerArgument(const std::string &name,
			int id,
			unsigned int location,
			int value) :
				Argument(TypeSampler, name),
				id(id),
				location(location),
				value(value) { }

	/// Dump
	void Dump(std::ostream &os = std::cout);
	
	/// Sampler Argument implementation of virtual function WriteInfo
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);
	
	/// Sampler Argument implementation of virtual function WriteReflection
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);
};


}  // namespace SI

#endif

