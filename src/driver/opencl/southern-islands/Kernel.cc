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

 
#include <lib/cpp/Misc.h>
#include "Kernel.h"
#include "Program.h"

using namespace misc;

namespace SI
{

static const char *OpenCLErrSIKernelSymbol =
	"\tThe ELF file analyzer is trying to find a name in the ELF symbol table.\n"
	"\tIf it is not found, it probably means that your application is requesting\n"
	"\texecution of a kernel function that is not present in the encoded binary.\n"
	"\tPlease, check the parameters passed to the 'clCreateKernel' function in\n"
	"\tyour application.\n"
	"\tThis could be also a symptom of compiling an OpenCL kernel source on a\n"
	"\tmachine with an installation of the AMD SDK (using 'm2s-clcc') but\n"
	"\twith an incorrect or missing installation of the GPU driver. In this case,\n"
	"\tthe tool will still compile the kernel into LLVM, but the ISA section will\n"
	"\tbe missing in the kernel binary.\n";

static const char *OpenCLErrSIKernelMetadata =
	"\tThe kernel binary loaded by your application is a valid ELF file. In this\n"
	"\tfile, a '.rodata' section contains specific information about the OpenCL\n"
	"\tkernel. However, this information is only partially supported by Multi2Sim.\n"
	"\tTo request support for this error, please email 'development@multi2sim.org'.\n";

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

}

void Kernel::LoadMetaData()
{

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
	
	Driver::OpenCLDriver::debug << StringFmt("\tmetadata symbol: offset=0x%x, size=%u\n",
			(unsigned)metadata_symbol->getValue(), (unsigned)metadata_symbol->getSize());
	Driver::OpenCLDriver::debug << StringFmt("\theader symbol: offset=0x%x, size=%u\n",
			(unsigned)header_symbol->getValue(), (unsigned)header_symbol->getSize());
	Driver::OpenCLDriver::debug << StringFmt("\tkernel symbol: offset=0x%x, size=%u\n",
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



}  // namespace SI
