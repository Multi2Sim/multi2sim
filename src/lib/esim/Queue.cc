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

void Queue::PushBack(std::shared_ptr<EventFrame> event_frame)
{
	// Mark frame as inserted
	assert(!event_frame->isInQueue());
	assert(!event_frame->getNext());
	event_frame->setInQueue(true);

	// Add to back of the list
	if (head == nullptr && tail == nullptr)
	{
		head = event_frame;
		tail = event_frame;
	}
	else
	{
		assert(head != nullptr && tail != nullptr);
		assert(tail->getNext() == nullptr);
		tail->setNext(event_frame);
		tail = event_frame;
	}
}


std::shared_ptr<EventFrame> Queue::PopFront()
{
	// Check if queue is empty
	if (head == nullptr)
	{
		assert(head == nullptr && tail == nullptr);
		return nullptr;
	}

	// Extract element from the head
	std::shared_ptr<EventFrame> event_frame = head;
	if (head == tail)
	{
		head = nullptr;
		tail = nullptr;
	}
	else
	{
		head = head->getNext();
	}

	// Mark as extracted
	event_frame->setNext(nullptr);
	event_frame->setInQueue(false);
	return event_frame;
}


void Queue::Wait(EventType *event_type)
{
	// This function must be invoked within an event handler
	Engine *engine = Engine::getInstance();
	Event *current_event = engine->getCurrentEvent();
	if (current_event == nullptr)
		throw misc::Panic("Function cannot be invoked outside of "
				"an event handler");

	// The event type must be valid
	if (event_type == nullptr)
		throw misc::Panic("Cannot suspend an event with a null "
				"wakeup event type");

	// Add event frame to the stack
	std::shared_ptr<EventFrame> event_frame = current_event->getFrame();
	assert(event_frame != nullptr);
	assert(event_frame->getWakeupEventType() == nullptr);
	event_frame->setWakeupEventType(event_type);
	PushBack(event_frame);
}


void Queue::WakeupOne()
{
	// Queue must have at least one event in it
	if (isEmpty())
		throw misc::Panic("Queue is empty");

	// Get event frame from the head
	std::shared_ptr<EventFrame> event_frame = PopFront();

	// Get event to schedule
	EventType *event_type = event_frame->getWakeupEventType();
	event_frame->setWakeupEventType(nullptr);
	assert(event_type);

	// Schedule event
	Engine *engine = Engine::getInstance();
	engine->Schedule(event_type, event_frame);
}


void Queue::WakeupAll()
{
	// Keep waking up events from the head
	while (!isEmpty())
		WakeupOne();
}

}  // namespace esim
