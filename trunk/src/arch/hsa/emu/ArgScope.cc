/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include "Emu.h"
#include "ArgScope.h"

namespace HSA
{

ArgScope::ArgScope() :
		arg_manager(Emu::getInstance()->getMemory())
{
}


ArgScope::~ArgScope()
{
}


void ArgScope::AddArgument(const std::string &name,
		unsigned int size, unsigned short type)
{
	unsigned int offset = arg_manager.Allocate(size);
	Argument *arg = new Argument;
	arg->offset = offset;
	arg->type = type;
	arg->size = size;
	argument_info.insert(std::make_pair(name,
			std::unique_ptr<Argument>(arg)));
}


char *ArgScope::getBuffer(const std::string &name)
{
	// Find argument information
	auto it = argument_info.find(name);
	if (it == argument_info.end())
	{
		throw Error(misc::fmt("Argument %s is not declared",
				name.c_str()));
		return nullptr;
	}

	// Retrieve guest address
	unsigned int guest_address = it->second->offset;

	// Retrieve buffer in host memory
	mem::Memory *memory = Emu::getInstance()->getMemory();
	return memory->getBuffer(guest_address,
			it->second->size, mem::Memory::AccessWrite);
}

}  // namespace HSA

