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
/// the event chain in a queue using a call to Frame::Wait(). When the
/// queue receives a wakeup signal, it will schedule another event with the
/// event frame.
class Queue
{
	// Head pointer
	std::shared_ptr<Frame> head;

	// Tail pointer
	std::shared_ptr<Frame> tail;

	// Remove an event frame from the queue.
	std::shared_ptr<Frame> PopFront();

	// Add an event frame to the tail of the queue
	void PushBack(std::shared_ptr<Frame> frame);

	// Add an event frame to the front of the queue
	void PushFront(std::shared_ptr<Frame> frame);

public:

	/// Suspend the current event chain in the queue. This function should
	/// only be invoked in the body of an event handler.
	///
	/// \param event
	///	Type of event to schedule when the queue receives a wakeup
	///	signal. This event will be scheduled using the current event
	///	frame. The event type cannot be `nullptr`.
	///
	/// \param priority
	///	If set to `true`, this event chain will be suspended at the
	///	head of the queue, making it the first candidate to wake up
	///	in the next call to WakeupOne() or WakeupAll(). Otherwise,
	///	suspend at the tail - this is the default behavior.
	///
	void Wait(Event *event, bool priority = false);

	/// Wake the least recently suspended event frame, scheduling the wakeup
	/// event for the current cycle. The queue must have at least one
	/// suspended event in it.
	void WakeupOne();

	/// Wake up all events in the queue, in the same order in which they
	/// were suspended, scheduling their wakeup events for the current
	/// cycle.
	void WakeupAll();

	/// Return `true` if the queue has no suspended events in it.
	bool isEmpty() const { return head == nullptr; }

	/// Return the frame at the head of the queue, or `nullptr` if the
	/// queue is empty. This is the frame that will wake up first upon a
	/// call to WakeupOne().
	Frame *getHead() const { return head.get(); }

	/// Return the frame at the tail of the queue, or `nullptr` if the
	/// queue is empty.
	Frame *getTail() const { return tail.get(); }
};

}  // namespace esim

#endif

