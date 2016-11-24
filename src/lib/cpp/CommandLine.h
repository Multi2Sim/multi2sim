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

#ifndef LIB_CPP_COMMAND_LINE_H
#define LIB_CPP_COMMAND_LINE_H

#include <cassert>
#include <deque>
#include <map>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>

#include "Error.h"
#include "Misc.h"
#include "String.h"


namespace misc
{

/// Base abstract class for command-line options.
class CommandLineOption
{
public:

	enum Type
	{
		TypeInvalid = 0,
		TypeBool,
		TypeString,
		TypeInt32,
		TypeInt64,
		TypeDouble,
		TypeEnum
	};

private:

	// Option type, determining actual subclass
	Type type;

	// Option name as given by the user in the command line
	std::string name;

	// Option name together with extra arguments
	std::string help_name;

	// Number of arguments for the option
	int num_args;

	// Help string associated with the option
	std::string help;

	// True if option was specified by the user
	bool present = false;

	// True if option is incompatible with any other option
	bool incompatible = false;

public:

	/// Constructor
	///
	/// \param name
	///	Name of the option. It must start with a double dash
	///	(`--`). Only the first token of this string is considered
	///	as the option name. The complete string will be used to display
	///	the help message.
	///
	/// \param num_args
	///	Number of additional arguments required by the command-line
	///	option.
	/// \param help
	///	String to print as a help message related with the option.
	CommandLineOption(Type type, const std::string &name, int num_args,
			const std::string &help);

	/// Virtual destructor to make class polymorphic.
	virtual ~CommandLineOption() { }

	/// Return the type of the command-line option. This type determines the
	/// actual instantiated subclass.
	Type getType() const { return type; }

	/// Return the option name
	const std::string &getName() const { return name; }

	/// Return the option name with all extra arguments used to display the
	/// help string for it.
	const std::string &getHelpName() const { return help_name; }

	/// Return \a true if the command-line option was specified by the user.
	bool isPresent() const { return present; }

	/// Return the number of arguments that the option expects
	int getNumArguments() const { return num_args; }

	/// Make option incompatible with any other option.
	void setIncompatible() { incompatible = true; }

	/// Return whether the option has been labeled as incompatible with any
	/// other option.
	bool isIncompatible() { return incompatible; }

	/// Dump help message related with this option
	void DumpHelp(std::ostream &os) const;

	/// Read the arguments for a command-line option from the head of the
	/// list provided in \a arguments. The option arguments will be removed
	/// from the head of the list.
	virtual void Read(std::deque<std::string> &arguments) = 0;

	/// Dump help about command line option into output stream
	void Help(std::ostream &os = std::cout) const;
};


/// Command-line option taking a string as an argument
class CommandLineOptionString : public CommandLineOption
{
	// Variable affected by this option
	std::string *variable;

public:

	/// Constructor
	CommandLineOptionString(const std::string &name,
			std::string *variable,
			const std::string &help) :
			CommandLineOption(TypeString, name, 1, help),
			variable(variable)
	{
	}
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(std::deque<std::string> &arguments);
};


/// Command-line option taking a 32-bit integer as an argument
class CommandLineOptionInt32 : public CommandLineOption
{
	// Variable affected by this option
	int *variable;

public:

	/// Constructor
	CommandLineOptionInt32(const std::string &name,
			int *variable,
			const std::string &help) :
			CommandLineOption(TypeInt32, name, 1, help),
			variable(variable)
	{
	}
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(std::deque<std::string> &arguments);
};


/// Command-line option taking a 64-bit integer as an argument
class CommandLineOptionInt64 : public CommandLineOption
{
	// Variable affected by this option
	long long *variable;

public:

	/// Constructor
	CommandLineOptionInt64(const std::string &name,
			long long *variable,
			const std::string &help) :
			CommandLineOption(TypeInt64, name, 1, help),
			variable(variable)
	{
	}
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(std::deque<std::string> &arguments);
};


/// Command-line option taking a double precision floating point as an argument
class CommandLineOptionDouble : public CommandLineOption
{
	// Variable affected by this option
	double *variable;

public:

	/// Constructor
	CommandLineOptionDouble(const std::string &name,
			double *variable,
			const std::string &help) :
			CommandLineOption(TypeDouble, name, 1, help),
			variable(variable)
	{
	}

	/// Read option from command line. See CommandLineOption::Read().
	void Read(std::deque<std::string> &arguments);
};


/// Command-line option taking an identifier as an argument
class CommandLineOptionEnum : public CommandLineOption
{
	// Variable affected by this option
	int *variable;

	// String map used to translate enumeration
	const StringMap &map;

public:

	/// Constructor
	CommandLineOptionEnum(const std::string &name,
			int *variable,
			const StringMap &map,
			const std::string &help) :
			CommandLineOption(TypeEnum, name, 1, help),
			variable(variable),
			map(map)
	{
	}
	
	/// Read option from command line. See CommandLineOption::Read()
	void Read(std::deque<std::string> &arguments);
};


/// Command-line option taking no argument
class CommandLineOptionBool : public CommandLineOption
{
	// Variable affected by this option
	bool *variable;

public:

	/// Constructor
	CommandLineOptionBool(const std::string &name,
			bool *variable,
			const std::string &help) :
			CommandLineOption(TypeBool, name, 0, help),
			variable(variable)
	{
	}
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(std::deque<std::string> &arguments);
};


/// Command-line options are grouped in categories represented by this class
class CommandLineCategory
{
	// Category name
	std::string name;

	// Description shown in help message
	std::string description;

	// Map of command-line options
	std::map<std::string, CommandLineOption *> option_map;

public:

	/// Constructor
	CommandLineCategory(const std::string &name) : name(name)
	{
	}

	/// Get category name
	const std::string &getName() const { return name; }

	/// Set category description
	void setDescription(const std::string &description)
	{
		this->description = description;
	}

	/// Add an option to the category
	void addOption(CommandLineOption *option)
	{
		option_map[option->getName()] = option;
	}

	/// Print help message for this category and all its options.
	void Help(std::ostream &os = std::cout);
};


/// Class representing the command-line used to invoke Multi2Sim.
class CommandLine
{
	// Error message to show when finding invalid options
	std::string error_message;

	// Program name, originally found in argv[0]
	std::string program_name;

	// Arguments left after removing program name and command-line options
	std::vector<std::string> arguments;

	// Hash table of registered command-line options.
	std::unordered_map<std::string, CommandLineOption *> option_table;

	// Sequential list of registered command-line options. We keep the
	// sequential order of registration to show an organized help message.
	std::vector<std::unique_ptr<CommandLineOption>> options;

	// Ordered map of categories
	std::map<std::string, CommandLineCategory *> category_map;

	// List of category unique pointers
	std::vector<std::unique_ptr<CommandLineCategory>> categories;

	// Current category, as set with function 'setCategory()'
	CommandLineCategory *current_category;

	// Register a command-line option
	void Register(std::unique_ptr<CommandLineOption> &&option);

	// Set to true when function Process() is invoked.
	bool processed = false;

	// Show help message with command-line options
	bool show_help = false;

	// Help message header
	std::string help;

	// Singleton instance
	static std::unique_ptr<CommandLine> instance;

public:

	/// Exception thrown by the command line processor
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Command line");
		}
	};
	
	/// Get instance of singleton
	static CommandLine *getInstance();

	/// Constructor
	CommandLine();

	/// Set the error message to be displayed when an invalid option is
	/// found in the command line during the execution of function
	/// Process().
	void setErrorMessage(const std::string &msg)
	{
		assert(!processed);
		error_message = msg;
	}

	/// Set the help message to be displayed when option '--help' is
	/// present, preceding the help for all command-line options.
	void setHelp(const std::string &msg)
	{
		assert(!processed);
		help = msg;
	}

	/// Return the program name used to invoke Multi2Sim. The command line
	/// must have been processed with a call to Process().
	const std::string &getProgramName() const
	{
		assert(processed);
		return program_name;
	}

	/// Return the number of arguments left after getting rid of the
	/// command-line options, and the program name. The command-line must
	/// have been processed with a call to Process().
	int getNumArguments() const
	{
		assert(processed);
		return arguments.size();
	}

	/// Return the argument with a specific \a index. A value of 0 for \a
	/// index specifies the first argument without considering all
	/// command-line options or the program name. The command line must have
	/// been processed with a call to Process().
	const std::string &getArgument(int index) const
	{
		assert(index >= 0 && index < (int) arguments.size());
		return arguments[index];
	}

	/// Return a constant reference to the internal vector containing the
	/// list of command-line arguments, after having gotten rid of all
	/// command-line options. The command line must have been processed with
	/// a call to Process().
	const std::vector<std::string> &getArguments() const
	{
		assert(processed);
		return arguments;
	}

	/// Register a command-line option that takes no argument. If present,
	/// variable \a variable will be set to \a true. If not, to \a false.
	void Register(const std::string &name, bool &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionBool>(name,
				&variable, help));
	}

	/// Register a command-line option taking a string as an argument. If
	/// present, variable \a variable takes the value of the argument. If not
	/// present, the original content of \a variable is not modified. The value
	/// passed in argument \a help is the help string shown to the user when
	/// invoking Help().
	///
	/// If \a name is a string with two tokens separated by a space, only
	/// the first token is considered as the option name, but the whole
	/// string will be shown when displaying the help message. This is
	/// useful to specify a name for the formal argument that the option
	/// takes (e.g. "--x86-max-inst <instructions>").
	///
	/// All Register<tt>xxx</tt>() calls must be invoked before processing
	/// the actual command line with a call to Process().
	void RegisterString(const std::string &name, std::string &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionString>(name,
				&variable, help));
	}

	/// Same as RegisterString(), but taking an unsigned 32-bit integer as the
	/// type of the command-line option.
	void RegisterInt32(const std::string &name, int &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionInt32>(name,
				&variable, help));
	}

	/// Same as RegisterString(), but taking a signed 64-bit integer as the
	/// type of the command-line option.
	void RegisterUInt32(const std::string &name, unsigned &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionInt32>(name,
				(int *) &variable, help));
	}

	/// Same as RegisterString(), but taking an unsigned 64-bit integer as
	/// the type of the command-line option.
	void RegisterInt64(const std::string &name, long long &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionInt64>(name,
				&variable, help));
	}

	/// Same as RegisterString(), but taking a signed 32-bit integer as the
	/// type of the command-line option.
	void RegisterUInt64(const std::string &name,
			unsigned long long &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionInt64>(name,
				(long long *) &variable, help));
	}

	/// Same as RegisterString(), but taking a double as the
	/// type of the command-line option.
	void RegisterDouble(const std::string &name,
			double &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionDouble>(name,
				(double *) &variable, help));
	}

	/// Register an option that can take values from the string map provided
	/// in argument \a map.
	void RegisterEnum(const std::string &name, int &variable,
			const StringMap &map, const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionEnum>(name,
				&variable, map, help));
	}

	/// Register a boolean option with no additional arguments. If the
	/// option is present, a value of \c True is assumed, and \c False if
	/// the option is not provided by the user.
	void RegisterBool(const std::string &name, bool &variable,
			const std::string &help)
	{
		Register(misc::new_unique<CommandLineOptionBool>(name,
				&variable, help));
	}

	/// Make option \a name incompatible with any other option.
	void setIncompatible(const std::string &name);
	
	/// Dump help for all registered command-line options.
	void Help(std::ostream &os = std::cout);

	/// Set the current category affecting all subsequently registered
	/// command-line options. Options will be grouped into these categories
	/// when dumping the help message.
	///
	/// \param name
	///	Name of an existing category, or name of a category to be
	///	created, affecting subsequent command-line options.
	///
	/// \param description
	///	Optional argument providing a description for the cateogry. If
	///	no value is given, the description will be the same as the name,
	///	or will take the value set by a previous call to setCategory().
	void setCategory(const std::string &name,
			const std::string &description = "");

	/// Process command line from the arguments
	///
	/// \param argc
	///	Number of arguments
	///
	/// \param argv
	///	Argument vector, as passed in the main program
	///
	/// \param options_anywhere (optional)
	///	If `true` (default), this flag indicates that there can be
	///	options anywhere in the command line, even after the first
	///	argument that is not an option. If `false`, option processing
	///	will stop after the first argument is found that is not an
	///	option.
	///
	/// \throw
	///	An exception will occur if any of the options passed in the
	///	command line are invalid or does not have enough arguments.
	void Process(int argc, char **argv, bool options_anywhere = true);
};


}  // namespace misc

#endif

