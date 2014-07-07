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

enum CommandType
{
	CommandTypeInvalid = 0,
	CommandTypePrecharge,
	CommandTypeActivate,
	CommandTypeRead,
	CommandTypeWrite
};

class Command
{
	std::shared_ptr<Request> request;
	CommandType type;
	int bank_id;
	int rank_id;

public:
	Command(std::shared_ptr<Request> request, CommandType type,
			int bank_id, int rank_id)
			:
			request(request),
			type(type),
			bank_id(bank_id),
			rank_id(rank_id)
	{
	}

	CommandType getType() const { return type; }

	std::string getTypeString() const { return CommandTypeMap[type]; }

	Address *getAddress();

	/// Map that converts the CommandType enum to a string.
	static std::map<CommandType, std::string> CommandTypeMap;
};

}  // namespace dram

#endif
