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

#include <algorithm>
#include <vector>

#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Event.h>

#include "Bank.h"
#include "Channel.h"
#include "Controller.h"
#include "Dram.h"
#include "Request.h"


namespace dram
{


Controller::Controller(int id)
		:
		id(id)
{
	// Create a new request processor event for this controller.
	Dram::CreateRequestProcessor(id);
}


Controller::Controller(int id, const std::string &section,
		misc::IniFile &config)
		:
		Controller(id)
{
	ParseConfiguration(section, config);
}


void Controller::ParseConfiguration(const std::string &section,
		misc::IniFile &config)
{
	// Load the name of the Controller
	// Format of section title is "MemoryController <name>"
	std::vector<std::string> section_tokens;
	misc::StringTokenize(section, section_tokens);
	name = section_tokens[1];

	// Read DRAM size settings
	num_channels = config.ReadInt(section, "NumChannels", 1);
	num_ranks = config.ReadInt(section, "NumRanks", 2);
	// int num_devices = config.ReadInt(section, "NumDevices", 8);
	num_banks = config.ReadInt(section, "NumBanks", 8);
	num_rows = config.ReadInt(section, "NumRows", 1024);
	num_columns = config.ReadInt(section, "NumColumns", 1024);
	num_bits = config.ReadInt(section, "NumBits", 8);

	// Create channels 
	for (int i = 0; i < num_channels; i++)
	{
		auto channel = std::make_shared<Channel>(i, this, num_ranks,
				num_banks, num_rows, num_columns, num_bits);
		channels.push_back(channel);
	}

	// Read DRAM timing parameters
	ParseTiming(section, config);

	// Create a set of new scheduler events for all the channels.
	Dram::CreateSchedulers(id, num_channels);
}


void Controller::ParseTiming(const std::string &section,
		misc::IniFile &config)
{
	// Create a timings object to use to read the parameters
	TimingParameters parameters;

	// Set the default timing parameters if set.  If a default is not set,
	// the parameters are all set to 0 and the user must set all of them.
	std::string set_default = config.ReadString(section, "Default", "");
	if (set_default == "DDR3_1066")
		DefaultDDR3_1066(parameters);

	// Read the timing parameters set by the user.
	parameters.tRC = config.ReadInt(section, "tRC", parameters.tRC);
	parameters.tRRD = config.ReadInt(section, "tRRD", parameters.tRRD);
	parameters.tRP = config.ReadInt(section, "tRP", parameters.tRP);
	parameters.tRFC = config.ReadInt(section, "tRFC", parameters.tRFC);
	parameters.tCCD = config.ReadInt(section, "tCCD", parameters.tCCD);
	parameters.tRTRS = config.ReadInt(section, "tRTRS", parameters.tRTRS);
	parameters.tCWD = config.ReadInt(section, "tCWD", parameters.tCWD);
	parameters.tWTR = config.ReadInt(section, "tWTR", parameters.tWTR);
	parameters.tCAS = config.ReadInt(section, "tCAS", parameters.tCAS);
	parameters.tRCD = config.ReadInt(section, "tRCD", parameters.tRCD);
	parameters.tOST = config.ReadInt(section, "tOST", parameters.tOST);
	parameters.tRAS = config.ReadInt(section, "tRAS", parameters.tRAS);
	parameters.tWR = config.ReadInt(section, "tWR", parameters.tWR);
	parameters.tRTP = config.ReadInt(section, "tRTP", parameters.tRTP);
	parameters.tBURST = config.ReadInt(section, "tBURST", parameters.tBURST);

	// Build the timing matrix.
	// A A s s
	timings[TimingActivate][TimingActivate][TimingSame][TimingSame] =
			parameters.tRC;
	// A A s d
	timings[TimingActivate][TimingActivate][TimingSame][TimingDifferent] =
			parameters.tRRD;
	// P A s d
	timings[TimingPrecharge][TimingActivate][TimingSame][TimingDifferent] =
			parameters.tRP;
	// A R s s
	timings[TimingActivate][TimingRead][TimingSame][TimingSame] =
			parameters.tRCD;
	// R R s a
	timings[TimingRead][TimingRead][TimingSame][TimingAny] =
			std::max(parameters.tBURST, parameters.tCCD);
	// R R d a
	timings[TimingRead][TimingRead][TimingDifferent][TimingAny] =
			parameters.tBURST + parameters.tRTRS;
	// W R s a
	timings[TimingWrite][TimingRead][TimingSame][TimingAny] =
			parameters.tCWD + parameters.tBURST + parameters.tWTR;
	// W R d a
	timings[TimingWrite][TimingRead][TimingDifferent][TimingAny] =
			parameters.tCWD + parameters.tBURST + parameters.tRTRS - parameters.tCAS;
	// A W s s
	timings[TimingActivate][TimingWrite][TimingSame][TimingSame] =
			parameters.tRCD;
	// R W a a
	timings[TimingRead][TimingWrite][TimingAny][TimingAny] =
			parameters.tCAS + parameters.tBURST + parameters.tRTRS - parameters.tCWD;
	// W W s a
	timings[TimingWrite][TimingWrite][TimingSame][TimingAny] =
			std::max(parameters.tBURST, parameters.tCCD);
	// W W d a
	timings[TimingWrite][TimingWrite][TimingDifferent][TimingAny] =
			parameters.tBURST + parameters.tOST;
	// A P s s
	timings[TimingActivate][TimingPrecharge][TimingSame][TimingSame] =
			parameters.tRAS;
	// R P s s
	timings[TimingRead][TimingPrecharge][TimingSame][TimingSame] =
			parameters.tBURST + parameters.tRTP - parameters.tCCD;
	// W P s s
	timings[TimingWrite][TimingPrecharge][TimingSame][TimingSame] =
			parameters.tCWD + parameters.tBURST + parameters.tWR;
}


void Controller::DefaultDDR3_1066(TimingParameters &parameters)
{
	// FIXME: These aren't actually 0.
	parameters.tRC = 0;
	parameters.tRRD = 0;
	parameters.tRP = 0;
	parameters.tRFC = 0;
	parameters.tCCD = 0;
	parameters.tRTRS = 0;
	parameters.tCWD = 0;
	parameters.tWTR = 0;
	parameters.tCAS = 0;
	parameters.tRCD = 0;
	parameters.tOST = 0;
	parameters.tRAS = 0;
	parameters.tWR = 0;
	parameters.tRTP = 0;
	parameters.tBURST = 0;
}


void Controller::AddRequest(std::shared_ptr<Request> request)
{
	// Add the request to the controller incoming request queue.
	incoming_requests.push(request);

	// Ensure the request processor is running.
	CallRequestProcessor();
}


void Controller::CallRequestProcessor()
{
	// Get the esim engine instance.
	esim::Engine *esim = esim::Engine::getInstance();

	// Get the request processor event for this controller.
	esim::EventType *request_processor = Dram::getRequestProcessor(id);

	// Check that the event isn't already scheduled.
	if (request_processor->isInFlight())
	{
		return;
	}

	// Create the frame to pass containing a reference to this controller.
	auto frame = std::make_shared<RequestProcessorFrame>();
	frame->controller = this;

	// Call the event for the request processor.
	esim->Call(request_processor, frame, nullptr, 1);
}


void Controller::RequestProcessorHandler(esim::EventType *type,
		esim::EventFrame *frame)
{
	// Get the controller pointer out of the frame.
	RequestProcessorFrame *request_frame =
			dynamic_cast<RequestProcessorFrame *>(frame);
	Controller *controller = request_frame->controller;

	// Call this controller's request processor.
	controller->RequestProcessor();
}


void Controller::RequestProcessor()
{
	// Just in case, make sure there are actually requests to process.
	if (incoming_requests.size() == 0)
		return;

	// Get the front request in the queue.
	std::shared_ptr<Request> request = incoming_requests.front();

	// Get the bank the request is destined for.
	Address *address = request->getAddress();
	Bank *bank = channels[address->logical]->getRank(address->rank)
			->getBank(address->bank);

	// Send the request to the bank to be processed.
	bank->ProcessRequest(request);

	// Remove the request from the queue.
	incoming_requests.pop();

	// If there are still requests to be processed, schedule again next cycle.
	if (incoming_requests.size() > 0)
		CallRequestProcessor();
}


void Controller::dump(std::ostream &os) const
{
	os << misc::fmt("Dumping Controller %d (%s)\n", id, name.c_str());
	os << misc::fmt("%d requests in the incoming queue\n",
			(int) incoming_requests.size());
	os << misc::fmt("%d Channels\nChannel dump:\n", (int) channels.size());

	for (auto channel : channels)
	{
		channel->dump(os);
	}
}


}  // namespace dram
