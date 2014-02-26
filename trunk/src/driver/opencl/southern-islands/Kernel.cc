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


#include <arch/x86/emu/Emu.h>
#include <arch/southern-islands/emu/Emu.h>
#include <arch/southern-islands/emu/NDRange.h>
#include <lib/cpp/Misc.h>
#include <string.h>

#include "Kernel.h"
#include "Program.h"


namespace SI
{

static const char *OpenCLErrSIKernelSymbol =
	"\tThe ELF file analyzer is trying to find a name in the ELF symbol table."
	"\tIf it is not found, it probably means that your application is requesting"
	"\texecution of a kernel function that is not present in the encoded binary."
	"\tPlease, check the parameters passed to the 'clCreateKernel' function in"
	"\tyour application.\n"
	"\tThis could be also a symptom of compiling an OpenCL kernel source on a"
	"\tmachine with an installation of the AMD SDK (using 'm2s-clcc') but"
	"\twith an incorrect or missing installation of the GPU driver. In this case,"
	"\tthe tool will still compile the kernel into LLVM, but the ISA section will"
	"\tbe missing in the kernel binary.";

static const char *OpenCLErrSIKernelMetadata =
	"\tThe kernel binary loaded by your application is a valid ELF file. In this"
	"\tfile, a '.rodata' section contains specific information about the OpenCL"
	"\tkernel. However, this information is only partially supported by Multi2Sim."
	"\tTo request support for this error, please email 'development@multi2sim.org'.";

// Private functions

void Kernel::Expect(std::vector<std::string> &token_list, std::string head_token)
{
	std::string token = token_list.at(0);

	if (token == head_token)
		misc::fatal("%s: token '%s' expected, '%s' found.\n%s",
				__FUNCTION__, head_token.c_str(), token_list[0].c_str(),
				OpenCLErrSIKernelMetadata);
}

void Kernel::ExpectInt(std::vector<std::string> &token_list)
{
	std::string token = token_list.at(0);
	misc::StringError err;

	StringToInt(token, err);
	if (err)
	{
		misc::fatal("%s: integer number expected, '%s' found.\n%s",
				__FUNCTION__, token.c_str(),
				OpenCLErrSIKernelMetadata);		
	}
}

void Kernel::ExpectCount(std::vector<std::string> &token_list, unsigned count)
{
	std::string head_token = token_list.at(0);

	if (token_list.size() != count)
	{
		misc::fatal("%s: %d tokens expected for '%s', %d found.\n%s",
				__FUNCTION__, count, head_token.c_str(), (unsigned)token_list.size(),
				OpenCLErrSIKernelMetadata);
	}
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
 
	for (;;)
	{
		//  Read the next line
		std::getline(metadata_stream, line);
		x86::Emu::opencl_debug << misc::fmt("\t%s\n", line.c_str());
		misc::StringTokenize(line, token_list, ";:");

		//  Stop when ARGEND is found or line is empty
		if (!token_list.size() || token_list.front() != "ARGEND")
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
			int data_type_int = misc::StringToInt(*token_list.begin());
			ArgDataType data_type = static_cast<ArgDataType>(data_type_int);
			const char *data_type_string = arg_data_type_map.MapValue(data_type, err);
			if (err)
				misc::fatal("%s: invalid data type '%s'.\n%s",
					__FUNCTION__, data_type_string, 
					OpenCLErrSIKernelMetadata);

			// Token 3 - Number of elements 
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int num_elems = misc::StringToInt(*token_list.begin());
			assert(num_elems > 0);

			// Token 4 - Constant buffer 
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			Expect(token_list, "1");
			int constant_buffer_num = misc::StringToInt(*token_list.begin());

			// Token 5 - Conastant offset 
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int constant_offset = misc::StringToInt(*token_list.begin());

			// Create argument object
			std::unique_ptr<SI::Arg> arg(new SI::ArgValue(name, data_type, num_elems, 
				constant_buffer_num, constant_offset));

			// Debug 
			x86::Emu::opencl_debug << misc::fmt("\targument '%s' - value stored in "
				"constant buffer %d at offset %d\n",
				name.c_str(), constant_buffer_num,
				constant_offset);

			// Add argument and clear token list
			args.push_back(std::move(arg));
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
			int data_type_int = misc::StringToInt(*token_list.begin());
			ArgDataType data_type = static_cast<ArgDataType>(data_type_int);
			const char *data_type_string = arg_data_type_map.MapValue(data_type, err);
			if (err)
				misc::fatal("%s: invalid data type '%s'.\n%s",
					__FUNCTION__, data_type_string, 
					OpenCLErrSIKernelMetadata);

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
			int constant_buffer_num = misc::StringToInt(*token_list.begin());

			// Token 5 - Conastant offset 
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int constant_offset = misc::StringToInt(*token_list.begin());

			// Token 6 - Memory scope 
			token_list.erase(token_list.begin());
			int arg_scope_int = misc::StringToInt(*token_list.begin());
			ArgScope arg_scope = static_cast<ArgScope>(arg_scope_int);
			const char *arg_scope_string = arg_scope_map.MapValue(arg_scope, err);
			if (err)
				misc::fatal("%s: invalid scope '%s'.\n%s",
					__FUNCTION__, arg_scope_string, 
					OpenCLErrSIKernelMetadata);

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
			int access_type_int = misc::StringToInt(*token_list.begin());
			ArgAccessType access_type = static_cast<ArgAccessType>(access_type_int);
			if (err)
				misc::fatal("%s: invalid access type '%s'.\n%s",
					__FUNCTION__, token.c_str(), 
					OpenCLErrSIKernelMetadata);

			// Token 10 - ??? 
			token_list.erase(token_list.begin());
			Expect(token_list, "0");

			// Token 11 - ??? 
			token_list.erase(token_list.begin());
			Expect(token_list, "0");

			// Create argument object
			std::unique_ptr<SI::Arg> arg(new SI::ArgPointer(name, data_type, num_elems, 
				constant_buffer_num, constant_offset, arg_scope, buffer_num, alignment, 
				access_type));

			// Debug 
			x86::Emu::opencl_debug << misc::fmt("\targument '%s' - Pointer stored in "
				"constant buffer %d at offset %d\n",
				name.c_str(), constant_buffer_num,
				constant_offset);

			// Add argument and clear token list
			args.push_back(std::move(arg));
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
			const char*dimension_string = arg_dimension_map.MapValue(dimension, err);
			if (err)
				misc::fatal("%s: invalid image dimensions '%s'.\n%s",
					__FUNCTION__, dimension_string, OpenCLErrSIKernelMetadata);

			// Token 3 - Access type 
			token_list.erase(token_list.begin());
			int access_type_int = misc::StringToInt(*token_list.begin());
			ArgAccessType access_type = static_cast<ArgAccessType>(access_type_int);
			if (err)
				misc::fatal("%s: invalid access type '%s'.\n%s",
					__FUNCTION__, token.c_str(), 
					OpenCLErrSIKernelMetadata);

			// Token 4 - UAV 
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int uav = misc::StringToInt(*token_list.begin());

			// Token 5 - Constant buffer 
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			Expect(token_list, "1");
			int constant_buffer_num = misc::StringToInt(*token_list.begin());

			// Token 6 - Conastant offset 
			token_list.erase(token_list.begin());
			ExpectInt(token_list);
			int constant_offset = misc::StringToInt(*token_list.begin());

			// Create argument object
			std::unique_ptr<SI::Arg> arg(new SI::ArgImage(name, dimension, access_type,
				uav, constant_buffer_num, constant_offset));

			// Debug 
			x86::Emu::opencl_debug << misc::fmt("\targument '%s' - Image stored in "
				"constant buffer %d at offset %d\n",
				name.c_str(), constant_buffer_num,
				constant_offset);

			// Add argument and clear token list
			args.push_back(std::move(arg));
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
			std::unique_ptr<SI::Arg> arg(new SI::ArgSampler(name, id, location, 
				value));

			// Add argument and clear token list
			args.push_back(std::move(arg));
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
				this->mem_size_local = misc::StringToInt(*token_list.begin());
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
				misc::fatal("%s: not supported metadata '%s'.\n%s",
						__FUNCTION__, token.c_str(), OpenCLErrSIKernelMetadata);
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
			this->func_uniqueid = misc::StringToInt(*token_list.begin());

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
		misc::fatal("kernel '%s': unknown metadata entry '%s'",
			this->name.c_str(), token.c_str());
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
		misc::fatal("%s: metadata version %d not supported.\n%s",
				__FUNCTION__, version, OpenCLErrSIKernelMetadata);
	}

}

void Kernel::CreateBufferDesc(unsigned base_addr, unsigned size, int num_elems, 
	ArgDataType data_type,
	EmuBufferDesc *buffer_desc)
{
	int num_format;
	int data_format;
	int elem_size;

	// Zero-out the buffer resource descriptor 
	assert(sizeof(struct EmuBufferDesc) == 16);
	memset(buffer_desc, 0, sizeof(EmuBufferDesc));

	num_format = EmuBufDescNumFmtInvalid;
	data_format = EmuBufDescDataFmtInvalid;

	switch (data_type)
	{

	case ArgDataTypeInt8:
	case ArgDataTypeUInt8:

		num_format = EmuBufDescNumFmtSint;
		switch (num_elems)
		{
		case 1:
			data_format = EmuBufDescDataFmt8;
			break;

		case 2:
			data_format = EmuBufDescDataFmt8_8;
			break;

		case 4:
			data_format = EmuBufDescDataFmt8_8_8_8;
			break;

		default:
			misc::fatal("%s: invalid number of i8/u8 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 1 * num_elems;
		break;

	case ArgDataTypeInt16:
	case ArgDataTypeUInt16:

		num_format = EmuBufDescNumFmtSint;
		switch (num_elems)
		{

		case 1:
			data_format = EmuBufDescDataFmt16;
			break;

		case 2:
			data_format = EmuBufDescDataFmt16_16;
			break;

		case 4:
			data_format = EmuBufDescDataFmt16_16_16_16;
			break;

		default:
			misc::fatal("%s: invalid number of i16/u16 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 2 * num_elems;
		break;

	case ArgDataTypeInt32:
	case ArgDataTypeUInt32:

		num_format = EmuBufDescNumFmtSint;
		switch (num_elems)
		{

		case 1:
			data_format = EmuBufDescDataFmt32;
			break;

		case 2:
			data_format = EmuBufDescDataFmt32_32;
			break;

		case 3:
			data_format = EmuBufDescDataFmt32_32_32;
			break;

		case 4:
			data_format = EmuBufDescDataFmt32_32_32_32;
			break;

		default:
			misc::fatal("%s: invalid number of i32/u32 elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 4 * num_elems;
		break;

	case ArgDataTypeFloat:

		num_format = EmuBufDescNumFmtFloat;
		switch (num_elems)
		{
		case 1:
			data_format = EmuBufDescDataFmt32;
			break;

		case 2:
			data_format = EmuBufDescDataFmt32_32;
			break;

		case 3:
			data_format = EmuBufDescDataFmt32_32_32;
			break;

		case 4:
			data_format = EmuBufDescDataFmt32_32_32_32;
			break;

		default:
			misc::fatal("%s: invalid number of float elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 4 * num_elems;
		break;

	case ArgDataTypeDouble:

		num_format = EmuBufDescNumFmtFloat;
		switch (num_elems)
		{
		case 1:
			data_format = EmuBufDescDataFmt32_32;
			break;

		case 2:
			data_format = EmuBufDescDataFmt32_32_32_32;
			break;

		default:
			misc::fatal("%s: invalid number of double elements (%d)",
					__FUNCTION__, num_elems);
		}
		elem_size = 8 * num_elems;
		break;
	case ArgDataTypeStruct:

		num_format = EmuBufDescNumFmtUint;
		data_format = EmuBufDescDataFmt8;
		elem_size = 1;
		break;

	default:
		misc::fatal("%s: invalid data type for SI buffer (%d)",
			__FUNCTION__, data_type);
	}
	assert(num_format != EmuBufDescNumFmtInvalid);
	assert(data_format != EmuBufDescDataFmtInvalid);

	buffer_desc->base_addr = base_addr;
	buffer_desc->num_format = num_format;
	buffer_desc->data_format = data_format;
	assert(!(size % elem_size));  
	buffer_desc->elem_size = elem_size;
	buffer_desc->num_records = size/elem_size;

	return;

}

// Public functions

Kernel::Kernel(int id, std::string name, Program *program)
{
	this->id = id;
	this->name = name;
	this->program = program;

	metadata_symbol = program->getSymbol("__OpenCL_" + name + "_metadata");
	header_symbol = program->getSymbol("__OpenCL_" + name + "_header");
	kernel_symbol = program->getSymbol("__OpenCL_" + name + "_kernel");
	if (!metadata_symbol || !header_symbol || !kernel_symbol)
		misc::fatal("%s: invalid kernel function (ELF symbol '__OpenCL_%s_xxx missing')\n%s",
				__FUNCTION__, name.c_str(), OpenCLErrSIKernelSymbol);
	
	x86::Emu::opencl_debug << misc::fmt("\tmetadata symbol: offset=0x%x, size=%u\n",
			(unsigned)metadata_symbol->getValue(), (unsigned)metadata_symbol->getSize());
	x86::Emu::opencl_debug << misc::fmt("\theader symbol: offset=0x%x, size=%u\n",
			(unsigned)header_symbol->getValue(), (unsigned)header_symbol->getSize());
	x86::Emu::opencl_debug << misc::fmt("\tkernel symbol: offset=0x%x, size=%u\n",
			(unsigned)kernel_symbol->getValue(), (unsigned)kernel_symbol->getSize());

	// Create and parse kernel binary (internal ELF).
	// The internal ELF is contained in the buffer pointer to by
	// the 'kernel' symbol.
	std::string symbol_name = "kernel<" + name + ">.InternalELF";
	const char *kernel_buf = kernel_symbol->getBuffer();
	unsigned kernel_buf_size = (unsigned)kernel_symbol->getSize();
	bin_file = std::unique_ptr<SI::Binary>(new Binary(kernel_buf, kernel_buf_size, symbol_name));
	
	// Load metadata
	LoadMetaData();
}

void Kernel::SetupNDRangeConstantBuffers(NDRange *ndrange)
{
	EmuBufferDesc buffer_desc;

	unsigned zero = 0;

	float f;

	/* Constant buffer 0 */
	CreateBufferDesc(ndrange->getConstBufferAddr(0), EmuConstBuf0Size, 
		1, ArgDataTypeInt32, &buffer_desc);

	ndrange->InsertBufferIntoConstantBufferTable(&buffer_desc, 0);

	/* Constant buffer 1 */
	CreateBufferDesc(ndrange->getConstBufferAddr(1), EmuConstBuf1Size,
		1, ArgDataTypeInt32, &buffer_desc);

	ndrange->InsertBufferIntoConstantBufferTable(&buffer_desc, 1);

	/* Initialize constant buffer 0 */

	/* CB0 bytes 0:15 */

	/* Global work size for the {x,y,z} dimensions */
	ndrange->ConstantBufferWrite(0, 0, 
		ndrange->getGlobalSizePtr(0), 4);
	ndrange->ConstantBufferWrite(0, 4, 
		ndrange->getGlobalSizePtr(1), 4);
	ndrange->ConstantBufferWrite(0, 8, 
		ndrange->getGlobalSizePtr(2), 4);

	/* Number of work dimensions */
	ndrange->ConstantBufferWrite(0, 12, ndrange->getWorkDimPtr(), 4);

	/* CB0 bytes 16:31 */

	/* Local work size for the {x,y,z} dimensions */
	ndrange->ConstantBufferWrite(0, 16, 
		ndrange->getLocalSizePtr(0), 4);
	ndrange->ConstantBufferWrite(0, 20, 
		ndrange->getLocalSizePtr(1), 4);
	ndrange->ConstantBufferWrite(0, 24, 
		ndrange->getLocalSizePtr(2), 4);

	/* 0  */
	ndrange->ConstantBufferWrite(0, 28, &zero, 4);

	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	ndrange->ConstantBufferWrite(0, 32, 
		ndrange->getGroupCountPtr(0), 4);
	ndrange->ConstantBufferWrite(0, 36, 
		ndrange->getGroupCountPtr(1), 4);
	ndrange->ConstantBufferWrite(0, 40, 
		ndrange->getGroupCountPtr(2), 4);

	/* 0  */
	ndrange->ConstantBufferWrite(0, 44, &zero, 4);

	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is
	 * not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	ndrange->ConstantBufferWrite(0, 56, &zero, 4);

	/* 0  */
	ndrange->ConstantBufferWrite(0, 60, &zero, 4);

	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is
	 * not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	ndrange->ConstantBufferWrite(0, 72, &zero, 4);

	/* FIXME Pointer to location in global buffer where math library
	 * tables start. */

	/* CB0 bytes 80:95 */

	/* 0.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	ndrange->ConstantBufferWrite(0, 80, &f, 4);

	/* 0.5 as IEEE-32bit float - required for math library. */
	f = 0.5f;
	ndrange->ConstantBufferWrite(0, 84, &f, 4);

	/* 1.0 as IEEE-32bit float - required for math library. */
	f = 1.0f;
	ndrange->ConstantBufferWrite(0, 88, &f, 4);

	/* 2.0 as IEEE-32bit float - required for math library. */
	f = 2.0f;
	ndrange->ConstantBufferWrite(0, 92, &f, 4);

	/* CB0 bytes 96:111 */

	/* Global offset for the {x,y,z} dimension of the work_item spawn */
	ndrange->ConstantBufferWrite(0, 96, &zero, 4);
	ndrange->ConstantBufferWrite(0, 100, &zero, 4);
	ndrange->ConstantBufferWrite(0, 104, &zero, 4);

	/* Global single dimension flat offset: x * y * z */
	ndrange->ConstantBufferWrite(0, 108, &zero, 4);

	/* CB0 bytes 112:127 */

	/* Group offset for the {x,y,z} dimensions of the work_item spawn */
	ndrange->ConstantBufferWrite(0, 112, &zero, 4);
	ndrange->ConstantBufferWrite(0, 116, &zero, 4);
	ndrange->ConstantBufferWrite(0, 120, &zero, 4);

	/* Group single dimension flat offset, x * y * z */
	ndrange->ConstantBufferWrite(0, 124, &zero, 4);

	/* CB0 bytes 128:143 */

	/* FIXME Offset in the global buffer where data segment exists */
	/* FIXME Offset in buffer for printf support */
	/* FIXME Size of the printf buffer */

}

}  // namespace SI
