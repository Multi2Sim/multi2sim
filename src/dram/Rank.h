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
	std::vector<std::shared_ptr<Bank>> banks;

	// Last scheduled command information
	CommandType last_scheduled_command_type = CommandTypeInvalid;
	long long last_scheduled_commands[5] = {0, 0, 0, 0, 0};

public:

	Rank(int id,
			Channel *parent,
			int num_banks,
			int num_rows,
			int num_columns,
			int num_bits);

	int getId() { return id; }

	Bank *getBank(int id) { return banks[id].get(); }
	Channel *getChannel() { return channel; }

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
