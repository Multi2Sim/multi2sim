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

#ifndef LIB_CPP_ESIM_ACTION_H
#define LIB_CPP_ESIM_ACTION_H

#include <memory>
#include <vector>

#include <lib/cpp/Debug.h>


namespace esim
{

// Forward declarations
class EventType;
class EventFrame;


struct EventInfo
{
	std::string name;
	long long time;

	EventInfo(const std::string &name, long long time)
			:
			name(name),
			time(time)
	{
	}
};


class EventChecks
{
	// Unique instance of this class
	static std::unique_ptr<EventChecks> instance;

	/// Debugger
	static misc::Debug debug;

	// Vector of events that should happen.
	std::vector<EventInfo> checks;

	// Vector of events that did happen.
	std::vector<EventInfo> events;

	// Private constructor, used internally to instantiate a singleton. Use
	// a call to getInstance() instead.
	EventChecks();

public:

	/// Obtain the instancesvn  of the event-driven simulator singleton.
	static EventChecks *getInstance();

	/// Event handler that can be registerd with esim that will add
	/// whatever events it's called for to the \a events vector.
	static void ActionEventHandler(EventType *, EventFrame *);

	/// Event handler that calls the \a DoChecks method.
	static void DoChecksHandler(EventType *, EventFrame *);

	/// Add a check that should be made.
	void AddCheck(const EventInfo &event) { checks.push_back(event); }

	/// Add event info for an event that happened.
	void AddEvent(const EventInfo &event) { events.push_back(event); }

	/// Check that events were scheduled when they should have been.  This
	/// should be called after simulation is done.
	void DoChecks();
};

}  // namespace esim

#endif
