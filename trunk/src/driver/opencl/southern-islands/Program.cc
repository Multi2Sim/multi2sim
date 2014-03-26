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
#include <driver/opencl/OpenCLDriver.h>
#include <lib/cpp/String.h>

#include "Program.h"


namespace SI
{

/* 
 * Class ConstantBuffer 
 */

ConstantBuffer::ConstantBuffer(int id, unsigned size, const char *data)
{
	this->id = id;
	this->size = size;
	this->data = new char[size];
	std::copy(&data[0], &data[size], this->data);
}

ConstantBuffer::~ConstantBuffer()
{
	delete[] data;
}


/*
 * Class Program
 */

void Program::InitializeConstantBuffers()
{
	ELFReader::Symbol *symbol;
	std::string symbol_name;

	assert(this->elf_file);

	/* Constant buffers encoded in ELF file */
	for (unsigned i = 0; i < EmuMaxNumConstBufs; i++) 
		constant_buffers.push_back(nullptr);

	/* We can't tell how many constant buffers exist in advance, but we
	 * know they should be enumerated, starting with '2'.  This loop
	 * searches until a constant buffer matching the format is not 
	 * found. */
	for (unsigned i = 2; i < EmuMaxNumConstBufs; i++) 
	{
		/* Create string of symbol name */
		symbol_name = misc::fmt("__OpenCL_%d_global", i);

		/* Check to see if symbol exists */
		symbol = this->elf_file->getSymbol(symbol_name);
		if (!symbol)
			break;

		/* Read the elf symbol into a buffer */
		x86::Emu::opencl_debug << misc::fmt("\tconstant buffer '%s' found with size %d\n",
			symbol->getName().c_str(), symbol->getSize());

		/* Create buffer and add constant buffer to list */
		constant_buffers[i] = std::unique_ptr<ConstantBuffer>(new ConstantBuffer(i, 
			symbol->getSize(), symbol->getBuffer()));
		
		/* Increase video memory top */
		driver->getEmuGpu()->incVideoMemTop(symbol->getSize());
	}
}


Program::Program(int id)
{
	this->id = id;
	this->driver = Driver::OpenCLSIDriver::getInstance();
}

void Program::SetBinary(const char *buf, unsigned int size)
{
	this->elf_file = std::unique_ptr<ELFReader::File>(new ELFReader::File(buf, size));

	InitializeConstantBuffers();
}

}  // namespace SI
