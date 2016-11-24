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

#ifndef DRAM_RANK_H
#define DRAM_RANK_H

#include <vector>
#include <iostream>

#include "Command.h"


namespace dram
{

// Forward declarations
class Bank;
class Channel;


class Rank
{
	int id;

	// Pointer to the owning channel.
	Channel *channel;

	// List of ranks contained in this channel
	std::vector<std::unique_ptr<Bank>> banks;

	// Last scheduled command information
	CommandType last_scheduled_command_type = CommandInvalid;
	long long last_scheduled_commands[5] = {-100, -100, -100, -100, -100};

public:

	Rank(int id,
			Channel *parent,
			int num_banks,
			int num_rows,
			int num_columns,
			int num_bits);

	/// Returns the id of this rank, which is unique in the channel that
	/// this rank belongs to.
	int getId() { return id; }

	/// Returns a bank that belongs to this rank with the specified id.
	Bank *getBank(int id) const { return banks[id].get(); }

	/// Returns the channel that this rank belongs to.
	Channel *getChannel() const { return channel; }

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

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const Rank &object)
	{
		object.dump(os);
		return os;
	}
};


}  // namespace dram

#endif
