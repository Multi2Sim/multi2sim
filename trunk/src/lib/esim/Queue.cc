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

#include <lib/cpp/Error.h>

#include "Engine.h"
#include "Queue.h"


namespace esim
{

void Queue::PushBack(std::shared_ptr<Frame> frame)
{
	// Mark frame as inserted
	assert(!frame->in_queue);
	assert(!frame->next);
	frame->in_queue = true;

	// Add to back of the list
	if (head == nullptr && tail == nullptr)
	{
		head = frame;
		tail = frame;
	}
	else
	{
		assert(head != nullptr && tail != nullptr);
		assert(tail->next == nullptr);
		tail->next = frame;
		tail = frame;
	}
}


void Queue::PushFront(std::shared_ptr<Frame> frame)
{
	// Mark frame as inserted
	assert(!frame->in_queue);
	assert(!frame->next);
	frame->in_queue = true;

	// Add to front of the list
	if (head == nullptr && tail == nullptr)
	{
		head = frame;
		tail = frame;
	}
	else
	{
		assert(head != nullptr && tail != nullptr);
		assert(!tail->next);
		frame->next = head;
		head = frame;
	}
}


std::shared_ptr<Frame> Queue::PopFront()
{
	// Check if queue is empty
	if (head == nullptr)
	{
		assert(head == nullptr && tail == nullptr);
		return nullptr;
	}

	// Extract element from the head
	std::shared_ptr<Frame> frame = head;
	if (head == tail)
	{
		head = nullptr;
		tail = nullptr;
	}
	else
	{
		head = head->next;
	}

	// Mark as extracted
	frame->next = nullptr;
	frame->in_queue = false;
	return frame;
}


void Queue::Wait(Event *event, bool priority)
{
	// Get current event frame
	Engine *engine = Engine::getInstance();
	std::shared_ptr<Frame> current_frame = engine->getCurrentFrame();
	
	// This function must be invoked within an event handler
	if (current_frame == nullptr)
		throw misc::Panic("Function cannot be invoked outside of "
				"an event handler");

	// The caller must have not scheduled another event for this frame
	if (current_frame->in_heap)
		throw misc::Panic("Cannot suspend the current event chain in "
				"a queue after having scheduled another "
				"event with Next().");

	// The event type must be valid
	if (event == nullptr)
		throw misc::Panic("Cannot suspend an event with a null "
				"wakeup event type");

	// Add event frame to the queue
	assert(!current_frame->wakeup_event);
	current_frame->wakeup_event = event;
	if (priority)
		PushFront(current_frame);
	else
		PushBack(current_frame);
}


void Queue::WakeupOne()
{
	// Queue must have at least one event in it
	if (isEmpty())
		throw misc::Panic("Queue is empty");

	// Get event frame from the head
	std::shared_ptr<Frame> frame = PopFront();

	// Get event to schedule
	Event *event = frame->wakeup_event;
	frame->wakeup_event = nullptr;
	assert(event);

	// Schedule event
	Engine *engine = Engine::getInstance();
	engine->Schedule(event, frame);
}


void Queue::WakeupAll()
{
	// Keep waking up events from the head
	while (!isEmpty())
		WakeupOne();
}

}  // namespace esim
