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
	// Only simulation engine and event queue can access private fields of
	// the frame. This is preferrable to creating public fields or getters/
	// setters, in order to make it clear that user classes derived from
	// this one should not have access to these values.
	friend class Engine;
	friend class Queue;

	// Event associated with this frame when the frame is enqueued in the
	// event heap.
	EventType *event_type = nullptr;

	// Time in picoseconds for when the event is scheduled in the event heap
	long long time = 0;

	// Number of cycles after which the event will repeat, in the frequency
	// domain associated with the event (0 = no repeat)
	int period = 0;

	// Flag indicating whether the frame is currently enqueue in the event
	// heap of the simulation engine
	bool in_heap = false;

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
	
	// Comparison lambda, used as the comparison function in the event
	// min-heap of the simulation engine.
	struct CompareSharedPointers
	{
		bool operator()(const std::shared_ptr<EventFrame> &lhs,
				const std::shared_ptr<EventFrame> &rhs) const
		{
			return lhs->time > rhs->time;
		}
	};

	/// Virtual destructor to make class polymorphic
	virtual ~EventFrame() { }

	/// Return whether the frame is currently suspended in an event queue.
	bool isInQueue() const { return in_queue; }
};


}  // namespace esim

#endif

