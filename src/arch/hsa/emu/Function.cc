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

#include <lib/cpp/String.h>
#include <arch/hsa/asm/BrigEntry.h>

#include "Emu.h"
#include "Function.h"

namespace HSA
{

Function::Function(const std::string& name, char *directive, char *entry_point)
{
	this->name = name;
	this->directive = directive;
	this->entry_point = entry_point;
}


void Function::addArgument(const std::string &name, bool is_input,
			unsigned short type)
{
	// Check if argument is defined
	std::map<std::string, std::unique_ptr<Argument>>::iterator it
			= arg_info.find(name);
	if (it != arg_info.end())
	{
		throw Error(misc::fmt("Function argument %s redefined",
				name.c_str()));
	}

	// Insert argument into table
	arg_info.insert(std::make_pair(name,
			std::unique_ptr<Argument>(new Argument)));
	struct Argument *argument = arg_info[name].get();
	argument->type = type;
	argument->offset = arg_size;
	argument->is_input = is_input;
	argument->size = BrigEntry::type2size(type);

	// Increase allocated argument size
	this->arg_size += argument->size;
}


unsigned int Function::getRegisterSizeByName(const std::string &name) const
{

	// First byte is '$'
	if (name[0] != '$')
	{
		throw Error("Invalid register name " + name
				+ ". Expect ($) to be register prefix");
		return 0;
	}

	// Get the index number of the register
	unsigned int register_index = atoi(name.c_str() + 2);

	switch (name[1])
	{
	case 'c':
		if (register_index > 7)
		{
			throw Error("Invalid register name " + name
					+ ". Expecte register index between"
					" 0 and 7");
			return 0;
		}
		return 1;
	case 's':
		if (register_index > 127)
		{
			throw Error("Invalid register name " + name
					+ ". Expecte register index between"
					" 0 and 127");
			return 0;
		}
		return 4;
	case 'd':
		if (register_index > 63)
		{
			throw Error("Invalid register name " + name
					+ ". Expecte register index between"
					" 0 and 63");
			return 0;
		}
		return 8;
	case 'q':
		if (register_index > 31)
		{
			throw Error("Invalid register name " + name
					+ ". Expecte register index between"
					" 0 and 31");
			return 0;
		}
		return 16;
	default:
		throw Error("Invalid register name " + name +
				+ ". Expect register type to be (c, s, d, q)");
		return 0;
	}
}

int Function::getRegisterOffset(const std::string &name)
{
	auto it = reg_info.find(name);
	if (it == reg_info.end())
	{
		return -1;
	}
	return it->second;
}

void Function::addRegister(const std::string &name)
{
	// Validate the name of the register
	int size = getRegisterSizeByName(name);

	// Skip C registers
	if (size == 1)
		return;

	// Check if the register exists
	int offset = getRegisterOffset(name);
	if (offset >= 0)
		return;

	// Check if enough space is available
	if (reg_size + size > 512)
		throw Error(misc::fmt("No enough space to allocated register %s"
				"in function %s.",
				name.c_str(), this->name.c_str()));

	// Insert the register information
	reg_info.insert(std::make_pair(name, reg_size));
	reg_size += size;
}


void Function::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n****************************************"
			"***************************************\n");

	// Dump function information
	os << misc::fmt("\tFunction name: %s.\n", name.c_str());

	// Dump argument related information
	DumpArgumentInfo(os);

	// Dump register related information
	DumpRegisterInfo(os);

	os << misc::fmt("****************************************"
			"***************************************\n");
}


void Function::DumpArgumentInfo(std::ostream &os = std::cout) const
{
	// Dump the argument information
	os << misc::fmt("\n\t***** Arguments *****\n");
	std::map<std::string, std::unique_ptr<Argument>>::const_iterator it;
	for (it = arg_info.begin(); it != arg_info.end(); it++)
	{
		os << "\t";
		if (it->second->is_input)
			os << "Input ";
		else
			os << "Output ";
		os << misc::fmt("argument %s, %s, size %d, offset %d\n",
				BrigEntry::type2str(it->second->type).c_str(),
				it->first.c_str(),
				it->second->size,
				it->second->offset);
	}
	os << misc::fmt("\tArgument size allocated %d bytes\n", arg_size);
	os << misc::fmt("\t*********************\n\n");
}


void Function::DumpRegisterInfo(std::ostream &os = std::cout) const
{
	// Dump the argument information
	os << misc::fmt("\n\t***** Registers *****\n");
	for (auto it = reg_info.begin(); it != reg_info.end(); it++)
	{
		os << misc::fmt("\tregister %s, offset %d\n",
				it->first.c_str(),
				it->second);
	}
	os << misc::fmt("\tRegister size allocated %d bytes\n", reg_size);
	os << misc::fmt("\t*********************\n\n");
}

}  // namespace HSA
