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

#ifndef DRIVER_OPENCL_SI_PROGRAM_H
#define DRIVER_OPENCL_SI_PROGRAM_H

#include <memory>
#include <src/lib/cpp/ELFReader.h>

namespace SI
{

class OpenCLDriver;
	
class ConstantBuffer
{
	// Constant buffer ID (2-24)
	int id;

	unsigned size;
	char *data;

public:
	ConstantBuffer(int id, unsigned size, const char *data);
	~ConstantBuffer();

};

class Program
{
	int id;

	// Opencl driver it belongs to
	OpenCLDriver *driver;

	// ELF binary
	std::unique_ptr<ELFReader::File> elf_file;

	// List of constant buffers
	std::vector<std::unique_ptr<ConstantBuffer>> constant_buffers;

	// 
	void InitializeConstantBuffers();

public:
	Program(int id, OpenCLDriver *driver);

	void SetBinary(const char *buf, unsigned int size);

};

}  // namespace SI

#endif
