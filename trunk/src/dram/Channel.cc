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

#include <algorithm>
#include <iostream>

#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "Bank.h"
#include "Channel.h"
#include "Controller.h"
#include "System.h"


namespace dram
{


Channel::Channel(int id,
		Controller *parent,
		int num_ranks,
		int num_banks,
		int num_rows,
		int num_columns,
		int num_bits,
		SchedulerType scheduler_type)
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

	switch(scheduler_type)
	{
	// Create a Rank Bank Round Robin scheduler.
	case SchedulerRankBankRoundRobin:
		scheduler = std::unique_ptr<Scheduler>(
				new RankBankRoundRobin(this));
		break;

	// Create an Oldest First scheduler.
	case SchedulerOldestFirst:
		scheduler = std::unique_ptr<Scheduler>(
				new OldestFirst(this));
		break;
	}
}


void Channel::CallScheduler(int after)
{
	// Get the esim engine instance.
	esim::Engine *esim = esim::Engine::getInstance();

	// Get the scheduler event for this channel.
	esim::Event *scheduler = controller->getScheduler(id);

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


void Channel::SchedulerHandler(esim::Event *type, esim::Frame *frame)
{
	// Get the channel pointer out of the frame.
	SchedulerFrame *scheduler_frame = dynamic_cast<SchedulerFrame *>(frame);
	Channel *channel = scheduler_frame->channel;

	// Call this channel's request processor.
	channel->RunScheduler();
}


void Channel::RunScheduler()
{
	// Get the current cycle
	long long cycle = System::frequency_domain->getCycle();

	// Debug
	System::debug << misc::fmt("[%lld] Controller %d Channel %d running "
			"scheduler\n", cycle, getController()->getId(), id);

	// Get a pointer to the bank whose front command should be run next.
	// This is either the result of the scheduling algorithm from a previous
	// cycle (if timing constraints prevented it from being run then), or
	// we run the scheduling algorithm again to get the next one.
	Bank *bank = next_scheduled_bank;
	if (bank == nullptr)
	{
		bank = scheduler->FindNext();

		// Don't do anything if no bank was found.
		if (bank == nullptr)
			return;
	}

	// Check the timing for the command at the front of the selected
	// bank's queue.
	long long cycle_ready = bank->getFrontCommandTiming();

	// Run the command now if able, otherwise put off running the
	// command until it is able.
	if (cycle >= cycle_ready)
	{
		// Run the command.
		bank->RunFrontCommand();
		next_scheduled_bank = nullptr;

		// Call the scheduler again for next cycle.
		CallScheduler(1);
	}
	else
	{
		// Keep track of the bank that should be scheduled for next
		// time the scheduler is run.
		next_scheduled_bank = bank;

		// Call the scheduler again when the selected command is ready
		// to be run.
		CallScheduler(cycle_ready - cycle);
	}
}


long long Channel::CalculateReadyCycle(Command *cmd)
{
	// Yes, Rafa, this is ugly.

	// Set the ready cycle to be this cycle.
	long long ready = System::frequency_domain->getCycle();

	// Get the rank and bank ids of the commands location.
	int cmd_bank = cmd->getBankId();
	int cmd_rank = cmd->getRankId();

	// Get the command's type.
	CommandType cmd_type = cmd->getType();

	// Iterate through every rank and bank in the channel.
	for (int rank_id = 0; rank_id < num_ranks; rank_id++)
	{
		// Get the current rank
		Rank *rank = getRank(rank_id);

		for (int bank_id = 0; bank_id < num_banks; bank_id++)
		{
			// Get the current bank
			Bank *bank = rank->getBank(bank_id);

			switch (cmd_type)
			{

			// All the following statements change the ready cycle
			// to follow the DRAM timing constraints depending on
			// when previous commands of certain types were
			// scheduled to certain locations.
			// The read cycle to follow each rule is calculated,
			// and then the max of that cycle and the current ready
			// cycle is taken and assigned to be the new ready cycle.

			case CommandPrecharge:
				// The rank and bank are the same as the
				// new command.
				if (rank_id == cmd_rank && bank_id == cmd_bank)
				{
					// A P s s
					ready = std::max(bank->getLastScheduledCommand(CommandActivate)
							+ controller->getTiming(TimingActivate, TimingPrecharge, TimingSame, TimingSame),
							ready);

					// R P s s
					ready = std::max(bank->getLastScheduledCommand(CommandRead)
							+ controller->getTiming(TimingRead, TimingPrecharge, TimingSame, TimingSame),
							ready);

					// W P s s
					ready = std::max(bank->getLastScheduledCommand(CommandWrite)
							+ controller->getTiming(TimingWrite, TimingPrecharge, TimingSame, TimingSame),
							ready);
				}
				break;

			case CommandActivate:
				// The rank and bank are the same as the
				// new command.
				if (rank_id == cmd_rank && bank_id == cmd_bank)
				{
					// A A s s
					ready = std::max(bank->getLastScheduledCommand(CommandActivate)
							+ controller->getTiming(TimingActivate, TimingActivate, TimingSame, TimingSame),
							ready);

					// P A s s
					ready = std::max(bank->getLastScheduledCommand(CommandPrecharge)
							+ controller->getTiming(TimingPrecharge, TimingActivate, TimingSame, TimingSame),
							ready);
				}

				// The rank is the same as the new command, and
				// the bank is different.
				if (rank_id == cmd_rank && bank_id != cmd_bank)
				{
					// A A s d
					ready = std::max(bank->getLastScheduledCommand(CommandActivate)
							+ controller->getTiming(TimingActivate, TimingActivate, TimingSame, TimingDifferent),
							ready);
				}
				break;

			case CommandRead:
				// The rank and bank are the same as the
				// new command.
				if (rank_id == cmd_rank && bank_id == cmd_bank)
				{
					// A R s s
					ready = std::max(bank->getLastScheduledCommand(CommandActivate)
							+ controller->getTiming(TimingActivate, TimingRead, TimingSame, TimingSame),
							ready);
				}

				// The rank is the same as the new command,
				// the bank is either the same or different.
				if (rank_id == cmd_rank)
				{
					// R R s a
					ready = std::max(rank->getLastScheduledCommand(CommandRead)
							+ controller->getTiming(TimingRead, TimingRead, TimingSame, TimingAny),
							ready);

					// W R s a
					ready = std::max(rank->getLastScheduledCommand(CommandWrite)
							+ controller->getTiming(TimingWrite, TimingRead, TimingSame, TimingAny),
							ready);
				}

				// The rank is different than the new command,
				// the bank is either the same or different.
				if (rank_id != cmd_rank)
				{
					// R R d a
					ready = std::max(rank->getLastScheduledCommand(CommandRead)
							+ controller->getTiming(TimingRead, TimingRead, TimingDifferent, TimingAny),
							ready);

					// W R d a
					ready = std::max(rank->getLastScheduledCommand(CommandWrite)
							+ controller->getTiming(TimingRead, TimingRead, TimingDifferent, TimingAny),
							ready);
				}
				break;

			case CommandWrite:
				// The rank and bank are the same as the
				// new command.
				if (rank_id == cmd_rank && bank_id == cmd_bank)
				{
					// A W s s
					ready = std::max(rank->getLastScheduledCommand(CommandActivate)
							+ controller->getTiming(TimingActivate, TimingWrite, TimingSame, TimingSame),
							ready);
				}

				// The rank is the same as the new command,
				// the bank is either the same or different.
				if (rank_id == cmd_rank)
				{
					// W W s a
					ready = std::max(rank->getLastScheduledCommand(CommandWrite)
							+ controller->getTiming(TimingWrite, TimingWrite, TimingSame, TimingAny),
							ready);
				}

				// The rank is different than the new command,
				// the bank is either the same or different.
				if (rank_id != cmd_rank)
				{
					// W W d a
					ready = std::max(rank->getLastScheduledCommand(CommandWrite)
							+ controller->getTiming(TimingWrite, TimingWrite, TimingDifferent, TimingAny),
							ready);
				}

				// Both the rank and bank are either the same
				// or different than the new command.
				// R W a a
				ready = std::max(rank->getLastScheduledCommand(CommandRead)
						+ controller->getTiming(TimingRead, TimingWrite, TimingAny, TimingAny),
						ready);
				break;

			// Timings cannot be calculated for an invalid command.
			case CommandInvalid:
				throw misc::Panic("Can't calculate timing for invalid command");

			}
		}
	}

	// Return the final cycle that the command will be ready to run.
	return ready;
}


void Channel::dump(std::ostream &os) const
{
	// Print header
	os << misc::fmt("\tDumping Channel %d\n", id);

	// Print ranks owned by this channel
	os << misc::fmt("\t%d Ranks\n\tRank dump:\n", (int) ranks.size());
	for (auto const& rank : ranks)
	{
		rank->dump(os);
	}
}


}  // namespace dram
