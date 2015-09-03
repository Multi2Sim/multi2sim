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

#ifndef DRAM_BANK_H
#define DRAM_BANK_H

#include <memory>
#include <deque>
#include <iostream>

#include "Command.h"


namespace dram
{

// Forward declarations
class Rank;
class Request;


class Bank
{
	int id;

	// Pointer to the owning rank.
	Rank *rank;

	// Bank geometry
	int num_rows;
	int num_columns;
	int num_bits;

	// Queue of commands to be sent to the Bank
	std::deque<std::shared_ptr<Command>> command_queue;

	// Last scheduled command information
	CommandType last_scheduled_command_type = CommandInvalid;
	long long last_scheduled_commands[5] = {-100, -100, -100, -100, -100};

	// Current and future row activations.
	// A value of -1 means the bank is precharged.
	int current_active_row = -1;
	int future_active_row = -1;

public:

	Bank(int id,
			Rank *parent,
			int num_rows,
			int num_columns,
			int num_bits);

	/// Returns the id of this bank, which is unique in the rank that this
	/// bank belongs to.
	int getId() const { return id; }

	/// Returns the rank that this bank belongs to.
	Rank *getRank() const { return rank; }

	/// Returns the type of the last scheduled command.
	CommandType getLastScheduledCommandType() const
	{
		return last_scheduled_command_type;
	}

	/// Returns the cycle that the last scheduled command of the specfied
	/// type was made in.
	long long getLastScheduledCommand(CommandType type) const
	{
		return last_scheduled_commands[type];
	}

	/// Updates the cycle of the last scheduled command of the specified
	/// type and updates the last scheduled command type.
	void setLastScheduledCommand(CommandType type);

	// Returns if the bank is precharged.
	bool isPrecharged() const { return current_active_row == -1; }

	/// Returns if the bank will be precharged.
	bool isPrechargedFuture() const { return future_active_row == -1; }

	/// Returns what row is activated.
	int getActiveRow() const { return current_active_row; }

	/// Returns what row will be activated.
	int getActiveRowFuture() const { return future_active_row; }

	/// Returns how many commands are in the queue.
	int getNumCommandsInQueue() const { return (int) command_queue.size(); }

	/// Returns the command in the queue at a certain position.
	std::string getCommandInQueueType(int position = 0)
	{
		return command_queue[position]->getTypeString();
	}

	/// Returns the cycle when the command at the front of the queue was
	/// created.
	long long getFrontCommandCycleCreated()
	{
		return command_queue.front()->getCycleCreated();
	}

	/// Returns the cycle when the command at the front of the queue will
	/// be ready to be run.
	long long getFrontCommandTiming();

	/// Pops off the top command in the queue.
	void RunFrontCommand();

	/// Breaks a request down into it's component commands and adds them to the
	/// bank's command queue.
	void ProcessRequest(std::shared_ptr<Request> request);

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const Bank &object)
	{
		object.dump(os);
		return os;
	}
};


}  // namespace dram

#endif
