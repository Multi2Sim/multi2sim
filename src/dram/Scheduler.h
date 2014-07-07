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

#include "Command.h"


namespace dram
{

// Forward declarations
class Channel;


/// To make a new scheduler, this base class should be subclassed.  The
/// constructor must contain at least a pointer to the channel that owns it
/// and should call the base class constructor.  The FindNext method should
/// be implemented with the scheduling algorithm, and any state variables
/// required should be added to the class.
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

	/// Returns the ids of the next Rank-Bank pair that should have its
	/// command scheduled next.  In the case that one isn't found, the pair
	/// (-1, -1) is returned.
	virtual std::pair<int, int> FindNext() = 0;
};


class RankBank : public Scheduler
{
	int cur_rank = 0;
	int cur_bank = 0;

public:

	RankBank(Channel *owner)
			:
			Scheduler(owner)
	{
	}

	/// Returns the ids of the next Rank-Bank pair that should have its
	/// command scheduled next based on the Rank Bank Round Robin
	/// algorithm.
	std::pair<int, int> FindNext();
};

}  // namespace dram

#endif
