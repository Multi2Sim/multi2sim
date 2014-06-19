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
class EventFrame;
class EventType;
class FrequencyDomain;


/// Event handler function prototype
typedef void (*EventHandler)(EventType *, EventFrame *);


/// Event type used for event registrations. This class should not be
/// instantiated directly by the user. The user should call
/// Engine::RegisterEventType instead.
class EventType
{
	// Name of the event
	std::string name;

	// Handler function
	EventHandler handler;

	// Frequency domain
	FrequencyDomain *frequency_domain;

public:

	/// Constructor
	EventType(const std::string &name,
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
};


/// This class represents data associated with an event.
class EventFrame
{

	// Parent frame is this event was invoked as a call
	std::shared_ptr<EventFrame> parent_frame;

	// Event type to invoke upon return, or null if there is no parent
	// event
	EventType *return_event_type = nullptr;

public:

	/// A virtual destructor guarantees that child classes are destructed
	/// properly when the last reference to this frame is destroyed.
	virtual ~EventFrame() { }

	/// Set the parent frame
	void setParentFrame(const std::shared_ptr<EventFrame> &parent_frame)
	{
		this->parent_frame = parent_frame;
	}

	/// Set the return event
	void setReturnEventType(EventType *return_event_type)
	{
		this->return_event_type = return_event_type;
	}

	/// Get parent frame
	const std::shared_ptr<EventFrame> &getParentFrame() const
	{
		return parent_frame;
	}

	/// Get the return event type
	EventType *getReturnEventType() const { return return_event_type; }
};


/// Class representing a scheduled event. This class should not be instantiated
/// directly by the user. Instead, it should be created implicitly with a call
/// to Engine::ScheduleEvent().
class Event
{
	// Event type
	EventType *type;

	// Data associated with the event
	std::shared_ptr<EventFrame> frame;

	// Time in picoseconds for when the event is scheduled
	long long time;

	// Cycles after which the event will repeat (0 = no repeat)
	int period;

public:

	// Comparison lambda
	struct CompareUniquePtrs
	{
		bool operator()(const std::unique_ptr<Event> &lhs,
				const std::unique_ptr<Event> &rhs) const
		{
			return lhs->time > rhs->time;
		}
	};

	/// Constructor
	Event(EventType *type,
			const std::shared_ptr<EventFrame> &frame,
			long long time,
			int period)
			:
			type(type),
			frame(frame),
			time(time),
			period(period)
	{
	}
	
	/// Return the event type
	EventType *getType() const { return type; }

	/// Return the frame associated with the event
	const std::shared_ptr<EventFrame> &getFrame() const { return frame; }

	/// Return the time in picoseconds for which the event is scheduled.
	long long getTime() const { return time; }

	/// Return the number of cycles after which the event is rescheduled.
	/// This number of cycles is given with respect to the event's frequency
	/// domain.
	int getPeriod() const { return period; }
};


}  // namespace esim

#endif

