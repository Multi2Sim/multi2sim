/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef DRAM_ACTION_H
#define DRAM_ACTION_H

#include <memory>
#include <vector>

#include <lib/cpp/IniFile.h>
#include <lib/esim/Event.h>

#include "Command.h"


namespace dram
{

// Forward declarations
class Request;


/// A class to act as a record for a command being scheduled, keeping track of
/// the command's type and the cycle it was scheduled in.
class CommandInfo
{
	CommandType type;
	long long cycle;

public:

	CommandInfo(CommandType type, long long cycle)
			:
			type(type),
			cycle(cycle)
	{
	}

	/// Returns the type of the command.
	CommandType getType() const { return type; }

	/// Returns the type of the command as a string.
	std::string getTypeString() const
	{
		return CommandTypeMapToString[type];
	}

	/// Returns the cycle the command was started in.
	long long getCycle() const { return cycle; }

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const CommandInfo &object)
	{
		object.dump(os);
		return os;
	}
};


class Actions
{
	// Unique instance of this class
	static std::unique_ptr<Actions> instance;

	// Private constructor, used internally to instantiate a singleton. Use
	// a call to getInstance() instead.
	Actions();

	/// Debugger
	static misc::Debug debug;

	// Vector of commands that should be scheduled
	std::vector<CommandInfo> checks;

	// Vector of commands that have been scheduled
	std::vector<CommandInfo> commands;

	// Private methods to help with configuration parsing.
	void ParseAction(const std::string &line);
	void ParseActionRequest(const std::vector<std::string> &tokens);
	void ParseActionDecode(const std::vector<std::string> &tokens);
	void ParseActionCheck(const std::vector<std::string> &tokens);

public:

	/// Obtain the instance of the Actions singleton.
	static Actions *getInstance();

	void ParseConfiguration(misc::IniFile &config);

	/// Event handler that adds a request to the DRAM system.
	static void ActionRequestHandler(esim::Event *, esim::Frame *);

	/// Adds a check for a command type at a cycle to the list of checks to
	/// be made at the end of simulation.
	void addCheck(CommandType type, long long cycle);

	/// Adds a command to the list of commands created during simulation.
	void addCommand(Command *command, long long cycle);

	/// Check that commands happened when they should have been.  This
	/// should be called after simulation is done.
	void DoChecks();

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const Actions &object)
	{
		object.dump(os);
		return os;
	}
};


class ActionRequestFrame : public esim::Frame
{

public:
	ActionRequestFrame(std::shared_ptr<Request> request)
			:
			request(request)
	{
	}

	std::shared_ptr<Request> request;
};

}  // namespace dram

#endif
