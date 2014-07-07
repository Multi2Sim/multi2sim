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

#include <iostream>

#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "Bank.h"
#include "Channel.h"
#include "Controller.h"
#include "Rank.h"
#include "Scheduler.h"
#include "System.h"


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
		controller(parent),
		num_ranks(num_ranks),
		num_banks(num_banks)
{
	// Create the ranks for this channel.
	for (int i = 0; i < num_ranks; i++)
		ranks.emplace_back(new Rank(i, this, num_banks, num_rows,
				num_columns, num_bits));
	scheduler = std::unique_ptr<Scheduler>(new RankBank(this));
}


void Channel::CallScheduler(int after)
{
	// Get the esim engine instance.
	esim::Engine *esim = esim::Engine::getInstance();

	// Get the scheduler event for this channel.
	esim::EventType *scheduler = controller->getScheduler(id);

	// Check that the event isn't already scheduled.
	if (scheduler->isInFlight())
	{
		// System::debug << "Not scheduling scheduler\n";
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
	channel->RunScheduler();
}


void Channel::RunScheduler()
{
	esim::Engine *esim = esim::Engine::getInstance();

	// Get a pointer to the bank whose front command should be run next.
	// This is either the result of the scheduling algorithm from a previous
	// cycle (if timing constraints prevented it from being run then), or
	// we run the scheduling algorithm again to get the next one.
	Bank *bank = next_scheduled_bank;
	if (bank == nullptr)
	{
		std::pair<int, int> bank_found = scheduler->FindNext();

		System::debug << misc::fmt("[%lld] Scheduler returns %d : %d "
				"for next command scheduling\n", esim->getTime(),
				bank_found.first, bank_found.second);

		// Don't do anything if no bank was found.
		if (bank_found.first == -1)
			return;

		// Get the rank found by the scheduling algorithm.
		bank = getRank(bank_found.first)->getBank(bank_found.second);
	}

	// Check the timing for the command at the front of the selected
	// bank's queue.
	long long cycle_ready = bank->getFrontCommandTiming();

	std::cout << misc::fmt("%lld, %lld", cycle_ready, esim->getTime());

	// Run the command now if able, otherwise put off running the
	// command until it is able.
	if (esim->getTime() >= cycle_ready)
	{
		bank->runFrontCommand();
		next_scheduled_bank = nullptr;

		// Call the scheduler again for next cycle.
		CallScheduler(1);

		std::cout << "\t\tRan command\n";
	}
	else
	{
		next_scheduled_bank = bank;

		// Call the scheduler again when the selected command is ready
		// to be run.
		CallScheduler((cycle_ready - esim->getTime()) / 1000);

		std::cout << "\t\tDelayed command\n";
	}

	// Debug
	System::debug << misc::fmt("[%lld] Controller %d Channel %d running "
			"scheduler\n", esim->getTime(),
			getController()->getId(), getId());
}


void Channel::dump(std::ostream &os) const
{
	os << misc::fmt("\tDumping Channel %d\n", id);
	os << misc::fmt("\t%d Ranks\n\tRank dump:\n", (int) ranks.size());

	for (auto const& rank : ranks)
	{
		rank->dump(os);
	}
}


}  // namespace dram
