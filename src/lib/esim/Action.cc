/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/cpp/String.h>

#include "Action.h"
#include "Event.h"
#include "Engine.h"


namespace esim
{

misc::Debug EventChecks::debug;

std::unique_ptr<EventChecks> EventChecks::instance;


void EsimTestLoop(std::string &config_path)
{
	Engine *esim = Engine::getInstance();

	// Capture errors from parsing config and running esim
	try
	{
		esim->ParseConfiguration(config_path);
		for (int i = 0; i < 1000; i++)
		{
			esim->ProcessEvents();
		}
	}
	catch (std::runtime_error &e)
	{
		misc::fatal("%s", e.what());
	}
	catch (std::logic_error &e)
	{
		misc::panic("%s", e.what());
	}

	// Process checks.
	EventChecks::getInstance()->DoChecks();
}


Action::Action(const std::string &line)
{
	// Parse the configuration line to a list of tokens.
	std::vector<std::string> tokens;
	misc::StringTokenize(line, tokens);

	// Every action type has 3 arguments, so assert that we have 3 tokens.
	if (tokens.size() != 3)
		throw std::logic_error(misc::fmt("Esim action malformed.\n"
				"Expected 3 tokens, but instead received %d.\n"
				">\t%s",
				(int) tokens.size(),
				line.c_str()));

	// Get the action type.
	if (tokens.at(0) == "FrequencyDomain")
		type = ActionTypeDomainRegistration;
	else if (tokens.at(0) == "EventType")
		type = ActionTypeEventRegistration;
	else if (tokens.at(0) == "Event")
		type = ActionTypeEventSchedule;
	else if (tokens.at(0) == "CheckEvent")
		type = ActionTypeCheck;
	else
		throw std::logic_error(misc::fmt("Esim action malformed.\n"
				"Invalid action type '%s'.\n"
				">\t%s", tokens.at(0).c_str(), line.c_str()));

	// Store the action specific information.
	if (type == ActionTypeDomainRegistration)
	{
		domain_registration_name = tokens.at(1);
		domain_registration_frequency = misc::StringToInt(tokens.at(2));
	}
	else if (type == ActionTypeEventRegistration)
	{
		event_registration_name = tokens.at(1);
		event_registration_domain = tokens.at(2);
	}
	else if (type == ActionTypeEventSchedule)
	{
		event_schedule_name = tokens.at(1);
		event_schedule_cycle = misc::StringToInt(tokens.at(2));
	}
	else if (type == ActionTypeCheck)
	{
		check_name = tokens.at(1);
		check_time = misc::StringToInt64(tokens.at(2));
	}
}


EventChecks::EventChecks()
{
	debug.setPath("stderr");
	debug.setPrefix("[esim-test]");
}


EventChecks *EventChecks::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();
	
	// Create instance
	instance.reset(new EventChecks());
	return instance.get();
}


void EventChecks::EventCheckHandler(EventType *event, EventFrame *frame)
{
	// Add the event name and the current time to the list of events
	// that have happened.
	EventChecks *checks = EventChecks::getInstance();
	Engine *esim = Engine::getInstance();
	EventInfo new_event(event->getName(), esim->getTime());
	checks->AddEvent(new_event);
}


void EventChecks::DoChecks()
{
	bool passed = true;

	// To ensure that events happened when they should, we iterate through
	// them and search for a corresponding check.  If a check is found,
	// the event should have happened and the check is removed.
	// This process is considered failed if:
	// There are checks leftover at the end.
	// - An event that should've happened didn't.
	// There is no check for an event.
	// - An event happened that shouldn't have.

	// Iterate through each event that happened.
	for (unsigned e_num = 0; e_num < events.size(); e_num++)
	{
		bool found = false;

		// For the event, search for it's check.
		for (unsigned c_num = 0; c_num < checks.size(); c_num++)
		{
			// Move to next check if names don't match.
			if (checks.at(c_num).name != events.at(e_num).name)
				continue;

			// Move to the next check if times don't match.
			if (checks.at(c_num).time != events.at(e_num).time)
				continue;

			// This check matches the event, so remove the check.
			found = true;
			checks.erase(checks.begin() + c_num);
		}

		// Print out info about the event if it didn't pass.
		if (!found)
		{
			passed = false;
			debug << "Event has no corresponding check.\n";
			debug << misc::fmt("\t%s at %lld\n",
					events.at(e_num).name.c_str(),
					events.at(e_num).time);
		}
	}

	// All events processed, see if we have any checks left.
	if (checks.size() > 0)
	{
		passed = false;

		// Print out info about the missing events.
		debug << misc::fmt("Extra checks left over; %d events weren't "
					"scheduled.\n",
					(int) checks.size());
		for (auto check : checks)
		{
			debug << misc::fmt("\t%s at %lld\n",
					check.name.c_str(), check.time);
		}
	}

	if (!passed)
		debug << "Failed.\n";
}


}  // namespace esim
