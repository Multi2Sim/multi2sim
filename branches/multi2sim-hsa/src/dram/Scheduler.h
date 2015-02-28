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

#ifndef DRAM_SCHEDULER_H
#define DRAM_SCHEDULER_H

#include <utility>

#include <lib/cpp/String.h>

#include "Command.h"


namespace dram
{

// Forward declarations
class Bank;
class Channel;


// Possible scheduling algorithms
enum SchedulerType
{
	SchedulerRankBankRoundRobin,
	SchedulerOldestFirst
};

// String map for SchedulerType
extern misc::StringMap SchedulerTypeMap;


/// To make a new scheduler, this base class should be subclassed.  The
/// constructor must contain at least a pointer to the channel that owns it
/// and should call the base class constructor.  The FindNext method should
/// be implemented with the scheduling algorithm, and any state variables
/// required should be added to the class.
/// After the new scheduler is made, add it to the SchedulerType enum,
/// SchedulerTypeMap StringMap and the switch block in Channel::Channel.
class Scheduler
{

protected:

	// Pointer to the owning channel.
	Channel *channel;

public:

	Scheduler(Channel *owner)
			:
			channel(owner)
	{
	}

	/// Returns the pointer to the next bank that should have its command
	/// scheduled next.  In the case that one isn't found, nullptr is
	/// returned.
	virtual Bank *FindNext() = 0;
};


class OldestFirst : public Scheduler
{

public:
	OldestFirst(Channel *owner)
			:
			Scheduler(owner)
	{
	}

	/// Returns the the pointer to the next bank that should have its
	/// command scheduled next based on the Oldest First algorithm.
	Bank *FindNext();
};


class RankBankRoundRobin : public Scheduler
{
	int current_rank = 0;
	int current_bank = 0;

public:

	RankBankRoundRobin(Channel *owner)
			:
			Scheduler(owner)
	{
	}

	/// Returns the the pointer to the next bank that should have its
	/// command scheduled next based on the Rank Bank Round Robin
	/// algorithm.
	Bank *FindNext();
};

}  // namespace dram

#endif
