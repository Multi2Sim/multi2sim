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
#include <string>
#include <unordered_map>
#include <vector>


namespace misc
{

class CommandLineOption
{
public:

	enum Type
	{
		TypeInvalid = 0,
		TypeBool,
		TypeString,
		TypeInt32,
		TypeInt64
	};

private:

	Type type;
	std::string name;
	std::string help;
	bool present;

public:

	/// Constructor
	///
	/// \param name Option name. It must start with a double dash
	///        (<tt>--</tt>)
	/// \param help String to print as a help message related with the
	///        option.
	CommandLineOption(Type type, const std::string &name,
			const std::string &help) :
			type(type), name(name), help(help), present(false)
		{ }

	/// Virtual destructor to make class polymorphic.
	virtual ~CommandLineOption() { }

	/// Return the type of the command-line option. This type determines the
	/// actual instantiated subclass.
	Type getType() { return type; }

	// Return the option name
	const std::string &getName() { return name; }

	/// Return \a true if the command-line option was specified by the user.
	bool isPresent() { return present; }

	/// Dump help message related with this option
	void DumpHelp(std::ostream &os);
};


/// Command-line option taking no argument
class CommandLineOptionBool : public CommandLineOption
{
	bool *var;
public:

	/// Constructor
	CommandLineOptionBool(const std::string &name, bool *var,
			const std::string &help)
			: CommandLineOption(TypeBool, name, help),
			var(var) { }
};


/// Command-line option taking a string as an argument
class CommandLineOptionString : public CommandLineOption
{
	std::string *var;
public:

	/// Constructor
	CommandLineOptionString(const std::string &name, std::string *var,
			const std::string &help)
			: CommandLineOption(TypeString, name, help),
			var(var) { }
};


/// Command-line option taking a 32-bit integer as an argument
class CommandLineOptionInt32 : public CommandLineOption
{
	int *var;
public:
	/// Constructor
	CommandLineOptionInt32(const std::string &name, int *var,
			const std::string &help)
		: CommandLineOption(TypeInt32, name, help),
		var(var) { }
};


/// Command-line option taking a 64-bit integer as an argument
class CommandLineOptionInt64 : public CommandLineOption
{
	long long *var;
public:
	/// Constructor
	CommandLineOptionInt64(const std::string &name, long long *var,
			const std::string &help)
		: CommandLineOption(TypeInt64, name, help),
		var(var) { }
};


/// Class representing the command-line used to invoke Multi2Sim.
class CommandLine
{
	// Arguments left after removing program name and command-line options
	std::vector<std::string> args;

	// Hash table of command-line options
	std::unordered_map<std::string, CommandLineOption *> option_table;

	// Sequential list of command-line options. We keep the sequential order
	// of registration to show an organized help message
	std::vector<std::unique_ptr<CommandLineOption>> option_list;

	// Register a newly created command-line option. The dynamically created
	// option will be taken ownership from by a unique_ptr that will be
	// automatically freed.
	void Register(CommandLineOption *option);

public:

	/// Constructor to be invoked in the beginning of the execution of
	/// function \a main(), passing its \a argc and \a argv arguments
	/// directly for post-processing.
	CommandLine(int argc, char **argv);

	/// Return the number of arguments left after getting rid of the
	/// command-line options, and the program name.
	int getNumArguments() const {
		return args.size();
	}

	/// Return the program name used to invoke Multi2Sim
	const std::string &getProgramName() const {
		assert(args.size() > 0);
		return args[0];
	}

	/// Return the argument with a specific \a index. A value of 0 for \a
	/// index specifies the first argument without considering all
	/// command-line options or the program name.
	const std::string &getArgument(int index) const {
		assert(index >= 0 && index < (int) args.size());
		return args[index];
	}

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

	/// Dump help for all registered command-line options.
	void Help(std::ostream &os);
};


}  // namespace misc

#endif

