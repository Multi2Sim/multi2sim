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

std::unique_ptr<Actions> Actions::instance;

Actions::Actions()
{
	// Get the esim engine instance and register events with it.
	esim::Engine *esim = esim::Engine::getInstance();
	System::ACTION_REQUEST = esim->RegisterEventType("ACTION_REQUEST",
			ActionRequestHandler, System::DRAM_DOMAIN);
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
}


void Actions::ParseActionRequest(const std::vector<std::string> &tokens)
{
	// Get the cycle of the action.
	long long cycle = misc::StringToInt64(tokens[0]);

	// Create the request object that will be inserted into the queue.
	std::shared_ptr<Request> request = std::make_shared<Request>();

	// Set the request's type.
	if (tokens[2] == "READ")
		request->setType(RequestTypeRead);
	else if (tokens[2] == "WRITE")
		request->setType(RequestTypeWrite);
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


void Actions::ActionRequestHandler(esim::EventType *type,
		esim::EventFrame *frame)
{
	ActionRequestFrame *action_frame = dynamic_cast<ActionRequestFrame *>(frame);
	System *dram = System::getInstance();
	dram->AddRequest(action_frame->request);
}

}  // namespace dram
