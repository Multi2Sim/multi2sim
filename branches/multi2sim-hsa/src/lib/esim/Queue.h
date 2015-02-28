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

#ifndef LIB_CPP_ESIM_QUEUE_H
#define LIB_CPP_ESIM_QUEUE_H

#include <cassert>
#include <memory>

#include "Event.h"


namespace esim
{

/// Event queue used to suspend a chain of events. An event handler can suspend
/// the event chain in a queue using a call to EventFrame::Wait(). When the
/// queue receives a wakeup signal, it will schedule another event with the
/// event frame.
class Queue
{
	// Head pointer
	std::shared_ptr<EventFrame> head;

	// Tail pointer
	std::shared_ptr<EventFrame> tail;

	// Remove an event frame from the queue.
	std::shared_ptr<EventFrame> PopFront();

	// Add an event frame to the queue
	void PushBack(std::shared_ptr<EventFrame> event_frame);

public:

	/// Suspend the current event chain in the queue. This function should
	/// only be invoked in the body of an event handler.
	///
	/// \param event_type
	///	Type of event to schedule when the queue receives a wakeup
	///	signal. This event will be scheduled using the current event
	///	frame. The event type cannot be `nullptr`.
	void Wait(EventType *event_type);

	/// Wake the last recently suspended event frame, scheduling the wakeup
	/// event for the current cycle. The queue must have at least one
	/// suspended event in it.
	void WakeupOne();

	/// Wake up all events in the queue, in the same order in which they
	/// were suspended, scheduling their wakeup events for the current
	/// cycle.
	void WakeupAll();

	/// Return `true` if the queue has no suspended events in it.
	bool isEmpty() { return head == nullptr; }
};

}  // namespace esim

#endif

