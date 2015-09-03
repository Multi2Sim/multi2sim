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

#include <climits>

#include <lib/cpp/String.h>

#include "Bank.h"
#include "Channel.h"
#include "System.h"
#include "Scheduler.h"


namespace dram
{

misc::StringMap SchedulerTypeMap
{
	{ "RankBankRoundRobin", SchedulerRankBankRoundRobin},
	{ "OldestFirst", SchedulerOldestFirst }
};


Bank *OldestFirst::FindNext()
{
	// Keep track of the bank with the oldest command found so far.
	long long oldest_cycle = LLONG_MAX;
	Bank *oldest_bank = nullptr;

	// Get the number of banks in each rank of the channel.
	int num_banks = channel->getNumBanks();

	// Iterate through all the ranks and banks.
	for (int i = 0; i < channel->getNumBanksTotal(); i++)
	{
		// Get the current bank
		Bank *bank = channel->getRank(i / num_banks)
				->getBank(i % num_banks);

		// Move to the next bank if this one has no commands in queue.
		if (bank->getNumCommandsInQueue() == 0)
		{
			continue;
		}

		// Get the cycle when the front command in this bank was
		// created.
		long long current_bank_cycle =
				bank->getFrontCommandCycleCreated();

		// Make this the bank with the oldest command if it is.
		if (current_bank_cycle < oldest_cycle)
		{
			oldest_cycle = current_bank_cycle;
			oldest_bank = bank;
		}
	}

	// Return the bank with the oldest command at the front of its queue.
	// If no bank was found, then the value of oldest_bank will still be
	// nullptr, which is the return value that indicates none was found.
	return oldest_bank;
}


Bank *RankBankRoundRobin::FindNext()
{
	// Iterate through all the ranks and banks.  No more, no less.
	for (int i = 0; i < channel->getNumBanksTotal(); i++)
	{
		// Increment the bank we're working on.
		current_bank++;

		// If we've finished all the banks in this rank, continue to
		// the beginning of the next rank.
		if (current_bank == channel->getNumBanks())
		{
			current_bank = 0;
			current_rank++;
		}

		// If we've finished all the ranks in this channel, move back
		// to the first one.
		if (current_rank == channel->getNumRanks())
			current_rank = 0;

		// Get a pointer to this bank.
		Bank *bank = channel->getRank(current_rank)->
				getBank(current_bank);

		// Move to the next bank if this one has no commands in queue.
		if (bank->getNumCommandsInQueue() == 0)
		{
			continue;
		}

		// Debug
		long long cycle = System::frequency_domain->getCycle();
		System::debug << misc::fmt("[%lld] Scheduler returns %d : %d "
				"for next command scheduling\n", cycle,
				current_rank, current_bank);

		// This bank has a command, so it's the one to be scheduled.
		return bank;
	}

	// If we reach this point, then every bank in every rank of the channel
	// has been searched and no command that can be scheduled has been
	// found.

	// Return nullptr to indicate that no command has been found.
	return nullptr;
}

}  // namespace dram
