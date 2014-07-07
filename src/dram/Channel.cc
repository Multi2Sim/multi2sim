/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "Channel.h"
#include "Controller.h"
#include "Dram.h"
#include "Rank.h"


namespace dram
{


Channel::Channel(int id,
		Controller *parent,
		int num_ranks,
		int num_banks,
		int num_rows,
		int num_columns,
		int num_bits)
		:
		id(id),
		controller(parent)
{
	// Create the ranks for this channel.
	for (int i = 0; i < num_ranks; i++)
	{
		auto rank = std::make_shared<Rank>(i, this, num_banks,
				num_rows, num_columns, num_bits);
		ranks.push_back(rank);
	}
}


void Channel::CallScheduler(int after)
{
	// Get the esim engine instance.
	esim::Engine *esim = esim::Engine::getInstance();

	// Get the scheduler event for this channel.
	esim::EventType *scheduler = Dram::getScheduler(
			getController()->getId(), id);

	// Check that the event isn't already scheduled.
	if (scheduler->isInFlight())
	{
		// Dram::debug << "Not scheduling scheduler\n";
		return;
	}

	// Create the frame to pass containing a reference to this controller.
	auto frame = std::make_shared<SchedulerFrame>();
	frame->channel = this;

	// Call the event for the request processor.
	esim->Call(scheduler, frame, nullptr, after);
}


void Channel::SchedulerHandler(esim::EventType *type,
		esim::EventFrame *frame)
{
	// Get the channel pointer out of the frame.
	SchedulerFrame *scheduler_frame = dynamic_cast<SchedulerFrame *>(frame);
	Channel *channel = scheduler_frame->channel;

	// Call this channel's request processor.
	channel->Scheduler();
}


void Channel::Scheduler()
{

	// Debug
	esim::Engine *esim = esim::Engine::getInstance();
	Dram::debug << misc::fmt("[%lld] Controller %d Channel %d running "
			"scheduler\n", esim->getTime(),
			getController()->getId(), getId());
}


void Channel::dump(std::ostream &os) const
{
	os << misc::fmt("\tDumping Channel %d\n", id);
	os << misc::fmt("\t%d Ranks\n\tRank dump:\n", (int) ranks.size());

	for (auto rank : ranks)
	{
		rank->dump(os);
	}
}


}  // namespace dram
