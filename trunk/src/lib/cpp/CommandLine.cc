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

#include <cstring>

#include "CommandLine.h"
#include "Misc.h"
#include "String.h"


namespace misc
{

CommandLineOption::CommandLineOption(Type type,
		const std::string &name,
		int num_args,
		const std::string &help) :
		type(type),
		num_args(num_args),
		help(help)
{
	// Option name
	std::vector<std::string> tokens;
	misc::StringTokenize(name, tokens);
	assert(tokens.size() > 0);
	this->name = tokens[0];
	help_name = name;
}


void CommandLineOption::Help(std::ostream &os) const
{
	// Option name
	os << "  " << help_name << "\n\n";

	// Description
	StringFormatter formatter;
	formatter.setIndent(8);
	formatter << help;
	os << formatter << "\n\n";
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
		throw CommandLine::Error(misc::fmt("Invalid value for option "
				"'%s': %s", getName().c_str(),
				StringErrorToString(error)));
}


void CommandLineOptionInt64::Read(int argc, char **argv, int index)
{
	// Checks
	assert(index < argc - 1);
	assert(argv[index] == getName());

	// Read value
	StringError error;
	*var = StringToInt64(argv[index + 1], error);
	if (error)
		throw CommandLine::Error(misc::fmt("Invalid value for option "
				"'%s': %s", getName().c_str(),
				StringErrorToString(error)));
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
		throw CommandLine::Error(misc::fmt("Invalid value for option "
				"'%s': %s\nPossible values are %s.",
				getName().c_str(),
				argv[index + 1],
				map.toString().c_str()));
}


void CommandLineCategory::Help(std::ostream &os)
{
	// Skip category if it has no options
	if (option_map.size() == 0)
		return;

	// Header for categories other than default
	std::string header = description.empty() ?
			name + " options" :
			description;
	os << "\n";
	os << header << "\n";
	os << std::string(header.length(), '=') << "\n";
	os << "\n";

	// Dump help for all options
	for (auto it : option_map)
		it.second->Help(os);
}


// Singleton instance
std::unique_ptr<CommandLine> CommandLine::instance;

CommandLine *CommandLine::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new CommandLine());
	return instance.get();
}


CommandLine::CommandLine()
{
	// Create default category
	CommandLineCategory *category = new CommandLineCategory("default");
	category->setDescription("General options");
	category_list.emplace_back(category);
	category_map[category->getName()] = category;
	current_category = category;
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

	// Add option to the current category
	option_table[option->getName()] = option;
	option_list.emplace_back(option);
	current_category->addOption(option);
}


void CommandLine::Help(std::ostream &os)
{
	// Command-line must have been processed
	assert(processed);

	// Header
	if (!help.empty())
	{
		StringFormatter formatter(help);
		os << '\n' << formatter;
	}

	// Empty line
	os << '\n';

	// Print options for default category
	category_map["default"]->Help();

	// Print help message for other categories
	for (auto it : category_map)
	{
		CommandLineCategory *category = it.second;
		if (category->getName() != "default")
			category->Help();
	}
}


void CommandLine::setCategory(const std::string &name,
		const std::string &description)
{
	// Create new category if it doesn't exist
	CommandLineCategory *category;
	if (category_map.find(name) == category_map.end())
	{
		category = new CommandLineCategory(name);
		category_map[name] = category;
		category_list.emplace_back(category);
	}
	else
	{
		category = category_map[name];
	}

	// Set description if present
	if (!description.empty())
		category->setDescription(description);
	
	// Set current category
	current_category = category;
}


void CommandLine::setIncompatible(const std::string &name)
{
	// Command-line must not have been processed yet
	assert(!processed);

	// Find option
	auto it = option_table.find(name);
	if (it == option_table.end())
		panic("%s: option '%s' has not been registered",
				__FUNCTION__, name.c_str());
	
	// Set incompatibility
	CommandLineOption *option = it->second;
	option->setIncompatible();
}


bool CommandLine::Process(int argc, char **argv, bool fatal_on_bad_option)
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

		// Special option -c
		if (!strcmp(argv[index], "-c"))
		{
			use_c = true;
			continue;
		}

		// Special option --help
		if (!strcmp(argv[index], "--help"))
		{
			show_help = true;
			continue;
		}

		// Find command-line option
		auto it = option_table.find(argv[index]);
		if (it == option_table.end())
		{
			if (fatal_on_bad_option)
			{
				throw Error(misc::fmt("Invalid option: %s\n%s",
						argv[index],
						error_message.c_str()));
			}
			else
			{
				return false;
			}
		}

		// Check extra arguments
		CommandLineOption *option = it->second;
		if (index + option->getNumArguments() >= argc)
			throw Error(misc::fmt("Option '%s' expects %d "
					"argument(s)",
					option->getName().c_str(),
					option->getNumArguments()));

		// Check if option was already specified
		if (options.find(argv[index]) != options.end())
			throw Error(misc::fmt("Multiple occurrences of "
					"option '%s'",
					option->getName().c_str()));
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
				throw Error(misc::fmt("Option '%s' is "
						"incompatible with any "
						"other option.",
						option->getName().c_str()));
		}
	}

	// Save rest of the arguments
	for (; index < argc; index++)
		args.push_back(argv[index]);

	// Option --help was specified
	if (!use_c && show_help)
	{
		if (args.size() || options.size())
			throw Error("Options '--help' is incompatible with any "
					"other command-line option or "
					"argument.");
		Help();
		exit(0);
	}

	// Command line successfully processed
	return true;
}

}  // namespace misc

