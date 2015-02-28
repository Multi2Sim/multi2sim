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

#include <lib/cpp/String.h>

#include "Bank.h"
#include "Channel.h"
#include "Command.h"
#include "Controller.h"
#include "Rank.h"
#include "System.h"


namespace dram
{

misc::StringMap CommandTypeMap
{
	{"Invalid", CommandInvalid},
	{"Precharge", CommandPrecharge},
	{"Activate", CommandActivate},
	{"Read", CommandRead},
	{"Write", CommandWrite}
};

std::map<CommandType, std::string> CommandTypeMapToString = {
	{CommandInvalid, "Invalid"},
	{CommandPrecharge, "Precharge"},
	{CommandActivate, "Activate"},
	{CommandRead, "Read"},
	{CommandWrite, "Write"}
};


Command::Command(std::shared_ptr<Request> request, CommandType type,
		long long cycle_created, Bank *bank)
		:
		request(request),
		type(type),
		cycle_created(cycle_created),
		bank(bank)
{
	// Set the rank.
	rank = bank->getRank();

	// Set the id.
	System *dram = System::getInstance();
	id = dram->getNextCommandId();
}


int Command::getDuration() const
{
	// Get the controller that this command is under.
	Controller *controller = bank->getRank()->getChannel()->getController();

	// Return the correct duration of this command based on its type.
	return controller->getCommandDuration(type);
}


int Command::getBankId() const
{
	return bank->getId();
}


int Command::getRankId() const
{
	return rank->getId();
}


Address *Command::getAddress()
{
	return request->getAddress();
}


void Command::setFinished()
{
	// Mark the associated request as finished, too, if this is the read or
	// write command for that request.
	if (type == CommandRead || type == CommandWrite)
		request->setFinished();
}

}  // namespace dram
