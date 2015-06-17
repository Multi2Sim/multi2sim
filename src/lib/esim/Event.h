/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CPP_ESIM_EVENT_H
#define LIB_CPP_ESIM_EVENT_H

#include <memory>
#include <string>


namespace esim
{

// Forward declarations
class Event;
class Frame;
class FrequencyDomain;


/// Event handler function prototype
typedef void (*EventHandler)(Event *, Frame *);


/// Event type used for event registrations. This class should not be
/// instantiated directly by the user. The user should call
/// Engine::RegisterEvent instead.
class Event
{
	// Name of the event
	std::string name;

	// Handler function
	EventHandler handler;

	// Frequency domain
	FrequencyDomain *frequency_domain;

	// Current number of scheduled events of this type
	int num_in_flight = 0;

public:

	/// Constructor
	Event(const std::string &name,
			EventHandler handler,
			FrequencyDomain *frequency_domain = nullptr)
			:
			name(name),
			handler(handler),
			frequency_domain(frequency_domain)
	{
	}

	/// Return the name of the event type
	const std::string &getName() const { return name; }

	/// Return the frequency domain for this event type
	FrequencyDomain *getFrequencyDomain() const
	{
		return frequency_domain;
	}

	/// Return the event handler for this event type
	EventHandler getEventHandler() const { return handler; }

	/// Returns whether or not there are currently any events of this type
	/// scheduled.  An event type is not considered to be in flight any
	/// more when the event handler for its last instance is being run.
	bool isInFlight() const { return num_in_flight != 0; }

	/// Increase the number of in-flight events of this type by one.
	void incInFlight() { num_in_flight++; }

	/// Decrease the number of in-flight events of this type by one.
	void decInFlight() { num_in_flight--; }
};

}  // namespace esim

#endif

