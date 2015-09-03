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

#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "Action.h"
#include "Address.h"
#include "System.h"
#include "Request.h"

namespace dram
{

misc::Debug Actions::debug;

std::unique_ptr<Actions> Actions::instance;


Actions::Actions()
{
	// Get the esim engine instance and register events with it.
	esim::Engine *esim = esim::Engine::getInstance();
	System::ACTION_REQUEST = esim->RegisterEvent("ACTION_REQUEST",
			ActionRequestHandler, System::frequency_domain);

	// Set the debugger
	debug.setPath("stderr");
	debug.setPrefix("[dram-actions]");
}


Actions *Actions::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Actions());
	return instance.get();
}


void Actions::ParseConfiguration(misc::IniFile &config)
{

	// Find and parse each action.  Start at 'Action[0]' and continue
	// sequentially until one isn't found.
	int i = 0;
	while (1)
	{
		// Get the next action.
		std::string current_action = config.ReadString("Actions",
				misc::fmt("Action[%d]", i));

		// Check that the action exists.
		if (current_action == "")
			break;

		// Parse the action.
		ParseAction(current_action);

		// Increment the action number.
		i++;
	}
}


void Actions::ParseAction(const std::string &line)
{
	// Parse the configuration line to a list of tokens.
	std::vector<std::string> tokens;
	misc::StringTokenize(line, tokens);

	// Check the action type and parse it accordingly.
	if (tokens[1] == "Request")
		ParseActionRequest(tokens);
	else if (tokens[1] == "Decode")
		ParseActionDecode(tokens);
	else if (tokens[1] == "CheckCommand")
		ParseActionCheck(tokens);
}


void Actions::ParseActionRequest(const std::vector<std::string> &tokens)
{
	// Get the cycle of the action.
	long long cycle = misc::StringToInt64(tokens[0]);

	// Create the request object that will be inserted into the queue.
	std::shared_ptr<Request> request = std::make_shared<Request>();

	// Set the request's type.
	if (tokens[2] == "READ")
		request->setType(RequestRead);
	else if (tokens[2] == "WRITE")
		request->setType(RequestWrite);
	else
		// Invalid request type
		return;

	// Set the request's address
	request->setEncodedAddress(misc::StringToInt64(tokens[3]));

	// Schedule an event to insert it at the specified cycle.
	esim::Engine *esim = esim::Engine::getInstance();
	auto request_frame = std::make_shared<ActionRequestFrame>(request);
	esim->Call(System::ACTION_REQUEST, request_frame, nullptr, cycle);
}

void Actions::ParseActionDecode(const std::vector<std::string> &tokens)
{
	// Get the address to decode and create an Address object for it.  The
	// constructor of the address object will decode it.
	Address address(misc::StringToInt64(tokens[2]));
	System::debug << address;
}


void Actions::ParseActionCheck(const std::vector<std::string> &tokens)
{
	// Get the cycle of the action.
	long long cycle = misc::StringToInt64(tokens[0]);

	// Get the type of command to check, defaulting to invalid.
	CommandType type;
	if (tokens[2] == "PRECHARGE")
		type = CommandPrecharge;
	else if (tokens[2] == "ACTIVATE")
		type = CommandActivate;
	else if (tokens[2] == "READ")
		type = CommandRead;
	else if (tokens[2] == "WRITE")
		type = CommandWrite;
	else
		// Invalid request type
		return;

	// Create the check.
	addCheck(type, cycle);
}


void Actions::ActionRequestHandler(esim::Event *type, esim::Frame *frame)
{
	ActionRequestFrame *action_frame = misc::cast<ActionRequestFrame *>(frame);
	System *dram = System::getInstance();
	dram->AddRequest(action_frame->request);
}


void Actions::addCheck(CommandType type, long long cycle)
{
	checks.emplace_back(type, cycle);
}


void Actions::addCommand(Command *command, long long cycle)
{
	commands.emplace_back(command->getType(), cycle);
}


void Actions::DoChecks()
{
	bool passed = true;

	// If no checks were sceduled to be made, don't do anything.
	if (checks.size() == 0)
		return;

	// To ensure that commands happened when they should, we iterate
	// through them and search for a corresponding check.  If a check is
	// found, the command should have happened and the check are removed.
	// This process is considered failed if:
	// There are checks leftover at the end.
	// - A command that should've happened didn't.
	// There is no check for a command.
	// - An command happened that shouldn't have.

	// Iterate through each command that happened.
	for (unsigned e_num = 0; e_num < commands.size(); e_num++)
	{
		bool found = false;

		// For the command, search for it's check.
		for (unsigned c_num = 0; c_num < checks.size(); c_num++)
		{
			// Move to next check if types don't match.
			if (checks[c_num].getType() !=
					commands[e_num].getType())
				continue;

			// Move to the next check if cycles don't match.
			if (checks[c_num].getCycle()!=
					commands[e_num].getCycle())
				continue;

			// This check matches the command, so remove the check.
			found = true;
			checks.erase(checks.begin() + c_num);
		}

		// Print out info about the command if it didn't pass.
		if (!found)
		{
			passed = false;
			debug << "Command has no corresponding check.\n";
			debug << misc::fmt("\t\t%s at %lld\n",
					commands[e_num].getTypeString().c_str(),
					commands[e_num].getCycle());
		}
	}

	// All commands have been processed, see if we have any checks left.
	if (checks.size() > 0)
	{
		passed = false;

		// Print out info about the missing commands.
		debug << misc::fmt("Extra checks left over; %d commands "
					"weren't scheduled.\n",
					(int) checks.size());
		for (auto &check : checks)
		{
			debug << misc::fmt("\t\t%s at %lld\n",
					check.getTypeString().c_str(),
					check.getCycle());
		}
	}

	// If the test didn't pass, print it out.
	// Don't print anything if tests did pass.
	if (!passed)
		debug << "Failed.\n";
}


void Actions::dump(std::ostream &os) const
{
	// Print header
	os << "\n\n--------------------\n\n";
	os << "Dumping DRAM Actions log\n";

	// Print checks that were scheduled to be made.
	os << misc::fmt("%d checks scheduled\nCheck dump:\n",
			(int) checks.size());
	for (auto &check : checks)
		os << check;

	// Print commands that actually happened.
	os << misc::fmt("%d commands happened\nCommand dump:\n",
			(int) commands.size());
	for (auto &command : commands)
		os << command;
}


void CommandInfo::dump(std::ostream &os) const
{
	// Print type and cycle of command
	os << misc::fmt("\t%s at %lld\n",
		getTypeString().c_str(), cycle);
}


}  // namespace dram
