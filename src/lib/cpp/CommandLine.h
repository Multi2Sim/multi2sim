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
#include <memory>
#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>

#include "String.h"


namespace misc
{

class CommandLine;


/// Base class used for configurable modules.
class CommandLineConfig
{
public:
	/// Register options associated with this configurable module. This
	/// function is invoked automatically when a configurable module is
	/// attached to object \a command_line with a call to
	/// \c CommandLine::AddConfig().
	virtual void Register(CommandLine &command_line) { }

	/// Process command-line options specific for this configurable module.
	/// This function is automatically invoked after all options in the
	/// command line have been read.
	virtual void Process() { }
};


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
		TypeEnum
	};

private:

	Type type;
	std::string name;
	std::string help_name;
	int num_args;
	std::string help;

	// True if option was specified by the user
	bool present;

	// True if option is incompatible with any other option
	bool incompatible;

public:

	/// Constructor
	///
	/// \param name Option name. It must start with a double dash
	///        (<tt>--</tt>). Only the first token of this string is
	///        considered as the option name. The complete string will be
	///        used to display the help message.
	/// \param num_args Number of additional arguments required by the
	///        command-line option.
	/// \param help String to print as a help message related with the
	///        option.
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

	/// Read value of the option from the command-line option given in \a
	/// argc and \a argv. The element in \a argv pointed to by \a index
	/// should be the option itself, optionally followed by its arguments in
	/// the following positions of \a argv. If \a argv does not have enough
	/// arguments as per what the option expects, an error message will
	/// terminate execution.
	virtual void Read(int argc, char **argv, int index) = 0;

	/// Dump help about command line option into output stream
	void Help(std::ostream &os = std::cout) const;
};


/// Command-line option taking a string as an argument
class CommandLineOptionString : public CommandLineOption
{
	std::string *var;
public:

	/// Constructor
	CommandLineOptionString(const std::string &name, std::string *var,
			const std::string &help)
			: CommandLineOption(TypeString, name, 1, help),
			var(var) { }
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(int argc, char **argv, int index);
};


/// Command-line option taking a 32-bit integer as an argument
class CommandLineOptionInt32 : public CommandLineOption
{
	int *var;
public:
	/// Constructor
	CommandLineOptionInt32(const std::string &name, int *var,
			const std::string &help)
		: CommandLineOption(TypeInt32, name, 1, help),
		var(var) { }
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(int argc, char **argv, int index);
};


/// Command-line option taking a 64-bit integer as an argument
class CommandLineOptionInt64 : public CommandLineOption
{
	long long *var;
public:
	/// Constructor
	CommandLineOptionInt64(const std::string &name, long long *var,
			const std::string &help)
		: CommandLineOption(TypeInt64, name, 1, help),
		var(var) { }
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(int argc, char **argv, int index);
};


/// Command-line option taking an identifier as an argument
class CommandLineOptionEnum : public CommandLineOption
{
	int *var;
	const StringMap &map;
public:
	/// Constructor
	CommandLineOptionEnum(const std::string &name, int *var,
			const StringMap &map, const std::string &help)
		: CommandLineOption(TypeEnum, name, 1, help),
		var(var), map(map) { }
	
	/// Read option from command line. See CommandLineOption::Read()
	void Read(int argc, char **argv, int index);
};


/// Command-line option taking no argument
class CommandLineOptionBool : public CommandLineOption
{
	bool *var;
public:

	/// Constructor
	CommandLineOptionBool(const std::string &name, bool *var,
			const std::string &help)
			: CommandLineOption(TypeBool, name, 0, help),
			var(var) { }
	
	/// Read option from command line. See CommandLineOption::Read().
	void Read(int argc, char **argv, int index);
};


/// Class representing the command-line used to invoke Multi2Sim.
class CommandLine
{
	// Original arguments
	int argc;
	char **argv;

	// Error message to show when finding invalid options
	std::string error_message;

	// Program name, originally found in argv[0]
	std::string program_name;

	// Arguments left after removing program name and command-line options
	std::vector<std::string> args;

	// Hash table of registered command-line options.
	std::unordered_map<std::string, CommandLineOption *> option_table;

	// Sequential list of registered command-line options. We keep the
	// sequential order of registration to show an organized help message.
	std::vector<std::unique_ptr<CommandLineOption>> option_list;

	// Register a newly created command-line option. The dynamically created
	// option will be taken ownership from by a unique_ptr that will be
	// automatically freed.
	void Register(CommandLineOption *option);

	// Set to true when function Process() is invoked.
	bool processed;

	// Configure modules
	std::list<CommandLineConfig *> configs;

	// Command-line option indicating whether to use C++
	bool use_cpp;

	// Show help message with command-line options
	bool show_help;

	// Help message header
	std::string help;

public:

	/// Constructor to be invoked in the beginning of the execution of
	/// function \a main(), passing its \a argc and \a argv arguments
	/// directly for post-processing.
	CommandLine(int argc, char **argv) :
			argc(argc), argv(argv), processed(false),
			use_cpp(false), show_help(false)
	{ }

	/// Set the error message to be displayed when an invalid option is
	/// found in the command line during the execution of function
	/// Process().
	void setErrorMessage(const std::string &msg) {
		assert(!processed);
		error_message = msg;
	}

	/// Set the help message to be displayed when option '--help' is
	/// present, preceding the help for all command-line options.
	void setHelp(const std::string &msg) {
		assert(!processed);
		help = msg;
	}

	/// Return the program name used to invoke Multi2Sim. The command line
	/// must have been processed with a call to Process().
	const std::string &getProgramName() const {
		assert(processed);
		return program_name;
	}

	/// Return the number of arguments left after getting rid of the
	/// command-line options, and the program name. The command-line must
	/// have been processed with a call to Process().
	int getNumArguments() const {
		assert(processed);
		return args.size();
	}

	/// Return the argument with a specific \a index. A value of 0 for \a
	/// index specifies the first argument without considering all
	/// command-line options or the program name. The command line must have
	/// been processed with a call to Process().
	const std::string &getArgument(int index) const {
		assert(index >= 0 && index < (int) args.size());
		return args[index];
	}

	/// Return a constant reference to the internal vector containing the
	/// list of command-line arguments, after having gotten rid of all
	/// command-line options. The command line must have been processed with
	/// a call to Process().
	const std::vector<std::string> &getArguments() const {
		assert(processed);
		return args;
	}

	/// Return \c true if the user specified command-line option \c --cpp.
	/// This option is temporarily used to activate the C++ version of
	/// Multi2Sim.
	bool getUseCpp() const { assert(processed); return use_cpp; }

	/// Register a command-line option that takes no argument. If present,
	/// variable \a var will be set to \a true. If not, to \a false.
	void Register(const std::string &name, bool &var,
			const std::string &help) {
		Register(new CommandLineOptionBool(name, &var, help));
	}

	/// Register a command-line option taking a string as an argument. If
	/// present, variable \a var takes the value of the argument. If not
	/// present, the original content of \a var is not modified. The value
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
	void RegisterString(const std::string &name, std::string &var,
			const std::string &help) {
		Register(new CommandLineOptionString(name, &var, help));
	}

	/// Same as RegisterString(), but taking an unsigned 32-bit integer as the
	/// type of the command-line option.
	void RegisterInt32(const std::string &name, int &var,
			const std::string &help) {
		Register(new CommandLineOptionInt32(name, &var, help));
	}

	/// Same as RegisterString(), but taking a signed 64-bit integer as the
	/// type of the command-line option.
	void RegisterUInt32(const std::string &name, unsigned &var,
			const std::string &help) {
		Register(new CommandLineOptionInt32(name,
				(int *) &var, help));
	}

	/// Same as RegisterString(), but taking an unsigned 64-bit integer as
	/// the type of the command-line option.
	void RegisterInt64(const std::string &name, long long &var,
			const std::string &help) {
		Register(new CommandLineOptionInt64(name, &var, help));
	}

	/// Same as RegisterString(), but taking a signed 32-bit integer as the
	/// type of the command-line option.
	void RegisterUInt64(const std::string &name, unsigned long long &var,
			const std::string &help) {
		Register(new CommandLineOptionInt64(name,
				(long long *) &var, help));
	}

	/// Register an option that can take values from the string map provided
	/// in argument \a map.
	void RegisterEnum(const std::string &name, int &var,
			const StringMap &map, const std::string &help) {
		Register(new CommandLineOptionEnum(name, &var, map, help));
	}

	/// Register a boolean option with no additional arguments. If the
	/// option is present, a value of \c True is assumed, and \c False if
	/// the option is not provided by the user.
	void RegisterBool(const std::string &name, bool &var,
			const std::string &help) {
		Register(new CommandLineOptionBool(name, &var, help));
	}

	/// Make option \a name incompatible with any other option.
	void setIncompatible(const std::string &name);
	
	/// Dump help for all registered command-line options.
	void Help(std::ostream &os = std::cout);

	/// Add a configurable module to the command line object. A configurable
	/// module is an object derived from class CommandLineConfig with
	/// virtual functions \c Register() and \c Process(). Function \c
	/// Register() will be invoked on the configurable module during the
	/// call to AddConfig(). Function \c Process() will be invoked after a
	/// call to CommandLine::Process().
	void AddConfig(CommandLineConfig &config);

	/// Process command line.
	/// Optional argument \a fatal_on_bad_option specified whether a fatal
	/// message should be issued when a command-line option is found that is
	/// not recognized. If this value is \c false and a bad command-line
	/// option is found, this error is indicated by the return value: \c
	/// true if no error, and \c false on error.
	bool Process(bool fatal_on_bad_option = true);
};


}  // namespace misc

#endif

