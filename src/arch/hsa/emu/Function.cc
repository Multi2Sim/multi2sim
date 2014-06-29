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

Function::Function(const std::string& name, char *entry_point)
{
	this->name = name;
	this->entry_point = entry_point;
}


void Function::addArgument(const std::string &name, bool isInput,
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
	struct Argument *argument = new Argument();
	arg_info.insert(std::make_pair(name,
			std::unique_ptr<Argument>(argument)));
	argument->type = type;
	argument->offset = arg_size;
	argument->isInput = isInput;
	argument->size = BrigEntry::type2size(type);

	// Increase allocated argument size
	this->arg_size += argument->size;
}

}  // namespace HSA
