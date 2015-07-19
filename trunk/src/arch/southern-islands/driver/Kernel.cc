/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <arch/southern-islands/emulator/Emulator.h>
#include <arch/southern-islands/emulator/NDRange.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <string.h>

#include "Kernel.h"
#include "Program.h"


namespace SI
{

static const char *OpenCLErrSIKernelSymbol =
	"\tThe ELF file analyzer is trying to find a name in the ELF symbol "
	"table. If it is not found, it probably means that your application is "
	"requesting execution of a kernel function that is not present in the "
	"encoded binary. Please, check the parameters passed to the "
	"'clCreateKernel' function in your application.\n"
	"\n"
	"\tThis could also be a symptom of compiling an OpenCL kernel source "
	"on a tmachine with an installation of the AMD SDK (using 'm2c') "
	"but with an incorrect or missing installation of the GPU driver. In "
	"this case, the tool will still compile the kernel into LLVM, but the "
	"ISA section will be missing in the kernel binary.";

static const char *OpenCLErrSIKernelMetadata =
	"\tThe kernel binary loaded by your application is a valid ELF file. "
	"In this file, a '.rodata' section contains specific information about "
	"the OpenCL kernel. However, this information is only partially "
	"supported by Multi2Sim. To request support for this error, please "
	"report a bug on www.multi2sim.org.";


void Kernel::Expect(std::vector<std::string> &token_list,
		std::string head_token)
{
	std::string token = token_list.at(0);
	if (token != head_token)
		throw Driver::Error(misc::fmt("Token '%s' expected, "
				"'%s' found.\n%s",
				head_token.c_str(),
				token_list[0].c_str(),
				OpenCLErrSIKernelMetadata));
}

void Kernel::ExpectInt(std::vector<std::string> &token_list)
{
	std::string token = token_list.at(0);
	misc::StringError err;
	StringToInt(token, err);
	if (err)
		throw Driver::Error(misc::fmt("Integer number expected, "
				"'%s' found.\n%s",
				token.c_str(),
				OpenCLErrSIKernelMetadata));
}

void Kernel::ExpectCount(std::vector<std::string> &token_list, unsigned count)
{
	std::string head_token = token_list.at(0);
	if (token_list.size() != count)
		throw Driver::Error(misc::fmt("%d tokens expected for '%s', "
				"%d found.\n%s",
				count, head_token.c_str(),
				(unsigned) token_list.size(),
				OpenCLErrSIKernelMetadata));
}

void Kernel::LoadMetaDataV3()
{
	// Load metadata content
	std::istringstream metadata_stream;
	metadata_symbol->getStream(metadata_stream);

 	bool err;
	std::string line;
	std::string token;
	std::vector<std::string> token_list;

	// Stream starts at ARGSTART again so skip first two lines
	// (ARGSTART and version number)
	std::getline(metadata_stream, line);
	std::getline(metadata_stream, line);

	for (;;)
	{
		// Read the next line
		std::getline(metadata_stream, line);
		Driver::debug << misc::fmt("\t%s\n", line.c_str());
		misc::StringTokenize(line, token_list, ";:");

		// Stop when ARGEND is found or line is empty
		if (!token_list.size() || token_list.front() == "ARGEND")
		{
			token_list.clear();
			break;
		}

		// Kernel argument metadata

		// Value
		if (token_list.front() == "value")
		{
			// 6 tokens expected
			ExpectCount(token_list, 6);

			// Token 1 - Name
			token_list.erase(token_list.begin());
			std::string name = *token_list.begin();

			// Token 2 - Data type
			token_list.erase(token_list.begin());
			const char *data_type_string = 
					(*token_list.begin()).c_str();
			int data_type_int = Argument::data_type_map.MapString(
					*token_list.begin(), err);
			Argument::DataType data_type = 
					static_cast<Argument::DataType>(
					data_type_int);
			if (err)
				throw Driver::Error(misc::fmt("Invalid data "
						"type: %s.\n%s",
						data_type_string,
						OpenCLErrSIKernelMetadata));

			// Token 3 - Number of elements
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int num_elems = misc::StringToInt(*token_list.begin());
			assert(num_elems > 0);

			// Token 4 - Constant buffer
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			Expect(token_list, "1");
			int constant_buffer_num = misc::StringToInt(
					*token_list.begin());

			// Token 5 - Constant offset
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int constant_offset = misc::StringToInt(
					*token_list.begin());

			// Create argument object
			std::unique_ptr<SI::Argument> arg(
					new SI::ValueArgument(
					name, data_type, num_elems,
					constant_buffer_num, constant_offset));

			// Debug
			Driver::debug << misc::fmt("\targument '%s' - "
				"value stored in constant buffer %d at "
				"offset %d\n", name.c_str(), 
				constant_buffer_num, constant_offset);

			// Add argument and clear token list
			arguments.push_back(std::move(arg));
			token_list.clear();
			continue;
		}

		// Pointer
		if (token_list.front() == "pointer")
		{
			// APP SDK 2.5 supplies 9 tokens, 2.6 supplies
			// 10 tokens. Metadata version 3:1:104 (as specified
			// in entry 'version') uses 12 items.
			ExpectCount(token_list, 12);

			// Token 1 - Name
			token_list.erase(token_list.begin());
			std::string name = *token_list.begin();
		
			// Token 2 - Data type
			token_list.erase(token_list.begin());
			const char *data_type_string = 
					(*token_list.begin()).c_str();
			int data_type_int = Argument::data_type_map.MapString(
					*token_list.begin(), err);
			Argument::DataType data_type = 
					static_cast<Argument::DataType>(
					data_type_int);
			if (err)
				throw Driver::Error(misc::fmt("Invalid data "
						"type: %s\n%s",
						data_type_string,
						OpenCLErrSIKernelMetadata));

			// Token 3 - Number of elements
			// Arrays of pointers not supported,
			// only "1" allowed.
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			Expect(token_list, "1");
			int num_elems = misc::StringToInt(*token_list.begin());

			// Token 4 - Constant buffer
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			Expect(token_list, "1");
			int constant_buffer_num = misc::StringToInt(
					*token_list.begin());

			// Token 5 - Constant offset
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int constant_offset = misc::StringToInt(
					*token_list.begin());

			// Token 6 - Memory scope
			token_list.erase(token_list.begin());
			const char *arg_scope_string = (
					*token_list.begin()).c_str();
			int arg_scope_int = Argument::scope_map.MapString(
					*token_list.begin(), err);
			Argument::Scope arg_scope = 
					static_cast<Argument::Scope>(
					arg_scope_int);
			if (err)
				throw Driver::Error(misc::fmt("Invalid scope: "
						"%s\n%s", arg_scope_string,
						OpenCLErrSIKernelMetadata));

			// Token 7 - Buffer number
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int buffer_num = misc::StringToInt(*token_list.begin());

			// Token 8 - Alignment
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int alignment = misc::StringToInt(*token_list.begin());

			// Token 9 - Access type
			token_list.erase(token_list.begin());
			const char *arg_access_type_string = 
					(*token_list.begin()).c_str();
			int access_type_int = 
					Argument::access_type_map.MapString(
					*token_list.begin(), err);
			Argument::AccessType access_type = 
					static_cast<Argument::AccessType>(
					access_type_int);
			if (err)
				throw Driver::Error(misc::fmt("Invalid access "
						"type: %s\n%s", 
						arg_access_type_string,
						OpenCLErrSIKernelMetadata));

			// Token 10 - ???
			token_list.erase(token_list.begin());
			Expect(token_list, "0");

			// Token 11 - ???
			token_list.erase(token_list.begin());
			Expect(token_list, "0");

			// Create argument object
			std::unique_ptr<SI::Argument> 
					arg(new SI::PointerArgument(name, 
					data_type, num_elems, 
					constant_buffer_num, constant_offset, 
					arg_scope, buffer_num, alignment, 
					access_type));
			
			// Debug
			Driver::debug << misc::fmt("\targument '%s' - "
				"Pointer stored in constant buffer %d at "
				"offset %d\n", name.c_str(), 
				constant_buffer_num, constant_offset);

			// Add argument and clear token list
			arguments.push_back(std::move(arg));
			token_list.clear();
			continue;
		}

		// Image
		if (token_list.front() == "image")
		{
			// 7 tokens expected
			ExpectCount(token_list, 7);

			// Token 1 - Name
			token_list.erase(token_list.begin());
			std::string name = *token_list.begin();

			// Token 2 - Dimension
			token_list.erase(token_list.begin());
			int dimension = misc::StringToInt(*token_list.begin());
			const char*dimension_string = 
					Argument::dimension_map.MapValue(
					dimension, err);
			if (err)
				throw Driver::Error(misc::fmt("Invalid image "
						"dimensions: %s\n%s",
						dimension_string,
						OpenCLErrSIKernelMetadata));

			// Token 3 - Access type
			token_list.erase(token_list.begin());
			int access_type_int = misc::StringToInt(*token_list.begin());
			Argument::AccessType access_type = 
					static_cast<Argument::AccessType>(
					access_type_int);
			if (err)
				throw Driver::Error(misc::fmt("Invalid access "
						"type: %s\n%s", token.c_str(),
						OpenCLErrSIKernelMetadata));

			// Token 4 - UAV
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int uav = misc::StringToInt(*token_list.begin());

			// Token 5 - Constant buffer
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			Expect(token_list, "1");
			int constant_buffer_num = misc::StringToInt(
					*token_list.begin());

			// Token 6 - Constant offset
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int constant_offset = misc::StringToInt(
					*token_list.begin());

			// Create argument object
			std::unique_ptr<SI::Argument> arg(new SI::ImageArgument(
					name, dimension, access_type,uav, 
					constant_buffer_num, constant_offset));

			// Debug
			Driver::debug << misc::fmt("\targument '%s' - Image stored in "
				"constant buffer %d at offset %d\n",
				name.c_str(), constant_buffer_num,
				constant_offset);

			// Add argument and clear token list
			arguments.push_back(std::move(arg));
			token_list.clear();
			continue;
		}

		// Sampler
		if (token_list.front() == "sampler")
		{
			// 5 tokens expected
			ExpectCount(token_list, 5);

			// Token 1 - Name
			token_list.erase(token_list.begin());
			std::string name = *token_list.begin();

			// Token 2 - ID
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int id = misc::StringToInt(*token_list.begin());

			// Token 3 - Location
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int location = misc::StringToInt(*token_list.begin());

			// Token 4 - Value
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int value = misc::StringToInt(*token_list.begin());

			// Create argument object
			std::unique_ptr<SI::Argument> arg(
					new SI::SamplerArgument(name, id, 
					location, value));

			// Add argument and clear token list
			arguments.push_back(std::move(arg));
			token_list.clear();
		}

		// Non-kernel argument metadata

		// Memory
		// Used to let the GPU know how much local and private memory
		// is required for a kernel, where it should be allocated,
		// as well as other information.
		if (token_list.front() == "memory")
		{
			// Token 1 - Memory scope
			token_list.erase(token_list.begin());
			if (token_list.front() == "hwprivate")
			{
				// FIXME Add support for private memory by
				// adding space in global memory

				// Token 2 - ???
				token_list.erase(token_list.begin());
				Expect(token_list, "0");
			}
			else if (token_list.front() == "hwregion")
			{
				// 2 more tokens expected
				ExpectCount(token_list, 2);

				// Token 2 - ???
				token_list.erase(token_list.begin());
				Expect(token_list, "0");
			}
			else if (token_list.front() == "hwlocal")
			{
				// 2 more tokens expected
				ExpectCount(token_list, 2);

				// Token 2 - Size of local memory
				token_list.erase(token_list.begin());
				ExpectInt(token_list);
				this->local_memory_size = misc::StringToInt(
						*token_list.begin());
			}
			else if (token_list.front() == "datareqd")
			{
				// 1 more token expected
				ExpectCount(token_list, 1);
			}
			else if (token_list.front() == "uavprivate")
			{
				// 2 more tokens expected
				ExpectCount(token_list, 2);
			}
			else
			{
				throw Driver::Error(misc::fmt("Unsupported "
						"metadata: %s\n%s",
						token.c_str(),
						OpenCLErrSIKernelMetadata));
			}

			// Next
			token_list.clear();
			continue;
		}

		// Function
		// Used for multi-kernel compilation units.
		if (token_list.front() == "function")
		{
			// Expect 3 token
			ExpectCount(token_list, 3);

			// Token 1 - ???
			token_list.erase(token_list.begin());
			Expect(token_list, "1");

			// Token 2 - Function ID
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			this->func_uniqueid = misc::StringToInt(
					*token_list.begin());

			// Next
			token_list.clear();
			continue;
		}

		// Reflection
		// Format: reflection:<arg_id>:<type>
		// Observed first in version 3:1:104 of metadata.
		// This entry specifies the type of the argument, as
		// specified in the OpenCL kernel function header. It is
		// currently ignored, since this information is extracted from
		// the argument descriptions in 'value' and 'pointer' entries.

		if (token_list.front() == "reflection")
		{
			// Expect 3 tokens
			ExpectCount(token_list, 3);

			// Next
			token_list.clear();
			continue;
		}

		// Privateid
		// Format: privateid:<id>
		// Observed first in version 3:1:104 of metadata.
		// Not sure what this entry is for.
		if (token_list.front() == "privateid")
		{
			// Expect 2 tokens
			ExpectCount(token_list, 2);

			// Next
			token_list.clear();
			continue;
		}

		// Constarg
		// Format: constarg:<arg_id>:<arg_name>
		// Observed first in version 3:1:104 of metadata.
		// It shows up when an argument is declared as
		// '__global const'. Entry ignored here.
		if (token_list.front() == "constarg")
		{
			// Expect 3 tokens
			ExpectCount(token_list, 3);

			// Next
			token_list.clear();
			continue;
		}

		// Device
		// Device that the kernel was compiled for.
		if (token_list.front() == "device")
		{
			// Expect 2 tokens
			ExpectCount(token_list, 2);

			// Next
			token_list.clear();
			continue;
		}

		// Uniqueid
		// A mapping between a kernel and its unique ID
		if (token_list.front() == "uniqueid")
		{
			// Expect 2 tokens
			ExpectCount(token_list, 2);

			// Next
			token_list.clear();
			continue;
		}

		// Uavid
		// ID of a raw UAV
		if (token_list.front() == "uavid")
		{
			// Expect 2 tokens
			ExpectCount(token_list, 2);

			// Next
			token_list.clear();
			continue;
		}

		// Crash when uninterpreted entries appear
		throw Driver::Error(misc::fmt("Kernel %s: Unknown metadata "
				"entry: %s", this->name.c_str(),
				token.c_str()));
	}
}

void Kernel::LoadMetaData()
{
	// Load metadata content
	std::istringstream metadata_stream;
	metadata_symbol->getStream(metadata_stream);

	// First line example:
	// ;ARGSTART:__OpenCL_opencl_mmul_kernel
	std::string line;
	std::vector<std::string> token_list;
	std::getline(metadata_stream, line);

	misc::StringTokenize(line, token_list, ";:");
	Expect(token_list, "ARGSTART");
	ExpectCount(token_list, 2);
	token_list.clear();

	// Second line contains version info. Example:
	// ;version:3:1:104
	std::getline(metadata_stream, line);
	misc::StringTokenize(line, token_list, ";:");
	Expect(token_list, "version");
	ExpectCount(token_list, 4);
	int version = misc::StringToInt(token_list[1]);
	token_list.clear();

	// Parse rest of the metadata based on version number
	switch (version)
	{
	case 3:

		LoadMetaDataV3();
		break;

	default:

		throw Driver::Error(misc::fmt("Unsupported metadata version "
				"%d\n%s", version, OpenCLErrSIKernelMetadata));
	}

}

void Kernel::CreateBufferDescriptor(unsigned base_addr,
		unsigned size,
		int num_elems,
		Argument::DataType data_type,
		WorkItem::BufferDescriptor *buffer_descriptor)
{
	int num_format;
	int data_format;
	int elem_size;

	// Zero-out the buffer resource descriptor
	assert(sizeof(struct WorkItem::BufferDescriptor) == 16);
	memset(buffer_descriptor, 0, sizeof(WorkItem::BufferDescriptor));

	num_format = Emulator::BufDescNumFmtInvalid;
	data_format = Emulator::BufDescDataFmtInvalid;

	switch (data_type)
	{

	case Argument::DataTypeInt8:
	case Argument::DataTypeUInt8:

		num_format = Emulator::BufDescNumFmtSint;
		switch (num_elems)
		{
		case 1:
			data_format = Emulator::BufDescDataFmt8;
			break;

		case 2:
			data_format = Emulator::BufDescDataFmt8_8;
			break;

		case 4:
			data_format = Emulator::BufDescDataFmt8_8_8_8;
			break;

		default:

			throw Driver::Error(misc::fmt("Invalid number of "
					"i8/u8 elements (%d)", num_elems));
		}
		elem_size = 1 * num_elems;
		break;

	case Argument::DataTypeInt16:
	case Argument::DataTypeUInt16:

		num_format = Emulator::BufDescNumFmtSint;
		switch (num_elems)
		{

		case 1:
			data_format = Emulator::BufDescDataFmt16;
			break;

		case 2:
			data_format = Emulator::BufDescDataFmt16_16;
			break;

		case 4:
			data_format = Emulator::BufDescDataFmt16_16_16_16;
			break;

		default:

			throw Driver::Error(misc::fmt("Invalid number of "
					"i16/u16 elements (%d)", num_elems));
		}
		elem_size = 2 * num_elems;
		break;

	case Argument::DataTypeInt32:
	case Argument::DataTypeUInt32:

		num_format = Emulator::BufDescNumFmtSint;
		switch (num_elems)
		{

		case 1:
			data_format = Emulator::BufDescDataFmt32;
			break;

		case 2:
			data_format = Emulator::BufDescDataFmt32_32;
			break;

		case 3:
			data_format = Emulator::BufDescDataFmt32_32_32;
			break;

		case 4:
			data_format = Emulator::BufDescDataFmt32_32_32_32;
			break;

		default:

			throw Driver::Error(misc::fmt("Invalid number of "
					"i32/u32 elements (%d)", num_elems));
		}
		elem_size = 4 * num_elems;
		break;

	case Argument::DataTypeFloat:

		num_format = Emulator::BufDescNumFmtFloat;
		switch (num_elems)
		{
		case 1:
			data_format = Emulator::BufDescDataFmt32;
			break;

		case 2:
			data_format = Emulator::BufDescDataFmt32_32;
			break;

		case 3:
			data_format = Emulator::BufDescDataFmt32_32_32;
			break;

		case 4:
			data_format = Emulator::BufDescDataFmt32_32_32_32;
			break;

		default:

			throw Driver::Error(misc::fmt("Invalid number of "
					"float elements (%d)", num_elems));
		}
		elem_size = 4 * num_elems;
		break;

	case Argument::DataTypeDouble:

		num_format = Emulator::BufDescNumFmtFloat;
		switch (num_elems)
		{
		case 1:
			data_format = Emulator::BufDescDataFmt32_32;
			break;

		case 2:
			data_format = Emulator::BufDescDataFmt32_32_32_32;
			break;

		default:

			throw Driver::Error(misc::fmt("Invalid number of "
					"double elements (%d)", num_elems));
		}
		elem_size = 8 * num_elems;
		break;
	case Argument::DataTypeStruct:

		num_format = Emulator::BufDescNumFmtUint;
		data_format = Emulator::BufDescDataFmt8;
		elem_size = 1;
		break;

	default:

		throw Driver::Error(misc::fmt("Invalid data type for buffer "
				"(%d)", data_type));
	}
	assert(num_format != Emulator::BufDescNumFmtInvalid);
	assert(data_format != Emulator::BufDescDataFmtInvalid);

	buffer_descriptor->base_addr = base_addr;
	buffer_descriptor->num_format = num_format;
	buffer_descriptor->data_format = data_format;
	assert(!(size % elem_size));
	buffer_descriptor->elem_size = elem_size;
	buffer_descriptor->num_records = size;

	return;

}

// Public functions

Kernel::Kernel(int id, const std::string &name, Program *program) :
		id(id),
		name(name),
		program(program)
{
	metadata_symbol = program->getSymbol("__OpenCL_" + name + "_metadata");
	header_symbol = program->getSymbol("__OpenCL_" + name + "_header");
	kernel_symbol = program->getSymbol("__OpenCL_" + name + "_kernel");
	if (!metadata_symbol || !header_symbol || !kernel_symbol)
		throw Driver::Error(misc::fmt("Invalid kernel function\n"
				"\tELF symbol 'OpenCL_%s_xxx missing'\n%s",
				name.c_str(), OpenCLErrSIKernelSymbol));

	Driver::debug << misc::fmt("\tmetadata symbol: offset=0x%x, size=%u\n",
			(unsigned)metadata_symbol->getValue(), 
			(unsigned)metadata_symbol->getSize());
	Driver::debug << misc::fmt("\theader symbol: offset=0x%x, size=%u\n",
			(unsigned)header_symbol->getValue(), 
			(unsigned)header_symbol->getSize());
	Driver::debug << misc::fmt("\tkernel symbol: offset=0x%x, size=%u\n",
			(unsigned)kernel_symbol->getValue(), 
			(unsigned)kernel_symbol->getSize());

	// Create and parse kernel binary (internal ELF).
	// The internal ELF is contained in the buffer pointer to by
	// the 'kernel' symbol.
	std::string symbol_name = "kernel<" + name + ">.InternalELF";
	unsigned kernel_buf_size = (unsigned) kernel_symbol->getSize();
	
	// Get the area of the text section pointed to by the symbol
	std::istringstream kernel_symbol_stream;
	kernel_symbol->getStream(kernel_symbol_stream);

	// Copy the symbol data into a buffer
	auto kernel_buffer = misc::new_unique_array<char>(kernel_buf_size);
	kernel_symbol_stream.read(kernel_buffer.get(), kernel_buf_size);

	// Create a new ELF based on the kernel buffer
	binary_file = misc::new_unique<Binary>(kernel_buffer.get(), 
			kernel_buf_size, symbol_name);

	// Load metadata
	LoadMetaData();
}

void Kernel::CreateNDRangeTables(NDRange *ndrange /* MMU *gpu_mmu */)
{
	// Get emulator instance and video_memory               
	Emulator *emulator = SI::Emulator::getInstance();
	mem::Memory *video_memory = emulator->getVideoMemory();

	unsigned size_of_tables = NDRange::ConstBufTableSize +
			NDRange::ResourceTableSize + NDRange::UAVTableSize;

	// if (gpu_mmu)
	// {
	// 	/* Allocate starting from nearest page boundary */
	// 	if (emu->video_mem_top & gpu_mmu->page_mask)
	// 	{
	// 		emu->video_mem_top += gpu_mmu->page_size -
	// 			(emu->video_mem_top & gpu_mmu->page_mask);
	// 	}
	// }

	// Allocate space in video memory for tables                             
	video_memory->Map(emulator->getVideoMemoryTop(), size_of_tables,              
			mem::Memory::AccessRead | mem::Memory::AccessWrite);             

	// Debug print out
	Driver::debug << misc::fmt("\t%u bytes of device memory allocated at "
		"0x%x for SI internal tables\n", size_of_tables,
		emulator->getVideoMemoryTop());

	// Set constant buffer table address                                     
	ndrange->setConstBufferTable(emulator->getVideoMemoryTop());                  
	emulator->incVideoMemoryTop(NDRange::ConstBufTableSize);                       

	// Set resource table address                                            
	ndrange->setResourceTable(emulator->getVideoMemoryTop());                     
	emulator->incVideoMemoryTop(NDRange::ResourceTableSize);                       

	// Set uav table address                                                 
	ndrange->setUAVTable(emulator->getVideoMemoryTop());                          
	emulator->incVideoMemoryTop(NDRange::UAVTableSize); 
	
	// Return
	return;
}

void Kernel::CreateNDRangeConstantBuffers(NDRange *ndrange)                                            
{                                                                                
	// Get emulator instance and video_memory                                
	SI::Emulator *emulator = SI::Emulator::getInstance();                                
	mem::Memory *video_memory = emulator->getVideoMemory();                       

	// Map new pages                                                         
	video_memory->Map(emulator->getVideoMemoryTop(), 
			NDRange::TotalConstBufSize,    
			mem::Memory::AccessRead | mem::Memory::AccessWrite);     

	// TODO - setup for timing simulator                                     

	// Set constant buffer addresses                                         
	ndrange->setCB0(emulator->getVideoMemoryTop());                                 
	ndrange->setCB1(emulator->getVideoMemoryTop() + NDRange::ConstBuf0Size);            

	// Increment video memory pointer                                        
	emulator->incVideoMemoryTop(NDRange::TotalConstBufSize);                       
}  


void Kernel::SetupNDRangeArgs(NDRange *ndrange /* MMU *gpu_mmu */)
{
	WorkItem::BufferDescriptor buffer_descriptor;
	int zero = 0;

	// Initial top of local memory is determined by the static local memory
	// specified in the kernel binary. Number of vector and scalar
	// registers used by the kernel recorded as well.
	const BinaryDictEntry *enc_dict = getKernelBinaryFile()->GetSIDictEntry();
	ndrange->setLocalMemTop(getLocalMemorySize());
	ndrange->setNumSgprUsed(enc_dict->num_sgpr);
	ndrange->setNumVgprUsed(enc_dict->num_vgpr);

	// Kernel arguments
	int index = 0;
	for (auto &arg : getArgs())
	{
		// Check that argument was set
		assert(arg);
		if (!(arg->set))
			throw Driver::Error(misc::fmt("Kernel '%s': "
					"Argument '%s' is not set",
					getName().c_str(),
					arg->name.c_str()));

		// Debug
		Driver::debug << misc::fmt("\targ[%d] = %s ",
				index, arg->name.c_str());

		// Process argument depending on its type
		switch (arg->getType())
		{

		case Argument::TypeValue:
		{
			ValueArgument *arg_value = 
					dynamic_cast<ValueArgument *>(
					arg.get());

			// Value copied directly into device constant memory
			assert(arg_value->size);
			ndrange->ConstantBufferWrite(
				arg_value->getConstantBufferNum(),
				arg_value->getConstantOffset(),
				arg_value->getValue(), arg_value->size);
			break;
		}

		case Argument::TypePointer:
		{
			PointerArgument *arg_ptr = 
					dynamic_cast<PointerArgument *>(
					arg.get());

			switch (arg_ptr->getScope())
			{

			// Hardware local memory
			case Argument::ScopeHwLocal:

				// Pointer in __local scope.
				// Argument value is always NULL, just assign
				// space for it.
				ndrange->ConstantBufferWrite(
					arg_ptr->getConstantBufferNum(),
					arg_ptr->getConstantOffset(),
					ndrange->getLocalMemTopPtr(), 4);

				Driver::debug << misc::fmt("%u bytes at 0x%x", 
					arg_ptr->size, 
					ndrange->getLocalMemTop());

				ndrange->incLocalMemTop(arg_ptr->size);

				break;

			// UAV
			case Argument::ScopeUAV:
			{
				Driver::debug << misc::fmt("(0x%x)", 
						arg_ptr->getDevicePtr());

				// Create descriptor for argument
				CreateBufferDescriptor(
					arg_ptr->getDevicePtr(),
					arg_ptr->size,
					arg_ptr->getNumElems(),
					arg_ptr->getDataType(), &buffer_descriptor);

				// Add to UAV table
				ndrange->InsertBufferIntoUAVTable(
					&buffer_descriptor,
					arg_ptr->getBufferNum());

				// Write 0 to CB1
				ndrange->ConstantBufferWrite(
					arg_ptr->getConstantBufferNum(),
					arg_ptr->getConstantOffset(),
					&zero, 4);

				break;
			}

			// Hardware constant memory
			case Argument::ScopeHwConstant:
			{
				CreateBufferDescriptor(
					arg_ptr->getDevicePtr(),
					arg_ptr->size,
					arg_ptr->getNumElems(),
					arg_ptr->getDataType(), 
					&buffer_descriptor);

				// Data stored in hw constant memory
				// uses a 4-byte stride
				buffer_descriptor.stride = 4;

				// Add to Constant Buffer table
				ndrange->InsertBufferIntoConstantBufferTable(
					&buffer_descriptor,
					arg_ptr->getBufferNum());

				// Write 0 to CB1
				ndrange->ConstantBufferWrite(
					arg_ptr->getConstantBufferNum(),
					arg_ptr->getConstantOffset(),
					&zero, 4);

				break;
			}

			default:

				throw Driver::Error("Invalid memory scope");
			}

			break;
		}

		case Argument::TypeImage:

			throw misc::Panic("Type 'image' not implemented");

		case Argument::TypeSampler:

			throw misc::Panic("Type 'sampler' not implemented");

		default:

			throw Driver::Error(misc::fmt("Invalid argument type "
					"(%d)", arg->getType()));

		}

		// Debug
		Driver::debug << "\n";

		// Next
		index++;
	}

	// Add program-wide constant buffers to the ND-range.
	// Program-wide constant buffers start at number 2.
	for (unsigned i = 2; i < NDRange::MaxNumConstBufs; i++)
	{
		// Assert that program is valid
		Program *program = getProgram();
		assert(program);

		// Retrieve constant buffer
		ConstantBuffer *constant_buffer = 
				program->getConstantBufferByIndex(i);
		
		// Check if cosntant buffer exists
		if (!constant_buffer)
			break;

		// Create a buffer descriptor for the constant buffer
		WorkItem::BufferDescriptor buffer_descriptor;
		CreateBufferDescriptor(
		 		constant_buffer->device_address,
		 		constant_buffer->getSize(),
		 		4,
		 		Argument::DataTypeFloat,
		 		&buffer_descriptor);

		// Data stored in hw constant memory uses a 16-byte stride
		// XXX Use or don't use?
		buffer_descriptor.stride = 16; 

		// Add to Constant Buffer table
		ndrange->InsertBufferIntoConstantBufferTable(
				&buffer_descriptor, i);
	}
}


void Kernel::DebugNDRangeState(NDRange *ndrange)
{

	// Get emulator instance and video memory
	SI::Emulator *emulator = SI::Emulator::getInstance();
	mem::Memory *video_memory = emulator->getVideoMemory();
	
	// Create a buffer descriptor
	WorkItem::BufferDescriptor buffer_desc;

	// Start writing NDRange debug output
	Emulator::isa_debug << misc::fmt("\n");
        Emulator::isa_debug << misc::fmt("================ Initialization Summary ================\n");
        Emulator::isa_debug << misc::fmt("\n");

        // Table locations
        Emulator::isa_debug << misc::fmt("NDRange table locations:\n");
        Emulator::isa_debug << misc::fmt("\t------------------------------------------------\n");
        Emulator::isa_debug << misc::fmt("\t|    Name            |    Address Range        |\n");
        Emulator::isa_debug << misc::fmt("\t------------------------------------------------\n");
        Emulator::isa_debug << misc::fmt("\t| Const Buffer table | [%10u:%10u] |\n",
               		ndrange->getConstBufferTableAddr(),
                	ndrange->getConstBufferTableAddr() +
                	NDRange::ConstBufTableSize - 1);
        Emulator::isa_debug << misc::fmt("\t| Resource table     | [%10u:%10u] |\n",
                	ndrange->getResourceTableAddr(),
                	ndrange->getResourceTableAddr() +
                	NDRange::ResourceTableSize - 1);
        Emulator::isa_debug << misc::fmt("\t| UAV table          | [%10u:%10u] |\n",
               		ndrange->getUAVTableAddr(),
                	ndrange->getUAVTableAddr() + NDRange::UAVTableSize - 1);
        Emulator::isa_debug << misc::fmt("\t------------------------------------------------\n");
        Emulator::isa_debug << misc::fmt("\n");

        // SREG initialization
        unsigned user_element_count = 
			binary_file.get()->GetSIDictEntry()->num_user_elements;
	BinaryUserElement *user_elements =
			binary_file.get()->GetSIDictEntry()->user_elements;
	Emulator::isa_debug << misc::fmt("Scalar register initialization prior to execution:\n");
        Emulator::isa_debug << misc::fmt("\t-------------------------------------------\n");
        Emulator::isa_debug << misc::fmt("\t|  Registers  |   Initialization Value    |\n");
        Emulator::isa_debug << misc::fmt("\t-------------------------------------------\n");
        
	// Iterate through the user elements
	for (unsigned i = 0; i < user_element_count; i++)
        {
                if (user_elements[i].dataClass == BinaryUserDataConstBuffer)
                {
                        // Add constant buffer info to debug output
                        if (user_elements[i].userRegCount > 1)
                        {
                                Emulator::isa_debug << misc::fmt("\t| SREG[%2d:%2d] |  CB%1d "
                                        	"Descriptor           |\n",
                                        	user_elements[i].startUserReg,
                                       		user_elements[i].startUserReg +
                                        	user_elements[i].userRegCount - 1,
                                        	user_elements[i].apiSlot);
                        }
                        else
                        {
                                Emulator::isa_debug << misc::fmt("\t| SREG[%2d]    |  CB%1d "
                                        	"Descriptor         |\n",
                                        	user_elements[i].startUserReg,
                                        	user_elements[i].apiSlot);
                        }
                }

                else if (user_elements[i].dataClass == BinaryUserDataUAV)
                {
                        // Add UAV info to debug output
                        Emulator::isa_debug << misc::fmt("\t| SREG[%2d:%2d] |  UAV%-2d "
                              		"Descriptor         |\n",
                                	user_elements[i].startUserReg,
                                	user_elements[i].startUserReg +
                                	user_elements[i].userRegCount - 1,
                                	user_elements[i].apiSlot);
                }
                else if (user_elements[i].dataClass ==  
			BinaryUserDataConstBufferTable)
                {
			// Add constant buffer table to debug output
                        Emulator::isa_debug << misc::fmt("\t| SREG[%2d:%2d] |  Constant Buffer "
                                	"Table    |\n",
                                	user_elements[i].startUserReg,
                                	user_elements[i].startUserReg +
                                	user_elements[i].userRegCount - 1);
                }
                else if (user_elements[i].dataClass == BinaryUserDataUAVTable)
                {
			// Add UAV table to debug output
                        Emulator::isa_debug << misc::fmt("\t| SREG[%2d:%2d] |  UAV "
                                	"Table                |\n",
                                	user_elements[i].startUserReg,
                               		user_elements[i].startUserReg +
                                	user_elements[i].userRegCount - 1);
                }
                else
                {
                        assert(0);
                }
        }
        Emulator::isa_debug << misc::fmt("\t-------------------------------------------\n");
        Emulator::isa_debug << misc::fmt("\n");

        // Initialized constant buffers
	Emulator::isa_debug << misc::fmt("Initialized constant buffers:\n");
	Emulator::isa_debug << misc::fmt("\t-----------------------------------\n");
	Emulator::isa_debug << misc::fmt("\t|  CB   |      Address Range      |\n");
	Emulator::isa_debug << misc::fmt("\t-----------------------------------\n");

	// Iterate through the constant buffers
        for (int i = 0; i < NDRange::MaxNumConstBufs; i++)
	{
		// Make sure the constant buffer is valid
		if (!ndrange->getConstBuffer(i)->valid)
                	continue;
	
		// Read a buffer description of the constant buffer
		video_memory->Read(
				ndrange->getConstBufferTableAddr() + 
				i * NDRange::ConstBufTableEntrySize, 
				sizeof(buffer_desc), (char *) &buffer_desc);

		// Add constant buffer information to debug output
        	Emulator::isa_debug << misc::fmt("\t| CB%-2d  | [%10llu:%10llu] |\n",
				i, (long long unsigned int) buffer_desc.base_addr,
				(long long unsigned int) buffer_desc.base_addr + 
				(long long unsigned int) buffer_desc.num_records - 1);
	}
	Emulator::isa_debug << misc::fmt("\t-----------------------------------\n");
        Emulator::isa_debug << misc::fmt("\n");

        // Initialized UAVs
	Emulator::isa_debug << misc::fmt("Initialized UAVs:\n");
	Emulator::isa_debug << misc::fmt("\t-----------------------------------\n");
	Emulator::isa_debug << misc::fmt("\t|  UAV  |      Address Range      |\n");
	Emulator::isa_debug << misc::fmt("\t-----------------------------------\n");
        
	// Iterate through the UAVs
	for (int i = 0; i < NDRange::MaxNumUAVs; i++)
	{
		// Check that the UAV is valid
		if (!ndrange->getUAV(i)->valid)
                	continue;

		// Read a buffer description of the UAV
		video_memory->Read(
				ndrange->getUAVTableAddr() + 
				i * NDRange::UAVTableEntrySize, 
				sizeof(buffer_desc), 
				(char *) &buffer_desc);

		// Add UAV information to debug output
        	Emulator::isa_debug << misc::fmt("\t| UAV%-2d | [%10u:%10u] |\n",
				i, (unsigned int) buffer_desc.base_addr,
				(unsigned int) buffer_desc.base_addr + 
				(unsigned int) buffer_desc.num_records - 1);
	}

	// Finish debug output
	Emulator::isa_debug << misc::fmt("\t-----------------------------------\n");
        Emulator::isa_debug << misc::fmt("\n");
        Emulator::isa_debug << misc::fmt("========================================================\n");
}


void Kernel::SetupNDRangeConstantBuffers(NDRange *ndrange)
{
	WorkItem::BufferDescriptor buffer_descriptor;
	unsigned zero = 0;
	float f;

	// Constant buffer 0
	CreateBufferDescriptor(ndrange->getConstBufferAddr(0),
			NDRange::ConstBuf0Size,
			1, Argument::DataTypeInt32, &buffer_descriptor);

	ndrange->InsertBufferIntoConstantBufferTable(&buffer_descriptor, 0);

	// Constant buffer 1
	CreateBufferDescriptor(ndrange->getConstBufferAddr(1),
			NDRange::ConstBuf1Size,
			1, Argument::DataTypeInt32, &buffer_descriptor);

	ndrange->InsertBufferIntoConstantBufferTable(&buffer_descriptor, 1);

	// Initialize constant buffer 0

	// CB0 bytes 0:15

	// Global work size for the {x,y,z} dimensions
	ndrange->ConstantBufferWrite(0, 0,
		ndrange->getGlobalSizePtr(0), 4);
	ndrange->ConstantBufferWrite(0, 4,
		ndrange->getGlobalSizePtr(1), 4);
	ndrange->ConstantBufferWrite(0, 8,
		ndrange->getGlobalSizePtr(2), 4);

	// Number of work dimensions
	ndrange->ConstantBufferWrite(0, 12, ndrange->getWorkDimPtr(), 4);

	// CB0 bytes 16:31

	// Local work size for the {x,y,z} dimensions
	ndrange->ConstantBufferWrite(0, 16,
		ndrange->getLocalSizePtr(0), 4);
	ndrange->ConstantBufferWrite(0, 20,
		ndrange->getLocalSizePtr(1), 4);
	ndrange->ConstantBufferWrite(0, 24,
		ndrange->getLocalSizePtr(2), 4);

	// 0 `
	ndrange->ConstantBufferWrite(0, 28, &zero, 4);

	// CB0 bytes 32:47

	// Global work size {x,y,z} / local work size {x,y,z}
	ndrange->ConstantBufferWrite(0, 32,
		ndrange->getGroupCountPtr(0), 4);
	ndrange->ConstantBufferWrite(0, 36,
		ndrange->getGroupCountPtr(1), 4);
	ndrange->ConstantBufferWrite(0, 40,
		ndrange->getGroupCountPtr(2), 4);

	// 0 
	ndrange->ConstantBufferWrite(0, 44, &zero, 4);

	// CB0 bytes 48:63

	// FIXME Offset to private memory ring (0 if private memory is
	// not emulated)

	// FIXME Private memory allocated per work_item

	// 0 
	ndrange->ConstantBufferWrite(0, 56, &zero, 4);

	// 0 
	ndrange->ConstantBufferWrite(0, 60, &zero, 4);

	// CB0 bytes 64:79

	// FIXME Offset to local memory ring (0 if local memory is
	// not emulated)

	// FIXME Local memory allocated per group

	// 0
	ndrange->ConstantBufferWrite(0, 72, &zero, 4);

	// FIXME Pointer to location in global buffer where math library
	// tables start.

	// CB0 bytes 80:95

	// 0.0 as IEEE-32bit float - required for math library.
	f = 0.0f;
	ndrange->ConstantBufferWrite(0, 80, &f, 4);

	// 0.5 as IEEE-32bit float - required for math library.
	f = 0.5f;
	ndrange->ConstantBufferWrite(0, 84, &f, 4);

	// 1.0 as IEEE-32bit float - required for math library.
	f = 1.0f;
	ndrange->ConstantBufferWrite(0, 88, &f, 4);

	// 2.0 as IEEE-32bit float - required for math library.
	f = 2.0f;
	ndrange->ConstantBufferWrite(0, 92, &f, 4);

	// CB0 bytes 96:111

	// Global offset for the {x,y,z} dimension of the work_item spawn
	ndrange->ConstantBufferWrite(0, 96, &zero, 4);
	ndrange->ConstantBufferWrite(0, 100, &zero, 4);
	ndrange->ConstantBufferWrite(0, 104, &zero, 4);

	// Global single dimension flat offset: x * y * z
	ndrange->ConstantBufferWrite(0, 108, &zero, 4);

	// CB0 bytes 112:127

	// Group offset for the {x,y,z} dimensions of the work_item spawn
	ndrange->ConstantBufferWrite(0, 112, &zero, 4);
	ndrange->ConstantBufferWrite(0, 116, &zero, 4);
	ndrange->ConstantBufferWrite(0, 120, &zero, 4);

	// Group single dimension flat offset, x * y * z
	ndrange->ConstantBufferWrite(0, 124, &zero, 4);

	// CB0 bytes 128:143

	// FIXME Offset in the global buffer where data segment exists
	// FIXME Offset in buffer for printf support
	// FIXME Size of the printf buffer

}

}  // namespace SI
