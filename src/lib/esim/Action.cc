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


void EventChecks::ActionEventHandler(EventType *event, EventFrame *frame)
{
	// Add the event name and the current time to the list of events
	// that have happened.
	EventChecks *checks = EventChecks::getInstance();
	Engine *esim = Engine::getInstance();
	EventInfo new_event(event->getName(), esim->getTime());
	checks->AddEvent(new_event);
}


void EventChecks::DoChecksHandler(EventType *event, EventFrame *frame)
{
	EventChecks *checks = EventChecks::getInstance();
	checks->DoChecks();
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
			if (checks[c_num].name != events[e_num].name)
				continue;

			// Move to the next check if times don't match.
			if (checks[c_num].time != events[e_num].time)
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
					events[e_num].name.c_str(),
					events[e_num].time);
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
		for (auto &check : checks)
		{
			debug << misc::fmt("\t%s at %lld\n",
					check.name.c_str(), check.time);
		}
	}

	if (!passed)
		debug << "Failed.\n";
}


}  // namespace esim
