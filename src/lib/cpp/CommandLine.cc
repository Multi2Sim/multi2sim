/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include "CommandLine.h"
#include "Misc.h"

using namespace misc;

void CommandLine::Register(CommandLineOption *option)
{
	// Option name must start with two dashes
	if (!StringPrefix(option->getName(), "--"))
		panic("%s: %s: option name must start with double dash",
				__FUNCTION__, option->getName().c_str());

	// Check that option with same name was not present already
	if (option_table.find(option->getName()) != option_table.end())
		panic("%s: %s: option already registered",
				__FUNCTION__, option->getName().c_str());

	// Add option
	option_table[option->getName()] = option;
	option_list.emplace_back(option);
}

