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
#include <string>

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


void CommandLineOptionBool::Read(std::deque<std::string> &arguments)
{
	// If option is present, set boolean variable
	*variable = true;
}


void CommandLineOptionString::Read(std::deque<std::string> &arguments)
{
	// Read value
	assert(arguments.size() > 0);
	*variable = arguments.front();
	arguments.pop_front();
}


void CommandLineOptionInt32::Read(std::deque<std::string> &arguments)
{
	// Read value
	assert(arguments.size() > 0);
	std::string argument = arguments.front();
	arguments.pop_front();

	// Convert value
	StringError error;
	*variable = StringToInt(argument, error);

	// Check valid value
	if (error)
		throw CommandLine::Error(misc::fmt("Invalid value for option "
				"'%s': %s", getName().c_str(),
				StringErrorToString(error)));
}


void CommandLineOptionInt64::Read(std::deque<std::string> &arguments)
{
	// Read value
	assert(arguments.size() > 0);
	std::string argument = arguments.front();
	arguments.pop_front();

	// Convert value
	StringError error;
	*variable = StringToInt64(argument, error);

	// Check valid value
	if (error)
		throw CommandLine::Error(misc::fmt("Invalid value for option "
				"'%s': %s", getName().c_str(),
				StringErrorToString(error)));
}


void CommandLineOptionDouble::Read(std::deque<std::string> &arguments)
{
	// Read value
	std::string argument = arguments.front();
	arguments.pop_front();

	// Convert value
	try
	{
		*variable = stod(argument);
	}
	catch (Exception &e)
	{
		throw CommandLine::Error(misc::fmt("Invalid value for option "
				"'%s'", getName().c_str()));
	}
}


void CommandLineOptionEnum::Read(std::deque<std::string> &arguments)
{
	// Read value
	assert(arguments.size() > 0);
	std::string argument = arguments.front();
	arguments.pop_front();

	// Convert value
	bool error;
	*variable = map.MapString(argument, error);

	// Check valid value
	if (error)
		throw CommandLine::Error(misc::fmt("Invalid value for option "
				"'%s': %s\nPossible values are %s.",
				getName().c_str(), argument.c_str(),
				map.toString().c_str()));
}


void CommandLineCategory::Help(std::ostream &os)
{
	// Skip category if it has no options
	if (option_map.size() == 0)
		return;

	// Header for categories other than default
	std::string header = description.empty() ?
			name + " Options" :
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
	instance = misc::new_unique<CommandLine>();
	return instance.get();
}


CommandLine::CommandLine()
{
	// Only one instance for singleton
	assert(instance == nullptr);

	// Create default category
	categories.emplace_back(misc::new_unique<CommandLineCategory>
			("default"));
	
	// Set category description
	CommandLineCategory *category = categories.back().get();
	category->setDescription("General options");

	// Add category into hash table
	category_map[category->getName()] = category;

	// Update current category
	current_category = category;
}


void CommandLine::Register(std::unique_ptr<CommandLineOption> &&option)
{
	// Command-line must not have been processed yet
	assert(!processed);

	// Option name must start with a dash
	if (!StringPrefix(option->getName(), "-"))
		throw misc::Panic(misc::fmt("Option name must start with a "
				"dash (%s)", option->getName().c_str()));
	
	// Options with one single dash must have one letter only
	if (!StringPrefix(option->getName(), "--") &&
			option->getName().length() != 2)
		throw misc::Panic(misc::fmt("Option with one dash must be one "
				"single character (%s)",
				option->getName().c_str()));

	// Check that option with same name was not present already
	if (option_table.find(option->getName()) != option_table.end())
		throw misc::Panic(misc::fmt("Option name already registered "
				"(%s)", option->getName().c_str()));

	// Add option to the current category
	option_table[option->getName()] = option.get();
	current_category->addOption(option.get());
	options.emplace_back(std::move(option));
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
		categories.emplace_back(misc::new_unique<CommandLineCategory>
				(name));
		category = categories.back().get();
		category_map[name] = category;
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
		throw misc::Panic(misc::fmt("Option no registered (%s)",
				name.c_str()));
	
	// Set incompatibility
	CommandLineOption *option = it->second;
	option->setIncompatible();
}


void CommandLine::Process(int argc, char **argv, bool options_anywhere)
{
	// Processed
	assert(!processed);
	processed = true;

	// Copy into a deque
	std::deque<std::string> arguments;
	for (int i = 0; i < argc; i++)
		arguments.push_back(argv[i]);

	// Save program name
	assert(arguments.size() > 0);
	program_name = arguments.front();
	arguments.pop_front();

	// Process command-line options
	std::unordered_map<std::string, CommandLineOption *> options;
	while (arguments.size() > 0)
	{
		// Get current argument
		std::string argument = arguments.front();

		// Not a command-line option
		if (!StringPrefix(argument, "-"))
		{
			// Skip argument if options can be anywhere in the
			// command line.
			if (options_anywhere)
			{
				
				this->arguments.push_back(argument);
				arguments.pop_front();
				continue;
			}

			// Stop processing if options can only be before the
			// first argument not being an option.
			break;
		}

		// Extract argument
		arguments.pop_front();

		// Special option --help
		if (argument == "--help")
		{
			show_help = true;
			continue;
		}

		// An option with a single dash can have its argument attached
		// to it without a space.
		bool attached_argument = !StringPrefix(argument, "--") &&
				argument.length() > 2;

		// Split attached argument
		if (attached_argument)
		{
			arguments.push_front(argument.substr(2));
			argument = argument.substr(0, 2);
		}

		// Find command-line option
		auto it = option_table.find(argument);
		if (it == option_table.end())
			throw Error(misc::fmt("Invalid option: %s\n%s",
					argument.c_str(),
					error_message.c_str()));

		// Get the command-line option
		CommandLineOption *option = it->second;

		// Check valid attached argument
		if (attached_argument && option->getNumArguments() != 1)
			throw Error(misc::fmt("Option '%s' expects %d "
					"argument(s)",
					option->getName().c_str(),
					option->getNumArguments()));

		// Check number of option arguments
		if ((int) arguments.size() < option->getNumArguments())
			throw Error(misc::fmt("Option '%s' expects %d "
					"argument(s)",
					option->getName().c_str(),
					option->getNumArguments()));

		// Check if option was already specified
		if (options.find(argument) != options.end())
			throw Error(misc::fmt("Multiple occurrences of "
					"option '%s'",
					option->getName().c_str()));

		// Store option in table
		options[argument] = option;

		// Process command-line option
		option->Read(arguments);
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
	while (arguments.size())
	{
		std::string argument = arguments.front();
		this->arguments.push_back(argument);
		arguments.pop_front();
	}

	// Option --help was specified
	if (show_help)
	{
		if (this->arguments.size() || options.size())
			throw Error("Options '--help' is incompatible with any "
					"other command-line option or "
					"argument.");
		Help();
		exit(0);
	}
}

}  // namespace misc

