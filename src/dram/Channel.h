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

#ifndef DRAM_CHANNEL_H
#define DRAM_CHANNEL_H

#include <memory>
#include <vector>

#include <lib/esim/Frame.h>

#include "Rank.h"
#include "Scheduler.h"


namespace dram
{

// Forward declarations
class Controller;
class Scheduler;


class Channel
{
	int id;

	// Pointer to the owning controller.
	Controller *controller;

	// Channel geometry
	int num_ranks;
	int num_banks;

	// Pointer to the bank whose front command should be run next.
	Bank *next_scheduled_bank = nullptr;

	// List of ranks contained in this channel
	std::vector<std::unique_ptr<Rank>> ranks;

	// Pointer to the scheduler instance that determines what commands
	// are run.
	std::unique_ptr<Scheduler> scheduler;

	// Total number of commands in bank queues under this channel
	int num_commands_in_queue = 0;

public:

	Channel(int id,
			Controller *parent,
			int num_ranks,
			int num_banks,
			int num_rows,
			int num_columns,
			int num_bits,
			SchedulerType scheduler_type);

	/// Returns the id of this channel, which is unique in the controller
	/// that this channel belongs to.
	int getId() const { return id; }

	/// Returns a rank belonging to this channel with the specified id.
	Rank *getRank(int id) const { return ranks[id].get(); }

	/// Returns the controller that this channel belongs to.
	Controller *getController() const { return controller; }

	/// Returns the number of ranks in this channel.
	int getNumRanks() const { return num_ranks; }

	/// Returns the number of banks in each rank in this channel.
	int getNumBanks() const { return num_banks; }

	/// Returns the total number of banks in this channel.
	int getNumBanksTotal() const { return num_banks * num_ranks; }

	/// Call the scheduler for this channel.  This function will only
	/// invoke the scheduler if it is not already scheduled to run.  The
	/// scheduler will keep reinvoking itself while there are commands in
	/// any bank queue under the channel.
	///
	/// \param after
	/// The number of cycles that the scheduler should be invoked after.
	/// For outside calls this should probably always stay at the default
	/// 1.  The scheduler itself will reschedule itself further in the
	/// future in some cases.
	void CallScheduler(int after = 1);

	/// Event handler that runs the scheduler.
	static void SchedulerHandler(esim::Event *, esim::Frame *);

	/// Schedules commands from all the bank queues under this channel,
	/// in the correct order according to the scheduling policy.
	void RunScheduler();

	/// Calculates the cycle that the command passed can be run in first,
	/// given the current state of the system.
	long long CalculateReadyCycle(Command *cmd);

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const Channel &object)
	{
		object.dump(os);
		return os;
	}
};


class SchedulerFrame : public esim::Frame
{

public:
	SchedulerFrame()
	{
	}

	Channel *channel;
};


}  // namespace dram

#endif
