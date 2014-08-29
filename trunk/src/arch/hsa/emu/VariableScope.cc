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
#include "VariableScope.h"
#include "Function.h"

namespace HSA
{

VariableScope::VariableScope()
{
}


VariableScope::~VariableScope()
{
}


void VariableScope::DeclearVariable(const std::string &name,
		unsigned int size, unsigned short type)
{
	// Retrieve global memory manager
	mem::Manager *manager = Emu::getInstance()->getMemoryManager();

	// Allocated memory
	unsigned address = manager->Allocate(size, 1);

	// Set up and insert into variable list
	Variable *arg = new Variable(name, type, size, address);
	variable_info.insert(std::make_pair(name,
			std::unique_ptr<Variable>(arg)));
}


char *VariableScope::getBuffer(const std::string &name)
{
	// Find argument information
	auto it = variable_info.find(name);
	if (it == variable_info.end())
	{
		return nullptr;
	}

	// Retrieve guest address
	unsigned int guest_address = it->second->getAddress();

	// Retrieve buffer in host memory
	mem::Memory *memory = Emu::getInstance()->getMemory();
	return memory->getBuffer(guest_address,
			it->second->getSize(), mem::Memory::AccessWrite);
}


void VariableScope::Dump(std::ostream &os, unsigned int indent) const
{
	for (auto it = variable_info.begin(); it != variable_info.end(); it++)
	{
		it->second->Dump(os, indent);
		os << "\n";
	}
}


void VariableScope::DumpInLine(std::ostream &os) const
{
	for (auto it = variable_info.begin(); it != variable_info.end(); it++)
	{
		if (it != variable_info.begin())
			os << ", ";
		it->second->Dump(os, 0, true);
	}
}

}  // namespace HSA

