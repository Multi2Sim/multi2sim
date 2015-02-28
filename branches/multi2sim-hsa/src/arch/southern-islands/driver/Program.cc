
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

#include <arch/x86/emu/Emu.h>
#include <arch/southern-islands/emu/Emu.h>
#include <lib/cpp/String.h>

#include "Program.h"

#define __UNIMPLEMENTED__ misc::fatal("%s: unimplemented function", \
		__FUNCTION__);

namespace SI
{

ConstantBuffer::ConstantBuffer(int id, unsigned size, const char *data)
{
	this->id = id;
	this->size = size;
	this->data = std::unique_ptr<char> (new char[size]);
	// FIXME
	// std::copy(data, this->data.get(), size);
}

Program::Program(int id)
{
	this->id = id;
}

void Program::setBinary(const char *buf, unsigned int size)
{
	this->elf_file = std::unique_ptr<ELFReader::File>(new ELFReader::File(buf, size));
}

}  // namespace SI
