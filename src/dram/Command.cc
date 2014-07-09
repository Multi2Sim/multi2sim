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

std::map<CommandType, std::string> Command::CommandTypeMap = {
	{CommandInvalid, "Invalid"},
	{CommandPrecharge, "Precharge"},
	{CommandActivate, "Activate"},
	{CommandRead, "Read"},
	{CommandWrite, "Write"}
};


Command::Command(std::shared_ptr<Request> request, CommandType type,
		Bank *bank)
		:
		request(request),
		type(type),
		bank(bank)
{
	// Increment the number of in flight commands for the
	// associated request.
	request->incCommands();

	// Set the rank.
	rank = bank->getRank();
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
	request->decCommands();
}

}  // namespace dram
