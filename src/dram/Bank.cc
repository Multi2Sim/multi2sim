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

#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "Address.h"
#include "Bank.h"
#include "Channel.h"
#include "Controller.h"
#include "System.h"
#include "Rank.h"

namespace dram
{

Bank::Bank(int id,
		Rank *parent,
		int num_rows,
		int num_columns,
		int num_bits)
		:
		id(id),
		rank(parent),
		num_rows(num_rows),
		num_columns(num_columns),
		num_bits(num_bits)
{
}


void Bank::setLastScheduledCommand(CommandType type)
{
	// Update the last scheduled command matrix for this bank.
	last_scheduled_commands[type] = System::frequency_domain->getCycle();
	last_scheduled_command_type = type;

	// Make the rank that this bank belongs to do the same update.
	rank->setLastScheduledCommand(type);
}


void Bank::ProcessRequest(std::shared_ptr<Request> request)
{
	// Get the current cycle.
	long long cycle = System::frequency_domain->getCycle();

	// Pull the address out of the request.
	Address *address = request->getAddress();

	// Break the request down into its commands and add them to the queue.
	// For all checks to the active row, the checks are made to what the
	// active row will be when all commands in the queue have run, because
	// the new commands are added at the end of the queue and work on the
	// bank at that point in time.

	// Create the activate command if the bank will be precharged.
	// (Row closed)
	if (isPrechargedFuture())
	{
		// Create the command.
		auto activate_command = std::make_shared<Command>(
				request, CommandActivate, cycle, this);

		// Add it to the command queue.
		command_queue.push_back(activate_command);

		// Set the future active row.
		future_active_row = address->getRow();
	}

	// Create the precharge and activate commands if the wrong row will
	// be open. (Row miss)
	else if (getActiveRowFuture() != address->getRow())
	{
		// Create the commands.
		auto precharge_command = std::make_shared<Command>(
				request, CommandPrecharge, cycle, this);
		auto activate_command = std::make_shared<Command>(
				request, CommandActivate, cycle, this);

		// Add them to the command queue.
		command_queue.push_back(precharge_command);
		command_queue.push_back(activate_command);

		// Set the future active row.
		future_active_row = address->getRow();
	}

	// Check that the desired row will actually be open.
	if (future_active_row != address->getRow())
		throw misc::Panic("Desired row will not be opened.");

	// At this point either the desired row will already be open, or we
	// just created the commands necessary for it to be open.  Create
	// the appropriate access command (read or write).
	std::shared_ptr<Command> access_command;
	if (request->getType() == RequestRead)
		access_command = std::make_shared<Command>(
				request, CommandRead, cycle, this);
	else if (request->getType() == RequestWrite)
		access_command = std::make_shared<Command>(
				request, CommandWrite, cycle, this);
	else
		// Invalid request type
		throw misc::Panic("Invalid request type");

	// Add it to the command queue.
	command_queue.push_back(access_command);

	// If the the page policy is set to closed page, then also add a
	// precharge command to the end of the queue.
	if (getRank()->getChannel()->getController()->getPagePolicy() ==
			PagePolicyClosed)
	{
		// Create the command.
		auto precharge_command = std::make_shared<Command>(
				request, CommandPrecharge, cycle, this);

		// Add it to the command queue.
		command_queue.push_back(precharge_command);

		// Set the future active row to indicate precharged.
		future_active_row = -1;
	}

	// Ensure the scheduler is running.
	getRank()->getChannel()->CallScheduler();

	// Debug
	System::debug << misc::fmt("[%lld] Processed request for 0x%llx in "
			"bank %d\n", cycle,
			address->getEncoded(), id);
}


long long Bank::getFrontCommandTiming()
{
	return getRank()->getChannel()->CalculateReadyCycle(
			command_queue.front().get());
}


void Bank::RunFrontCommand()
{
	long long cycle = System::frequency_domain->getCycle();

	// Get the command that is being run.
	std::shared_ptr<Command> command = command_queue.front();

	// Add this command to the last scheduled command matrix.
	setLastScheduledCommand(command->getType());

	// Get the esim engine instance.
	esim::Engine *esim = esim::Engine::getInstance();

	// Create return event
	auto frame = std::make_shared<CommandReturnFrame>(command);
	esim->Call(System::event_command_return, frame, nullptr,
			command->getDuration());

	// Debug
	System::activity << misc::fmt("[%lld] [%d : %d] Running command #%d "
			"%s for 0x%llx\n", cycle, rank->getId(), id,
			command->getId(), command->getTypeString().c_str(),
			command->getAddress()->getEncoded());

	// Command is being run, remove it from the queue.
	command_queue.pop_front();
}


void Bank::dump(std::ostream &os) const
{
	// Print header
	os << misc::fmt("\t\t\tDumping Bank %d\n", id);

	// Print bank geometry
	os << misc::fmt("\t\t\t%d Rows, %d Columns, %d Bits per Column\n",
			num_rows, num_columns, num_bits);

	// Print the commands currently in queue
	os << misc::fmt("\t\t\t%d Commands in queue\n", (int)command_queue.size());
	for (int i = 0; i < (int)command_queue.size(); i++)
	{
		os << misc::fmt("\t\t\t\t%d - %s\n", i,
				command_queue[i]->getTypeString().c_str());
	}
}


}  // namespace dram
