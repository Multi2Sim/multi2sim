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

 
#include <string.h>
#include <src/lib/cpp/ELFReader.h>
#include "Program.h"

using namespace ELFReader;

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
	memcpy(this->data, data, size);
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

}


Program::Program(int id, OpenCLDriver *driver)
{
	this->id = id;
	this->driver = driver;
}

void Program::SetBinary(const char *buf, unsigned int size)
{
	elf_file = std::unique_ptr<ELFReader::File>(new File(buf, size));

	InitializeConstantBuffers();
}

}  // namespace SI
