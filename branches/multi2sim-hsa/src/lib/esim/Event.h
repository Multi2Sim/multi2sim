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

	// Current number of scheduled events of this type
	int num_in_flight = 0;

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

	/// Returns whether or not there are currently any events of this type
	/// scheduled.  An event type is not considered to be in flight any
	/// more when the event handler for its last instance is being run.
	bool isInFlight() const { return num_in_flight != 0; }

	/// Increase the number of in-flight events of this type by one.
	void incInFlight() { num_in_flight++; }

	/// Decrease the number of in-flight events of this type by one.
	void decInFlight() { num_in_flight--; }
};


/// This class represents data associated with an event.
class EventFrame
{

	// Parent frame is this event was invoked as a call
	std::shared_ptr<EventFrame> parent_frame;

	// Event type to invoke upon return, or null if there is no parent
	// event
	EventType *return_event_type = nullptr;

	// Flag indicating whether the frame is currently suspended in a queue
	bool in_queue = false;

	// Pointer to next frames in a waiting queue, or null if the event
	// frame is not suspended in a queue.
	std::shared_ptr<EventFrame> next;

	// Event type scheduled when the frame is woken up from a queue
	EventType *wakeup_event_type = nullptr;

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

	/// Return whether the event frame is currently suspended in an
	/// event queue.
	bool isInQueue() const { return in_queue; }

	/// Mark the event frame as suspended in an event queue. This function
	/// should only be used internally.
	void setInQueue(bool in_queue) { this->in_queue = in_queue; }

	/// Return the next event frame suspended in an event queue. Only used
	/// internally by class Queue.
	std::shared_ptr<EventFrame> getNext() { return next; }

	/// Set the next event frame suspended in an event queue. Only used
	/// internally by class Queue;
	void setNext(std::shared_ptr<EventFrame> next) { this->next = next; }

	/// Return the event type to schedule when the frame is woken up from
	/// a suspension queue.
	EventType *getWakeupEventType() const { return wakeup_event_type; }

	/// When an event is suspended in a queue, set the event to schedule
	/// when the frame is woken up.
	void setWakeupEventType(EventType *wakeup_event_type)
	{
		this->wakeup_event_type = wakeup_event_type;
	}
};


/// Class representing a scheduled event. This class should not be instantiated
/// directly from outside of this library.
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

	// Comparison lambda, used as the comparison function in the event
	// min-heap of the simulation engine.
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

