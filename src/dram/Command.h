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

class Command
{
	// The request associated with this command
	std::shared_ptr<Request> request;
	
	// The type of command
	CommandType type;

	// Location information
	Bank *bank;
	Rank *rank;

public:
	Command(std::shared_ptr<Request> request, CommandType type,
			Bank *bank);

	/// Returns the command's type.
	CommandType getType() const { return type; }

	/// Returns the command's type as a string.
	std::string getTypeString() const { return CommandTypeMap[type]; }

	int getDuration() const;

	/// Returns the bank that the command was created in.
	Bank *getBank() { return bank; }

	/// Returns the rank that the command's bank belonds to.
	Rank *getRank() { return rank; }

	int getBankId() const;
	int getRankId() const;

	Address *getAddress();

	/// Marks the command as finished and decrements the number of in
	/// flight commands for associated request.
	void setFinished();

	/// Map that converts the CommandType enum to a string.
	static std::map<CommandType, std::string> CommandTypeMap;
};

}  // namespace dram

#endif
