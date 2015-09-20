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

#ifndef DRAM_CONTROLLER_H
#define DRAM_CONTROLLER_H

#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <queue>

#include <lib/cpp/IniFile.h>
#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Event.h>

#include "Command.h"

namespace dram
{

// Forward declarations
class Channel;
class Command;
class Request;


// Definitions of lookup values for timing tables
// The timing table would be defined as having dimensions:
//	[TimingCommand][TimingCommand][TimingLocation][TimingLocation]
enum TimingCommand
{
	TimingPrecharge = 0,
	TimingActivate,
	TimingRead,
	TimingWrite
};


enum TimingLocation
{
	TimingSame = 0,
	TimingDifferent,
	TimingAny = 0
};


/// Page Policy Types
enum PagePolicyType
{
	PagePolicyOpen = 0,
	PagePolicyClosed
};

/// String map for PagePolicyType
extern misc::StringMap PagePolicyTypeMap;


class Controller
{
	int id;

	std::string name;

	// Controller geometry
	int num_channels;
	int num_ranks;
	int num_banks;
	int num_rows;
	int num_columns;
	int num_bits;

	// The page policy that command processors in this controller follow
	PagePolicyType page_policy;

	// Timing matrix
	int timings[4][4][2][2] = {};

	// Command durations
	int command_durations[5] = {};

	// List of physical channels contained in this controller
	std::vector<std::unique_ptr<Channel>> channels;

	// Incoming request queue
	std::queue<std::shared_ptr<Request>> incoming_requests;

	// Map of ids to EventTypes for each controller's request processor
	static std::map<int, esim::Event *> REQUEST_PROCESSORS;

	// Map of ids to EventTypes for each channel's scheduler for the
	// controller
	std::map<int, esim::Event *> SCHEDULERS;

public:

	Controller(int id);
	Controller(int id, misc::IniFile *config, const std::string &section);

	/// Returns the id of this controller, which is unique for all
	/// controllers.
	int getId() const { return id; }

	/// Returns a channel that belongs to this controller with the
	/// specified id.
	Channel *getChannel(int id) { return channels[id].get(); }

	int getNumChannels() const { return num_channels; }
	int getNumRanks() const { return num_ranks; }
	int getNumBanks() const { return num_banks; }
	int getNumRows() const { return num_rows; }
	int getNumColumns() const { return num_columns; }

	/// Returns that page policy that command processers under this
	/// controller follow.
	PagePolicyType getPagePolicy() { return page_policy; }

	/// Returns the minimum timing seperation (in number of cycles) between
	/// two commands in two locations, based on the timing protocol matrix.
	int getTiming(TimingCommand prev, TimingCommand next,
			TimingLocation rank, TimingLocation bank) const
	{
		return std::max(timings[prev][next][rank][bank], 1);
	}

	/// Returns the duration of a command of the specified type.
	int getCommandDuration(CommandType type)
	{
		return command_durations[type];
	}

	/// Parse a MemoryController section of a dram configuration file
	void ParseConfiguration(misc::IniFile *ini_file,
			const std::string &section);
	void ParseConfigurationTiming(misc::IniFile *config,
			const std::string &section);

	/// Add a request to the controller's incoming request queue.
	void AddRequest(std::shared_ptr<Request> request);

	/// Obtain the Event for the controller's request processor.
	static esim::Event *getRequestProcessor(int controller)
	{
		return REQUEST_PROCESSORS[controller];
	}

	/// Create a new Event for a controller's request processor.
	static void CreateRequestProcessor(int controller);

	/// Obtain the Event for the channel's scheduler.
	esim::Event *getScheduler(int channel)
	{
		return SCHEDULERS[channel];
	}

	/// Create a set of new EventTypes for the controller's schedulers.
	void CreateSchedulers(int num_channels);

	/// Call the request processor for this controller.  This function will
	/// only invoke the request processor if it is not already scheduled to
	/// run.  The request processor will keep reinvoking itself while there
	/// are requests in the incoming request queue.
	void CallRequestProcessor();

	/// Event handler that runs the request processor.
	static void RequestProcessorHandler(esim::Event *, esim::Frame *);

	/// Process requests in the incoming requests queue, breaking them
	/// down into their commands.
	void RunRequestProcessor();

	/// Event handler that for when a command finishes executing.
	static void CommandReturnHandler(esim::Event *, esim::Frame *);

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const Controller &object)
	{
		object.dump(os);
		return os;
	}
};


class RequestProcessorFrame : public esim::Frame
{

public:
	RequestProcessorFrame()
	{
	}

	Controller *controller;
};


class CommandReturnFrame : public esim::Frame
{
	// The command that this event was created for.
	std::shared_ptr<Command> command;

public:
	CommandReturnFrame(std::shared_ptr<Command> command)
			:
			command(command)
	{
	}

	// Returns a pointer to the command that this event was created for.
	Command *getCommand() { return command.get(); }
};

}  // namespace dram

#endif
