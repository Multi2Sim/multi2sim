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
#include "String.h"

using namespace misc;
	
	
int CommandLineOptionBool::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc);
	assert(argv[index] == getName());

	// If option is present, set boolean variable
	*var = true;

	// No extra argument consumed
	return 0;
}


int CommandLineOptionString::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc);
	assert(argv[index] == getName());

	// Check extra argument
	if (index == argc - 1)
		fatal("command-line option '%s' expects one argument",
				getName().c_str());
	
	// Read value
	*var = argv[index + 1];

	// One extra argument consumed
	return 1;
}


int CommandLineOptionInt32::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc);
	assert(argv[index] == getName());

	// Check extra argument
	if (index == argc - 1)
		fatal("command-line option '%s' expects one argument",
				getName().c_str());
	
	// Read value
	StringError error;
	*var = StringToInt(argv[index + 1], error);
	if (error)
		fatal("invalid value of option '%s': %s",
				getName().c_str(),
				StringGetErrorString(error));

	// One extra argument consumed
	return 1;
}


int CommandLineOptionInt64::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc);
	assert(argv[index] == getName());

	// Check extra argument
	if (index == argc - 1)
		fatal("command-line option '%s' expects one argument",
				getName().c_str());

	// Read value
	StringError error;
	*var = StringToInt(argv[index + 1], error);
	if (error)
		fatal("invalid value of option '%s': %s",
				getName().c_str(),
				StringGetErrorString(error));

	// One extra argument consumed
	return 1;
}


void CommandLine::Register(CommandLineOption *option)
{
	// Command-line must not have been processed yet
	assert(!processed);

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


void CommandLine::Help(std::ostream &os)
{
	// Command-line must have been processed
	assert(processed);
}


void CommandLine::Process()
{
	// Processed
	assert(!processed);
	processed = true;

	// Save program name
	assert(argc);
	program_name = argv[0];

	// Process command-line options
	int index;
	for (index = 1; index < argc; index++)
	{
		// No more command-line options
		if (!StringPrefix(argv[index], "-"))
			break;

		// Find command-line option
		auto it = option_table.find(argv[index]);
		if (it == option_table.end())
			fatal("command-line option '%s' is not recognized.\n%s",
					argv[index], error_message.c_str());

		// Read value of command-line option
		CommandLineOption *option = it->second;
		index += option->Read(argc, argv, index);
	}

	// Save rest of the arguments
	for (; index < argc; index++)
		args.push_back(argv[index]);
}

