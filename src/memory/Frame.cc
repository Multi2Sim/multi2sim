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

#include <cassert>

#include <lib/esim/Engine.h>

#include "Frame.h"
#include "System.h"


namespace mem
{

long long Frame::id_counter = 0;


void Frame::AddWaitingFrame(std::shared_ptr<Frame> frame,
		esim::EventType *event_type)
{
	assert(frame.get() != this);
	frame->wakeup_event_type = event_type;
	waiting_frames.push_back(frame);
}


void Frame::WakeupWaitingFrames()
{
	// No frame to wake up
	if (waiting_frames.size() == 0)
		return;

	// Debug
	esim::Engine *esim = esim::Engine::getInstance();
	System::debug << misc::fmt("  %lld %lld 0x%x wake up accesses:",
			esim->getTime(),
			id,
			address);
/*
	// Wake up all coalesced accesses
	while (waiting_frames.size())
	{
		// Remove from head
		std::shared_ptr<Frame> frame = waiting_frames.pop_front();
		esim::EventType *event_type = frame->wakeup_event_type;

		// Schedule wakeup event
		// FIXME
	}
	while (master_stack->waiting_list_head)
	{
		stack = master_stack->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(master_stack, waiting, stack);
		esim_schedule_event(event, stack, 0);
		mem_debug(" %lld", stack->id);
	}
*/
	// Debug
	System::debug << '\n';
}

}  // namespace mem
