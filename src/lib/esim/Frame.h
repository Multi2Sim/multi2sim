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

#ifndef LIB_CPP_ESIM_FRAME_H
#define LIB_CPP_ESIM_FRAME_H

#include <memory>
#include <string>


namespace esim
{

// Forward declarations
class Event;


/// This class represents data associated with an event.
class Frame
{
	// Only simulation engine and event queue can access private fields of
	// the frame. This is preferrable to creating public fields or getters/
	// setters, in order to make it clear that user classes derived from
	// this one should not have access to these values.
	friend class Engine;
	friend class Queue;

	// Event associated with this frame when the frame is enqueued in the
	// event heap.
	Event *event = nullptr;

	// Time in picoseconds for when the event is scheduled in the event heap
	long long time = 0;

	// Every time an event is scheduled, it is assigned a new schdule
	// sequence number. This is used in order to guarantee the strict
	// ordering requirements for frames inserted in the priority queue,
	// where, among events scheduled for the same time, those scheduled
	// first (with lower schedule sequence numbers) will have priority.
	long long schedule_sequence = 0;

	// Number of cycles after which the event will repeat, in the frequency
	// domain associated with the event (0 = no repeat)
	int period = 0;

	// Flag indicating whether the frame is currently enqueue in the event
	// heap of the simulation engine
	bool in_heap = false;

	// Parent frame is this event was invoked as a call
	std::shared_ptr<Frame> parent_frame;

	// Event type to invoke upon return, or null if there is no parent
	// event
	Event *return_event = nullptr;

	// Flag indicating whether the frame is currently suspended in a queue
	bool in_queue = false;

	// Pointer to next frames in a waiting queue, or null if the event
	// frame is not suspended in a queue.
	std::shared_ptr<Frame> next;

	// Event type scheduled when the frame is woken up from a queue
	Event *wakeup_event = nullptr;

public:
	
	// Comparison lambda, used as the comparison function in the event
	// min-heap of the simulation engine.
	struct CompareSharedPointers
	{
		bool operator()(const std::shared_ptr<Frame> &lhs,
				const std::shared_ptr<Frame> &rhs) const
		{
			return lhs->time > rhs->time ||
					(lhs->time == rhs->time &&
					lhs->schedule_sequence >
					rhs->schedule_sequence);
		}
	};

	/// Virtual destructor to make class polymorphic
	virtual ~Frame() { }

	/// Return whether the frame is currently suspended in an event queue.
	bool isInQueue() const { return in_queue; }
	
	/// Return a pointer to next frames in a waiting queue, or null if the
	/// event frame is not suspended in a queue.
	Frame *getNext() const { return next.get(); }
};


}  // namespace esim

#endif

