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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_ARG_H
#define ARCH_SOUTHERN_ISLANDS_ASM_ARG_H

#include <memory>

#include <lib/cpp/ELFWriter.h>
#include <lib/cpp/String.h>


namespace SI
{

enum ArgType
{
	ArgTypeInvalid = 0,
	ArgTypeValue,
	ArgTypePointer,
	ArgTypeImage,
	ArgTypeSampler
};

extern misc::StringMap arg_dimension_map;

enum ArgAccessType
{
	ArgAccessTypeInvalid = 0,
	ArgAccessTypeReadOnly,
	ArgAccessTypeWriteOnly,
	ArgAccessTypeReadWrite
};

extern misc::StringMap arg_scope_map;
enum ArgScope
{
	ArgScopeInavlid = 0,
	ArgScopeGlobal,
	ArgScopeEmuPrivate,
	ArgScopeEmuLocal,
	ArgScopeUAV,
	ArgScopeEmuConstant,
	ArgScopeEmuGDS,
	ArgScopeHwLocal,
	ArgScopeHwPrivate,
	ArgScopeHwConstant,
	ArgScopeHwGDS
};

enum ArgDataType
{
	ArgDataTypeInvalid = 0,
	ArgDataTypeInt1,
	ArgDataTypeInt8,
	ArgDataTypeInt16,
	ArgDataTypeInt32,
	ArgDataTypeInt64,
	ArgDataTypeUInt1,
	ArgDataTypeUInt8,
	ArgDataTypeUInt16,
	ArgDataTypeUInt32,
	ArgDataTypeUInt64,
	ArgDataTypeFloat,
	ArgDataTypeDouble,
	ArgDataTypeStruct,
	ArgDataTypeUnion,
	ArgDataTypeEvent,
	ArgDataTypeOpaque
};

extern misc::StringMap arg_access_type_map;
extern misc::StringMap arg_data_type_map;

class Arg
{
	ArgType type;
	std::string name;

	bool set;  /* Set to true when it is assigned */
	int size; /* Inferred from metadata or user calls */
	bool constarg; /* Set to true when argument is constant */

public:
	Arg(ArgType type, const std::string &name) :
			type(type), name(name) { }

	/* Getters */
	ArgType getType() { return type; }
	std::string getName() { return name; }
	bool getConstArg() { return constarg; }
	int getSize() const { return size; }
	bool isSet() const { return set; }

	/* Setters */
	void setName(const std::string &name) { this->name = name; }
	void setConstArg(bool constarg) { this->constarg = constarg; }
	void setSize(int size) { this->size = size; }
	void setSetFlag(bool flag) { this->set = flag; }

	/* Dump */
	virtual void Dump(std::ostream &os) { os << name; }
	friend std::ostream &operator<<(std::ostream &os, Arg &arg) {
			arg.Dump(os); return os; }

	static int getDataSize(ArgDataType data_type);

	virtual void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index, 
			unsigned int &offset, int *uav) = 0;
	virtual void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index) = 0;
};


class ArgPointer : public Arg
{
	/* Metadata info */
	ArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	ArgScope scope;
	int buffer_num;
	int alignment;
	ArgAccessType access_type;

	/* Value set by user */
	unsigned int device_ptr;

public:
	ArgPointer(const std::string &name,
			ArgDataType data_type,
			int num_elems,
			int constant_buffer_num,
			int constant_offset,
			ArgScope scope,
			int buffer_num,
			int alignment,
			ArgAccessType access_type) :
				Arg(ArgTypePointer, name),
				data_type(data_type),
				num_elems(num_elems),
				constant_buffer_num(constant_buffer_num),
				scope(scope),
				buffer_num(buffer_num),
				alignment(alignment),
				access_type(access_type) { };

	/* Getters */
	ArgDataType getDataType() { return data_type; }
	int getNumElems() { return num_elems; }
	ArgScope getScope() { return scope; }
	ArgAccessType getAccessType() { return access_type; }
	int getConstantBufferNum() { return constant_buffer_num; }
	int getConstantOffset() { return constant_offset; }
	int getBufferNum() { return buffer_num; }
	unsigned int getDevicePtr() const { return device_ptr; }

	/* Setters */
	void setDevicePtr(unsigned int device_ptr) {
		this->device_ptr = device_ptr; }
	void setDataType(ArgDataType data_type) { this->data_type = data_type; }
	void setNumElems(int num_elems) { this->num_elems = num_elems; }
	void setConstantBufferNum(int constant_buffer_num) { this->constant_buffer_num = constant_buffer_num; }
	void setConstantOffset(int constant_offset) { this->constant_offset = constant_offset; }
	void setScope(ArgScope scope) { this->scope = scope; }
	void setBufferNum(int buffer_num) { this->buffer_num = buffer_num; }
	void setAlignment(int alignment) { this->alignment = alignment; }
	void setAccessType(ArgAccessType access_type) { this->access_type = access_type; }

	/* Dump */
	void Dump(std::ostream &os);
	
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);
};


class ArgValue : public Arg
{
	/* Metadata info */
	ArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;

	/* Value set by user */
	std::unique_ptr<void> value;

public:
	ArgValue(const std::string &name,
			ArgDataType data_type,
			int num_elems,
			int constant_buffer_num,
			int constant_offset) :
				Arg(ArgTypeValue, name),
				data_type(data_type),
				num_elems(num_elems),
				constant_buffer_num(constant_buffer_num),
				constant_offset(constant_offset) { }

	/* Gtters */
	ArgDataType getDataType() { return data_type; }
	int getNumElems() { return num_elems; }
	int getConstantBufferNum() const { return constant_buffer_num; }
	int getConstantOffset() const { return constant_offset; }
	void *getValuePtr() const { return value.get(); }

	/* Setters */
	void setDataType(ArgDataType data_type) { this->data_type = data_type; }
	void setNumElems(int num_elems) { this->num_elems = num_elems; }
	void setConstantBufferNum(int constant_buffer_num) { this->constant_buffer_num = constant_buffer_num; }
	void setConstantOffset(int constant_offset) { this->constant_offset = constant_offset; }
	void setValue(void *value) { this->value.reset(value); }

	/* Dump */
	void Dump(std::ostream &os);
	
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);
};


class ArgImage : public Arg
{
	int dimension;  /* 2 or 3 */
	ArgAccessType access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;

public:

	ArgImage(const std::string &name,
			int dimension,
			ArgAccessType access_type,
			int uav,
			int constant_buffer_num,
			int constant_offset) :
				Arg(ArgTypeImage, name),
				dimension(dimension),
				access_type(access_type),
				uav(uav),
				constant_buffer_num(constant_buffer_num),
				constant_offset(constant_offset) { }

	/* Dump */
	void Dump(std::ostream &os);
	
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);

};


class ArgSampler : public Arg
{
	int id;
	unsigned int location;
	int value;

public:

	ArgSampler(const std::string &name,
			int id,
			unsigned int location,
			int value) :
				Arg(ArgTypeSampler, name),
				id(id),
				location(location),
				value(value) { }

	/* Dump */
	void Dump(std::ostream &os);
	
	void WriteInfo(ELFWriter::Buffer *buffer, unsigned int index,
			unsigned int &offset, int *uav);
	void WriteReflection(ELFWriter::Buffer *buffer, unsigned int index);
};


}  /* namespace SI */

#endif

