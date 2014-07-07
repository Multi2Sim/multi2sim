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

#include <map>
#include <memory>
#include <vector>
#include <queue>

#include <lib/cpp/IniFile.h>
#include <lib/esim/Event.h>


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


struct TimingParameters
{
	int tRC = 0;
	int tRRD = 0;
	int tRP = 0;
	int tRFC = 0;
	int tCCD = 0;
	int tRTRS = 0;
	int tCWD = 0;
	int tWTR = 0;
	int tCAS = 0;
	int tRCD = 0;
	int tOST = 0;
	int tRAS = 0;
	int tWR = 0;
	int tRTP = 0;
	int tBURST = 0;
};


class Controller
{
	int id;

	std::string name;

	// Sizes of components under this controller
	int num_channels;
	int num_ranks;
	int num_banks;
	int num_rows;
	int num_columns;
	int num_bits;

	// Timing matrix
	int timings[4][4][2][2] = {};

	// List of physical channels contained in this controller
	std::vector<std::unique_ptr<Channel>> channels;

	// Incoming request queue
	std::queue<std::shared_ptr<Request>> incoming_requests;

	// Map of ids to EventTypes for each controller's request processor
	static std::map<int, esim::EventType *> REQUEST_PROCESSORS;

	// Map of ids to EventTypes for each channel's scheduler for the
	// controller
	std::map<int, esim::EventType *> SCHEDULERS;

public:

	Controller(int id);
	Controller(int id, const std::string &section,
			misc::IniFile &config);

	/// Parse a MemoryController section of a dram configuration file
	void ParseConfiguration(const std::string &section,
			misc::IniFile &config);
	void ParseTiming(const std::string &section,
			misc::IniFile &config);

	/// Set default timing parameters for DDR3 1600
	void DefaultDDR3_1600(TimingParameters &parameters);

	int getId() const { return id; }

	int getNumChannels() const { return num_channels; }
	int getNumRanks() const { return num_ranks; }
	int getNumBanks() const { return num_banks; }
	int getNumRows() const { return num_rows; }
	int getNumColumns() const { return num_columns; }

	Channel *getChannel(int id) { return channels[id].get(); }

	/// Add a request to the controller's incoming request queue.
	void AddRequest(std::shared_ptr<Request> request);

	/// Obtain the EventType for the controller's request processor.
	static esim::EventType *getRequestProcessor(int controller)
	{
		return REQUEST_PROCESSORS[controller];
	}

	/// Create a new EventType for a controller's request processor.
	static void CreateRequestProcessor(int controller);

	/// Obtain the EventType for the channel's scheduler.
	esim::EventType *getScheduler(int channel)
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
	static void RequestProcessorHandler(esim::EventType *,
			esim::EventFrame *);

	/// Process requests in the incoming requests queue, breaking them
	/// down into their commands.
	void RunRequestProcessor();

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


class RequestProcessorFrame : public esim::EventFrame
{

public:
	RequestProcessorFrame()
	{
	}

	Controller *controller;
};

}  // namespace dram

#endif
