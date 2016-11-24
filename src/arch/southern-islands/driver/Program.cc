
/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#include <string>

#include <arch/southern-islands/emulator/Emulator.h>
#include <lib/cpp/String.h>

#include "Program.h"


#define __UNIMPLEMENTED__ misc::fatal("%s: unimplemented function", \
		__FUNCTION__);

namespace SI
{

ConstantBuffer::ConstantBuffer(int id, unsigned size)
{
	this->id = id;
	this->size = size;
	this->data = misc::new_unique_array<char>(size);
}


Program::Program(int id)
{
	this->id = id;
}


ConstantBuffer *Program::addConstantBuffer(int id, unsigned size)
{
	// Create new constant buffer and insert it to program list
	constant_buffers.emplace_back(misc::new_unique<ConstantBuffer>(id, 
			size));

	// Return
	return constant_buffers.back().get();
}


void Program::InitializeConstantBuffers()
{
	// Get emulator and video memory
	SI::Emulator *emulator = SI::Emulator::getInstance();
	mem::Memory *video_memory = emulator->getVideoMemory();
	
	// Reserve constant buffers encoded in ELF file
	constant_buffers.reserve(25);

	// We can't tell how many constant buffers exist in advance, but we      
	// know they should be enumerated, starting with '2'.  This loop         
	// searches until a constant buffer matching the format is not           
	// found.                                                          
	for (int i = 2; i < 25; i++)                                                 
	{                                                                        
		// Create string of symbol name
		std::ostringstream symbol_name;
		symbol_name << "__OpenCL_" << i << "_global";

		// Check to see if symbol exists
		ELFReader::Symbol *symbol = getSymbol(symbol_name.str());
		if (!symbol)                                                     
			break;                                                   

		// Read the elf symbol into a buffer
		std::istringstream symbol_stream;
		symbol->getStream(symbol_stream);
		Driver::debug << misc::fmt("\tconstant buffer '%s' found with size %d and offset 0x%x\n",                              
				symbol->getName().c_str(), 
				(unsigned) symbol->getSize(),
				(unsigned) symbol->getValue());

		// Create the constant buffer object and add it to the vector
		ConstantBuffer *constant_buffer = addConstantBuffer(i, 
				symbol->getSize());
		
		// Set device pointer
		constant_buffer->device_address = emulator->getVideoMemoryTop();
		
		// Copy the symbol data into the constant buffer object
		symbol_stream.read(constant_buffer->getData(), 
				symbol->getSize());

		// Map new pages                                                         
		video_memory->Map(emulator->getVideoMemoryTop(), 
				symbol->getSize(),    
				mem::Memory::AccessRead | 
				mem::Memory::AccessWrite);     
		
		// Copy constant buffer into device memory
		video_memory->Write(emulator->getVideoMemoryTop(), 
				symbol->getSize(), 
				constant_buffer->getData());                           

		// Increment video memory
		emulator->incVideoMemoryTop(symbol->getSize());
	}      


}


void Program::setBinary(const char *buf, unsigned int size)
{
	// Create a new ELF file based on the passed buffer
	elf_file = misc::new_unique<ELFReader::File>(buf, size);

	// Initialize constant buffers based on global symbols
	InitializeConstantBuffers();
}


ConstantBuffer *Program::getConstantBufferByIndex(int index) const 
{
	// Retrieve CB by index. Program does not hold CB0 or CB1
	if (index >= 0 && index < 2)
		throw misc::Error(misc::fmt("Progam object does not hold"
				"constant buffers lower than 2"));
	else if (index >= 2 && index < (int) constant_buffers.size() + 2)
		return constant_buffers[index - 2].get();
	else
		return nullptr;
}


}  // namespace SI
