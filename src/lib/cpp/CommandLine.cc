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

void CommandLineOption::Help(std::ostream &os) const
{
	// Option name
	os << "  " << name;

	// Arguments
	if (num_args == 1)
		os << " <arg>";
	else
		for (int i = 1; i <= num_args; i++)
			os << " <arg" << i << ">";

	// Description
	os << "\n\n" << StringParagraph(help, 8, 8) << '\n';
}


void CommandLineOptionBool::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc);
	assert(argv[index] == getName());

	// If option is present, set boolean variable
	*var = true;
}


void CommandLineOptionString::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc - 1);
	assert(argv[index] == getName());

	// Read value
	*var = argv[index + 1];
}


void CommandLineOptionInt32::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc - 1);
	assert(argv[index] == getName());

	// Read value
	StringError error;
	*var = StringToInt(argv[index + 1], error);
	if (error)
		fatal("invalid value of option '%s': %s",
				getName().c_str(),
				StringErrorToString(error));
}


void CommandLineOptionInt64::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc - 1);
	assert(argv[index] == getName());

	// Read value
	StringError error;
	*var = StringToInt(argv[index + 1], error);
	if (error)
		fatal("invalid value of option '%s': %s",
				getName().c_str(),
				StringErrorToString(error));
}


void CommandLineOptionEnum::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc - 1);
	assert(argv[index] == getName());

	// Read value
	bool error;
	*var = map.MapString(argv[index + 1], error);
	if (error)
		fatal("'%s' is not a valid value of option '%s'.\n"
				"Possible values are %s.",
				argv[index + 1],
				getName().c_str(),
				map.toString().c_str());
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

	// Print help message for every option
	for (auto &option : option_list)
	{
		option->Help(os);
	}
}


void CommandLine::setIncompatible(const std::string &name)
{
	// Find option
	auto it = option_table.find(name);
	if (it == option_table.end())
		panic("%s: option '%s' has not been registered",
				__FUNCTION__, name.c_str());
	
	// Set incompatibility
	CommandLineOption *option = it->second;
	option->setIncompatible();
}


bool CommandLine::Process(bool fatal_on_bad_option)
{
	// Processed
	assert(!processed);
	processed = true;

	// Save program name
	assert(argc);
	program_name = argv[0];

	// Process command-line options
	int index;
	std::unordered_map<std::string, CommandLineOption *> options;
	for (index = 1; index < argc; index++)
	{
		// No more command-line options
		if (!StringPrefix(argv[index], "-"))
			break;

		// Find command-line option
		auto it = option_table.find(argv[index]);
		if (it == option_table.end())
		{
			if (fatal_on_bad_option)
				fatal("command-line option '%s' is not "
						"recognized. %s",
						argv[index],
						error_message.c_str());
			else
				return false;
		}

		// Check extra arguments
		CommandLineOption *option = it->second;
		if (index + option->getNumArguments() >= argc)
			fatal("option '%s' expects %d argument(s)",
					option->getName().c_str(),
					option->getNumArguments());

		// Check if option was already specified
		if (options.find(argv[index]) != options.end())
			fatal("options '%s' found multiple times",
					option->getName().c_str());
		options[argv[index]] = option;

		// Process command-line option
		option->Read(argc, argv, index);

		// Consume extra arguments
		index += option->getNumArguments();
	}

	// If the user specified more than one option, check that none of them
	// has been defines as incompatible.
	if (options.size() > 1)
	{
		for (auto &it : options)
		{
			CommandLineOption *option = it.second;
			if (option->isIncompatible())
				fatal("option '%s' is incompatible with any "
						"other option.",
						option->getName().c_str());
		}
	}

	// Save rest of the arguments
	for (; index < argc; index++)
		args.push_back(argv[index]);

	// Command line successfully processed
	return true;
}

