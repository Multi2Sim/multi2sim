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

#ifndef DRAM_COMMAND_H
#define DRAM_COMMAND_H

#include <map>
#include <memory>
#include <string>

#include <lib/cpp/String.h>

#include "Request.h"


namespace dram
{

// Forward declarations
struct Address;
class Bank;
class Rank;

enum CommandType
{
	CommandInvalid = 0,
	CommandPrecharge,
	CommandActivate,
	CommandRead,
	CommandWrite
};

// String map for SchedulerType
extern misc::StringMap CommandTypeMap;

/// Map that converts the CommandType enum to a string.
extern std::map<CommandType, std::string> CommandTypeMapToString;


class Command
{
	int id;

	// The request associated with this command
	std::shared_ptr<Request> request;

	// The type of command
	CommandType type;

	// Creation cycle of the command
	long long cycle_created;

	// Location information
	Bank *bank;
	Rank *rank;

public:
	Command(std::shared_ptr<Request> request, CommandType type,
			long long cycle_created, Bank *bank);

	/// Returns the id of this command, which is unique in the
	/// memory system.
	int getId() const { return id; }

	/// Returns the type of the command.
	CommandType getType() const { return type; }

	/// Returns the type of the command as a string.
	std::string getTypeString() const
	{
		return CommandTypeMapToString[type];
	}

	/// Returns how long the command will take to execute once it is
	/// scheduled.
	int getDuration() const;

	/// Returns the cycle when the command was created.
	long long getCycleCreated() { return cycle_created; }

	/// Returns the bank that the command was created in.
	Bank *getBank() { return bank; }

	/// Returns the rank that the command's bank belonds to.
	Rank *getRank() { return rank; }

	int getBankId() const;
	int getRankId() const;

	/// Returns a pointer to the address object of the command.
	Address *getAddress();

	/// Marks the command as finished and decrements the number of in
	/// flight commands for associated request.
	void setFinished();
};

}  // namespace dram

#endif
