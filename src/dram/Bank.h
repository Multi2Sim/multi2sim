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

	int num_rows;
	int num_columns;
	int num_bits;

	// Queue of commands to be sent to the Bank
	std::deque<std::shared_ptr<Command>> command_queue;

	// Last scheduled command information
	CommandType last_scheduled_command_type = CommandTypeInvalid;
	long long last_scheduled_commands[5] = {0, 0, 0, 0, 0};

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

	int getId() { return id; }

	// Returns if the bank is precharged.
	bool isPrecharged() { return current_active_row == -1; }

	/// Returns if the bank will be precharged.
	bool isPrechargedFuture() { return future_active_row == -1; }

	/// Returns what row is activated.
	int getActiveRow() { return current_active_row; }

	/// Returns what row will be activated.
	int getActiveRowFuture() { return future_active_row; }

	Rank *getRank() { return rank; }

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
