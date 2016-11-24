
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

#ifndef ARCH_SI_PROGRAM_H
#define ARCH_SI_PROGRAM_H

#include <lib/cpp/ELFReader.h>

#include "Driver.h"

namespace SI
{

/// ConstantBuffer Class
class ConstantBuffer
{
	// Constant buffer ID (2-24)
	int id = 0;

	// Size of buffer
	unsigned size = 0;

	// Buffer data
	std::unique_ptr<char[]> data;

public:

	/// Constructor
	ConstantBuffer(int id, unsigned size);

	//
	// Class Members
	//

	// Pointer in memory to constant buffer
	unsigned device_address = 0;




	//
	// Getters
	//

	/// Returns the size of the constant buffer
	unsigned getSize() const { return size; }
	
	/// Returns a pointer to the constant buffer data
	char *getData() const { return data.get(); }
};

/// Program Class
class Program
{
	int id = 0;

	// ELF binary
	std::unique_ptr<ELFReader::File> elf_file;

	// List of constant buffers
	std::vector<std::unique_ptr<ConstantBuffer>> constant_buffers;

	// Initialize constant buffers from ELF binary 
	void InitializeConstantBuffers();

public:

	/// Constructor
	Program(int id);

	/// Load ELF binary into program object
	///
	/// \param buf
	///	Buffer containing OpenCL ELF program binary
	///
	/// \param size
	///	Size of buffer
	void setBinary(const char *buf, unsigned int size);

	/// Get the symbol in the Program ELF file by symbol name
	///
	/// \param name
	///	Name of the symbol
	ELFReader::Symbol *getSymbol(const std::string &name) const 
	{
		return elf_file->getSymbol(name); 
	}

	/// Get the id of the program
	int getId() const { return id; }

	/// Add a constant buffer object to the program
	ConstantBuffer *addConstantBuffer(int id, unsigned size);
	
	/// Get pointer to constant buffer by index. If index is out of bounds,
	/// the function will return a nullptr. This allows the user to keep 
	/// retrieving successive constant buffers. They will receive the
	/// nullptr when they have reached the end of the implemented constant
	/// buffers.
	ConstantBuffer *getConstantBufferByIndex(int index) const;
};


} // namespace SI

#endif /* ARCH_SI_PROGRAM_H */
